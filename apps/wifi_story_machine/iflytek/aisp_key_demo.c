#include "server/audio_server.h"
#include "server/server_core.h"
#include "generic/circular_buf.h"
#include "os/os_api.h"
#include "event.h"
#include "app_config.h"
#include "event/key_event.h"
/* #include "jlsp_kws_aec.h" */

#ifdef CONFG_NO_KW_ENABLE

char aisp_mic_channel_set = 0;
extern int keyworld_start;
extern const int speech_energy_min;

const int CONFIG_KWS_RAM_USE_ENABLE = 1;
extern int websockets_wait_recv_end_msg(void);
extern char keyworld_wifi_enter_congfig;
extern void aisp_resume(void);
extern int websockets_send_500ms_pcm_buf_push(char *buf, int len, int clear);
extern int websockets_send_pcm_buf_init(void);
extern int websocket_client_thread_create_new(void *priv, char key_vad);

static u8 ifly_send_pcm = 0;
enum send_pcm_type {
    SEND_PCM_INI = 0,
    SEND_PCM_START,
    SEND_PCM_DOING,
    SEND_PCM_STOP,
    SEND_PCM_PAUSE,
};

#define ONCE_SR_POINTS  320

#define AISP_BUF_SIZE   (ONCE_SR_POINTS * 2 * 16)   //跑不过来时适当加大倍数
#define MIC_SR_LEN      (ONCE_SR_POINTS * 2)

#ifdef CONFIG_AEC_ENC_ENABLE

/*aec module enable bit define*/
#define AEC_EN              BIT(0)
#define NLP_EN              BIT(1)
#define ANS_EN              BIT(2)

#define AEC_MODE_ADVANCE    (AEC_EN | NLP_EN | ANS_EN)
#define AEC_MODE_REDUCE     (NLP_EN | ANS_EN)
#define AEC_MODE_SIMPLEX    (ANS_EN)

#endif

static struct {
    int pid;
    u16 sample_rate;
    u8 volatile exit_flag;
    u8 vad_enbable;
    u8 vad_reset;
    u8 key_strat;
    u8 vad_lock;
    int volatile run_flag;
    OS_SEM sem;
    s16 mic_buf[AISP_BUF_SIZE * 2];
    void *mic_enc;
    cbuffer_t mic_cbuf;
} aisp_server;

#define __this (&aisp_server)

static void vad_event_handler(void *priv);

void  __attribute__((weak)) aisp_timeout_callback(void)
{
}
int  __attribute__((weak)) aisp_wake_callback(int status)
{
    return 0;
}
int  __attribute__((weak)) aisp_wake_no_play_notice(void)
{
    return 0;
}
int  __attribute__((weak)) aisp_wake_world_callback(int index)//定制唤醒词更改，返回值：1唤醒词， 2配网模式
{
    return 0;
}
int  __attribute__((weak)) aisp_record_callbak(int start)//start 1开始录音， 0结束录音
{
    return 0;//返回0开唤醒词，1关闭唤醒词
}
void  __attribute__((weak)) sys_net_channel_info_clear(void)
{
}

static float calculate_frequency_zero_crossing(short* audio_data, int sample_count, int sample_rate) //过零检测法计算频率
{
    int zero_crossings = 0;
    for (int i = 1; i < sample_count; i++) {
        if ((audio_data[i - 1] >= 0 && audio_data[i] < 0) ||
            (audio_data[i - 1] < 0 && audio_data[i] >= 0)) {
            zero_crossings++;
        }
    }
    float frequency = (zero_crossings / 2.0f) * sample_rate / sample_count;
    return frequency;
}

float detect_frequency(short* audio_data, int sample_count, int sample_rate) //频率检测函数
{
    return calculate_frequency_zero_crossing(audio_data, sample_count, sample_rate);
}

static void aisp_task(void *priv)
{
    u32 mic_len, linein_len;
    int ret;
    short near_data_buf[ONCE_SR_POINTS];
    void *jlvad = NULL;
    char vad_start = 0;
    char jlvad_reset = 0;
    unsigned int vad_start_time = 0;
    int pd_test_time = 0;
    int freq = 0;
#define AUDIO_SIN_FREQ      1000 //播放的sin正弦音频频率
#define FABS_SIN_FREQ       10 //频率误差
#define CHECK_FREQ_TIME     500  //检测播放的sin正弦音频频率连续时间（作为判断正常拾音条件）

    jlvad = vad_init(__this->sample_rate, 300, 0);

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

        if ((cbuf_get_data_size(&__this->mic_cbuf) < ONCE_SR_POINTS * 2)) {
            os_sem_pend(&__this->sem, 0);
            continue;
        }

        mic_len = cbuf_read(&__this->mic_cbuf, near_data_buf, ONCE_SR_POINTS * 2);
        if (!mic_len) {
            continue;
        }

        int pd_test = is_production_test_enter(0) && (is_production_test_mic1_enter() | is_production_test_mic2_enter());
        if (pd_test) {
            if (!pd_test_time) {
                pd_test_time = timer_get_ms();
            }
            freq = (int)detect_frequency(near_data_buf, ONCE_SR_POINTS, __this->sample_rate);
            if (fabs(freq - AUDIO_SIN_FREQ) > FABS_SIN_FREQ) {
                pd_test_time = timer_get_ms();
            }
            if ((timer_get_ms() - pd_test_time) > CHECK_FREQ_TIME) {
                is_production_test_enter(1);
                pd_test_time = 0;
                printf("-------- 厂测实测频率：%d Hz ---------", freq);
            }
        } else if (pd_test_time) {
            pd_test_time = 0;
        }

        if (!__this->vad_enbable && !__this->key_strat && (ifly_send_pcm == SEND_PCM_INI || ifly_send_pcm == SEND_PCM_PAUSE)) {
            continue;
        }

        if (ifly_send_pcm == SEND_PCM_START) {
            if (!sys_connect_net_success()) {
                printf("err net connect\n\n");
                goto reinit;
            }
            music_buf_play_free_lbuf();
            if (websockets_send_pcm_buf_start()) {
                if (websockets_send_pcm_buf_start()) {
                    printf("err pcm_buf_start\n\n");
                    continue;
                }
            }
            if (!websockets_send_pcm_buf_push((u8 *)near_data_buf, mic_len)) {
                ifly_send_pcm = SEND_PCM_DOING;
            }
        } else if (ifly_send_pcm == SEND_PCM_DOING) {
            if (websockets_send_pcm_buf_push((u8 *)near_data_buf, mic_len) == -2) { //发送过程中缓存满且掉线
                goto reinit;
            }
        } else if (ifly_send_pcm == SEND_PCM_STOP) {
            websockets_send_pcm_buf_push((u8 *)near_data_buf, mic_len);
            websockets_send_pcm_buf_end();
reinit:
            ifly_send_pcm = SEND_PCM_INI;
        } else {
            websockets_send_500ms_pcm_buf_push(near_data_buf, mic_len, 0);//
        }

        int val = 0;
        if (jlvad) {
            if (__this->vad_reset) {
                __this->vad_reset = 0;
                vad_reset(jlvad);
                jlvad_reset = 0;
                vad_start = 0;
            } else {
                val = vad_main(jlvad, (char*)near_data_buf, mic_len);// 0 1 2
                if (val == 2 && !jlvad_reset) {
                    vad_reset(jlvad);
                    jlvad_reset = 1;
                } else if ((val == 0 || val == 1) && jlvad_reset) {
                    jlvad_reset = 0;
                }
                if (val == 1 && !vad_start) {
                    vad_start = 1;
                    vad_start_time = timer_get_ms();
                    //printf("->vad_start_time = %d\n",vad_start_time);
                    sys_timeout_add_to_task("sys_timer", AUDIO_SERVER_EVENT_SPEAK_START, vad_event_handler, 10);
                } else if (vad_start && val != 1) {
                    if ((timer_get_ms() - vad_start_time) > 500) {
                        printf("->vad_start_time = %d,ms\n", timer_get_ms() - vad_start_time);
                        printf("->vad_note\n");
                        vad_start = 0;
                        sys_timeout_add_to_task("sys_timer", AUDIO_SERVER_EVENT_SPEAK_STOP, vad_event_handler, 10);
                    }
                }
            }
        }
    }
__exit:
    __this->run_flag = 0;
}

static void enc_server_event_handler(void *priv, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
    case AUDIO_SERVER_EVENT_END:
        break;
#if 1//(defined CONFIG_SXY_QYAI_ENABLE)
    case AUDIO_SERVER_EVENT_SPEAK_START:       /*!< VAD检测到开始说话 */
        if (!__this->vad_enbable) {
            break;
        }
        if ((ifly_send_pcm == SEND_PCM_STOP || ifly_send_pcm == SEND_PCM_INI)
            && music_buf_play_stop_staus() && music_play_stop_status() && !websockets_wait_recv_end_msg()) {
            music_buf_play_set_stop();
            aisp_record_callbak(1);
            websockets_send_pcm_buf_init();
            websocket_client_thread_create(0);
            music_buf_play_accept(1);
            printf("--->SPEAK_START\n");
            ifly_send_pcm = SEND_PCM_START;
        }
        break;
    case AUDIO_SERVER_EVENT_SPEAK_STOP:        /*!< VAD检测到停止说话 */
        if (ifly_send_pcm == SEND_PCM_DOING || ifly_send_pcm == SEND_PCM_START) {
            printf("--->SPEAK_STOP = %d \n", ifly_send_pcm);
            ifly_send_pcm = SEND_PCM_STOP;
            aisp_record_callbak(0);
        }
        break;
#endif
    default:
        break;
    }
}
static void vad_event_handler(void *priv)
{
    switch ((int)priv) {
    case AUDIO_SERVER_EVENT_ERR:
    case AUDIO_SERVER_EVENT_END:
        break;
#if 1//(defined CONFIG_SXY_QYAI_ENABLE)
    case AUDIO_SERVER_EVENT_SPEAK_START:       /*!< VAD检测到开始说话 */
        printf("SPEAK_START：%d\n", __this->vad_enbable);
        if (!__this->vad_enbable) {
            break;
        }
        if ((ifly_send_pcm == SEND_PCM_STOP || ifly_send_pcm == SEND_PCM_INI)
            && music_buf_play_stop_staus() && music_play_stop_status() && !websockets_wait_recv_end_msg()) {
            music_buf_play_set_stop();
            aisp_record_callbak(1);
            websockets_send_pcm_buf_init();
            websocket_client_thread_create(0);
            music_buf_play_accept(1);
            printf("--->SPEAK_START\n");
            ifly_send_pcm = SEND_PCM_START;
        }
        break;
    case AUDIO_SERVER_EVENT_SPEAK_STOP:        /*!< VAD检测到停止说话 */
        if (__this->vad_enbable) {
            if (ifly_send_pcm == SEND_PCM_DOING || ifly_send_pcm == SEND_PCM_START) {
                printf("--->SPEAK_STOP = %d \n", ifly_send_pcm);
                ifly_send_pcm = SEND_PCM_STOP;
                aisp_record_callbak(0);
            }
        }
        break;
#endif
    default:
        break;
    }
}
static int aisp_vfs_fwrite(void *file, void *data, u32 len)
{
    cbuffer_t *cbuf = (cbuffer_t *)file;

    u32 wlen = cbuf_write(cbuf, data, len);
    //if (wlen != len) {
    //cbuf_clear(&__this->mic_cbuf);
    //puts("busy!\n");
    //}
    os_sem_set(&__this->sem, 0);
    os_sem_post(&__this->sem);

    return len;
}
void pcm_buf_clear(void)
{
    if (__this->run_flag) {
        cbuf_clear(&__this->mic_cbuf);
    }
    if (ifly_send_pcm != SEND_PCM_PAUSE) {
        ifly_send_pcm = SEND_PCM_INI;
    }
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

    return thread_fork("aisp", 3, 820, 0, &__this->pid, aisp_task, __this);
}

void aisp_suspend(void)
{
    union audio_req req = {0};

    if (!__this->run_flag) {
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

    if (__this->run_flag) {
        return;
    }
    __this->run_flag = 1;
    os_sem_set(&__this->sem, 0);
    os_sem_post(&__this->sem);

    req.enc.cmd = AUDIO_ENC_OPEN;
    req.enc.channel = 1;
    if (aisp_mic_channel_set) {
        req.enc.channel_bit_map = BIT(aisp_mic_channel_set);
    } else {
        req.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL);
    }
    req.enc.frame_size = ONCE_SR_POINTS * 2 * req.enc.channel;
    req.enc.volume = CONFIG_AISP_MIC_ADC_GAIN;
    req.enc.sample_rate = __this->sample_rate;
    req.enc.format = "pcm";
#if CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_PLNK0
    req.enc.sample_source = "plnk0";
#elif CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_PLNK1
    req.enc.sample_source = "plnk1";
#elif CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_IIS0
    req.enc.sample_source = "iis0";
#elif CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_IIS1
    req.enc.sample_source = "iis1";
#else
    req.enc.sample_source = "mic";
#endif
    req.enc.vfs_ops = &aisp_vfs_ops;
    req.enc.output_buf_len = req.enc.frame_size * 3;
    req.enc.file = (FILE *)&__this->mic_cbuf;

#ifdef CONFIG_AEC_ENC_ENABLE
    struct aec_s_attr aec_param = {0};
    aec_param.EnableBit = AEC_MODE_ADVANCE;
    aec_param.output_way = 0;    //1:使用硬件回采 0:使用软件回采
    req.enc.aec_attr = &aec_param;
    req.enc.aec_enable = 1; //默认不开
#endif

#if 0//(defined CONFIG_SXY_QYAI_ENABLE)
    if (req.enc.channel == 1 && !strcmp(req.enc.sample_source, "mic") && (req.enc.sample_rate == 8000 || req.enc.sample_rate == 16000)) {
        req.enc.use_vad = 1; //打开VAD断句功能
        req.enc.dns_enable = 0; //打开降噪功能
        req.enc.vad_auto_refresh = 1; //VAD自动刷新
        req.enc.vad_start_threshold = 300;
        req.enc.vad_stop_threshold = 0;
        printf("->vad en\n");
    }
#endif
    server_request(__this->mic_enc, AUDIO_REQ_ENC, &req);
    printf("-------------- aisp open ---------------\n");
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
    printf("-------------- aisp close ---------------\n");
}
void aisp_all_pause(char stop)
{
    int to = 100;
    if (stop) {
        ifly_send_pcm = SEND_PCM_PAUSE;
        __this->vad_enbable = 0;
    } else {
        ifly_send_pcm = SEND_PCM_INI;
        __this->vad_enbable = 0;//1:采用唤醒词则开启1，按键则0
    }
    union audio_req req = {0};
    if (__this->mic_enc) {
        req.enc.cmd = AUDIO_ENC_GET_STATUS;
        server_request(__this->mic_enc, AUDIO_REQ_ENC, &req);
        int status = req.enc.status;
        memset(&req, 0, sizeof(req));
        if (status == AUDIO_ENC_START && stop) {
            req.enc.cmd = AUDIO_ENC_PAUSE;
            server_request(__this->mic_enc, AUDIO_REQ_ENC, &req);
            printf("-> AUDIO_ENC_PAUSE \n");
        } else if (status == AUDIO_ENC_PAUSE && stop == 0) {
            req.enc.cmd = AUDIO_ENC_START;
            server_request(__this->mic_enc, AUDIO_REQ_ENC, &req);
            printf("-> AUDIO_ENC_START \n");
        }
    }
    music_buf_play_set_stop();
    websockets_free_lbuf_buf();
    music_buf_play_free_lbuf();
    websockets_close_request(1);
    music_buf_play_stop_waite();//等待关闭完成再播放提示音
    net_music_num_clear();
    pcm_buf_clear();
    while (!music_buf_play_stop_staus() && --to) {
        os_time_dly(1);
    }
}
void aisp_clear(char stop_net_music)
{
    __this->vad_enbable = 0;
    ifly_send_pcm = SEND_PCM_STOP;
    music_buf_play_set_stop();
    if (stop_net_music) {
        net_music_play_set_stop();
    }
    websockets_free_lbuf_buf();
    music_buf_play_free_lbuf();
    websockets_close_request(1);
    websockets_dialogue_timeout_del();
    music_buf_play_stop_waite();//等待关闭完成再播放提示音
    net_music_num_clear();
    pcm_buf_clear();
    __this->vad_enbable = 0;//1:采用唤醒词则开启1，按键则0
}
void aisp_timeout_exit(void)
{
    keyworld_start = 0;
    __this->vad_enbable = 0;
    __this->key_strat = 0;
    printf("->aisp_timeout_exit\n");
}
int sys_vad_lock(char lock)
{
    __this->vad_lock = lock;
    return __this->vad_lock;
}
void sys_vad_clear(char enable)
{
    if (__this->vad_lock) {
        return;
    }
    pcm_buf_clear();
    if (__this->key_strat) {
        __this->vad_enbable = enable;
    }
    __this->vad_reset = true;
    printf("vad_enbable = %d\n", __this->vad_enbable);
}
void aisp_wake_no_notic(void)
{
}
static void key_vad_pcm_send_set(char start, char noice_note, int vad_end_enable)
{
    if (start) {
        if ((vad_end_enable) && (ifly_send_pcm == SEND_PCM_DOING || ifly_send_pcm == SEND_PCM_START)) {
            puts("kay vad already start\n");
            return;
        }
        puts("--->KAY VAD START\n");
        //通知app挂起
        auto_sleep_check_clear();
        music_buf_play_accept(0);
        websockets_free_lbuf_buf();
        websockets_close_request(1);
        websockets_dialogue_timeout_del();

        music_play_stop_all();
        music_buf_play_stop_all();
        music_buf_play_set_stop();
        music_buf_play_free_lbuf();
        music_buf_play_stop_waite();//等待关闭完成再播放提示音
        net_music_num_clear();
        //led_eya_wake(1);//LED眼睛唤醒显示
        if (noice_note) {
            //aisp_mic_gain_suspend();
            if (!music_play_hello()) {
                music_play_waite();
            }
            //aisp_mic_gain_resum();
        } else {
            if (!music_play_res_file("Recording.mp3")) {
                music_play_waite();
            }
        }
        music_buf_play_set_stop();//停止TTS使用PIPE BUF
        pcm_buf_clear();
        websockets_send_pcm_buf_init();
        websockets_send_500ms_pcm_buf_push(NULL, 0, 1);
        //websocket_client_thread_create(1);
        if (vad_end_enable) {
            __this->vad_enbable = 1;
        } else {
            __this->vad_enbable = 0;
        }
        __this->key_strat = 1;
        keyworld_start = 1;
        if (vad_end_enable) {//使能开启VAD结束，则使用vad自动检测对话
            websocket_client_thread_create_new(1, 0);
        } else {//不使能vad自动结束，则开启按键长按对话
            ifly_send_pcm = SEND_PCM_START;
            websocket_client_thread_create_new(1, 1);
        }
        music_buf_play_accept(1);
    } else {
        if (ifly_send_pcm == SEND_PCM_DOING || ifly_send_pcm == SEND_PCM_START) {
            ifly_send_pcm = SEND_PCM_STOP;
        }
        __this->vad_enbable = 0;
        __this->key_strat = 0;
        keyworld_start = 0;
        puts("--->KAY VAD STOP\n");
    }
}
void key_vad_pcm_send_set_status(char start, char noice_note)
{
    key_vad_pcm_send_set(start, noice_note, 0);
}
void key_vad_start_vad_end_auto(char start, char noice_note)
{
    key_vad_pcm_send_set(start, noice_note, 1);
}
void aisp_wake(char index)//0:小飞小飞，8：配网模式
{
    if (index == 0) {
        auto_sleep_check_clear();
        if (sys_connect_net_success() && !keyworld_wifi_enter_congfig) {
            if (ifly_send_pcm != SEND_PCM_PAUSE) {
                key_vad_start_vad_end_auto(1, 1);
            }
        } else if (keyworld_wifi_enter_congfig) {
            if (!aisp_wake_callback(-1)) {
                if (keyworld_wifi_enter_congfig == 3) { //正在连接WiFi
                    music_play_res_file("WifiConting.mp3");
                } else { //进入配网
                    music_play_res_file("WifiNote.mp3");
                }
            }
        } else { //网络异常
            if (!aisp_wake_callback(-1)) {
                music_play_res_file("WifiConErr.mp3");
            }
        }
    } else if (index == 8) {
        auto_sleep_check_clear();
        keyworld_wifi_enter_congfig = 1;
        __this->vad_enbable = 0;
        ifly_send_pcm = SEND_PCM_STOP;

        websockets_client_dialogue_exit();
        websockets_close_request(1);
        websockets_dialogue_timeout_del();

        music_buf_play_stop_all();
        music_buf_play_set_stop();
        net_music_play_set_stop();
        websockets_free_lbuf_buf();
        music_buf_play_free_lbuf();

        music_buf_play_stop_waite();//等待关闭完成再播放提示音
        net_music_num_clear();
        pcm_buf_clear();
        if (!aisp_wake_callback(8)) {
            music_play_res_file("WifiNote.mp3");
        }
        if (!is_in_config_network_state()) {
            config_network_start();
        }
#if (defined TCFG_LED_STATUES_VBAT_NET_EN && TCFG_LED_STATUES_VBAT_NET_EN == 1)
        extern void led_status_set(int status);//0初始化 1网络异常-快闪  2电池<20%-快闪  3网络正常和电池>=20%-常亮
        led_status_set(1);
#endif
    }
}
int aisp_mic_gain_suspend(void)
{
    return 0;
}
int aisp_mic_gain_resum(void)
{
    return 0;
}
int aisp_mic_gain_set(unsigned char volume)
{
    return 0;
}
int aisp_all_mic_gain_set(int volume, int aec_gian, int mic0_gian, int mic1_gian)
{
    return 0;
}
void aisp_app_resum(void)
{
}
void aisp_app_suspend(void)
{
}
void user_aiui_connected_init(void)
{
}
void user_aiui_discon_uninit(void)
{
}

#endif

