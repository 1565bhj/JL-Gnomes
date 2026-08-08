#include "server/audio_server.h"
#include "server/server_core.h"
#include "generic/circular_buf.h"
#include "fs/fs.h"
#include "os/os_api.h"
#include "fn_asr.h"
#include "event.h"
#include "gpio.h"
#include "app_config.h"
#include "avctp_user.h"
#include "system/timer.h"
#include "wifi/wifi_connect.h"
#include "app_core.h"
#include "event/key_event.h"

//做应用请参考以下两个分组的指令
//分组0: 主要用于在播放的情况下需要唤醒打断做识别，有较高的识别率
//Wanson_ASR_Reset(0);
//小艾小艾 or 你好小兔

//分组1：主要用于唤醒词识别后，音乐播放停止后用来做识别
//Wanson_ASR_Reset(1);
//继续播放、暂停播放

#if (defined CONFIG_ASR_ALGORITHM) && (CONFIG_ASR_ALGORITHM == WANSON_ALGORITHM)

//#define CHANNEL_NUM       (AUDIO_RECORD_MIC_COUNT+1) //(2) //1mic+ 1aec
//#define FRAME_SAMPLES     (512)
//#define UNI_BUFF_SIZE       (FRAME_SAMPLES*CHANNEL_NUM)

//#define AEC_GAIN(v)     (int)(CONFIG_AISP_AEC_ADC_GAIN)
//#define MIC0_GAIN(v)    (int)(CONFIG_AISP_MIC_ADC_GAIN)   //固定CONFIG_AISP_MIC_ADC_GAIN-80
//#define MIC1_GAIN(v)    (MIC0_GAIN(v))

//宇航员.................
//y = -0.7143x + 112.86
#define AEC_GAIN(v)     (int)(-0.7143 * (double)(v) + 115.86)
//y = -0.7143x + 122.86
#define MIC0_GAIN(v)    (int)(-0.7143 * (double)(v) + 127.86)
#define MIC1_GAIN(v)     MIC0_GAIN(v)

#define MIC_MAX_GAIN 95
#define AUDIO_RECORD_MIC_COUNT 1
/* Constants and Macros */
#ifdef CONFIG_AEC_ENC_ENABLE
/* AEC module enable bit define */
//AEC模式,AEC_EN:BIT(0) NLP_EN:BIT(1) NS_EN:BIT(2) AGC_EN:BIT(4) DNS_EN:BIT(5)
#define AEC_EN              BIT(0)
#define NLP_EN              BIT(1)
#define ANS_EN              BIT(2)

#define AEC_MODE_ADVANCE    (AEC_EN | NLP_EN)// | ANS_EN)
#define AEC_MODE_REDUCE     (NLP_EN | ANS_EN)
#define AEC_MODE_SIMPLEX    (ANS_EN)
#endif

//将唤醒前的mic/dac/aec数据写卡进行查看,3channel(mic,dac,aec)
#define AEC_DATA_TO_SD      0
#if AEC_DATA_TO_SD
#define READSIZE            128
#define MIX_DATA_LEN        (128*2*2) //AEC READSIZE = 128, 3:channel, 2:s16
#define DAC_DATA_LEN        (128*2*3) //dac_sr/aec_sr*aec_points*channel  48000/16000 * 128 * 2
#endif

#define ASR_FEAME           256
#define ONCE_SR_POINTS      160
#define WANSON_BUF_SIZE     (24 * ONCE_SR_POINTS) //跑不过来时适当加大倍数

#define ASR_ENABLE          1

#if ASR_ENABLE
#include "asr_score.h"
#endif

char aisp_mic_channel_set = 0;

extern bool s_start_send;

extern int aisp_aec_gain;
extern int aisp_mic0_gain;
extern int aisp_mic1_gain;

void aisp_clear(char stop_net_music);
static volatile char ifly_send_pcm;

extern int keyword_start;
extern int keyword_wifi_enter_congfig;
extern char websockets_wait_recv_end;
extern const int speech_energy_min;
extern char speech_digital_vol_agc;

#ifdef WIFI_PCM_STREAN_SOCKET_ENABLE
static short wifi_stream_buf[ASR_FEAME * 2];//mic2 + mic2 + mic3 + aec
#else
static short *aec_out_buf = NULL;
#endif
/* Global Variables */
static struct {
    int pid;
    u32 sample_rate;
    int volatile run_flag;
    u8 volatile exit_flag;
    OS_SEM sem;
    s16 mic_buf[WANSON_BUF_SIZE * 2];
    void *mic_enc;
    cbuffer_t mic_cbuf;
    char last_key_supspend;
#if AEC_DATA_TO_SD
    FILE *fd;
    FILE *fd1;
    OS_SEM w_sem;
    s16 cache[READSIZE * 2];
    s16 cache2[READSIZE * 2];
    void *mic_cache_buf;
    void *aec_cache_buf;
    cbuffer_t save_mic_cbuf;
    cbuffer_t save_aec_cbuf;
#endif
    char aisp_open_init;
    char asr_open_init;
    struct server *server_handle;
} wanson_server;

#define __this (&wanson_server)
enum send_pcm_type {
    SEND_PCM_INI = 0,
    SEND_PCM_START,
    SEND_PCM_DOING,
    SEND_PCM_STOP,
    SEND_PCM_PAUSE,
};

unsigned char net_flag = 0;

/* Function Prototypes */
void aisp_resume(void);
static void net_timerloop();
static void enc_server_event_handler(void* priv, int argc, int* argv);
static int wanson_vfs_fwrite(void* file, void* data, u32 len);
static int wanson_vfs_fclose(void* file);
static void wanson_task(void* priv);

int aisp_mic_gain_set(unsigned char volume);
void aisp_wake(char index);
int aisp_aec_gain_set(unsigned char volume);

#if ASR_ENABLE
/* ASR Related Functions */
void asr_timer_cnt_clear(void);
static void asr_timer_loop(void);
static void asr_timer_init(void);
static int asr_is_timeout(unsigned int timeOut);
void asr_timerout_check();
static int get_wakeup_status();
static void set_wakeup_status(unsigned char value);
static int wanson_algorithm_init();
static int bt_a2dp_status_check();
static void asr_group_check();
#endif
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

int __attribute__((weak)) websockets_send_500ms_pcm_buf_push(char *buf, int len, int clear)
{
    return 0;
}
int __attribute__((weak)) websockets_send_pcm_buf_init(void)
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

/* VFS Operations Structure */
static const struct audio_vfs_ops wanson_vfs_ops = {
    .fwrite = wanson_vfs_fwrite,
    .fclose = wanson_vfs_fclose,
};

/* Function Implementations */

static void net_timerloop()
{
    net_flag = 1;
    printf("switch_net_config\n");
}

static void enc_server_event_handler(void* priv, int argc, int* argv)
{
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
    case AUDIO_SERVER_EVENT_END:
        break;
    case AUDIO_SERVER_EVENT_SPEAK_START:
        printf("AUDIO_SERVER_EVENT_SPEAK_START");
        break;
    case AUDIO_SERVER_EVENT_SPEAK_STOP:
        printf("AUDIO_SERVER_EVENT_SPEAK_STOP");
        break;
    default:
        break;
    }
}

static int wanson_vfs_fwrite(void* file, void* data, u32 len)
{
    cbuffer_t *cbuf = (cbuffer_t*)file;
    u32 wlen = cbuf_write(cbuf, data, len);

    if (wlen != len) {
        cbuf_clear(&__this->mic_cbuf);
        printf("*******************busy**********************\n");
    }

    if (file == (void *)&__this->mic_cbuf) {
        os_sem_set(&__this->sem, 0);
        os_sem_post(&__this->sem);
    }

    return len;
}

static int wanson_vfs_fclose(void* file)
{
    return 0;
}

#if ASR_ENABLE
/* ASR Implementation */
unsigned char wakeup_flag = 0; //Wakeup flag to indicate if the system is in wakeup state
static unsigned int asr_wakeup_tick_timer = 0; //Timer for tracking wakeup duration
#define VOICE_WAKEUP_TIMEOUT 10 // Timeout duration for wakeup in seconds

//Clear the ASR wakeup timer
void asr_timer_cnt_clear(void)
{
    asr_wakeup_tick_timer = 0; // Reset the timer
}

//ASR timer loop function
static void asr_timer_loop(void)
{
    static int cnt = 0; //Counter for timer ticks
    cnt++;
    if (cnt >= 10) { //Every second (assuming this function is called every 100 ms)
        cnt = 0;
        if (wakeup_flag) {
            asr_wakeup_tick_timer++; //Increment the wakeup timer if awake
        } else {
            asr_wakeup_tick_timer = 0; //Reset the timer if not awake
        }
        //printf("%d ", get_app_music_prompt_dec_status()); //Print the music prompt status
    }
}

//Initialize the ASR timer
static void asr_timer_init(void)
{
    asr_wakeup_tick_timer = 0;//Reset timer on init
    wakeup_flag = 0; //Set wakeup flag to inactive
    sys_s_hi_timer_add(NULL, asr_timer_loop, 100); //Add timer for periodic checks
}

//Check if the wakeup timer has timed out
static int asr_is_timeout(unsigned int timeOut)
{
    return asr_wakeup_tick_timer > timeOut ? 1 : 0; //Return 1 if timeout
}

//Check for ASR timeout condition
void asr_timerout_check()
{
    if (wakeup_flag && asr_is_timeout(VOICE_WAKEUP_TIMEOUT)) {//If awake  and timeout occurs
        asr_timer_cnt_clear(); //Clear the timer
        wakeup_flag = 0; //Reset wakeup flag
        printf("==================asr_timerout_check=======================\n");
        //tone_redemeplay((char*)"timerout.mp3");//Play timeout tone
    }
}

//Get the current wakeup status
static int get_wakeup_status()
{
    return wakeup_flag; //Return current wakeup flag status
}

//Set the wakeup status
static void set_wakeup_status(unsigned char value)
{
    wakeup_flag = value; //Update wakeup flag
}

//Initialize Wanson algorithm
static int wanson_algorithm_init()
{
    if (Wanson_ASR_Init() < 0) {
        return -1; //Initialization failed
    }
    Wanson_ASR_Reset(1); //Reset ASR Gruop

    //Update Wanson threshold
    extern int wanson_threshold_update(void* custom_wake_ptr, unsigned short custom_wake_size,
                                       void *custom_cmd_ptr, unsigned short custom_cmd_size);
    if (wanson_threshold_update(&wake_score, sizeof(wake_score), &cmd_score, sizeof(cmd_score)) < 0) {
        printf("wanson_threshold_update Failed!");
    }

    extern void wanson_score_display_set(unsigned char value);
    wanson_score_display_set(1); //Set score display
    return 0; //Success
}

//Check Bluetooth A2DP status
static int bt_a2dp_status_check()
{
    return (a2dp_get_status() == BT_MUSIC_STATUS_STARTING) ? 1 : 0; //Return 1 if bt music is starting
}

//Check ASR group status
static void asr_group_check()
{
    int wakeup_status = get_wakeup_status(); //Get current wakeup status
    int bt_status = bt_a2dp_status_check(); //Check Bluetooth status
    int current_index = wanson_kws_group_index_get(); //Get current KWS group index

    if (wakeup_status) {
        //If in wakeup state
        if (bt_status == 0 && current_index != 1) {
            Wanson_ASR_Reset(1); //Reset ASR to group 1
            printf("Wake up // Voice recognition packet switching: 1 !!!");
        }
    } else {
        //If not in wakeup state
        if (bt_status == 1) {
            if (current_index != 0) {
                Wanson_ASR_Reset(0); //Reset to group 0 only if music is playing
                printf("Wake down // Voice recognition packet switching: 0 !!!");
            }
        } else {

            if (current_index != 1) {
                Wanson_ASR_Reset(1); //Reset to group 1 if not awake
                printf("Wake down // Voice recognition packet switching: 1 !!!");
            }
        }
    }
}
#endif

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
static volatile char sys_vad_active = 0;//有效检测语音
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
        }
        sys_vad_active = 0;
    }
}
static void wanson_task(void* priv)
{
    volatile unsigned int t1 = 0;
    volatile char vad_active = 0;
    short buf[ASR_FEAME]; //Buffer for audio frames
    u32 mic_len; //Length of mic data
    aisp_resume();

#if ASR_ENABLE
    const char *text = NULL; //Pointer for recognized text
    float score = 0; //Recognition score
    //Initialize voice recognition
    if (wanson_algorithm_init()) {
        return; //Exit if initialization fails
    }
    Wanson_ASR_Reset(1); // Reset ASR Gruop
    printf("Wanson ASR started successfully\n");
    //Initialize ASR timer
    asr_timer_init();
#endif

    char opus = !strcmp(qyai_chat_audio_enc_format(), "opus");
    int ASR_FIRST_BEFORE_BYTES = __this->sample_rate * 2 / 2;//500ms
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

#if (ASR_USED_AVD == ASR_WEBRTC_AVD) || (ASR_USED_AVD == ASR_MY_AVD) || (ASR_USED_AVD == ASR_JL_AVD)
    void *vad_cbuf = sys_vad_init(__this->sample_rate, ONCE_SR_POINTS * 2 * 64);
    if (vad_cbuf) {
        sys_vad_create(NULL);
    } else {
        printf("->vad_cbuf init err\n");
    }
#endif
    __this->run_flag = 1;
    sys_vad_lock(0);
    sys_vad_clear(1);
    while (1) {
        if (__this->exit_flag) {
            break;
        }

#if ASR_ENABLE
        //Check for ASR timeout
        asr_timerout_check();
        //Check current ASR group status
        asr_group_check();
#endif

        if (!__this->run_flag) {
            os_sem_pend(&__this->sem, 0);
            continue;
        }
        if ((cbuf_get_data_size(&__this->mic_cbuf) < ASR_FEAME * 2)) {
            os_sem_pend(&__this->sem, 0);
            continue;
        }

        mic_len = cbuf_read(&__this->mic_cbuf, buf, sizeof(buf));
        if (!mic_len) {
            os_sem_pend(&__this->sem, 0);
            continue;
        }

#if ASR_ENABLE
        //Skip recognition if music prompt is playing
        //if (get_app_music_prompt_dec_status() == 1) {

        /*
        if (!net_music_play_stop_status()) {
            putchar('!'); //Indicate that recognition is skipped
            continue;
        }
        */

        //Voice recognition API call
        //printf("ASR_FEAME = %d\n",ASR_FEAME);
        if (1 == Wanson_ASR_Recog(buf, ASR_FEAME, &text, &score)) {
            printf("==================== ASR_Result: %s", text);
            if (strcmp(text, (const char *)"你好小艾") == 0 || strcmp(text, (const char *)"小艾小艾") == 0) {
                //Handle the wakeup command
                if (bt_a2dp_status_check()) {
                    printf("Wake up: music stop");
                    asr_timer_cnt_clear(); //Clear timer on wakeup
                    //user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PAUSE, 0, NULL); //Pause music
                }
                wakeup_flag = 1;
                asr_timer_cnt_clear();
                aisp_wake(0);
                //tone_redemeplay((char*)"wakeup.mp3");
            } else if (strcmp(text, (const char *)"播放音乐") == 0) {
                if (wakeup_flag) {
                    wakeup_flag = 0;
                    if (!bt_a2dp_status_check()) {
                        printf("Music: play");
                        //user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PLAY, 0, NULL); //Play music
                    }
                }
            } else if (strcmp(text, (const char *)"继续播放") == 0) {
                if (wakeup_flag) {
                    wakeup_flag = 0;
                    if (!bt_a2dp_status_check()) {
                        printf("Music: play");
                        //user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PLAY, 0, NULL);
                    }
                }
            } else if (strcmp(text, (const char *)"停止播放") == 0) {
                if (wakeup_flag) {
                    if (bt_a2dp_status_check()) {
                        printf("Music: stop");
                        asr_timer_cnt_clear();
                        //user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PAUSE, 0, NULL);
                    }
                }
            } else if (strcmp(text, (const char *)"上一首") == 0) {
                if (wakeup_flag) {
                    wakeup_flag = 0;
                    if (!bt_a2dp_status_check()) {
                        printf("Music: prev");
                        //user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PREV, 0, NULL);
                    }
                }
            } else if (strcmp(text, (const char *)"下一首") == 0) {
                if (wakeup_flag) {
                    wakeup_flag = 0;
                    if (!bt_a2dp_status_check()) {
                        printf("Music: next");
                        //user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_NEXT, 0, NULL);
                    }
                }
            } else {
                if (wakeup_flag) {
                    //asr_deal(text);
                }
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
                for (volatile int j = 0, idx = 0, vol_min = 0; j < ASR_FEAME; j++) {
                    if (buf[idx] >= speech_energy_min) { //mic1
                        max_vad_min = 1;//有效检测语音幅值
                        break;
                    }
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
        } else {
            wifi_pcm_stream_socket_send(buf, mic_len);
        }
#endif
#endif

#if (ASR_USED_AVD == ASR_WEBRTC_AVD) || (ASR_USED_AVD == ASR_MY_AVD) || (ASR_USED_AVD == ASR_JL_AVD)
        if (vad_cbuf) {
            sys_vad_send_msg(0x1, vad_cbuf, buf, mic_len);
        }
#endif

        if (first_500ms_buf && first_500ms_copy) {
            if ((first_500ms_len + mic_len) <= ASR_FIRST_BEFORE_BYTES) { //还没满buf则继续填满
                memcpy((char*)first_500ms_buf + first_500ms_len, (char*)buf, mic_len);
                first_500ms_len += mic_len;
            } else if (mic_len < ASR_FIRST_BEFORE_BYTES) { //满buf则继续更新最新数据在buf最后区域
                if (first_500ms_len > mic_len) {
                    memcpy((char*)first_500ms_buf, (char*)first_500ms_buf + mic_len, first_500ms_len - mic_len);
                    memcpy((char*)first_500ms_buf + first_500ms_len - mic_len, (char*)buf, mic_len);
                } else {
                    memcpy((char*)first_500ms_buf, (char*)buf, mic_len);
                    first_500ms_len = mic_len;
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
                    if (!qyai_audio_send_push((u8 *)buf, mic_len)) {
                        ifly_send_pcm = SEND_PCM_DOING;
                    }
                }
            } else if (ifly_send_pcm == SEND_PCM_DOING) {
                if (qyai_audio_send_push((u8 *)buf, mic_len) == -2) { //发送过程中缓存满且掉线
                    goto reinit;
                }
//                        sdfile_onefile_save_test(buf, mic_len, 0);//sd卡存储PCM
            } else if (ifly_send_pcm == SEND_PCM_STOP) {
                qyai_audio_send_push((u8 *)buf, mic_len);
//                        sdfile_onefile_save_test(buf, mic_len, 1);//sd卡存储PCM
                qyai_audio_send_end();
reinit:
                ifly_send_pcm = SEND_PCM_INI;
                first_500ms_len = 0;
                first_500ms_copy = true;

            } else if (!opus) {
                websockets_send_500ms_audio_buf_push(buf, mic_len, 0);//
            }
        }
    }

__exit:
    __this->run_flag = 0;
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
    if (__this->server_handle) {
        req.enc.cmd = AUDIO_ENC_GET_STATUS;
        server_request(__this->server_handle, AUDIO_REQ_ENC, &req);
        int status = req.enc.status;
        memset(&req, 0, sizeof(req));
        if (status == AUDIO_ENC_START && stop) {
            req.enc.cmd = AUDIO_ENC_PAUSE;
            server_request(__this->server_handle, AUDIO_REQ_ENC, &req);
            printf("-> AUDIO_ENC_PAUSE \n");
        } else if (status == AUDIO_ENC_PAUSE && stop == 0) {
            req.enc.cmd = AUDIO_ENC_START;
            server_request(__this->server_handle, AUDIO_REQ_ENC, &req);
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
void aisp_wake(char index)//0:小飞小飞，8：配网模式
{
    int msg[4] = {0};
    if (index == 8) {
        keyword_start = 1;
    }
    msg[0] = index;
    os_taskq_post_type("esr_queue_task", Q_USER, 1, msg);
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
                alow_enter_wifi_cfg = 0;
                if (ifly_send_pcm != SEND_PCM_PAUSE) {
                    //通知app挂起
                    if (__this->last_key_supspend != KEY_SUPSPEND) {
                        key.action = KEY_EVENT_LONG;
                        key.value = KEY_SUPSPEND;
                        if (!ai_speaker_app()) {
                            printf("-> ! ai_speaker_app\n");
                            key_event_notify(KEY_EVENT_FROM_USER, &key);
                            __this->last_key_supspend = KEY_SUPSPEND;
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

                    char hello_index = rand() % 5;
#ifdef CONFIG_LVGL_UI_ENABLE
                    lv_demo_ai_dialogue_start(hello_index);
#endif
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
            //printf("-> __this->last_key_supspend = %d \n",__this->last_key_supspend);
            extern int ai_speaker_app(void);
            websockets_close_request(1);
            if (__this->last_key_supspend != KEY_RESUM) {
                __this->last_key_supspend = KEY_RESUM;
                key.action = KEY_EVENT_LONG;
                key.value = KEY_RESUM;
                if (!ai_speaker_app()) {
                    key_event_notify(KEY_EVENT_FROM_USER, &key);
                }
            }
            break;
        case 21:
            //通知app挂起
            if (__this->last_key_supspend != KEY_SUPSPEND) {
                key.action = KEY_EVENT_LONG;
                key.value = KEY_SUPSPEND;
                if (!ai_speaker_app()) {
                    printf("-> ! ai_speaker_app\n");
                    key_event_notify(KEY_EVENT_FROM_USER, &key);
                    __this->last_key_supspend = KEY_SUPSPEND;
                }
            }
            break;
        case 22:
//                keyword_start = 0;
//                sys_vad_clear(0);
//                sys_vad_lock(1);
//                ifly_send_pcm = SEND_PCM_STOP;
//                http_tts_play_stop_force();
//                music_buf_play_set_stop();
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
void aisp_wake_no_notic(void)
{
    if (sys_connect_net_success() && !keyword_wifi_enter_congfig) {
        if (ifly_send_pcm != SEND_PCM_PAUSE) {
            //通知app挂起
            struct key_event key = {0};
            key.type = KEY_EVENT_USER;
            if (__this->last_key_supspend != KEY_SUPSPEND) {
                key.action = KEY_EVENT_LONG;
                key.value = KEY_SUPSPEND;
                if (!ai_speaker_app()) {
                    printf("-> ! ai_speaker_app\n");
                    key_event_notify(KEY_EVENT_FROM_USER, &key);
                    __this->last_key_supspend = KEY_SUPSPEND;
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

            music_buf_play_stop_all();
            music_buf_play_set_stop();
            http_tts_play_stop_force();
            //net_music_play_set_stop();
            websockets_free_lbuf_buf();
            music_buf_play_free_lbuf();
            websockets_close_request(1);
            music_buf_play_stop_waite();//等待关闭完成再播放提示音
            net_music_num_clear();
            led_eya_wake(1);//LED眼睛唤醒显示

            websockets_send_audio_buf_init();
            websockets_send_500ms_audio_buf_push(NULL, 0, 1);

            ifly_send_pcm = SEND_PCM_START;
            keyword_start = 1;
            pcm_buf_clear();
            sys_vad_lock(0);
            sys_vad_clear(1);
            websocket_client_thread_create(1);
            music_buf_play_accept(1);
        }
    }
}
static void key_vad_pcm_send_set(char start, char noice_note, int vad_end_enable)
{
    if (start) {
        printf("--->KAY VAD SPEAK_START\n");
        //通知app挂起
        struct key_event key = {0};
        key.type = KEY_EVENT_USER;
        if (__this->last_key_supspend != KEY_SUPSPEND) {
            key.action = KEY_EVENT_LONG;
            key.value = KEY_SUPSPEND;
            if (!ai_speaker_app()) {
                printf("-> ! ai_speaker_app\n");
                key_event_notify(KEY_EVENT_FROM_USER, &key);
                __this->last_key_supspend = KEY_SUPSPEND;
            }
            os_time_dly(10);
        }
        auto_sleep_check_clear();

        music_buf_play_accept(0);
        websockets_close_request(1);
        websockets_dialogue_timeout_del();

        music_buf_play_stop_all();
        music_buf_play_set_stop();
        http_tts_play_stop_force();
        websockets_free_lbuf_buf();
        music_buf_play_free_lbuf();
        music_buf_play_stop_waite();//等待关闭完成再播放提示音
        net_music_num_clear();
        led_eya_wake(1);//LED眼睛唤醒显示
        if (noice_note) {
            aisp_mic_gain_suspend();
            if (!music_play_hello(rand() % 5)) {
                music_play_waite();
            }
            aisp_mic_gain_resum();
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
    } else {
        if (ifly_send_pcm == SEND_PCM_DOING || ifly_send_pcm == SEND_PCM_START) {
            ifly_send_pcm = SEND_PCM_STOP;
        }
        printf("--->KAY VAD SPEAK_STOP\n");
        keyword_start = 0;
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
static void record_config()
{
    printf("jsyan record_config.\r\n");
    cbuf_init(&__this->mic_cbuf, __this->mic_buf, sizeof(__this->mic_buf));//原始音频buf
    os_sem_create(&__this->sem, 0);
    int ret = 0;
    ret = thread_fork("asr_task", 3, 3840, 0,  &__this->pid, wanson_task, __this);
    if (OS_NO_ERR != ret) {
        printf("create asr_task failed.\n");
    }
}
static void record_server_init()
{
    union audio_req req_mic = {0};

    if (!__this->server_handle) {
        __this->server_handle = server_open("audio_server", "enc");
        if (!__this->server_handle) {
            return;
        }
        server_register_event_handler(__this->server_handle, NULL, enc_server_event_handler);
    }
    aisp_resume();
}
int aisp_open(u16 sample_rate)
{
    if (__this->aisp_open_init) {
        printf("---------aisp_already open---------\n");
        aisp_clear(0);
        return 0;
    }
    __this->aisp_open_init = 1;
    __this->sample_rate = sample_rate;
    printf("---------aisp_open---------\n");

    if (!__this->asr_open_init) {
        __this->asr_open_init = 1;
        record_config();

        record_server_init();

        // 唤醒识别处理任务
        app_esr_take();

        //    aisp_clear(0);
    }
    record_server_init();
    return 0;
}

void aisp_suspend(void)
{
    union audio_req req = {0};

    if (!__this->run_flag || !__this->server_handle) {
        return;
    }

    printf("---------aisp_suspend---------\n");
    __this->run_flag = 0;
    req.enc.cmd = AUDIO_ENC_STOP;
    server_request(__this->server_handle, AUDIO_REQ_ENC, &req);

    memset(&req, 0, sizeof(req));
    req.enc.cmd = AUDIO_ENC_CLOSE;
    server_request(__this->server_handle, AUDIO_REQ_ENC, &req);

    cbuf_clear(&__this->mic_cbuf);
}

void aisp_resume(void)
{
    union audio_req req_mic = {0};

    if (__this->run_flag || !__this->server_handle) {
        return;
    }

    __this->run_flag = 1;

    // os_sem_set(&__this->sem, 0);
    // os_sem_post(&__this->sem);

    req_mic.enc.cmd = AUDIO_ENC_OPEN;
    req_mic.enc.sample_rate = __this->sample_rate;
    req_mic.enc.format = "pcm";

    //
#if AUDIO_RECORD_MIC_COUNT == 1
    req_mic.enc.channel = 1;
    if (aisp_mic_channel_set) {
        req_mic.enc.channel_bit_map = BIT(aisp_mic_channel_set);
    } else {
        req_mic.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL);
    }
//    req_mic.enc.frame_size = ONCE_SR_POINTS * 2 * 2;
#else
#error "no surpport mic channel 2"
#endif // AUDIO_RECORD_MIC_COUNT

    req_mic.enc.frame_size = ONCE_SR_POINTS * 2 * req_mic.enc.channel;
    req_mic.enc.sample_source = "mic";
    req_mic.enc.volume = CONFIG_AISP_MIC_ADC_GAIN;
    //
    req_mic.enc.vfs_ops = &wanson_vfs_ops;
    req_mic.enc.output_buf_len = req_mic.enc.frame_size * 5;
    req_mic.enc.file = (FILE *)&__this->mic_cbuf;


#ifdef CONFIG_AEC_ENC_ENABLE
    struct aec_s_attr aec_param = { 0 };
    req_mic.enc.aec_attr = &aec_param;
    req_mic.enc.aec_enable = kws_aec_init(&aec_param, req_mic.enc.sample_rate, 16000, 1, AEC_MODE_ADVANCE);

#if defined CONFIG_ALL_ADC_CHANNEL_OPEN_ENABLE// && defined CONFIG_AISP_LINEIN_ADC_CHANNEL && defined CONFIG_AEC_LINEIN_CHANNEL_ENABLE
    if (req_mic.enc.aec_enable) {
        aec_param.output_way = 1; //1:使用硬件回采 0:使用软件回采
        req_mic.enc.channel_bit_map |= BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL); //配置回采硬件通道
        if (CONFIG_AISP_MIC2_AEC_ADC_CHANNEL < CONFIG_AISP_MIC0_ADC_CHANNEL) {
            req_mic.enc.ch_data_exchange = 1; //如果回采通道使用的硬件channel比MIC通道使用的硬件channel靠前的话处理数据时需要交换一下顺序
        }
    }
#endif
#endif

    server_request(__this->server_handle, AUDIO_REQ_ENC, &req_mic);

#if defined CONFIG_AEC_ENC_ENABLE && !defined CONFIG_FPGA_ENABLE
    if (aec_param.output_way) {
#ifdef CONFIG_ALL_ADC_CHANNEL_OPEN_ENABLE
        extern void adc_multiplex_set_gain(const char* source, u8 channel_bit_map, u8 gain);
        adc_multiplex_set_gain("mic", BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL), CONFIG_AISP_LINEIN_ADC_GAIN * 2);
#endif
    }
#endif

    union audio_req req_vol = {0};
    //麦克风和参考信号增益分开设置
    if (aisp_mic_channel_set) {
        req_mic.enc.channel_bit_map = BIT(aisp_mic_channel_set);
    } else {
        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL);
    }
    req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
    req_vol.enc.volume = CONFIG_AISP_MIC_ADC_GAIN;
    server_request(__this->server_handle, AUDIO_REQ_ENC, &req_vol);

#if AUDIO_RECORD_MIC_COUNT == 2
    req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC1_ADC_CHANNEL);
    req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
    req_vol.enc.volume = CONFIG_AISP_MIC_ADC_GAIN;
    server_request(__this->server_handle, AUDIO_REQ_ENC, &req_vol);
#endif // AUDIO_RECORD_MIC_COUNT

#ifdef CONFIG_AEC_ENC_ENABLE
    memset(&req_vol, 0, sizeof(req_vol));
    req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
    req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
    req_vol.enc.volume = CONFIG_AISP_AEC_ADC_GAIN;
    server_request(__this->server_handle, AUDIO_REQ_ENC, &req_vol);
#endif
    //SFR(JL_ANA->DAA_CON0, 2,  2, 0x3);//设置DACVDD-3V
}
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
    if (__this->server_handle && __this->run_flag) {
        speech_digital_vol_agc_enable(0);//只有播放音乐才加数字增益
        if (aisp_mic_channel_set) {
            req_vol.enc.channel_bit_map = BIT(aisp_mic_channel_set);
        } else {
            req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL);
        }
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = CONFIG_AISP_MIC_ADC_GAIN;//恢复增益到默认
        server_request(__this->server_handle, AUDIO_REQ_ENC, &req_vol);

#if AUDIO_RECORD_MIC_COUNT == 2
        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC1_ADC_CHANNEL);//接近喇叭的MIC需要减5左右
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = CONFIG_AISP_MIC_ADC_GAIN;//恢复增益到默认
        server_request(__this->server_handle, AUDIO_REQ_ENC, &req_vol);
#endif

        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = CONFIG_AISP_AEC_ADC_GAIN;//恢复增益到默认
        server_request(__this->server_handle, AUDIO_REQ_ENC, &req_vol);
    }
    return 0;
}
int aisp_mic_gain_set(unsigned char volume)
{
    union audio_req req_vol = {0};

    if (__this->server_handle && __this->run_flag) {
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
        server_request(__this->server_handle, AUDIO_REQ_ENC, &req_vol);

#if AUDIO_RECORD_MIC_COUNT == 2
        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC1_ADC_CHANNEL);//接近喇叭的MIC需要减5左右
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = aisp_mic1_gain;//CONFIG_AISP_MIC_ADC_GAIN;
        server_request(__this->server_handle, AUDIO_REQ_ENC, &req_vol);
#endif // AUDIO_RECORD_MIC_COUNT

        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = aisp_aec_gain;
        server_request(__this->server_handle, AUDIO_REQ_ENC, &req_vol);
    }
    return 0;
}
int aisp_all_mic_gain_set(int volume, int aec_gian, int mic0_gian, int mic1_gian)
{
    union audio_req req_vol = {0};

    if (__this->server_handle && __this->run_flag && aec_gian) {
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
        server_request(__this->server_handle, AUDIO_REQ_ENC, &req_vol);

#if AUDIO_RECORD_MIC_COUNT == 2
        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC1_ADC_CHANNEL);//接近喇叭的MIC需要减5左右
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = mic1_gian;//CONFIG_AISP_MIC_ADC_GAIN;
        server_request(__this->server_handle, AUDIO_REQ_ENC, &req_vol);
#endif // AUDIO_RECORD_MIC_COUNT

        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = aec_gian;
        server_request(__this->server_handle, AUDIO_REQ_ENC, &req_vol);
    }
    return 0;
}
void aisp_close(void)
{
    union audio_req req = {0};
    if (!__this->server_handle || !__this->aisp_open_init) {
        return;
    }
    __this->aisp_open_init = 0;
    printf("---------aisp_close---------\n");
    aisp_suspend();
    server_close(__this->server_handle);
    __this->server_handle = NULL;

//    __this->asr_open_init = 0;
//    os_sem_post(&read_available_sem);
//    thread_kill(&g_ivwThreadID, KILL_WAIT);
//    ifly_engine_uninit();

    aisp_clear(0);
    printf("---------aisp_close ok---------\n");
}
#endif
