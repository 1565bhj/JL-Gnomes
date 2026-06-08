

#include "app_config.h"
#include "avctp_user.h"
#include "event.h"
#include "fn_asr.h"
#include "fs/fs.h"
#include "generic/circular_buf.h"
#include "gpio.h"
#include "log_macro.h"
#include "os/os_api.h"
#include "server/audio_server.h"
#include "server/server_core.h"
#include "system/timer.h"
#include "wifi/wifi_connect.h"
#undef TAG
#define TAG "WANSON_ASR"

// 做应用请参考以下两个分组的指令
// 分组0 主要用于在播放的情况下需要唤醒打断做识别，有较高的识别率
// Wanson_ASR_Reset(0);
//  小艾小艾

// 分组1 主要用于唤醒词识别后，音乐播放停止后用来做识别
// Wanson_ASR_Reset(1);
//  小艾小艾
//  播放音乐

#if (defined CONFIG_ASR_ALGORITHM) && (CONFIG_ASR_ALGORITHM == WANSON_ALGORITHM)

/*aec module enable bit define*/
////AEC模式,AEC_EN:BIT(0) NLP_EN:BIT(1) NS_EN:BIT(2) AGC_EN:BIT(4) DNS_EN:BIT(5)
#ifdef CONFIG_AEC_ENC_ENABLE

/*aec module enable bit define*/
#define AEC_EN BIT(0)
#define NLP_EN BIT(1)
#define ANS_EN BIT(2)

//// | ANS_EN
#define AEC_MODE_ADVANCE (AEC_EN | NLP_EN) //
#define AEC_MODE_REDUCE (NLP_EN | ANS_EN)
#define AEC_MODE_SIMPLEX (ANS_EN)

#endif

//========================================================================================
//                                     AEC_DATA_TO_SD
//========================================================================================
#define AEC_DATA_TO_SD 0 // 将唤醒前的mic/dac/aec数据写卡进行查看,3channel(mic,dac,aec)
#define TO_SD_CHANNEL_NUM 2

#if AEC_DATA_TO_SD
#define READSIZE 128

#if (TO_SD_CHANNEL_NUM == 2)
#define MIX_DATA_LEN (128 * 2 * 2) // AEC READSIZE = 128, 3:channel, 2:s16
#endif

#if (TO_SD_CHANNEL_NUM == 6)
#define MIX_DATA_LEN (128 * 2 * 6) // AEC READSIZE = 128, 3:channel, 2:s16
#endif

#endif

//========================================================================================
//                                      ASR PARA
//========================================================================================
#define ASR_FEAME 256
#define ONCE_SR_POINTS 160
#define WANSON_BUF_SIZE (24 * ONCE_SR_POINTS) // 跑不过来时适当加大倍数

#define ASR_ENABLE 1
#if ASR_ENABLE
#include "asr_score.h"
#endif

static struct {
    int pid;
    u32 sample_rate;
    int volatile run_flag;
    u8 volatile exit_flag;
    OS_SEM sem;

    s16 mic_buf[WANSON_BUF_SIZE * 2];
    void *mic_enc;
    cbuffer_t mic_cbuf;

#if AEC_DATA_TO_SD
    FILE *fd;
    FILE *fd1;
    // void* dac_orig_buf;
    // void* cache_buf;
    OS_SEM w_sem;
    // cbuffer_t save_cbuf;
    // cbuffer_t dac_save_cbuf;
#if (TO_SD_CHANNEL_NUM == 2)
    s16 cache[READSIZE * 2];
    s16 cache2[READSIZE * 2];
#endif
#if (TO_SD_CHANNEL_NUM == 6)
    s16 cache[READSIZE * 6];
    s16 cache2[READSIZE * 6];
#endif
    void *mic_cache_buf;
    void *aec_cache_buf;
    cbuffer_t save_mic_cbuf;
    cbuffer_t save_aec_cbuf;
#endif
} wanson_server;

#define __this (&wanson_server)


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

int  __attribute__((weak)) speech_digital_vol_agc_enable(char enable)
{
    return 0;
}
int  __attribute__((weak)) aisp_mic_gain_suspend(void)
{
    return 0;
}
int  __attribute__((weak)) aisp_mic_gain_resum(void)
{
    return 0;
}
int  __attribute__((weak)) aisp_mic_gain_set(unsigned char volume)
{
    return 0;
}
int __attribute__((weak)) aisp_all_mic_gain_set(int volume, int aec_gian, int mic0_gian, int mic1_gian)
{
    return 0;
}

void aisp_all_pause(char stop)
{

}
void aisp_clear(char stop_net_music)
{

}
void aisp_wake(char index)//0:小飞小飞，8：配网模式
{
}
void aisp_timeout_exit(void)
{

}
void aisp_app_resum(void)
{
}
void aisp_app_suspend(void)
{
}

extern int keyworld_start;
static volatile char ifly_send_pcm;
enum send_pcm_type{
    SEND_PCM_INI = 0,
    SEND_PCM_START,
    SEND_PCM_DOING,
    SEND_PCM_STOP,
    SEND_PCM_PAUSE,
};
int pcm_send_set_status(char start)
{
    if(start){
        if(ifly_send_pcm != SEND_PCM_START && ifly_send_pcm != SEND_PCM_DOING){
            keyworld_start = 1;
            ifly_send_pcm = SEND_PCM_START;
        }
    }else if(ifly_send_pcm == SEND_PCM_DOING || ifly_send_pcm == SEND_PCM_START){
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
    if(ifly_send_pcm != SEND_PCM_PAUSE){
        ifly_send_pcm = SEND_PCM_INI;
#if ASR_FIRST_BEFORE_EN
        first_500ms_len = 0;
        first_500ms_copy = ASR_FIRST_BEFORE_EN;
#endif
    }
}
//========================================================================================
//                                       AEC_DATA_TO_SD
//========================================================================================
#if AEC_DATA_TO_SD

#ifdef WIFI_PCM_STREAN_SOCKET_ENABLE
unsigned char net_flag = 0;
static void net_timerloop()
{
    net_flag = 1;
    WANSON_LOG_D("switch_net_config\n");
}
#endif

void aec_mix_data_to_sd()
{
    while (1) {
#ifndef WIFI_PCM_STREAN_SOCKET_ENABLE
        if (__this->fd && __this->fd1) {
            os_sem_pend(&__this->w_sem, 0);

#if (TO_SD_CHANNEL_NUM == 2)
            u8 data[MIX_DATA_LEN];
            cbuf_read(&__this->save_aec_cbuf, data, MIX_DATA_LEN);
            fwrite(data, MIX_DATA_LEN, 1, __this->fd1);
#endif

#if (TO_SD_CHANNEL_NUM == 6)
            u8 data[MIX_DATA_LEN];
            cbuf_read(&__this->save_mic_cbuf, data, MIX_DATA_LEN);
            fwrite(data, MIX_DATA_LEN, 1, __this->fd);
#endif
        } else {
        }
#else
        if (net_flag) {
            net_flag = 0;
            if (wifi_get_sta_connect_state() != WIFI_STA_NETWORK_STACK_DHCP_SUCC) {
                extern void app_music_net_config(void);
                app_music_net_config();
            }
        }

        if (wifi_get_sta_connect_state() == WIFI_STA_NETWORK_STACK_DHCP_SUCC) {
            os_sem_pend(&__this->w_sem, 0);
            putchar('a');
#if (TO_SD_CHANNEL_NUM == 2)
            u8 data[MIX_DATA_LEN];
            // cbuf_read(&__this->save_mic_cbuf, data, MIX_DATA_LEN);
            cbuf_read(&__this->save_aec_cbuf, data, MIX_DATA_LEN);
            wifi_pcm_stream_socket_send((u8 *)data, MIX_DATA_LEN);
#endif
        }
#endif
    }
}

void aec_mix_data_set_cb(s16 *data, int step)
{
#ifndef WIFI_PCM_STREAN_SOCKET_ENABLE
    if (__this->fd) {
#endif
        if (step == 1) { // !ch_data_exchange
            for (u32 i = 0; i < READSIZE; ++i) {

#if (TO_SD_CHANNEL_NUM == 2)
                __this->cache[2 * i] = data[2 * i];
                __this->cache[2 * i + 1] = data[2 * i + 1];
                __this->cache2[2 * i] = data[2 * i];
#endif

#if (TO_SD_CHANNEL_NUM == 6)
                __this->cache[6 * i] = data[2 * i];
                __this->cache[6 * i + 1] = data[2 * i + 1];
#endif
            }

#if (TO_SD_CHANNEL_NUM == 2)
            if (0 == cbuf_write(&__this->save_mic_cbuf, __this->cache, MIX_DATA_LEN)) {
                cbuf_clear(&__this->save_mic_cbuf);
                WANSON_LOG_D("error jlkws mic_data cbuf write full!");
            }
#endif
        } else if (step == 2) { // ch_data_exchange
            // for (u32 i = 0; i < READSIZE; ++i) {
            //     __this->cache[2 * i + 1] = data[i];
            // }
        }

        if (step == 3) { // aec_data

#if (TO_SD_CHANNEL_NUM == 2)
            for (u32 i = 0; i < READSIZE; ++i) {
                __this->cache2[2 * i + 1] = data[i];
            }

            if (0 == cbuf_write(&__this->save_aec_cbuf, __this->cache2, MIX_DATA_LEN)) {
                cbuf_clear(&__this->save_aec_cbuf);
                WANSON_LOG_D("error jlkws aec_data cbuf write full!");
            }
#endif

#if (TO_SD_CHANNEL_NUM == 6)
            for (u32 i = 0; i < READSIZE; ++i) {
                __this->cache[6 * i + 2] = data[i];
            }
            if (0 == cbuf_write(&__this->save_mic_cbuf, __this->cache, MIX_DATA_LEN)) {
                cbuf_clear(&__this->save_mic_cbuf);
                WANSON_LOG_D("error jlkws mic_data cbuf write full!");
            }
#endif
            os_sem_set(&__this->w_sem, 0);
            os_sem_post(&__this->w_sem);
            memset(__this->cache, 0, sizeof(__this->cache));
            memset(__this->cache2, 0, sizeof(__this->cache2));
        }

#ifndef WIFI_PCM_STREAN_SOCKET_ENABLE
    }
#endif
}

void aec_soft_mix_data_set_cb(s16 *data, int step)
{
#ifndef WIFI_PCM_STREAN_SOCKET_ENABLE
    if (__this->fd) {
#endif
        // mic data
        if (step == 1) {

            for (u32 i = 0; i < READSIZE; ++i) {
                __this->cache[2 * i] = data[i];
            }
            for (u32 i = 0; i < READSIZE; ++i) {
                __this->cache2[2 * i] = data[i];
            }
        }

        if (step == 2) {
            for (u32 i = 0; i < READSIZE; ++i) {
                __this->cache[2 * i + 1] = data[i];
            }
            if (0 == cbuf_write(&__this->save_mic_cbuf, __this->cache, MIX_DATA_LEN)) {
                cbuf_clear(&__this->save_mic_cbuf);
                WANSON_LOG_D("error jlkws mic_data cbuf write full!");
            }
        }

#ifndef WIFI_PCM_STREAN_SOCKET_ENABLE
    }
#endif
#ifndef WIFI_PCM_STREAN_SOCKET_ENABLE
    if (__this->fd1) {
#endif
        // dac data
        if (step == 3) {
            for (u32 i = 0; i < READSIZE; ++i) {
                __this->cache2[2 * i + 1] = data[i];
            }

            if (0 == cbuf_write(&__this->save_aec_cbuf, __this->cache2, MIX_DATA_LEN)) {
                cbuf_clear(&__this->save_aec_cbuf);
                WANSON_LOG_D("error jlkws aec_data cbuf write full!");
            }
            os_sem_set(&__this->w_sem, 0);
            os_sem_post(&__this->w_sem);
        }
        if (step == 4) {
            memset(__this->cache, 0, sizeof(__this->cache));
            memset(__this->cache2, 0, sizeof(__this->cache));
        }
#ifndef WIFI_PCM_STREAN_SOCKET_ENABLE
    }
#endif
}

void aisp_set_key_test()
{
    extern void aisp_suspend(void);
    aisp_suspend();

    if (__this->fd) {
        fclose(__this->fd);
        __this->fd = NULL;
    }

    if (__this->fd1) {
        fclose(__this->fd1);
        __this->fd1 = NULL;
    }
    WANSON_LOG_D(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>aisp_set_key_test\n");
}
#endif

//========================================================================================

// cb
static void enc_server_event_handler(void *priv, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
    case AUDIO_SERVER_EVENT_END:
        break;
    case AUDIO_SERVER_EVENT_SPEAK_START:
        WANSON_LOG_D("AUDIO_SERVER_EVENT_SPEAK_START");
        break;
    case AUDIO_SERVER_EVENT_SPEAK_STOP:
        WANSON_LOG_D("AUDIO_SERVER_EVENT_SPEAK_STOP");
        break;
    default:
        break;
    }
}

static int wanson_vfs_fwrite(void *file, void *data, u32 len)
{
    cbuffer_t *cbuf = (cbuffer_t *)file;

    u32 wlen = cbuf_write(cbuf, data, len);
    if (wlen != len) {
        cbuf_clear(&__this->mic_cbuf);
        WANSON_LOG_E("wanson_vfs_fwrite cbuf write full!");
    }

    if (file == (void *)&__this->mic_cbuf) {
        os_sem_set(&__this->sem, 0);
        os_sem_post(&__this->sem);
    }

    return len;
}

static int wanson_vfs_fclose(void *file)
{
    return 0;
}

static const struct audio_vfs_ops wanson_vfs_ops = {
    .fwrite = wanson_vfs_fwrite,
    .fclose = wanson_vfs_fclose,
};

///////////////////////////
// 语音识别

#if ASR_ENABLE

static unsigned char wakeup_flag = 0;             // Wakeup flag to indicate if the system is in wakeup state
static unsigned int asr_wakeup_tick_timer = 0;    // Timer for tracking wakeup duration
extern int get_app_music_prompt_dec_status(void); // Function to get music prompt decoder status
#define VOICE_WAKEUP_TIMEOUT 10                   // Timeout duration for wakeup in seconds

// Clear the ASR wakeup timer
static void asr_timer_cnt_clear(void)
{
    asr_wakeup_tick_timer = 0; // Reset the timer
}

// ASR timer loop function
static void asr_timer_loop(void)
{
    static int cnt = 0; // Counter for timer ticks
    cnt++;
    if (cnt >= 10) { // Every second (assuming this function is called every 100 ms)
        cnt = 0;
        if (wakeup_flag) {
            asr_wakeup_tick_timer++; // Increment the wakeup timer if awake
        } else {
            asr_wakeup_tick_timer = 0; // Reset the timer if not awake
        }
        // WANSON_LOG_D("%d ", get_app_music_prompt_dec_status()); // Print the music prompt status
    }
}

// Initialize the ASR timer
static void asr_timer_init(void)
{
    asr_wakeup_tick_timer = 0;                     // Reset timer on init
    wakeup_flag = 0;                               // Set wakeup flag to inactive
    sys_s_hi_timer_add(NULL, asr_timer_loop, 100); // Add timer for periodic checks
}

// Check if the wakeup timer has timed out
static int asr_is_timeout(unsigned int timeOut)
{
    return asr_wakeup_tick_timer > timeOut ? 1 : 0; // Return 1 if timeout
}

// Check for ASR timeout condition
void asr_timerout_check()
{
    if (wakeup_flag && asr_is_timeout(VOICE_WAKEUP_TIMEOUT)) { // If awake  and timeout occurs
        asr_timer_cnt_clear();                                 // Clear the timer
        wakeup_flag = 0;                                       // Reset wakeup flag
    }
}

// Get the current wakeup status
static int get_wakeup_status()
{
    return wakeup_flag; // Return current wakeup flag status
}

// Set the wakeup status
static void set_wakeup_status(unsigned char value)
{
    wakeup_flag = value; // Update wakeup flag
}

// Initialize Wanson algorithm
static int wanson_algorithm_init()
{
    if (Wanson_ASR_Init() < 0) {
        return -1; // Initialization failed
    }
    Wanson_ASR_Reset(1); // Reset ASR Gruop

    // Update Wanson threshold
    extern int wanson_threshold_update(void *custom_wake_ptr, unsigned short custom_wake_size, void *custom_cmd_ptr, unsigned short custom_cmd_size);
    if (wanson_threshold_update(&wake_score, sizeof(wake_score), &cmd_score, sizeof(cmd_score)) < 0) {
        WANSON_LOG_E("wanson_threshold_update Failed!"); // Log failure
    }

    extern void wanson_score_display_set(unsigned char value);
    wanson_score_display_set(1); // Set score display
    return 0;                    // Success
}

// Check Bluetooth A2DP status
static int bt_a2dp_status_check()
{
    return (a2dp_get_status() == BT_MUSIC_STATUS_STARTING) ? 1 : 0; // Return 1 if bt music is starting
}

// Check ASR group status
static void asr_group_check()
{
    int wakeup_status = get_wakeup_status();          // Get current wakeup status
    int bt_status = bt_a2dp_status_check();           // Check Bluetooth status
    int current_index = wanson_kws_group_index_get(); // Get current KWS group index

    if (wakeup_status) {
        // If in wakeup state
        if (bt_status == 0 && current_index != 1) {
            Wanson_ASR_Reset(1); // Reset ASR to group 1
            WANSON_LOG_D("Wake up // Voice recognition packet switching: 1 !!!");
        }
    } else {
        // If not in wakeup state
        if (bt_status == 1) {
            if (current_index != 0) {
                Wanson_ASR_Reset(0); // Reset to group 0 only if music is playing
                WANSON_LOG_D("Wake down // Voice recognition packet switching: 0 !!!");
            }
        } else {
            if (current_index != 1) {
                Wanson_ASR_Reset(1); // Reset to group 1 if not awake
                WANSON_LOG_D("Wake down // Voice recognition packet switching: 1 !!!");
            }
        }
    }
}
#endif

static void wanson_task(void *priv)
{
    short buf[ASR_FEAME]; // Buffer for audio frames
    u32 mic_len;          // Length of mic data

#if ASR_ENABLE
    const char *text = NULL; // Pointer for recognized text
    float score = 0;         // Recognition score
    // Initialize voice recognition
    if (wanson_algorithm_init()) {
        return; // Exit if initialization fails
    }
    WANSON_LOG_I("Wanson ASR started successfully\n");

    // Initialize ASR timer
    asr_timer_init();
#endif

#if AEC_DATA_TO_SD
#ifndef WIFI_PCM_STREAN_SOCKET_ENABLE
    extern int storage_device_ready(void);
    while (!storage_device_ready()) { // 等待sd文件系统挂载完成
        os_time_dly(1);
    }
    __this->fd = fopen("storage/sd0/C/mic.pcm", "w+");
    __this->fd1 = fopen("storage/sd0/C/aec.pcm", "w+");
#endif
    os_sem_create(&__this->w_sem, 0);

    __this->mic_cache_buf = malloc(1024 * 128);
    if (__this->mic_cache_buf == NULL) {
        goto __exit;
    }

    __this->aec_cache_buf = malloc(1024 * 128);
    if (__this->aec_cache_buf == NULL) {
        goto __exit;
    }

    cbuf_init(&__this->save_mic_cbuf, __this->mic_cache_buf, 1024 * 128);
    cbuf_init(&__this->save_aec_cbuf, __this->aec_cache_buf, 1024 * 128);
    WANSON_LOG_D(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>storage_device_ready ok\n");
#if WIFI_PCM_STREAN_SOCKET_ENABLE
    sys_s_hi_timerout_add(NULL, net_timerloop, 1000 * 10); // Add timer for periodic checks
#endif
    thread_fork("aec_mix_data_to_sd", 4, 10 * 1024, 0, 0, aec_mix_data_to_sd, NULL);
#endif

    extern void aisp_resume(void);
    aisp_resume();
    while (1) {

        if (__this->exit_flag) {
            break;
        }

#if ASR_ENABLE
        // Check for ASR timeout
        asr_timerout_check();
        // Check current ASR group status
        // 需要注意只要做打断的场景（喇叭有声音播放且要做唤醒）就需要切换到分组0（ Wanson_ASR_Reset(0)）
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
        if (mic_len != sizeof(buf)) {
            os_sem_pend(&__this->sem, 0);
            continue;
        }

#if ASR_ENABLE
        // Skip recognition if music prompt is playing
//        if (get_app_music_prompt_dec_status() == 1) {
//            putchar('!'); // Indicate that recognition is skipped
//            continue;
//        }

        // Voice recognition API call
//        printf("-->buf : %d\n", ASR_FEAME);
        if (1 == Wanson_ASR_Recog(buf, ASR_FEAME, &text, &score)) {
            WANSON_LOG_D(">>>>>>>>>>>>>>>>>>>>>>wakeup_word: %s\n", text); // Print recognized wakeup word
            if (strcmp(text, (const char *)"小艾小艾") == 0) {
                // Handle the wakeup command
                if (bt_a2dp_status_check()) {
                    WANSON_LOG_D("Wake up: music stop");
                    asr_timer_cnt_clear();                                      // Clear timer on wakeup
                    user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PAUSE, 0, NULL); // Pause music
                }

                wakeup_flag = 1;       // Set wakeup flag
                asr_timer_cnt_clear(); // Clear timer
            } else if (strcmp(text, (const char *)"播放音乐") == 0) {
                // Handle play music command
                if (wakeup_flag) {
                    wakeup_flag = 0; // Reset wakeup flag
                    if (!bt_a2dp_status_check()) {
                        WANSON_LOG_D("Music: play");
                        user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PLAY, 0, NULL); // Play music
                    }
                }
            } else if (strcmp(text, (const char *)"停止播放") == 0) {
                // Handle stop music command
                if (wakeup_flag) {
                    if (bt_a2dp_status_check()) {
                        WANSON_LOG_D("Music: stop");
                        asr_timer_cnt_clear();                                      // Clear timer
                        user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PAUSE, 0, NULL); // Pause music
                    }
                }
            }
        }
#endif
    }

__exit:
    __this->run_flag = 0;
}

int aisp_open(u16 sample_rate)
{ // 16000
    __this->exit_flag = 0;
    __this->mic_enc = server_open("audio_server", "enc");

    server_register_event_handler(__this->mic_enc, NULL, enc_server_event_handler);
    cbuf_init(&__this->mic_cbuf, __this->mic_buf, sizeof(__this->mic_buf));
    os_sem_create(&__this->sem, 0);
    __this->sample_rate = sample_rate;
    return thread_fork("aisp", 3, 3840 * 2, 0, &__this->pid, wanson_task, __this);
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
    req.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL);
    req.enc.frame_size = ONCE_SR_POINTS * 2 * req.enc.channel;
    req.enc.volume = CONFIG_AISP_MIC_ADC_GAIN;
    req.enc.sample_rate = __this->sample_rate;
    req.enc.format = "pcm";
    req.enc.sample_source = "mic";
    req.enc.vfs_ops = &wanson_vfs_ops;
    req.enc.output_buf_len = req.enc.frame_size * 5;
    req.enc.file = (FILE *)&__this->mic_cbuf;

#ifdef CONFIG_AEC_ENC_ENABLE
    struct aec_s_attr aec_param = {0};
    req.enc.aec_attr = &aec_param;

    extern int kws_aec_init(struct aec_s_attr * aec_param,
                            unsigned short sample_rate,
                            unsigned short ref_rate,
                            unsigned char output_way,
                            unsigned char enable_bit);
    req.enc.aec_enable = kws_aec_init(&aec_param, req.enc.sample_rate, 16000, 1, AEC_MODE_ADVANCE);

#if defined CONFIG_ALL_ADC_CHANNEL_OPEN_ENABLE && defined CONFIG_AISP_LINEIN_ADC_CHANNEL && defined CONFIG_AEC_LINEIN_CHANNEL_ENABLE
    if (req.enc.aec_enable) {
        aec_param.output_way = 1;                                       // 1:使用硬件回采 0:使用软件回采
        req.enc.channel_bit_map |= BIT(CONFIG_AISP_LINEIN_ADC_CHANNEL); // 配置回采硬件通道
        if (CONFIG_AISP_LINEIN_ADC_CHANNEL < CONFIG_PHONE_CALL_ADC_CHANNEL) {
            req.enc.ch_data_exchange = 1; // 如果回采通道使用的硬件channel比MIC通道使用的硬件channel靠前的话处理数据时需要交换一下顺序
        }
    }
#endif

#endif

    server_request(__this->mic_enc, AUDIO_REQ_ENC, &req);

#if defined CONFIG_AEC_ENC_ENABLE && !defined CONFIG_FPGA_ENABLE
    if (aec_param.output_way) {
#ifdef CONFIG_ALL_ADC_CHANNEL_OPEN_ENABLE
        extern void adc_multiplex_set_gain(const char *source, u8 channel_bit_map, u8 gain);
        adc_multiplex_set_gain("mic", BIT(CONFIG_AISP_LINEIN_ADC_CHANNEL), CONFIG_AISP_LINEIN_ADC_GAIN * 2);
#endif
    }
#endif
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
