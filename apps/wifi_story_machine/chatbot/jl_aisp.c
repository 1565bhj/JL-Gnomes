#include "server/audio_server.h"
#include "server/server_core.h"
#include "generic/circular_buf.h"
#include "json_c/json_tokener.h"
#include "fs/fs.h"
#include "asm/sfc_norflash_api.h"
#include "os/os_api.h"
#include "event.h"
#include "event/key_event.h"
#include "app_config.h"
#include "jlsp_far_keyword.h"
#include <time.h>

#if (defined CONFIG_ASR_ALGORITHM) && (CONFIG_ASR_ALGORITHM == JLKWS_SXY_ALGORITHM)
//单麦方案
#define JLASR_EN    1

#define ASR_CONTINIU_EN     ASR_CONTINIU_ENABLE//连续识别对话
#define PCM_SAVE_TEST       0//pcm保存测试

//#undef CONFIG_AEC_ENC_ENABLE
#ifdef CONFIG_AEC_ENC_ENABLE
#define AEC_ENC_HARDWAIR_ENABLE       0   //1:使用硬件回采 0:使用软件回采
#endif
#define AISP_DUAL_MIC_ALGORITHM    0   //0选择单mic/1选择双mic算法


#define AEC_EN              BIT(0)
#define NLP_EN              BIT(1)
#define ANS_EN              BIT(2)
/*aec module enable bit define*/
#define AEC_MODE_ADVANCE    (AEC_EN | NLP_EN | ANS_EN)
#define AEC_MODE_REDUCE     (NLP_EN | ANS_EN)
#define AEC_MODE_SIMPLEX    (ANS_EN)


#define ONCE_SR_POINTS  320//160
#define AISP_BUF_SIZE   2 * (4  * ONCE_SR_POINTS)   //跑不过来时适当加大倍数


#define REC_AEC_MIC_GAIN_AVR        70  //aec、mic的gain值在该值相等
#define REC_TO_AEC_MIC_GAIN_DIIF    10  //mic的gain值比aec gain值大多少
#define AEC_MIC_GAIN_MAX            80  //最大的aec gain值
#define AEC_MIC_GAIN_MIN            40  //最大的aec gain值

//y = -x + 140
#define AEC_GAIN(v)    (int)(-(v) + 140)
//y = -x + 130
#define MIC0_GAIN(v)    (int)(-(v) + 130)
#define MIC1_GAIN(v)    (MIC0_GAIN(v) - 10)//常理需要相等，在播放歌曲音乐时候，近喇叭端需要减10

extern int websockets_send_pcm_buf_start(void);
extern int websockets_send_pcm_buf_end(void);
extern int websockets_send_pcm_buf_push(char *buf, int len);
extern int websocket_client_thread_create(void *priv);
extern int websocket_client_thread_create_new(void *priv, char key_vad);
extern int websockets_client_next_dialogue_init(void);
extern int websockets_dialogue_timeout_init(int time_out, char use_voice_note);
extern int websockets_close_request(char force_close);

extern int wifi_sta_is_connected(void);
extern void sdfile_save_test(char *buf, int len, char close);
extern void music_buf_play_set_stop(void);
extern void websockets_free_lbuf_buf(void);
extern void music_buf_play_free_lbuf(void);
extern int music_play_res_file(const char *name);
extern int music_play_hello(void);
extern int music_buf_play_stop(void);
extern int music_buf_play_stop_waite(void);

int aisp_aec_gain_set(unsigned char volume);
int aisp_mic_gain_set(unsigned char volume);

extern u32 timer_get_ms(void);
extern void aisp_resume(void);

extern int aisp_aec_gain;
extern int aisp_mic0_gain;
extern int aisp_mic1_gain;

enum send_pcm_type {
    SEND_PCM_INI = 0,
    SEND_PCM_START,
    SEND_PCM_DOING,
    SEND_PCM_STOP,
    SEND_PCM_PAUSE,
};
enum {
    ASR_WAKEUP_EVENT = 2,//小杰小杰，小杰同学
    ASR_PLAY_MUSIC_EVENT = 4,//播放音乐、播放歌曲
    ASR_STOP_MUSIC_EVENT = 5,//暂停播放
    ASR_VOLUME_INC_EVENT = 7,
    ASR_VOLUME_DEC_EVENT = 8,
    ASR_SONG_PREVIOUS_EVENT = 9,
    ASR_SONG_NEXT_EVENT = 10,
};
static const float confidence[8] = {
    0.4, 0.4, 0.4, 0.4, //小杰小杰，小杰同学，播放音乐，暂停播放
    0.4, 0.4, 0.4, 0.4, //增大音量，减小音量，上一首, 下一首
};
typedef struct jl_asr {
    void *private_heap;
    int private_heap_size;
    void *share_heap;
    int share_heap_size;
    void *kws;
    int model;
    int model_size;
    int online;
} JL_INFO;
static JL_INFO jl_asr_info = {0};

typedef struct {
    int len;
    char name[64];
} FILE_INFO;

static struct {
    int pid;
    u16 sample_rate;
    u8 volatile exit_flag;
    u8 volatile run_flag;
    u8 volatile send_pcm;
    u8 auth_flag;
    OS_SEM sem;
    s16 mic_buf[AISP_BUF_SIZE * (1 + AISP_DUAL_MIC_ALGORITHM)];
    void *mic_enc;
    cbuffer_t mic_cbuf;
} aisp_server;

extern int keyworld_start;
int pcm_save_start = 0;

#define __this (&aisp_server)


#if JLASR_EN
int jl_asr_init(void)
{
    //============杰理-小杰小杰语音识别===============//
    int model = 0, model_size, private_heap_size, share_heap_size;
    int online = 0;
    void *kws = NULL;
    u8 *private_heap = NULL, *share_heap = NULL;

    if (!jl_asr_info.private_heap) {
        jl_far_kws_model_get_heap_size(model, &model_size, &private_heap_size, &share_heap_size);

        private_heap = zalloc(private_heap_size);
        if (!private_heap) {
            goto __exit;
        }

        share_heap   = zalloc(share_heap_size);
        if (!share_heap) {
            goto __exit;
        }

        kws = jl_far_kws_model_init(model, private_heap, private_heap_size, share_heap, share_heap_size, model_size, confidence, online);
        if (!kws) {
            goto __exit;
        }
        jl_asr_info.private_heap = private_heap;
        jl_asr_info.share_heap = share_heap;
        jl_asr_info.private_heap_size = private_heap_size;
        jl_asr_info.share_heap_size = share_heap_size;
        jl_asr_info.kws = kws;
        jl_asr_info.model = model;
        jl_asr_info.model_size = model_size;
        jl_asr_info.online = online;
    } else {
        kws = jl_far_kws_model_init(jl_asr_info.model, jl_asr_info.private_heap, jl_asr_info.private_heap_size,
                                    jl_asr_info.share_heap, jl_asr_info.share_heap_size,
                                    jl_asr_info.model_size,
                                    confidence,
                                    jl_asr_info.online);
        if (!kws) {
            goto __exit;
        }
        jl_asr_info.kws = kws;
    }
    return 0;
__exit:
    if (private_heap) {
        free(private_heap);
    }
    if (share_heap) {
        free(share_heap);
    }
    jl_asr_info.kws = NULL;
    return -1;
    //============杰理-小杰小杰语音识别===============//
}
int jl_asr_process(u8 *pcm, int len)
{
    if (!jl_asr_info.kws) {
        return 0;
    }
    return jl_far_kws_model_process(jl_asr_info.kws, jl_asr_info.model, (u8 *)pcm, len);
}
void jl_asr_reset(void)
{
    jl_far_kws_model_reset(jl_asr_info.kws);
}
void jl_asr_free(void)
{
    if (jl_asr_info.kws) {
        jl_far_kws_model_free(jl_asr_info.kws);
        jl_asr_info.kws = NULL;
    }
#if 0
    if (jl_asr_info.private_heap) {
        free(jl_asr_info.private_heap);
        jl_asr_info.private_heap = NULL;
    }
    if (jl_asr_info.share_heap) {
        free(jl_asr_info.share_heap);
        jl_asr_info.share_heap = NULL;
    }
#endif
}
int jl_asr_reinit(void)
{
    int err;
    jl_asr_free();
    if (jl_asr_init()) {
        printf("->err jl_asr_reinit\n");
    }
}
#endif // JLASR_EN

static char jl_wake = 0;
void aisp_wake(char index)//0:小飞小飞，8：配网模式
{
    jl_wake = index + 1;
}
void aisp_all_pause(char stop)
{
    int to = 100;
    if (stop) {
        __this->send_pcm = SEND_PCM_PAUSE;
        sys_vad_clear(0);
    } else {
        __this->send_pcm = SEND_PCM_INI;
        sys_vad_clear(1);
    }
    keyworld_start = 0;
    music_buf_play_set_stop();
    websockets_free_lbuf_buf();
    music_buf_play_free_lbuf();
    websockets_close_request(1);
    music_buf_play_stop_waite();//等待关闭完成再播放提示音
    net_music_num_clear();
    while (!music_buf_play_stop_staus() && --to) {
        os_time_dly(1);
    }
}
static int first_500ms_len = 0;
static int first_500ms_copy = 0;
void pcm_buf_clear(void)
{
    if (__this->send_pcm != SEND_PCM_PAUSE) {
        __this->send_pcm = SEND_PCM_INI;
        first_500ms_len = 0;
        first_500ms_copy = ASR_FIRST_BEFORE_EN;
    }
}
static void aisp_task(void *priv)
{
    short buf[ONCE_SR_POINTS * (2 + AISP_DUAL_MIC_ALGORITHM)] ALIGNED(32);
    char *first_500ms_buf = malloc(ASR_FIRST_BEFORE_BYTES);//500ms
    u32 time = 0, time_cnt = 0, cnt = 0, asr_rst = 0;
    u32 mic_len, linein_len;
    int ret;
    void *kws;

    keyworld_start = 0;

#if (ASR_USED_AVD == ASR_WEBRTC_AVD)
    void *vad_cbuf = sys_vad_init(__this->sample_rate, AISP_BUF_SIZE * (1 + AISP_DUAL_MIC_ALGORITHM));
    if (vad_cbuf) {
        sys_vad_create(NULL);
    } else {
        printf("->vad_cbuf init err\n");
    }
#endif

#if JLASR_EN
    if (jl_asr_init()) {
        printf("->err in jl_asr_init\n");
    }
#endif // JLASR_EN

    __this->auth_flag = 1;

    aisp_resume();

    while (1) {
        if (__this->exit_flag) {
            break;
        }

        if (!__this->run_flag) {
            os_sem_pend(&__this->sem, 0);
            continue;
        }

        if (__this->exit_flag) {
            break;
        }

        //line in回采
        if ((cbuf_get_data_size(&__this->mic_cbuf) < ONCE_SR_POINTS * 2 * (AISP_DUAL_MIC_ALGORITHM + 1))) {
            os_sem_pend(&__this->sem, 0);
            continue;
        }
        short tempbuf[ONCE_SR_POINTS * (2 + AISP_DUAL_MIC_ALGORITHM)] ALIGNED(32);
        mic_len = cbuf_read(&__this->mic_cbuf, tempbuf, ONCE_SR_POINTS * 2 * (AISP_DUAL_MIC_ALGORITHM + 1));
        if (!mic_len) {
            continue;
        }
#if (ASR_USED_AVD == ASR_WEBRTC_AVD)
        if (vad_cbuf) {
            sys_vad_send_msg(0x1, vad_cbuf, tempbuf, mic_len);
        }
#endif

#if DIGITAL_VOL_AGC
        //数字放大
#define GAIN_MAX    30
        static double pcm_gain = 5;//数字放大倍数，初始值为3
        static int pcm_max = 0;//记录最大值，最大值不能超过32000
        static int pcm_min = 0;//记录最小值，最大值不能超过-32000
        int pcm_cont = mic_len / 2;
        int tmp;
        short *val = (short *)tempbuf;
        for (volatile int j = 0; j < pcm_cont; j++) {
            tmp = (*val) * pcm_gain;
            if (tmp > pcm_max) {
                pcm_max = tmp;
            }
            if (tmp < pcm_min) {
                pcm_min = tmp;
            }
            if (pcm_max > 32000 || pcm_min < -32000) {
                pcm_gain /= 1.5;
                pcm_max /= 1.5;
                pcm_min /= 1.5;
                tmp = (*val) * pcm_gain;
            }
            *val++ = (short)tmp;
        }
        pcm_gain += 0.05;
        if (pcm_gain > GAIN_MAX) {
            pcm_gain = GAIN_MAX;
        }
#endif
#ifdef WIFI_PCM_STREAN_SOCKET_ENABLE
        if (wifi_pcm_stream_socket_valid()) {
            wifi_pcm_stream_socket_send(tempbuf, mic_len);
        }
#endif

        if (first_500ms_buf && first_500ms_copy) {
            if ((first_500ms_len + mic_len) <= ASR_FIRST_BEFORE_BYTES) { //还没满buf则继续填满
                memcpy((char*)first_500ms_buf + first_500ms_len, (char*)tempbuf, mic_len);
                first_500ms_len += mic_len;
            } else if (mic_len < ASR_FIRST_BEFORE_BYTES) { //满buf则继续更新最新数据在buf最后区域
                if (first_500ms_len > mic_len) {
                    memcpy((char*)first_500ms_buf, (char*)first_500ms_buf + mic_len, first_500ms_len - mic_len);
                    memcpy((char*)first_500ms_buf + first_500ms_len - mic_len, (char*)tempbuf, mic_len);
                } else {
                    memcpy((char*)first_500ms_buf, (char*)tempbuf, mic_len);
                    first_500ms_len = mic_len;
                }
            }
        }

#if PCM_SAVE_TEST
        static int time_1;
        if (!time_1) {
            time_1 = timer_get_ms();
            sdfile_onefile_save_test(tempbuf, mic_len, 0);//sd卡存储PCM
        } else if (time_1 != 0x12345678) {
            if (timer_get_ms() - time_1 >= 15 * 1000) {
                time_1 = 0x12345678;
                sdfile_onefile_save_test(tempbuf, mic_len, 1);//sd卡存储PCM
            } else {
                sdfile_onefile_save_test(tempbuf, mic_len, 0);//sd卡存储PCM
            }
        }
#endif // PCM_SAVE_TEST


        //在重组数据前计算个数
        mic_len /= 2;//mic_len设置为PCM个数
        //重组数据
        for (u32 i = 0, j = 0; j < ONCE_SR_POINTS; ++j) {//重组数据后PCM收数据格式：LRE LER 左通道右通道LINEIN
            buf[i++] = tempbuf[j];
        }

#if JLASR_EN
        ret = jl_asr_process(tempbuf, sizeof(tempbuf));
        if (ret > 1 || jl_wake) {
            printf("============ASR %d ===========time = %d ms\n", ret, timer_get_ms() - time);
            //add your button event according to ret
            if (jl_wake == 1) {
                ret = ASR_WAKEUP_EVENT;
            }
            struct key_event key = {0};
            switch (ret) {
            case ASR_WAKEUP_EVENT:;//小杰小杰，小杰同学
                extern int sys_connect_net_success(void);
                if (sys_connect_net_success()) {
                    if (__this->send_pcm == SEND_PCM_INI) {
                        music_buf_play_set_stop();
                        websockets_free_lbuf_buf();
                        music_buf_play_free_lbuf();
                        websockets_close_request(1);
                        music_buf_play_stop_waite();//等待关闭完成再播放提示音
                        music_play_hello();
                        music_buf_play_stop_waite();//等待关闭完成再播放提示音
                        websocket_client_thread_create(1);
                        keyworld_start = ASR_WAKEUP_EVENT;
                    } else {
                        sys_timeout_add_to_task("sys_timer", NULL, music_play_hello, 10);//music_play_hello();
                    }
                } else {
#ifdef CONFIG_LTE_PHY_ENABLE
                    music_play_res_file("NetDiscon.mp3");
#else
                    music_play_res_file("WifiConErr.mp3");
#endif
                }
                break;
            case ASR_PLAY_MUSIC_EVENT://播放音乐、播放歌曲
//                key.action = KEY_EVENT_CLICK;
//                key.value = KEY_PLAY;
                break;
            case ASR_STOP_MUSIC_EVENT://暂停播放
                key.action = KEY_EVENT_HOLD;
                key.value = KEY_CANCLE;
                break;
            case ASR_VOLUME_INC_EVENT://增大音量
                key.action = KEY_EVENT_CLICK;
                key.value = KEY_VOLUME_INC;
                break;
            case ASR_VOLUME_DEC_EVENT://减小音量
                key.action = KEY_EVENT_CLICK;
                key.value = KEY_VOLUME_DEC;
                break;
            case ASR_SONG_PREVIOUS_EVENT://播放上一首
                key.action = KEY_EVENT_CLICK;
                key.value = KEY_PREV;
                break;
            case ASR_SONG_NEXT_EVENT://播放下一首
                key.action = KEY_EVENT_CLICK;
                key.value = KEY_NEXT;
                break;
            }
            key.type = KEY_EVENT_USER;
            if (key.value == KEY_VOLUME_INC || key.value == KEY_VOLUME_DEC) {
                key_event_notify(KEY_EVENT_FROM_USER, &key);
            }
            jl_asr_reset();
        } else {
            if (!asr_rst) {
                asr_rst = timer_get_ms();
            } else {
                time = timer_get_ms();
                if (time - asr_rst >= 5 * 60 * 1000 || time - asr_rst < 0) {
                    jl_asr_reinit();
                    asr_rst = timer_get_ms();
                    printf("->reset jl_asr_reset\n");
                }
            }
        }
#endif // JLASR_EN

        //if(keyworld_start)
        {
            if (__this->send_pcm == SEND_PCM_START) {
                music_buf_play_set_stop();
                //websockets_free_lbuf_buf();
                music_buf_play_free_lbuf();
                websockets_send_pcm_buf_start();
                if (first_500ms_copy) {
#if ASR_FIRST_BEFORE_EN
                    if (!websockets_send_pcm_buf_push((u8 *)first_500ms_buf, first_500ms_len)) {
#else
                    if (!websockets_send_pcm_buf_push((u8 *)tempbuf, mic_len * 2)) {
#endif // ASR_FIRST_BEFORE_EN
//#if PCM_SAVE_TEST
//                        sdfile_onefile_save_test(first_500ms_buf, first_500ms_len, 0);//sd卡存储PCM
//#endif
                        first_500ms_copy = 0;
                        __this->send_pcm = SEND_PCM_DOING;
                    }
                } else {
                    if (!websockets_send_pcm_buf_push((u8 *)tempbuf, mic_len * 2)) {
                        __this->send_pcm = SEND_PCM_DOING;
                    }
                }
            } else if (__this->send_pcm == SEND_PCM_DOING) {
                if (websockets_send_pcm_buf_push((u8 *)tempbuf, mic_len * 2) == -2) { //发送过程中缓存满且掉线
                    goto reinit;
                }
//#if PCM_SAVE_TEST
//                sdfile_onefile_save_test(tempbuf, mic_len*2, 0);//sd卡存储PCM
//#endif
            } else if (__this->send_pcm == SEND_PCM_STOP) {
                websockets_send_pcm_buf_push((u8 *)tempbuf, mic_len * 2);
//#if PCM_SAVE_TEST
//                sdfile_onefile_save_test(tempbuf, mic_len*2, 1);//sd卡存储PCM
//#endif
                websockets_send_pcm_buf_end();
reinit:
                __this->send_pcm = SEND_PCM_INI;
                first_500ms_len = 0;
                first_500ms_copy = ASR_FIRST_BEFORE_EN;
            }
        }

        //buf:多通道数据
    }

__exit:
#if JLASR_EN
    jl_asr_free();
#endif // JLASR_EN
#if (ASR_USED_AVD == ASR_WEBRTC_AVD)
    if (vad_cbuf) {
        sys_vad_uninit(vad_cbuf);
    }
#endif
    __this->auth_flag = 0;
    __this->run_flag = 0;
}
int pcm_send_set_status(char start)
{
    if (start) {
        if (__this->send_pcm != SEND_PCM_START && __this->send_pcm != SEND_PCM_DOING) {
            keyworld_start = 1;
            __this->send_pcm = SEND_PCM_START;
        }
    } else if (__this->send_pcm == SEND_PCM_DOING || __this->send_pcm == SEND_PCM_START) {
        __this->send_pcm = SEND_PCM_STOP;
    }
}

static void enc_server_event_handler(void *priv, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
    case AUDIO_SERVER_EVENT_END:
        break;
    case AUDIO_SERVER_EVENT_SPEAK_START:
#if ASR_CONTINIU_EN
        if (keyworld_start) {
#else   //播放完当前对话才能继续
        if (keyworld_start && (__this->send_pcm == SEND_PCM_STOP || __this->send_pcm == SEND_PCM_INI)
            && music_buf_play_stop_staus() && music_play_stop_status()) {
#endif // ASR_CONTINIU_EN
            websockets_client_next_dialogue_init();
            __this->send_pcm = SEND_PCM_START;
            music_buf_play_set_stop();
            printf("--->SPEAK_START\n");
            websocket_client_thread_create(0);
        }
        pcm_save_start = 1;
        break;
    case AUDIO_SERVER_EVENT_SPEAK_STOP:
        pcm_save_start = 0;
        if (__this->send_pcm == SEND_PCM_DOING || __this->send_pcm == SEND_PCM_START) {
            __this->send_pcm = SEND_PCM_STOP;
            printf("--->SPEAK_STOP\n");
        }
        break;
    default:
        break;
    }
}

void sys_vad_event_handler(void *priv)
{
    if (priv) {
#if ASR_CONTINIU_EN
        if (keyworld_start) {
#else   //播放完当前对话才能继续
        if (keyworld_start && (__this->send_pcm == SEND_PCM_STOP || __this->send_pcm == SEND_PCM_INI)
            && music_buf_play_stop_staus() && music_play_stop_status()) {
#endif // ASR_CONTINIU_EN
            __this->send_pcm = SEND_PCM_START;
            music_buf_play_set_stop();
            printf("--->SPEAK_START\n");
            websocket_client_thread_create(0);
        }
        pcm_save_start = 1;
    } else {
        pcm_save_start = 0;
        if (__this->send_pcm == SEND_PCM_DOING || __this->send_pcm == SEND_PCM_START) {
            __this->send_pcm = SEND_PCM_STOP;
            printf("--->SPEAK_STOP\n");
        }
    }
}
static int aisp_vfs_fwrite(void *file, void *data, u32 len)
{
    cbuffer_t *cbuf = (cbuffer_t *)file;
    u32 wlen = cbuf_write(cbuf, data, len);
    if (wlen != len) {
        os_sem_set(&__this->sem, 0);
        os_sem_post(&__this->sem);
        os_time_dly(2);
        wlen = cbuf_write(cbuf, data, len);
        if (wlen != len) {
            cbuf_clear(&__this->mic_cbuf);
            os_sem_post(&__this->sem);
            puts("busy!\n");
        }
    }
    if (file == (void *)&__this->mic_cbuf) {
        os_sem_set(&__this->sem, 0);
        os_sem_post(&__this->sem);
    }

    return len;
}

static int aisp_vfs_fclose(void *file)
{
    return 0;
}

static const struct audio_vfs_ops aisp_vfs_ops = {
    .fwrite = aisp_vfs_fwrite,
    .fclose = aisp_vfs_fclose,
};



int aisp_open(u16 sample_rate)
{
    __this->exit_flag = 0;
    __this->mic_enc = server_open("audio_server", "enc");
    server_register_event_handler(__this->mic_enc, NULL, enc_server_event_handler);
    cbuf_init(&__this->mic_cbuf, __this->mic_buf, sizeof(__this->mic_buf));
    os_sem_create(&__this->sem, 0);
    __this->sample_rate = sample_rate;
    printf("--->sample_rate = %d \n", __this->sample_rate);
    return thread_fork("aisp", 5, 3840, 0, &__this->pid, aisp_task, __this);
}

void aisp_suspend(void)
{
    union audio_req req = {0};

    if (!__this->auth_flag || !__this->run_flag) {
        return;
    }

    __this->run_flag = 0;

    req.enc.cmd = AUDIO_ENC_STOP;
    server_request(__this->mic_enc, AUDIO_REQ_ENC, &req);
    cbuf_clear(&__this->mic_cbuf);
}

void aisp_resume(void)
{
    union audio_req req = {0};

    if (!__this->auth_flag || __this->run_flag) {
        return;
    }
    __this->run_flag = 1;
    os_sem_set(&__this->sem, 0);
    os_sem_post(&__this->sem);

    req.enc.cmd = AUDIO_ENC_OPEN;
#if AISP_DUAL_MIC_ALGORITHM
    req.enc.channel = 2;
    req.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL) | BIT(CONFIG_AISP_MIC1_ADC_CHANNEL);
#else
    req.enc.channel = 1;
    req.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL);
#endif

    req.enc.frame_size = ONCE_SR_POINTS * 2 * req.enc.channel;
    req.enc.format = "pcm";
    req.enc.volume = CONFIG_AISP_MIC_ADC_GAIN;
    req.enc.sample_rate = __this->sample_rate;
    req.enc.sample_source = "mic";
    req.enc.vfs_ops = &aisp_vfs_ops;
    req.enc.output_buf_len = req.enc.frame_size * 3;
    req.enc.file = (FILE *)&__this->mic_cbuf;

#ifdef CONFIG_AEC_ENC_ENABLE
    struct aec_s_attr aec_param = {0};
    aec_param.EnableBit = AEC_MODE_ADVANCE;
    aec_param.output_way = 0;    //1:使用硬件回采 0:使用软件回采
    req.enc.aec_attr = &aec_param;
    req.enc.aec_enable = 1; //默认不开

    extern void get_cfg_file_aec_config(struct aec_s_attr * aec_param);
    get_cfg_file_aec_config(&aec_param);

    if (aec_param.EnableBit == 0) {
        req.enc.aec_enable = 0;
        req.enc.aec_attr = NULL;
    }
    if (aec_param.EnableBit != AEC_MODE_ADVANCE) {
        aec_param.output_way = 0;
    }
#if AEC_ENC_HARDWAIR_ENABLE
#if defined CONFIG_ALL_ADC_CHANNEL_OPEN_ENABLE && \
        defined CONFIG_AISP_MIC2_AEC_ADC_CHANNEL && \
        defined CONFIG_AISP_MIC2_AEC_ADC_CHANNEL
    if (req.enc.aec_enable) {
        aec_param.output_way = 1;    //1:使用硬件回采 0:使用软件回采
        req.enc.channel_bit_map |= BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);    //配置回采硬件通道
        if (CONFIG_AISP_MIC2_AEC_ADC_CHANNEL < CONFIG_PHONE_CALL_ADC_CHANNEL) {
            req.enc.ch_data_exchange = 1;    //如果回采通道使用的硬件channel比MIC通道使用的硬件channel靠前的话处理数据时需要交换一下顺序
        }
    }
#endif

    if (req.enc.sample_rate == 16000) {
        aec_param.wideband = 1;
        aec_param.hw_delay_offset = 50;
    } else {
        aec_param.wideband = 0;
        aec_param.hw_delay_offset = 75;
    }
#endif // AEC_ENC_HARDWAIR_ENABLE
#endif // CONFIG_AEC_ENC_ENABLE

#if (ASR_USED_AVD == ASR_JL_AVD)
    if (req.enc.sample_rate == 8000 || req.enc.sample_rate == 16000) {
        req.enc.use_vad = 1;            //打开VAD断句功能
        req.enc.vad_auto_refresh = 1;   //VAD自动刷新
    }
    if (req.enc.use_vad == 1) {
        req.enc.vad_start_threshold = 0;    //ms
        req.enc.vad_stop_threshold  = 500;    //ms
    }
#endif

    server_request(__this->mic_enc, AUDIO_REQ_ENC, &req);


    memset(&req, 0, sizeof(req));
    req.enc.channel_bit_map = BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
    req.enc.cmd = AUDIO_ENC_SET_VOLUME;
    req.enc.volume = CONFIG_AISP_AEC_ADC_GAIN;
    server_request(__this->mic_enc, AUDIO_REQ_ENC, &req);


#ifdef CONFIG_AEC_ENC_ENABLE
//    if (aec_param.output_way) {
//#ifdef CONFIG_ALL_ADC_CHANNEL_OPEN_ENABLE
//        extern void adc_multiplex_set_gain(const char *source, u8 channel_bit_map, u8 gain);
//        adc_multiplex_set_gain("mic", BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL), CONFIG_AISP_AEC_ADC_GAIN * 2);
//#endif
//    }

//    memset(&req, 0, sizeof(req));

//#if AISP_DUAL_MIC_ALGORITHM
//    req.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL) | BIT(CONFIG_AISP_MIC1_ADC_CHANNEL);
//#else
//    req.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL);
//#endif
//    req.enc.cmd = AUDIO_ENC_SET_VOLUME;
//    req.enc.volume = CONFIG_AISP_AEC_ADC_GAIN;
//    server_request(__this->mic_enc, AUDIO_REQ_ENC, &req);


//回采PCM音频
//#ifdef CONFIG_AEC_ENC_ENABLE
//    req.enc.cmd = AUDIO_ENC_OPEN;
//    req.enc.channel = 1;
//    req.enc.volume = CONFIG_AISP_LINEIN_ADC_GAIN;
//    req.enc.sample_rate = __this->sample_rate;
//    req.enc.format = "pcm";
//    req.enc.frame_size = ONCE_SR_POINTS * 2;
//#ifdef CONFIG_AISP_DIFFER_MIC_REPLACE_LINEIN_OTHER
//    req.enc.sample_source = "mic";    //使用数字MIC且用差分MIC做回采时需要打开这个
//#else
//    req.enc.sample_source = "linein";
//#endif
//    req.enc.vfs_ops = &aisp_vfs_ops;
//    req.enc.output_buf_len = req.enc.frame_size * 3;
//    req.enc.channel_bit_map = BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
//    req.enc.file = (FILE *)&__this->linein_cbuf;
//    server_request(__this->linein_enc, AUDIO_REQ_ENC, &req);

//#if AEC_ENC_HARDWAIR_ENABLE
//    memset(&req, 0, sizeof(req));
//    req.enc.channel_bit_map = BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
//    req.enc.cmd = AUDIO_ENC_SET_VOLUME;
//    req.enc.volume = CONFIG_AISP_AEC_ADC_GAIN;
//    server_request(__this->mic_enc, AUDIO_REQ_ENC, &req);
//#endif

#endif
}
int aisp_mic_gain_suspend(void)
{
    int pa_volume = 0;
    sys_volume_read(&pa_volume);//获取音量
    aisp_mic_gain_set(pa_volume);//设置回采和mic的增益
    return 0;
}

int aisp_mic_gain_resum(void)
{
    union audio_req req_vol = {0};
    if (__this->mic_enc) {
        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL);
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = CONFIG_AISP_MIC_ADC_GAIN;
        server_request(__this->mic_enc, AUDIO_REQ_ENC, &req_vol);
    }
    return 0;
}

int aisp_mic_gain_set(unsigned char volume)
{
    union audio_req req_vol = {0};

    if (__this->mic_enc) {
        aisp_aec_gain = AEC_GAIN(volume);
        aisp_mic0_gain = MIC0_GAIN(volume);

        aisp_aec_gain = aisp_aec_gain >= 95 ? 95 : (aisp_aec_gain < 30 ? 30 : aisp_aec_gain);
        aisp_mic0_gain = aisp_mic0_gain >= 95 ? 95 : (aisp_mic0_gain < 10 ? 10 : aisp_mic0_gain);
        printf("-> volume = %d, aec = %d, mic0 = %d, mic1 = %d \n", volume, aisp_aec_gain, aisp_mic0_gain, aisp_mic1_gain);

        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL);
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = aisp_mic0_gain;//CONFIG_AISP_MIC_ADC_GAIN;
        server_request(__this->mic_enc, AUDIO_REQ_ENC, &req_vol);

        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = aisp_aec_gain;
        server_request(__this->mic_enc, AUDIO_REQ_ENC, &req_vol);
    }
    return aisp_aec_gain;
}
int aisp_all_mic_gain_set(int volume, int aec_gian, int mic0_gian, int mic1_gian)
{
    union audio_req req_vol = {0};

    if (__this->mic_enc && aec_gian) {
        aec_gian = aec_gian >= 95 ? 95 : (aec_gian < 30 ? 30 : aec_gian);
        mic0_gian = mic0_gian >= 95 ? 95 : (mic0_gian < 10 ? 10 : mic0_gian);
        printf("-> volume = %d, aec_gain = %d, mic0_gian = %d, mic1_gian = %d\n", volume, aec_gian, mic0_gian, mic1_gian);

        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL);
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = mic0_gian;//CONFIG_AISP_MIC_ADC_GAIN;
        server_request(__this->mic_enc, AUDIO_REQ_ENC, &req_vol);

        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = aec_gian;
        server_request(__this->mic_enc, AUDIO_REQ_ENC, &req_vol);
    }
    return aec_gian;
}

void aisp_close(void)
{
    if (__this->exit_flag) {
        return;
    }

    aisp_suspend();

    __this->exit_flag = 1;

    os_sem_post(&__this->sem);

    if (__this->mic_enc) {
        server_close(__this->mic_enc);
        __this->mic_enc = NULL;
    }

    thread_kill(&__this->pid, KILL_WAIT);
}

#endif

