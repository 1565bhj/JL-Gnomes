#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdbool.h"
#include "gui_guider.h"
#include "custom.h"
#include "alarm.h"




// 用于跟踪p_circle, p_once, p_set的样式状态
static bool g_p_circle_style_state = false;
static bool g_p_once_style_state = false;
static bool g_p_set_style_state = false;

// 用于跟踪btn_1到btn_7的样式状态
static bool g_btn_style_states[7] = {false, false, false, false, false, false, false};

// 全局变量用于存储从alarm_add页面传递过来的时间
int g_alarm_hour = 0;
int g_alarm_minute = 0;

static void show_weekday_selector(lv_ui *ui);
static void hide_weekday_selector(lv_ui *ui);
static void reset_alarm_set_options(lv_ui *ui);

// 使用alarm.h中定义的结构体字段存储闹钟重复状态
extern bool g_weekdays_selected[7]; // 周一到周日的选中状态（在alarm.h中声明）

void extern_reset_alarm_set_options(void)
{
    reset_alarm_set_options(&guider_ui);
}
// 重置所有重复选项按钮的状态
static void reset_repeat_buttons(lv_ui *ui)
{
    // 重置按钮状态变量
    g_p_circle_style_state = false;
    g_p_once_style_state = false;
    g_p_set_style_state = false;

    // 重置所有按钮的样式
    // p_circle
    lv_obj_set_style_bg_opa(ui->screen_alarm_set_p_circle, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_set_p_circle, lv_color_hex(0xe8e8e8), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_alarm_set_p_circle, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_alarm_set_p_circle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_alarm_set_p_circle, lv_color_hex(0xc1c1c1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(ui->screen_alarm_set_p_circle_label, "");

    // p_once
    lv_obj_set_style_bg_opa(ui->screen_alarm_set_p_once, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_set_p_once, lv_color_hex(0xe8e8e8), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_alarm_set_p_once, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_alarm_set_p_once, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_alarm_set_p_once, lv_color_hex(0xc1c1c1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(ui->screen_alarm_set_p_once_label, "");

    // p_set
    lv_obj_set_style_bg_opa(ui->screen_alarm_set_p_set, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_set_p_set, lv_color_hex(0xe8e8e8), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_alarm_set_p_set, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_alarm_set_p_set, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_alarm_set_p_set, lv_color_hex(0xc1c1c1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(ui->screen_alarm_set_p_set_label, "");

    // 隐藏星期选择器
    hide_weekday_selector(ui);
    // 重置g_weekdays_selected数组，避免影响下一次设置
    for (int i = 0; i < 7; i++) {
        g_weekdays_selected[i] = false;
    }
    // 重置重复状态
    g_alarms[g_current_alarm_index].repeat_status = 0;
}

// 重置闹钟设置页面所有选项的按压状态
static void reset_alarm_set_options(lv_ui *ui)
{
    reset_repeat_buttons(ui);
    // 可以在这里添加其他需要重置的选项
}

// 确认按钮点击事件处理函数
static void alarm_set_yes_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED) {
        printf("-> 确认设置闹钟，重复状态: %d\n", g_alarms[g_current_alarm_index].repeat_status);

        // 保存闹钟设置，传递星期选择信息
        int repeat_status = 0;

        // 检查重复状态
        if (g_alarms[g_current_alarm_index].repeat_status == 1) {
            repeat_status = 1; // 每天
        } else if (g_alarms[g_current_alarm_index].repeat_status == 0) {
            repeat_status = 0; // 只响一次
        } else {
            // 打印当前g_weekdays_selected数组的值，用于调试
            printf("-> 当前g_weekdays_selected数组的值: ");
            for (int i = 0; i < 7; i++) {
                printf("%d ", g_weekdays_selected[i]);
            }
            printf("\n");

            // 根据选中的星期计算repeat_status
            for (int i = 0; i < 7; i++) {
                if (g_weekdays_selected[i]) {
                    repeat_status |= (1 << (i + 1)); // 对应WEEK_MONDAY到WEEK_SUNDAY
                }
            }
        }

        // 重置g_weekdays_selected数组，避免影响下一次设置
        for (int i = 0; i < 7; i++) {
            g_weekdays_selected[i] = false;
        }

        printf("-> 计算得到的repeat_status: 0x%02x\n", repeat_status);

        Sync_ui_dynamic_add_new_alarm(ui, g_alarm_hour, g_alarm_minute, 0, g_current_alarm_index, repeat_status);
        Sync_app_dynamic_add_new_alarm(ui, g_alarm_hour, g_alarm_minute, 0, g_current_alarm_index, repeat_status);
        // 切换回闹钟主页面
        switch_to_alarm_page();
        reset_alarm_set_options(ui);

    }
}

// 返回按钮点击事件处理函数
static void alarm_set_return_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED) {
        printf("-> 返回上一页\n");
        // 返回上一页
        switch_to_alarm_page();
        g_alarms[g_current_alarm_index].repeat_status = 0; // 无重复
        reset_alarm_set_options(ui);
    }
}


//每天选项点击事件处理函数
static void alarm_set_daily_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED) {
        // 如果已经选中，则取消选中
        if (g_p_circle_style_state) {
            // 重置所有重复选项按钮
            reset_repeat_buttons(ui);
            // 更新重复状态
            g_alarms[g_current_alarm_index].repeat_status = 0; // 无重复
            return;
        }

        // 重置所有重复选项按钮
        reset_repeat_buttons(ui);

        // 设置当前按钮为选中状态
        g_p_circle_style_state = true;

        // 切换到选中样式
        lv_obj_set_style_bg_opa(ui->screen_alarm_set_p_circle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(ui->screen_alarm_set_p_circle, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(ui->screen_alarm_set_p_circle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(ui->screen_alarm_set_p_circle, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
        // 同时设置label样式和文本
        lv_obj_set_style_text_color(ui->screen_alarm_set_p_circle, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(ui->screen_alarm_set_p_circle, &lv_font_Barlow__12, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_opa(ui->screen_alarm_set_p_circle_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        // 添加LV_SYMBOL_OK符号
        lv_label_set_text(ui->screen_alarm_set_p_circle_label, " " LV_SYMBOL_OK " ");

        // 更新重复状态
        g_alarms[g_current_alarm_index].repeat_status = 1; // 每天
    }
}

//  只响一次选项点击事件处理函数
static void alarm_set_once_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED) {
        // 如果已经选中，则取消选中
        if (g_p_once_style_state) {
            // 重置所有重复选项按钮
            reset_repeat_buttons(ui);
            // 更新重复状态
            g_alarms[g_current_alarm_index].repeat_status = 0; // 无重复
            return;
        }

        // 重置所有重复选项按钮
        reset_repeat_buttons(ui);

        // 设置当前按钮为选中状态
        g_p_once_style_state = true;

        // 切换到选中样式
        lv_obj_set_style_bg_opa(ui->screen_alarm_set_p_once, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(ui->screen_alarm_set_p_once, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(ui->screen_alarm_set_p_once, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(ui->screen_alarm_set_p_once, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
        // 同时设置label样式和文本
        lv_obj_set_style_text_color(ui->screen_alarm_set_p_once, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(ui->screen_alarm_set_p_once, &lv_font_Barlow__12, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_opa(ui->screen_alarm_set_p_once_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        // 添加LV_SYMBOL_OK符号
        lv_label_set_text(ui->screen_alarm_set_p_once_label, " " LV_SYMBOL_OK " ");

        // 更新重复状态
        g_alarms[g_current_alarm_index].repeat_status = 0; // 只响一次
    }
}

// 自定义日期选项点击事件处理函数
static void alarm_set_custom_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED) {
        // 如果已经选中，则取消选中
        if (g_p_set_style_state) {
            // 重置所有重复选项按钮
            reset_repeat_buttons(ui);
            // 更新重复状态
            g_alarms[g_current_alarm_index].repeat_status = 0; // 无重复
            return;
        }

        // 重置所有重复选项按钮
        reset_repeat_buttons(ui);

        // 设置当前按钮为选中状态
        g_p_set_style_state = true;

        // 切换到选中样式
        lv_obj_set_style_bg_opa(ui->screen_alarm_set_p_set, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(ui->screen_alarm_set_p_set, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(ui->screen_alarm_set_p_set, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(ui->screen_alarm_set_p_set, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
        // 同时设置label样式和文本
        lv_obj_set_style_text_color(ui->screen_alarm_set_p_set, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(ui->screen_alarm_set_p_set, &lv_font_Barlow__12, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_opa(ui->screen_alarm_set_p_set_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        // 添加LV_SYMBOL_OK符号
        lv_label_set_text(ui->screen_alarm_set_p_set_label, " " LV_SYMBOL_OK " ");

        // 显示星期选择器
        show_weekday_selector(ui);

        // 更新重复状态
        g_alarms[g_current_alarm_index].repeat_status = 2; // 自定义
    }
}
static void show_weekday_selector(lv_ui *ui)
{
    // 显示btn1-7按钮
    lv_obj_clear_flag(ui->screen_alarm_set_btn_1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui->screen_alarm_set_btn_2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui->screen_alarm_set_btn_3, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui->screen_alarm_set_btn_4, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui->screen_alarm_set_btn_5, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui->screen_alarm_set_btn_6, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui->screen_alarm_set_btn_7, LV_OBJ_FLAG_HIDDEN);
}
static void hide_weekday_selector(lv_ui *ui)
{
    // 先将所有按钮恢复为未按下状态
    for (int i = 0; i < 7; i++) {
        if (g_btn_style_states[i]) { // 只有处于按下状态的按钮才需要重置
            g_btn_style_states[i] = false; // 重置状态标志
            g_weekdays_selected[i] = false; // 重置选中状态

            // 恢复未按下状态的样式
            switch (i) {
            case 0:
                lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_1, 39, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_color(ui->screen_alarm_set_btn_1, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_1_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                break;
            case 1:
                lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_2, 39, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_color(ui->screen_alarm_set_btn_2, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_2_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                break;
            case 2:
                lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_3, 39, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_color(ui->screen_alarm_set_btn_3, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_3_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                break;
            case 3:
                lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_4, 39, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_color(ui->screen_alarm_set_btn_4, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_4_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                break;
            case 4:
                lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_5, 39, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_color(ui->screen_alarm_set_btn_5, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_5_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                break;
            case 5:
                lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_6, 39, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_color(ui->screen_alarm_set_btn_6, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_6_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                break;
            case 6:
                lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_7, 39, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_color(ui->screen_alarm_set_btn_7, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_7_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                break;
            }
        }
    }

    // 隐藏btn1-7按钮
    lv_obj_add_flag(ui->screen_alarm_set_btn_1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->screen_alarm_set_btn_2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->screen_alarm_set_btn_3, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->screen_alarm_set_btn_4, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->screen_alarm_set_btn_5, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->screen_alarm_set_btn_6, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->screen_alarm_set_btn_7, LV_OBJ_FLAG_HIDDEN);
}
// btn_1到btn_7样式切换事件处理函数
static void alarm_set_btn_weekday_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED) {
        // 确定是哪个按钮被点击
        lv_obj_t *target = lv_event_get_target(e);
        int btn_index = -1;

        if (target == ui->screen_alarm_set_btn_1) {
            btn_index = 0;
        } else if (target == ui->screen_alarm_set_btn_2) {
            btn_index = 1;
        } else if (target == ui->screen_alarm_set_btn_3) {
            btn_index = 2;
        } else if (target == ui->screen_alarm_set_btn_4) {
            btn_index = 3;
        } else if (target == ui->screen_alarm_set_btn_5) {
            btn_index = 4;
        } else if (target == ui->screen_alarm_set_btn_6) {
            btn_index = 5;
        } else if (target == ui->screen_alarm_set_btn_7) {
            btn_index = 6;
        }

        if (btn_index != -1) {
            g_btn_style_states[btn_index] = !g_btn_style_states[btn_index];

            if (g_btn_style_states[btn_index]) {
                // 更新重复状态
                g_weekdays_selected[btn_index] = true;
                // 切换到新样式：背景透明度156，字体颜色ffffff
                if (btn_index == 0) {
                    lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_1, 156, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(ui->screen_alarm_set_btn_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_1_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                } else if (btn_index == 1) {
                    lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_2, 156, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(ui->screen_alarm_set_btn_2, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_2_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                } else if (btn_index == 2) {
                    lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_3, 156, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(ui->screen_alarm_set_btn_3, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_3_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                } else if (btn_index == 3) {
                    lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_4, 156, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(ui->screen_alarm_set_btn_4, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_4_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                } else if (btn_index == 4) {
                    lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_5, 156, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(ui->screen_alarm_set_btn_5, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_5_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                } else if (btn_index == 5) {
                    lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_6, 156, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(ui->screen_alarm_set_btn_6, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_6_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                } else if (btn_index == 6) {
                    lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_7, 156, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(ui->screen_alarm_set_btn_7, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_7_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            } else {
                // 更新重复状态
                g_weekdays_selected[btn_index] = false;
                // 恢复原样式
                if (btn_index == 0) {
                    lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_1, 39, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(ui->screen_alarm_set_btn_1, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_1_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                } else if (btn_index == 1) {
                    lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_2, 39, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(ui->screen_alarm_set_btn_2, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_2_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                } else if (btn_index == 2) {
                    lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_3, 39, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(ui->screen_alarm_set_btn_3, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_3_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                } else if (btn_index == 3) {
                    lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_4, 39, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(ui->screen_alarm_set_btn_4, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_4_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                } else if (btn_index == 4) {
                    lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_5, 39, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(ui->screen_alarm_set_btn_5, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_5_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                } else if (btn_index == 5) {
                    lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_6, 39, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(ui->screen_alarm_set_btn_6, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_6_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                } else if (btn_index == 6) {
                    lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_7, 39, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(ui->screen_alarm_set_btn_7, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_7_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
    }
}

// 初始化闹钟设置页面事件
void alarm_set_event_init(lv_ui *ui)
{
    // 注册确认按钮点击事件
    lv_obj_remove_event_cb(ui->screen_alarm_set_yes, alarm_set_yes_event_handler);
    lv_obj_add_event_cb(ui->screen_alarm_set_yes, alarm_set_yes_event_handler, LV_EVENT_CLICKED, ui);

    // 注册返回按钮点击事件
    lv_obj_remove_event_cb(ui->screen_alarm_set_return, alarm_set_return_event_handler);
    lv_obj_add_event_cb(ui->screen_alarm_set_return, alarm_set_return_event_handler, LV_EVENT_CLICKED, ui);

    // 注册p_circle、p_once、p_set样式切换事件

    lv_obj_remove_event_cb(ui->screen_alarm_set_p_circle, alarm_set_daily_event_handler);
    lv_obj_remove_event_cb(ui->screen_alarm_set_p_once, alarm_set_once_event_handler);
    lv_obj_remove_event_cb(ui->screen_alarm_set_p_set, alarm_set_custom_event_handler);

    lv_obj_add_event_cb(ui->screen_alarm_set_p_circle, alarm_set_daily_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_alarm_set_p_once, alarm_set_once_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_alarm_set_p_set, alarm_set_custom_event_handler, LV_EVENT_CLICKED, ui);

    // 同时为未按下、按下状态和容器添加事件监听器）
    lv_obj_remove_event_cb(ui->screen_alarm_set_cont_circle, alarm_set_daily_event_handler);
    lv_obj_remove_event_cb(ui->screen_alarm_set_cont_once, alarm_set_once_event_handler);
    lv_obj_add_event_cb(ui->screen_alarm_set_cont_circle, alarm_set_daily_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_alarm_set_cont_once, alarm_set_once_event_handler, LV_EVENT_CLICKED, ui);

    // 注册星期几选择事件
    lv_obj_remove_event_cb(ui->screen_alarm_set_btn_1, alarm_set_btn_weekday_event_handler);
    lv_obj_remove_event_cb(ui->screen_alarm_set_btn_2, alarm_set_btn_weekday_event_handler);
    lv_obj_remove_event_cb(ui->screen_alarm_set_btn_3, alarm_set_btn_weekday_event_handler);
    lv_obj_remove_event_cb(ui->screen_alarm_set_btn_4, alarm_set_btn_weekday_event_handler);
    lv_obj_remove_event_cb(ui->screen_alarm_set_btn_5, alarm_set_btn_weekday_event_handler);
    lv_obj_remove_event_cb(ui->screen_alarm_set_btn_6, alarm_set_btn_weekday_event_handler);
    lv_obj_remove_event_cb(ui->screen_alarm_set_btn_7, alarm_set_btn_weekday_event_handler);

    lv_obj_add_event_cb(ui->screen_alarm_set_btn_1, alarm_set_btn_weekday_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_alarm_set_btn_2, alarm_set_btn_weekday_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_alarm_set_btn_3, alarm_set_btn_weekday_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_alarm_set_btn_4, alarm_set_btn_weekday_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_alarm_set_btn_5, alarm_set_btn_weekday_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_alarm_set_btn_6, alarm_set_btn_weekday_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_alarm_set_btn_7, alarm_set_btn_weekday_event_handler, LV_EVENT_CLICKED, ui);


    // 初始化时隐藏星期选择器
    hide_weekday_selector(ui);
    // 添加这行代码完全禁止页面滚动
    lv_obj_set_scroll_dir(ui->screen_alarm_set_cont_set, LV_DIR_NONE);
}

void alarm_set_event_setup(void)
{
    alarm_set_event_init(&guider_ui);
}



