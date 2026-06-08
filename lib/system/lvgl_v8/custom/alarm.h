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
    int repeat_status;       // 闹钟重复状态引用(0: 只响一次, 1: 每天, BIT(1)-BIT(7): 对应星期一到星期日)
    bool is_selected;        // 是否被选择用于删除
    bool is_enabled;         // 闹钟开关状态（true:开启, false:关闭）
} AlarmItem;

// 全局闹钟管理变量声明
extern int g_alarm_count; // 当前闹钟总数
extern AlarmItem g_alarms[MAX_ALARMS]; // 闹钟数组
extern bool g_weekdays_selected[7]; // 周一到周日的选中状态
extern int g_current_alarm_index; // 当前操作的闹钟索引
int get_alarm_count(void);

// 闹钟删除参数结构体
typedef struct {
    lv_ui *ui;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} alarm_delete_params_t;

// 闹钟添加参数结构体
typedef struct {
    lv_ui *ui;           // UI对象指针
    uint8_t hour;        // 小时
    uint8_t minute;      // 分钟
    uint8_t second;      // 秒
    int alarm_index;     // 闹钟索引
    uint8_t repeat_status;  // 重复状态
} alarm_add_params_t;

// 全局闹钟管理变量声明
#define WEEK_MONDAY    (1 << 1)  // BIT(1) = 0x02
#define WEEK_TUESDAY   (1 << 2)  // BIT(2) = 0x04
#define WEEK_WEDNESDAY (1 << 3)  // BIT(3) = 0x08
#define WEEK_THURSDAY  (1 << 4)  // BIT(4) = 0x10
#define WEEK_FRIDAY    (1 << 5)  // BIT(5) = 0x20
#define WEEK_SATURDAY  (1 << 6)  // BIT(6) = 0x40
#define WEEK_SUNDAY    (1 << 7)  // BIT(7) = 0x80

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
