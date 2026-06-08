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



void setup_scr_screen_alarm(lv_ui *ui)
{
    //Write codes screen_alarm
    ui->screen_alarm = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_alarm, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_alarm, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_alarm, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_alarm, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_alarm_add
    ui->screen_alarm_alarm_add = lv_img_create(ui->screen_alarm);
    lv_obj_add_flag(ui->screen_alarm_alarm_add, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_alarm_alarm_add, &_add_alpha_30x30);
    lv_img_set_pivot(ui->screen_alarm_alarm_add, 50, 50);
    lv_img_set_angle(ui->screen_alarm_alarm_add, 0);
    lv_obj_set_pos(ui->screen_alarm_alarm_add, 253, 15);
    lv_obj_set_size(ui->screen_alarm_alarm_add, 30, 30);

    //Write style for screen_alarm_alarm_add, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_alarm_alarm_add, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_alarm_alarm_add, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_alarm_add, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_alarm_alarm_add, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_alarm_del
    ui->screen_alarm_alarm_del = lv_img_create(ui->screen_alarm);
    lv_obj_add_flag(ui->screen_alarm_alarm_del, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_alarm_alarm_del, &_clear_alpha_30x30);
    lv_img_set_pivot(ui->screen_alarm_alarm_del, 50, 50);
    lv_img_set_angle(ui->screen_alarm_alarm_del, 0);
    lv_obj_set_pos(ui->screen_alarm_alarm_del, 33, 15);
    lv_obj_set_size(ui->screen_alarm_alarm_del, 30, 30);

    //Write style for screen_alarm_alarm_del, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_alarm_alarm_del, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_alarm_alarm_del, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_alarm_del, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_alarm_alarm_del, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_name
    ui->screen_alarm_name = lv_spangroup_create(ui->screen_alarm);
    lv_spangroup_set_align(ui->screen_alarm_name, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_alarm_name, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_alarm_name, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_alarm_name_span = lv_spangroup_new_span(ui->screen_alarm_name);
    lv_span_set_text(ui->screen_alarm_name_span, "闹钟");
    lv_style_set_text_color(&ui->screen_alarm_name_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_alarm_name_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_alarm_name_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_alarm_name, 139, 22);
    lv_obj_set_size(ui->screen_alarm_name, 39, 18);

    //Write style state: LV_STATE_DEFAULT for &style_screen_alarm_name_main_main_default
    static lv_style_t style_screen_alarm_name_main_main_default;
    ui_init_style(&style_screen_alarm_name_main_main_default);

    lv_style_set_border_width(&style_screen_alarm_name_main_main_default, 0);
    lv_style_set_radius(&style_screen_alarm_name_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_alarm_name_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_alarm_name_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_alarm_name_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_alarm_name_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_alarm_name_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_alarm_name_main_main_default, 0);
    lv_obj_add_style(ui->screen_alarm_name, &style_screen_alarm_name_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_alarm_name);

    //Write codes screen_alarm_cont
    ui->screen_alarm_cont = lv_obj_create(ui->screen_alarm);
    lv_obj_set_pos(ui->screen_alarm_cont, 28, 60);
    lv_obj_set_size(ui->screen_alarm_cont, 260, 176);
    lv_obj_set_scrollbar_mode(ui->screen_alarm_cont, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_alarm_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_alarm_cont, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_alarm_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_alarm_cont, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_alarm_cont, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_alarm_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_alarm_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_alarm_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_alarm_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_alarm_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_cont_3
    ui->screen_alarm_cont_3 = lv_obj_create(ui->screen_alarm_cont);
    lv_obj_set_pos(ui->screen_alarm_cont_3, 2, 59);
    lv_obj_set_size(ui->screen_alarm_cont_3, 258, 51);
    lv_obj_set_scrollbar_mode(ui->screen_alarm_cont_3, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_alarm_cont_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_alarm_cont_3, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_alarm_cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_alarm_cont_3, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_alarm_cont_3, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_cont_3, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_alarm_cont_3, 53, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_cont_3, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_cont_3, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_alarm_cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_alarm_cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_alarm_cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_alarm_cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_repeat3
    ui->screen_alarm_repeat3 = lv_spangroup_create(ui->screen_alarm_cont_3);
    lv_spangroup_set_align(ui->screen_alarm_repeat3, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_alarm_repeat3, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_alarm_repeat3, LV_SPAN_MODE_BREAK);
    //create span
    ui->screen_alarm_repeat3_span = lv_spangroup_new_span(ui->screen_alarm_repeat3);
    lv_span_set_text(ui->screen_alarm_repeat3_span, "每天");
    lv_style_set_text_color(&ui->screen_alarm_repeat3_span->style, lv_color_hex(0xc4c4c4));
    lv_style_set_text_decor(&ui->screen_alarm_repeat3_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_alarm_repeat3_span->style, &lv_font_MiSansDemibold_12);
    lv_obj_set_pos(ui->screen_alarm_repeat3, 13, 32);
    lv_obj_set_size(ui->screen_alarm_repeat3, 239, 12);

    //Write style state: LV_STATE_DEFAULT for &style_screen_alarm_repeat3_main_main_default
    static lv_style_t style_screen_alarm_repeat3_main_main_default;
    ui_init_style(&style_screen_alarm_repeat3_main_main_default);

    lv_style_set_border_width(&style_screen_alarm_repeat3_main_main_default, 0);
    lv_style_set_radius(&style_screen_alarm_repeat3_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_alarm_repeat3_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_alarm_repeat3_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_alarm_repeat3_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_alarm_repeat3_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_alarm_repeat3_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_alarm_repeat3_main_main_default, 0);
    lv_obj_add_style(ui->screen_alarm_repeat3, &style_screen_alarm_repeat3_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_alarm_repeat3);

    //Write codes screen_alarm_sw_3
    ui->screen_alarm_sw_3 = lv_switch_create(ui->screen_alarm_cont_3);
    lv_obj_set_pos(ui->screen_alarm_sw_3, 204, 16);
    lv_obj_set_size(ui->screen_alarm_sw_3, 35, 20);

    //Write style for screen_alarm_sw_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_alarm_sw_3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_sw_3, lv_color_hex(0xe6e2e6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_sw_3, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_alarm_sw_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_sw_3, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_sw_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style for screen_alarm_sw_3, Part: LV_PART_INDICATOR, State: LV_STATE_CHECKED.
    lv_obj_set_style_bg_opa(ui->screen_alarm_sw_3, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui->screen_alarm_sw_3, lv_color_hex(0x2195f6), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_sw_3, LV_GRAD_DIR_NONE, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_width(ui->screen_alarm_sw_3, 0, LV_PART_INDICATOR | LV_STATE_CHECKED);

    //Write style for screen_alarm_sw_3, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_alarm_sw_3, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_sw_3, lv_color_hex(0xffffff), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_sw_3, LV_GRAD_DIR_NONE, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_alarm_sw_3, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_sw_3, 10, LV_PART_KNOB | LV_STATE_DEFAULT);

    //Write codes screen_alarm_time3
    ui->screen_alarm_time3 = lv_spangroup_create(ui->screen_alarm_cont_3);
    lv_spangroup_set_align(ui->screen_alarm_time3, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_alarm_time3, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_alarm_time3, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_alarm_time3_span = lv_spangroup_new_span(ui->screen_alarm_time3);
    lv_span_set_text(ui->screen_alarm_time3_span, "12:12");
    lv_style_set_text_color(&ui->screen_alarm_time3_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_alarm_time3_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_alarm_time3_span->style, &lv_font_Barlow__24);
    lv_obj_set_pos(ui->screen_alarm_time3, 12, 1);
    lv_obj_set_size(ui->screen_alarm_time3, 57, 25);

    //Write style state: LV_STATE_DEFAULT for &style_screen_alarm_time3_main_main_default
    static lv_style_t style_screen_alarm_time3_main_main_default;
    ui_init_style(&style_screen_alarm_time3_main_main_default);

    lv_style_set_border_width(&style_screen_alarm_time3_main_main_default, 0);
    lv_style_set_radius(&style_screen_alarm_time3_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_alarm_time3_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_alarm_time3_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_alarm_time3_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_alarm_time3_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_alarm_time3_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_alarm_time3_main_main_default, 0);
    lv_obj_add_style(ui->screen_alarm_time3, &style_screen_alarm_time3_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_alarm_time3);

    //Write codes screen_alarm_cont_2
    ui->screen_alarm_cont_2 = lv_obj_create(ui->screen_alarm_cont);
    lv_obj_set_pos(ui->screen_alarm_cont_2, 2, 118);
    lv_obj_set_size(ui->screen_alarm_cont_2, 258, 51);
    lv_obj_set_scrollbar_mode(ui->screen_alarm_cont_2, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_alarm_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_alarm_cont_2, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_alarm_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_alarm_cont_2, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_alarm_cont_2, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_cont_2, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_alarm_cont_2, 53, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_cont_2, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_cont_2, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_alarm_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_alarm_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_alarm_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_alarm_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_repeat2
    ui->screen_alarm_repeat2 = lv_spangroup_create(ui->screen_alarm_cont_2);
    lv_spangroup_set_align(ui->screen_alarm_repeat2, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_alarm_repeat2, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_alarm_repeat2, LV_SPAN_MODE_BREAK);
    //create span
    ui->screen_alarm_repeat2_span = lv_spangroup_new_span(ui->screen_alarm_repeat2);
    lv_span_set_text(ui->screen_alarm_repeat2_span, "周一 周二 周三 周四 周五 周六 周日");
    lv_style_set_text_color(&ui->screen_alarm_repeat2_span->style, lv_color_hex(0xc4c4c4));
    lv_style_set_text_decor(&ui->screen_alarm_repeat2_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_alarm_repeat2_span->style, &lv_font_MiSansDemibold_12);
    lv_obj_set_pos(ui->screen_alarm_repeat2, 13, 32);
    lv_obj_set_size(ui->screen_alarm_repeat2, 239, 12);

    //Write style state: LV_STATE_DEFAULT for &style_screen_alarm_repeat2_main_main_default
    static lv_style_t style_screen_alarm_repeat2_main_main_default;
    ui_init_style(&style_screen_alarm_repeat2_main_main_default);

    lv_style_set_border_width(&style_screen_alarm_repeat2_main_main_default, 0);
    lv_style_set_radius(&style_screen_alarm_repeat2_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_alarm_repeat2_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_alarm_repeat2_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_alarm_repeat2_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_alarm_repeat2_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_alarm_repeat2_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_alarm_repeat2_main_main_default, 0);
    lv_obj_add_style(ui->screen_alarm_repeat2, &style_screen_alarm_repeat2_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_alarm_repeat2);

    //Write codes screen_alarm_sw_2
    ui->screen_alarm_sw_2 = lv_switch_create(ui->screen_alarm_cont_2);
    lv_obj_set_pos(ui->screen_alarm_sw_2, 204, 16);
    lv_obj_set_size(ui->screen_alarm_sw_2, 35, 20);

    //Write style for screen_alarm_sw_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_alarm_sw_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_sw_2, lv_color_hex(0xe6e2e6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_sw_2, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_alarm_sw_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_sw_2, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_sw_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style for screen_alarm_sw_2, Part: LV_PART_INDICATOR, State: LV_STATE_CHECKED.
    lv_obj_set_style_bg_opa(ui->screen_alarm_sw_2, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui->screen_alarm_sw_2, lv_color_hex(0x2195f6), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_sw_2, LV_GRAD_DIR_NONE, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_width(ui->screen_alarm_sw_2, 0, LV_PART_INDICATOR | LV_STATE_CHECKED);

    //Write style for screen_alarm_sw_2, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_alarm_sw_2, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_sw_2, lv_color_hex(0xffffff), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_sw_2, LV_GRAD_DIR_NONE, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_alarm_sw_2, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_sw_2, 10, LV_PART_KNOB | LV_STATE_DEFAULT);

    //Write codes screen_alarm_time2
    ui->screen_alarm_time2 = lv_spangroup_create(ui->screen_alarm_cont_2);
    lv_spangroup_set_align(ui->screen_alarm_time2, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_alarm_time2, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_alarm_time2, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_alarm_time2_span = lv_spangroup_new_span(ui->screen_alarm_time2);
    lv_span_set_text(ui->screen_alarm_time2_span, "12:12");
    lv_style_set_text_color(&ui->screen_alarm_time2_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_alarm_time2_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_alarm_time2_span->style, &lv_font_Barlow__24);
    lv_obj_set_pos(ui->screen_alarm_time2, 12, 1);
    lv_obj_set_size(ui->screen_alarm_time2, 57, 25);

    //Write style state: LV_STATE_DEFAULT for &style_screen_alarm_time2_main_main_default
    static lv_style_t style_screen_alarm_time2_main_main_default;
    ui_init_style(&style_screen_alarm_time2_main_main_default);

    lv_style_set_border_width(&style_screen_alarm_time2_main_main_default, 0);
    lv_style_set_radius(&style_screen_alarm_time2_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_alarm_time2_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_alarm_time2_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_alarm_time2_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_alarm_time2_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_alarm_time2_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_alarm_time2_main_main_default, 0);
    lv_obj_add_style(ui->screen_alarm_time2, &style_screen_alarm_time2_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_alarm_time2);

    //Write codes screen_alarm_cont_1
    ui->screen_alarm_cont_1 = lv_obj_create(ui->screen_alarm_cont);
    lv_obj_set_pos(ui->screen_alarm_cont_1, 2, 0);
    lv_obj_set_size(ui->screen_alarm_cont_1, 258, 51);
    lv_obj_set_scrollbar_mode(ui->screen_alarm_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_alarm_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_alarm_cont_1, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_alarm_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_alarm_cont_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_alarm_cont_1, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_cont_1, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_alarm_cont_1, 52, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_cont_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_alarm_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_alarm_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_alarm_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_alarm_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_repeat1
    ui->screen_alarm_repeat1 = lv_spangroup_create(ui->screen_alarm_cont_1);
    lv_spangroup_set_align(ui->screen_alarm_repeat1, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_alarm_repeat1, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_alarm_repeat1, LV_SPAN_MODE_BREAK);
    //create span
    ui->screen_alarm_repeat1_span = lv_spangroup_new_span(ui->screen_alarm_repeat1);
    lv_span_set_text(ui->screen_alarm_repeat1_span, "只响一次");
    lv_style_set_text_color(&ui->screen_alarm_repeat1_span->style, lv_color_hex(0xc4c4c4));
    lv_style_set_text_decor(&ui->screen_alarm_repeat1_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_alarm_repeat1_span->style, &lv_font_MiSansDemibold_12);
    lv_obj_set_pos(ui->screen_alarm_repeat1, 13, 32);
    lv_obj_set_size(ui->screen_alarm_repeat1, 239, 12);

    //Write style state: LV_STATE_DEFAULT for &style_screen_alarm_repeat1_main_main_default
    static lv_style_t style_screen_alarm_repeat1_main_main_default;
    ui_init_style(&style_screen_alarm_repeat1_main_main_default);

    lv_style_set_border_width(&style_screen_alarm_repeat1_main_main_default, 0);
    lv_style_set_radius(&style_screen_alarm_repeat1_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_alarm_repeat1_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_alarm_repeat1_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_alarm_repeat1_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_alarm_repeat1_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_alarm_repeat1_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_alarm_repeat1_main_main_default, 0);
    lv_obj_add_style(ui->screen_alarm_repeat1, &style_screen_alarm_repeat1_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_alarm_repeat1);

    //Write codes screen_alarm_sw_1
    ui->screen_alarm_sw_1 = lv_switch_create(ui->screen_alarm_cont_1);
    lv_obj_set_pos(ui->screen_alarm_sw_1, 204, 16);
    lv_obj_set_size(ui->screen_alarm_sw_1, 35, 20);

    //Write style for screen_alarm_sw_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_alarm_sw_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_sw_1, lv_color_hex(0xe6e2e6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_sw_1, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_alarm_sw_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_sw_1, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_sw_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style for screen_alarm_sw_1, Part: LV_PART_INDICATOR, State: LV_STATE_CHECKED.
    lv_obj_set_style_bg_opa(ui->screen_alarm_sw_1, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui->screen_alarm_sw_1, lv_color_hex(0x2195f6), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_sw_1, LV_GRAD_DIR_NONE, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_width(ui->screen_alarm_sw_1, 0, LV_PART_INDICATOR | LV_STATE_CHECKED);

    //Write style for screen_alarm_sw_1, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_alarm_sw_1, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_sw_1, lv_color_hex(0xffffff), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_sw_1, LV_GRAD_DIR_NONE, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_alarm_sw_1, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_sw_1, 10, LV_PART_KNOB | LV_STATE_DEFAULT);

    //Write codes screen_alarm_time1
    ui->screen_alarm_time1 = lv_spangroup_create(ui->screen_alarm_cont_1);
    lv_spangroup_set_align(ui->screen_alarm_time1, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_alarm_time1, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_alarm_time1, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_alarm_time1_span = lv_spangroup_new_span(ui->screen_alarm_time1);
    lv_span_set_text(ui->screen_alarm_time1_span, "12:12");
    lv_style_set_text_color(&ui->screen_alarm_time1_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_alarm_time1_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_alarm_time1_span->style, &lv_font_Barlow__28);
    lv_obj_set_pos(ui->screen_alarm_time1, 11, 2);
    lv_obj_set_size(ui->screen_alarm_time1, 86, 25);

    //Write style state: LV_STATE_DEFAULT for &style_screen_alarm_time1_main_main_default
    static lv_style_t style_screen_alarm_time1_main_main_default;
    ui_init_style(&style_screen_alarm_time1_main_main_default);

    lv_style_set_border_width(&style_screen_alarm_time1_main_main_default, 0);
    lv_style_set_radius(&style_screen_alarm_time1_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_alarm_time1_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_alarm_time1_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_alarm_time1_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_alarm_time1_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_alarm_time1_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_alarm_time1_main_main_default, 0);
    lv_obj_add_style(ui->screen_alarm_time1, &style_screen_alarm_time1_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_alarm_time1);

    //The custom code of screen_alarm.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_alarm);

    //Init events for screen.
    events_init_screen_alarm(ui);
}
