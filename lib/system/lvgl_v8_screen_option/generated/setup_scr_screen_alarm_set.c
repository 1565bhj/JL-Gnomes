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



void setup_scr_screen_alarm_set(lv_ui *ui)
{
    //Write codes screen_alarm_set
    ui->screen_alarm_set = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_alarm_set, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_alarm_set, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_alarm_set, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_alarm_set, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_set, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_set, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_set_yes
    ui->screen_alarm_set_yes = lv_img_create(ui->screen_alarm_set);
    lv_obj_add_flag(ui->screen_alarm_set_yes, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_alarm_set_yes, &_yes_alpha_30x30);
    lv_img_set_pivot(ui->screen_alarm_set_yes, 50, 50);
    lv_img_set_angle(ui->screen_alarm_set_yes, 0);
    lv_obj_set_pos(ui->screen_alarm_set_yes, 249, 15);
    lv_obj_set_size(ui->screen_alarm_set_yes, 30, 30);
//    lv_obj_set_style_pad_all(ui->screen_alarm_set_yes, 20);

    //Write style for screen_alarm_set_yes, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_alarm_set_yes, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_alarm_set_yes, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_set_yes, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_alarm_set_yes, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_set_repeat_name
    ui->screen_alarm_set_repeat_name = lv_spangroup_create(ui->screen_alarm_set);
    lv_spangroup_set_align(ui->screen_alarm_set_repeat_name, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_alarm_set_repeat_name, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_alarm_set_repeat_name, LV_SPAN_MODE_BREAK);
    //create span
    ui->screen_alarm_set_repeat_name_span = lv_spangroup_new_span(ui->screen_alarm_set_repeat_name);
    lv_span_set_text(ui->screen_alarm_set_repeat_name_span, "重复");
    lv_style_set_text_color(&ui->screen_alarm_set_repeat_name_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_alarm_set_repeat_name_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_alarm_set_repeat_name_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_alarm_set_repeat_name, 62, 20);
    lv_obj_set_size(ui->screen_alarm_set_repeat_name, 36, 18);

    //Write style state: LV_STATE_DEFAULT for &style_screen_alarm_set_repeat_name_main_main_default
    static lv_style_t style_screen_alarm_set_repeat_name_main_main_default;
    ui_init_style(&style_screen_alarm_set_repeat_name_main_main_default);

    lv_style_set_border_width(&style_screen_alarm_set_repeat_name_main_main_default, 0);
    lv_style_set_radius(&style_screen_alarm_set_repeat_name_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_alarm_set_repeat_name_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_alarm_set_repeat_name_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_alarm_set_repeat_name_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_alarm_set_repeat_name_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_alarm_set_repeat_name_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_alarm_set_repeat_name_main_main_default, 0);
    lv_obj_add_style(ui->screen_alarm_set_repeat_name, &style_screen_alarm_set_repeat_name_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_alarm_set_repeat_name);

    //Write codes screen_alarm_set_return
    ui->screen_alarm_set_return = lv_img_create(ui->screen_alarm_set);
    lv_obj_add_flag(ui->screen_alarm_set_return, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_alarm_set_return, &_return_alpha_22x22);
    lv_img_set_pivot(ui->screen_alarm_set_return, 50, 50);
    lv_img_set_angle(ui->screen_alarm_set_return, 0);
    lv_obj_set_pos(ui->screen_alarm_set_return, 30, 20);
    lv_obj_set_size(ui->screen_alarm_set_return, 22, 22);

    //Write style for screen_alarm_set_return, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_alarm_set_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_alarm_set_return, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_set_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_alarm_set_return, true, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_pad_all(ui->screen_alarm_set_return, 20);

    //Write codes screen_alarm_set_cont_circle
    ui->screen_alarm_set_cont_circle = lv_obj_create(ui->screen_alarm_set);
    lv_obj_set_pos(ui->screen_alarm_set_cont_circle, 30, 51);
    lv_obj_set_size(ui->screen_alarm_set_cont_circle, 258, 40);
    lv_obj_set_scrollbar_mode(ui->screen_alarm_set_cont_circle, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_alarm_set_cont_circle, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_alarm_set_cont_circle, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_alarm_set_cont_circle, 122, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_alarm_set_cont_circle, lv_color_hex(0xe2e2e2), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_alarm_set_cont_circle, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_set_cont_circle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_alarm_set_cont_circle, 26, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_set_cont_circle, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_set_cont_circle, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_alarm_set_cont_circle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_alarm_set_cont_circle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_alarm_set_cont_circle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_alarm_set_cont_circle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_set_cont_circle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_set_circle
    ui->screen_alarm_set_circle = lv_spangroup_create(ui->screen_alarm_set_cont_circle);
    lv_spangroup_set_align(ui->screen_alarm_set_circle, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_alarm_set_circle, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_alarm_set_circle, LV_SPAN_MODE_BREAK);
    //create span
    ui->screen_alarm_set_circle_span = lv_spangroup_new_span(ui->screen_alarm_set_circle);
    lv_span_set_text(ui->screen_alarm_set_circle_span, "每天");
    lv_style_set_text_color(&ui->screen_alarm_set_circle_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_alarm_set_circle_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_alarm_set_circle_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_alarm_set_circle, 7, 9);
    lv_obj_set_size(ui->screen_alarm_set_circle, 83, 18);

    //Write style state: LV_STATE_DEFAULT for &style_screen_alarm_set_circle_main_main_default
    static lv_style_t style_screen_alarm_set_circle_main_main_default;
    ui_init_style(&style_screen_alarm_set_circle_main_main_default);

    lv_style_set_border_width(&style_screen_alarm_set_circle_main_main_default, 0);
    lv_style_set_radius(&style_screen_alarm_set_circle_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_alarm_set_circle_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_alarm_set_circle_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_alarm_set_circle_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_alarm_set_circle_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_alarm_set_circle_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_alarm_set_circle_main_main_default, 0);
    lv_obj_add_style(ui->screen_alarm_set_circle, &style_screen_alarm_set_circle_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_alarm_set_circle);

    //Write codes screen_alarm_set_p_circle
    ui->screen_alarm_set_p_circle = lv_btn_create(ui->screen_alarm_set_cont_circle);
    ui->screen_alarm_set_p_circle_label = lv_label_create(ui->screen_alarm_set_p_circle);
    lv_label_set_text(ui->screen_alarm_set_p_circle_label, "");
    lv_label_set_long_mode(ui->screen_alarm_set_p_circle_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_alarm_set_p_circle_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_alarm_set_p_circle, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_alarm_set_p_circle_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_alarm_set_p_circle, 222, 7);
    lv_obj_set_size(ui->screen_alarm_set_p_circle, 20, 20);

    //Write style for screen_alarm_set_p_circle, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_alarm_set_p_circle, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_set_p_circle, lv_color_hex(0xe8e8e8), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_set_p_circle, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_alarm_set_p_circle, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_alarm_set_p_circle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_alarm_set_p_circle, lv_color_hex(0xc1c1c1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_alarm_set_p_circle, LV_BORDER_SIDE_FULL | LV_BORDER_SIDE_RIGHT | LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_set_p_circle, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_set_p_circle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_alarm_set_p_circle, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_alarm_set_p_circle, &lv_font_Barlow__12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_alarm_set_p_circle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_alarm_set_p_circle, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_set_cont_once
    ui->screen_alarm_set_cont_once = lv_obj_create(ui->screen_alarm_set);
    lv_obj_set_pos(ui->screen_alarm_set_cont_once, 30, 91);
    lv_obj_set_size(ui->screen_alarm_set_cont_once, 258, 41);
    lv_obj_set_scrollbar_mode(ui->screen_alarm_set_cont_once, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_alarm_set_cont_once, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_alarm_set_cont_once, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_alarm_set_cont_once, 130, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_alarm_set_cont_once, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_alarm_set_cont_once, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_set_cont_once, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_alarm_set_cont_once, 28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_set_cont_once, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_set_cont_once, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_alarm_set_cont_once, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_alarm_set_cont_once, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_alarm_set_cont_once, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_alarm_set_cont_once, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_set_cont_once, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_set_once
    ui->screen_alarm_set_once = lv_spangroup_create(ui->screen_alarm_set_cont_once);
    lv_spangroup_set_align(ui->screen_alarm_set_once, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_alarm_set_once, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_alarm_set_once, LV_SPAN_MODE_BREAK);
    //create span
    ui->screen_alarm_set_once_span = lv_spangroup_new_span(ui->screen_alarm_set_once);
    lv_span_set_text(ui->screen_alarm_set_once_span, "只响一次");
    lv_style_set_text_color(&ui->screen_alarm_set_once_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_alarm_set_once_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_alarm_set_once_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_alarm_set_once, 7, 9);
    lv_obj_set_size(ui->screen_alarm_set_once, 83, 18);

    //Write style state: LV_STATE_DEFAULT for &style_screen_alarm_set_once_main_main_default
    static lv_style_t style_screen_alarm_set_once_main_main_default;
    ui_init_style(&style_screen_alarm_set_once_main_main_default);

    lv_style_set_border_width(&style_screen_alarm_set_once_main_main_default, 0);
    lv_style_set_radius(&style_screen_alarm_set_once_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_alarm_set_once_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_alarm_set_once_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_alarm_set_once_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_alarm_set_once_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_alarm_set_once_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_alarm_set_once_main_main_default, 0);
    lv_obj_add_style(ui->screen_alarm_set_once, &style_screen_alarm_set_once_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_alarm_set_once);

    //Write codes screen_alarm_set_p_once
    ui->screen_alarm_set_p_once = lv_btn_create(ui->screen_alarm_set_cont_once);
    ui->screen_alarm_set_p_once_label = lv_label_create(ui->screen_alarm_set_p_once);
    lv_label_set_text(ui->screen_alarm_set_p_once_label, "");
    lv_label_set_long_mode(ui->screen_alarm_set_p_once_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_alarm_set_p_once_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_alarm_set_p_once, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_alarm_set_p_once_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_alarm_set_p_once, 222, 7);
    lv_obj_set_size(ui->screen_alarm_set_p_once, 20, 20);

    //Write style for screen_alarm_set_p_once, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_alarm_set_p_once, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_set_p_once, lv_color_hex(0xe8e8e8), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_set_p_once, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_alarm_set_p_once, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_alarm_set_p_once, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_alarm_set_p_once, lv_color_hex(0xc1c1c1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_alarm_set_p_once, LV_BORDER_SIDE_FULL | LV_BORDER_SIDE_RIGHT | LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_set_p_once, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_set_p_once, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_alarm_set_p_once, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_alarm_set_p_once, &lv_font_Barlow__12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_alarm_set_p_once, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_alarm_set_p_once, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_pad_all(ui->screen_alarm_set_p_once, 20);
    //Write codes screen_alarm_set_cont_set
    ui->screen_alarm_set_cont_set = lv_obj_create(ui->screen_alarm_set);
    lv_obj_set_pos(ui->screen_alarm_set_cont_set, 30, 132);
    lv_obj_set_size(ui->screen_alarm_set_cont_set, 258, 83);
    lv_obj_set_scrollbar_mode(ui->screen_alarm_set_cont_set, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_alarm_set_cont_set, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_alarm_set_cont_set, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_alarm_set_cont_set, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_alarm_set_cont_set, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_alarm_set_cont_set, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_set_cont_set, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_alarm_set_cont_set, 28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_set_cont_set, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_set_cont_set, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_alarm_set_cont_set, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_alarm_set_cont_set, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_alarm_set_cont_set, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_alarm_set_cont_set, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_set_cont_set, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_set_data_set
    ui->screen_alarm_set_data_set = lv_spangroup_create(ui->screen_alarm_set_cont_set);
    lv_spangroup_set_align(ui->screen_alarm_set_data_set, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_alarm_set_data_set, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_alarm_set_data_set, LV_SPAN_MODE_BREAK);
    //create span
    ui->screen_alarm_set_data_set_span = lv_spangroup_new_span(ui->screen_alarm_set_data_set);
    lv_span_set_text(ui->screen_alarm_set_data_set_span, "日期设置");
    lv_style_set_text_color(&ui->screen_alarm_set_data_set_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_alarm_set_data_set_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_alarm_set_data_set_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_alarm_set_data_set, 5, 9);
    lv_obj_set_size(ui->screen_alarm_set_data_set, 83, 18);

    //Write style state: LV_STATE_DEFAULT for &style_screen_alarm_set_data_set_main_main_default
    static lv_style_t style_screen_alarm_set_data_set_main_main_default;
    ui_init_style(&style_screen_alarm_set_data_set_main_main_default);

    lv_style_set_border_width(&style_screen_alarm_set_data_set_main_main_default, 0);
    lv_style_set_radius(&style_screen_alarm_set_data_set_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_alarm_set_data_set_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_alarm_set_data_set_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_alarm_set_data_set_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_alarm_set_data_set_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_alarm_set_data_set_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_alarm_set_data_set_main_main_default, 0);
    lv_obj_add_style(ui->screen_alarm_set_data_set, &style_screen_alarm_set_data_set_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_alarm_set_data_set);

    //Write codes screen_alarm_set_btn_7
    ui->screen_alarm_set_btn_7 = lv_btn_create(ui->screen_alarm_set_cont_set);
    ui->screen_alarm_set_btn_7_label = lv_label_create(ui->screen_alarm_set_btn_7);
    lv_label_set_text(ui->screen_alarm_set_btn_7_label, "日");
    lv_label_set_long_mode(ui->screen_alarm_set_btn_7_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_alarm_set_btn_7_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_alarm_set_btn_7, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_alarm_set_btn_7_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_alarm_set_btn_7, 6, 42);
    lv_obj_set_size(ui->screen_alarm_set_btn_7, 30, 30);

    //Write style for screen_alarm_set_btn_7, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_7, 37, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_set_btn_7, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_set_btn_7, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_alarm_set_btn_7, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_set_btn_7, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_set_btn_7, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_alarm_set_btn_7, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_alarm_set_btn_7, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_7, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_alarm_set_btn_7, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_set_btn_2
    ui->screen_alarm_set_btn_2 = lv_btn_create(ui->screen_alarm_set_cont_set);
    ui->screen_alarm_set_btn_2_label = lv_label_create(ui->screen_alarm_set_btn_2);
    lv_label_set_text(ui->screen_alarm_set_btn_2_label, "三");
    lv_label_set_long_mode(ui->screen_alarm_set_btn_2_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_alarm_set_btn_2_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_alarm_set_btn_2, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_alarm_set_btn_2_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_alarm_set_btn_2, 111, 42);
    lv_obj_set_size(ui->screen_alarm_set_btn_2, 30, 30);

    //Write style for screen_alarm_set_btn_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_2, 40, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_set_btn_2, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_set_btn_2, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_alarm_set_btn_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_set_btn_2, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_set_btn_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_alarm_set_btn_2, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_alarm_set_btn_2, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_alarm_set_btn_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_set_btn_3
    ui->screen_alarm_set_btn_3 = lv_btn_create(ui->screen_alarm_set_cont_set);
    ui->screen_alarm_set_btn_3_label = lv_label_create(ui->screen_alarm_set_btn_3);
    lv_label_set_text(ui->screen_alarm_set_btn_3_label, "二");
    lv_label_set_long_mode(ui->screen_alarm_set_btn_3_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_alarm_set_btn_3_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_alarm_set_btn_3, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_alarm_set_btn_3_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_alarm_set_btn_3, 76, 42);
    lv_obj_set_size(ui->screen_alarm_set_btn_3, 30, 30);

    //Write style for screen_alarm_set_btn_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_3, 40, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_set_btn_3, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_set_btn_3, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_alarm_set_btn_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_set_btn_3, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_set_btn_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_alarm_set_btn_3, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_alarm_set_btn_3, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_alarm_set_btn_3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_set_btn_4
    ui->screen_alarm_set_btn_4 = lv_btn_create(ui->screen_alarm_set_cont_set);
    ui->screen_alarm_set_btn_4_label = lv_label_create(ui->screen_alarm_set_btn_4);
    lv_label_set_text(ui->screen_alarm_set_btn_4_label, "四");
    lv_label_set_long_mode(ui->screen_alarm_set_btn_4_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_alarm_set_btn_4_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_alarm_set_btn_4, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_alarm_set_btn_4_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_alarm_set_btn_4, 146, 42);
    lv_obj_set_size(ui->screen_alarm_set_btn_4, 30, 30);

    //Write style for screen_alarm_set_btn_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_4, 39, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_set_btn_4, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_set_btn_4, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_alarm_set_btn_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_set_btn_4, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_set_btn_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_alarm_set_btn_4, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_alarm_set_btn_4, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_alarm_set_btn_4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_set_btn_5
    ui->screen_alarm_set_btn_5 = lv_btn_create(ui->screen_alarm_set_cont_set);
    ui->screen_alarm_set_btn_5_label = lv_label_create(ui->screen_alarm_set_btn_5);
    lv_label_set_text(ui->screen_alarm_set_btn_5_label, "五");
    lv_label_set_long_mode(ui->screen_alarm_set_btn_5_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_alarm_set_btn_5_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_alarm_set_btn_5, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_alarm_set_btn_5_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_alarm_set_btn_5, 181, 42);
    lv_obj_set_size(ui->screen_alarm_set_btn_5, 30, 30);

    //Write style for screen_alarm_set_btn_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_5, 40, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_set_btn_5, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_set_btn_5, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_alarm_set_btn_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_set_btn_5, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_set_btn_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_alarm_set_btn_5, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_alarm_set_btn_5, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_alarm_set_btn_5, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_set_btn_1
    ui->screen_alarm_set_btn_1 = lv_btn_create(ui->screen_alarm_set_cont_set);
    ui->screen_alarm_set_btn_1_label = lv_label_create(ui->screen_alarm_set_btn_1);
    lv_label_set_text(ui->screen_alarm_set_btn_1_label, "一");
    lv_label_set_long_mode(ui->screen_alarm_set_btn_1_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_alarm_set_btn_1_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_alarm_set_btn_1, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_alarm_set_btn_1_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_alarm_set_btn_1, 41, 42);
    lv_obj_set_size(ui->screen_alarm_set_btn_1, 30, 30);

    //Write style for screen_alarm_set_btn_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_1, 39, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_set_btn_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_set_btn_1, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_alarm_set_btn_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_set_btn_1, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_set_btn_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_alarm_set_btn_1, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_alarm_set_btn_1, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_alarm_set_btn_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_set_btn_6
    ui->screen_alarm_set_btn_6 = lv_btn_create(ui->screen_alarm_set_cont_set);
    ui->screen_alarm_set_btn_6_label = lv_label_create(ui->screen_alarm_set_btn_6);
    lv_label_set_text(ui->screen_alarm_set_btn_6_label, "六");
    lv_label_set_long_mode(ui->screen_alarm_set_btn_6_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_alarm_set_btn_6_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_alarm_set_btn_6, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_alarm_set_btn_6_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_alarm_set_btn_6, 216, 42);
    lv_obj_set_size(ui->screen_alarm_set_btn_6, 30, 30);

    //Write style for screen_alarm_set_btn_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_alarm_set_btn_6, 40, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_set_btn_6, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_set_btn_6, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_alarm_set_btn_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_set_btn_6, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_set_btn_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_alarm_set_btn_6, lv_color_hex(0x9e9e9e), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_alarm_set_btn_6, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_alarm_set_btn_6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_alarm_set_btn_6, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_alarm_set_p_set
    ui->screen_alarm_set_p_set = lv_btn_create(ui->screen_alarm_set_cont_set);
    ui->screen_alarm_set_p_set_label = lv_label_create(ui->screen_alarm_set_p_set);
    lv_label_set_text(ui->screen_alarm_set_p_set_label, "");
    lv_label_set_long_mode(ui->screen_alarm_set_p_set_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_alarm_set_p_set_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_alarm_set_p_set, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_alarm_set_p_set_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_alarm_set_p_set, 222, 7);
    lv_obj_set_size(ui->screen_alarm_set_p_set, 20, 20);

    //Write style for screen_alarm_set_p_set, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_alarm_set_p_set, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_alarm_set_p_set, lv_color_hex(0xe8e8e8), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_alarm_set_p_set, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_alarm_set_p_set, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_alarm_set_p_set, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_alarm_set_p_set, lv_color_hex(0xc1c1c1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_alarm_set_p_set, LV_BORDER_SIDE_FULL | LV_BORDER_SIDE_RIGHT | LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_alarm_set_p_set, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_alarm_set_p_set, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_alarm_set_p_set, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_alarm_set_p_set, &lv_font_Barlow__12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_alarm_set_p_set, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_alarm_set_p_set, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_pad_all(ui->screen_alarm_set_p_set, 20);
    //The custom code of screen_alarm_set.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_alarm_set);

    //Init events for screen.
    events_init_screen_alarm_set(ui);
}
