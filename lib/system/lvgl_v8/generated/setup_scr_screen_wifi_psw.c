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

void setup_scr_screen_wifi_psw(lv_ui *ui)
{
    //Write codes screen_wifi_psw
    ui->screen_wifi_psw = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_wifi_psw, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_wifi_psw, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_wifi_psw, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_wifi_psw, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_wifi_psw, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_wifi_psw, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_psw_ta_input
    ui->screen_wifi_psw_ta_input = lv_textarea_create(ui->screen_wifi_psw);
    lv_textarea_set_text(ui->screen_wifi_psw_ta_input, "");
    lv_textarea_set_placeholder_text(ui->screen_wifi_psw_ta_input, "");
#if CONFIG_LVGL_WIFI_PASSWORD_MASK_ENABLE
    lv_textarea_set_password_bullet(ui->screen_wifi_psw_ta_input, "*");
    lv_textarea_set_password_mode(ui->screen_wifi_psw_ta_input, true);
    lv_textarea_set_password_show_time(ui->screen_wifi_psw_ta_input, 0);
#else
    lv_textarea_set_password_mode(ui->screen_wifi_psw_ta_input, false);
#endif
    lv_textarea_set_one_line(ui->screen_wifi_psw_ta_input, true);
    lv_textarea_set_accepted_chars(ui->screen_wifi_psw_ta_input, "");
    lv_textarea_set_max_length(ui->screen_wifi_psw_ta_input, 32);
    lv_obj_set_scrollbar_mode(ui->screen_wifi_psw_ta_input, LV_SCROLLBAR_MODE_OFF);
#if LV_USE_KEYBOARD != 0 || LV_USE_ZH_KEYBOARD != 0
    lv_obj_add_event_cb(ui->screen_wifi_psw_ta_input, ta_event_cb, LV_EVENT_ALL, ui->g_kb_top_layer);
#endif
    lv_obj_set_pos(ui->screen_wifi_psw_ta_input, 16, 47);
    lv_obj_set_size(ui->screen_wifi_psw_ta_input, 287, 37);

    //Write style for screen_wifi_psw_ta_input, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_text_color(ui->screen_wifi_psw_ta_input, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_wifi_psw_ta_input, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_wifi_psw_ta_input, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_wifi_psw_ta_input, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_wifi_psw_ta_input, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_wifi_psw_ta_input, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_wifi_psw_ta_input, lv_color_hex(0x2d2d2d), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_wifi_psw_ta_input, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_wifi_psw_ta_input, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_wifi_psw_ta_input, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_wifi_psw_ta_input, lv_color_hex(0xe6e6e6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_wifi_psw_ta_input, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_wifi_psw_ta_input, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_wifi_psw_ta_input, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_wifi_psw_ta_input, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_wifi_psw_ta_input, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_wifi_psw_ta_input, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_psw_ta_input, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    /* 减少输入时文字上下微移：禁止纵向滚动与焦点整页滚动、内层单行裁剪 */
    lv_obj_clear_flag(ui->screen_wifi_psw_ta_input, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_scroll_dir(ui->screen_wifi_psw_ta_input, LV_DIR_HOR);
    lv_obj_set_scroll_snap_y(ui->screen_wifi_psw_ta_input, LV_SCROLL_SNAP_NONE);
    lv_label_set_long_mode(lv_textarea_get_label(ui->screen_wifi_psw_ta_input), LV_LABEL_LONG_CLIP);

    //Write codes screen_wifi_psw_img_return
    ui->screen_wifi_psw_img_return = lv_img_create(ui->screen_wifi_psw);
    lv_obj_add_flag(ui->screen_wifi_psw_img_return, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_wifi_psw_img_return, &_return_alpha_20x20);
    lv_img_set_pivot(ui->screen_wifi_psw_img_return, 50, 50);
    lv_img_set_angle(ui->screen_wifi_psw_img_return, 0);
    lv_obj_set_pos(ui->screen_wifi_psw_img_return, 12, 10);
    lv_obj_set_size(ui->screen_wifi_psw_img_return, 20, 20);

    //Write style for screen_wifi_psw_img_return, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_wifi_psw_img_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_wifi_psw_img_return, 128, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_psw_img_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_wifi_psw_img_return, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_psw_span_wifi_name
    ui->screen_wifi_psw_span_wifi_name = lv_spangroup_create(ui->screen_wifi_psw);
    lv_spangroup_set_align(ui->screen_wifi_psw_span_wifi_name, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_wifi_psw_span_wifi_name, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_wifi_psw_span_wifi_name, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_wifi_psw_span_wifi_name_span = lv_spangroup_new_span(ui->screen_wifi_psw_span_wifi_name);
    lv_span_set_text(ui->screen_wifi_psw_span_wifi_name_span, "数芯语");
    lv_style_set_text_color(&ui->screen_wifi_psw_span_wifi_name_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_wifi_psw_span_wifi_name_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_wifi_psw_span_wifi_name_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_wifi_psw_span_wifi_name, 48, 10);
    lv_obj_set_size(ui->screen_wifi_psw_span_wifi_name, 85, 17);

    //Write style state: LV_STATE_DEFAULT for &style_screen_wifi_psw_span_wifi_name_main_main_default
    static lv_style_t style_screen_wifi_psw_span_wifi_name_main_main_default;
    ui_init_style(&style_screen_wifi_psw_span_wifi_name_main_main_default);

    lv_style_set_border_width(&style_screen_wifi_psw_span_wifi_name_main_main_default, 0);
    lv_style_set_radius(&style_screen_wifi_psw_span_wifi_name_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_wifi_psw_span_wifi_name_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_wifi_psw_span_wifi_name_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_wifi_psw_span_wifi_name_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_wifi_psw_span_wifi_name_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_wifi_psw_span_wifi_name_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_wifi_psw_span_wifi_name_main_main_default, 0);
    lv_obj_add_style(ui->screen_wifi_psw_span_wifi_name, &style_screen_wifi_psw_span_wifi_name_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_wifi_psw_span_wifi_name);

    //Write codes screen_wifi_psw_span_tip
    ui->screen_wifi_psw_span_tip = lv_spangroup_create(ui->screen_wifi_psw);
    lv_spangroup_set_align(ui->screen_wifi_psw_span_tip, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_wifi_psw_span_tip, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_wifi_psw_span_tip, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_wifi_psw_span_tip_span = lv_spangroup_new_span(ui->screen_wifi_psw_span_tip);
    lv_span_set_text(ui->screen_wifi_psw_span_tip_span, "点击输入框可弹出键盘");
    lv_style_set_text_color(&ui->screen_wifi_psw_span_tip_span->style, lv_color_hex(0xd7d7d7));
    lv_style_set_text_decor(&ui->screen_wifi_psw_span_tip_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_wifi_psw_span_tip_span->style, &lv_font_MiSansDemibold_14);
    lv_obj_set_pos(ui->screen_wifi_psw_span_tip, 90, 95);
    lv_obj_set_size(ui->screen_wifi_psw_span_tip, 141, 17);

    //Write style state: LV_STATE_DEFAULT for &style_screen_wifi_psw_span_tip_main_main_default
    static lv_style_t style_screen_wifi_psw_span_tip_main_main_default;
    ui_init_style(&style_screen_wifi_psw_span_tip_main_main_default);

    lv_style_set_border_width(&style_screen_wifi_psw_span_tip_main_main_default, 0);
    lv_style_set_radius(&style_screen_wifi_psw_span_tip_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_wifi_psw_span_tip_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_wifi_psw_span_tip_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_wifi_psw_span_tip_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_wifi_psw_span_tip_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_wifi_psw_span_tip_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_wifi_psw_span_tip_main_main_default, 0);
    lv_obj_add_style(ui->screen_wifi_psw_span_tip, &style_screen_wifi_psw_span_tip_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_wifi_psw_span_tip);

    //Write codes screen_wifi_psw_btn_connect
    ui->screen_wifi_psw_btn_connect = lv_btn_create(ui->screen_wifi_psw);
    ui->screen_wifi_psw_btn_connect_label = lv_label_create(ui->screen_wifi_psw_btn_connect);
    lv_label_set_text(ui->screen_wifi_psw_btn_connect_label, "连 接");
    lv_label_set_long_mode(ui->screen_wifi_psw_btn_connect_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_wifi_psw_btn_connect_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_wifi_psw_btn_connect, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_wifi_psw_btn_connect_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_wifi_psw_btn_connect, 162, 92);
    lv_obj_set_size(ui->screen_wifi_psw_btn_connect, 140, 25);
    lv_obj_add_flag(ui->screen_wifi_psw_btn_connect, LV_OBJ_FLAG_HIDDEN);

    //Write style for screen_wifi_psw_btn_connect, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_wifi_psw_btn_connect, 53, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_wifi_psw_btn_connect, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_wifi_psw_btn_connect, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_wifi_psw_btn_connect, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_wifi_psw_btn_connect, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_wifi_psw_btn_connect, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_wifi_psw_btn_connect, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_psw_btn_connect, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_wifi_psw_btn_connect, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_wifi_psw_btn_connect, lv_color_hex(0xa4a4a5), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_wifi_psw_btn_connect, &lv_font_MiSansDemibold_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_wifi_psw_btn_connect, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_wifi_psw_btn_connect, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_psw_btn_del
    ui->screen_wifi_psw_btn_del = lv_btn_create(ui->screen_wifi_psw);
    ui->screen_wifi_psw_btn_del_label = lv_label_create(ui->screen_wifi_psw_btn_del);
    lv_label_set_text(ui->screen_wifi_psw_btn_del_label, "取 消");
    lv_label_set_long_mode(ui->screen_wifi_psw_btn_del_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_wifi_psw_btn_del_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_wifi_psw_btn_del, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_wifi_psw_btn_del_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_wifi_psw_btn_del, 16, 92);
    lv_obj_set_size(ui->screen_wifi_psw_btn_del, 140, 25);
    lv_obj_add_flag(ui->screen_wifi_psw_btn_del, LV_OBJ_FLAG_HIDDEN);

    //Write style for screen_wifi_psw_btn_del, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_wifi_psw_btn_del, 53, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_wifi_psw_btn_del, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_wifi_psw_btn_del, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_wifi_psw_btn_del, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_wifi_psw_btn_del, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_wifi_psw_btn_del, lv_color_hex(0x9b9b9b), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_wifi_psw_btn_del, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_psw_btn_del, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_wifi_psw_btn_del, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_wifi_psw_btn_del, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_wifi_psw_btn_del, &lv_font_MiSansDemibold_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_wifi_psw_btn_del, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_wifi_psw_btn_del, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_psw_cont_err
    ui->screen_wifi_psw_cont_err = lv_obj_create(ui->screen_wifi_psw);
    lv_obj_set_pos(ui->screen_wifi_psw_cont_err, 336, 0);
    lv_obj_set_size(ui->screen_wifi_psw_cont_err, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_wifi_psw_cont_err, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(ui->screen_wifi_psw_cont_err, LV_OBJ_FLAG_HIDDEN);

    //Write style for screen_wifi_psw_cont_err, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_wifi_psw_cont_err, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_psw_cont_err, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_wifi_psw_cont_err, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_wifi_psw_cont_err, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_wifi_psw_cont_err, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_wifi_psw_cont_err, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_wifi_psw_cont_err, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_wifi_psw_cont_err, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_wifi_psw_cont_err, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_wifi_psw_cont_err, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_psw_img_err
    ui->screen_wifi_psw_img_err = lv_img_create(ui->screen_wifi_psw_cont_err);
    lv_obj_add_flag(ui->screen_wifi_psw_img_err, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_wifi_psw_img_err, &_wifi_err_alpha_80x80);
    lv_img_set_pivot(ui->screen_wifi_psw_img_err, 50, 50);
    lv_img_set_angle(ui->screen_wifi_psw_img_err, 0);
    lv_obj_set_pos(ui->screen_wifi_psw_img_err, 120, 35);
    lv_obj_set_size(ui->screen_wifi_psw_img_err, 80, 80);

    //Write style for screen_wifi_psw_img_err, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_wifi_psw_img_err, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_wifi_psw_img_err, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_psw_img_err, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_wifi_psw_img_err, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_psw_span_tip_err
    ui->screen_wifi_psw_span_tip_err = lv_spangroup_create(ui->screen_wifi_psw_cont_err);
    lv_spangroup_set_align(ui->screen_wifi_psw_span_tip_err, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_wifi_psw_span_tip_err, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_wifi_psw_span_tip_err, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_wifi_psw_span_tip_err_span = lv_spangroup_new_span(ui->screen_wifi_psw_span_tip_err);
    lv_span_set_text(ui->screen_wifi_psw_span_tip_err_span, "密码错误，连接失败。");
    lv_style_set_text_color(&ui->screen_wifi_psw_span_tip_err_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_wifi_psw_span_tip_err_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_wifi_psw_span_tip_err_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_wifi_psw_span_tip_err, 75, 129);
    lv_obj_set_size(ui->screen_wifi_psw_span_tip_err, 185, 17);

    //Write style state: LV_STATE_DEFAULT for &style_screen_wifi_psw_span_tip_err_main_main_default
    static lv_style_t style_screen_wifi_psw_span_tip_err_main_main_default;
    ui_init_style(&style_screen_wifi_psw_span_tip_err_main_main_default);

    lv_style_set_border_width(&style_screen_wifi_psw_span_tip_err_main_main_default, 0);
    lv_style_set_radius(&style_screen_wifi_psw_span_tip_err_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_wifi_psw_span_tip_err_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_wifi_psw_span_tip_err_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_wifi_psw_span_tip_err_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_wifi_psw_span_tip_err_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_wifi_psw_span_tip_err_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_wifi_psw_span_tip_err_main_main_default, 0);
    lv_obj_add_style(ui->screen_wifi_psw_span_tip_err, &style_screen_wifi_psw_span_tip_err_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_wifi_psw_span_tip_err);

    //Write codes screen_wifi_psw_btn_know
    ui->screen_wifi_psw_btn_know = lv_btn_create(ui->screen_wifi_psw_cont_err);
    ui->screen_wifi_psw_btn_know_label = lv_label_create(ui->screen_wifi_psw_btn_know);
    lv_label_set_text(ui->screen_wifi_psw_btn_know_label, "知道了");
    lv_label_set_long_mode(ui->screen_wifi_psw_btn_know_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_wifi_psw_btn_know_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_wifi_psw_btn_know, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_wifi_psw_btn_know_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_wifi_psw_btn_know, 45, 173);
    lv_obj_set_size(ui->screen_wifi_psw_btn_know, 233, 42);

    //Write style for screen_wifi_psw_btn_know, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_wifi_psw_btn_know, 53, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_wifi_psw_btn_know, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_wifi_psw_btn_know, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_wifi_psw_btn_know, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_wifi_psw_btn_know, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_wifi_psw_btn_know, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_wifi_psw_btn_know, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_psw_btn_know, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_wifi_psw_btn_know, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_wifi_psw_btn_know, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_wifi_psw_btn_know, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_wifi_psw_btn_know, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_wifi_psw_btn_know, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_psw_cont_connecting
    ui->screen_wifi_psw_cont_connecting = lv_obj_create(ui->screen_wifi_psw);
    lv_obj_set_pos(ui->screen_wifi_psw_cont_connecting, 336, -257);
    lv_obj_set_size(ui->screen_wifi_psw_cont_connecting, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_wifi_psw_cont_connecting, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(ui->screen_wifi_psw_cont_connecting, LV_OBJ_FLAG_HIDDEN);

    //Write style for screen_wifi_psw_cont_connecting, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_wifi_psw_cont_connecting, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_psw_cont_connecting, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_wifi_psw_cont_connecting, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_wifi_psw_cont_connecting, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_wifi_psw_cont_connecting, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_wifi_psw_cont_connecting, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_wifi_psw_cont_connecting, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_wifi_psw_cont_connecting, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_wifi_psw_cont_connecting, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_wifi_psw_cont_connecting, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_wifi_psw_spangroup_1
    ui->screen_wifi_psw_spangroup_1 = lv_spangroup_create(ui->screen_wifi_psw_cont_connecting);
    lv_spangroup_set_align(ui->screen_wifi_psw_spangroup_1, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_wifi_psw_spangroup_1, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_wifi_psw_spangroup_1, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_wifi_psw_spangroup_1_span = lv_spangroup_new_span(ui->screen_wifi_psw_spangroup_1);
    lv_span_set_text(ui->screen_wifi_psw_spangroup_1_span, "加速连接中……");
    lv_style_set_text_color(&ui->screen_wifi_psw_spangroup_1_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_wifi_psw_spangroup_1_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_wifi_psw_spangroup_1_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_wifi_psw_spangroup_1, 101, 175);
    lv_obj_set_size(ui->screen_wifi_psw_spangroup_1, 136, 17);

    //Write style state: LV_STATE_DEFAULT for &style_screen_wifi_psw_spangroup_1_main_main_default
    static lv_style_t style_screen_wifi_psw_spangroup_1_main_main_default;
    ui_init_style(&style_screen_wifi_psw_spangroup_1_main_main_default);

    lv_style_set_border_width(&style_screen_wifi_psw_spangroup_1_main_main_default, 0);
    lv_style_set_radius(&style_screen_wifi_psw_spangroup_1_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_wifi_psw_spangroup_1_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_wifi_psw_spangroup_1_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_wifi_psw_spangroup_1_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_wifi_psw_spangroup_1_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_wifi_psw_spangroup_1_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_wifi_psw_spangroup_1_main_main_default, 0);
    lv_obj_add_style(ui->screen_wifi_psw_spangroup_1, &style_screen_wifi_psw_spangroup_1_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_wifi_psw_spangroup_1);

    //Write codes screen_wifi_psw_spinner_wifi
    ui->screen_wifi_psw_spinner_wifi = lv_spinner_create(ui->screen_wifi_psw_cont_connecting, 2000, 60);
    lv_obj_set_pos(ui->screen_wifi_psw_spinner_wifi, 110, 36);
    lv_obj_set_size(ui->screen_wifi_psw_spinner_wifi, 100, 100);

    //Write style for screen_wifi_psw_spinner_wifi, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_pad_top(ui->screen_wifi_psw_spinner_wifi, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_wifi_psw_spinner_wifi, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_wifi_psw_spinner_wifi, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_wifi_psw_spinner_wifi, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_wifi_psw_spinner_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(ui->screen_wifi_psw_spinner_wifi, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui->screen_wifi_psw_spinner_wifi, 103, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(ui->screen_wifi_psw_spinner_wifi, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_wifi_psw_spinner_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style for screen_wifi_psw_spinner_wifi, Part: LV_PART_INDICATOR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_arc_width(ui->screen_wifi_psw_spinner_wifi, 12, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui->screen_wifi_psw_spinner_wifi, 217, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(ui->screen_wifi_psw_spinner_wifi, lv_color_hex(0xffffff), LV_PART_INDICATOR | LV_STATE_DEFAULT);

#if CONFIG_LVGL_WIFI_PASSWORD_IMGBTN_ENABLE
    //Write codes screen_wifi_psw_imgbtn_psw (password visibility toggle; wire events in wifi_psw.c later)
    ui->screen_wifi_psw_imgbtn_psw = lv_imgbtn_create(ui->screen_wifi_psw);
    lv_obj_add_flag(ui->screen_wifi_psw_imgbtn_psw, LV_OBJ_FLAG_CHECKABLE);
    /* No click focus: keeps ta_input focused so ta_event_cb does not hide the top-layer keyboard */
    lv_obj_clear_flag(ui->screen_wifi_psw_imgbtn_psw, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_imgbtn_set_src(ui->screen_wifi_psw_imgbtn_psw, LV_IMGBTN_STATE_RELEASED, NULL, &_close_alpha_20x20, NULL);
    lv_imgbtn_set_src(ui->screen_wifi_psw_imgbtn_psw, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &_open_alpha_20x20, NULL);
    ui->screen_wifi_psw_imgbtn_psw_label = lv_label_create(ui->screen_wifi_psw_imgbtn_psw);
    lv_label_set_text(ui->screen_wifi_psw_imgbtn_psw_label, "");
    lv_label_set_long_mode(ui->screen_wifi_psw_imgbtn_psw_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_wifi_psw_imgbtn_psw_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_wifi_psw_imgbtn_psw, 0, LV_STATE_DEFAULT);
    lv_obj_set_pos(ui->screen_wifi_psw_imgbtn_psw, 272, 55);
    lv_obj_set_size(ui->screen_wifi_psw_imgbtn_psw, 20, 20);

    lv_obj_set_style_text_color(ui->screen_wifi_psw_imgbtn_psw, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_wifi_psw_imgbtn_psw, &lv_font_MiSansDemibold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_wifi_psw_imgbtn_psw, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_wifi_psw_imgbtn_psw, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_wifi_psw_imgbtn_psw, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_wifi_psw_imgbtn_psw, true, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_wifi_psw_imgbtn_psw, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_img_recolor_opa(ui->screen_wifi_psw_imgbtn_psw, 0, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_img_opa(ui->screen_wifi_psw_imgbtn_psw, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_color(ui->screen_wifi_psw_imgbtn_psw, lv_color_hex(0xFF33FF), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_font(ui->screen_wifi_psw_imgbtn_psw, &lv_font_MiSansDemibold_12, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_opa(ui->screen_wifi_psw_imgbtn_psw, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(ui->screen_wifi_psw_imgbtn_psw, 0, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_set_style_img_recolor_opa(ui->screen_wifi_psw_imgbtn_psw, 0, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_img_opa(ui->screen_wifi_psw_imgbtn_psw, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(ui->screen_wifi_psw_imgbtn_psw, lv_color_hex(0xFF33FF), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_font(ui->screen_wifi_psw_imgbtn_psw, &lv_font_MiSansDemibold_12, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(ui->screen_wifi_psw_imgbtn_psw, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_width(ui->screen_wifi_psw_imgbtn_psw, 0, LV_PART_MAIN | LV_STATE_CHECKED);

    lv_obj_set_style_img_recolor_opa(ui->screen_wifi_psw_imgbtn_psw, 0, LV_PART_MAIN | LV_IMGBTN_STATE_RELEASED);
    lv_obj_set_style_img_opa(ui->screen_wifi_psw_imgbtn_psw, 255, LV_PART_MAIN | LV_IMGBTN_STATE_RELEASED);
#endif

    //The custom code of screen_wifi_psw.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_wifi_psw);

    //Init events for screen.
    events_init_screen_wifi_psw(ui);
}

#else

void setup_scr_screen_wifi_psw(lv_ui *ui)
{
    (void)ui;
}

#endif
