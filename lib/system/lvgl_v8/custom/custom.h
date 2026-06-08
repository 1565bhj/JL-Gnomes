/*
* Copyright 2023 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef __CUSTOM_H_
#define __CUSTOM_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

// 图标位置宏定义
#define WEATHER_ICON_X           110
#define WEATHER_ICON_Y           14
#define ALARM_ICON_X             220
#define ALARM_ICON_Y             19
#define WIFI_ICON_X              262
#define WIFI_ICON_Y              19
#define BLUETOOTH_ICON_X         242
#define BLUETOOTH_ICON_Y         19
#define BATTERY_ICON_X           288
#define BATTERY_ICON_Y           21


#ifdef CONFIG_LVGL_UI_ENABLE
#define APPLAYER_ENABLE 1
#endif


void custom_init(lv_ui *ui);

// 页面切换函数声明
void switch_to_alarm_page(void);
void switch_to_alarm_add_page(void);
void switch_to_alarm_set_page(void);
void switch_to_alarm_remove_page(void);
void switch_to_timer_page(void);
void switch_to_timer_add_page(void);
void switch_to_timer_remove_page(void);
// 音乐播放模式结构体
typedef struct {
    uint8_t mode_id;     // 模式ID
    char name[20];       // 模式名称
    char mode_status[20]; // 模式状态
    bool is_clicked;     // 是否点击
} music_play_mode_t;

// 定义三种播放模式的常量
#define MODE_NET_ID 0    // 网络模式ID
#define MODE_BT_ID 1     // 蓝牙模式ID
#define MODE_SD_ID 2     // SD卡模式ID

// 外部声明音乐播放模式变量
extern music_play_mode_t g_current_play_mode;

void switch_to_music_page(const char *source_type);
void switch_to_conversation_page(void);

// 会话消息设置函数声明
void conversation_set_ai_message(lv_ui *ui, const char *message);
void conversation_set_user_message(lv_ui *ui, const char *message);

// 事件设置函数声明
void music_event_setup(void);
void alarm_event_setup(void);
void alarm_add_event_setup(void);
void alarm_set_event_setup(void);
void alarm_del_event_setup(void);

void timer_event_setup(void);
void timer_add_event_setup(void);
void timer_del_event_setup(void);

// 其他函数声明
void refresh_timer_del_page(lv_ui *ui);
void Dynamic_add_new_timer(lv_ui *ui, int hour, int minute, int second);
void page_move_init(void);


#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */
