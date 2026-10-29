#ifndef KEY_EVENT_H
#define KEY_EVENT_H
#include "event/event.h"

#define     KEY_POWER           0
#define     KEY_PREV            1
#define     KEY_NEXT            2
#define     KEY_PLAY            3
#define     KEY_VOLUME_DEC      4
#define     KEY_VOLUME_INC      5
#define     KEY_MODE            6
#define     KEY_MENU            7
#define     KEY_ENC             8
#define     KEY_PHONE           9
#define     KEY_PHOTO           10

#define     KEY_F1              11
#define     KEY_OK              12
#define     KEY_CANCLE          13
#define     KEY_LEFT            14
#define     KEY_UP              15
#define     KEY_RIGHT           16
#define     KEY_DOWN            17
#define     KEY_MUTE            18
#define     KEY_COLLECT         19
#define     KEY_LOCAL           20

#define     KEY_ADD             21
#define     KEY_MINUS           22
#define     KEY_SUPSPEND        23
#define     KEY_RESUM           24
#define     KEY_CPU_RESET       25

#define     KEY_LOOP            26
#define     KEY_EXIT_LOOP       27

#define     KEY_0               30
#define     KEY_1               31
#define     KEY_2               32
#define     KEY_3               33
#define     KEY_4               34
#define     KEY_5               35
#define     KEY_6               36
#define     KEY_7               37
#define     KEY_8               38
#define     KEY_9               39

#define     NO_KEY              255

#define     KEY_K1              13
#define     KEY_K2              6
#define     KEY_K3              4
#define     KEY_K4              5
#define     KEY_K5              12
#define     KEY_K6              10
#define     KEY_K7              8
#define     KEY_K8              0

enum key_action {
    KEY_EVENT_CLICK = 0,
    KEY_EVENT_LONG,
    KEY_EVENT_HOLD,
    KEY_EVENT_UP,
    KEY_EVENT_DOUBLE_CLICK,
    KEY_EVENT_TRIPLE_CLICK,
    KEY_EVENT_FOURTH_CLICK,
    KEY_EVENT_FIRTH_CLICK,
    KEY_EVENT_SIX_CLICK,
    KEY_EVENT_MORE_CLICK,
    KEY_EVENT_USER,
    KEY_EVENT_MAX,
};

enum key_from {
    KEY_EVENT_FROM_KEY,
    KEY_EVENT_FROM_TWS,
    KEY_EVENT_FROM_USER,
};

/* ===================== 按键功能枚举 ===================== */
/* 短按功能 */
enum {
    KEY_SHORT_NONE = 0,
    KEY_SHORT_WAKEUP,          /* 短按唤醒 */
    KEY_SHORT_NEXT_SONG,       /* 短按下一首 */
    KEY_SHORT_PREV_SONG,       /* 短按上一首 */
    KEY_SHORT_LED_MODE,        /* 短按灯模式(开/关灯)切换 */
    KEY_SHORT_MODE_SWITCH,     /* 短按模式切换 */
    KEY_SHORT_MUSIC_SWITCH,    /* 短按暂停/播放音乐 */
};

/* 长按功能 */
enum {
    KEY_LONG_NONE = 0,
    KEY_LONG_POWER,            /* 长按开关机 */
    KEY_LONG_VOL_UP,           /* 长按音量加 */
    KEY_LONG_VOL_DOWN,         /* 长按音量减 */
    KEY_LONG_4G_WIFI,          /* 长按4G/WIFI模式 */
    KEY_LONG_NET_CFG,          /* 长按配网模式 */
    KEY_LONG_KEY_VAD,          /* 长按对话 */
    KEY_LONG_LED_ONOFF,        /* 长按开关灯 */
};

/* 双击功能 */
enum {
    KEY_DOUBLE_NONE = 0,
    KEY_DOUBLE_MODE_SWITCH,    /* 双击模式切换 */
    KEY_DOUBLE_NET_CFG,        /* 双击配网模式 */
};

/* 三击功能 */
enum {
    KEY_TRIPLE_NONE = 0,
    KEY_TRIPLE_NET_CFG,        /* 三击配网模式 */
};

/* 四击功能 */
enum {
    KEY_FOURTH_NONE = 0,
    KEY_FOURTH_FACTORY_TEST,   /* 四击厂测模式 */
    KEY_FOURTH_4G_WIFI,        /* 四击4G/WIFI模式 */
};

/* 五击功能 */
enum {
    KEY_FIVE_NONE = 0,
    KEY_FIVE_FACTORY_TEST,     /* 五击厂测模式 */
};

/* 六击以上功能 */
enum {
    KEY_MANY_NONE = 0,
    KEY_MANY_FACTORY_RESET,    /* 六击以上恢复出厂设置 */
};

struct key_event {
    u8 type;
    u8 action;
    u8 value;
    u16 msg;
};

struct key_event_msg_table {
    u8 key_value;
    u16 msg_list[8];
};

#ifdef __cplusplus
extern "C" {
#endif

void key_event_disable();
void key_event_enable();
void key_event_filter_hold_disable();
void key_event_filter_hold_enable();
void key_event_set_msg_table(const struct key_event_msg_table *table);
int key_event_notify(enum key_from from, struct key_event *event);
int key_event_send_msg(u16 msg);


/* 各按键事件对应的功能执行函数，返回 true 表示已处理，false 表示未处理 */
int key_do_short_action(int act);
int key_do_long_action(int last_key, int act);
int key_do_handup(int last_key, int act);
int key_do_double_action(int act);
int key_do_triple_action(int act);
int key_do_fourth_action(int act);
int key_do_five_action(int act);
int key_do_many_action(int act);

/* 按键事件统一入口：last_key 为上次事件编码，key_action 为事件类型，key_val 为按键值（KEY_1~KEY_6） */
int key_func_set(int last_key, int key_action, int key_val);

/* 1-2-3 组合按键检测，返回 true 表示触发恢复配置 */
int key_press_123(int action_value);

#ifdef __cplusplus
}
#endif

/*
struct key_event_msg_table music_key_table[] = {
    {
       .key_value   = KEY_POWER,
       .msg_list    = {MSG_MUSIC_PP, MSG_POWER_OFF },
    },
    {
       .key_value   = KEY_NEXT,
       .msg_list    = {MSG_MUSIC_PP, MSG_POWER_OFF },
    },
    {
        .key_value = NO_KEY;
    },
};
*/
#endif

