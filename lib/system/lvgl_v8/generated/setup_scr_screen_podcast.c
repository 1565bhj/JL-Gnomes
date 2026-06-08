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



void setup_scr_screen_podcast(lv_ui *ui)
{
    //Write codes screen_podcast
    ui->screen_podcast = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_podcast, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_podcast, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_podcast, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_podcast, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_podcast, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_podcast, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);


    //Write codes screen_podcast_next
    ui->screen_podcast_next = lv_img_create(ui->screen_podcast);
    lv_obj_add_flag(ui->screen_podcast_next, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_podcast_next, &_next2_alpha_50x50);
    lv_img_set_pivot(ui->screen_podcast_next, 50, 50);
    lv_img_set_angle(ui->screen_podcast_next, 0);
    lv_obj_set_pos(ui->screen_podcast_next, 240, 185);
    lv_obj_set_size(ui->screen_podcast_next, 50, 50);

    //Write style for screen_podcast_next, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_podcast_next, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_podcast_next, 217, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_podcast_next, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_podcast_next, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_podcast_last
    ui->screen_podcast_last = lv_img_create(ui->screen_podcast);
    lv_obj_add_flag(ui->screen_podcast_last, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_podcast_last, &_last2_alpha_50x50);
    lv_img_set_pivot(ui->screen_podcast_last, 50, 50);
    lv_img_set_angle(ui->screen_podcast_last, 0);
    lv_obj_set_pos(ui->screen_podcast_last, 25, 185);
    lv_obj_set_size(ui->screen_podcast_last, 50, 50);

    //Write style for screen_podcast_last, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_podcast_last, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_podcast_last, 217, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_podcast_last, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_podcast_last, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_podcast_slider
    ui->screen_podcast_slider = lv_slider_create(ui->screen_podcast);
    lv_slider_set_range(ui->screen_podcast_slider, 0, 100);
    lv_slider_set_mode(ui->screen_podcast_slider, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_value(ui->screen_podcast_slider, 0, LV_ANIM_OFF);
    lv_obj_set_pos(ui->screen_podcast_slider, 44, 168);
    lv_obj_set_size(ui->screen_podcast_slider, 231, 4);

    //Write style for screen_podcast_slider, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_podcast_slider, 80, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_podcast_slider, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_podcast_slider, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_podcast_slider, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui->screen_podcast_slider, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_podcast_slider, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style for screen_podcast_slider, Part: LV_PART_INDICATOR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_podcast_slider, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_podcast_slider, lv_color_hex(0xffffff), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_podcast_slider, LV_GRAD_DIR_NONE, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_podcast_slider, 8, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    //Write style for screen_podcast_slider, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_podcast_slider, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_podcast_slider, lv_color_hex(0xffffff), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_podcast_slider, LV_GRAD_DIR_NONE, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_podcast_slider, 8, LV_PART_KNOB | LV_STATE_DEFAULT);

    //Write codes screen_podcast_time_start
    ui->screen_podcast_time_start = lv_spangroup_create(ui->screen_podcast);
    lv_spangroup_set_align(ui->screen_podcast_time_start, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_podcast_time_start, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_podcast_time_start, LV_SPAN_MODE_BREAK);
    //create span
    ui->screen_podcast_time_start_span = lv_spangroup_new_span(ui->screen_podcast_time_start);
    lv_span_set_text(ui->screen_podcast_time_start_span, "0:00");
    lv_style_set_text_color(&ui->screen_podcast_time_start_span->style, lv_color_hex(0xbfbebe));
    lv_style_set_text_decor(&ui->screen_podcast_time_start_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_podcast_time_start_span->style, &lv_font_Barlow__12);
    lv_obj_set_pos(ui->screen_podcast_time_start, 41, 144);
    lv_obj_set_size(ui->screen_podcast_time_start, 31, 11);

    //Write style state: LV_STATE_DEFAULT for &style_screen_podcast_time_start_main_main_default
    static lv_style_t style_screen_podcast_time_start_main_main_default;
    ui_init_style(&style_screen_podcast_time_start_main_main_default);

    lv_style_set_border_width(&style_screen_podcast_time_start_main_main_default, 0);
    lv_style_set_radius(&style_screen_podcast_time_start_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_podcast_time_start_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_podcast_time_start_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_podcast_time_start_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_podcast_time_start_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_podcast_time_start_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_podcast_time_start_main_main_default, 0);
    lv_obj_add_style(ui->screen_podcast_time_start, &style_screen_podcast_time_start_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_podcast_time_start);

    //Write codes screen_podcast_time_end
    ui->screen_podcast_time_end = lv_spangroup_create(ui->screen_podcast);
    lv_spangroup_set_align(ui->screen_podcast_time_end, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_podcast_time_end, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_podcast_time_end, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_podcast_time_end_span = lv_spangroup_new_span(ui->screen_podcast_time_end);
    lv_span_set_text(ui->screen_podcast_time_end_span, "0:00");
    lv_style_set_text_color(&ui->screen_podcast_time_end_span->style, lv_color_hex(0xbfbebe));
    lv_style_set_text_decor(&ui->screen_podcast_time_end_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_podcast_time_end_span->style, &lv_font_Barlow__12);
    lv_obj_set_pos(ui->screen_podcast_time_end, 250, 144);
    lv_obj_set_size(ui->screen_podcast_time_end, 31, 11);

    //Write style state: LV_STATE_DEFAULT for &style_screen_podcast_time_end_main_main_default
    static lv_style_t style_screen_podcast_time_end_main_main_default;
    ui_init_style(&style_screen_podcast_time_end_main_main_default);

    lv_style_set_border_width(&style_screen_podcast_time_end_main_main_default, 0);
    lv_style_set_radius(&style_screen_podcast_time_end_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_podcast_time_end_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_podcast_time_end_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_podcast_time_end_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_podcast_time_end_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_podcast_time_end_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_podcast_time_end_main_main_default, 0);
    lv_obj_add_style(ui->screen_podcast_time_end, &style_screen_podcast_time_end_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_podcast_time_end);

    //Write codes screen_podcast_cont
    ui->screen_podcast_cont = lv_obj_create(ui->screen_podcast);
    lv_obj_set_pos(ui->screen_podcast_cont, -74, -33);
    lv_obj_set_size(ui->screen_podcast_cont, 462, 154);
    lv_obj_set_scrollbar_mode(ui->screen_podcast_cont, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_podcast_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_podcast_cont, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_podcast_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_podcast_cont, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_podcast_cont, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_podcast_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_podcast_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_podcast_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_podcast_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_podcast_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_podcast_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_podcast_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_podcast_img_music
    ui->screen_podcast_img_music = lv_img_create(ui->screen_podcast_cont);
    lv_obj_add_flag(ui->screen_podcast_img_music, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_podcast_img_music, &_groove_alpha_228x145);
    lv_img_set_pivot(ui->screen_podcast_img_music, 115, 70);
    lv_img_set_angle(ui->screen_podcast_img_music, 0);
    lv_obj_set_pos(ui->screen_podcast_img_music, 119, 20);
    lv_obj_set_size(ui->screen_podcast_img_music, 228, 145);

    //Write style for screen_podcast_img_music, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_podcast_img_music, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_podcast_img_music, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_podcast_img_music, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_podcast_img_music, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_podcast_img_tape_right
    ui->screen_podcast_img_tape_right = lv_img_create(ui->screen_podcast_cont);
    lv_obj_add_flag(ui->screen_podcast_img_tape_right, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_podcast_img_tape_right, &_blog_alpha_80x80);
    lv_img_set_pivot(ui->screen_podcast_img_tape_right, 50, 50);
    lv_img_set_angle(ui->screen_podcast_img_tape_right, 0);
    lv_obj_set_pos(ui->screen_podcast_img_tape_right, 358, 45);
    lv_obj_set_size(ui->screen_podcast_img_tape_right, 80, 80);

    //Write style for screen_podcast_img_tape_right, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_podcast_img_tape_right, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_podcast_img_tape_right, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_podcast_img_tape_right, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_podcast_img_tape_right, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_podcast_img_tape
    ui->screen_podcast_img_tape = lv_img_create(ui->screen_podcast_cont);
    lv_obj_add_flag(ui->screen_podcast_img_tape, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_podcast_img_tape, &_blog_alpha_100x100);
    lv_img_set_pivot(ui->screen_podcast_img_tape, 50, 50);
    lv_img_set_angle(ui->screen_podcast_img_tape, 0);
    lv_obj_set_pos(ui->screen_podcast_img_tape, 184, 40);
    lv_obj_set_size(ui->screen_podcast_img_tape, 100, 100);

    //Write style for screen_podcast_img_tape, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_podcast_img_tape, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_podcast_img_tape, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_podcast_img_tape, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_podcast_img_tape, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_podcast_img_tape_left
    ui->screen_podcast_img_tape_left = lv_img_create(ui->screen_podcast_cont);
    lv_obj_add_flag(ui->screen_podcast_img_tape_left, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_podcast_img_tape_left, &_blog_alpha_80x80);
    lv_img_set_pivot(ui->screen_podcast_img_tape_left, 50, 50);
    lv_img_set_angle(ui->screen_podcast_img_tape_left, 0);
    lv_obj_set_pos(ui->screen_podcast_img_tape_left, 25, 45);
    lv_obj_set_size(ui->screen_podcast_img_tape_left, 80, 80);

    //Write style for screen_podcast_img_tape_left, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_podcast_img_tape_left, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_podcast_img_tape_left, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_podcast_img_tape_left, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_podcast_img_tape_left, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_podcast_img_bar
    ui->screen_podcast_img_bar = lv_img_create(ui->screen_podcast_cont);
    lv_obj_add_flag(ui->screen_podcast_img_bar, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_podcast_img_bar, &_tape_bar_alpha_47x98);
    lv_img_set_pivot(ui->screen_podcast_img_bar, 5, 7);
    lv_img_set_angle(ui->screen_podcast_img_bar, -80);
    lv_obj_set_pos(ui->screen_podcast_img_bar, 161, 38);
    lv_obj_set_size(ui->screen_podcast_img_bar, 47, 98);

    //Write style for screen_podcast_img_bar, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_podcast_img_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_podcast_img_bar, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_podcast_img_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_podcast_img_bar, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_podcast_podcast_title
    ui->screen_podcast_podcast_title = lv_label_create(ui->screen_podcast);
    lv_label_set_text(ui->screen_podcast_podcast_title, "轻语播客");
    lv_label_set_long_mode(ui->screen_podcast_podcast_title, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_anim_speed(ui->screen_podcast_podcast_title, 10, LV_PART_MAIN);
    lv_obj_set_pos(ui->screen_podcast_podcast_title, 28, 121);
    lv_obj_set_size(ui->screen_podcast_podcast_title, 265, 21);

    //Write style for screen_podcast_podcast_title, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_podcast_podcast_title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_podcast_podcast_title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_podcast_podcast_title, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_podcast_podcast_title, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_podcast_podcast_title, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_podcast_podcast_title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_podcast_podcast_title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_podcast_podcast_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_podcast_podcast_title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_podcast_podcast_title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_podcast_podcast_title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_podcast_podcast_title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_podcast_podcast_title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_podcast_podcast_title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_podcast_btn_play_pause
    ui->screen_podcast_btn_play_pause = lv_btn_create(ui->screen_podcast);
    ui->screen_podcast_btn_play_pause_label = lv_label_create(ui->screen_podcast_btn_play_pause);
    lv_label_set_text(ui->screen_podcast_btn_play_pause_label, "  " LV_SYMBOL_PLAY " ");
    lv_label_set_long_mode(ui->screen_podcast_btn_play_pause_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_podcast_btn_play_pause_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_podcast_btn_play_pause, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_podcast_btn_play_pause_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_podcast_btn_play_pause, 110, 188);
    lv_obj_set_size(ui->screen_podcast_btn_play_pause, 100, 39);

    //Write style for screen_podcast_btn_play_pause, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_podcast_btn_play_pause, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_podcast_btn_play_pause, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_podcast_btn_play_pause, 21, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_podcast_btn_play_pause, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui->screen_podcast_btn_play_pause, lv_color_hex(0xfcc698), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui->screen_podcast_btn_play_pause, 79, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui->screen_podcast_btn_play_pause, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(ui->screen_podcast_btn_play_pause, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(ui->screen_podcast_btn_play_pause, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->screen_podcast_btn_play_pause, &_bg2_100x39, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->screen_podcast_btn_play_pause, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->screen_podcast_btn_play_pause, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_podcast_btn_play_pause, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_podcast_btn_play_pause, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_podcast_btn_play_pause, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_podcast_btn_play_pause, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //The custom code of screen_podcast.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_podcast);

    //Init events for screen.
    events_init_screen_podcast(ui);
}
