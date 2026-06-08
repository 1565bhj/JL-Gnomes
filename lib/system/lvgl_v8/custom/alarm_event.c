#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdbool.h"
#include "gui_guider.h"
#include "custom.h"
#include "alarm.h"
#include "sys_time.h"

// 定义alarm_info结构体以匹配alarm.c中的定义
struct alarm_info { //10字节
    unsigned char enable: 1; //是否使能闹钟
    unsigned char music: 1; //闹钟自定义音乐，0系统音乐
    unsigned char index: 4; //设置的闹钟index
    unsigned char resver: 2; //预留
    unsigned char cyc; //是否循环的闹钟: 0不循环，BIT(0)每天循环，BIT(1)-BIT(7)对应星期一到星期日
    unsigned char resver2; //预留
    struct sys_time time;
};

#define ALARM_NUM 10  // 与alarm.c中保持一致
struct alarm_info alarm_data[ALARM_NUM] = {0};

// 全局闹钟管理变量定义
int g_alarm_count = 0; // 当前闹钟总数
AlarmItem g_alarms[MAX_ALARMS] = {0}; // 全局变量定义
bool g_weekdays_selected[7] = {false}; // 周一到周日的选中状态
int g_current_alarm_index = 0; // 当前操作的闹钟索引


/**
 * 获取当前闹钟总数
 * @return 当前设置的闹钟数量
 */
int get_alarm_count(void)
{
    return g_alarm_count;
}
// 函数前向声明
static void alarm_switch_event_handler(lv_event_t *e);
static void async_ui_delete_once_alarm(void *params);
/**
 * 动态添加新闹钟（闹钟都通过此函数动态创建，共支持10个）
 * @param ui: GUI全局对象
 * @param hour: 选中的小时
 * @param minute: 选中的分钟
 * @param second: 选中的秒
 * @param status: 闹钟设置状态 // 0: 只响一次，1: 每天，2：日期设置
 */
void Dynamic_add_new_alarm(lv_ui *ui, int hour, int minute, int second, int status)
{
    // 1. 检查是否达到最大闹钟数
    if (g_alarm_count >= MAX_ALARMS) {
        printf("-> 已达到最大闹钟数量(%d个)\n", MAX_ALARMS);
        return;
    }
    printf("-> 正在添加新闹钟，时间: %02d:%02d:%02d，状态: %d\n", hour, minute, second, status);
    // 2. 检查是否存在相同时间的闹钟
    for (int i = 0; i < g_alarm_count; i++) {
        if (g_alarms[i].time_span && g_alarms[i].time_span->txt) {
            int existing_hour = 0, existing_minute = 0, existing_sec = 0;
            int parsed = sscanf(g_alarms[i].time_span->txt, "%d:%d:%d", &existing_hour, &existing_minute, &existing_sec);
            if (parsed == 2) {
                existing_sec = 0; // 如果只有时分，秒默认为0
            }

            // 比较时、分、秒是否完全相同
            if ((existing_hour == hour && existing_minute == minute && existing_sec == second) &&
                (g_alarms[i].repeat_status == status ||
                 (status == 1))) {
                // 根据是否有秒显示不同的日志格式
                if (second == 0) {
                    printf("-> 已存在相同时间的闹钟: %02d:%02d，不重复添加\n", hour, minute);
                } else {
                    printf("-> 已存在相同时间的闹钟: %02d:%02d:%02d，不重复添加\n", hour, minute, second);
                }
                return;
            }
        }
    }

    // 3. 计算新闹钟的位置（从1开始计数，Y轴间隔51px）
    int new_alarm_idx = g_alarm_count + 1; // 新闹钟序号（1~10）
    int y_pos = 0 + (new_alarm_idx - 1) * 61; // Y轴位置
    printf("-> 正在添加新闹钟%d，位置Y: %d\n", new_alarm_idx, y_pos);

    // 4. 动态创建alarm_cont_1容器（闹钟条目容器）
    lv_obj_t *alarm_cont = lv_obj_create(ui->screen_alarm_cont);
    lv_obj_set_pos(alarm_cont, 2, y_pos);
    lv_obj_set_size(alarm_cont, 258, 51);
    // 使用user_data标记这是动态创建的闹钟条目
    lv_obj_set_user_data(alarm_cont, (void*)1);

    // 设置容器样式（与静态创建的容器样式一致）
    static lv_style_t cont_style;
    lv_style_init(&cont_style);
    lv_style_set_border_width(&cont_style, 2);
    lv_style_set_border_opa(&cont_style, 0);
    lv_style_set_border_color(&cont_style, lv_color_hex(0xffffff));
    lv_style_set_border_side(&cont_style, LV_BORDER_SIDE_FULL);
    lv_style_set_radius(&cont_style, 0);
    lv_style_set_bg_opa(&cont_style, 53);
    lv_style_set_bg_color(&cont_style, lv_color_hex(0xffffff));
    lv_style_set_bg_grad_dir(&cont_style, LV_GRAD_DIR_NONE);
    lv_style_set_pad_top(&cont_style, 0);
    lv_style_set_pad_bottom(&cont_style, 0);
    lv_style_set_pad_left(&cont_style, 0);
    lv_style_set_pad_right(&cont_style, 0);
    lv_style_set_shadow_width(&cont_style, 0);
    lv_obj_add_style(alarm_cont, &cont_style, 0);
    lv_obj_set_style_radius(alarm_cont, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 5. 动态创建时间标签组
    lv_obj_t *time_group = lv_spangroup_create(alarm_cont);
    lv_spangroup_set_align(time_group, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(time_group, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(time_group, LV_SPAN_MODE_EXPAND);

    // 设置时间标签组位置和大小
    lv_obj_set_pos(time_group, 11, 2);
    lv_obj_set_size(time_group, 86, 25);

    // 创建时间文本span
    lv_span_t *time_span = lv_spangroup_new_span(time_group);
    // 设置时间文本格式（根据秒是否为0决定是否显示秒）
    char time_str[9];
    if (second == 0) {
        sprintf(time_str, "%02d:%02d", hour, minute);
    } else {
        sprintf(time_str, "%02d:%02d:%02d", hour, minute, second);
    }
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

    // 6. 动态创建重复文本标签组
    lv_obj_t *repeat_group = lv_spangroup_create(alarm_cont);
    lv_spangroup_set_align(repeat_group, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(repeat_group, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(repeat_group, LV_SPAN_MODE_EXPAND);

    // 创建重复文本span
    lv_span_t *repeat_span = lv_spangroup_new_span(repeat_group);

    // 根据状态设置重复文本内容
    char repeat_str[50] = "只响一次"; // 默认值
    int repeat_status = 0;

    if (status == 1) { // 每天
        strcpy(repeat_str, "每天");
        repeat_status = 1;
    } else if (status & (WEEK_MONDAY | WEEK_TUESDAY | WEEK_WEDNESDAY | WEEK_THURSDAY | WEEK_FRIDAY | WEEK_SATURDAY | WEEK_SUNDAY)) { // BIT(1)-BIT(7): 对应星期一到星期日
        repeat_str[0] = '\0'; // 清空字符串
        repeat_status = status;

        // 检查是否所有星期都被选中
        if ((status & (WEEK_MONDAY | WEEK_TUESDAY | WEEK_WEDNESDAY | WEEK_THURSDAY | WEEK_FRIDAY | WEEK_SATURDAY | WEEK_SUNDAY)) == (WEEK_MONDAY | WEEK_TUESDAY | WEEK_WEDNESDAY | WEEK_THURSDAY | WEEK_FRIDAY | WEEK_SATURDAY | WEEK_SUNDAY)) {
            strcpy(repeat_str, "每天");
        } else {
            // 定义星期名称数组（周一到周日）
            const char *weekday_names[] = {"周一", "周二", "周三", "周四", "周五", "周六", "周日"};

            // 遍历所有星期，构建选中的星期字符串
            for (int i = 0; i < 7; i++) {
                if (status & (1 << (i + 1))) { // 对应WEEK_MONDAY到WEEK_SUNDAY
                    if (strlen(repeat_str) > 0) {
                        strcat(repeat_str, " "); // 添加分隔符
                    }
                    strcat(repeat_str, weekday_names[i]); // 添加星期名称
                }
            }

            // 如果没有选择任何星期，默认显示只响一次
            if (strlen(repeat_str) == 0) {
                strcpy(repeat_str, "只响一次");
                repeat_status = 0; // 更新状态为只响一次
            }
        }

    }

    lv_span_set_text(repeat_span, repeat_str);

    // 设置重复文本样式
    lv_style_set_text_color(&repeat_span->style, lv_color_hex(0xc4c4c4));
    lv_style_set_text_decor(&repeat_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&repeat_span->style, &lv_font_MiSansDemibold_12);

    // 设置重复文本组位置和大小
    lv_obj_set_pos((lv_obj_t *)repeat_group, 13, 33);
    lv_obj_set_size((lv_obj_t *)repeat_group, 239, 12);

    // 设置重复文本组主样式
    static lv_style_t repeat_group_style;
    lv_style_init(&repeat_group_style);
    lv_style_set_border_width(&repeat_group_style, 0);
    lv_style_set_radius(&repeat_group_style, 0);
    lv_style_set_bg_opa(&repeat_group_style, 0);
    lv_style_set_pad_top(&repeat_group_style, 0);
    lv_style_set_pad_right(&repeat_group_style, 0);
    lv_style_set_pad_bottom(&repeat_group_style, 0);
    lv_style_set_pad_left(&repeat_group_style, 0);
    lv_style_set_shadow_width(&repeat_group_style, 0);
    lv_obj_add_style(repeat_group, &repeat_group_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode((lv_obj_t *)repeat_group);

    // 7. 动态创建开关控件
    lv_obj_t *sw = lv_switch_create(alarm_cont);
    lv_obj_set_pos(sw, 204, 16);
    lv_obj_set_size(sw, 35, 20);

    // 设置开关主样式（默认状态）
    static lv_style_t switch_main_style;
    lv_style_init(&switch_main_style);
    lv_style_set_bg_opa(&switch_main_style, 255);
    lv_style_set_bg_color(&switch_main_style, lv_color_hex(0xe6e2e6));
    lv_style_set_bg_grad_dir(&switch_main_style, LV_GRAD_DIR_NONE);
    lv_obj_add_style(sw, &switch_main_style, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 设置开关KNOB样式
    static lv_style_t switch_knob_style;
    lv_style_init(&switch_knob_style);
    lv_style_set_bg_opa(&switch_knob_style, 255);
    lv_style_set_bg_color(&switch_knob_style, lv_color_hex(0xffffff));
    lv_style_set_bg_grad_dir(&switch_knob_style, LV_GRAD_DIR_NONE);
    lv_style_set_border_width(&switch_knob_style, 0);
    lv_style_set_radius(&switch_knob_style, 10);
    lv_obj_add_style(sw, &switch_knob_style, LV_PART_KNOB | LV_STATE_DEFAULT);
    // 更新全局闹钟数组（记录动态闹钟信息）
    g_alarms[new_alarm_idx - 1] = (AlarmItem) {
        .switch_obj = sw,
        .time_group = time_group,
        .time_span = time_span,
        .repeat_group = repeat_group,
        .repeat_span = repeat_span,
        .index = new_alarm_idx,
        .is_dynamic = true, // 标记为动态闹钟
        .repeat_status = repeat_status, // 根据参数设置闹钟重复状态
        .is_enabled = true, // 默认开启闹钟
    };
    // 设置独立存储的时间文本
    strcpy(g_alarms[new_alarm_idx - 1].time_text, time_str);

    // 设置独立存储的重复设置文本和星期选择状态
    strcpy(g_alarms[new_alarm_idx - 1].repeat_text, repeat_str);
    for (int i = 0; i < 7; i++) {
        g_alarms[new_alarm_idx - 1].weekdays_selected[i] = g_weekdays_selected[i];
    }

    // 9. 增加当前闹钟总数
    g_alarm_count++;
    printf("-> 新闹钟%d添加完成，当前总数：%d\n", new_alarm_idx, g_alarm_count);

    // 10. 设置新闹钟开关状态为开启（默认状态）
    lv_obj_add_state(sw, LV_STATE_CHECKED);
    printf("-> 已打开新闹钟%d的UI开关\n", new_alarm_idx);

    // 为开关添加状态改变事件处理函数
    // 传递闹钟索引作为用户数据，以便在事件处理函数中知道是哪个闹钟的开关被操作
    lv_obj_add_event_cb(sw, alarm_switch_event_handler, LV_EVENT_VALUE_CHANGED, (void*)(intptr_t)(new_alarm_idx - 1));
    refresh_alarm_del_page(&guider_ui);
    extern_reset_alarm_set_options();
}
// 语音添加新闹钟参数结构体
typedef struct {
    int hour;
    int minute;
    int second;
    int cyc;
} voice_alarm_add_params_t;

/**
 * 异步语音添加新闹钟回调函数
 * @param param: 语音添加新闹钟参数结构体
 */
static void async_voice_add_alarm(void *param)
{
    if (!param) {
        return;
    }

    voice_alarm_add_params_t *add_params = (voice_alarm_add_params_t *)param;
    int hour = add_params->hour;
    int minute = add_params->minute;
    int second = add_params->second;
    int cyc = add_params->cyc;

    // 先检查是否存在相同时间的闹钟，如果存在则删除
    printf("-> 检查是否存在相同时间的闹钟...\n");

    // 遍历所有闹钟，删除所有匹配的闹钟
    bool found_duplicate = false;
    int i = 0;
    while (i < g_alarm_count) {
        if (g_alarms[i].time_text[0] != '\0') {
            int existing_hour = 0, existing_minute = 0, existing_sec = 0;
            int parsed = sscanf(g_alarms[i].time_text, "%d:%d:%d", &existing_hour, &existing_minute, &existing_sec);
            if (parsed == 2) {
                existing_sec = 0; // 如果只有时分，秒默认为0
            }

            // 比较时、分、秒是否完全相同，以及重复状态
            if ((existing_hour == hour && existing_minute == minute && existing_sec == second) &&
                (g_alarms[i].repeat_status == cyc ||
                 (cyc == 1))) {
                printf("-> 语音发现相同时间的闹钟，正在删除...\n");

                // 删除应用层的闹钟数据
                alarm_del_by_hour_min(0, existing_hour, existing_minute, g_alarms[i].repeat_status);

                // 删除UI对象
                if (g_alarms[i].time_group) {
                    lv_obj_del(g_alarms[i].time_group);
                    g_alarms[i].time_group = NULL;
                    g_alarms[i].time_span = NULL;
                }

                if (g_alarms[i].switch_obj) {
                    lv_obj_del(g_alarms[i].switch_obj);
                    g_alarms[i].switch_obj = NULL;
                }

                if (g_alarms[i].repeat_group) {
                    lv_obj_del(g_alarms[i].repeat_group);
                    g_alarms[i].repeat_group = NULL;
                    g_alarms[i].repeat_span = NULL;
                }

                // 从数组中移除并重新排列
                for (int j = i; j < g_alarm_count - 1; j++) {
                    g_alarms[j] = g_alarms[j + 1];
                }

                g_alarm_count--;
                found_duplicate = true;
                // 不增加i，因为删除后后面的元素会前移
                continue;
            }
        }
        i++;
    }

    // 如果找到并删除了相同时间的闹钟，刷新页面
    if (found_duplicate) {
        // 更新重新排列后所有闹钟的索引值
        for (int j = 0; j < g_alarm_count; j++) {
            g_alarms[j].index = j + 1; // 索引从1开始编号
        }

        printf("-> 成功删除相同时间的闹钟，剩余 %d 个\n", g_alarm_count);

        // 刷新页面
        refresh_alarm_del_page(&guider_ui);
        refresh_alarm_page(&guider_ui);
    }

    // 调用添加闹钟的函数
    Sync_ui_dynamic_add_new_alarm(&guider_ui, hour, minute, second, 0, cyc);
    // 同时添加应用层的闹钟
    Sync_app_dynamic_add_new_alarm(&guider_ui, hour, minute, second, 0, cyc);

    // 释放参数内存
    free(param);
}

// 语音添加新闹钟的函数（异步）
void Voice_dynamic_add_new_alarm(int hour, int minute, int second, int cyc)
{
    // 创建并初始化参数结构体
    voice_alarm_add_params_t *params = (voice_alarm_add_params_t *)malloc(sizeof(voice_alarm_add_params_t));
    if (params) {
        params->hour = hour;
        params->minute = minute;
        params->second = second;
        params->cyc = cyc;

        // 异步调用函数
        lv_async_call(async_voice_add_alarm, (void *)params);
    }
}
// 同步调用异步删除只响一次闹钟的函数
void Sync_ui_delete_once_alarm(lv_ui *ui, uint8_t hour, uint8_t minute, uint8_t second)
{
    // 创建并初始化参数结构体
    alarm_delete_params_t *params = (alarm_delete_params_t *)malloc(sizeof(alarm_delete_params_t));
    if (params) {
        params->ui = ui;
        params->hour = hour;
        params->minute = minute;
        params->second = second;

        // 异步调用函数
        lv_async_call(async_ui_delete_once_alarm, (void *)params);
    }
}

// 删除只响一次闹钟函数
bool Delete_once_alarm(int hour, int minute, int second)
{
    Sync_ui_delete_once_alarm(&guider_ui, hour, minute, second);
    return true;
}
bool Delete_once_alarm_ui(int hour, int minute, int second)
{
    printf("-> 进入删除只响一次闹钟函数，删除时间: %02d:%02d:%02d\n", hour, minute, second);

    // 参数有效性检查
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) {
        printf("-> 时间参数无效，无法删除\n");
        return false;
    }

    // 遍历所有闹钟，查找匹配的闹钟
    int alarm_index = -1;
    for (int i = 0; i < g_alarm_count; i++) {
        if (g_alarms[i].time_text[0] != '\0') {
            // 解析闹钟时间的小时、分钟和秒
            int alarm_hour, alarm_minute, alarm_second = 0;
            int parsed = sscanf(g_alarms[i].time_text, "%d:%d:%d", &alarm_hour, &alarm_minute, &alarm_second);

            // 检查是否只响一次
            bool is_once_alarm = (g_alarms[i].repeat_status == 0);

            // 比较时间和重复状态
            if (parsed == 2 && alarm_hour == hour && alarm_minute == minute && second == 0 && is_once_alarm) {
                // 没有秒的情况，只比较小时和分钟，且秒为0
                alarm_index = i;
                break;
            } else if (parsed == 3 && alarm_hour == hour && alarm_minute == minute && alarm_second == second && is_once_alarm) {
                // 有秒的情况，完全匹配
                alarm_index = i;
                break;
            }
        }
    }

    if (alarm_index == -1) {
        printf("-> 未找到匹配的只响一次闹钟\n");
        return false;
    }

    printf("-> 找到匹配的只响一次闹钟索引: %d, 时间: %s\n", alarm_index, g_alarms[alarm_index].time_text);

    // 删除UI对象
    if (g_alarms[alarm_index].time_group) {
        lv_obj_del(g_alarms[alarm_index].time_group);
        g_alarms[alarm_index].time_group = NULL;
        g_alarms[alarm_index].time_span = NULL;
    }

    if (g_alarms[alarm_index].switch_obj) {
        lv_obj_del(g_alarms[alarm_index].switch_obj);
        g_alarms[alarm_index].switch_obj = NULL;
    }

    if (g_alarms[alarm_index].repeat_group) {
        lv_obj_del(g_alarms[alarm_index].repeat_group);
        g_alarms[alarm_index].repeat_group = NULL;
        g_alarms[alarm_index].repeat_span = NULL;
    }

    // 从数组中移除并重新排列
    for (int j = alarm_index; j < g_alarm_count - 1; j++) {
        g_alarms[j] = g_alarms[j + 1];
    }

    // 更新重新排列后所有闹钟的索引值
    for (int j = 0; j < g_alarm_count - 1; j++) {
        g_alarms[j].index = j + 1; // 索引从1开始编号
    }

    g_alarm_count--;
    printf("-> 成功删除只响一次闹钟，剩余 %d 个\n", g_alarm_count);

    // 刷新页面
    refresh_alarm_page(&guider_ui);
    refresh_alarm_del_page(&guider_ui);
    // // 重置全局星期选择数组，避免影响下一次设置
    // for (int i = 0; i < 7; i++) {
    //     g_weekdays_selected[i] = false;
    // }
    return true;
}
static int repeat_status = -1;
// 闹钟开关状态改变事件处理函数
static void alarm_switch_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        // 获取触发事件的开关对象
        lv_obj_t *switch_obj = lv_event_get_target(e);
        // 获取闹钟索引
        int alarm_index = (int)(intptr_t)lv_event_get_user_data(e);

        // 检查索引是否有效
        if (alarm_index >= 0 && alarm_index < MAX_ALARMS) {
            // 更新is_enabled变量为开关的当前状态
            bool is_checked = lv_obj_has_state(switch_obj, LV_STATE_CHECKED);
            g_alarms[alarm_index].is_enabled = is_checked;

            printf("-> 闹钟[%d]开关状态改变为: %s\n",
                   alarm_index, is_checked ? "开启" : "关闭");

            // 管理应用层闹钟
#ifdef APPLAYER_ENABLE
            if (is_checked) {
                // 从 time_text 解析小时和分钟
                int hour = 0, minute = 0, second = 0;
                int parsed = sscanf(g_alarms[alarm_index].time_text, "%d:%d:%d", &hour, &minute, &second);
                if (parsed == 2) {
                    second = 0; // 如果只有时分，秒默认为0
                }
                if (repeat_status != -1) {
                    // 打开闹钟，添加应用层闹钟
                    alarm_add_by_hour_min(alarm_index, hour, minute, repeat_status);
                    printf("-> 应用层添加闹钟[%d]: %02d:%02d, 重复状态: 0x%02x\n",
                           alarm_index, hour, minute, repeat_status);
                }
            } else {
                // 保存重复状态
                alarm_time_read(alarm_data, sizeof(alarm_data));
                repeat_status = alarm_data[alarm_index].cyc;
                printf("-> 保存闹钟[%d]重复状态: 0x%02x\n", alarm_index, repeat_status);
                // 关闭闹钟，删除应用层闹钟
                alarm_delete_by_index(alarm_index);
                printf("-> 应用层删除闹钟[%d]\n", alarm_index);

            }
#endif
        }
    }
}

// 异步删除只响一次闹钟的函数
static void async_ui_delete_once_alarm(void *params)
{
    if (params) {
        alarm_delete_params_t *delete_params = (alarm_delete_params_t *)params;
        // 调用实际的删除函数
        Delete_once_alarm_ui(delete_params->hour, delete_params->minute, delete_params->second);
        // 释放参数内存
        free(params);
    }
}

// 闹钟添加按钮点击事件处理函数
static void alarm_add_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if (!ui || !ui->screen_alarm_add) {
        return;
    }
    if (code == LV_EVENT_CLICKED) {
        // 切换到添加闹钟页面
        switch_to_alarm_add_page();

    }

}

// 闹钟删除按钮点击事件处理函数
static void alarm_del_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if (!ui || !ui->screen_alarm_remove) {
        return;
    }
    if (code == LV_EVENT_CLICKED) {
        // 切换到删除闹钟页面
        switch_to_alarm_remove_page();

    }
}
/**
 * 清除所有闹钟（现在所有闹钟都是动态创建的）
 * @param ui: GUI全局对象
 */
static void clear_all_alarms_cb(void *p)
{
    printf("-> 正在清除所有闹钟\n");

    // 1. 遍历所有闹钟，删除UI组件
    for (int i = 0; i < g_alarm_count; i++) {
        // 删除时间标签组
        if (g_alarms[i].time_group) {
            lv_obj_del(g_alarms[i].time_group);
            g_alarms[i].time_group = NULL;
            g_alarms[i].time_span = NULL;
        }

        // 删除开关控件
        if (g_alarms[i].switch_obj) {
            lv_obj_del(g_alarms[i].switch_obj);
            g_alarms[i].switch_obj = NULL;
        }

        // 删除重复设置标签组
        if (g_alarms[i].repeat_group) {
            lv_obj_del(g_alarms[i].repeat_group);
            g_alarms[i].repeat_group = NULL;
            g_alarms[i].repeat_span = NULL;
        }
    }

    // 2. 重置全局闹钟数组
    memset(g_alarms, 0, sizeof(g_alarms));

    // 3. 重置闹钟计数
    g_alarm_count = 0;

    // 4. 刷新页面
    refresh_alarm_page(&guider_ui);
    refresh_alarm_del_page(&guider_ui);

    printf("-> 所有闹钟已清除\n");
}
void clear_all_alarms(void)
{
    lv_async_call(clear_all_alarms_cb, NULL);
}

/**
 * 刷新闹钟主页面的闹钟列表
 * @param ui: GUI全局对象
 */
void refresh_alarm_page(lv_ui *ui)
{
    // 确保UI指针有效
    if (ui == NULL || ui->screen_alarm_cont == NULL) {
        printf("-> 警告: UI指针无效，无法刷新闹钟页面\n");
        return;
    }

    // 1. 清除当前所有动态创建的闹钟条目
    lv_obj_t *cont = ui->screen_alarm_cont;
    int i = 0;
    lv_obj_t *child;
    while ((child = lv_obj_get_child(cont, i)) != NULL) {
        // 删除所有动态创建的闹钟条目，除了静态创建的三个容器
        if (lv_obj_get_user_data(child) == (void *)1 &&
            child != ui->screen_alarm_cont_1 &&
            child != ui->screen_alarm_cont_2 &&
            child != ui->screen_alarm_cont_3) {
            lv_obj_del(child);
            // 删除后索引不变，因为下一个子对象会移动到当前位置
        } else {
            i++;
        }
    }

    // 2. 遍历全局闹钟数组，重新创建闹钟条目
    for (int i = 0; i < g_alarm_count; i++) {
        // 跳过无效的闹钟条目
        if (g_alarms[i].time_text[0] == '\0') {
            printf("-> 跳过无效闹钟条目[%d]\n", i);
            continue;
        }

        // 计算新闹钟条目的Y轴位置（从1开始计数，Y轴间隔51px）
        int y_pos = 0 + i * 61; // Y轴位置

        // 创建闹钟条目容器
        lv_obj_t *alarm_cont = lv_obj_create(ui->screen_alarm_cont);
        lv_obj_set_pos(alarm_cont, 2, y_pos);
        lv_obj_set_size(alarm_cont, 258, 51);
        // 使用user_data标记这是动态创建的闹钟条目
        lv_obj_set_user_data(alarm_cont, (void*)1);

        // 设置容器样式（与静态创建的容器样式一致）
        static lv_style_t cont_style;
        lv_style_init(&cont_style);
        lv_style_set_border_width(&cont_style, 2);
        lv_style_set_border_opa(&cont_style, 0);
        lv_style_set_border_color(&cont_style, lv_color_hex(0xffffff));
        lv_style_set_border_side(&cont_style, LV_BORDER_SIDE_FULL);
        lv_style_set_radius(&cont_style, 0);
        lv_style_set_bg_opa(&cont_style, 53);
        lv_style_set_bg_color(&cont_style, lv_color_hex(0xffffff));
        lv_style_set_bg_grad_dir(&cont_style, LV_GRAD_DIR_NONE);
        lv_style_set_pad_top(&cont_style, 0);
        lv_style_set_pad_bottom(&cont_style, 0);
        lv_style_set_pad_left(&cont_style, 0);
        lv_style_set_pad_right(&cont_style, 0);
        lv_style_set_shadow_width(&cont_style, 0);
        lv_obj_add_style(alarm_cont, &cont_style, 0);
        lv_obj_set_style_radius(alarm_cont, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

        // 创建时间标签组
        lv_obj_t *time_group = lv_spangroup_create(alarm_cont);
        lv_spangroup_set_align(time_group, LV_TEXT_ALIGN_LEFT);
        lv_spangroup_set_overflow(time_group, LV_SPAN_OVERFLOW_CLIP);
        lv_spangroup_set_mode(time_group, LV_SPAN_MODE_EXPAND);

        // 设置时间标签组位置和大小
        lv_obj_set_pos(time_group, 11, 2);
        lv_obj_set_size(time_group, 86, 25);

        // 创建时间文本span
        lv_span_t *time_span = lv_spangroup_new_span(time_group);

        // 使用独立存储的时间文本，避免依赖可能失效的time_span指针
        lv_span_set_text(time_span, g_alarms[i].time_text);

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

        // 创建重复文本标签组
        lv_obj_t *repeat_group = lv_spangroup_create(alarm_cont);
        lv_spangroup_set_align(repeat_group, LV_TEXT_ALIGN_LEFT);
        lv_spangroup_set_overflow(repeat_group, LV_SPAN_OVERFLOW_CLIP);
        lv_spangroup_set_mode(repeat_group, LV_SPAN_MODE_EXPAND);

        // 创建重复文本span
        lv_span_t *repeat_span = lv_spangroup_new_span(repeat_group);

        // 直接使用独立存储的重复设置文本，避免重新计算导致的不一致
        // 如果重复文本为空，设置默认值
        if (g_alarms[i].repeat_text[0] == '\0') {
            lv_span_set_text(repeat_span, "只响一次");
        } else {
            lv_span_set_text(repeat_span, g_alarms[i].repeat_text);
        }

        // 设置重复文本样式
        lv_style_set_text_color(&repeat_span->style, lv_color_hex(0xc4c4c4));
        lv_style_set_text_decor(&repeat_span->style, LV_TEXT_DECOR_NONE);
        lv_style_set_text_font(&repeat_span->style, &lv_font_MiSansDemibold_12);

        // 设置重复文本组位置和大小
        lv_obj_set_pos((lv_obj_t *)repeat_group, 13, 33);
        lv_obj_set_size((lv_obj_t *)repeat_group, 239, 12);

        // 设置重复文本组主样式
        static lv_style_t repeat_group_style;
        lv_style_init(&repeat_group_style);
        lv_style_set_border_width(&repeat_group_style, 0);
        lv_style_set_radius(&repeat_group_style, 0);
        lv_style_set_bg_opa(&repeat_group_style, 0);
        lv_style_set_pad_top(&repeat_group_style, 0);
        lv_style_set_pad_right(&repeat_group_style, 0);
        lv_style_set_pad_bottom(&repeat_group_style, 0);
        lv_style_set_pad_left(&repeat_group_style, 0);
        lv_style_set_shadow_width(&repeat_group_style, 0);
        lv_obj_add_style(repeat_group, &repeat_group_style, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_spangroup_refr_mode((lv_obj_t *)repeat_group);

        // 创建开关控件
        lv_obj_t *sw = lv_switch_create(alarm_cont);
        lv_obj_set_pos(sw, 204, 16);
        lv_obj_set_size(sw, 35, 20);

        // 设置开关主样式（默认状态）
        static lv_style_t switch_main_style;
        lv_style_init(&switch_main_style);
        lv_style_set_bg_opa(&switch_main_style, 255);
        lv_style_set_bg_color(&switch_main_style, lv_color_hex(0xe6e2e6));
        lv_style_set_bg_grad_dir(&switch_main_style, LV_GRAD_DIR_NONE);
        lv_obj_add_style(sw, &switch_main_style, LV_PART_MAIN | LV_STATE_DEFAULT);

        // 设置开关KNOB样式
        static lv_style_t switch_knob_style;
        lv_style_init(&switch_knob_style);
        lv_style_set_bg_opa(&switch_knob_style, 255);
        lv_style_set_bg_color(&switch_knob_style, lv_color_hex(0xffffff));
        lv_style_set_bg_grad_dir(&switch_knob_style, LV_GRAD_DIR_NONE);
        lv_style_set_border_width(&switch_knob_style, 0);
        lv_style_set_radius(&switch_knob_style, 10);
        lv_obj_add_style(sw, &switch_knob_style, LV_PART_KNOB | LV_STATE_DEFAULT);

        // 保存当前开关状态到is_enabled变量
        bool current_is_enabled = false;
        if (g_alarms[i].switch_obj != NULL && lv_obj_has_state(g_alarms[i].switch_obj, LV_STATE_CHECKED)) {
            current_is_enabled = true;
        }

        // 安全地更新全局闹钟数组中的指针
        // 先保存原有数据，再更新指针
        int repeat_status = g_alarms[i].repeat_status;
        bool is_selected = g_alarms[i].is_selected;
        int index = g_alarms[i].index;

        // 更新指针 - 包括所有UI元素指针
        g_alarms[i].switch_obj = sw;
        g_alarms[i].time_group = time_group;
        g_alarms[i].time_span = time_span;
        g_alarms[i].repeat_group = repeat_group;  // 更新重复标签组指针
        g_alarms[i].repeat_span = repeat_span;   // 更新重复文本span指针

        // 恢复原有数据
        g_alarms[i].repeat_status = repeat_status;
        g_alarms[i].is_selected = is_selected;
        g_alarms[i].index = index;

        // 设置开关状态，使用保存的is_enabled值
        if (g_alarms[i].is_enabled) {
            lv_obj_add_state(sw, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(sw, LV_STATE_CHECKED);
        }

        // 为开关添加状态改变事件处理函数
        // 传递闹钟索引作为用户数据
        lv_obj_add_event_cb(sw, alarm_switch_event_handler, LV_EVENT_VALUE_CHANGED, (void*)(intptr_t)i);

        printf("-> 成功刷新闹钟[%d], 时间: %s, 开关状态: %s\n",
               i, time_span->txt, g_alarms[i].is_enabled ? "开启" : "关闭");
    }

    printf("-> 闹钟页面刷新完成，共显示 %d 个闹钟\n", g_alarm_count);
}


// 从服务器读取闹钟数据并仅在初始化时更新UI（不重复创建已有UI）
static void load_alarms_on_initialization(void)
{

    // 读取服务器闹钟数据
    printf("[闹钟load初始化] 开始从服务器读取闹钟数据...\n");
    alarm_time_read(alarm_data, sizeof(alarm_data));

    int loaded_count = 0;
    int created_count = 0;

    // 遍历服务器返回的所有闹钟
    for (int i = 0; i < ALARM_NUM; i++) {
        // 跳过无效闹钟(未使能)
        if (!alarm_data[i].enable) {
            continue;
        }
        loaded_count++;
        bool found_existing_alarm = false;

        // 检查是否已经存在相同时间的UI闹钟元素
        for (int j = 0; j < g_alarm_count; j++) {
            if (g_alarms[j].time_span && g_alarms[j].time_span->txt) {
                int existing_hour = 0, existing_minute = 0, existing_second = 0;
                int parsed = sscanf(g_alarms[j].time_span->txt, "%d:%d:%d", &existing_hour, &existing_minute, &existing_second);
                if (parsed == 2) {
                    existing_second = 0; // 如果只有时分，秒默认为0
                }

                if (existing_hour == alarm_data[i].time.hour &&
                    existing_minute == alarm_data[i].time.min &&
                    existing_second == alarm_data[i].time.sec) {
                    // 找到现有闹钟，更新其状态
                    g_alarms[j].is_enabled = (alarm_data[i].enable != 0);
                    // 根据 cyc 值设置 repeat_status：0 表示只响一次，1 表示每天，其他值表示具体星期（BIT(1)-BIT(7)）
                    if (alarm_data[i].cyc == 0) {
                        g_alarms[j].repeat_status = 0;
                    } else if (alarm_data[i].cyc == 1) {
                        g_alarms[j].repeat_status = 1;
                    } else {
                        g_alarms[j].repeat_status = alarm_data[i].cyc;
                        // 解析星期位（BIT(1)-BIT(7)）
                        // 初始化星期选择数组
                        for (int k = 0; k < 7; k++) {
                            g_weekdays_selected[k] = false;
                        }
                        // 根据 cyc 值设置星期选择
                        if (alarm_data[i].cyc & WEEK_MONDAY) {
                            g_weekdays_selected[0] = true; // 星期一
                        }
                        if (alarm_data[i].cyc & WEEK_TUESDAY) {
                            g_weekdays_selected[1] = true; // 星期二
                        }
                        if (alarm_data[i].cyc & WEEK_WEDNESDAY) {
                            g_weekdays_selected[2] = true; // 星期三
                        }
                        if (alarm_data[i].cyc & WEEK_THURSDAY) {
                            g_weekdays_selected[3] = true; // 星期四
                        }
                        if (alarm_data[i].cyc & WEEK_FRIDAY) {
                            g_weekdays_selected[4] = true; // 星期五
                        }
                        if (alarm_data[i].cyc & WEEK_SATURDAY) {
                            g_weekdays_selected[5] = true; // 星期六
                        }
                        if (alarm_data[i].cyc & WEEK_SUNDAY) {
                            g_weekdays_selected[6] = true; // 星期日
                        }
                        // 打印设置的星期
                        char weekdays_str[100] = "";
                        const char *weekday_names[] = {"星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期日"};
                        for (int k = 0; k < 7; k++) {
                            if (g_weekdays_selected[k]) {
                                if (strlen(weekdays_str) > 0) {
                                    strcat(weekdays_str, "、");
                                }
                                strcat(weekdays_str, weekday_names[k]);
                            }
                        }
                        if (strlen(weekdays_str) > 0) {
                            printf("[闹钟load初始化] 设置星期: %s\n", weekdays_str);
                        }

                        // 重置g_weekdays_selected数组，避免影响下一次设置
                        for (int k = 0; k < 7; k++) {
                            g_weekdays_selected[k] = false;
                        }
                    }
                    found_existing_alarm = true;
                    // 根据是否有秒显示不同的日志格式
                    if (existing_second == 0) {
                        printf("[闹钟load初始化] 已存在相同时间的闹钟: %02d:%02d，仅更新状态\n",
                               alarm_data[i].time.hour, alarm_data[i].time.min);
                    } else {
                        printf("[闹钟load初始化] 已存在相同时间的闹钟: %02d:%02d:%02d，仅更新状态\n",
                               alarm_data[i].time.hour, alarm_data[i].time.min, alarm_data[i].time.sec);
                    }
                    break;
                }
            }
        }

        // 如果不存在相同时间的闹钟，则添加新闹钟到UI数据结构
        if (!found_existing_alarm && g_alarm_count < MAX_ALARMS) {
            // 使用现有的Dynamic_add_new_alarm函数添加新闹钟
            int repeat_status = 0;
            if (alarm_data[i].cyc == 0) {
                repeat_status = 0;
                // 只响一次，清空星期选择
                for (int k = 0; k < 7; k++) {
                    g_weekdays_selected[k] = false;
                }
            } else if (alarm_data[i].cyc == 1) {
                repeat_status = 1;
                // 每天，设置所有星期
                for (int k = 0; k < 7; k++) {
                    g_weekdays_selected[k] = true;
                }
                printf("[闹钟load初始化] 设置星期: 每天\n");
            } else {
                // 星期值（2,4,8,16,32,64,128）对应日期设置
                repeat_status = alarm_data[i].cyc;
                // 初始化星期选择数组
                for (int k = 0; k < 7; k++) {
                    g_weekdays_selected[k] = false;
                }
                // 根据 cyc 值设置星期选择
                if (alarm_data[i].cyc & WEEK_MONDAY) {
                    g_weekdays_selected[0] = true; // 星期一
                }
                if (alarm_data[i].cyc & WEEK_TUESDAY) {
                    g_weekdays_selected[1] = true; // 星期二
                }
                if (alarm_data[i].cyc & WEEK_WEDNESDAY) {
                    g_weekdays_selected[2] = true; // 星期三
                }
                if (alarm_data[i].cyc & WEEK_THURSDAY) {
                    g_weekdays_selected[3] = true; // 星期四
                }
                if (alarm_data[i].cyc & WEEK_FRIDAY) {
                    g_weekdays_selected[4] = true; // 星期五
                }
                if (alarm_data[i].cyc & WEEK_SATURDAY) {
                    g_weekdays_selected[5] = true; // 星期六
                }
                if (alarm_data[i].cyc & WEEK_SUNDAY) {
                    g_weekdays_selected[6] = true; // 星期日
                }
                // 打印设置的星期
                char weekdays_str[100] = "";
                const char *weekday_names[] = {"星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期日"};
                for (int k = 0; k < 7; k++) {
                    if (g_weekdays_selected[k]) {
                        if (strlen(weekdays_str) > 0) {
                            strcat(weekdays_str, "、");
                        }
                        strcat(weekdays_str, weekday_names[k]);
                    }
                }
                if (strlen(weekdays_str) > 0) {
                    printf("[闹钟load初始化] 设置星期: %s\n", weekdays_str);
                }

                // 重置g_weekdays_selected数组，避免影响下一次设置
                for (int k = 0; k < 7; k++) {
                    g_weekdays_selected[k] = false;
                }
            }
            Dynamic_add_new_alarm(&guider_ui, alarm_data[i].time.hour, alarm_data[i].time.min, alarm_data[i].time.sec, repeat_status);
            created_count++;

            // 根据是否有秒显示不同的日志格式
            if (alarm_data[i].time.sec == 0) {
                printf("[闹钟load初始化] 添加新闹钟: %02d:%02d (使能: %d, 循环: %d)\n",
                       alarm_data[i].time.hour, alarm_data[i].time.min,
                       alarm_data[i].enable, alarm_data[i].cyc);
            } else {
                printf("[闹钟load初始化] 添加新闹钟: %02d:%02d:%02d (使能: %d, 循环: %d)\n",
                       alarm_data[i].time.hour, alarm_data[i].time.min, alarm_data[i].time.sec,
                       alarm_data[i].enable, alarm_data[i].cyc);
            }
        }
    }

    // 刷新UI显示
    printf("[闹钟load初始化] 完成从服务器加载闹钟数据，共加载 %d 个有效闹钟，创建新UI元素 %d 个\n",
           loaded_count, created_count);
// 刷新页面以显示新的顺序
    refresh_alarm_page(&guider_ui);
    refresh_alarm_del_page(&guider_ui);
}
// 修改alarm_ui_init函数
void alarm_ui_init(void)
{
    printf("[闹钟UI初始化] 开始从服务器读取现有闹钟数据...\n");

    load_alarms_on_initialization();

//    // 添加定期检查闹钟的定时器，每分钟检查一次
//    if (!alarm_check_timer_id) {
//        alarm_check_timer_id = sys_timeout_add_to_task("sys_timer", NULL, ui_check_alarm_and_trigger, 60*1000);
//        printf("[闹钟UI初始化] 添加定期闹钟检查定时器\n");
//    }
}

// 初始化闹钟事件
void alarm_event_init(lv_ui *ui)
{
    // 注册添加闹钟按钮点击事件
    lv_obj_add_event_cb(ui->screen_alarm_alarm_add, alarm_add_event_handler, LV_EVENT_CLICKED, ui);
    // 注册删除闹钟按钮点击事件
    lv_obj_add_event_cb(ui->screen_alarm_alarm_del, alarm_del_event_handler, LV_EVENT_CLICKED, ui);

    // 隐藏静态创建的闹钟容器（cont1、cont2、cont3）
    lv_obj_add_flag(ui->screen_alarm_cont_1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->screen_alarm_cont_2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->screen_alarm_cont_3, LV_OBJ_FLAG_HIDDEN);
}

void alarm_event_setup(void)
{
    alarm_event_init(&guider_ui);
}

