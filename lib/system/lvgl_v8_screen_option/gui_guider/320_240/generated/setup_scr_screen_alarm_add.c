/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"



void setup_scr_screen_alarm_add(lv_ui *ui)
{
    //Write codes screen_alarm_add
    ui->screen_alarm_add = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_alarm_add, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_alarm_add, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_alarm_add, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_alarm_add, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_add, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_add, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_add_roller_min
    ui->screen_alarm_add_roller_min = lv_roller_create(ui->screen_alarm_add);
    lv_roller_set_options(ui->screen_alarm_add_roller_min, "1\n2\n3\n4\n5", LV_ROLLER_MODE_INFINITE);
    lv_obj_set_pos(ui->screen_alarm_add_roller_min, 181, 58);
    lv_obj_set_width(ui->screen_alarm_add_roller_min, 80);

    //Write style for screen_alarm_add_roller_min, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_radius(ui->screen_alarm_add_roller_min, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_alarm_add_roller_min, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_add_roller_min, lv_color_hex(0x5d4646), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_add_roller_min, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_alarm_add_roller_min, lv_color_hex(0x7e7e7e), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_alarm_add_roller_min, &lv_font_Barlow__18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_alarm_add_roller_min, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_alarm_add_roller_min, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_alarm_add_roller_min, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_alarm_add_roller_min, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_alarm_add_roller_min, lv_color_hex(0xe6e6e6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_alarm_add_roller_min, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_alarm_add_roller_min, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_alarm_add_roller_min, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_add_roller_min, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style for screen_alarm_add_roller_min, Part: LV_PART_SELECTED, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_alarm_add_roller_min, 84, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_add_roller_min, lv_color_hex(0xcbcbcb), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_add_roller_min, LV_GRAD_DIR_NONE, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_alarm_add_roller_min, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_alarm_add_roller_min, &lv_font_Barlow__24, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_alarm_add_roller_min, 255, LV_PART_SELECTED | LV_STATE_DEFAULT);

    lv_roller_set_visible_row_count(ui->screen_alarm_add_roller_min, 3);
    //Write codes screen_alarm_add_roller_hour
    ui->screen_alarm_add_roller_hour = lv_roller_create(ui->screen_alarm_add);
    lv_roller_set_options(ui->screen_alarm_add_roller_hour, "1\n2\n3\n4\n5", LV_ROLLER_MODE_INFINITE);
    lv_obj_set_pos(ui->screen_alarm_add_roller_hour, 62, 58);
    lv_obj_set_width(ui->screen_alarm_add_roller_hour, 80);

    //Write style for screen_alarm_add_roller_hour, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_radius(ui->screen_alarm_add_roller_hour, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_alarm_add_roller_hour, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_add_roller_hour, lv_color_hex(0x5d4646), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_add_roller_hour, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_alarm_add_roller_hour, lv_color_hex(0x7e7e7e), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_alarm_add_roller_hour, &lv_font_Barlow__18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_alarm_add_roller_hour, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_alarm_add_roller_hour, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_alarm_add_roller_hour, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_alarm_add_roller_hour, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_alarm_add_roller_hour, lv_color_hex(0xe6e6e6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_alarm_add_roller_hour, LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_alarm_add_roller_hour, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_alarm_add_roller_hour, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_add_roller_hour, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style for screen_alarm_add_roller_hour, Part: LV_PART_SELECTED, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_alarm_add_roller_hour, 83, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_add_roller_hour, lv_color_hex(0xcbcbcb), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_add_roller_hour, LV_GRAD_DIR_NONE, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_alarm_add_roller_hour, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_alarm_add_roller_hour, &lv_font_Barlow__24, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_alarm_add_roller_hour, 255, LV_PART_SELECTED | LV_STATE_DEFAULT);

    lv_roller_set_visible_row_count(ui->screen_alarm_add_roller_hour, 3);
    //Write codes screen_alarm_add_yes
    ui->screen_alarm_add_yes = lv_img_create(ui->screen_alarm_add);
    lv_obj_add_flag(ui->screen_alarm_add_yes, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_alarm_add_yes, &_yes_alpha_30x30);
    lv_img_set_pivot(ui->screen_alarm_add_yes, 50, 50);
    lv_img_set_angle(ui->screen_alarm_add_yes, 0);
    lv_obj_set_pos(ui->screen_alarm_add_yes, 249, 15);
    lv_obj_set_size(ui->screen_alarm_add_yes, 30, 30);

    //Write style for screen_alarm_add_yes, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_alarm_add_yes, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_alarm_add_yes, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_add_yes, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_alarm_add_yes, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_add_add_alarm_name
    ui->screen_alarm_add_add_alarm_name = lv_spangroup_create(ui->screen_alarm_add);
    lv_spangroup_set_align(ui->screen_alarm_add_add_alarm_name, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_alarm_add_add_alarm_name, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_alarm_add_add_alarm_name, LV_SPAN_MODE_BREAK);
    //create span
    ui->screen_alarm_add_add_alarm_name_span = lv_spangroup_new_span(ui->screen_alarm_add_add_alarm_name);
    lv_span_set_text(ui->screen_alarm_add_add_alarm_name_span, "新建闹钟");
    lv_style_set_text_color(&ui->screen_alarm_add_add_alarm_name_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_alarm_add_add_alarm_name_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_alarm_add_add_alarm_name_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_alarm_add_add_alarm_name, 115, 20);
    lv_obj_set_size(ui->screen_alarm_add_add_alarm_name, 73, 18);

    //Write style state: LV_STATE_DEFAULT for &style_screen_alarm_add_add_alarm_name_main_main_default
    static lv_style_t style_screen_alarm_add_add_alarm_name_main_main_default;
    ui_init_style(&style_screen_alarm_add_add_alarm_name_main_main_default);

    lv_style_set_border_width(&style_screen_alarm_add_add_alarm_name_main_main_default, 0);
    lv_style_set_radius(&style_screen_alarm_add_add_alarm_name_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_alarm_add_add_alarm_name_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_alarm_add_add_alarm_name_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_alarm_add_add_alarm_name_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_alarm_add_add_alarm_name_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_alarm_add_add_alarm_name_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_alarm_add_add_alarm_name_main_main_default, 0);
    lv_obj_add_style(ui->screen_alarm_add_add_alarm_name, &style_screen_alarm_add_add_alarm_name_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_alarm_add_add_alarm_name);

    //Write codes screen_alarm_add_return
    ui->screen_alarm_add_return = lv_img_create(ui->screen_alarm_add);
    lv_obj_add_flag(ui->screen_alarm_add_return, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_alarm_add_return, &_return_alpha_22x22);
    lv_img_set_pivot(ui->screen_alarm_add_return, 50, 50);
    lv_img_set_angle(ui->screen_alarm_add_return, 0);
    lv_obj_set_pos(ui->screen_alarm_add_return, 30, 20);
    lv_obj_set_size(ui->screen_alarm_add_return, 22, 22);

    //Write style for screen_alarm_add_return, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_alarm_add_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_alarm_add_return, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_add_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_alarm_add_return, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_add_cont_repeat
    ui->screen_alarm_add_cont_repeat = lv_obj_create(ui->screen_alarm_add);
    lv_obj_set_pos(ui->screen_alarm_add_cont_repeat, 44, 185);
    lv_obj_set_size(ui->screen_alarm_add_cont_repeat, 239, 41);
    lv_obj_set_scrollbar_mode(ui->screen_alarm_add_cont_repeat, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_alarm_add_cont_repeat, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_alarm_add_cont_repeat, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_alarm_add_cont_repeat, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_alarm_add_cont_repeat, lv_color_hex(0x526779), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_alarm_add_cont_repeat, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_add_cont_repeat, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_alarm_add_cont_repeat, 53, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_add_cont_repeat, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_add_cont_repeat, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_alarm_add_cont_repeat, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_alarm_add_cont_repeat, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_alarm_add_cont_repeat, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_alarm_add_cont_repeat, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_add_cont_repeat, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_add_repeat_name
    ui->screen_alarm_add_repeat_name = lv_spangroup_create(ui->screen_alarm_add_cont_repeat);
    lv_spangroup_set_align(ui->screen_alarm_add_repeat_name, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_alarm_add_repeat_name, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_alarm_add_repeat_name, LV_SPAN_MODE_BREAK);
    //create span
    ui->screen_alarm_add_repeat_name_span = lv_spangroup_new_span(ui->screen_alarm_add_repeat_name);
    lv_span_set_text(ui->screen_alarm_add_repeat_name_span, "重复");
    lv_style_set_text_color(&ui->screen_alarm_add_repeat_name_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_alarm_add_repeat_name_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_alarm_add_repeat_name_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_alarm_add_repeat_name, 15, 9);
    lv_obj_set_size(ui->screen_alarm_add_repeat_name, 45, 21);

    //Write style state: LV_STATE_DEFAULT for &style_screen_alarm_add_repeat_name_main_main_default
    static lv_style_t style_screen_alarm_add_repeat_name_main_main_default;
    ui_init_style(&style_screen_alarm_add_repeat_name_main_main_default);

    lv_style_set_border_width(&style_screen_alarm_add_repeat_name_main_main_default, 0);
    lv_style_set_radius(&style_screen_alarm_add_repeat_name_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_alarm_add_repeat_name_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_alarm_add_repeat_name_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_alarm_add_repeat_name_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_alarm_add_repeat_name_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_alarm_add_repeat_name_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_alarm_add_repeat_name_main_main_default, 0);
    lv_obj_add_style(ui->screen_alarm_add_repeat_name, &style_screen_alarm_add_repeat_name_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_alarm_add_repeat_name);

    //Write codes screen_alarm_add_enter
    ui->screen_alarm_add_enter = lv_img_create(ui->screen_alarm_add_cont_repeat);
    lv_obj_add_flag(ui->screen_alarm_add_enter, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_alarm_add_enter, &_enter_alpha_24x17);
    lv_img_set_pivot(ui->screen_alarm_add_enter, 50, 50);
    lv_img_set_angle(ui->screen_alarm_add_enter, 0);
    lv_obj_set_pos(ui->screen_alarm_add_enter, 199, 10);
    lv_obj_set_size(ui->screen_alarm_add_enter, 24, 17);

    //Write style for screen_alarm_add_enter, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_alarm_add_enter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_alarm_add_enter, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_add_enter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_alarm_add_enter, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_add_dot
    ui->screen_alarm_add_dot = lv_spangroup_create(ui->screen_alarm_add);
    lv_spangroup_set_align(ui->screen_alarm_add_dot, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_alarm_add_dot, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_alarm_add_dot, LV_SPAN_MODE_BREAK);
    //create span
    ui->screen_alarm_add_dot_span = lv_spangroup_new_span(ui->screen_alarm_add_dot);
    lv_span_set_text(ui->screen_alarm_add_dot_span, ":");
    lv_style_set_text_color(&ui->screen_alarm_add_dot_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_alarm_add_dot_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_alarm_add_dot_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_alarm_add_dot, 158, 100);
    lv_obj_set_size(ui->screen_alarm_add_dot, 5, 21);

    //Write style state: LV_STATE_DEFAULT for &style_screen_alarm_add_dot_main_main_default
    static lv_style_t style_screen_alarm_add_dot_main_main_default;
    ui_init_style(&style_screen_alarm_add_dot_main_main_default);

    lv_style_set_border_width(&style_screen_alarm_add_dot_main_main_default, 0);
    lv_style_set_radius(&style_screen_alarm_add_dot_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_alarm_add_dot_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_alarm_add_dot_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_alarm_add_dot_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_alarm_add_dot_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_alarm_add_dot_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_alarm_add_dot_main_main_default, 0);
    lv_obj_add_style(ui->screen_alarm_add_dot, &style_screen_alarm_add_dot_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_alarm_add_dot);

    //The custom code of screen_alarm_add.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_alarm_add);

    //Init events for screen.
    events_init_screen_alarm_add(ui);
}
