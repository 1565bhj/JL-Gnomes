#ifndef _IFLY_AIUI_H_
#define _IFLY_AIUI_H_

#include "syscfg/syscfg_id.h"
//#include "ifly_aiui_net.h"
#include "fvad.h"
#include "ifly_aiui_api.h"
#include "ifly_aiui_sdk_common.h"

#define IFLY_AIUI_SIGNATURE 0x41495549 //AIUI

typedef struct tat_t_ifly_aiui_device_id {
    unsigned int v_signature;
    char a_deviceid[IFLY_AIUI_DEVICE_ID_LEN + 1];
} t_ifly_aiui_device_id;

typedef struct tat_t_ifly_aiui_rec_volume {
    unsigned int v_signature;
    unsigned char v_rec_mic0_vol;
    unsigned char v_rec_mic1_vol;
    unsigned char v_rec_linein_vol;
} t_ifly_aiui_rec_volume;

enum {
    CFG_AIUI_ITEM_DEVICE_ID = CFG_USER_DEFINE_BEGIN + 1,
    CFG_AIUI_ITEM_RECORD_VOL,
    CFG_AIUI_ITEM_MAX_PLAY_VOL,
    CFG_AIUI_ITEM_REBOOT_TYPE
};

//#define IFLY_AIUI_V2_FTR //长连接

#define IFLY_AIUI_APP_TASK_NAME "aiui_task"

#define CONFIG_CLOUD_VAD_ENABLE 1

enum IFLY_AIUI_SDK_EVENT {
    IFLY_AIUI_SPEAK_END = 0x01,
    IFLY_AIUI_MEDIA_END = 0x02,
    IFLY_AIUI_PLAY_PAUSE = 0x03,
    IFLY_AIUI_PREVIOUS_SONG = 0x04,
    IFLY_AIUI_NEXT_SONG = 0x05,
    IFLY_AIUI_VOLUME_CHANGE = 0X06,
    IFLY_AIUI_VOLUME_INCR = 0x07,
    IFLY_AIUI_VOLUME_DECR = 0x08,
    IFLY_AIUI_VOLUME_MUTE = 0x09,
    IFLY_AIUI_RECORD_START = 0x0a,
    IFLY_AIUI_RECORD_SEND = 0x0b,
    IFLY_AIUI_RECORD_STOP = 0x0c,
    IFLY_AIUI_VOICE_MODE = 0x0d,
    IFLY_AIUI_MEDIA_STOP = 0x0e,
    IFLY_AIUI_BIND_DEVICE = 0x0f,
    IFLY_AIUI_RECORD_ERR = 0x10,
    IFLY_AIUI_PICTURE_RECOG = 0x11,
    IFLY_AIUI_COLLECT_RESOURCE = 0x12,
    IFLY_AIUI_PLAY_CONTIUE = 0x13,
    IFLY_AIUI_SET_VOLUME = 0x14,
    IFLY_AIUI_SPEAK_START = 0x15,
    IFLY_AIUI_MEDIA_START = 0x16,
    IFLY_AIUI_RECORD_BREAK = 0x17,
    IFLY_AIUI_AI_MIX_PLAY_END = 0x18,
    IFLY_AIUI_SESSION_START = 0x19,
    IFLY_AIUI_SESSION_FINISH = 0x1a,
    IFLY_AIUI_LOCAL_PROMPT_END = 0x1b,
    IFLY_AIUI_WAKEUP_EVENT = 0x1c,
    IFLY_AIUI_QUIT = 0xff,
};


typedef enum {
    ORDERPLAY_MODE,  //orderPlay
    LOOPPLAY_MODE,   //loopPlay
    SINGLELOOP_MODE, //singleLoop
    RANDOMPLAY_MODE, //randomPlay
} PLAY_MODE;

typedef enum {
    STOP,
    PAUSE,
    PLAY
} PLAY_STATUS;


#define IFLY_AIUI_OS_TASKQ_POST(name, argc, ...)                   \
    do                                                             \
    {                                                              \
        int err = os_taskq_post(name, argc, __VA_ARGS__);          \
        if (err)                                                   \
        {                                                          \
            log_e("\n %s %d err = %d\n", __func__, __LINE__, err); \
        }                                                          \
    } while (0)


extern int ifly_aiui_app_init(void);
extern void ifly_aiui_app_uninit(void);
extern u8 get_ifly_aiui_msg_notify(void);
extern struct dui_var *get_dui_hdl(void);
extern const char *wifi_module_get_sta_ssid(void);
extern int get_update_data(const char *url);
extern int get_app_music_volume(void);
extern int get_app_music_total_time(void);
extern int get_dui_token(struct dui_para *para);
extern void ifly_aiui_media_music_mode_set(int mode);
extern int ifly_aiui_tone_tts_play(int tts_type);
extern const int ifly_aiui_media_get_playing_status(void);
extern void ifly_aiui_ai_media_audio_play(const char *url);
extern int ifly_aiui_net_music_play_next(void);
extern void ifly_aiui_media_set_playing_status(int value);


#endif
