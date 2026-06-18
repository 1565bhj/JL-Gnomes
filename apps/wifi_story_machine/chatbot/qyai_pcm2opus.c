#include "system/includes.h"
#include "server/audio_server.h"
#include "server/server_core.h"
#include "app_config.h"
#include "qyai_config.h"
#include "qyai_pcm2opus.h"
#include "os/os_api.h"

extern int websockets_send_pcm_buf_start(void);
extern int websockets_send_pcm_buf_end(void);
extern int websockets_send_pcm_buf_push(char *buf, int len);
extern int websockets_send_audio_buf_start(void);
extern int websockets_send_audio_buf_end(void);
extern int websockets_send_audio_buf_push(char *buf, int len);

static int QYAI_OPUS_SAMPLE_RATE = 16000;

#define QYAI_OPUS_BITRATE           16000
/* ivw 每包 320B(10ms)；16k Opus 虚拟源与 dui/turing 一致用 640B(20ms) */
#define QYAI_IVW_PCM_CHUNK          320
#define QYAI_OPUS_ENC_FRAME_BYTES   640
#define QYAI_OPUS_OUTPUT_BUF_LEN    (8 * 1024)
/* 约 4s 的 320B 块缓冲 */
#define QYAI_OPUS_PCM_CBUF_SIZE     (QYAI_IVW_PCM_CHUNK * 128)

/*
 * 云端未部署 Opus 时置 1：只跑本地编码，不走 websockets_send_pcm_buf_*，
 * 验证完成后务必改回 0 再联调云端。
 */
#ifndef QYAI_OPUS_LOCAL_VERIFY
#define QYAI_OPUS_LOCAL_VERIFY 0
#endif

static u16 s_pcm_frame_bytes = QYAI_OPUS_ENC_FRAME_BYTES;
static u32 s_verify_frame_cnt;
static u32 s_verify_byte_cnt;
static u16 s_verify_len_min;
static u16 s_verify_len_max;
static void *s_opus_enc;
static u8 s_enc_running;
static u8 s_pcm_eof;
static u8 s_pcm_cbuf_buf[QYAI_OPUS_PCM_CBUF_SIZE];
static cbuffer_t s_pcm_cbuf;
static OS_SEM s_pcm_sem;
static u8 s_sem_inited;
static int s_last_push_ret;
static u32 s_read_input_cnt;

static void qyai_opus_enc_event_handler(void *priv, int argc, int *argv)
{
    (void)priv;
    (void)argc;
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
        printf("qyai opus uplink: AUDIO_SERVER_EVENT_ERR\n");
        break;
    case AUDIO_SERVER_EVENT_END:
        printf("qyai opus uplink: AUDIO_SERVER_EVENT_END\n");
        break;
    default:
        break;
    }
}

int qyai_audio_use_opus(void)
{
    return !strcmp(qyai_chat_audio_enc_format(), "opus");
}

void qyai_audio_set_pcm_frame_bytes(u16 bytes)
{
    if (bytes > 0 && bytes <= 2048) {
        s_pcm_frame_bytes = bytes;
    }
}

static void audio_verify_stat_reset(void)
{
    s_verify_frame_cnt = 0;
    s_verify_byte_cnt = 0;
    s_verify_len_min = 0xffff;
    s_verify_len_max = 0;
}

static int audio_vfs_fwrite(void *file, void *data, u32 len)
{
    s_last_push_ret = websockets_send_audio_buf_push((char *)data, len);
    if (s_last_push_ret < 0) {
        return s_last_push_ret;
    }
    return (int)len;
}

static int audio_vfs_fclose(void *file)
{
    (void)file;
    return 0;
}

static const struct audio_vfs_ops audio_vfs_ops = {
    .fwrite = audio_vfs_fwrite,
    .fclose = audio_vfs_fclose,
};

static u32 audio_read_input(u8 *buf, u32 len)
{
    u32 rlen = 0;

    if (!s_enc_running) {
        return (u32) -1;
    }

    do {
        if (!s_enc_running) {
            return (u32) -1;
        }

        rlen = cbuf_read(&s_pcm_cbuf, buf, len);
        if (rlen == len) {
            break;
        }

        if (s_pcm_eof) {
            return (u32) -1;
        }

        os_sem_pend(&s_pcm_sem, 100);
    } while (!rlen);
    s_read_input_cnt++;
    return rlen;
}

static void audio_sem_init(void)
{
    if (!s_sem_inited) {
        os_sem_create(&s_pcm_sem, 0);
        s_sem_inited = 1;
    }
}

static int qyai_audio_pcm_write(u8 *buf, int len)
{
    u32 wlen;

    if (!s_opus_enc || !s_enc_running) {
        return -1;
    }

    if (!buf || len <= 0) {
        return 0;
    }

    wlen = cbuf_write(&s_pcm_cbuf, buf, len);
    if (wlen != (u32)len) {
        printf("qyai opus uplink: pcm cbuf full, wlen=%u len=%d\n", wlen, len);
        return -1;
    }
    if (os_sem_valid(&s_pcm_sem)) {
        os_sem_post(&s_pcm_sem);
    }
    return 0;
}

static int qyai_opus_enc_open(void)
{
    union audio_req req = {0};
    int err;

    if (s_opus_enc) {
        return 0;
    }

    audio_sem_init();

    s_opus_enc = server_open("audio_server", "enc");
    if (!s_opus_enc) {
        printf("qyai opus uplink: enc open fail\n");
        return -1;
    }

    server_register_event_handler_to_task(s_opus_enc, NULL,
                                          qyai_opus_enc_event_handler, "app_core");

    cbuf_init(&s_pcm_cbuf, s_pcm_cbuf_buf, sizeof(s_pcm_cbuf_buf));
    cbuf_clear(&s_pcm_cbuf);
    os_sem_set(&s_pcm_sem, 0);
    s_read_input_cnt = 0;
    s_pcm_eof = 0;

    req.enc.cmd = AUDIO_ENC_OPEN;
    req.enc.channel = 1;
    req.enc.sample_rate = QYAI_OPUS_SAMPLE_RATE;
    req.enc.format = "opus";
    req.enc.no_header = 1;
    req.enc.bitrate = QYAI_OPUS_BITRATE;
    req.enc.sample_source = "virtual";
    req.enc.read_input = audio_read_input;
    req.enc.vfs_ops = &audio_vfs_ops;
    req.enc.frame_size = s_pcm_frame_bytes;
    req.enc.output_buf_len = QYAI_OPUS_OUTPUT_BUF_LEN;
    req.enc.vir_data_wait = 1;

    s_enc_running = 1;

    err = server_request(s_opus_enc, AUDIO_REQ_ENC, &req);
    if (err) {
        printf("qyai opus uplink: AUDIO_ENC_OPEN err %d\n", err);
        server_close(s_opus_enc);
        s_opus_enc = NULL;
        s_enc_running = 0;
        return -1;
    }
    return 0;
}

static void qyai_opus_enc_close(void)
{
    union audio_req req = {0};

    if (!s_opus_enc) {
        return;
    }

    s_enc_running = 0;
    s_pcm_eof = 1;
    os_sem_post(&s_pcm_sem);

    req.enc.cmd = AUDIO_ENC_CLOSE;
    server_request(s_opus_enc, AUDIO_REQ_ENC, &req);
    server_close(s_opus_enc);
    s_opus_enc = NULL;
    cbuf_clear(&s_pcm_cbuf);
}

int qyai_audio_send_init(void)
{
    return 0;
}

int qyai_audio_send_start(void)
{
    if (qyai_audio_use_opus()) {
        if (qyai_opus_enc_open()) {
            return -1;
        }
    }
    return websockets_send_audio_buf_start();
}

void qyai_audio_sample_rate_set(int sample_rate)
{
    QYAI_OPUS_SAMPLE_RATE = sample_rate;
}

int qyai_audio_send_push(u8 *buf, int len)
{
    if (!qyai_audio_use_opus()) {
        return websockets_send_pcm_buf_push((char *)buf, len);
    }

    return qyai_audio_pcm_write(buf, len);
}

void qyai_audio_send_abort(void)
{
    qyai_opus_enc_close();
}

int qyai_audio_send_end(void)
{
    if (qyai_audio_use_opus()) {
        s_pcm_eof = 1;
        os_sem_post(&s_pcm_sem);
        os_time_dly(2);
        qyai_opus_enc_close();
    }
    return websockets_send_audio_buf_end();
}
