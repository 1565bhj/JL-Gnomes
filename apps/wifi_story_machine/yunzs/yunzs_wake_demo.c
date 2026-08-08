#include "server/audio_server.h"
#include "app_config.h"
#include "app_core.h"
#include <time.h>
#include "asm/ladc.h"
#include "system/timer.h"
#include "event/key_event.h"
#include "uni_ssp.h"
#include "uni_record.h"
#include "uni_kws.h"
#include "uni_record.h"

//云知声
#if (defined CONFIG_ASR_ALGORITHM) && (CONFIG_ASR_ALGORITHM == KWS_YUNZS_ALGORITHM)

char aisp_mic_channel_set = 0;

//#define ASR_WORDS_CHANGE //唤醒词更改：唤醒词、配网模式
#ifndef AUDIO_RECORD_MIC_COUNT
#error "error in no define AUDIO_RECORD_MIC_COUNT"
#elif (AUDIO_RECORD_MIC_COUNT == 2)
#undef AUDIO_RECORD_MIC_COUNT
#define AUDIO_RECORD_MIC_COUNT 1
#endif

#define AUDIO_RECORD_ONCE_SR_POINTS     256 //一次采样点
#define AUDIO_ONCE_SR_POINTS            (AUDIO_RECORD_ONCE_SR_POINTS * (AUDIO_RECORD_MIC_COUNT + 1)) //一次采样缓冲区点数
#define AUDIO_ONCE_SR_POINTS_SIZE       (AUDIO_ONCE_SR_POINTS * 2)  //一次采样缓冲区的缓冲的字节数

#define AUDIO_RECORD_BUF_SIZE           (128 * AUDIO_ONCE_SR_POINTS_SIZE)   //整个MIC的缓冲区大小字节
#define AEC_FRAME_POINTS                (AUDIO_RECORD_ONCE_SR_POINTS)       //AEC回采的采样点
#define AEC_FRAME_SIZE                  (AUDIO_RECORD_ONCE_SR_POINTS * 2)   //AEC回采的字节大小长度


//#define CHANNEL_NUM       (AUDIO_RECORD_MIC_COUNT+1) //(2) //1mic+ 1aec
//#define FRAME_SAMPLES     (512)
//#define UNI_BUFF_SIZE       (FRAME_SAMPLES*CHANNEL_NUM)

//#define AEC_GAIN(v)     (int)(CONFIG_AISP_AEC_ADC_GAIN)
//#define MIC0_GAIN(v)    (int)(CONFIG_AISP_MIC_ADC_GAIN)   //固定CONFIG_AISP_MIC_ADC_GAIN-80
//#define MIC1_GAIN(v)    (MIC0_GAIN(v))

#define MIC_MAX_GAIN 95

void aisp_wake(char index);
int aisp_aec_gain_set(unsigned char volume);
SEC_USED(.sram) ALIGNE(4) static cbuffer_t gs_mic_cbuf;

s16 mic_buf[AUDIO_RECORD_BUF_SIZE];
OS_SEM read_available_sem;
OS_SEM asr_sem;

static char aisp_open_init = 0;
static char asr_open_init = 0;
extern bool s_start_send;

extern int aisp_aec_gain;
extern int aisp_mic0_gain;
extern int aisp_mic1_gain;

static uint8_t run_flag = 0;
static struct server *server_handle = NULL;
static int ifly_sample_rate;
void aisp_clear(char stop_net_music);
static volatile char ifly_send_pcm;
int g_ivwThreadID;

int bt_music_play_get_pause(void);

extern int keyword_start;
extern int keyword_wifi_enter_congfig;

static volatile char sys_vad_active = 0;//有效检测语音
void led_eya_wake(char start);
void led_eya_sleep(void);
int sys_vad_clear(int enable);
void sys_vad_lock(int lock);

int aisp_all_mic_gain_set(int volume, int aec_gian, int mic0_gian, int mic1_gian);
int aisp_mic_gain_set(unsigned char volume);


SEC_USED(.sram) ALIGNE(4) static short audio_record_buf_read[AUDIO_ONCE_SR_POINTS];
SEC_USED(.sram) ALIGNE(4) static short asr_buf_feed_to_engine[AUDIO_ONCE_SR_POINTS];

SEC_USED(.sram) ALIGNE(4) static short g_aecOutput[AEC_FRAME_POINTS] = {0};

cbuffer_t asr_buf = {0};

static u8 last_key_supspend = 0;
OS_SEM aiui_sem;

#ifdef WIFI_PCM_STREAN_SOCKET_ENABLE
static short wifi_stream_buf[AUDIO_ONCE_SR_POINTS * 2];//mic2 + mic2 + mic3 + aec
#else
static short *aec_out_buf = NULL;
#endif
enum send_pcm_type {
    SEND_PCM_INI = 0,
    SEND_PCM_START,
    SEND_PCM_DOING,
    SEND_PCM_STOP,
    SEND_PCM_PAUSE,
};

extern void config_network_start(void);
extern int wifi_sta_is_connected(void);
extern int wifi_pcm_stream_socket_valid(void);
extern void wifi_pcm_stream_socket_send(u8 *buf, u32 len);

extern int websockets_send_audio_buf_start(void);
extern int websockets_send_audio_buf_end(void);
extern int websockets_send_audio_buf_push(char *buf, int len);
extern int websocket_client_thread_create(void *priv);
extern int websockets_client_next_dialogue_init(void);
extern int websockets_dialogue_timeout_init(int time_out, char use_voice_note);
extern int websockets_close_request(char force_close);
extern void websockets_dialogue_timeout_del(void);
extern int websockets_wait_recv_end_msg(void);

extern int wifi_sta_is_connected(void);
extern void sdfile_save_test(char *buf, int len, char close);
extern void music_buf_play_set_stop(void);
extern void websockets_free_lbuf_buf(void);
extern void music_buf_play_free_lbuf(void);
extern int music_play_res_file(const char *name);
extern int music_play_hello(char index);
extern int music_buf_play_stop(void);
extern int music_buf_play_stop_waite(void);
extern int music_play_waite(void);
extern int net_music_num_clear(void);
extern void net_music_play_set_stop(void);
extern void music_buf_play_stop_all(void);
extern int auto_sleep_check_clear(void);

int __attribute__((weak)) websockets_send_500ms_audio_buf_push(char *buf, int len, int clear)
{
    return 0;
}
int __attribute__((weak)) websockets_send_audio_buf_init(void)
{
    return 0;
}
int __attribute__((weak))  websockets_nobind_check(void)
{
    return 0;
}
int  __attribute__((weak)) aisp_record_callbak(int start)//start 1开始录音， 0结束录音
{
    return 0;//返回0开唤醒词，1关闭唤醒词
}
int  __attribute__((weak)) aisp_asr_disable(void)
{
    return 0;//返回0开唤醒词，1关闭唤醒词
}
void  __attribute__((weak)) aisp_vad_callback(int start)//start：1 VAD开始，0 VAD结束
{
}
void  __attribute__((weak)) aisp_audio_pcm_callback(char *buf, int size)
{
}
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
static void ifly_audio_record_enc_server_event_handler(void *priv, int argc, int *argv)
{
    printf("ifly_audio_record_enc_server_event_handler, argv[0]=0x%x\r\n", argv[0]);
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
    case AUDIO_SERVER_EVENT_END:
        break;
    case AUDIO_SERVER_EVENT_SPEAK_START:
        break;
    case AUDIO_SERVER_EVENT_SPEAK_STOP:
        break;
    default:
        break;
    }
}

static int ifly_audio_record_vfs_fwrite(void *file, void *data, u32 vp_len)
{
    cbuffer_t *pl_cbuf = (cbuffer_t *)file;
    u32 vl_writed_len = cbuf_write(pl_cbuf, data, vp_len);
//    putbyte('@');
    //printf("jsyan vp_len:%d\r\n",vp_len);
    if (vl_writed_len != vp_len) {
        printf("ifly_audio_record_vfs_fwrite, busy, wlen=%d, len=%d\r\n", vl_writed_len, vp_len);
        cbuf_clear(pl_cbuf);
        return vp_len;
    }
//    os_sem_set(&read_available_sem, 0);
    os_sem_post(&read_available_sem);
    return vp_len;
}

static int ifly_audio_record_vfs_fclose(void *file)
{
    return 0;
}

static const struct audio_vfs_ops ifly_audio_record_vfs_ops = {
    .fwrite = ifly_audio_record_vfs_fwrite,
    .fclose = ifly_audio_record_vfs_fclose,
};

#define KWS_WAKEUP_SCORE  (0.5)
#define KWS_CMD_SCORE     (0.5)
#define KWS_EXIT_TIME     (10)

static unsigned short kws_timer = 0;
static unsigned char kws_mode = 0;
// 超时退出唤醒
static void user_kws_timeout_cb(void)
{
    printf("LOCAL_ASR****Timeout exit wakeup.\n");
    kws_mode = WAKEUP_MODE;
    //uni_mode_switch(WAKEUP_MODE); // 退出唤醒状态
    kws_timer = 0;
    // TODO: 可发消息通知退出唤醒
}

// 更新超时退出唤醒时间
static void user_kws_time_update(int is_wakeup)
{
    if (is_wakeup && kws_timer) {
        sys_timer_re_run(kws_timer);
    } else {
        if (is_wakeup) {
            kws_timer = sys_timeout_add_to_task("sys_timer", NULL, user_kws_timeout_cb, KWS_EXIT_TIME * 1000);
        } else {
            if (kws_timer) {
                sys_timer_del(kws_timer);
                kws_timer = 0;
            }
            kws_mode = WAKEUP_MODE;
            //uni_mode_switch(WAKEUP_MODE); // 退出唤醒状态
        }
    }
}
int user_sys_close_force(int close);
static void AisEventCb(AikEvent event, void *args)
{
    switch (event) {
    case  AIK_EVENT_KWS_WAKEUP: {
            AikEventKwsArgs *kws = (AikEventKwsArgs *)args;
            printf("KWS [%s]#####score[%f]#####{wakeup}#index[%d]#is_oneshot[%d]\n", kws->word, kws->score, kws->kws_index, kws->is_oneshot);
            if (kws->score >= KWS_WAKEUP_SCORE) {
                if (!strcmp("小飞小飞", kws->word) || !strcmp("小雄小雄", kws->word)) {
                    //user_sys_close_force(0);
                    if (!aisp_asr_disable()) {
                        aisp_wake(0);
                    }
                } else if (!strcmp("配网模式", kws->word)) {
                    //user_sys_close_force(1);
                    if (!aisp_asr_disable()) {
                        aisp_wake(8);
                    }
                }
            }
            kws_mode = CMD_MODE;
            //uni_mode_switch(CMD_MODE);
            user_kws_time_update(1);
            break;
        }
    case AIK_EVENT_KWS_COMMAND: {
            AikEventKwsArgs *kws = (AikEventKwsArgs *)args;
            printf("Command[%s]#####score[%f]#####{asr}#index[%d]#is_oneshot[%d]\n", kws->word, kws->score, kws->kws_index, kws->is_oneshot);
            if (kws->score >= KWS_CMD_SCORE) {
                if (!strcmp("小飞小飞", kws->word) || !strcmp("小雄小雄", kws->word)) {
                    //user_sys_close_force(0);
                    if (!aisp_asr_disable()) {
                        aisp_wake(0);
                    }
                } else if (!strcmp("配网模式", kws->word)) {
                    //user_sys_close_force(1);
                    user_kws_time_update(0);
                    if (!aisp_asr_disable()) {
                        aisp_wake(8);
                    }
                } else {
                    //user_sys_close_force(1);
                    aisp_clear(0);
                    user_kws_time_update(0);
                    //int light_pwm_instruction_word_callback(char *word); //语音控制灯
                    //light_pwm_instruction_word_callback(kws->word);
                }
            }
            break;
        }
    case AIK_EVENT_NONE:
        printf("AIK_EVENT_NONE\n");
        break;
    case  AIK_EVENT_START:
        printf("AIK_EVENT_START\n");
        break;
    case  AIK_EVENT_STOP:
        printf("AIK_EVENT_STOP\n");
        break;
    case  AIK_EVENT_EXIT:
        printf("AIK_EVENT_EXIT\n");
        break;
    case  AIK_EVENT_KWS_TIMEOUT:
        uni_mode_switch(WAKEUP_MODE);
        printf("AIK_EVENT_KWS_TIMEOUT\n");
        break;
    case  AIK_EVENT_HEARTBEAT:
        printf("AIK_EVENT_HEARTBEAT\n");
        break;
    default:
        printf("Nothing\n");
        break;
    }
    return;
}
void aisp_resume(void);
void record_server_init()
{
    union audio_req req_mic = {0};

    if (!server_handle) {
        server_handle = server_open("audio_server", "enc");
        if (!server_handle) {
            return;
        }
        server_register_event_handler(server_handle, NULL, ifly_audio_record_enc_server_event_handler);
    }
    aisp_resume();
}

void record_config()
{
    printf("jsyan record_config.\r\n");
    cbuf_init(&gs_mic_cbuf, mic_buf, sizeof(mic_buf));//原始音频buf
    os_sem_create(&read_available_sem, 0);
}
void aisp_wake(char index)//0:小飞小飞，8：配网模式
{
    int msg[4] = {0};
    if (index == 8) {
        keyword_start = 1;
    }
    msg[0] = index;
    os_taskq_post_type("esr_queue_task", Q_USER, 1, msg);
}

extern const int speech_energy_min;
extern char speech_digital_vol_agc;

int speech_digital_vol_agc_enable(char enable)
{
    static char init_agc = 0;
    if (!init_agc && speech_digital_vol_agc) { //开启数字增益：远场识别
        init_agc = speech_digital_vol_agc;
    }
    if (enable) {
        speech_digital_vol_agc = init_agc ? init_agc : enable;
    } else {
        if (!init_agc) { //不开启数字增益：近场识别
            speech_digital_vol_agc = enable;
        }
    }
    return speech_digital_vol_agc;
}
int pcm_send_set_status(char start)
{
    if (start) {
        if (ifly_send_pcm != SEND_PCM_START && ifly_send_pcm != SEND_PCM_DOING) {
            keyword_start = 1;
            ifly_send_pcm = SEND_PCM_START;
        }
    } else if (ifly_send_pcm == SEND_PCM_DOING || ifly_send_pcm == SEND_PCM_START) {
        ifly_send_pcm = SEND_PCM_STOP;
    }
}
int pcm_send_get_status(void)
{
    return ifly_send_pcm == SEND_PCM_START || ifly_send_pcm == SEND_PCM_DOING;
}

static int first_500ms_len = 0;
static int first_500ms_copy = 0;
void pcm_buf_clear(void)
{
    if (ifly_send_pcm != SEND_PCM_PAUSE) {
        ifly_send_pcm = SEND_PCM_INI;
        first_500ms_len = 0;
        first_500ms_copy = true;
    }
}
void aisp_wake_set_noice(char noice_note)
{
    if (sys_connect_net_success() && !keyword_wifi_enter_congfig) {
        if (ifly_send_pcm != SEND_PCM_PAUSE) {
            //通知app挂起
            struct key_event key = {0};
            key.type = KEY_EVENT_USER;
            if (last_key_supspend != KEY_SUPSPEND) {
                key.action = KEY_EVENT_LONG;
                key.value = KEY_SUPSPEND;
                if (!ai_speaker_app()) {
                    printf("-> ! ai_speaker_app\n");
                    key_event_notify(KEY_EVENT_FROM_USER, &key);
                    last_key_supspend = KEY_SUPSPEND;
                }
                os_time_dly(10);
            }
            auto_sleep_check_clear();
            //通知app挂起
            keyword_start = 0;
            sys_vad_clear(0);
            sys_vad_lock(1);
            ifly_send_pcm = SEND_PCM_STOP;

            music_buf_play_accept(0);
            websockets_close_request(1);
            websockets_dialogue_timeout_del();
            websockets_free_lbuf_buf();

            if (alarm_music_play_stop()) { //闹钟在响停止闹钟
                music_play_stop(NULL);
                music_buf_play_stop_all();
                music_buf_play_set_stop();
                http_tts_play_stop_force();
                //net_music_play_set_stop();
                music_buf_play_free_lbuf();
                music_buf_play_stop_waite();//等待关闭完成再播放提示音
            }
            net_music_num_clear();
            led_eya_wake(1);//LED眼睛唤醒显示

            char hello_index = rand() % 5;
            if (noice_note) {
                aisp_mic_gain_suspend();
                if (!music_play_hello(hello_index)) {
                    music_play_waite();
                }
                aisp_mic_gain_resum();
                music_buf_play_set_stop();//停止TTS使用PIPE BUF
            }

            websockets_send_audio_buf_init();
            websockets_send_500ms_audio_buf_push(NULL, 0, 1);

            ifly_send_pcm = SEND_PCM_START;
            keyword_start = 1;
            pcm_buf_clear();
            sys_vad_lock(0);
            sys_vad_clear(1);
#ifdef AI_UART_CMD_CTROL_ENABLE
            ai_uart_cmd_data_push(AI_UART_CMD_DIALUOGE_START, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
            at_uart_cmd_send(AI_UART_CMD_DIALUOGE_START, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
            play_face_emoji(AI_UART_CMD_DIALUOGE_START);
#endif
            websocket_client_thread_create(1);
            music_buf_play_accept(1);
        }
    }
}

static void key_vad_pcm_send_set(char start, char noice_note, int vad_end_enable)
{
    char hello_index = rand() % 5;
    if (start) {
        printf("--->KAY VAD SPEAK_START\n");
        //通知app挂起
        struct key_event key = {0};
        key.type = KEY_EVENT_USER;
        if (last_key_supspend != KEY_SUPSPEND) {
            key.action = KEY_EVENT_LONG;
            key.value = KEY_SUPSPEND;
            if (!ai_speaker_app()) {
                printf("-> ! ai_speaker_app\n");
                key_event_notify(KEY_EVENT_FROM_USER, &key);
                last_key_supspend = KEY_SUPSPEND;
                os_time_dly(10);
            }
        }
        auto_sleep_check_clear();

        music_buf_play_accept(0);
        websockets_close_request(1);
        websockets_dialogue_timeout_del();
        websockets_free_lbuf_buf();

        if (alarm_music_play_stop()) { //闹钟在响停止闹钟
            music_play_stop(NULL);
            music_buf_play_stop_all();
            music_buf_play_set_stop();
            http_tts_play_stop_force();
            //net_music_play_set_stop();
            music_buf_play_free_lbuf();
            music_buf_play_stop_waite();//等待关闭完成再播放提示音
        }
        net_music_num_clear();
        led_eya_wake(1);//LED眼睛唤醒显示

#ifdef CONFIG_LVGL_UI_ENABLE
        lv_demo_ai_dialogue_start_mode(1);
        lv_demo_ai_dialogue_start(hello_index);
#endif

#ifdef AI_UART_CMD_CTROL_ENABLE
        ai_uart_cmd_data_push(AI_UART_CMD_DIALUOGE_START, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
        at_uart_cmd_send(AI_UART_CMD_DIALUOGE_START, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
        play_face_emoji(AI_UART_CMD_DIALUOGE_START);
#endif

        if (noice_note) {
            aisp_mic_gain_suspend();
            if (!music_play_hello(hello_index)) {
                music_play_waite();
            }
            aisp_mic_gain_resum();
            music_buf_play_set_stop();//停止TTS使用PIPE BUF
        }
        //websocket_client_thread_create(1);
        websockets_send_audio_buf_init();
        websockets_send_500ms_audio_buf_push(NULL, 0, 1);
        websocket_client_thread_create_new(1, 1);
        music_buf_play_accept(1);
        if (vad_end_enable) {
            keyword_start = 1;
        } else {
            keyword_start = 2;
        }
        ifly_send_pcm = SEND_PCM_START;
    } else if (qyai_chat_addr_version() <= 1) {
        if (ifly_send_pcm == SEND_PCM_DOING || ifly_send_pcm == SEND_PCM_START) {
            ifly_send_pcm = SEND_PCM_STOP;
#ifdef CONFIG_UI_PLAY_EMOJI
            play_face_emoji(AI_UART_CMD_REC_END);
#endif
        }
        printf("--->KAY VAD SPEAK_STOP\n");
        keyword_start = 0;
        sys_vad_active = 0;
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

void sys_vad_event_handler(void *priv)
{
    aisp_vad_callback((int)priv);
    if (keyword_start == 2) {
        sys_vad_active = 1;
        return;
    }
    if (priv) {
        sys_vad_active = 1;
#if ASR_CONTINIU_EN
        if (keyword_start) {
#else   //播放完当前对话才能继续
        if (keyword_start && (ifly_send_pcm == SEND_PCM_STOP || ifly_send_pcm == SEND_PCM_INI)
            && music_buf_play_stop_status() && music_play_stop_status() && !websockets_wait_recv_end_msg()) {
#endif // ASR_CONTINIU_EN
#ifdef CONFIG_UI_PLAY_EMOJI
            play_face_emoji(AI_UART_CMD_REC_START);
#endif
#ifdef AI_UART_CMD_CTROL_ENABLE
            ai_uart_cmd_data_push(AI_UART_CMD_REC_START, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
            at_uart_cmd_send(AI_UART_CMD_REC_START, NULL);
#endif
            music_buf_play_set_stop();
            aisp_record_callbak(1);
            websockets_send_audio_buf_init();
            websocket_client_thread_create(0);
            music_buf_play_accept(1);
            printf("--->SPEAK_START\n");
            ifly_send_pcm = SEND_PCM_START;
        } else {
            printf("->keyword_start %d %d %d %d %d\n", keyword_start,
                   ifly_send_pcm,
                   music_buf_play_stop_status(),
                   music_play_stop_status(),
                   !websockets_wait_recv_end_msg());
        }
    } else if (qyai_chat_addr_version() <= 1) {
        if (ifly_send_pcm == SEND_PCM_DOING || ifly_send_pcm == SEND_PCM_START) {
            printf("--->SPEAK_STOP = %d \n", ifly_send_pcm);
            ifly_send_pcm = SEND_PCM_STOP;
            aisp_record_callbak(0);
#ifdef CONFIG_UI_PLAY_EMOJI
            play_face_emoji(AI_UART_CMD_REC_END);
#endif
        }
        sys_vad_active = 0;
    }
}

static int start_t;
SEC_USED(.volatile_ram_code) ALIGNE(4) void aecProcess2(void* pdata, void *aec_out, int aec_out_size)
{
    int out_size = 0;
    char *aec_buf = g_aecOutput;
    int ret = uni_ssp_process(pdata, &aec_buf, &out_size);
    unsigned short len = AEC_FRAME_SIZE;
    if (ret) {
        printf("err ssp process auth timeout\n");
        return;
    }

    if (speech_digital_vol_agc) {
        int GAIN_MAX = speech_digital_vol_agc == 1 ? 8 : speech_digital_vol_agc;
        static double pcm_gain = 1;
        static int pcm_max = 0;
        static int pcm_min = 0;
        int pcm_cont = AEC_FRAME_POINTS;
        int tmp;
        short *val = (short *)g_aecOutput;
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
        pcm_gain += 0.01;
        if (pcm_gain > GAIN_MAX) {
            pcm_gain = GAIN_MAX;
        }
    }

    if (aec_out && aec_out_size >= len) {
        memcpy(aec_out, g_aecOutput, len);
    }
    return;
}
SEC_USED(.volatile_ram_code) ALIGNE(4) void ivwProcess()
{
    int ret = 0;
    int rlen = 0, vl_write_len;
    unsigned int vl_frame_len = AUDIO_ONCE_SR_POINTS_SIZE;
    volatile int vad_active = 0;
    volatile unsigned int t1 = 0;
    volatile short pd_mic2_test = 0xFFFF;
    volatile short pd_mic1_test = 0xFFFF;
    volatile short pd_aec_ref_test = 0xFFFF;
    volatile short pd_test = 0xFFFF;

    uni_ais_event_set_cb(AisEventCb);

    ret = uni_asr_init();
    if (ret) {
        printf("err asr init fail\n");
        return;
    }

    ret = uni_ssp_init();
    if (ret) {
        goto ssp_err;
    }

    char opus = !strcmp(qyai_chat_audio_enc_format(), "opus");
    int ASR_FIRST_BEFORE_BYTES = ifly_sample_rate * 2 / 2;//500ms
    char *first_500ms_buf = NULL;
    if (opus) {
        ASR_FIRST_BEFORE_BYTES = ASR_FIRST_BEFORE_BYTES > 0 ? ASR_FIRST_BEFORE_BYTES : (16000 * 2 / 2);
        first_500ms_buf = malloc(ASR_FIRST_BEFORE_BYTES); //500ms
        first_500ms_copy = 0;
        if (first_500ms_buf) {
            first_500ms_copy = true;
        }
        first_500ms_len = 0;
    }

    printf("-->asr_task start====================\r\n");
    start_t = timer_get_ms();

    keyword_start = 0;

#if (ASR_USED_AVD == ASR_WEBRTC_AVD) || (ASR_USED_AVD == ASR_MY_AVD) || (ASR_USED_AVD == ASR_JL_AVD)
    void *vad_cbuf = sys_vad_init(ifly_sample_rate, AUDIO_RECORD_ONCE_SR_POINTS * 2 * 64);
    if (vad_cbuf) {
        sys_vad_create(NULL);
    } else {
        printf("->vad_cbuf init err\n");
    }
#endif
    run_flag = 1;
    sys_vad_lock(0);
    sys_vad_clear(1);
    while (1) {
        //if(thread_kill_req()){
        if (kws_mode == CMD_MODE || kws_mode == WAKEUP_MODE) {
            uni_mode_switch(kws_mode);
            kws_mode = -1;
        }
        if (!asr_open_init) {
            printf("ivwProcess thread_kill_req\r\n");
            break;
        }
        //get audio
        if (cbuf_get_data_size(&gs_mic_cbuf) < vl_frame_len) {
            os_sem_pend(&read_available_sem, 3);
            continue;
        }
        extern int is_production_test_enter(char wake);
        extern int is_production_test_mic2_enter(void);
        extern int is_production_test_mic1_enter(void);
        extern int is_production_test_aec_enter(void);

        pd_test = is_production_test_enter(0) ? 0 : 0xFFFF;
        if (pd_test == 0) {
            pd_mic2_test = is_production_test_mic2_enter() ? 0 : 0xFFFF;
            pd_mic1_test = is_production_test_mic1_enter() ? 0 : 0xFFFF;
            //pd_aec_ref_test = is_production_test_aec_enter();
        }

        int vl_read_len = cbuf_read(&gs_mic_cbuf, audio_record_buf_read, vl_frame_len);
        if (vl_read_len != vl_frame_len) {
            printf("ifly_audio_record_read_pcm_frame, error, vl_read_len=%d, vl_mic_channel_frame_len=%d\r\n",
                   vl_read_len,
                   vl_frame_len);
            continue;
        }

        //重组数据
        short *pl_pcm_out = (short *)asr_buf_feed_to_engine;
        int vl_idx = 0;
        if (pd_test == 0) { //厂测模式
            for (volatile int i = 0; i < AUDIO_RECORD_ONCE_SR_POINTS; i++) {
#if AUDIO_RECORD_MIC_COUNT == 1
                *pl_pcm_out++ = audio_record_buf_read[vl_idx + 1];
                *pl_pcm_out++ = pd_test & audio_record_buf_read[vl_idx];//ref
                vl_idx += 2;
                //aec_ref_buf[i] = audio_record_buf_read[vl_idx];//ref
#else
                //2 mics
#ifdef WIFI_MODULE_V2_ENABLE
                *pl_pcm_out++ = pd_mic2_test & audio_record_buf_read[vl_idx + 1];//mic1
                *pl_pcm_out++ = pd_mic1_test & audio_record_buf_read[vl_idx + 2];//mic2
                *pl_pcm_out++ = pd_test & audio_record_buf_read[vl_idx];//ref
                //aec_ref_buf[i] = audio_record_buf_read[vl_idx];//ref
#else
                *pl_pcm_out++ = pd_mic2_test & audio_record_buf_read[vl_idx];//mic1
                *pl_pcm_out++ = pd_mic1_test & audio_record_buf_read[vl_idx + 1];//mic2
                *pl_pcm_out++ = pd_test & audio_record_buf_read[vl_idx + 2];//ref
                //aec_ref_buf[i] = audio_record_buf_read[vl_idx + 2];//ref
#endif // WIFI_MODULE_V2_ENABLE
                vl_idx += 3;
#endif // AUDIO_RECORD_MIC_COUNT
            }
        } else { //正常模式
            for (volatile int i = 0; i < AUDIO_RECORD_ONCE_SR_POINTS; i++) {
#if AUDIO_RECORD_MIC_COUNT == 1
                *pl_pcm_out++ = audio_record_buf_read[vl_idx + 1];
                *pl_pcm_out++ = audio_record_buf_read[vl_idx];//ref
                vl_idx += 2;
#else
                //2 mics
#ifdef WIFI_MODULE_V2_ENABLE
                *pl_pcm_out++ = audio_record_buf_read[vl_idx + 1];//mic1
                *pl_pcm_out++ = audio_record_buf_read[vl_idx + 2];//mic2
                *pl_pcm_out++ = audio_record_buf_read[vl_idx];//ref
#else
                *pl_pcm_out++ = audio_record_buf_read[vl_idx];//mic1
                *pl_pcm_out++ = audio_record_buf_read[vl_idx + 1];//mic2
                *pl_pcm_out++ = audio_record_buf_read[vl_idx + 2];//ref
#endif // WIFI_MODULE_V2_ENABLE
                vl_idx += 3;
#endif // AUDIO_RECORD_MIC_COUNT
            }
        }

#ifdef WIFI_PCM_STREAN_SOCKET_ENABLE
        if (!wifi_pcm_stream_socket_valid()) {
#endif
#ifdef CONFIG_IFLY_AEC_ASR_FOLLOW_VAD
            //=========================检测语音的幅值小于SPEECH_ENERGY_MIN(48)时判断为环境静音，不做AEC和语音识别处理，降低功耗15mA-20mA左右
            if (!sys_vad_active && (ifly_send_pcm == SEND_PCM_STOP || ifly_send_pcm == SEND_PCM_INI)) { //无语音活动
                if (!t1) {
                    t1 = timer_get_ms();
                }
                volatile char max_vad_min = 0;
                for (volatile int j = 0, idx = 0, vol_min = 0; j < AUDIO_RECORD_ONCE_SR_POINTS; j++) {
                    if (asr_buf_feed_to_engine[idx] >= speech_energy_min) { //mic1
                        max_vad_min = 1;//有效检测语音幅值
                        break;
                    }
#if AUDIO_RECORD_MIC_COUNT == 2
                    if (asr_buf_feed_to_engine[++idx] >= speech_energy_min) { //mic2
                        max_vad_min = 1;//有效检测语音幅值
                        break;
                    }
#endif
                    idx += 2;
                }
                if (!max_vad_min) {
                    if (timer_get_ms() - t1 > 2000) { //2秒内没有语音活动
                        vad_active = 0;
                        t1 = 0;
                    }
                } else {
                    vad_active = 1;
                    t1 = 0;
                }
            } else {
                vad_active = 1;
                t1 = 0;
            }
            if (!vad_active) {
                continue;
            }
#endif
#ifdef WIFI_PCM_STREAN_SOCKET_ENABLE
        }
#endif

//===================在这里做双MIC+回声消除算法，输出单MIC===============================
//      int _rec_vfs_fwrite(void *file, void *buf, u32 len);
//      _rec_vfs_fwrite(NULL, asr_buf_feed_to_engine, AUDIO_ONCE_SR_POINTS_SIZE);
//        cbuf_write(&asr_buf, asr_buf_feed_to_engine, AUDIO_ONCE_SR_POINTS_SIZE);
//        os_sem_post(&asr_sem);

        int out_size = 0;
        short *aec_buf = g_aecOutput;
        int ret = uni_ssp_process(asr_buf_feed_to_engine, &aec_buf, &out_size);

        if (speech_digital_vol_agc) {
            int GAIN_MAX = speech_digital_vol_agc == 1 ? 8 : speech_digital_vol_agc;
            static double pcm_gain = 1;
            static int pcm_max = 0;
            static int pcm_min = 0;
            int pcm_cont = AEC_FRAME_POINTS;
            int tmp;
            short *val = (short *)g_aecOutput;
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
            pcm_gain += 0.01;
            if (pcm_gain > GAIN_MAX) {
                pcm_gain = GAIN_MAX;
            }
        }

//#ifdef WIFI_PCM_STREAN_SOCKET_ENABLE
//        //alg
//        aecProcess2(asr_buf_feed_to_engine, aec_out_buf, sizeof(aec_out_buf));
//#else
//        aecProcess2(asr_buf_feed_to_engine, NULL, 0);
//#endif
//=============================================================================


#ifdef WIFI_PCM_STREAN_SOCKET_ENABLE
        if (wifi_pcm_stream_socket_valid()) {
            pl_pcm_out = (char*)wifi_stream_buf;
            for (volatile int i = 0, vl_idx = 0; i < AUDIO_RECORD_ONCE_SR_POINTS; i++) {
#if AUDIO_RECORD_MIC_COUNT == 1
                *pl_pcm_out++ = asr_buf_feed_to_engine[vl_idx];//mic1
                *pl_pcm_out++ = asr_buf_feed_to_engine[vl_idx + 1];//ref
                *pl_pcm_out++ = aec_buf[i];
                vl_idx += 2;
#else
                *pl_pcm_out++ = asr_buf_feed_to_engine[vl_idx];//mic1
                *pl_pcm_out++ = asr_buf_feed_to_engine[vl_idx + 1];//mic2
                *pl_pcm_out++ = asr_buf_feed_to_engine[vl_idx + 2];//ref
                *pl_pcm_out++ = aec_buf[i];
                vl_idx += 3;
#endif
            }
            wifi_pcm_stream_socket_send(wifi_stream_buf, AUDIO_ONCE_SR_POINTS * (AUDIO_RECORD_MIC_COUNT == 1 ? 3 : 4));
        }
#endif

        //ivw   q3twf
        if (1) {
            rlen = AEC_FRAME_SIZE;
//                printf("rlen = %d\n",rlen);
            if (!aisp_asr_disable() && keyword_start != 2) {
//===================在这里做算法写入音频===============================
                ret = uni_asr_process(aec_buf, rlen);
//=======================================================================
            }
            aisp_audio_pcm_callback(aec_buf, rlen);

#if (ASR_USED_AVD == ASR_WEBRTC_AVD) || (ASR_USED_AVD == ASR_MY_AVD) || (ASR_USED_AVD == ASR_JL_AVD)
            if (vad_cbuf) {
                sys_vad_send_msg(0x1, vad_cbuf, aec_buf, rlen);
            }
#elif (ASR_USED_AVD == ASR_IFLY_AVD)

            if (!cbuf_is_write_able(&s_vad_cbuf, rlen)) {
                cbuf_read_updata(&s_vad_cbuf, rlen);
                //cbuf_clear(&s_vad_cbuf);
            }
            vl_write_len = cbuf_write(&s_vad_cbuf, (void*)aec_buf, rlen);
            if (vl_write_len != rlen) {
                printf("vad buf, busy, wlen=%d\r\n", vl_write_len);
            }
#endif

            if (first_500ms_buf && first_500ms_copy) {
                if ((first_500ms_len + rlen) <= ASR_FIRST_BEFORE_BYTES) { //还没满buf则继续填满
                    memcpy((char*)first_500ms_buf + first_500ms_len, (char*)aec_buf, rlen);
                    first_500ms_len += rlen;
                } else if (rlen < ASR_FIRST_BEFORE_BYTES) { //满buf则继续更新最新数据在buf最后区域
                    if (first_500ms_len > rlen) {
                        memcpy((char*)first_500ms_buf, (char*)first_500ms_buf + rlen, first_500ms_len - rlen);
                        memcpy((char*)first_500ms_buf + first_500ms_len - rlen, (char*)aec_buf, rlen);
                    } else {
                        memcpy((char*)first_500ms_buf, (char*)aec_buf, rlen);
                        first_500ms_len = rlen;
                    }
                }
            }

            //if(keyword_start)
            {
                if (ifly_send_pcm == SEND_PCM_START) {
                    if (!sys_connect_net_success()) {
                        printf("err net connect\n\n");
                        goto reinit;
                    }
                    //music_buf_play_set_stop();
                    //websockets_free_lbuf_buf();
                    music_buf_play_free_lbuf();
                    if (qyai_audio_send_start()) {
                        if (qyai_audio_send_start()) {
                            printf("err pcm_buf_start\n\n");
                            continue;
                        }
                    }

                    if (first_500ms_copy) {
                        if (!qyai_audio_send_push((u8 *)first_500ms_buf, first_500ms_len)) {
//                                sdfile_onefile_save_test(first_500ms_buf, first_500ms_len, 0);//sd卡存储PCM
                            first_500ms_copy = 0;
                            ifly_send_pcm = SEND_PCM_DOING;
                        }
                    } else {
                        if (!qyai_audio_send_push((u8 *)aec_buf, rlen)) {
                            ifly_send_pcm = SEND_PCM_DOING;
                        }
                    }
                } else if (ifly_send_pcm == SEND_PCM_DOING) {
                    if (qyai_audio_send_push((u8 *)aec_buf, rlen) == -2) { //发送过程中缓存满且掉线
                        goto reinit;
                    }
//                        sdfile_onefile_save_test(aec_buf, rlen, 0);//sd卡存储PCM
                } else if (ifly_send_pcm == SEND_PCM_STOP) {
                    qyai_audio_send_push((u8 *)aec_buf, rlen);
//                        sdfile_onefile_save_test(aec_buf, rlen, 1);//sd卡存储PCM
                    qyai_audio_send_end();
reinit:
                    ifly_send_pcm = SEND_PCM_INI;
                    first_500ms_len = 0;
                    first_500ms_copy = true;

                } else if (!opus) {
                    websockets_send_500ms_audio_buf_push(aec_buf, rlen, 0);//
                }
            }
        }
    }
    uni_ssp_release();
ssp_err:
    uni_asr_deinit();
#if (ASR_USED_AVD == ASR_WEBRTC_AVD) || (ASR_USED_AVD == ASR_MY_AVD) || (ASR_USED_AVD == ASR_JL_AVD)
    if (vad_cbuf) {
        sys_vad_uninit(vad_cbuf);
        vad_cbuf = NULL;
    }
#endif
    if (first_500ms_buf) {
        free(first_500ms_buf);
    }
    return;
}
void aisp_all_pause(char stop)
{
    int to = 100;
    if (stop) {
        ifly_send_pcm = SEND_PCM_PAUSE;
        sys_vad_clear(0);
    } else {
        ifly_send_pcm = SEND_PCM_INI;
        sys_vad_clear(1);
    }
    keyword_start = 0;
    sys_vad_active = 0;

    union audio_req req = {0};
    if (server_handle) {
        req.enc.cmd = AUDIO_ENC_GET_STATUS;
        server_request(server_handle, AUDIO_REQ_ENC, &req);
        int status = req.enc.status;
        memset(&req, 0, sizeof(req));
        if (status == AUDIO_ENC_START && stop) {
            req.enc.cmd = AUDIO_ENC_PAUSE;
            server_request(server_handle, AUDIO_REQ_ENC, &req);
            printf("-> AUDIO_ENC_PAUSE \n");
        } else if (status == AUDIO_ENC_PAUSE && stop == 0) {
            req.enc.cmd = AUDIO_ENC_START;
            server_request(server_handle, AUDIO_REQ_ENC, &req);
            printf("-> AUDIO_ENC_START \n");
        }
    }
    music_buf_play_set_stop();
    http_tts_play_stop_force();
    websockets_free_lbuf_buf();
    music_buf_play_free_lbuf();
    websockets_close_request(1);
    music_buf_play_stop_waite();//等待关闭完成再播放提示音
    net_music_num_clear();
    while (!music_buf_play_stop_status() && --to) {
        os_time_dly(1);
    }
}

//本地唤醒识别线程
void create_awake_task()
{
    int ret = 0;
    ret = thread_fork("ivw_thread", 3, 3840, 0, &g_ivwThreadID, ivwProcess, NULL);
    if (OS_NO_ERR != ret) {
        printf("create ivw thread failed.\n");
    }
}

void aisp_clear(char stop_net_music)
{
//    aisp_wake(21);//
    keyword_start = 0;
    sys_vad_active = 0;
    sys_vad_clear(0);
    sys_vad_lock(1);
    ifly_send_pcm = SEND_PCM_STOP;
    music_buf_play_set_stop();
    http_tts_play_stop_force();
    if (stop_net_music) {
        net_music_play_set_stop();
    }
    sys_vad_clear(0);
    websockets_free_lbuf_buf();
    music_buf_play_free_lbuf();
    websockets_close_request(1);
    websockets_dialogue_timeout_del();
    music_buf_play_stop_waite();//等待关闭完成再播放提示音
    net_music_num_clear();
    sys_vad_lock(0);
    sys_vad_clear(1);
}

void aisp_timeout_exit(void)
{
    aisp_wake(20);//恢复APP
    aisp_timeout_callback();
}
void aisp_app_resum(void)
{
    aisp_wake(20);//恢复APP
}
void aisp_app_suspend(void)
{
    aisp_wake(21);//挂起APP
}
void esr_queue_task(void *priv)
{
    int err = 0;
    int msg[8] = {0};   //接收消息队列buf
    int alow_enter_wifi_cfg = 0;
    int index = 0;
    while (1) {
        //阻塞等待消息
        err = os_taskq_pend("taskq", msg, ARRAY_SIZE(msg));
        if (err != OS_TASKQ || msg[0] != Q_USER) {
            continue;
        }
        printf("-->recv msg kid[%d]\n", msg[1]);
        struct key_event key = {0};
        key.type = KEY_EVENT_USER;
        int net_ch = 0;
#if (defined CONFIG_WIFI_ENABLE && defined CONFIG_LTE_PHY_ENABLE)
        net_ch = sys_net_channel_read();
        if (net_ch < 0) {
            net_ch = 0;
        }
        if (net_ch == 1) {
            websockets_client_best_one_packge_set(1024);
        }
#endif
        index = aisp_wake_world_callback(msg[1]);
        if (index == 1) {
            goto __wakeup;
        } else if (index == 2) {
            goto __wifi_cfg;
        } else if (index) {
            continue;
        }
        switch (msg[1]) {
//keyword[0]=xiao3 fei1 xiao3 fei1, state=30, threshold=1450, low=10000, bound=3, subcm=1
//keyword[1]=da3 kai1 deng1 guang1, state=33, threshold=650, low=10000, bound=4, subcm=1
//keyword[2]=qie1 huan4 deng1 guang1, state=42, threshold=600, low=10000, bound=4, subcm=1
//keyword[3]=guan1 bi deng1 guang1, state=33, threshold=650, low=10000, bound=3, subcm=1
//keyword[4]=da3 kai1 xiang1 xun1, state=33, threshold=650, low=10000, bound=2, subcm=1
//keyword[5]=guan1 bi xiang1 xun1, state=33, threshold=700, low=10000, bound=2, subcm=1
//keyword[6]=xian3 shi4 shi1 du1, state=30, threshold=650, low=10000, bound=2, subcm=1
//keyword[7]=xian3 shi4 kong1 qi4 zhi3 shu4, state=45, threshold=550, low=10000, bound=2, subcm=1
//keyword[8]=pei4 wang3 mo2 shi4, state=27, threshold=600, low=10000, bound=2, subcm=1
//keyword[9]=lan2 ya2 yin1 xiang3 mo2 shi4, state=45, threshold=650, low=10000, bound=2, subcm=1
//keyword[10]=tiao2 gao1 yin1 liang4, state=33, threshold=700, low=10000, bound=4, subcm=1
//keyword[11]=tiao2 di1 yin1 liang4, state=33, threshold=700, low=10000, bound=4, subcm=1
        case 0:; // wake up
__wakeup:
            ;
            extern int sys_connect_net_success(void);
            extern int is_production_test_enter(char wake);
            if (is_production_test_enter(1)) {
                break;
            }
            auto_sleep_check_clear();
            if (sys_connect_net_success() && !keyword_wifi_enter_congfig) {
#ifdef AI_UART_CMD_CTROL_ENABLE
                ai_uart_cmd_data_push(AI_UART_CMD_DIALUOGE_START, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
                at_uart_cmd_send(AI_UART_CMD_DIALUOGE_START, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
                play_face_emoji(AI_UART_CMD_DIALUOGE_START);
#endif
                alow_enter_wifi_cfg = 0;
                if (ifly_send_pcm != SEND_PCM_PAUSE) {
                    //通知app挂起
                    if (last_key_supspend != KEY_SUPSPEND) {
                        key.action = KEY_EVENT_LONG;
                        key.value = KEY_SUPSPEND;
                        if (!ai_speaker_app()) {
                            printf("-> ! ai_speaker_app\n");
                            key_event_notify(KEY_EVENT_FROM_USER, &key);
                            last_key_supspend = KEY_SUPSPEND;
                            os_time_dly(10);
                        }
                    }

                    struct application *app = get_current_app();
                    if (app && (!strcmp(app->name, "bt_music"))) { //蓝牙播放模式下延时
                        os_time_dly(50);
                    }
                    keyword_start = 0;
                    sys_vad_clear(0);
                    sys_vad_lock(1);
                    ifly_send_pcm = SEND_PCM_STOP;

                    music_buf_play_accept(0);
                    websockets_close_request(1);
                    websockets_dialogue_timeout_del();
                    websockets_free_lbuf_buf();

                    if (alarm_music_play_stop()) { //闹钟在响停止闹钟
                        music_play_stop(NULL);
                        music_buf_play_stop_all();
                        music_buf_play_set_stop();
                        http_tts_play_stop_force();
                        //net_music_play_set_stop();
                        music_buf_play_free_lbuf();
                        music_buf_play_stop_waite();//等待关闭完成再播放提示音
                    }
                    net_music_num_clear();
                    led_eya_wake(1);//LED眼睛唤醒显示
#ifdef CONFIG_LVGL_UI_ENABLE
                    lv_demo_ai_dialogue_start(hello_index);
#endif
                    char hello_index = rand() % 5;
                    if (!aisp_wake_no_play_notice()) {
                        aisp_mic_gain_suspend();
                        if (!music_play_hello(hello_index)) {
                            music_play_waite();
                        }
                        aisp_mic_gain_resum();
                        music_buf_play_set_stop();//停止TTS使用PIPE BUF
                    }
                    keyword_start = 1;
                    pcm_buf_clear();
                    sys_vad_lock(0);
                    sys_vad_clear(1);
                    if (aisp_wake_callback(msg[1])) {
                        keyword_start = 0;
                    } else {
                        websockets_send_audio_buf_init();
                        websockets_send_500ms_audio_buf_push(NULL, 0, 1);
                        websocket_client_thread_create(1);
                    }
                    music_buf_play_accept(1);
                }
            } else if (keyword_wifi_enter_congfig) {
                alow_enter_wifi_cfg = 0;
                if (aisp_wake_callback(-1)) {
                    break;
                }
                if (keyword_wifi_enter_congfig == 3) { //正在连接WiFi
                    if (net_ch == 0) {
                        music_play_res_file("WifiConting.mp3");
                    } else {
                        music_play_res_file("NetConting.mp3");
                    }
                } else { //进入配网
                    if (net_ch == 0) {
                        music_play_res_file("WifiNote.mp3");
                    } else {
                        music_play_res_file("NetNote.mp3");
                    }
                }
            } else { //网络异常
                alow_enter_wifi_cfg = 1;
                if (aisp_wake_callback(-1)) {
                    break;
                }
                if (net_ch == 0) {
                    music_play_res_file("WifiConErr.mp3");
                } else {
                    music_play_res_file("NetDiscon.mp3");
                }
            }
            break;
        case 1:
            aisp_wake_callback(msg[1]);
#ifdef ASR_WORDS_CHANGE
            goto __wifi_cfg;
#endif
            break;
        case 2:
            aisp_wake_callback(msg[1]);
            break;
        case 3:
            aisp_wake_callback(msg[1]);
            break;
        case 4:
            aisp_wake_callback(msg[1]);
            break;
        case 5:
            aisp_wake_callback(msg[1]);
            break;
        case 6:
            aisp_wake_callback(msg[1]);
            break;
        case 7:
            aisp_wake_callback(msg[1]);
            break;
        case 8:; // 配网模式
__wifi_cfg:
            ;
            extern int is_production_test_enter(char wake);
            if (is_production_test_enter(1)) {
                break;
            }
            if (keyword_start || alow_enter_wifi_cfg || websockets_nobind_check() || (keyword_wifi_enter_congfig == 3 && !sys_connect_net_success)) {
#ifdef AI_UART_CMD_CTROL_ENABLE
                ai_uart_cmd_data_push(AI_UART_CMD_WIFI_CONFIG, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
                at_uart_cmd_send(AI_UART_CMD_WIFI_CONFIG, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
                play_face_emoji(AI_UART_CMD_WIFI_CONFIG);
#endif
                if (!ai_speaker_app()) {
                    extern int audio_app_mode_switch_set(char *name, char note);
                    audio_app_mode_switch_set("ai_speaker", 0);
                }
                keyword_start = 0;
                keyword_wifi_enter_congfig = 1;
                sys_vad_clear(0);
                sys_vad_lock(1);
                ifly_send_pcm = SEND_PCM_STOP;

                websockets_client_dialogue_exit();
                websockets_close_request(1);
                websockets_dialogue_timeout_del();
                websockets_free_lbuf_buf();

                if (alarm_music_play_stop()) { //闹钟在响停止闹钟
                    music_play_stop(NULL);
                    music_buf_play_stop_all();
                    music_buf_play_set_stop();
                    http_tts_play_stop_force();
                    //net_music_play_set_stop();
                    music_buf_play_free_lbuf();
                    music_buf_play_stop_waite();//等待关闭完成再播放提示音
                }
                net_music_num_clear();
                pcm_buf_clear();
                if (net_ch == 0) {
                    music_play_res_file("WifiNote.mp3");
                } else {
                    music_play_res_file("NetNote.mp3");
                }
                if (!is_in_config_network_state()) {
                    config_network_start();
                }
                sys_vad_lock(0);
                sys_vad_clear(1);
#if (defined TCFG_LED_STATUES_VBAT_NET_EN && TCFG_LED_STATUES_VBAT_NET_EN == 1)
                extern void led_status_set(int status);//0初始化 1网络异常-快闪  2电池<20%-快闪  3网络正常和电池>=20%-常亮
                led_status_set(1);
#endif
            }
            aisp_wake_callback(msg[1]);
            break;
        case 9:
            break;
        case 10://调高音量
            aisp_wake_callback(msg[1]);
            /*
            if(keyword_start){
                aisp_clear(0);
                key.action = KEY_EVENT_CLICK;
                key.value = KEY_VOLUME_INC;
                key_event_notify(KEY_EVENT_FROM_USER, &key);
                extern int music_play_anser_OK(void);
                sys_timeout_add_to_task("sys_timer",NULL,music_play_anser_OK,500);
            }*/
            break;
        case 11://调低音量
            aisp_wake_callback(msg[1]);
            /*
            if(keyword_start){
                aisp_clear(0);
                key.action = KEY_EVENT_CLICK;
                key.value = KEY_VOLUME_DEC;
                key_event_notify(KEY_EVENT_FROM_USER, &key);
                extern int music_play_anser_OK(void);
                sys_timeout_add_to_task("sys_timer",NULL,music_play_anser_OK,500);
            }*/
            break;
        case 20:;//恢复APP
            //printf("-> last_key_supspend = %d \n",last_key_supspend);
            extern int ai_speaker_app(void);
            websockets_close_request(1);
            if (last_key_supspend != KEY_RESUM) {
                last_key_supspend = KEY_RESUM;
                key.action = KEY_EVENT_LONG;
                key.value = KEY_RESUM;
                if (!ai_speaker_app()) {
                    key_event_notify(KEY_EVENT_FROM_USER, &key);
                }
            }
            break;
        case 21:
            //通知app挂起
            if (last_key_supspend != KEY_SUPSPEND) {
                key.action = KEY_EVENT_LONG;
                key.value = KEY_SUPSPEND;
                if (!ai_speaker_app()) {
                    printf("-> ! ai_speaker_app\n");
                    key_event_notify(KEY_EVENT_FROM_USER, &key);
                    last_key_supspend = KEY_SUPSPEND;
                }
            }
            break;
        case 22:
//                keyword_start = 0;
//                sys_vad_clear(0);
//                sys_vad_lock(1);
//                ifly_send_pcm = SEND_PCM_STOP;
//                music_buf_play_set_stop();
//                http_tts_play_stop_force();
//                net_music_play_set_stop();
//                sys_vad_clear(0);
//                websockets_free_lbuf_buf();
//                music_buf_play_free_lbuf();
//                websockets_close_request(1);
//                websockets_dialogue_timeout_del();
//                music_buf_play_stop_waite();//等待关闭完成再播放提示音
//                net_music_num_clear();
//                sys_vad_lock(0);
//                sys_vad_clear(1);
            break;
        default:
            break;
        }
    }
}


void app_esr_take()
{
    static char esr_queue_task_init = 0;
    if (!esr_queue_task_init) {
        if (thread_fork("esr_queue_task", 27,   1024, 256, NULL, esr_queue_task, NULL) != OS_NO_ERR) {
            printf("esr_queue_task thread fork fail\n");
        } else {
            esr_queue_task_init = 1;
        }
    }
}
int aisp_open(u16 sample_rate)
{
    if (aisp_open_init) {
        printf("---------aisp_already open---------\n");
        aisp_clear(0);
        return 0;
    }
    aisp_open_init = 1;
    ifly_sample_rate = sample_rate;
    qyai_audio_sample_rate_set(sample_rate);
    printf("---------aisp_open---------\n");

    if (!asr_open_init) {
        asr_open_init = 1;
        record_config();

        //本地唤醒识别线程
        create_awake_task();

        // 唤醒识别处理任务
        app_esr_take();

        // aiui处理线程
#if (ASR_USED_AVD == ASR_IFLY_AVD)
        user_app_recognize_task();
#endif

        //    user_app_main();
        //    aisp_clear(0);
    }
    record_server_init();
    return 0;
}

void aisp_suspend(void)
{
    union audio_req req = {0};

    if (!run_flag || !server_handle) {
        return;
    }

    printf("---------aisp_suspend---------\n");
    run_flag = 0;
    req.enc.cmd = AUDIO_ENC_STOP;
    server_request(server_handle, AUDIO_REQ_ENC, &req);

    memset(&req, 0, sizeof(req));
    req.enc.cmd = AUDIO_ENC_CLOSE;
    server_request(server_handle, AUDIO_REQ_ENC, &req);

    cbuf_clear(&gs_mic_cbuf);
}

void aisp_resume(void)
{
    union audio_req req_mic = {0};

    if (run_flag || !server_handle) {
        return;
    }

    run_flag = 1;

    // os_sem_set(&read_available_sem, 0);
    // os_sem_post(&read_available_sem);

    req_mic.enc.cmd = AUDIO_ENC_OPEN;
    req_mic.enc.sample_rate = ifly_sample_rate;
    req_mic.enc.format = qyai_mic_audio_enc_format();//"pcm";

    if (!strcmp(req_mic.enc.format, "opus")) {
        req_mic.enc.no_header = 1;//1表示没有头部的意思
    }

#if AUDIO_RECORD_MIC_COUNT == 1
    req_mic.enc.channel = 2;
    if (aisp_mic_channel_set) {
        req_mic.enc.channel_bit_map = BIT(aisp_mic_channel_set) | BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
    } else {
        req_mic.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL) | BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
    }
//    req_mic.enc.frame_size = AUDIO_RECORD_ONCE_SR_POINTS * 2 * 2;
#else
    req_mic.enc.channel = 3;
    req_mic.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL) | BIT(CONFIG_AISP_MIC1_ADC_CHANNEL) | BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
//    req_mic.enc.frame_size = AUDIO_RECORD_ONCE_SR_POINTS * 2 * 3;
#endif // AUDIO_RECORD_MIC_COUNT

    req_mic.enc.frame_size = AUDIO_RECORD_ONCE_SR_POINTS * 2 * req_mic.enc.channel;
    req_mic.enc.sample_source = "mic";
    req_mic.enc.volume = CONFIG_AISP_MIC_ADC_GAIN;
    //
    req_mic.enc.vfs_ops = &ifly_audio_record_vfs_ops;
    req_mic.enc.output_buf_len = req_mic.enc.frame_size * 3;
    req_mic.enc.file = (FILE *)&gs_mic_cbuf;


//    struct aec_s_attr aec_param = {0};
//    aec_param.EnableBit = AEC_MODE_ADVANCE;
//    aec_param.agc_en = 1;
//    /*aec_param.output_way = 1;*/
//    extern void get_cfg_file_aec_config(struct aec_s_attr * aec_param);
//    get_cfg_file_aec_config(&aec_param);
//    req_mic.enc.aec_attr = &aec_param;
//    req_mic.enc.aec_enable = 1;

    server_request(server_handle, AUDIO_REQ_ENC, &req_mic);


//    extern void adc_multiplex_set_gain(const char *source, u8 channel_bit_map, u8 gain);
//    adc_multiplex_set_gain("mic", BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL), CONFIG_AISP_LINEIN_ADC_GAIN);

    union audio_req req_vol = {0};
    //麦克风和参考信号增益分开设置
    if (aisp_mic_channel_set) {
        req_mic.enc.channel_bit_map = BIT(aisp_mic_channel_set);
    } else {
        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL);
    }
    req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
    req_vol.enc.volume = CONFIG_AISP_MIC_ADC_GAIN;
    server_request(server_handle, AUDIO_REQ_ENC, &req_vol);

#if AUDIO_RECORD_MIC_COUNT == 2
    req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC1_ADC_CHANNEL);
    req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
    req_vol.enc.volume = CONFIG_AISP_MIC_ADC_GAIN;
    server_request(server_handle, AUDIO_REQ_ENC, &req_vol);
#endif // AUDIO_RECORD_MIC_COUNT

    memset(&req_vol, 0, sizeof(req_vol));
    req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
    req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
    req_vol.enc.volume = CONFIG_AISP_AEC_ADC_GAIN;
    server_request(server_handle, AUDIO_REQ_ENC, &req_vol);

    //SFR(JL_ANA->DAA_CON0, 2,  2, 0x3);//设置DACVDD-3V
}

int aisp_mic_gain_suspend(void)
{
    int pa_volume = 0;
    sys_volume_read(&pa_volume);//获取音量
    aisp_mic_gain_set(pa_volume);//设置回采和mic的增益
    speech_digital_vol_agc_enable(1);//只有播放音乐才加数字增益
    //speech_digital_vol_agc_enable(speech_digital_vol_agc);//只有播放音乐才加数字增益
    return 0;
}
int aisp_mic_gain_resum(void)
{
    union audio_req req_vol = {0};
    if (server_handle && run_flag) {
        speech_digital_vol_agc_enable(0);//只有播放音乐才加数字增益
        if (aisp_mic_channel_set) {
            req_vol.enc.channel_bit_map = BIT(aisp_mic_channel_set);
        } else {
            req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL);
        }
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = CONFIG_AISP_MIC_ADC_GAIN;//恢复增益到默认
        server_request(server_handle, AUDIO_REQ_ENC, &req_vol);

#if AUDIO_RECORD_MIC_COUNT == 2
        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC1_ADC_CHANNEL);//接近喇叭的MIC需要减5左右
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = CONFIG_AISP_MIC_ADC_GAIN;//恢复增益到默认
        server_request(server_handle, AUDIO_REQ_ENC, &req_vol);
#endif

        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = CONFIG_AISP_AEC_ADC_GAIN;//恢复增益到默认
        server_request(server_handle, AUDIO_REQ_ENC, &req_vol);
    }
    return 0;
}
int aisp_mic_gain_set(unsigned char volume)
{
    union audio_req req_vol = {0};

    if (server_handle && run_flag) {
        aisp_aec_gain = AEC_GAIN(volume);
        aisp_mic0_gain = MIC0_GAIN(volume);
        aisp_mic1_gain = MIC1_GAIN(volume);

        aisp_aec_gain = aisp_aec_gain >= MIC_MAX_GAIN ? MIC_MAX_GAIN : (aisp_aec_gain < 30 ? 30 : aisp_aec_gain);
        aisp_mic0_gain = aisp_mic0_gain >= MIC_MAX_GAIN ? MIC_MAX_GAIN : (aisp_mic0_gain < 10 ? 10 : aisp_mic0_gain);
        aisp_mic1_gain = aisp_mic1_gain >= MIC_MAX_GAIN ? MIC_MAX_GAIN : (aisp_mic1_gain < 10 ? 10 : aisp_mic1_gain);
        printf("-> volume = %d, aec = %d, mic0 = %d, mic1 = %d \n", volume, aisp_aec_gain, aisp_mic0_gain, aisp_mic1_gain);

        if (aisp_mic_channel_set) {
            req_vol.enc.channel_bit_map = BIT(aisp_mic_channel_set);
        } else {
            req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL);
        }
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = aisp_mic0_gain;//CONFIG_AISP_MIC_ADC_GAIN;
        server_request(server_handle, AUDIO_REQ_ENC, &req_vol);

#if AUDIO_RECORD_MIC_COUNT == 2
        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC1_ADC_CHANNEL);//接近喇叭的MIC需要减5左右
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = aisp_mic1_gain;//CONFIG_AISP_MIC_ADC_GAIN;
        server_request(server_handle, AUDIO_REQ_ENC, &req_vol);
#endif // AUDIO_RECORD_MIC_COUNT

        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = aisp_aec_gain;
        server_request(server_handle, AUDIO_REQ_ENC, &req_vol);
    }
    return 0;
}
int aisp_all_mic_gain_set(int volume, int aec_gian, int mic0_gian, int mic1_gian)
{
    union audio_req req_vol = {0};

    if (server_handle && run_flag && aec_gian) {
        aec_gian = aec_gian >= MIC_MAX_GAIN ? MIC_MAX_GAIN : (aec_gian < 30 ? 30 : aec_gian);
        mic0_gian = mic0_gian >= MIC_MAX_GAIN ? MIC_MAX_GAIN : (mic0_gian < 10 ? 10 : mic0_gian);
        mic1_gian = mic1_gian >= MIC_MAX_GAIN ? MIC_MAX_GAIN : (mic1_gian < 10 ? 10 : mic1_gian);
        printf("-> volume = %d, aec_gain = %d, mic0_gian = %d, mic1_gian = %d\n", volume, aec_gian, mic0_gian, mic1_gian);

        if (aisp_mic_channel_set) {
            req_vol.enc.channel_bit_map = BIT(aisp_mic_channel_set);
        } else {
            req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL);
        }
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = mic0_gian;//CONFIG_AISP_MIC_ADC_GAIN;
        server_request(server_handle, AUDIO_REQ_ENC, &req_vol);

#if AUDIO_RECORD_MIC_COUNT == 2
        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC1_ADC_CHANNEL);//接近喇叭的MIC需要减5左右
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = mic1_gian;//CONFIG_AISP_MIC_ADC_GAIN;
        server_request(server_handle, AUDIO_REQ_ENC, &req_vol);
#endif // AUDIO_RECORD_MIC_COUNT

        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = aec_gian;
        server_request(server_handle, AUDIO_REQ_ENC, &req_vol);
    }
    return 0;
}
void aisp_close(void)
{
    union audio_req req = {0};
    if (!server_handle || !aisp_open_init) {
        return;
    }
    aisp_open_init = 0;
    printf("---------aisp_close---------\n");
    aisp_suspend();
    server_close(server_handle);
    server_handle = NULL;

//    asr_open_init = 0;
//    os_sem_post(&read_available_sem);
//    thread_kill(&g_ivwThreadID, KILL_WAIT);
//    ifly_engine_uninit();

    aisp_clear(0);
    printf("---------aisp_close ok---------\n");
}



#endif
