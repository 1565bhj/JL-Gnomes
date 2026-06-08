/*
 * Copyright 2020 Unisound AI Technology Co., Ltd.
 * Author: Hao Peng
 * All Rights Reserved.
 */

#ifndef UAL_AIK_EVENT_H_
#define UAL_AIK_EVENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(4)
/* WARNING: 修改该文件后必须同步修改 jni/AikEvent.java! */
typedef enum AikEvent {
    AIK_EVENT_NONE = 0,
    AIK_EVENT_START,
    AIK_EVENT_STOP,
    AIK_EVENT_EXIT,
    /* 离线唤醒结果，args：type of AikEventKwsArgs */
    AIK_EVENT_KWS_WAKEUP = 10,
    /* 离线识别结果 */
    AIK_EVENT_KWS_COMMAND,
    /* 离线识别超时，args：NULL */
    AIK_EVENT_KWS_TIMEOUT,
    /* 心跳，Master模式上报，2s一次 */
    AIK_EVENT_HEARTBEAT,
    /* 学习事件结果 */
    AIK_EVENT_KWS_STUDY,
    /* 测试模式 */
    AIK_EVENT_TEST_END = 999,
    /* END */
    AIK_EVENT_END
} AikEvent;

/* JNI 接口会将 args 转换成 cJSON */
typedef struct AikEventKwsArgs {
    const char *word;
    int start_ms;
    int end_ms;
    int kws_index;
    int is_oneshot;
    double score;
} AikEventKwsArgs;
#pragma pack()

#ifdef __cplusplus
}
#endif

#endif  // UAL_AIK_EVENT_H_
