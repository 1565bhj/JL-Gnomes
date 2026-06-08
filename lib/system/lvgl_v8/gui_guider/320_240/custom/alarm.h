/*
* Copyright 2023 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef __ALARM_H_
#define __ALARM_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"


// 全局闹钟管理结构体（跟踪所有闹钟，包括静态和动态）
#define MAX_ALARMS 10
typedef struct {
    lv_obj_t *switch_obj;    // 闹钟开关
    lv_obj_t *time_group; // 时间标签组（包含span）
    lv_span_t *time_span;    // 时间文本span
    lv_obj_t *repeat_group; // 重复标签组（包含span）
    lv_span_t *repeat_span;    // 重复文本span
    char time_text[10];      // 独立存储的时间文本（HH:MM格式）
    char repeat_text[50];    // 独立存储的重复设置文本
    bool weekdays_selected[7]; // 每个闹钟自己的星期选择状态（周一到周日）

    int index;               // 闹钟序号（1~10）
    bool is_dynamic;         // 是否为动态创建的闹钟（用于清除时区分）
    int repeat_status;       // 闹钟重复状态引用(0: 只响一次, 1: 每天, 2: 自定义)
    bool is_selected;        // 是否被选择用于删除
    bool is_enabled;         // 闹钟开关状态（true:开启, false:关闭）
}AlarmItem;

// 全局闹钟管理变量声明
extern int g_alarm_count; // 当前闹钟总数
extern AlarmItem g_alarms[MAX_ALARMS]; // 闹钟数组
extern bool g_weekdays_selected[7]; // 周一到周日的选中状态
extern int g_current_alarm_index; // 当前操作的闹钟索引
int get_alarm_count(void);


void alarm_event_setup(void);
void alarm_add_event_setup(void);
void alarm_del_event_setup(void);
void Dynamic_add_new_alarm(lv_ui *ui, int hour, int minute, int second, int status);
void refresh_alarm_del_page(lv_ui *ui);
void refresh_alarm_page(lv_ui *ui);
bool Voice_delete_alarm(int hour, int minute, int second); // 语音删除闹钟函数（秒参数会被忽略，仅使用小时和分钟进行匹配）

#ifdef __cplusplus
}
#endif
#endif /* ALARM_H_ */
