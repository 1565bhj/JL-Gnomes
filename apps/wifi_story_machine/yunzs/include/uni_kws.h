#ifndef __UNI_KWS_H__
#define __UNI_KWS_H__

#include "ual-aik-event.h"

#define WAKEUP_MODE 0
#define CMD_MODE 1

typedef void (*uni_ais_event_cb_f)(AikEvent event, void *args);

int uni_asr_init();
int uni_asr_process(void *data, int size);
void uni_mode_switch(int mode);
void uni_asr_deinit(void);
void uni_ais_event_set_cb(uni_ais_event_cb_f cbk);

typedef enum {
    WAKEUP_WORD,
    CMD_WORD,
} USER_WORD_TYPE;
// 离线词识别后的回调函数
typedef void (*user_kws_event_cb)(char* word, double score, USER_WORD_TYPE type, int is_oneshot);
void user_set_kws_event_cb(user_kws_event_cb ucb);

#endif
