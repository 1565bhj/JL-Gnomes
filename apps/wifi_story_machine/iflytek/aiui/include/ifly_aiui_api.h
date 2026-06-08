#ifndef _DUI_API_H_
#define _DUI_API_H_

#include "generic/typedef.h"
#include "generic/list.h"

#define ONE_CACHE_BUF_LEN      1344

#define SYS_USERMSG_EVENT           0x0020

typedef u32(*t_pfn_read_audio_pcm)(u8 *buf, u32 len);

typedef struct  {
    struct list_head entry;
    u8 buf[1344];
    int len;
    u32 sessionid;
} IFLY_AIUI_AUDIO_INFO;

enum {
    IFLY_AIUI_LOG_SWITCH_CLOUD = 0,
    IFLY_AIUI_LOG_SWITCH_SEND_AUDIO,

    IFLY_AIUI_LOG_SWITCH_INTERNAL_MAX = 16,
    IFLY_AIUI_LOG_SWITCH_MAX = 31,
};
enum ifly_ai_server_event {
    AI_EVENT_MIX_PLAY_END = 0x90,
    AI_EVENT_SESSION_START,
    AI_EVENT_SESSION_FINISH,
};

enum {
    AI_SERVER_EVENT_LOGIN = 0x80,
    AI_SERVER_EVENT_LOGOUT,
    AI_SERVER_EVENT_SEND_FAIL,
    AI_SERVER_EVENT_SESSION_FINISHED,
    AI_SERVER_EVENT_SESSION_STARTED,
    AI_SERVER_EVENT_NET_VAD,
};
int ifly_aiui_recorder_get_audio_pcm(u8 *pp_pcm, u32 vp_len);

extern void ifly_aiui_media_speak_play(const char *url);
extern void ifly_aiui_media_audio_play(const char *url);
extern void ifly_aiui_media_audio_continue_play(const char *url);
extern void ifly_aiui_media_audio_pause_play(const char *url);
extern void ifly_aiui_media_audio_stop_play(const char *url);
extern void ifly_aiui_volume_change_notify(int volume);
extern void ifly_aiui_media_audio_resume_play(void);
void ifly_aiui_media_audio_resume_play_ex(void);
extern void ifly_aiui_media_audio_prev_play(void);
extern void ifly_aiui_media_audio_next_play(void);
extern void ifly_aiui_event_notify(int event, void *arg);
extern int ifly_aiui_media_audio_play_seek(u32 time_elapse);
extern void ifly_aiui_vad_notify();
void ifly_aiui_wakeup_notify();
extern unsigned int ifly_aiui_get_sys_ms_count();
void ifly_aiui_login_notify();
void ifly_aiui_logout_notify();
void ifly_aiui_session_started_notify();
void ifly_aiui_session_finished_notify();
void ifly_aiui_set_log_switch(unsigned char vp_switch, unsigned char vp_on);
unsigned char ifly_aiui_get_log_switch(unsigned char vp_switch);
unsigned int ifly_aiui_get_sys_ms_count();
void ifly_aiui_key_notify(u8 vp_action, u8 vp_key_value);
void ifly_aiui_ivw_res_cb(int vp_res_id);
void ifly_aiui_reboot(int vp_reboot_type);

#endif
