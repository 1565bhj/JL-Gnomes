#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdbool.h"
#include "gui_guider.h"
#include "custom.h"
#include "timer.h"

// 全局变量定义
int g_timer_count = 0; // 当前倒计时总数
TimerItem g_timers[MAX_TIMERS] = {0}; // 倒计时数组

// 函数原型声明
void Dynamic_add_new_timer(lv_ui *ui, int hour, int minute, int second);
static void timer_switch_event_handler(lv_event_t *e);
static void timer_countdown_cb(lv_timer_t *timer);

// 倒计时添加按钮点击事件处理函数
static void timer_add_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if (!ui || !ui->screen_timer_img_add) {
        return;
    }
    if (code == LV_EVENT_CLICKED) {
        // 切换到添加倒计时页面
        switch_to_timer_add_page();
    }

}

// 倒计时删除按钮点击事件处理函数
static void timer_del_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if (!ui || !ui->screen_timer_img_del) {
        return;
    }
    if (code == LV_EVENT_CLICKED) {
        // 进入删除页面时关闭所有倒计时开关
        for (int i = 0; i < g_timer_count; i++) {
            TimerItem *timer = &g_timers[i];
            // 停止计时器（如果正在运行）
            if (timer->countdown_timer) {
                lv_timer_del(timer->countdown_timer);
                timer->countdown_timer = NULL;
            }
            // 强制更新状态为关闭
            timer->is_running = false;
            timer->is_enabled = false;
            // 强制更新UI开关状态
            if (timer->switch_obj) {
                lv_obj_clear_state(timer->switch_obj, LV_STATE_CHECKED);
            }
            printf("-> 倒计时%d 已关闭\n", timer->index);
        }


        // 切换到删除倒计时页面
        switch_to_timer_remove_page();

    }
}

/**
 * 倒计时开关事件处理（控制倒计时启动/停止）
 */
static void timer_switch_event_handler(lv_event_t *e)
{
    lv_obj_t *sw = lv_event_get_target(e);
    bool is_checked = lv_obj_has_state(sw, LV_STATE_CHECKED);
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);

    // 遍历所有倒计时，匹配当前开关
    for (int i = 0; i < MAX_TIMERS; i++) {
        TimerItem *timer = &g_timers[i];
        if (timer->switch_obj != sw) {
            continue;    // 跳过不匹配的倒计时
        }

        // 校验倒计时时间文本有效性
        if (!timer->time_span || !timer->time_span->txt) {
            printf("-> 倒计时%d 时间文本无效，无法操作\n", timer->index);
            lv_obj_clear_state(sw, LV_STATE_CHECKED); // 强制关闭开关
            return;
        }

        // 解析时间文本（HH:MM:SS）
        int h = 0, m = 0, s = 0;
        if (sscanf(timer->time_span->txt, "%d:%d:%d", &h, &m, &s) != 3) {
            printf("-> 倒计时%d 时间格式错误（需HH:MM:SS），无法操作\n", timer->index);
            lv_obj_clear_state(sw, LV_STATE_CHECKED); // 强制关闭开关
            return;
        }
        // 停止现有定时器（如果存在）
        if (timer->countdown_timer) {
            lv_timer_del(timer->countdown_timer);
            timer->countdown_timer = NULL;
        }
        // 更新is_enabled状态
        timer->is_enabled = is_checked;
        printf("-> 倒计时%d开关状态更新为: %s\n", timer->index, is_checked ? "开启" : "关闭");

        // 启动/停止倒计时
        if (is_checked && !timer->is_running) {
            printf("-> 倒计时%d 开启\n", timer->index);
            // 若剩余时间为0（已结束），重置为初始时长
            if (timer->remaining_seconds <= 0) {
                timer->remaining_seconds = h * 3600 + m * 60 + s;
            }

            // 启动新定时器
            if (timer->remaining_seconds > 0) {
                timer->countdown_timer = lv_timer_create(timer_countdown_cb, 1000, (void*)(intptr_t)i);
                lv_timer_set_repeat_count(timer->countdown_timer, -1); // 无限重复
                timer->is_running = true;
                printf("-> 倒计时%d 已启动，剩余时间: %d秒\n", timer->index, timer->remaining_seconds);
            }
        } else if (!is_checked && timer->is_running) {
            printf("-> 倒计时%d 关闭\n", timer->index);
            timer->is_running = false;
            printf("-> 倒计时%d 已停止\n", timer->index);
        }
#ifdef APPLAYER_ENABLE
        start_timer_countdown(h, m, s, timer->is_running); // 传递参数倒计时
#endif
        break; // 匹配到目标倒计时，退出循环
    }
}

/**
 * 倒计时定时器回调函数（修复版，兼容不同LVGL v8版本）
 */
// 倒计时触发：异步处理（避免LVGL线程冲突）
static void async_timer_trigger_wrapper(void *data)
{

    int timer_idx = (int)(intptr_t)data;

    // 自动关闭到期倒计时
    if (timer_idx >= 0 && timer_idx < MAX_TIMERS) {
        TimerItem *item = &g_timers[timer_idx];

        // 停止定时器并断开关联
        if (item->countdown_timer) {
            lv_timer_del(item->countdown_timer);
            item->countdown_timer = NULL;
        }

        // 更新状态
        item->is_running = false;
        item->is_enabled = false;

        // 删除倒计时UI元素
        if (item->switch_obj) {
            lv_obj_del(item->switch_obj);      // 删除开关控件
            item->switch_obj = NULL;
        }
        if (item->time_group) {
            lv_obj_del(item->time_group);      // 删除时间标签组
            item->time_group = NULL;
        }
        if (item->cont) {
            lv_obj_del(item->cont);            // 删除整个容器
            item->cont = NULL;
        }

        // 清空计时器数据
        memset(item, 0, sizeof(TimerItem));
        g_timer_count--;                  // 减少计时器总数

        // 添加倒计时重排逻辑
        if (g_timer_count > 0) {
            // 如果还有倒计时，重新刷新页面以重排
            refresh_timer_page(&guider_ui);
            refresh_timer_del_page(&guider_ui);
        }
    }

}

static void timer_countdown_cb(lv_timer_t *timer)
{
    // 直接访问timer结构体的user_data成员（兼容所有LVGL v8版本）
    int timer_idx = (int)(intptr_t)timer->user_data;
    TimerItem *item = &g_timers[timer_idx];

    // 检查剩余时间有效性
    if (item->remaining_seconds <= 0) {
        // 记录日志
        printf("-> 倒计时%d 已结束\n", item->index);

        // 使用异步调用处理UI更新和定时器清理，避免线程冲突
        lv_async_call(async_timer_trigger_wrapper, (void*)(intptr_t)timer_idx);
        return;
    }

    // 减少剩余时间并更新显示
    item->remaining_seconds--;
    int hour = item->remaining_seconds / 3600;
    int remaining = item->remaining_seconds % 3600;
    int minute = remaining / 60;
    int second = remaining % 60;

    // 更新时间文本
    char time_str[9];
    sprintf(time_str, "%02d:%02d:%02d", hour, minute, second);
    lv_span_set_text(item->time_span, time_str);

    // 同时刷新删除页面，确保倒计时同步显示
    refresh_timer_del_page(&guider_ui);
}
/**
 * 动态添加新倒计时（倒计时都通过此函数动态创建，共支持10个）
 * @param ui: GUI全局对象
 * @param hour: 选中的小时
 * @param minute: 选中的分钟
 * @param second: 选中的秒
 */
void Dynamic_add_new_timer(lv_ui *ui, int hour, int minute, int second)
{
    // 1. 检查是否达到最大倒计时数
    if (g_timer_count >= MAX_TIMERS) {
        printf("-> 已达到最大倒计时数量(%d个)\n", MAX_TIMERS);
        return;
    }

    // 2. 计算新倒计时的位置（从1开始计数，Y轴间隔51px）
    int new_timer_idx = g_timer_count + 1; // 新倒计时序号（1~10）
    int y_pos = 0 + (new_timer_idx - 1) * 61; // Y轴位置
    printf("-> 正在添加新倒计时%d，位置Y: %d\n", new_timer_idx, y_pos);

    // 3. 动态创建timer_cont容器（倒计时条目容器）
    lv_obj_t *timer_cont = lv_obj_create(ui->screen_timer_cont);
    lv_obj_set_pos(timer_cont, 2, y_pos);
    lv_obj_set_size(timer_cont, 258, 51);
    // 使用user_data标记这是动态创建的倒计时条目
    lv_obj_set_user_data(timer_cont, (void*)1);

    // 设置容器样式（与静态创建的容器样式一致）
    static lv_style_t cont_style;
    lv_style_init(&cont_style);
    lv_style_set_border_width(&cont_style, 2);
    lv_style_set_border_opa(&cont_style, 0);
    lv_style_set_border_color(&cont_style, lv_color_hex(0xffffff));
    lv_style_set_border_side(&cont_style, LV_BORDER_SIDE_FULL);
    lv_style_set_radius(&cont_style, 5);  // 直接设置半径为5，与静态容器一致
    lv_style_set_bg_opa(&cont_style, 53);
    lv_style_set_bg_color(&cont_style, lv_color_hex(0xffffff));
    lv_style_set_bg_grad_dir(&cont_style, LV_GRAD_DIR_NONE);
    lv_style_set_pad_top(&cont_style, 0);
    lv_style_set_pad_bottom(&cont_style, 0);
    lv_style_set_pad_left(&cont_style, 0);
    lv_style_set_pad_right(&cont_style, 0);
    lv_style_set_shadow_width(&cont_style, 0);
    lv_obj_add_style(timer_cont, &cont_style, 0);

    // 4. 动态创建时间标签组
    lv_obj_t *time_group = lv_spangroup_create(timer_cont);
    lv_spangroup_set_align(time_group, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(time_group, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(time_group, LV_SPAN_MODE_EXPAND);

    // 设置时间标签组位置和大小（与静态创建的时间标签一致）
    lv_obj_set_pos(time_group, 12, 9);
    lv_obj_set_size(time_group, 57, 25);

    // 创建时间文本span
    lv_span_t *time_span = lv_spangroup_new_span(time_group);
    // 设置时间文本格式
    char time_str[9];
    sprintf(time_str, "%02d:%02d:%02d", hour, minute, second);
    lv_span_set_text(time_span, time_str);

    // 设置时间文本样式
    lv_style_set_text_font(&time_span->style, &lv_font_Barlow__28);
    lv_style_set_text_color(&time_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&time_span->style, LV_TEXT_DECOR_NONE);

    // 设置span组主样式
    static lv_style_t span_group_style;
    lv_style_init(&span_group_style);
    lv_style_set_border_width(&span_group_style, 0);
    lv_style_set_radius(&span_group_style, 0);
    lv_style_set_bg_opa(&span_group_style, 0);
    lv_style_set_pad_top(&span_group_style, 0);
    lv_style_set_pad_right(&span_group_style, 0);
    lv_style_set_pad_bottom(&span_group_style, 0);
    lv_style_set_pad_left(&span_group_style, 0);
    lv_style_set_shadow_width(&span_group_style, 0);
    lv_obj_add_style(time_group, &span_group_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(time_group);

    // 5. 动态创建开关控件
    lv_obj_t *sw = lv_switch_create(timer_cont);
    lv_obj_set_pos(sw, 204, 16);
    lv_obj_set_size(sw, 35, 20);

    // 设置开关主样式（默认状态）
    static lv_style_t switch_main_style;
    lv_style_init(&switch_main_style);
    lv_style_set_bg_opa(&switch_main_style, 255);
    lv_style_set_bg_color(&switch_main_style, lv_color_hex(0xe6e2e6));
    lv_style_set_bg_grad_dir(&switch_main_style, LV_GRAD_DIR_NONE);
    lv_style_set_border_width(&switch_main_style, 0);
    lv_style_set_radius(&switch_main_style, 10);
    lv_style_set_shadow_width(&switch_main_style, 0);
    lv_obj_add_style(sw, &switch_main_style, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 设置开关INDICATOR样式（选中状态）
    static lv_style_t switch_indicator_style;
    lv_style_init(&switch_indicator_style);
    lv_style_set_bg_opa(&switch_indicator_style, 255);
    lv_style_set_bg_color(&switch_indicator_style, lv_color_hex(0x2195f6));
    lv_style_set_bg_grad_dir(&switch_indicator_style, LV_GRAD_DIR_NONE);
    lv_style_set_border_width(&switch_indicator_style, 0);
    lv_obj_add_style(sw, &switch_indicator_style, LV_PART_INDICATOR | LV_STATE_CHECKED);

    // 设置开关KNOB样式
    static lv_style_t switch_knob_style;
    lv_style_init(&switch_knob_style);
    lv_style_set_bg_opa(&switch_knob_style, 255);
    lv_style_set_bg_color(&switch_knob_style, lv_color_hex(0xffffff));
    lv_style_set_bg_grad_dir(&switch_knob_style, LV_GRAD_DIR_NONE);
    lv_style_set_border_width(&switch_knob_style, 0);
    lv_style_set_radius(&switch_knob_style, 10);
    lv_obj_add_style(sw, &switch_knob_style, LV_PART_KNOB | LV_STATE_DEFAULT);

    // 6. 更新全局倒计时数组（记录动态倒计时信息）
    int total_seconds = hour * 3600 + minute * 60 + second;
    g_timers[new_timer_idx - 1] = (TimerItem) {
        .cont = timer_cont,
        .switch_obj = sw,
        .time_group = time_group,
        .time_span = time_span,
        .index = new_timer_idx,
        .is_dynamic = true, // 标记为动态倒计时
        .total_seconds = total_seconds,
        .remaining_seconds = total_seconds, // 初始剩余时间等于总时间
        .is_running = false, // 初始状态为未运行
        .is_selected = false,  // 初始化选择状态为未选中
        .is_enabled = true,     // 初始化开关状态为开启
        .countdown_timer = NULL
    };
    // 创建并启动计时器
    TimerItem *timer = &g_timers[new_timer_idx - 1];
    timer->countdown_timer = lv_timer_create(timer_countdown_cb, 1000, (void*)(intptr_t)(new_timer_idx - 1));
    lv_timer_set_repeat_count(timer->countdown_timer, -1);
    // 7. 增加当前倒计时总数
    g_timer_count++;
    printf("-> 新倒计时%d添加完成，当前总数：%d\n", new_timer_idx, g_timer_count);

    // 8. 设置新倒计时开关状态为选中（默认状态）
    lv_obj_add_state(sw, LV_STATE_CHECKED);
    printf("-> 新倒计时%d的UI开关默认选中\n", new_timer_idx);

    // 绑定开关事件处理函数，传递UI指针作为user_data
    lv_obj_add_event_cb(sw, timer_switch_event_handler, LV_EVENT_VALUE_CHANGED, ui);
    refresh_timer_del_page(ui);
}
// 定义添加计时器的参数结构体
typedef struct {
    lv_ui *ui;       // UI对象指针
    uint8_t hour;    // 小时
    uint8_t minute;  // 分钟
    uint8_t second;  // 秒
} timer_add_params_t;

// 异步动态添加新计时器UI函数
static void async_ui_dynamic_add_new_timer(void* param)
{
    if (!param) {
        return;
    }

    // 解析参数
    timer_add_params_t *timer_params = (timer_add_params_t *)param;

    // 根据参数创建新计时器
    Dynamic_add_new_timer(timer_params->ui, timer_params->hour, timer_params->minute, timer_params->second);

    // 释放参数内存
    free(param);
}

// 同步调用异步添加计时器的UI函数
void Sync_ui_dynamic_add_new_timer(lv_ui *ui, uint8_t hour, uint8_t minute, uint8_t second)
{
    // 创建并初始化参数结构体
    timer_add_params_t *params = (timer_add_params_t *)malloc(sizeof(timer_add_params_t));
    if (params) {
        params->ui = ui;
        params->hour = hour;
        params->minute = minute;
        params->second = second;

        // 异步调用函数
        lv_async_call(async_ui_dynamic_add_new_timer, (void *)params);
    }
}
// 异步动态添加新计时器应用层函数
static void async_app_dynamic_add_new_timer(void* param)
{
    if (!param) {
        return;
    }

    // 解析参数
    timer_add_params_t *timer_params = (timer_add_params_t *)param;

#ifdef APPLAYER_ENABLE
    // 启动倒计时
    start_timer_countdown(timer_params->hour, timer_params->minute, timer_params->second, true);
#endif

    // 释放参数内存
    free(param);
}

// 同步调用异步添加计时器的应用层函数
void Sync_app_dynamic_add_new_timer(lv_ui *ui, uint8_t hour, uint8_t minute, uint8_t second)
{
    // 创建并初始化参数结构体
    timer_add_params_t *params = (timer_add_params_t *)malloc(sizeof(timer_add_params_t));
    if (params) {
        params->ui = ui;
        params->hour = hour;
        params->minute = minute;
        params->second = second;

        // 异步调用函数
        lv_async_call(async_app_dynamic_add_new_timer, (void *)params);
    }
}

void Voice_dynamic_add_new_timer(int timeout)
{
    int hour = timeout / 3600;
    int minute = (timeout % 3600) / 60;
    int second = timeout % 60;
    Sync_ui_dynamic_add_new_timer(&guider_ui, hour, minute, second);
}
/**
 * 刷新倒计时主页面
 * @param ui: GUI全局对象
 */
void refresh_timer_page(lv_ui *ui)
{
    // 1. 清除当前所有动态创建的倒计时条目
    lv_obj_t *cont = ui->screen_timer_cont;
    int i = 0;
    lv_obj_t *child;
    while ((child = lv_obj_get_child(cont, i)) != NULL) {
        // 检查当前子对象是否需要删除（通过user_data标记）
        if (lv_obj_get_user_data(child) == (void *)1) {
            lv_obj_del(child);
            // 删除后索引不变，因为下一个子对象会移动到当前位置
        } else {
            i++;
        }
    }

    // 2. 遍历全局倒计时数组，重新创建倒计时条目
    for (int i = 0; i < g_timer_count; i++) {
        // 计算新倒计时条目的Y轴位置（从1开始计数，Y轴间隔61px）
        int y_pos = 0 + i * 61; // Y轴位置

        // 创建timer_cont容器（倒计时条目容器）
        lv_obj_t *timer_cont = lv_obj_create(ui->screen_timer_cont);
        lv_obj_set_pos(timer_cont, 2, y_pos);
        lv_obj_set_size(timer_cont, 258, 51);
        // 使用user_data标记这是动态创建的倒计时条目
        lv_obj_set_user_data(timer_cont, (void*)1);

        // 设置容器样式（与静态创建的容器样式一致）
        static lv_style_t cont_style;
        lv_style_init(&cont_style);
        lv_style_set_border_width(&cont_style, 2);
        lv_style_set_border_opa(&cont_style, 0);
        lv_style_set_border_color(&cont_style, lv_color_hex(0xffffff));
        lv_style_set_border_side(&cont_style, LV_BORDER_SIDE_FULL);
        lv_style_set_radius(&cont_style, 5);
        lv_style_set_bg_opa(&cont_style, 53);
        lv_style_set_bg_color(&cont_style, lv_color_hex(0xffffff));
        lv_style_set_bg_grad_dir(&cont_style, LV_GRAD_DIR_NONE);
        lv_style_set_pad_top(&cont_style, 0);
        lv_style_set_pad_bottom(&cont_style, 0);
        lv_style_set_pad_left(&cont_style, 0);
        lv_style_set_pad_right(&cont_style, 0);
        lv_style_set_shadow_width(&cont_style, 0);
        lv_obj_add_style(timer_cont, &cont_style, 0);

        // 创建时间标签组
        lv_obj_t *time_group = lv_spangroup_create(timer_cont);
        lv_spangroup_set_align(time_group, LV_TEXT_ALIGN_LEFT);
        lv_spangroup_set_overflow(time_group, LV_SPAN_OVERFLOW_CLIP);
        lv_spangroup_set_mode(time_group, LV_SPAN_MODE_EXPAND);

        // 设置时间标签组位置和大小
        lv_obj_set_pos(time_group, 12, 9);
        lv_obj_set_size(time_group, 57, 25);

        // 创建时间文本span
        lv_span_t *time_span = lv_spangroup_new_span(time_group);

        // 格式化倒计时时间（时:分:秒）- 使用remaining_seconds显示真实剩余时间
        int hours = g_timers[i].remaining_seconds / 3600;
        int minutes = (g_timers[i].remaining_seconds % 3600) / 60;
        int seconds = g_timers[i].remaining_seconds % 60;
        char time_str[9];
        sprintf(time_str, "%02d:%02d:%02d", hours, minutes, seconds);
        lv_span_set_text(time_span, time_str);

        // 设置时间文本样式
        lv_style_set_text_font(&time_span->style, &lv_font_Barlow__28);
        lv_style_set_text_color(&time_span->style, lv_color_hex(0xffffff));
        lv_style_set_text_decor(&time_span->style, LV_TEXT_DECOR_NONE);

        // 设置span组主样式
        static lv_style_t span_group_style;
        lv_style_init(&span_group_style);
        lv_style_set_border_width(&span_group_style, 0);
        lv_style_set_radius(&span_group_style, 0);
        lv_style_set_bg_opa(&span_group_style, 0);
        lv_style_set_pad_top(&span_group_style, 0);
        lv_style_set_pad_right(&span_group_style, 0);
        lv_style_set_pad_bottom(&span_group_style, 0);
        lv_style_set_pad_left(&span_group_style, 0);
        lv_style_set_shadow_width(&span_group_style, 0);
        lv_obj_add_style(time_group, &span_group_style, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_spangroup_refr_mode(time_group);

        // 创建开关控件
        lv_obj_t *sw = lv_switch_create(timer_cont);
        lv_obj_set_pos(sw, 204, 16);
        lv_obj_set_size(sw, 35, 20);

        // 设置开关主样式（默认状态）
        static lv_style_t switch_main_style;
        lv_style_init(&switch_main_style);
        lv_style_set_bg_opa(&switch_main_style, 255);
        lv_style_set_bg_color(&switch_main_style, lv_color_hex(0xe6e2e6));
        lv_style_set_bg_grad_dir(&switch_main_style, LV_GRAD_DIR_NONE);
        lv_style_set_border_width(&switch_main_style, 0);
        lv_style_set_radius(&switch_main_style, 10);
        lv_style_set_shadow_width(&switch_main_style, 0);
        lv_obj_add_style(sw, &switch_main_style, LV_PART_MAIN | LV_STATE_DEFAULT);

        // 设置开关INDICATOR样式（选中状态）
        static lv_style_t switch_indicator_style;
        lv_style_init(&switch_indicator_style);
        lv_style_set_bg_opa(&switch_indicator_style, 255);
        lv_style_set_bg_color(&switch_indicator_style, lv_color_hex(0x2195f6));
        lv_style_set_bg_grad_dir(&switch_indicator_style, LV_GRAD_DIR_NONE);
        lv_style_set_border_width(&switch_indicator_style, 0);
        lv_obj_add_style(sw, &switch_indicator_style, LV_PART_INDICATOR | LV_STATE_CHECKED);

        // 设置开关KNOB样式
        static lv_style_t switch_knob_style;
        lv_style_init(&switch_knob_style);
        lv_style_set_bg_opa(&switch_knob_style, 255);
        lv_style_set_bg_color(&switch_knob_style, lv_color_hex(0xffffff));
        lv_style_set_bg_grad_dir(&switch_knob_style, LV_GRAD_DIR_NONE);
        lv_style_set_border_width(&switch_knob_style, 0);
        lv_style_set_radius(&switch_knob_style, 10);
        lv_obj_add_style(sw, &switch_knob_style, LV_PART_KNOB | LV_STATE_DEFAULT);

        // 设置开关状态
        if (g_timers[i].is_enabled) {
            lv_obj_add_state(sw, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(sw, LV_STATE_CHECKED);
        }

        // 绑定开关事件处理函数
        lv_obj_add_event_cb(sw, timer_switch_event_handler, LV_EVENT_VALUE_CHANGED, (void*)(intptr_t)(i + 1));

        // 更新全局数组中的指针
        g_timers[i].cont = timer_cont;
        g_timers[i].time_group = time_group;
        g_timers[i].time_span = time_span;
        g_timers[i].switch_obj = sw;
    }
}

// 初始化倒计时事件
void timer_event_init(lv_ui *ui)
{
    // 注册添加倒计时按钮点击事件
    lv_obj_add_event_cb(ui->screen_timer_img_add, timer_add_event_handler, LV_EVENT_CLICKED, ui);
    // 注册删除倒计时按钮点击事件
    lv_obj_add_event_cb(ui->screen_timer_img_del, timer_del_event_handler, LV_EVENT_CLICKED, ui);

    // 隐藏静态创建的倒计时容器（cont1、cont2、cont3）
    lv_obj_add_flag(ui->screen_timer_cont_1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->screen_timer_cont_2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->screen_timer_cont_3, LV_OBJ_FLAG_HIDDEN);
}


void timer_event_setup(void)
{
    timer_event_init(&guider_ui);
}

