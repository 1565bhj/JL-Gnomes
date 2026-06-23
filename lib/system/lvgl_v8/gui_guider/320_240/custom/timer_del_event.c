#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdbool.h"
#include "gui_guider.h"
#include "custom.h"
#include "timer.h"

// 函数原型声明
static void timer_del_button_event_handler(lv_event_t *e);
static void timer_del_return_event_handler(lv_event_t *e);
static void timer_del_all_choice_event_handler(lv_event_t *e);
static void timer_del_remove_event_handler(lv_event_t *e);
static void clear_timer_del_events_and_selection(lv_ui *ui);
static bool all_selected = false;

/**
 * 刷新倒计时删除页面的倒计时列表
 * @param ui: GUI全局对象
 */
void refresh_timer_del_page(lv_ui *ui)
{
    // 1. 清除当前所有动态创建的倒计时条目
    lv_obj_t *cont = ui->screen_timer_remove_cont;
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

    // 2. 遍历全局倒计时数组，动态创建倒计时条目
    for (int i = 0; i < g_timer_count; i++) {
        // 检查倒计时是否有效
        if (g_timers[i].cont == NULL) {
            continue;
        }

        // 计算新倒计时条目的Y轴位置（从1开始计数，Y轴间隔51px）
        int y_pos = 0 + i * 61; // Y轴位置，与倒计时主页面保持一致

        // 创建倒计时条目容器
        lv_obj_t *timer_cont = lv_obj_create(ui->screen_timer_remove_cont);
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
        lv_style_set_radius(&cont_style, 0);
        lv_style_set_bg_opa(&cont_style, 53);
        lv_style_set_bg_color(&cont_style, lv_color_hex(0xffffff));
        lv_style_set_bg_grad_dir(&cont_style, LV_GRAD_DIR_NONE);
        lv_style_set_pad_top(&cont_style, 0);
        lv_style_set_pad_bottom(&cont_style, 0);
        lv_style_set_pad_left(&cont_style, 0);
        lv_style_set_pad_right(&cont_style, 0);
        lv_style_set_shadow_width(&cont_style, 0);
        lv_obj_add_style(timer_cont, &cont_style, 0);
        lv_obj_set_style_radius(timer_cont, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

        // 创建时间标签组（参考Dynamic_add_new_timer的布局）
        lv_obj_t *time_group = lv_spangroup_create(timer_cont);
        lv_spangroup_set_align((lv_obj_t *)time_group, LV_TEXT_ALIGN_LEFT);
        lv_spangroup_set_overflow((lv_obj_t *)time_group, LV_SPAN_OVERFLOW_CLIP);
        lv_spangroup_set_mode((lv_obj_t *)time_group, LV_SPAN_MODE_EXPAND);

        // 设置时间标签组位置和大小（与Dynamic_add_new_timer保持一致）
        lv_obj_set_pos((lv_obj_t *)time_group, 12, 9);
        lv_obj_set_size((lv_obj_t *)time_group, 57, 25);

        // 创建时间文本span
        lv_span_t *time_span = lv_spangroup_new_span((lv_obj_t *)time_group);

        // 格式化倒计时时间（时:分:秒）- 使用remaining_seconds显示真实剩余时间
        char time_str[9];
        int hours = g_timers[i].remaining_seconds / 3600;
        int minutes = (g_timers[i].remaining_seconds % 3600) / 60;
        int seconds = g_timers[i].remaining_seconds % 60;
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
        lv_obj_add_style((lv_obj_t *)time_group, &span_group_style, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_spangroup_refr_mode((lv_obj_t *)time_group);

        // 创建删除选择按钮（替换开关位置，使用imgbtn_del）
        lv_obj_t *imgbtn_del = lv_imgbtn_create(timer_cont);
        lv_obj_add_flag(imgbtn_del, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(imgbtn_del, LV_OBJ_FLAG_CHECKABLE);
        lv_imgbtn_set_src(imgbtn_del, LV_IMGBTN_STATE_RELEASED, NULL, &_un_press_alpha_20x20, NULL);
        lv_imgbtn_set_src(imgbtn_del, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &_alarm_del_alpha_20x20, NULL);

        // 创建按钮标签
        lv_obj_t *imgbtn_del_label = lv_label_create(imgbtn_del);
        lv_label_set_text(imgbtn_del_label, "");
        lv_label_set_long_mode(imgbtn_del_label, LV_LABEL_LONG_WRAP);
        lv_obj_align(imgbtn_del_label, LV_ALIGN_CENTER, 0, 0);

        // 设置按钮内边距
        lv_obj_set_style_pad_all(imgbtn_del, 0, LV_STATE_DEFAULT);

        // 设置删除按钮位置和大小
        lv_obj_set_pos(imgbtn_del, 221, 14);
        lv_obj_set_size(imgbtn_del, 20, 20);

        // 设置按钮默认样式
        lv_obj_set_style_text_color(imgbtn_del, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(imgbtn_del, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_opa(imgbtn_del, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_align(imgbtn_del, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(imgbtn_del, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_clip_corner(imgbtn_del, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(imgbtn_del, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        // 设置按钮按下状态样式
        lv_obj_set_style_img_recolor_opa(imgbtn_del, 0, LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_set_style_img_opa(imgbtn_del, 255, LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_set_style_text_color(imgbtn_del, lv_color_hex(0xFF33FF), LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_set_style_text_font(imgbtn_del, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_set_style_text_opa(imgbtn_del, 255, LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_set_style_shadow_width(imgbtn_del, 0, LV_PART_MAIN | LV_STATE_PRESSED);

        // 设置按钮选中状态样式
        lv_obj_set_style_img_recolor_opa(imgbtn_del, 0, LV_PART_MAIN | LV_STATE_CHECKED);
        lv_obj_set_style_img_opa(imgbtn_del, 255, LV_PART_MAIN | LV_STATE_CHECKED);
        lv_obj_set_style_text_color(imgbtn_del, lv_color_hex(0xFF33FF), LV_PART_MAIN | LV_STATE_CHECKED);
        lv_obj_set_style_text_font(imgbtn_del, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_CHECKED);
        lv_obj_set_style_text_opa(imgbtn_del, 255, LV_PART_MAIN | LV_STATE_CHECKED);
        lv_obj_set_style_shadow_width(imgbtn_del, 0, LV_PART_MAIN | LV_STATE_CHECKED);

        // 设置按钮释放状态样式
        lv_obj_set_style_img_recolor_opa(imgbtn_del, 0, LV_PART_MAIN | LV_IMGBTN_STATE_RELEASED);
        lv_obj_set_style_img_opa(imgbtn_del, 255, LV_PART_MAIN | LV_IMGBTN_STATE_RELEASED);

        // 根据倒计时的is_selected状态设置按钮的初始状态
        if (g_timers[i].is_selected) {
            lv_imgbtn_set_state(imgbtn_del, LV_IMGBTN_STATE_CHECKED_RELEASED);
        } else {
            lv_imgbtn_set_state(imgbtn_del, LV_IMGBTN_STATE_RELEASED);
        }

        // 设置删除按钮事件
        lv_obj_add_event_cb(imgbtn_del, timer_del_button_event_handler, LV_EVENT_CLICKED, &g_timers[i]);

        // 标记为动态创建的对象，方便后续删除
        lv_obj_set_user_data(imgbtn_del, (void *)0x1);
    }

    // 检查是否有倒计时，没有则禁用全选按钮
    if (g_timer_count == 0) {
        // 同时设置禁用状态和移除可点击标志
        lv_obj_add_state(ui->screen_timer_remove_imgbtn_choice, LV_STATE_DISABLED);
        lv_obj_clear_flag(ui->screen_timer_remove_imgbtn_choice, LV_OBJ_FLAG_CLICKABLE);
        all_selected = false;
    } else {
        // 同时清除禁用状态和添加可点击标志
        lv_obj_clear_state(ui->screen_timer_remove_imgbtn_choice, LV_STATE_DISABLED);
        lv_obj_add_flag(ui->screen_timer_remove_imgbtn_choice, LV_OBJ_FLAG_CLICKABLE);
    }
}
// 单个倒计时删除按钮点击事件处理函数
static void timer_del_button_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED) {
        // 当手动选择/取消选择单个倒计时时，全选按钮恢复未按下状态
        lv_imgbtn_set_state(guider_ui.screen_timer_remove_imgbtn_choice, LV_IMGBTN_STATE_RELEASED);
        all_selected = false;
        // 获取当前点击的倒计时对象
        TimerItem *current_timer = (TimerItem *)lv_event_get_user_data(e);

        // 切换选择状态
        current_timer->is_selected = !current_timer->is_selected;

        // 更新按钮图片状态
        if (current_timer->is_selected) {
            lv_imgbtn_set_state(btn, LV_IMGBTN_STATE_CHECKED_RELEASED);
        } else {
            lv_imgbtn_set_state(btn, LV_IMGBTN_STATE_RELEASED);
        }

        // 统计已选择的项数和实际存在的倒计时数量
        int selected_count = 0;
        bool all_timers_selected = true;

        // 使用g_timer_count来遍历实际存在的倒计时
        for (int i = 0; i < g_timer_count; i++) {
            if (g_timers[i].is_selected) {
                selected_count++;
            } else {
                all_timers_selected = false;
            }
        }

        // 如果手动选中了所有实际存在的倒计时，设置全选按钮为选中状态
        if (all_timers_selected && selected_count > 0 && selected_count == g_timer_count) {
            lv_imgbtn_set_state(guider_ui.screen_timer_remove_imgbtn_choice, LV_IMGBTN_STATE_CHECKED_RELEASED);
            all_selected = true;
        }

        // 更新选择文本显示
        char choice_text[50];
        if (selected_count > 0) {
            sprintf(choice_text, "已选择%d项", selected_count);
        } else {
            sprintf(choice_text, "未选择");
        }

        // 设置选择文本
        lv_span_set_text(guider_ui.screen_timer_remove_choice_span, choice_text);
    }
}

// 全选按钮点击事件处理函数
static void timer_del_all_choice_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        // 检查当前是否所有倒计时都已被选中
        all_selected = true;
        for (int i = 0; i < g_timer_count; i++) {
            if (!g_timers[i].is_selected) {
                all_selected = false;
                break;
            }
        }

        // 切换全选/取消全选状态
        all_selected = !all_selected;

        // 更新所有倒计时的选择状态
        for (int i = 0; i < g_timer_count; i++) {
            g_timers[i].is_selected = all_selected;
        }

        // 更新全选按钮状态
        if (all_selected) {
            lv_imgbtn_set_state(guider_ui.screen_timer_remove_imgbtn_choice, LV_IMGBTN_STATE_CHECKED_RELEASED);
        } else {
            lv_imgbtn_set_state(guider_ui.screen_timer_remove_imgbtn_choice, LV_IMGBTN_STATE_RELEASED);
        }

        // 刷新页面以更新选择状态
        refresh_timer_del_page(&guider_ui);

        // 更新选择文本显示
        char choice_text[50];
        if (all_selected) {
            sprintf(choice_text, "已选择%d项", g_timer_count);
        } else {
            sprintf(choice_text, "未选择");
        }

        // 设置选择文本
        lv_span_set_text(guider_ui.screen_timer_remove_choice_span, choice_text);
    }
}

// 删除选中倒计时的函数
static void timer_del_remove_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *img_del = lv_event_get_target(e);

    if (code == LV_EVENT_PRESSED) {
        // 按下时放大图片（1.1倍）
        lv_img_set_zoom(img_del, 282); // 256 * 1.1 ≈ 282
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        // 松开或失去焦点时恢复原始大小
        lv_img_set_zoom(img_del, 256); // LV_IMG_ZOOM_NONE = 256

        if (code == LV_EVENT_RELEASED) {
            // 遍历倒计时数组，从后往前删除选中的倒计时
            int removed_count = 0;
            for (int i = g_timer_count - 1; i >= 0; i--) {
                if (g_timers[i].is_selected) {
                    // 删除倒计时容器
                    if (g_timers[i].cont) {
                        lv_obj_del(g_timers[i].cont);
                        g_timers[i].cont = NULL;
                    }

                    // 从数组中移除并重新排列
                    for (int j = i; j < g_timer_count - 1; j++) {
                        g_timers[j] = g_timers[j + 1];
                    }

                    removed_count++;
                    g_timer_count--;
                }
            }

            // 刷新删除页面和主页面
            refresh_timer_del_page(&guider_ui);
            refresh_timer_page(&guider_ui);

            // 重置全选状态
            all_selected = false;
            lv_imgbtn_set_state(guider_ui.screen_timer_remove_imgbtn_choice, LV_IMGBTN_STATE_RELEASED);

            // 更新选择文本显示
            lv_span_set_text(guider_ui.screen_timer_remove_choice_span, "未选择");

            // 通知主页面更新
            printf("-> 倒计时删除成功，删除数量: %d，剩余数量: %d\n", removed_count, g_timer_count);
        }
    }
}

// 清除所有倒计时删除页面的选择状态和事件
static void clear_timer_del_events_and_selection(lv_ui *ui)
{
    if (!ui) {
        return;
    }

    // 重置全选状态
    all_selected = false;
    lv_imgbtn_set_state(ui->screen_timer_remove_imgbtn_choice, LV_IMGBTN_STATE_RELEASED);

    // 清除所有动态创建的倒计时的选择状态
    // 遍历全局倒计时数组，重置选择状态
    for (int i = 0; i < g_timer_count; i++) {
        g_timers[i].is_selected = false;
    }

    // 清除动态创建的删除按钮的选择状态
    // 获取倒计时容器
    lv_obj_t *cont = ui->screen_timer_remove_cont;
    if (!cont) {
        return;
    }

    // 遍历容器中的所有子对象
    uint32_t child_count = lv_obj_get_child_cnt(cont);
    for (uint32_t i = 0; i < child_count; i++) {
        lv_obj_t *child = lv_obj_get_child(cont, i);
        if (!child) {
            continue;
        }

        // 检查是否为动态创建的倒计时条目容器
        if (lv_obj_get_user_data(child) == (void *)1) {
            // 遍历容器中的子对象，查找删除按钮
            uint32_t grandchild_count = lv_obj_get_child_cnt(child);
            for (uint32_t j = 0; j < grandchild_count; j++) {
                lv_obj_t *grandchild = lv_obj_get_child(child, j);
                if (!grandchild) {
                    continue;
                }

                // 检查是否为动态创建的删除按钮
                if (lv_obj_get_user_data(grandchild) == (void *)0x1) {
                    // 重置按钮状态为未选中
                    lv_imgbtn_set_state(grandchild, LV_IMGBTN_STATE_RELEASED);
                }
            }
        }
    }

    // 更新选择文本显示
    lv_span_set_text(ui->screen_timer_remove_choice_span, "未选择");
}

// 倒计时删除页面返回按钮点击事件处理函数
static void timer_del_return_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if (!ui || !ui->screen_timer_remove_return) {
        return;
    }
    if (code == LV_EVENT_CLICKED) {
        // 切换回倒计时主页面
        clear_timer_del_events_and_selection(ui);
        switch_to_timer_page();
    }
}

// 初始化删除倒计时事件
void timer_del_event_init(lv_ui *ui)
{
    // 隐藏静态创建的倒计时容器（cont1、cont2、cont3）
    lv_obj_add_flag(ui->screen_timer_remove_cont_1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->screen_timer_remove_cont_2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->screen_timer_remove_cont_3, LV_OBJ_FLAG_HIDDEN);
    // 注册返回按钮点击事件
    lv_obj_remove_event_cb(ui->screen_timer_remove_return, timer_del_return_event_handler);
    lv_obj_add_event_cb(ui->screen_timer_remove_return, timer_del_return_event_handler, LV_EVENT_CLICKED, ui);

    // 注册全选按钮点击事件
    lv_obj_remove_event_cb(ui->screen_timer_remove_imgbtn_choice, timer_del_all_choice_event_handler);
    lv_obj_add_event_cb(ui->screen_timer_remove_imgbtn_choice, timer_del_all_choice_event_handler, LV_EVENT_CLICKED, ui);

    // 注册删除图片点击事件
    lv_obj_remove_event_cb(ui->screen_timer_remove_img_del, timer_del_remove_event_handler);
    lv_obj_add_event_cb(ui->screen_timer_remove_img_del, timer_del_remove_event_handler, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui->screen_timer_remove_img_del, timer_del_remove_event_handler, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(ui->screen_timer_remove_img_del, timer_del_remove_event_handler, LV_EVENT_PRESS_LOST, NULL);
    lv_obj_add_event_cb(ui->screen_timer_remove_img_del, timer_del_remove_event_handler, LV_EVENT_CLICKED, ui);

    // 初始化选择文本
    lv_span_set_text(ui->screen_timer_remove_choice_span, "未选择");
    // 刷新倒计时列表
    refresh_timer_del_page(ui);
    clear_timer_del_events_and_selection(ui);
}

void timer_del_event_setup(void)
{
    timer_del_event_init(&guider_ui);

}

