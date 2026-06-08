#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdbool.h"
#include "gui_guider.h"
#include "custom.h"
#include "timer.h"
#include "alarm.h"

// 外部变量声明，用于从alarm_add页面传递时间到alarm_set页面
extern int g_alarm_hour;
extern int g_alarm_minute;

// 获取分钟/秒滚轮的值，考虑到滚轮的特殊实现（有额外的选项）
static uint8_t get_min_sec_roller_value(lv_obj_t *roller)
{
    uint16_t selected = lv_roller_get_selected(roller);
    // 由于我们在滚轮前面添加了3个额外选项，所以实际值需要减去3
    uint8_t value = (selected - 3) % 60;
    return value;
}

// 动态生成滚轮选项的辅助函数
static void generate_roller_options(lv_obj_t *roller, int start, int end, bool infinite)
{
    char options[2000] = {0};  // 足够容纳所有选项
    char buffer[10] = {0};

    if (infinite) {
        // 无限循环模式：直接生成所有选项
        for (int i = start; i <= end; i++) {
            sprintf(buffer, "%d\n", i);
            strcat(options, buffer);
        }
    } else {
        // 正常模式但需要视觉上的无缝循环：添加额外的边界选项
        // 在开始之前添加3个结束的数字
        for (int i = end - 2; i <= end; i++) {
            sprintf(buffer, "%d\n", i);
            strcat(options, buffer);
        }

        // 生成主要选项
        for (int i = start; i <= end; i++) {
            sprintf(buffer, "%d\n", i);
            strcat(options, buffer);
        }

        // 在结束之后添加3个开始的数字
        for (int i = start; i <= start + 2; i++) {
            sprintf(buffer, "%d\n", i);
            strcat(options, buffer);
        }
    }

    // 移除最后一个换行符
    options[strlen(options) - 1] = '\0';

    // 根据参数设置滚轮模式
    lv_roller_set_options(roller, options, infinite ? LV_ROLLER_MODE_INFINITE : LV_ROLLER_MODE_NORMAL);

    if (!infinite) {
        // 在正常模式下，初始位置设置为第一个实际选项（索引3）
        lv_roller_set_selected(roller, 3, LV_ANIM_OFF);
    }
}


// 分钟/秒滚轮自定义无缝循环事件处理函数（正常模式下的视觉无缝循环）
static void min_sec_roller_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *roller = lv_event_get_target(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        uint16_t selected = lv_roller_get_selected(roller);

        // 实现视觉上的无缝循环效果：0-59
        if (selected == 0) {
            // 当滚动到最上端的57（我们添加的额外选项）时，无缝跳转到实际的57（索引60）
            lv_roller_set_selected(roller, 60, LV_ANIM_OFF);
        } else if (selected == 1) {
            // 当滚动到最上端的58（我们添加的额外选项）时，无缝跳转到实际的58（索引61）
            lv_roller_set_selected(roller, 61, LV_ANIM_OFF);
        } else if (selected == 2) {
            // 当滚动到最上端的59（我们添加的额外选项）时，无缝跳转到实际的59（索引62）
            lv_roller_set_selected(roller, 62, LV_ANIM_OFF);
        } else if (selected == 63) {
            // 当滚动到最下端的0（我们添加的额外选项）时，无缝跳转到实际的0（索引3）
            lv_roller_set_selected(roller, 3, LV_ANIM_OFF);
        } else if (selected == 64) {
            // 当滚动到最下端的1（我们添加的额外选项）时，无缝跳转到实际的1（索引4）
            lv_roller_set_selected(roller, 4, LV_ANIM_OFF);
        } else if (selected == 65) {
            // 当滚动到最下端的2（我们添加的额外选项）时，无缝跳转到实际的2（索引5）
            lv_roller_set_selected(roller, 5, LV_ANIM_OFF);
        }
    }
}

// 使用alarm.h中定义的alarm_add_params_t结构体

// 异步动态添加新闹钟UI函数
static void async_ui_dynamic_add_new_alarm(void* param)
{
    if (!param) {
        return;
    }

    // 解析参数
    alarm_add_params_t *alarm_params = (alarm_add_params_t *)param;

    // 先检查是否存在相同时间的闹钟，如果存在则删除
    int duplicate_index = -1;
    for (int i = 0; i < g_alarm_count; i++) {
        if (g_alarms[i].time_text[0] != '\0') {
            int existing_hour = 0, existing_minute = 0, existing_sec = 0;
            int parsed = sscanf(g_alarms[i].time_text, "%d:%d:%d", &existing_hour, &existing_minute, &existing_sec);
            if (parsed == 2) {
                existing_sec = 0; // 如果只有时分，秒默认为0
            }

            // 比较时、分、秒是否完全相同
            if ((existing_hour == alarm_params->hour && existing_minute == alarm_params->minute && existing_sec == alarm_params->second)) {
                duplicate_index = i;
                break;
            }
        }
    }

    // 如果找到相同时间的闹钟，先删除它
    if (duplicate_index != -1) {
        printf("-> 发现相同时间的闹钟，正在删除...\n");
        return;
    }

    // 根据参数创建新闹钟
    Dynamic_add_new_alarm(alarm_params->ui, alarm_params->hour, alarm_params->minute, alarm_params->second, alarm_params->repeat_status);
    // 释放参数内存
    free(param);
}
// 同步调用异步添加闹钟的函数
void Sync_ui_dynamic_add_new_alarm(lv_ui *ui, uint8_t hour, uint8_t minute, uint8_t second, int alarm_index, uint8_t repeat_status)
{
    // 创建并初始化参数结构体
    alarm_add_params_t *params = (alarm_add_params_t *)malloc(sizeof(alarm_add_params_t));
    if (params) {
        params->ui = ui;
        params->hour = hour;
        params->minute = minute;
        params->second = second;
        params->alarm_index = alarm_index;
        params->repeat_status = repeat_status;

        // 异步调用函数
        lv_async_call(async_ui_dynamic_add_new_alarm, (void *)params);
    }
}
// 异步动态添加新闹钟应用层函数
static void async_app_dynamic_add_new_alarm(void* param)
{
    if (!param) {
        return;
    }

    // 解析参数
    alarm_add_params_t *alarm_params = (alarm_add_params_t *)param;
    uint8_t date_set = 0;

    // 先检查是否存在相同时间的闹钟，如果存在则删除
    int duplicate_index = -1;
    for (int i = 0; i < g_alarm_count; i++) {
        if (g_alarms[i].time_text[0] != '\0') {
            int existing_hour = 0, existing_minute = 0, existing_sec = 0;
            int parsed = sscanf(g_alarms[i].time_text, "%d:%d:%d", &existing_hour, &existing_minute, &existing_sec);
            if (parsed == 2) {
                existing_sec = 0; // 如果只有时分，秒默认为0
            }

            // 比较时、分、秒是否完全相同
            if ((existing_hour == alarm_params->hour && existing_minute == alarm_params->minute && existing_sec == alarm_params->second)) {
                duplicate_index = i;
                break;
            }
        }
    }

    // 如果找到相同时间的闹钟，先删除它
    if (duplicate_index != -1) {
        printf("-> 发现相同时间的闹钟，正在删除...\n");
        return;
        // 先删除应用层的闹钟数据
        alarm_del_by_hour_min(0, alarm_params->hour, alarm_params->minute, alarm_params->repeat_status);

    }

#ifdef APPLAYER_ENABLE
    alarm_add_by_hour_min(alarm_params->alarm_index, alarm_params->hour, alarm_params->minute, alarm_params->repeat_status);
    printf("date_set: 0x%02x\n", alarm_params->repeat_status);
#endif

    // 释放参数内存
    free(param);
}
// 同步调用异步添加闹钟的函数
void Sync_app_dynamic_add_new_alarm(lv_ui *ui, uint8_t hour, uint8_t minute, uint8_t second, int alarm_index, uint8_t repeat_status)
{
    // 创建并初始化参数结构体
    alarm_add_params_t *params = (alarm_add_params_t *)malloc(sizeof(alarm_add_params_t));
    if (params) {
        params->ui = ui;
        params->hour = hour;
        params->minute = minute;
        params->second = second;
        params->alarm_index = alarm_index;
        params->repeat_status = repeat_status;

        // 异步调用函数
        lv_async_call(async_app_dynamic_add_new_alarm, (void *)params);
    }
}

// 添加页面确认按钮点击事件处理函数
static void alarm_add_yes_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if (!ui || !ui->screen_alarm_add_yes) {
        return;
    }
    if (code == LV_EVENT_CLICKED) {
        AlarmItem *alarm = &g_alarms[g_current_alarm_index];

        // 获取用户选择的时间
        uint8_t hour = lv_roller_get_selected(ui->screen_alarm_add_roller_hour);
        uint8_t min = get_min_sec_roller_value(ui->screen_alarm_add_roller_min);

        // 同步调用添加闹钟函数
        Sync_ui_dynamic_add_new_alarm(ui, hour, min, 0, 0, g_alarms[g_current_alarm_index].repeat_status);
        // 同步调用添加闹钟函数
        Sync_app_dynamic_add_new_alarm(ui, hour, min, 0, g_current_alarm_index, g_alarms[g_current_alarm_index].repeat_status);
        // 切换回闹钟主页面
        switch_to_alarm_page();
    }
}

// 闹钟添加页面返回按钮点击事件处理函数
static void alarm_add_return_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if (!ui || !ui->screen_alarm_add_return) {
        return;
    }
    if (code == LV_EVENT_CLICKED) {
        // 切换回闹钟主页面
        switch_to_alarm_page();
    }
}

// 重复设置容器点击事件处理函数
static void alarm_add_cont_repeat_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if (!ui || !ui->screen_alarm_add_cont_repeat) {
        return;
    }
    if (code == LV_EVENT_CLICKED) {
        // 保存当前选择的时间到全局变量
        g_alarm_hour = lv_roller_get_selected(ui->screen_alarm_add_roller_hour);
        g_alarm_minute = get_min_sec_roller_value(ui->screen_alarm_add_roller_min);

        // 切换到闹钟重复设置页面
        switch_to_alarm_set_page();
    }
}

// 初始化新建闹钟事件
void alarm_add_event_init(lv_ui *ui)
{
    // 动态生成小时、分钟滚轮选项
    generate_roller_options(ui->screen_alarm_add_roller_hour, 0, 23, true);  // 小时: 0-99 (无限循环模式)
    generate_roller_options(ui->screen_alarm_add_roller_min, 0, 59, false);  // 分钟: 0-59 (正常模式)

    // 设置滚轮行间距（减小数字间的上下间距）
    lv_obj_set_style_text_line_space(ui->screen_alarm_add_roller_hour, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_alarm_add_roller_hour, 10, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_alarm_add_roller_min, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_alarm_add_roller_min, 10, LV_PART_SELECTED | LV_STATE_DEFAULT);


    // 先移除可能已存在的事件处理函数，防止多次注册
    lv_obj_remove_event_cb(ui->screen_alarm_add_return, alarm_add_return_event_handler);
    lv_obj_remove_event_cb(ui->screen_alarm_add_yes, alarm_add_yes_event_handler);
    lv_obj_remove_event_cb(ui->screen_alarm_add_roller_min, min_sec_roller_event_handler);
    // 注册返回按钮点击事件
    lv_obj_add_event_cb(ui->screen_alarm_add_return, alarm_add_return_event_handler, LV_EVENT_CLICKED, ui);

    // 注册确认按钮点击事件
    lv_obj_add_event_cb(ui->screen_alarm_add_yes, alarm_add_yes_event_handler, LV_EVENT_CLICKED, ui);

    // 注册分钟和秒滚轮事件处理函数（自定义无限循环）
    lv_obj_add_event_cb(ui->screen_alarm_add_roller_min, min_sec_roller_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    // 注册重复设置容器点击事件
    lv_obj_add_event_cb(ui->screen_alarm_add_cont_repeat, alarm_add_cont_repeat_event_handler, LV_EVENT_CLICKED, ui);

}

void alarm_add_event_setup(void)
{
    alarm_add_event_init(&guider_ui);
}
