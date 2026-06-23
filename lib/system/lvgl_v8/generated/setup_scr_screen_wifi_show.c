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
#include "lvgl_develop.h"

#if CONFIG_LVGL_OPTION_WIFI_ENABLE

void setup_scr_screen_wifi_show(lv_ui *ui)
{
    //Write codes screen_wifi_show
    ui->screen_wifi_show = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_wifi_show, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_wifi_show, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_wifi_show, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_wifi_show, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_wifi_show, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_wifi_show, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_cont_wifi_disconnected
    ui->screen_wifi_show_cont_wifi_disconnected = lv_obj_create(ui->screen_wifi_show);
    lv_obj_set_pos(ui->screen_wifi_show_cont_wifi_disconnected, 0, 118);
    lv_obj_set_size(ui->screen_wifi_show_cont_wifi_disconnected, 320, 120);
    lv_obj_set_scrollbar_mode(ui->screen_wifi_show_cont_wifi_disconnected, LV_SCROLLBAR_MODE_AUTO);

    //Write style for screen_wifi_show_cont_wifi_disconnected, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_wifi_show_cont_wifi_disconnected, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_cont_wifi_disconnected, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_wifi_show_cont_wifi_disconnected, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_wifi_show_cont_wifi_disconnected, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_wifi_show_cont_wifi_disconnected, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_wifi_show_cont_wifi_disconnected, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_wifi_show_cont_wifi_disconnected, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_wifi_show_cont_wifi_disconnected, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_span_net
    ui->screen_wifi_show_span_net = lv_spangroup_create(ui->screen_wifi_show_cont_wifi_disconnected);
    lv_spangroup_set_align(ui->screen_wifi_show_span_net, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_wifi_show_span_net, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_wifi_show_span_net, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_wifi_show_span_net_span = lv_spangroup_new_span(ui->screen_wifi_show_span_net);
    lv_span_set_text(ui->screen_wifi_show_span_net_span, "仅支持2.4G网络");
    lv_style_set_text_color(&ui->screen_wifi_show_span_net_span->style, lv_color_hex(0xd7d7d7));
    lv_style_set_text_decor(&ui->screen_wifi_show_span_net_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_wifi_show_span_net_span->style, &lv_font_MiSansDemibold_14);
    lv_obj_set_pos(ui->screen_wifi_show_span_net, 24, 12);
    lv_obj_set_size(ui->screen_wifi_show_span_net, 85, 16);

    //Write style state: LV_STATE_DEFAULT for &style_screen_wifi_show_span_net_main_main_default
    static lv_style_t style_screen_wifi_show_span_net_main_main_default;
    ui_init_style(&style_screen_wifi_show_span_net_main_main_default);

    lv_style_set_border_width(&style_screen_wifi_show_span_net_main_main_default, 0);
    lv_style_set_radius(&style_screen_wifi_show_span_net_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_wifi_show_span_net_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_wifi_show_span_net_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_wifi_show_span_net_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_wifi_show_span_net_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_wifi_show_span_net_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_wifi_show_span_net_main_main_default, 0);
    lv_obj_add_style(ui->screen_wifi_show_span_net, &style_screen_wifi_show_span_net_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_wifi_show_span_net);

    //Write codes screen_wifi_show_cont_wifi1
    ui->screen_wifi_show_cont_wifi1 = lv_obj_create(ui->screen_wifi_show_cont_wifi_disconnected);
    lv_obj_set_pos(ui->screen_wifi_show_cont_wifi1, 12, 39);
    lv_obj_set_size(ui->screen_wifi_show_cont_wifi1, 295, 35);
    lv_obj_set_scrollbar_mode(ui->screen_wifi_show_cont_wifi1, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_wifi_show_cont_wifi1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_wifi_show_cont_wifi1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_cont_wifi1, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_wifi_show_cont_wifi1, 39, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_wifi_show_cont_wifi1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_wifi_show_cont_wifi1, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_wifi_show_cont_wifi1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_wifi_show_cont_wifi1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_wifi_show_cont_wifi1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_wifi_show_cont_wifi1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_wifi_show_cont_wifi1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_span_wifi_name1
    ui->screen_wifi_show_span_wifi_name1 = lv_spangroup_create(ui->screen_wifi_show_cont_wifi1);
    lv_spangroup_set_align(ui->screen_wifi_show_span_wifi_name1, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_wifi_show_span_wifi_name1, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_wifi_show_span_wifi_name1, LV_SPAN_MODE_FIXED);
    //create span
    ui->screen_wifi_show_span_wifi_name1_span = lv_spangroup_new_span(ui->screen_wifi_show_span_wifi_name1);
    lv_span_set_text(ui->screen_wifi_show_span_wifi_name1_span, "数芯语1");
    lv_style_set_text_color(&ui->screen_wifi_show_span_wifi_name1_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_wifi_show_span_wifi_name1_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_wifi_show_span_wifi_name1_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_wifi_show_span_wifi_name1, 12, 9);
    lv_obj_set_size(ui->screen_wifi_show_span_wifi_name1, 221, 20);

    //Write style state: LV_STATE_DEFAULT for &style_screen_wifi_show_span_wifi_name1_main_main_default
    static lv_style_t style_screen_wifi_show_span_wifi_name1_main_main_default;
    ui_init_style(&style_screen_wifi_show_span_wifi_name1_main_main_default);

    lv_style_set_border_width(&style_screen_wifi_show_span_wifi_name1_main_main_default, 0);
    lv_style_set_radius(&style_screen_wifi_show_span_wifi_name1_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_wifi_show_span_wifi_name1_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_wifi_show_span_wifi_name1_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_wifi_show_span_wifi_name1_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_wifi_show_span_wifi_name1_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_wifi_show_span_wifi_name1_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_wifi_show_span_wifi_name1_main_main_default, 0);
    lv_obj_add_style(ui->screen_wifi_show_span_wifi_name1, &style_screen_wifi_show_span_wifi_name1_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_wifi_show_span_wifi_name1);

    //Write codes screen_wifi_show_img_wifi1
    ui->screen_wifi_show_img_wifi1 = lv_img_create(ui->screen_wifi_show_cont_wifi1);
    lv_obj_add_flag(ui->screen_wifi_show_img_wifi1, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_wifi_show_img_wifi1, &_set_wifi_alpha_16x14);
    lv_img_set_pivot(ui->screen_wifi_show_img_wifi1, 50, 50);
    lv_img_set_angle(ui->screen_wifi_show_img_wifi1, 0);
    lv_obj_set_pos(ui->screen_wifi_show_img_wifi1, 267, 12);
    lv_obj_set_size(ui->screen_wifi_show_img_wifi1, 16, 14);

    //Write style for screen_wifi_show_img_wifi1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_wifi_show_img_wifi1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_wifi_show_img_wifi1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_img_wifi1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_wifi_show_img_wifi1, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_img_lock1
    ui->screen_wifi_show_img_lock1 = lv_img_create(ui->screen_wifi_show_cont_wifi1);
    lv_obj_add_flag(ui->screen_wifi_show_img_lock1, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_wifi_show_img_lock1, &_lock_alpha_18x16);
    lv_img_set_pivot(ui->screen_wifi_show_img_lock1, 50, 50);
    lv_img_set_angle(ui->screen_wifi_show_img_lock1, 0);
    lv_obj_set_pos(ui->screen_wifi_show_img_lock1, 240, 11);
    lv_obj_set_size(ui->screen_wifi_show_img_lock1, 18, 16);

    //Write style for screen_wifi_show_img_lock1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_wifi_show_img_lock1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_wifi_show_img_lock1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_img_lock1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_wifi_show_img_lock1, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_cont_wifi2
    ui->screen_wifi_show_cont_wifi2 = lv_obj_create(ui->screen_wifi_show_cont_wifi_disconnected);
    lv_obj_set_pos(ui->screen_wifi_show_cont_wifi2, 12, 83);
    lv_obj_set_size(ui->screen_wifi_show_cont_wifi2, 295, 35);
    lv_obj_set_scrollbar_mode(ui->screen_wifi_show_cont_wifi2, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_wifi_show_cont_wifi2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_wifi_show_cont_wifi2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_cont_wifi2, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_wifi_show_cont_wifi2, 39, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_wifi_show_cont_wifi2, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_wifi_show_cont_wifi2, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_wifi_show_cont_wifi2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_wifi_show_cont_wifi2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_wifi_show_cont_wifi2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_wifi_show_cont_wifi2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_wifi_show_cont_wifi2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_span_wifi_name2
    ui->screen_wifi_show_span_wifi_name2 = lv_spangroup_create(ui->screen_wifi_show_cont_wifi2);
    lv_spangroup_set_align(ui->screen_wifi_show_span_wifi_name2, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_wifi_show_span_wifi_name2, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_wifi_show_span_wifi_name2, LV_SPAN_MODE_BREAK);
    //create span
    ui->screen_wifi_show_span_wifi_name2_span = lv_spangroup_new_span(ui->screen_wifi_show_span_wifi_name2);
    lv_span_set_text(ui->screen_wifi_show_span_wifi_name2_span, "数芯语2");
    lv_style_set_text_color(&ui->screen_wifi_show_span_wifi_name2_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_wifi_show_span_wifi_name2_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_wifi_show_span_wifi_name2_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_wifi_show_span_wifi_name2, 12, 9);
    lv_obj_set_size(ui->screen_wifi_show_span_wifi_name2, 224, 20);

    //Write style state: LV_STATE_DEFAULT for &style_screen_wifi_show_span_wifi_name2_main_main_default
    static lv_style_t style_screen_wifi_show_span_wifi_name2_main_main_default;
    ui_init_style(&style_screen_wifi_show_span_wifi_name2_main_main_default);

    lv_style_set_border_width(&style_screen_wifi_show_span_wifi_name2_main_main_default, 0);
    lv_style_set_radius(&style_screen_wifi_show_span_wifi_name2_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_wifi_show_span_wifi_name2_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_wifi_show_span_wifi_name2_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_wifi_show_span_wifi_name2_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_wifi_show_span_wifi_name2_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_wifi_show_span_wifi_name2_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_wifi_show_span_wifi_name2_main_main_default, 0);
    lv_obj_add_style(ui->screen_wifi_show_span_wifi_name2, &style_screen_wifi_show_span_wifi_name2_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_wifi_show_span_wifi_name2);

    //Write codes screen_wifi_show_img_wifi2
    ui->screen_wifi_show_img_wifi2 = lv_img_create(ui->screen_wifi_show_cont_wifi2);
    lv_obj_add_flag(ui->screen_wifi_show_img_wifi2, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_wifi_show_img_wifi2, &_set_wifi_alpha_16x14);
    lv_img_set_pivot(ui->screen_wifi_show_img_wifi2, 50, 50);
    lv_img_set_angle(ui->screen_wifi_show_img_wifi2, 0);
    lv_obj_set_pos(ui->screen_wifi_show_img_wifi2, 267, 12);
    lv_obj_set_size(ui->screen_wifi_show_img_wifi2, 16, 14);

    //Write style for screen_wifi_show_img_wifi2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_wifi_show_img_wifi2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_wifi_show_img_wifi2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_img_wifi2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_wifi_show_img_wifi2, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_img_lock2
    ui->screen_wifi_show_img_lock2 = lv_img_create(ui->screen_wifi_show_cont_wifi2);
    lv_obj_add_flag(ui->screen_wifi_show_img_lock2, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_wifi_show_img_lock2, &_lock_alpha_18x16);
    lv_img_set_pivot(ui->screen_wifi_show_img_lock2, 50, 50);
    lv_img_set_angle(ui->screen_wifi_show_img_lock2, 0);
    lv_obj_set_pos(ui->screen_wifi_show_img_lock2, 240, 11);
    lv_obj_set_size(ui->screen_wifi_show_img_lock2, 18, 16);

    //Write style for screen_wifi_show_img_lock2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_wifi_show_img_lock2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_wifi_show_img_lock2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_img_lock2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_wifi_show_img_lock2, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_cont_wifi
    ui->screen_wifi_show_cont_wifi = lv_obj_create(ui->screen_wifi_show);
    lv_obj_set_pos(ui->screen_wifi_show_cont_wifi, 0, 40);
    lv_obj_set_size(ui->screen_wifi_show_cont_wifi, 320, 78);
    lv_obj_set_scrollbar_mode(ui->screen_wifi_show_cont_wifi, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_wifi_show_cont_wifi, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_wifi_show_cont_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_cont_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_wifi_show_cont_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_wifi_show_cont_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_wifi_show_cont_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_wifi_show_cont_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_wifi_show_cont_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_wifi_show_cont_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_span_connect_suc
    ui->screen_wifi_show_span_connect_suc = lv_spangroup_create(ui->screen_wifi_show_cont_wifi);
    lv_spangroup_set_align(ui->screen_wifi_show_span_connect_suc, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_wifi_show_span_connect_suc, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_wifi_show_span_connect_suc, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_wifi_show_span_connect_suc_span = lv_spangroup_new_span(ui->screen_wifi_show_span_connect_suc);
    lv_span_set_text(ui->screen_wifi_show_span_connect_suc_span, "已连接");
    lv_style_set_text_color(&ui->screen_wifi_show_span_connect_suc_span->style, lv_color_hex(0xd7d7d7));
    lv_style_set_text_decor(&ui->screen_wifi_show_span_connect_suc_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_wifi_show_span_connect_suc_span->style, &lv_font_MiSansDemibold_14);
    lv_obj_set_pos(ui->screen_wifi_show_span_connect_suc, 24, 12);
    lv_obj_set_size(ui->screen_wifi_show_span_connect_suc, 61, 14);

    //Write style state: LV_STATE_DEFAULT for &style_screen_wifi_show_span_connect_suc_main_main_default
    static lv_style_t style_screen_wifi_show_span_connect_suc_main_main_default;
    ui_init_style(&style_screen_wifi_show_span_connect_suc_main_main_default);

    lv_style_set_border_width(&style_screen_wifi_show_span_connect_suc_main_main_default, 0);
    lv_style_set_radius(&style_screen_wifi_show_span_connect_suc_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_wifi_show_span_connect_suc_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_wifi_show_span_connect_suc_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_wifi_show_span_connect_suc_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_wifi_show_span_connect_suc_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_wifi_show_span_connect_suc_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_wifi_show_span_connect_suc_main_main_default, 0);
    lv_obj_add_style(ui->screen_wifi_show_span_connect_suc, &style_screen_wifi_show_span_connect_suc_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_wifi_show_span_connect_suc);

    //Write codes screen_wifi_show_cont_wifi_success
    ui->screen_wifi_show_cont_wifi_success = lv_obj_create(ui->screen_wifi_show_cont_wifi);
    lv_obj_set_pos(ui->screen_wifi_show_cont_wifi_success, 12, 39);
    lv_obj_set_size(ui->screen_wifi_show_cont_wifi_success, 295, 35);
    lv_obj_set_scrollbar_mode(ui->screen_wifi_show_cont_wifi_success, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_wifi_show_cont_wifi_success, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_wifi_show_cont_wifi_success, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_wifi_show_cont_wifi_success, 138, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_wifi_show_cont_wifi_success, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_wifi_show_cont_wifi_success, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_cont_wifi_success, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_wifi_show_cont_wifi_success, 39, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_wifi_show_cont_wifi_success, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_wifi_show_cont_wifi_success, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_wifi_show_cont_wifi_success, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_wifi_show_cont_wifi_success, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_wifi_show_cont_wifi_success, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_wifi_show_cont_wifi_success, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_wifi_show_cont_wifi_success, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_span_wifi_success
    ui->screen_wifi_show_span_wifi_success = lv_spangroup_create(ui->screen_wifi_show_cont_wifi_success);
    lv_spangroup_set_align(ui->screen_wifi_show_span_wifi_success, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_wifi_show_span_wifi_success, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_wifi_show_span_wifi_success, LV_SPAN_MODE_FIXED);
    //create span
    ui->screen_wifi_show_span_wifi_success_span = lv_spangroup_new_span(ui->screen_wifi_show_span_wifi_success);
    lv_span_set_text(ui->screen_wifi_show_span_wifi_success_span, "数芯语");
    lv_style_set_text_color(&ui->screen_wifi_show_span_wifi_success_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_wifi_show_span_wifi_success_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_wifi_show_span_wifi_success_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_wifi_show_span_wifi_success, 10, 7);
    lv_obj_set_size(ui->screen_wifi_show_span_wifi_success, 224, 20);

    //Write style state: LV_STATE_DEFAULT for &style_screen_wifi_show_span_wifi_success_main_main_default
    static lv_style_t style_screen_wifi_show_span_wifi_success_main_main_default;
    ui_init_style(&style_screen_wifi_show_span_wifi_success_main_main_default);

    lv_style_set_border_width(&style_screen_wifi_show_span_wifi_success_main_main_default, 0);
    lv_style_set_radius(&style_screen_wifi_show_span_wifi_success_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_wifi_show_span_wifi_success_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_wifi_show_span_wifi_success_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_wifi_show_span_wifi_success_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_wifi_show_span_wifi_success_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_wifi_show_span_wifi_success_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_wifi_show_span_wifi_success_main_main_default, 0);
    lv_obj_add_style(ui->screen_wifi_show_span_wifi_success, &style_screen_wifi_show_span_wifi_success_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_wifi_show_span_wifi_success);

    //Write codes screen_wifi_show_img_suceess
    ui->screen_wifi_show_img_suceess = lv_img_create(ui->screen_wifi_show_cont_wifi_success);
    lv_obj_add_flag(ui->screen_wifi_show_img_suceess, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_wifi_show_img_suceess, &_connect_success_alpha_20x20);
    lv_img_set_pivot(ui->screen_wifi_show_img_suceess, 50, 50);
    lv_img_set_angle(ui->screen_wifi_show_img_suceess, 0);
    lv_obj_set_pos(ui->screen_wifi_show_img_suceess, 241, 7);
    lv_obj_set_size(ui->screen_wifi_show_img_suceess, 20, 20);

    //Write style for screen_wifi_show_img_suceess, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_wifi_show_img_suceess, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_wifi_show_img_suceess, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_img_suceess, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_wifi_show_img_suceess, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_img_wifi_success
    ui->screen_wifi_show_img_wifi_success = lv_img_create(ui->screen_wifi_show_cont_wifi_success);
    lv_obj_add_flag(ui->screen_wifi_show_img_wifi_success, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_wifi_show_img_wifi_success, &_set_wifi_alpha_16x14);
    lv_img_set_pivot(ui->screen_wifi_show_img_wifi_success, 50, 50);
    lv_img_set_angle(ui->screen_wifi_show_img_wifi_success, 0);
    lv_obj_set_pos(ui->screen_wifi_show_img_wifi_success, 264, 10);
    lv_obj_set_size(ui->screen_wifi_show_img_wifi_success, 16, 14);

    //Write style for screen_wifi_show_img_wifi_success, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_wifi_show_img_wifi_success, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_wifi_show_img_wifi_success, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_img_wifi_success, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_wifi_show_img_wifi_success, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_cont_title
    ui->screen_wifi_show_cont_title = lv_obj_create(ui->screen_wifi_show);
    lv_obj_set_pos(ui->screen_wifi_show_cont_title, 0, 0);
    lv_obj_set_size(ui->screen_wifi_show_cont_title, 320, 40);
    lv_obj_set_scrollbar_mode(ui->screen_wifi_show_cont_title, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_wifi_show_cont_title, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_wifi_show_cont_title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_cont_title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_wifi_show_cont_title, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_wifi_show_cont_title, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_wifi_show_cont_title, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_wifi_show_cont_title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_wifi_show_cont_title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_wifi_show_cont_title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_wifi_show_cont_title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_wifi_show_cont_title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_img_return
    ui->screen_wifi_show_img_return = lv_img_create(ui->screen_wifi_show_cont_title);
    lv_obj_add_flag(ui->screen_wifi_show_img_return, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_wifi_show_img_return, &_return_alpha_20x20);
    lv_img_set_pivot(ui->screen_wifi_show_img_return, 50, 50);
    lv_img_set_angle(ui->screen_wifi_show_img_return, 0);
    lv_obj_set_pos(ui->screen_wifi_show_img_return, 12, 10);
    lv_obj_set_size(ui->screen_wifi_show_img_return, 20, 20);

    //Write style for screen_wifi_show_img_return, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_wifi_show_img_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_wifi_show_img_return, 128, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_img_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_wifi_show_img_return, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_cont_QR_code
    ui->screen_wifi_show_cont_QR_code = lv_obj_create(ui->screen_wifi_show_cont_title);
    lv_obj_set_pos(ui->screen_wifi_show_cont_QR_code, 143, 5);
    lv_obj_set_size(ui->screen_wifi_show_cont_QR_code, 95, 30);
    lv_obj_set_scrollbar_mode(ui->screen_wifi_show_cont_QR_code, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_wifi_show_cont_QR_code, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_wifi_show_cont_QR_code, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_wifi_show_cont_QR_code, 129, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_wifi_show_cont_QR_code, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_wifi_show_cont_QR_code, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_cont_QR_code, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_wifi_show_cont_QR_code, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_wifi_show_cont_QR_code, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_wifi_show_cont_QR_code, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_wifi_show_cont_QR_code, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_wifi_show_cont_QR_code, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_wifi_show_cont_QR_code, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_img_QR_code
    ui->screen_wifi_show_img_QR_code = lv_img_create(ui->screen_wifi_show_cont_QR_code);
    lv_obj_add_flag(ui->screen_wifi_show_img_QR_code, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_wifi_show_img_QR_code, &_QR_code_alpha_30x30);
    lv_img_set_pivot(ui->screen_wifi_show_img_QR_code, 50, 50);
    lv_img_set_angle(ui->screen_wifi_show_img_QR_code, 0);
    lv_obj_set_pos(ui->screen_wifi_show_img_QR_code, 2, 0);
    lv_obj_set_size(ui->screen_wifi_show_img_QR_code, 30, 30);

    //Write style for screen_wifi_show_img_QR_code, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_wifi_show_img_QR_code, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_wifi_show_img_QR_code, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_img_QR_code, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_wifi_show_img_QR_code, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_label_QR_code
    ui->screen_wifi_show_label_QR_code = lv_label_create(ui->screen_wifi_show_cont_QR_code);
    lv_label_set_text(ui->screen_wifi_show_label_QR_code, "扫码配网");
    lv_label_set_long_mode(ui->screen_wifi_show_label_QR_code, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_wifi_show_label_QR_code, 29, 7);
    lv_obj_set_size(ui->screen_wifi_show_label_QR_code, 59, 14);

    //Write style for screen_wifi_show_label_QR_code, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_wifi_show_label_QR_code, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_label_QR_code, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_wifi_show_label_QR_code, lv_color_hex(0xd7d7d7), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_wifi_show_label_QR_code, &lv_font_MiSansDemibold_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_wifi_show_label_QR_code, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_wifi_show_label_QR_code, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_wifi_show_label_QR_code, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_wifi_show_label_QR_code, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_wifi_show_label_QR_code, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_wifi_show_label_QR_code, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_wifi_show_label_QR_code, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_wifi_show_label_QR_code, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_wifi_show_label_QR_code, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_wifi_show_label_QR_code, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_cont_refresh
    ui->screen_wifi_show_cont_refresh = lv_obj_create(ui->screen_wifi_show_cont_title);
    lv_obj_set_pos(ui->screen_wifi_show_cont_refresh, 242, 5);
    lv_obj_set_size(ui->screen_wifi_show_cont_refresh, 64, 30);
    lv_obj_set_scrollbar_mode(ui->screen_wifi_show_cont_refresh, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_wifi_show_cont_refresh, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_wifi_show_cont_refresh, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_wifi_show_cont_refresh, 129, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_wifi_show_cont_refresh, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_wifi_show_cont_refresh, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_cont_refresh, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_wifi_show_cont_refresh, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_wifi_show_cont_refresh, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_wifi_show_cont_refresh, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_wifi_show_cont_refresh, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_wifi_show_cont_refresh, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_wifi_show_cont_refresh, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_img_refresh
    ui->screen_wifi_show_img_refresh = lv_img_create(ui->screen_wifi_show_cont_refresh);
    lv_obj_add_flag(ui->screen_wifi_show_img_refresh, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_wifi_show_img_refresh, &_refresh_alpha_15x15);
    lv_img_set_pivot(ui->screen_wifi_show_img_refresh, 7, 6);
    lv_img_set_angle(ui->screen_wifi_show_img_refresh, 0);
    lv_obj_set_pos(ui->screen_wifi_show_img_refresh, 7, 7);
    lv_obj_set_size(ui->screen_wifi_show_img_refresh, 15, 15);

    //Write style for screen_wifi_show_img_refresh, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_wifi_show_img_refresh, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_wifi_show_img_refresh, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_img_refresh, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_wifi_show_img_refresh, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_label_refresh
    ui->screen_wifi_show_label_refresh = lv_label_create(ui->screen_wifi_show_cont_refresh);
    lv_label_set_text(ui->screen_wifi_show_label_refresh, "刷新");
    lv_label_set_long_mode(ui->screen_wifi_show_label_refresh, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_wifi_show_label_refresh, 27, 7);
    lv_obj_set_size(ui->screen_wifi_show_label_refresh, 29, 14);

    //Write style for screen_wifi_show_label_refresh, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_wifi_show_label_refresh, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_label_refresh, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_wifi_show_label_refresh, lv_color_hex(0xd7d7d7), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_wifi_show_label_refresh, &lv_font_MiSansDemibold_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_wifi_show_label_refresh, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_wifi_show_label_refresh, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_wifi_show_label_refresh, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_wifi_show_label_refresh, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_wifi_show_label_refresh, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_wifi_show_label_refresh, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_wifi_show_label_refresh, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_wifi_show_label_refresh, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_wifi_show_label_refresh, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_wifi_show_label_refresh, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_span_wifi
    ui->screen_wifi_show_span_wifi = lv_spangroup_create(ui->screen_wifi_show_cont_title);
    lv_spangroup_set_align(ui->screen_wifi_show_span_wifi, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_wifi_show_span_wifi, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_wifi_show_span_wifi, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_wifi_show_span_wifi_span = lv_spangroup_new_span(ui->screen_wifi_show_span_wifi);
    lv_span_set_text(ui->screen_wifi_show_span_wifi_span, "选择Wi-Fi");
    lv_style_set_text_color(&ui->screen_wifi_show_span_wifi_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_wifi_show_span_wifi_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_wifi_show_span_wifi_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_wifi_show_span_wifi, 42, 9);
    lv_obj_set_size(ui->screen_wifi_show_span_wifi, 85, 20);

    //Write style state: LV_STATE_DEFAULT for &style_screen_wifi_show_span_wifi_main_main_default
    static lv_style_t style_screen_wifi_show_span_wifi_main_main_default;
    ui_init_style(&style_screen_wifi_show_span_wifi_main_main_default);

    lv_style_set_border_width(&style_screen_wifi_show_span_wifi_main_main_default, 0);
    lv_style_set_radius(&style_screen_wifi_show_span_wifi_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_wifi_show_span_wifi_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_wifi_show_span_wifi_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_wifi_show_span_wifi_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_wifi_show_span_wifi_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_wifi_show_span_wifi_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_wifi_show_span_wifi_main_main_default, 0);
    lv_obj_add_style(ui->screen_wifi_show_span_wifi, &style_screen_wifi_show_span_wifi_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_wifi_show_span_wifi);

    //Write codes screen_wifi_show_cont_wifi_choose
    ui->screen_wifi_show_cont_wifi_choose = lv_obj_create(ui->screen_wifi_show);
    lv_obj_set_pos(ui->screen_wifi_show_cont_wifi_choose, 336, 0);
    lv_obj_set_size(ui->screen_wifi_show_cont_wifi_choose, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_wifi_show_cont_wifi_choose, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(ui->screen_wifi_show_cont_wifi_choose, LV_OBJ_FLAG_HIDDEN);

    //Write style for screen_wifi_show_cont_wifi_choose, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_wifi_show_cont_wifi_choose, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_cont_wifi_choose, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_wifi_show_cont_wifi_choose, 231, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_wifi_show_cont_wifi_choose, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_wifi_show_cont_wifi_choose, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_wifi_show_cont_wifi_choose, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_wifi_show_cont_wifi_choose, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_wifi_show_cont_wifi_choose, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_wifi_show_cont_wifi_choose, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_wifi_show_cont_wifi_choose, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_img_wifi
    ui->screen_wifi_show_img_wifi = lv_img_create(ui->screen_wifi_show_cont_wifi_choose);
    lv_obj_add_flag(ui->screen_wifi_show_img_wifi, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_wifi_show_img_wifi, &_set_wifi_alpha_80x62);
    lv_img_set_pivot(ui->screen_wifi_show_img_wifi, 50, 50);
    lv_img_set_angle(ui->screen_wifi_show_img_wifi, 0);
    lv_obj_set_pos(ui->screen_wifi_show_img_wifi, 54, 60);
    lv_obj_set_size(ui->screen_wifi_show_img_wifi, 80, 62);

    //Write style for screen_wifi_show_img_wifi, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_wifi_show_img_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_wifi_show_img_wifi, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_img_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_wifi_show_img_wifi, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_span_wifi_name
    ui->screen_wifi_show_span_wifi_name = lv_spangroup_create(ui->screen_wifi_show_cont_wifi_choose);
    lv_spangroup_set_align(ui->screen_wifi_show_span_wifi_name, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_wifi_show_span_wifi_name, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_wifi_show_span_wifi_name, LV_SPAN_MODE_FIXED);
    //create span
    ui->screen_wifi_show_span_wifi_name_span = lv_spangroup_new_span(ui->screen_wifi_show_span_wifi_name);
    lv_span_set_text(ui->screen_wifi_show_span_wifi_name_span, "数芯语");
    lv_style_set_text_color(&ui->screen_wifi_show_span_wifi_name_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_wifi_show_span_wifi_name_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_wifi_show_span_wifi_name_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_wifi_show_span_wifi_name, 147, 71);
    lv_obj_set_size(ui->screen_wifi_show_span_wifi_name, 156, 20);

    //Write style state: LV_STATE_DEFAULT for &style_screen_wifi_show_span_wifi_name_main_main_default
    static lv_style_t style_screen_wifi_show_span_wifi_name_main_main_default;
    ui_init_style(&style_screen_wifi_show_span_wifi_name_main_main_default);

    lv_style_set_border_width(&style_screen_wifi_show_span_wifi_name_main_main_default, 0);
    lv_style_set_radius(&style_screen_wifi_show_span_wifi_name_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_wifi_show_span_wifi_name_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_wifi_show_span_wifi_name_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_wifi_show_span_wifi_name_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_wifi_show_span_wifi_name_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_wifi_show_span_wifi_name_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_wifi_show_span_wifi_name_main_main_default, 0);
    lv_obj_add_style(ui->screen_wifi_show_span_wifi_name, &style_screen_wifi_show_span_wifi_name_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_wifi_show_span_wifi_name);

    //Write codes screen_wifi_show_span_wifi_status
    ui->screen_wifi_show_span_wifi_status = lv_spangroup_create(ui->screen_wifi_show_cont_wifi_choose);
    lv_spangroup_set_align(ui->screen_wifi_show_span_wifi_status, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_wifi_show_span_wifi_status, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_wifi_show_span_wifi_status, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_wifi_show_span_wifi_status_span = lv_spangroup_new_span(ui->screen_wifi_show_span_wifi_status);
    lv_span_set_text(ui->screen_wifi_show_span_wifi_status_span, "已连接");
    lv_style_set_text_color(&ui->screen_wifi_show_span_wifi_status_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_wifi_show_span_wifi_status_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_wifi_show_span_wifi_status_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_wifi_show_span_wifi_status, 147, 104);
    lv_obj_set_size(ui->screen_wifi_show_span_wifi_status, 85, 20);

    //Write style state: LV_STATE_DEFAULT for &style_screen_wifi_show_span_wifi_status_main_main_default
    static lv_style_t style_screen_wifi_show_span_wifi_status_main_main_default;
    ui_init_style(&style_screen_wifi_show_span_wifi_status_main_main_default);

    lv_style_set_border_width(&style_screen_wifi_show_span_wifi_status_main_main_default, 0);
    lv_style_set_radius(&style_screen_wifi_show_span_wifi_status_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_wifi_show_span_wifi_status_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_wifi_show_span_wifi_status_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_wifi_show_span_wifi_status_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_wifi_show_span_wifi_status_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_wifi_show_span_wifi_status_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_wifi_show_span_wifi_status_main_main_default, 0);
    lv_obj_add_style(ui->screen_wifi_show_span_wifi_status, &style_screen_wifi_show_span_wifi_status_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_wifi_show_span_wifi_status);

    //Write codes screen_wifi_show_btn_del
    ui->screen_wifi_show_btn_del = lv_btn_create(ui->screen_wifi_show_cont_wifi_choose);
    ui->screen_wifi_show_btn_del_label = lv_label_create(ui->screen_wifi_show_btn_del);
    lv_label_set_text(ui->screen_wifi_show_btn_del_label, "移除此网络");
    lv_label_set_long_mode(ui->screen_wifi_show_btn_del_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_wifi_show_btn_del_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_wifi_show_btn_del, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_wifi_show_btn_del_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_wifi_show_btn_del, 43, 161);
    lv_obj_set_size(ui->screen_wifi_show_btn_del, 233, 42);

    //Write style for screen_wifi_show_btn_del, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_wifi_show_btn_del, 40, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_wifi_show_btn_del, lv_color_hex(0xff0027), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_wifi_show_btn_del, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_wifi_show_btn_del, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_wifi_show_btn_del, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_wifi_show_btn_del, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_wifi_show_btn_del, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_btn_del, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_wifi_show_btn_del, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_wifi_show_btn_del, lv_color_hex(0xb42323), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_wifi_show_btn_del, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_wifi_show_btn_del, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_wifi_show_btn_del, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_show_img_return_choose
    ui->screen_wifi_show_img_return_choose = lv_img_create(ui->screen_wifi_show_cont_wifi_choose);
    lv_obj_add_flag(ui->screen_wifi_show_img_return_choose, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_wifi_show_img_return_choose, &_return_alpha_20x20);
    lv_img_set_pivot(ui->screen_wifi_show_img_return_choose, 50, 50);
    lv_img_set_angle(ui->screen_wifi_show_img_return_choose, 0);
    lv_obj_set_pos(ui->screen_wifi_show_img_return_choose, 12, 8);
    lv_obj_set_size(ui->screen_wifi_show_img_return_choose, 20, 20);

    //Write style for screen_wifi_show_img_return_choose, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_wifi_show_img_return_choose, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_wifi_show_img_return_choose, 128, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_show_img_return_choose, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_wifi_show_img_return_choose, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //The custom code of screen_wifi_show.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_wifi_show);

    //Init events for screen.
    events_init_screen_wifi_show(ui);
}

#else

void setup_scr_screen_wifi_show(lv_ui *ui)
{
    (void)ui;
}

#endif
