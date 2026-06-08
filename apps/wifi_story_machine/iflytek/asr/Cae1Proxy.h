#ifndef  __CAE1_PROXY_H__
#define  __CAE1_PROXY_H__

#include "app.h"

#define  AEC_FRAME_SIZE    (128)
#define  IVW_FRAME_SIZE    (320)

int createCAE(const char* vtn_cfg,
              ivw_res_cb ivw_cb,
              iat_audio_cb iat_cb,
              void *user_data);

int engineIsInit();

int writeAudioData(const void* audio_data, unsigned int audio_len);

int setWakeupNcm(int val);

void setRecordMode();

int destroyCAE();

const char *CAEVersion();

int cae1proxy_set_keyword_threshhold(int vp_keyword_id, int vp_threshold);
int cae1proxy_get_keyword_threshhold(int vp_keyword_id, int *pp_threshold);

#endif // __CAE1_PROXY_H__
