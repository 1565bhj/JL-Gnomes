/*
* Copyright 2025 NXP
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



void setup_scr_screen_timer_add(lv_ui *ui)
{
    //Write codes screen_timer_add
    ui->screen_timer_add = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_timer_add, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_timer_add, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_timer_add, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_timer_add, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer_add, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer_add, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_timer_add_img_return
    ui->screen_timer_add_img_return = lv_img_create(ui->screen_timer_add);
    lv_obj_add_flag(ui->screen_timer_add_img_return, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_timer_add_img_return, &_return_alpha_22x22);
    lv_img_set_pivot(ui->screen_timer_add_img_return, 50, 50);
    lv_img_set_angle(ui->screen_timer_add_img_return, 0);
    lv_obj_set_pos(ui->screen_timer_add_img_return, 30, 20);
    lv_obj_set_size(ui->screen_timer_add_img_return, 22, 22);

    //Write style for screen_timer_add_img_return, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_timer_add_img_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_timer_add_img_return, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_timer_add_img_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_timer_add_img_return, true, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_pad_all(ui->screen_timer_add_img_return, 20);

    //Write codes screen_timer_add_add_timer_name
    ui->screen_timer_add_add_timer_name = lv_spangroup_create(ui->screen_timer_add);
    lv_spangroup_set_align(ui->screen_timer_add_add_timer_name, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_timer_add_add_timer_name, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_timer_add_add_timer_name, LV_SPAN_MODE_BREAK);
    //create span
    ui->screen_timer_add_add_timer_name_span = lv_spangroup_new_span(ui->screen_timer_add_add_timer_name);
    lv_span_set_text(ui->screen_timer_add_add_timer_name_span, "新建倒计时");
    lv_style_set_text_color(&ui->screen_timer_add_add_timer_name_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_timer_add_add_timer_name_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_timer_add_add_timer_name_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_timer_add_add_timer_name, 62, 20);
    lv_obj_set_size(ui->screen_timer_add_add_timer_name, 94, 18);

    //Write style state: LV_STATE_DEFAULT for &style_screen_timer_add_add_timer_name_main_main_default
    static lv_style_t style_screen_timer_add_add_timer_name_main_main_default;
    ui_init_style(&style_screen_timer_add_add_timer_name_main_main_default);

    lv_style_set_border_width(&style_screen_timer_add_add_timer_name_main_main_default, 0);
    lv_style_set_radius(&style_screen_timer_add_add_timer_name_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_timer_add_add_timer_name_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_timer_add_add_timer_name_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_timer_add_add_timer_name_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_timer_add_add_timer_name_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_timer_add_add_timer_name_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_timer_add_add_timer_name_main_main_default, 0);
    lv_obj_add_style(ui->screen_timer_add_add_timer_name, &style_screen_timer_add_add_timer_name_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_timer_add_add_timer_name);

    //Write codes screen_timer_add_img_yes
    ui->screen_timer_add_img_yes = lv_img_create(ui->screen_timer_add);
    lv_obj_add_flag(ui->screen_timer_add_img_yes, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_timer_add_img_yes, &_yes_alpha_30x30);
    lv_img_set_pivot(ui->screen_timer_add_img_yes, 50, 50);
    lv_img_set_angle(ui->screen_timer_add_img_yes, 0);
    lv_obj_set_pos(ui->screen_timer_add_img_yes, 249, 13);
    lv_obj_set_size(ui->screen_timer_add_img_yes, 30, 30);

    //Write style for screen_timer_add_img_yes, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_timer_add_img_yes, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_timer_add_img_yes, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_timer_add_img_yes, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_timer_add_img_yes, true, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_pad_all(ui->screen_timer_add_img_yes, 20);

    //Write codes screen_timer_add_roller_min
    ui->screen_timer_add_roller_min = lv_roller_create(ui->screen_timer_add);
    lv_roller_set_options(ui->screen_timer_add_roller_min, "1\n2\n3\n4\n5", LV_ROLLER_MODE_INFINITE);
    lv_obj_set_pos(ui->screen_timer_add_roller_min, 127, 98);
    lv_obj_set_width(ui->screen_timer_add_roller_min, 70);

    //Write style for screen_timer_add_roller_min, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_radius(ui->screen_timer_add_roller_min, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_timer_add_roller_min, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer_add_roller_min, lv_color_hex(0x5d4646), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer_add_roller_min, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_timer_add_roller_min, lv_color_hex(0x7e7e7e), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_timer_add_roller_min, &lv_font_Barlow__18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_timer_add_roller_min, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_timer_add_roller_min, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_timer_add_roller_min, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_timer_add_roller_min, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_timer_add_roller_min, lv_color_hex(0xe6e6e6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_timer_add_roller_min, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_timer_add_roller_min, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_timer_add_roller_min, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_timer_add_roller_min, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style for screen_timer_add_roller_min, Part: LV_PART_SELECTED, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_timer_add_roller_min, 84, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer_add_roller_min, lv_color_hex(0xcbcbcb), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer_add_roller_min, LV_GRAD_DIR_NONE, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_timer_add_roller_min, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_timer_add_roller_min, &lv_font_Barlow__24, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_timer_add_roller_min, 255, LV_PART_SELECTED | LV_STATE_DEFAULT);

    lv_roller_set_visible_row_count(ui->screen_timer_add_roller_min, 3);
    //Write codes screen_timer_add_roller_hour
    ui->screen_timer_add_roller_hour = lv_roller_create(ui->screen_timer_add);
    lv_roller_set_options(ui->screen_timer_add_roller_hour, "1\n2\n3\n4\n5", LV_ROLLER_MODE_INFINITE);
    lv_obj_set_pos(ui->screen_timer_add_roller_hour, 35, 98);
    lv_obj_set_width(ui->screen_timer_add_roller_hour, 70);

    //Write style for screen_timer_add_roller_hour, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_radius(ui->screen_timer_add_roller_hour, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_timer_add_roller_hour, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer_add_roller_hour, lv_color_hex(0x5d4646), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer_add_roller_hour, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_timer_add_roller_hour, lv_color_hex(0x7e7e7e), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_timer_add_roller_hour, &lv_font_Barlow__18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_timer_add_roller_hour, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_timer_add_roller_hour, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_timer_add_roller_hour, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_timer_add_roller_hour, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_timer_add_roller_hour, lv_color_hex(0xe6e6e6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_timer_add_roller_hour, LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_timer_add_roller_hour, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_timer_add_roller_hour, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_timer_add_roller_hour, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style for screen_timer_add_roller_hour, Part: LV_PART_SELECTED, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_timer_add_roller_hour, 83, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer_add_roller_hour, lv_color_hex(0xcbcbcb), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer_add_roller_hour, LV_GRAD_DIR_NONE, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_timer_add_roller_hour, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_timer_add_roller_hour, &lv_font_Barlow__24, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_timer_add_roller_hour, 255, LV_PART_SELECTED | LV_STATE_DEFAULT);

    lv_roller_set_visible_row_count(ui->screen_timer_add_roller_hour, 3);
    //Write codes screen_timer_add_roller_sec
    ui->screen_timer_add_roller_sec = lv_roller_create(ui->screen_timer_add);
    lv_roller_set_options(ui->screen_timer_add_roller_sec, "1\n2\n3\n4\n5", LV_ROLLER_MODE_INFINITE);
    lv_obj_set_pos(ui->screen_timer_add_roller_sec, 213, 98);
    lv_obj_set_width(ui->screen_timer_add_roller_sec, 70);

    //Write style for screen_timer_add_roller_sec, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_radius(ui->screen_timer_add_roller_sec, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_timer_add_roller_sec, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer_add_roller_sec, lv_color_hex(0x5d4646), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer_add_roller_sec, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_timer_add_roller_sec, lv_color_hex(0x7e7e7e), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_timer_add_roller_sec, &lv_font_Barlow__18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_timer_add_roller_sec, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_timer_add_roller_sec, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_timer_add_roller_sec, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_timer_add_roller_sec, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_timer_add_roller_sec, lv_color_hex(0xe6e6e6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_timer_add_roller_sec, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_timer_add_roller_sec, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_timer_add_roller_sec, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_timer_add_roller_sec, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style for screen_timer_add_roller_sec, Part: LV_PART_SELECTED, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_timer_add_roller_sec, 84, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer_add_roller_sec, lv_color_hex(0xcbcbcb), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer_add_roller_sec, LV_GRAD_DIR_NONE, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_timer_add_roller_sec, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_timer_add_roller_sec, &lv_font_Barlow__24, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_timer_add_roller_sec, 255, LV_PART_SELECTED | LV_STATE_DEFAULT);

    lv_roller_set_visible_row_count(ui->screen_timer_add_roller_sec, 3);
    //Write codes screen_timer_add_dot1
    ui->screen_timer_add_dot1 = lv_spangroup_create(ui->screen_timer_add);
    lv_spangroup_set_align(ui->screen_timer_add_dot1, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_timer_add_dot1, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_timer_add_dot1, LV_SPAN_MODE_BREAK);
    //create span
    ui->screen_timer_add_dot1_span = lv_spangroup_new_span(ui->screen_timer_add_dot1);
    lv_span_set_text(ui->screen_timer_add_dot1_span, ":");
    lv_style_set_text_color(&ui->screen_timer_add_dot1_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_timer_add_dot1_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_timer_add_dot1_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_timer_add_dot1, 111, 139);
    lv_obj_set_size(ui->screen_timer_add_dot1, 5, 21);

    //Write style state: LV_STATE_DEFAULT for &style_screen_timer_add_dot1_main_main_default
    static lv_style_t style_screen_timer_add_dot1_main_main_default;
    ui_init_style(&style_screen_timer_add_dot1_main_main_default);

    lv_style_set_border_width(&style_screen_timer_add_dot1_main_main_default, 0);
    lv_style_set_radius(&style_screen_timer_add_dot1_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_timer_add_dot1_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_timer_add_dot1_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_timer_add_dot1_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_timer_add_dot1_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_timer_add_dot1_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_timer_add_dot1_main_main_default, 0);
    lv_obj_add_style(ui->screen_timer_add_dot1, &style_screen_timer_add_dot1_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_timer_add_dot1);

    //Write codes screen_timer_add_dot2
    ui->screen_timer_add_dot2 = lv_spangroup_create(ui->screen_timer_add);
    lv_spangroup_set_align(ui->screen_timer_add_dot2, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_timer_add_dot2, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_timer_add_dot2, LV_SPAN_MODE_BREAK);
    //create span
    ui->screen_timer_add_dot2_span = lv_spangroup_new_span(ui->screen_timer_add_dot2);
    lv_span_set_text(ui->screen_timer_add_dot2_span, ":");
    lv_style_set_text_color(&ui->screen_timer_add_dot2_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_timer_add_dot2_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_timer_add_dot2_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_timer_add_dot2, 201, 139);
    lv_obj_set_size(ui->screen_timer_add_dot2, 5, 21);

    //Write style state: LV_STATE_DEFAULT for &style_screen_timer_add_dot2_main_main_default
    static lv_style_t style_screen_timer_add_dot2_main_main_default;
    ui_init_style(&style_screen_timer_add_dot2_main_main_default);

    lv_style_set_border_width(&style_screen_timer_add_dot2_main_main_default, 0);
    lv_style_set_radius(&style_screen_timer_add_dot2_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_timer_add_dot2_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_timer_add_dot2_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_timer_add_dot2_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_timer_add_dot2_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_timer_add_dot2_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_timer_add_dot2_main_main_default, 0);
    lv_obj_add_style(ui->screen_timer_add_dot2, &style_screen_timer_add_dot2_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_timer_add_dot2);

    //Write codes screen_timer_add_hour
    ui->screen_timer_add_hour = lv_spangroup_create(ui->screen_timer_add);
    lv_spangroup_set_align(ui->screen_timer_add_hour, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_timer_add_hour, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_timer_add_hour, LV_SPAN_MODE_BREAK);
    //create span
    ui->screen_timer_add_hour_span = lv_spangroup_new_span(ui->screen_timer_add_hour);
    lv_span_set_text(ui->screen_timer_add_hour_span, "时");
    lv_style_set_text_color(&ui->screen_timer_add_hour_span->style, lv_color_hex(0x929292));
    lv_style_set_text_decor(&ui->screen_timer_add_hour_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_timer_add_hour_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_timer_add_hour, 62, 214);
    lv_obj_set_size(ui->screen_timer_add_hour, 20, 17);

    //Write style state: LV_STATE_DEFAULT for &style_screen_timer_add_hour_main_main_default
    static lv_style_t style_screen_timer_add_hour_main_main_default;
    ui_init_style(&style_screen_timer_add_hour_main_main_default);

    lv_style_set_border_width(&style_screen_timer_add_hour_main_main_default, 0);
    lv_style_set_radius(&style_screen_timer_add_hour_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_timer_add_hour_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_timer_add_hour_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_timer_add_hour_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_timer_add_hour_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_timer_add_hour_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_timer_add_hour_main_main_default, 0);
    lv_obj_add_style(ui->screen_timer_add_hour, &style_screen_timer_add_hour_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_timer_add_hour);

    //Write codes screen_timer_add_min
    ui->screen_timer_add_min = lv_spangroup_create(ui->screen_timer_add);
    lv_spangroup_set_align(ui->screen_timer_add_min, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_timer_add_min, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_timer_add_min, LV_SPAN_MODE_BREAK);
    //create span
    ui->screen_timer_add_min_span = lv_spangroup_new_span(ui->screen_timer_add_min);
    lv_span_set_text(ui->screen_timer_add_min_span, "分");
    lv_style_set_text_color(&ui->screen_timer_add_min_span->style, lv_color_hex(0x929292));
    lv_style_set_text_decor(&ui->screen_timer_add_min_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_timer_add_min_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_timer_add_min, 153, 214);
    lv_obj_set_size(ui->screen_timer_add_min, 20, 17);

    //Write style state: LV_STATE_DEFAULT for &style_screen_timer_add_min_main_main_default
    static lv_style_t style_screen_timer_add_min_main_main_default;
    ui_init_style(&style_screen_timer_add_min_main_main_default);

    lv_style_set_border_width(&style_screen_timer_add_min_main_main_default, 0);
    lv_style_set_radius(&style_screen_timer_add_min_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_timer_add_min_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_timer_add_min_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_timer_add_min_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_timer_add_min_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_timer_add_min_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_timer_add_min_main_main_default, 0);
    lv_obj_add_style(ui->screen_timer_add_min, &style_screen_timer_add_min_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_timer_add_min);

    //Write codes screen_timer_add_sec
    ui->screen_timer_add_sec = lv_spangroup_create(ui->screen_timer_add);
    lv_spangroup_set_align(ui->screen_timer_add_sec, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_timer_add_sec, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_timer_add_sec, LV_SPAN_MODE_BREAK);
    //create span
    ui->screen_timer_add_sec_span = lv_spangroup_new_span(ui->screen_timer_add_sec);
    lv_span_set_text(ui->screen_timer_add_sec_span, "秒");
    lv_style_set_text_color(&ui->screen_timer_add_sec_span->style, lv_color_hex(0x929292));
    lv_style_set_text_decor(&ui->screen_timer_add_sec_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_timer_add_sec_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_timer_add_sec, 241, 214);
    lv_obj_set_size(ui->screen_timer_add_sec, 20, 17);

    //Write style state: LV_STATE_DEFAULT for &style_screen_timer_add_sec_main_main_default
    static lv_style_t style_screen_timer_add_sec_main_main_default;
    ui_init_style(&style_screen_timer_add_sec_main_main_default);

    lv_style_set_border_width(&style_screen_timer_add_sec_main_main_default, 0);
    lv_style_set_radius(&style_screen_timer_add_sec_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_timer_add_sec_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_timer_add_sec_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_timer_add_sec_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_timer_add_sec_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_timer_add_sec_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_timer_add_sec_main_main_default, 0);
    lv_obj_add_style(ui->screen_timer_add_sec, &style_screen_timer_add_sec_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_timer_add_sec);

    //Write codes screen_timer_add_min5
    ui->screen_timer_add_min5 = lv_btn_create(ui->screen_timer_add);
    ui->screen_timer_add_min5_label = lv_label_create(ui->screen_timer_add_min5);
    lv_label_set_text(ui->screen_timer_add_min5_label, "5分钟");
    lv_label_set_long_mode(ui->screen_timer_add_min5_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_timer_add_min5_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_timer_add_min5, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_timer_add_min5_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_timer_add_min5, 35, 55);
    lv_obj_set_size(ui->screen_timer_add_min5, 72, 37);

    //Write style for screen_timer_add_min5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_timer_add_min5, 40, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer_add_min5, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer_add_min5, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_timer_add_min5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_timer_add_min5, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_timer_add_min5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_timer_add_min5, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_timer_add_min5, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_timer_add_min5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_timer_add_min5, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_timer_add_min10
    ui->screen_timer_add_min10 = lv_btn_create(ui->screen_timer_add);
    ui->screen_timer_add_min10_label = lv_label_create(ui->screen_timer_add_min10);
    lv_label_set_text(ui->screen_timer_add_min10_label, "10分钟");
    lv_label_set_long_mode(ui->screen_timer_add_min10_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_timer_add_min10_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_timer_add_min10, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_timer_add_min10_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_timer_add_min10, 124, 55);
    lv_obj_set_size(ui->screen_timer_add_min10, 72, 37);

    //Write style for screen_timer_add_min10, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_timer_add_min10, 40, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer_add_min10, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer_add_min10, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_timer_add_min10, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_timer_add_min10, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_timer_add_min10, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_timer_add_min10, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_timer_add_min10, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_timer_add_min10, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_timer_add_min10, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_timer_add_min25
    ui->screen_timer_add_min25 = lv_btn_create(ui->screen_timer_add);
    ui->screen_timer_add_min25_label = lv_label_create(ui->screen_timer_add_min25);
    lv_label_set_text(ui->screen_timer_add_min25_label, "25分钟");
    lv_label_set_long_mode(ui->screen_timer_add_min25_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_timer_add_min25_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_timer_add_min25, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_timer_add_min25_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_timer_add_min25, 213, 55);
    lv_obj_set_size(ui->screen_timer_add_min25, 72, 37);

    //Write style for screen_timer_add_min25, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_timer_add_min25, 40, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer_add_min25, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer_add_min25, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_timer_add_min25, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_timer_add_min25, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_timer_add_min25, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_timer_add_min25, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_timer_add_min25, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_timer_add_min25, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_timer_add_min25, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //The custom code of screen_timer_add.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_timer_add);

    //Init events for screen.
    events_init_screen_timer_add(ui);
}
