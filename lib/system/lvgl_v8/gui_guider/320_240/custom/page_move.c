#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdbool.h"
#include "gui_guider.h"
#include "custom.h"

// // 全局UI对象
// lv_ui guider_ui;

// 页面触摸事件相关变量
static int touch_start_x = 0;
static int touch_start_y = 0;
static int touch_end_x = 0;
static int touch_end_y = 0;
static bool is_dragging = false;

// 页面移动阈值（用于判断滑动方向）
#define SWIPE_THRESHOLD 50

// 页面滑动事件处理函数
static void page_swipe_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_point_t point;
    
    switch (code) {
        case LV_EVENT_PRESSED:
            // 记录触摸开始位置
            lv_indev_get_point(lv_indev_get_act(), &point);
            touch_start_x = point.x;
            touch_start_y = point.y;
            is_dragging = false;
            break;
            
        case LV_EVENT_PRESSING:
            // 检测是否正在拖动
            lv_indev_get_point(lv_indev_get_act(), &point);
            if (abs(point.x - touch_start_x) > SWIPE_THRESHOLD || abs(point.y - touch_start_y) > SWIPE_THRESHOLD) {
                is_dragging = true;
            }
            break;
            
        case LV_EVENT_RELEASED:
            // 记录触摸结束位置
            lv_indev_get_point(lv_indev_get_act(), &point);
            touch_end_x = point.x;
            touch_end_y = point.y;
            
            if (!is_dragging) {
                // 点击事件处理
                printf("页面点击事件: X=%d, Y=%d\n", touch_end_x, touch_end_y);
                return;
            }
            
            // 计算滑动距离
            int delta_x = touch_end_x - touch_start_x;
            int delta_y = touch_end_y - touch_start_y;
            
            // 判断滑动方向
            if (abs(delta_x) > abs(delta_y)) {
                // 水平滑动
                if (delta_x > SWIPE_THRESHOLD) {
                    // 向右滑动
                    printf("页面向右滑动\n");
                    switch_to_main_page();
                } else if (delta_x < -SWIPE_THRESHOLD) {
                    // 向左滑动
                    printf("页面向左滑动\n");
                    switch_to_main_page();
                 
                }
            } else {
                // 垂直滑动
                if (delta_y > SWIPE_THRESHOLD) {
                    // 向下滑动
                    printf("页面向下滑动\n");
                    switch_to_main_page();
                } else if (delta_y < -SWIPE_THRESHOLD) {
                    // 向上滑动
                    printf("页面向上滑动\n");
                    switch_to_main_page();
                }
            }
            
            is_dragging = false;
            break;
            
        default:
            break;
    }
}

// 为页面添加触摸事件
void add_page_touch_events(void) {
    // 为主页面添加触摸事件
    if (guider_ui.screen_main) {
        lv_obj_add_event_cb(guider_ui.screen_main, page_swipe_event_handler, LV_EVENT_ALL, NULL);
        printf("为主页面添加触摸事件\n");
    }
    
    // 为闹钟页面添加触摸事件
    if (guider_ui.screen_alarm) {
        lv_obj_add_event_cb(guider_ui.screen_alarm, page_swipe_event_handler, LV_EVENT_ALL, NULL);
        printf("为闹钟页面添加触摸事件\n");
    }
    
    // 为添加闹钟页面添加触摸事件
    if (guider_ui.screen_alarm_add) {
        lv_obj_add_event_cb(guider_ui.screen_alarm_add, page_swipe_event_handler, LV_EVENT_ALL, NULL);
        printf("为添加闹钟页面添加触摸事件\n");
    }
    
    // 为删除闹钟页面添加触摸事件
    if (guider_ui.screen_alarm_remove) {
        lv_obj_add_event_cb(guider_ui.screen_alarm_remove, page_swipe_event_handler, LV_EVENT_ALL, NULL);
        printf("为删除闹钟页面添加触摸事件\n");
    }
    
    // 为闹钟设置页面添加触摸事件
    if (guider_ui.screen_alarm_set) {
        lv_obj_add_event_cb(guider_ui.screen_alarm_set, page_swipe_event_handler, LV_EVENT_ALL, NULL);
        printf("为闹钟设置页面添加触摸事件\n");
    }
    
    // 为倒计时页面添加触摸事件
    if (guider_ui.screen_timer) {
        lv_obj_add_event_cb(guider_ui.screen_timer, page_swipe_event_handler, LV_EVENT_ALL, NULL);
        printf("为倒计时页面添加触摸事件\n");
    }
    
    // 为添加倒计时页面添加触摸事件
    if (guider_ui.screen_timer_add) {
        lv_obj_add_event_cb(guider_ui.screen_timer_add, page_swipe_event_handler, LV_EVENT_ALL, NULL);
        printf("为添加倒计时页面添加触摸事件\n");
    }
    
    // 为删除倒计时页面添加触摸事件
    if (guider_ui.screen_timer_remove) {
        lv_obj_add_event_cb(guider_ui.screen_timer_remove, page_swipe_event_handler, LV_EVENT_ALL, NULL);
        printf("为删除倒计时页面添加触摸事件\n");
    }
}

// 初始化页面移动功能
void page_move_init(void) {
    // 为所有页面添加触摸事件
    add_page_touch_events();
    printf("页面移动功能初始化完成\n");
}
