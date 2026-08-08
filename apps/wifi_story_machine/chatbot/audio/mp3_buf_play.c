#include "system/includes.h"
#include "server/audio_server.h"
#include "server/server_core.h"
#include "app_config.h"
//#include "storage_device.h"
//#include "reverb_deal.h"
#include "event/key_event.h"
#include "event/device_event.h"
#include "system/wait.h"
#include "system/app_core.h"
#include "os/os_api.h"
#include "asm/system_reset_reason.h"
//#include "volume.h"
#include "lbuf.h"

#define MP3_BUF_PLAY_NAME               "mp3_buf_play"
#define CONFIG_AUDIO_DEC_PLAY_SOURCE    "dac"

struct music_buf_file {
    FILE *file;
    char *buf;
    int len;
    int seek;
    struct server *dec_server;
    OS_MUTEX mutex;
    OS_SEM sem;
};
struct music_play_hdl {
    char play_loop_path[64];
    char *play_loop_buf;
    int play_loop_buf_len;
    int play_loop_cnt;
    int play_loop_interval_time;
    char volume;
    char play_pause;
    int play_time;
    int total_time;
    struct music_buf_file vt_file;
};

static struct music_play_hdl music_handler = {0};

#define __this  (&music_handler)

extern void music_play_dialog_timeout_cb(void);
int mp3_buf_play_res_file(char *name);
static int mp3_buf_play_loop_callback(void *priv);
/*****************************************缓冲buf文件播放****************************************/
static void mp3_buf_play_dec_server_event_handler(void *priv, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
        printf("local_music: AUDIO_SERVER_EVENT_ERR\n");
    case AUDIO_SERVER_EVENT_END:
        printf("local_music: AUDIO_SERVER_EVENT_END\n");
        mp3_buf_play_stop(priv);
        music_play_dialog_timeout_cb();
        if (__this->play_loop_path[0]) {
            sys_timeout_add_to_task("sys_timer", __this->play_loop_path, mp3_buf_play_res_file, 500);
        } else if (__this->play_loop_buf && __this->play_loop_buf_len > 0 && __this->play_loop_cnt > 0) {
            sys_timeout_add_to_task("sys_timer", NULL, mp3_buf_play_loop_callback, __this->play_loop_interval_time ? __this->play_loop_interval_time : 500);
        }
        break;
    case AUDIO_SERVER_EVENT_CURR_TIME:
        printf("play_time: %d\n", argv[1]);
        __this->play_time = argv[1];
        break;
    }
}

static int mp3_buf_play_init(void)
{
    if (os_mutex_valid(&__this->vt_file.mutex)) {
        return 0;
    }
    memset(__this, 0, sizeof(struct music_play_hdl));

    os_mutex_create(&__this->vt_file.mutex);
    os_sem_create(&__this->vt_file.sem, 0);
    sys_volume_read(&__this->volume);

    if (!__this->vt_file.dec_server) {
        __this->vt_file.dec_server = server_open("audio_server", "dec");
        if (!__this->vt_file.dec_server) {
            return -1;
        }
        server_register_event_handler_to_task(__this->vt_file.dec_server, &__this->vt_file, mp3_buf_play_dec_server_event_handler, "sys_timer");
    }
    return 0;
}

static void mp3_buf_play_uninit(void)
{
    mp3_buf_play_stop(&__this->vt_file);
    server_close(__this->vt_file.dec_server);
    __this->vt_file.dec_server = NULL;
    memset(__this, 0, sizeof(struct music_play_hdl));
    log_i("music_play_uninit\n");
}
static int music_audio_read(void *file, void *buf, u32 len)
{
    struct music_buf_file *vt_file = (struct music_buf_file *)file;
    if (!vt_file) {
        return 0;
    }

    int size = MIN(len, vt_file->len - vt_file->seek);
    if (!size) {
        return 0;
    }
    memcpy(buf, vt_file->buf + vt_file->seek, size);
    vt_file->seek += size;
    return size;
}
static int music_audio_seek(void *file, u32 offset, int seek_mode)
{
    struct music_buf_file *vt_file = (struct music_buf_file *)file;
    if (!vt_file) {
        return 0;
    }
    switch (seek_mode) {
    case SEEK_SET:
        vt_file->seek = offset;
        break;
    case SEEK_CUR:
        vt_file->seek += offset;
        break;
    case SEEK_END:
        vt_file->seek = vt_file->len;
        break;
    default:
        return -1;
    }
    return 0;
}
static int music_audio_flen(void *file)
{
    struct music_buf_file *vt_file = (struct music_buf_file *)file;
    if (!vt_file) {
        return 0;
    }
    return vt_file->len;
}

static int music_audio_close(void *file)
{
    struct music_buf_file *vt_file = (struct music_buf_file *)file;
    if (!vt_file) {
        return 0;
    }
    vt_file->buf = NULL;
    vt_file->len = 0;
    vt_file->seek = 0;
    return 0;
}

//解码需要的数据访问句柄
static const struct audio_vfs_ops mp3_buf_audio_dec_vfs_ops = {
    .fread = music_audio_read,
    .fseek = music_audio_seek,
    .flen  = music_audio_flen,
    .fclose = music_audio_close,
};
//停止播放
int mp3_buf_play_stop(void *priv)
{
    int err = 0;
    union audio_req req = {0};
    struct music_buf_file *vt_file = &__this->vt_file;
    struct server *dec_server = vt_file->dec_server;
    os_mutex_pend(&vt_file->mutex, 1000);
    if ((!vt_file->buf  && !vt_file->file) || !dec_server) {
        __this->play_pause = 0;
        os_mutex_post(&vt_file->mutex);
        return 0;
    }
    req.dec.cmd = AUDIO_DEC_STOP;
    server_request(dec_server, AUDIO_REQ_DEC, &req);

//    int argv[2];
//    argv[0] = AUDIO_SERVER_EVENT_END;
//    argv[1] = 0;
//    server_event_handler_del(dec_server, 2, argv);

    if (vt_file->buf) {
        vt_file->buf = NULL;
        vt_file->len = 0;
        vt_file->seek = 0;
    }
    if (__this->vt_file.dec_server) {
        server_close(__this->vt_file.dec_server);
        __this->vt_file.dec_server = NULL;
    }
    __this->play_pause = 0;
    if (vt_file->file) {
        fclose(vt_file->file);
        vt_file->file = NULL;
    }
    printf("mp3_buf_play_stop\n");
    os_sem_post(&vt_file->sem);
    os_mutex_post(&vt_file->mutex);
    return 0;
}
int mp3_buf_play_stop_pause(char stop)
{
    int err = 0;
    union audio_req req = {0};
    struct music_buf_file *vt_file = &__this->vt_file;
    struct server *dec_server = vt_file->dec_server;
    os_mutex_pend(&vt_file->mutex, 1000);
    if ((!vt_file->buf  && !vt_file->file) || !dec_server) {
        os_mutex_post(&vt_file->mutex);
        return 0;
    }
    if (stop) {
        if (!__this->play_pause) {
            req.dec.cmd = AUDIO_DEC_PP;
            server_request(dec_server, AUDIO_REQ_DEC, &req);
            __this->play_pause = stop;
            printf("music_play_pause\n");
        }
    } else {
        if (__this->play_pause) {
            req.dec.cmd = AUDIO_DEC_PP;
            server_request(dec_server, AUDIO_REQ_DEC, &req);
            __this->play_pause = stop;
            printf("music_play_resum\n");
        }
    }
    os_mutex_post(&vt_file->mutex);
    return __this->play_pause;
}
int mp3_buf_play_stop_status(void)
{
    if (!__this->vt_file.buf || !__this->vt_file.dec_server || __this->play_pause) {
        return 1;
    }
    return 0;
}
int mp3_buf_play_stop_waite(void)
{
    struct music_buf_file *vt_file = &__this->vt_file;
    struct server *dec_server = vt_file->dec_server;
    os_mutex_pend(&vt_file->mutex, 1000);
    if (!__this->vt_file.buf || !__this->vt_file.dec_server || __this->play_pause) {
        os_mutex_post(&vt_file->mutex);
        return 1;
    }
    os_mutex_post(&vt_file->mutex);
    return os_sem_pend(&vt_file->sem, 1500);
}
//设置音量大小
int mp3_buf_play_set_volume_step(int step)
{
    union audio_req req = {0};
    struct music_buf_file *vt_file = &__this->vt_file;
    struct server *dec_server = vt_file->dec_server;

    if ((!vt_file->buf  && !vt_file->file)) {
        return -1;
    }
    os_mutex_pend(&vt_file->mutex, 1000);
    sys_volume_read(&__this->volume);
    __this->volume = sys_volume_chack(__this->volume + step);

    sys_volume_write(&__this->volume);

    printf("->set_dec_volume: %d\n", __this->volume);
    req.dec.cmd     = AUDIO_DEC_SET_VOLUME;
    req.dec.volume  = GET_SET_VOLUME(__this->volume);
    server_request(dec_server, AUDIO_REQ_DEC, &req);
    os_mutex_post(&vt_file->mutex);
    return 0;
}
//设置音量大小
int mp3_buf_play_set_volume(int volume)
{
    union audio_req req = {0};
    struct music_buf_file *vt_file = &__this->vt_file;
    struct server *dec_server = vt_file->dec_server;

    if ((!vt_file->buf  && !vt_file->file)) {
        return -1;
    }
    os_mutex_pend(&vt_file->mutex, 1000);

    __this->volume = sys_volume_chack(volume);

    sys_volume_write(&__this->volume);

    printf("->set_dec_volume: %d\n", __this->volume);
    req.dec.cmd     = AUDIO_DEC_SET_VOLUME;
    req.dec.volume  = GET_SET_VOLUME(__this->volume);
    server_request(dec_server, AUDIO_REQ_DEC, &req);
    os_mutex_post(&vt_file->mutex);
    return 0;
}
//解码文件
int mp3_buf_play_file(char *buf, int len)
{
    int err;
    union audio_req req = {0};
    struct music_buf_file *vt_file = &__this->vt_file;
    struct server *dec_server = vt_file->dec_server;


    mp3_buf_play_init();
    os_mutex_pend(&vt_file->mutex, 1000);
    if (!len && buf && strstr(buf, CONFIG_VOICE_PROMPT_FILE_PATH)) {
        printf("--> mp3_buf_play_file : %s \n", buf);
        if (vt_file->file) {
            mp3_buf_play_stop(vt_file);
        }
        vt_file->file = fopen(buf, "rb");
        if (!vt_file->file) {
            printf("fopen err %s\n", buf);
            os_mutex_post(&vt_file->mutex);
            return -1;
        }
    } else {
        vt_file->buf = buf;
        vt_file->len = len;
        vt_file->seek = 0;
    }
    if (!__this->vt_file.dec_server) {
        __this->vt_file.dec_server = server_open("audio_server", "dec");
        if (!__this->vt_file.dec_server) {
            os_mutex_post(&vt_file->mutex);
            return -1;
        }
        server_register_event_handler_to_task(__this->vt_file.dec_server, &__this->vt_file, mp3_buf_play_dec_server_event_handler, "sys_timer");
        dec_server = vt_file->dec_server;
    }
    sys_volume_read(&__this->volume);

    req.dec.dec_type        = "mp3";
    req.dec.cmd             = AUDIO_DEC_OPEN;
    req.dec.volume          = GET_SET_VOLUME(__this->volume);
    req.dec.output_buf_len  = 4 * 1024;
    req.dec.file            = vt_file->file ? vt_file->file : (FILE*)vt_file;
    req.dec.channel         = 0;
    req.dec.sample_rate     = 0;
    req.dec.priority        = 1;
    req.dec.vfs_ops         = vt_file->file ? NULL : &mp3_buf_audio_dec_vfs_ops;
    req.dec.sample_source   = CONFIG_AUDIO_DEC_PLAY_SOURCE;
    req.dec.force_sr        = 0;//sample_rate > 0 ? sample_rate : 0;//强制使用采样率
    //req.dec.attr = AUDIO_ATTR_NO_WAIT_READY;

#if 0   //变声变调功能
    req.dec.speedV = 75; // >80是变快，<80是变慢，建议范围：30到130
    req.dec.pitchV = 32768; // >32768是音调变高，<32768音调变低，建议范围20000到50000
    req.dec.attr = AUDIO_ATTR_PS_EN;
#endif

#if TCFG_EQ_ENABLE && defined EQ_CORE_V1
    req.dec.attr |= AUDIO_ATTR_EQ_EN;
#if TCFG_LIMITER_ENABLE
    req.dec.attr |= AUDIO_ATTR_EQ32BIT_EN;
#endif
#endif

    /*
    #if TCFG_DRC_ENABLE
        req.dec.attr |= AUDIO_ATTR_DRC_EN;
    #endif */

    err = server_request(dec_server, AUDIO_REQ_DEC, &req);
    if (err) {
        printf("audio_dec_open: err = %d\n", err);
        mp3_buf_play_stop(vt_file);
        os_mutex_post(&vt_file->mutex);
        return err;
    }
    __this->play_time = req.dec.play_time;
    __this->total_time = req.dec.total_time; //获取播放总时长
    req.dec.cmd = AUDIO_DEC_START;
    err = server_request(dec_server, AUDIO_REQ_DEC, &req);
    if (err) {
        printf("audio_dec_start: err = %d\n", err);
        os_mutex_post(&vt_file->mutex);
        return err;
    }
    __this->play_pause = 0;
    printf("mp3_buf_play_file: ok\n");
    os_sem_set(&vt_file->sem, 0);
    os_mutex_post(&vt_file->mutex);
    return 0;
}
static int mp3_play_init(void)
{
    if (production_io_is_enter() || is_production_test_enter(0)) {
        return 0;
    }
    mp3_buf_play_init();
}
late_initcall(mp3_play_init);

int mp3_buf_play_res_file(char *name)
{
    char path[48] = {0};
    sprintf(path, "%s%s", CONFIG_VOICE_PROMPT_FILE_PATH, name);
    return mp3_buf_play_file(path, 0);
}
int mp3_buf_play_res_file_loop(void *name)
{
    if (name) {
        int ret = mp3_buf_play_res_file(name);
        if (!ret) {
            strcpy(__this->play_loop_path, name);
        }
        return ret;
    }
    return 0;
}
int mp3_buf_play_res_file_unloop(void)
{
    memset(__this->play_loop_path, 0, sizeof(__this->play_loop_path));
    return mp3_buf_play_stop(NULL);
}

static int mp3_buf_play_loop_callback(void *priv)
{
    if (__this->play_loop_buf && __this->play_loop_buf_len > 0 && __this->play_loop_cnt > 0) {
        __this->play_loop_cnt--;
        mp3_buf_play_file(__this->play_loop_buf, __this->play_loop_buf_len);
        return 0;
    }
    return -1;
}
int mp3_buf_play_loop(int optcode, void *buf, int len)
{
    switch (optcode) {
    case 0:
        if (mp3_buf_play_file(buf, len)) {
            break;
        }
        __this->play_loop_buf = buf;
        __this->play_loop_buf_len = len;
        __this->play_loop_cnt = 100;
        __this->play_loop_interval_time = 500;
        break;
    case 1:
        __this->play_loop_buf = buf;
        __this->play_loop_buf_len = len;
        __this->play_loop_cnt = 100;
        __this->play_loop_interval_time = 500;
        break;
    case 2:
        __this->play_loop_buf = NULL;
        __this->play_loop_buf_len = 0;
        __this->play_loop_cnt = 0;
        __this->play_loop_interval_time = 0;
        break;
    case 3:
        __this->play_loop_cnt = (int)buf;
        __this->play_loop_interval_time = len;
        break;
    }
    return 0;
}
