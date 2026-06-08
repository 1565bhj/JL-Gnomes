/*
* Copyright 2023 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef __TIMER_H_
#define __TIMER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

// 全局倒计时管理变量定义
#define MAX_TIMERS 10

// 倒计时项结构体
typedef struct {
    lv_obj_t *cont;          // 倒计时容器
    lv_obj_t *time_group;    // 时间标签组
    lv_span_t *time_span;    // 时间文本span
    lv_obj_t *switch_obj;    // 倒计时开关
    int index;               // 倒计时序号（1~10）
    bool is_dynamic;         // 是否为动态创建的倒计时
    int total_seconds;       // 总秒数
    int remaining_seconds;   // 剩余秒数
    bool is_running;         // 是否正在运行
    bool is_selected;        // 是否被选中（用于删除页面）
    bool is_enabled;         // 计时器开关状态（true:开启, false:关闭）
    lv_timer_t *countdown_timer; // 倒计时定时器（驱动秒数递减）

}TimerItem;

// 全局变量声明
extern int g_timer_count; // 当前倒计时总数
extern TimerItem g_timers[MAX_TIMERS]; // 倒计时数组

// 函数声明
void refresh_timer_page(lv_ui *ui); // 刷新倒计时主页面
void refresh_timer_del_page(lv_ui *ui); // 刷新倒计时删除页面
#ifdef APPLAYER_ENABLE
int start_timer_countdown(uint8_t hour, uint8_t min, uint8_t sec,bool enable); // 倒计时功能函数
#endif
#ifdef __cplusplus
}
#endif
#endif /* ALARM_H_ */
