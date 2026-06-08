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



void setup_scr_screen_timer(lv_ui *ui)
{
    //Write codes screen_timer
    ui->screen_timer = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_timer, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_timer, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_timer, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_timer, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_timer_img_add
    ui->screen_timer_img_add = lv_img_create(ui->screen_timer);
    lv_obj_add_flag(ui->screen_timer_img_add, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_timer_img_add, &_add_alpha_30x30);
    lv_img_set_pivot(ui->screen_timer_img_add, 50, 50);
    lv_img_set_angle(ui->screen_timer_img_add, 0);
    lv_obj_set_pos(ui->screen_timer_img_add, 253, 15);
    lv_obj_set_size(ui->screen_timer_img_add, 30, 30);

    //Write style for screen_timer_img_add, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_timer_img_add, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_timer_img_add, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_timer_img_add, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_timer_img_add, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_timer_span_timer
    ui->screen_timer_span_timer = lv_spangroup_create(ui->screen_timer);
    lv_spangroup_set_align(ui->screen_timer_span_timer, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_timer_span_timer, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_timer_span_timer, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_timer_span_timer_span = lv_spangroup_new_span(ui->screen_timer_span_timer);
    lv_span_set_text(ui->screen_timer_span_timer_span, "计时器");
    lv_style_set_text_color(&ui->screen_timer_span_timer_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_timer_span_timer_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_timer_span_timer_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_timer_span_timer, 132, 22);
    lv_obj_set_size(ui->screen_timer_span_timer, 55, 18);

    //Write style state: LV_STATE_DEFAULT for &style_screen_timer_span_timer_main_main_default
    static lv_style_t style_screen_timer_span_timer_main_main_default;
    ui_init_style(&style_screen_timer_span_timer_main_main_default);

    lv_style_set_border_width(&style_screen_timer_span_timer_main_main_default, 0);
    lv_style_set_radius(&style_screen_timer_span_timer_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_timer_span_timer_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_timer_span_timer_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_timer_span_timer_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_timer_span_timer_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_timer_span_timer_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_timer_span_timer_main_main_default, 0);
    lv_obj_add_style(ui->screen_timer_span_timer, &style_screen_timer_span_timer_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_timer_span_timer);

    //Write codes screen_timer_img_del
    ui->screen_timer_img_del = lv_img_create(ui->screen_timer);
    lv_obj_add_flag(ui->screen_timer_img_del, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_timer_img_del, &_clear_alpha_30x30);
    lv_img_set_pivot(ui->screen_timer_img_del, 50, 50);
    lv_img_set_angle(ui->screen_timer_img_del, 0);
    lv_obj_set_pos(ui->screen_timer_img_del, 33, 15);
    lv_obj_set_size(ui->screen_timer_img_del, 30, 30);

    //Write style for screen_timer_img_del, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_timer_img_del, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_timer_img_del, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_timer_img_del, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_timer_img_del, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_timer_cont
    ui->screen_timer_cont = lv_obj_create(ui->screen_timer);
    lv_obj_set_pos(ui->screen_timer_cont, 28, 60);
    lv_obj_set_size(ui->screen_timer_cont, 260, 176);
    lv_obj_set_scrollbar_mode(ui->screen_timer_cont, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_timer_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_timer_cont, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_timer_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_timer_cont, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_timer_cont, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_timer_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_timer_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_timer_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_timer_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_timer_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_timer_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_timer_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_timer_cont_3
    ui->screen_timer_cont_3 = lv_obj_create(ui->screen_timer_cont);
    lv_obj_set_pos(ui->screen_timer_cont_3, 2, 118);
    lv_obj_set_size(ui->screen_timer_cont_3, 258, 51);
    lv_obj_set_scrollbar_mode(ui->screen_timer_cont_3, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_timer_cont_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_timer_cont_3, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_timer_cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_timer_cont_3, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_timer_cont_3, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_timer_cont_3, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_timer_cont_3, 53, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer_cont_3, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer_cont_3, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_timer_cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_timer_cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_timer_cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_timer_cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_timer_cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_timer_sw_3
    ui->screen_timer_sw_3 = lv_switch_create(ui->screen_timer_cont_3);
    lv_obj_set_pos(ui->screen_timer_sw_3, 204, 16);
    lv_obj_set_size(ui->screen_timer_sw_3, 35, 20);

    //Write style for screen_timer_sw_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_timer_sw_3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer_sw_3, lv_color_hex(0xe6e2e6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer_sw_3, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_timer_sw_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_timer_sw_3, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_timer_sw_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style for screen_timer_sw_3, Part: LV_PART_INDICATOR, State: LV_STATE_CHECKED.
    lv_obj_set_style_bg_opa(ui->screen_timer_sw_3, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui->screen_timer_sw_3, lv_color_hex(0x2195f6), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer_sw_3, LV_GRAD_DIR_NONE, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_width(ui->screen_timer_sw_3, 0, LV_PART_INDICATOR | LV_STATE_CHECKED);

    //Write style for screen_timer_sw_3, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_timer_sw_3, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer_sw_3, lv_color_hex(0xffffff), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer_sw_3, LV_GRAD_DIR_NONE, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_timer_sw_3, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_timer_sw_3, 10, LV_PART_KNOB | LV_STATE_DEFAULT);

    //Write codes screen_timer_time3
    ui->screen_timer_time3 = lv_spangroup_create(ui->screen_timer_cont_3);
    lv_spangroup_set_align(ui->screen_timer_time3, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_timer_time3, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_timer_time3, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_timer_time3_span = lv_spangroup_new_span(ui->screen_timer_time3);
    lv_span_set_text(ui->screen_timer_time3_span, "12:12:12");
    lv_style_set_text_color(&ui->screen_timer_time3_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_timer_time3_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_timer_time3_span->style, &lv_font_Barlow__28);
    lv_obj_set_pos(ui->screen_timer_time3, 12, 9);
    lv_obj_set_size(ui->screen_timer_time3, 57, 25);

    //Write style state: LV_STATE_DEFAULT for &style_screen_timer_time3_main_main_default
    static lv_style_t style_screen_timer_time3_main_main_default;
    ui_init_style(&style_screen_timer_time3_main_main_default);

    lv_style_set_border_width(&style_screen_timer_time3_main_main_default, 0);
    lv_style_set_radius(&style_screen_timer_time3_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_timer_time3_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_timer_time3_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_timer_time3_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_timer_time3_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_timer_time3_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_timer_time3_main_main_default, 0);
    lv_obj_add_style(ui->screen_timer_time3, &style_screen_timer_time3_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_timer_time3);

    //Write codes screen_timer_cont_2
    ui->screen_timer_cont_2 = lv_obj_create(ui->screen_timer_cont);
    lv_obj_set_pos(ui->screen_timer_cont_2, 2, 59);
    lv_obj_set_size(ui->screen_timer_cont_2, 258, 51);
    lv_obj_set_scrollbar_mode(ui->screen_timer_cont_2, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_timer_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_timer_cont_2, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_timer_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_timer_cont_2, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_timer_cont_2, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_timer_cont_2, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_timer_cont_2, 53, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer_cont_2, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer_cont_2, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_timer_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_timer_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_timer_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_timer_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_timer_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_timer_sw_2
    ui->screen_timer_sw_2 = lv_switch_create(ui->screen_timer_cont_2);
    lv_obj_set_pos(ui->screen_timer_sw_2, 204, 16);
    lv_obj_set_size(ui->screen_timer_sw_2, 35, 20);

    //Write style for screen_timer_sw_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_timer_sw_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer_sw_2, lv_color_hex(0xe6e2e6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer_sw_2, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_timer_sw_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_timer_sw_2, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_timer_sw_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style for screen_timer_sw_2, Part: LV_PART_INDICATOR, State: LV_STATE_CHECKED.
    lv_obj_set_style_bg_opa(ui->screen_timer_sw_2, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui->screen_timer_sw_2, lv_color_hex(0x2195f6), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer_sw_2, LV_GRAD_DIR_NONE, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_width(ui->screen_timer_sw_2, 0, LV_PART_INDICATOR | LV_STATE_CHECKED);

    //Write style for screen_timer_sw_2, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_timer_sw_2, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer_sw_2, lv_color_hex(0xffffff), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer_sw_2, LV_GRAD_DIR_NONE, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_timer_sw_2, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_timer_sw_2, 10, LV_PART_KNOB | LV_STATE_DEFAULT);

    //Write codes screen_timer_time2
    ui->screen_timer_time2 = lv_spangroup_create(ui->screen_timer_cont_2);
    lv_spangroup_set_align(ui->screen_timer_time2, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_timer_time2, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_timer_time2, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_timer_time2_span = lv_spangroup_new_span(ui->screen_timer_time2);
    lv_span_set_text(ui->screen_timer_time2_span, "12:12:12");
    lv_style_set_text_color(&ui->screen_timer_time2_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_timer_time2_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_timer_time2_span->style, &lv_font_Barlow__28);
    lv_obj_set_pos(ui->screen_timer_time2, 12, 9);
    lv_obj_set_size(ui->screen_timer_time2, 57, 25);

    //Write style state: LV_STATE_DEFAULT for &style_screen_timer_time2_main_main_default
    static lv_style_t style_screen_timer_time2_main_main_default;
    ui_init_style(&style_screen_timer_time2_main_main_default);

    lv_style_set_border_width(&style_screen_timer_time2_main_main_default, 0);
    lv_style_set_radius(&style_screen_timer_time2_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_timer_time2_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_timer_time2_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_timer_time2_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_timer_time2_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_timer_time2_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_timer_time2_main_main_default, 0);
    lv_obj_add_style(ui->screen_timer_time2, &style_screen_timer_time2_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_timer_time2);

    //Write codes screen_timer_cont_1
    ui->screen_timer_cont_1 = lv_obj_create(ui->screen_timer_cont);
    lv_obj_set_pos(ui->screen_timer_cont_1, 2, 0);
    lv_obj_set_size(ui->screen_timer_cont_1, 258, 51);
    lv_obj_set_scrollbar_mode(ui->screen_timer_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_timer_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_timer_cont_1, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_timer_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_timer_cont_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_timer_cont_1, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_timer_cont_1, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_timer_cont_1, 53, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer_cont_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_timer_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_timer_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_timer_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_timer_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_timer_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_timer_sw_1
    ui->screen_timer_sw_1 = lv_switch_create(ui->screen_timer_cont_1);
    lv_obj_set_pos(ui->screen_timer_sw_1, 204, 16);
    lv_obj_set_size(ui->screen_timer_sw_1, 35, 20);

    //Write style for screen_timer_sw_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_timer_sw_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer_sw_1, lv_color_hex(0xe6e2e6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer_sw_1, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_timer_sw_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_timer_sw_1, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_timer_sw_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style for screen_timer_sw_1, Part: LV_PART_INDICATOR, State: LV_STATE_CHECKED.
    lv_obj_set_style_bg_opa(ui->screen_timer_sw_1, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui->screen_timer_sw_1, lv_color_hex(0x2195f6), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer_sw_1, LV_GRAD_DIR_NONE, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_width(ui->screen_timer_sw_1, 0, LV_PART_INDICATOR | LV_STATE_CHECKED);

    //Write style for screen_timer_sw_1, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_timer_sw_1, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer_sw_1, lv_color_hex(0xffffff), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_timer_sw_1, LV_GRAD_DIR_NONE, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_timer_sw_1, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_timer_sw_1, 10, LV_PART_KNOB | LV_STATE_DEFAULT);

    //Write codes screen_timer_time1
    ui->screen_timer_time1 = lv_spangroup_create(ui->screen_timer_cont_1);
    lv_spangroup_set_align(ui->screen_timer_time1, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_timer_time1, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_timer_time1, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_timer_time1_span = lv_spangroup_new_span(ui->screen_timer_time1);
    lv_span_set_text(ui->screen_timer_time1_span, "12:12:12");
    lv_style_set_text_color(&ui->screen_timer_time1_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_timer_time1_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_timer_time1_span->style, &lv_font_Barlow__28);
    lv_obj_set_pos(ui->screen_timer_time1, 12, 9);
    lv_obj_set_size(ui->screen_timer_time1, 86, 25);

    //Write style state: LV_STATE_DEFAULT for &style_screen_timer_time1_main_main_default
    static lv_style_t style_screen_timer_time1_main_main_default;
    ui_init_style(&style_screen_timer_time1_main_main_default);

    lv_style_set_border_width(&style_screen_timer_time1_main_main_default, 0);
    lv_style_set_radius(&style_screen_timer_time1_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_timer_time1_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_timer_time1_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_timer_time1_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_timer_time1_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_timer_time1_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_timer_time1_main_main_default, 0);
    lv_obj_add_style(ui->screen_timer_time1, &style_screen_timer_time1_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_timer_time1);

    //The custom code of screen_timer.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_timer);

    //Init events for screen.
    events_init_screen_timer(ui);
}
