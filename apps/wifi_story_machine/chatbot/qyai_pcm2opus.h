#ifndef __QYAI_AUDIO_H__
#define __QYAI_AUDIO_H__

#include "typedef.h"

/* 麦克风/AEC/唤醒链路：固定 PCM */
char *qyai_mic_pipeline_enc_format(void);

/* 云端上行编解码标识（opus / pcm） */
char *qyai_chat_audio_enc_format(void);

int qyai_audio_use_opus(void);

void qyai_audio_set_pcm_frame_bytes(u16 bytes);

int qyai_audio_send_init(void);
int qyai_audio_send_start(void);
int qyai_audio_send_push(u8 *buf, int len);
/* Opus 时写入 PCM 环形缓冲（任意长度，编码侧按 640B 取数） */
int qyai_audio_send_push_frames(u8 *buf, int len);
int qyai_audio_send_end(void);
void qyai_audio_send_abort(void);

#endif
