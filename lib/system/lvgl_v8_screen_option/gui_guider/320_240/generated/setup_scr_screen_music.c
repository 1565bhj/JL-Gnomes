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



void setup_scr_screen_music(lv_ui *ui)
{
    //Write codes screen_music
    ui->screen_music = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_music, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_music, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_music, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_music, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_music, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_music, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_music_img_tape
    ui->screen_music_img_tape = lv_img_create(ui->screen_music);
    lv_obj_add_flag(ui->screen_music_img_tape, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_music_img_tape, &_tape_alpha_62x62);
    lv_img_set_pivot(ui->screen_music_img_tape, 31, 31);
    lv_img_set_angle(ui->screen_music_img_tape, 0);
    lv_obj_set_pos(ui->screen_music_img_tape, 41, 28);
    lv_obj_set_size(ui->screen_music_img_tape, 62, 62);

    //Write style for screen_music_img_tape, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_music_img_tape, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_music_img_tape, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_music_img_tape, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_music_img_tape, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_music_img_bar
    ui->screen_music_img_bar = lv_img_create(ui->screen_music);
    lv_obj_add_flag(ui->screen_music_img_bar, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_music_img_bar, &_tape_bar_alpha_35x68);
    lv_img_set_pivot(ui->screen_music_img_bar, 5, 5);
    lv_img_set_angle(ui->screen_music_img_bar, 220);
    lv_obj_set_pos(ui->screen_music_img_bar, 31, 25);
    lv_obj_set_size(ui->screen_music_img_bar, 35, 68);

    //Write style for screen_music_img_bar, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_music_img_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_music_img_bar, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_music_img_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_music_img_bar, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_music_img_volume
    ui->screen_music_img_volume = lv_img_create(ui->screen_music);
    lv_obj_add_flag(ui->screen_music_img_volume, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_music_img_volume, &_volume_alpha_20x20);
    lv_img_set_pivot(ui->screen_music_img_volume, 50, 50);
    lv_img_set_angle(ui->screen_music_img_volume, 0);
    lv_obj_set_pos(ui->screen_music_img_volume, 250, 193);
    lv_obj_set_size(ui->screen_music_img_volume, 20, 20);

    //Write style for screen_music_img_volume, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_music_img_volume, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_music_img_volume, 205, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_music_img_volume, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_music_img_volume, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_music_img_next
    ui->screen_music_img_next = lv_img_create(ui->screen_music);
    lv_obj_add_flag(ui->screen_music_img_next, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_music_img_next, &_next_alpha_14x14);
    lv_img_set_pivot(ui->screen_music_img_next, 50, 50);
    lv_img_set_angle(ui->screen_music_img_next, 0);
    lv_obj_set_pos(ui->screen_music_img_next, 206, 196);
    lv_obj_set_size(ui->screen_music_img_next, 14, 14);

    //Write style for screen_music_img_next, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_music_img_next, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_music_img_next, 217, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_music_img_next, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_music_img_next, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_music_img_prev
    ui->screen_music_img_prev = lv_img_create(ui->screen_music);
    lv_obj_add_flag(ui->screen_music_img_prev, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_music_img_prev, &_last_alpha_14x14);
    lv_img_set_pivot(ui->screen_music_img_prev, 50, 50);
    lv_img_set_angle(ui->screen_music_img_prev, 0);
    lv_obj_set_pos(ui->screen_music_img_prev, 97, 196);
    lv_obj_set_size(ui->screen_music_img_prev, 14, 14);

    //Write style for screen_music_img_prev, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_music_img_prev, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_music_img_prev, 217, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_music_img_prev, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_music_img_prev, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_music_slider
    ui->screen_music_slider = lv_slider_create(ui->screen_music);
    lv_slider_set_range(ui->screen_music_slider, 0, 100);
    lv_slider_set_mode(ui->screen_music_slider, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_value(ui->screen_music_slider, 0, LV_ANIM_OFF);
    lv_obj_set_pos(ui->screen_music_slider, 45, 164);
    lv_obj_set_size(ui->screen_music_slider, 231, 4);

    //Write style for screen_music_slider, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_music_slider, 80, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_music_slider, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_music_slider, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_music_slider, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui->screen_music_slider, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_music_slider, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style for screen_music_slider, Part: LV_PART_INDICATOR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_music_slider, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_music_slider, lv_color_hex(0xffffff), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_music_slider, LV_GRAD_DIR_NONE, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_music_slider, 8, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    //Write style for screen_music_slider, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_music_slider, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_music_slider, lv_color_hex(0xffffff), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_music_slider, LV_GRAD_DIR_NONE, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_music_slider, 8, LV_PART_KNOB | LV_STATE_DEFAULT);

    //Write codes screen_music_span_singer
    ui->screen_music_span_singer = lv_spangroup_create(ui->screen_music);
    lv_spangroup_set_align(ui->screen_music_span_singer, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_music_span_singer, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_music_span_singer, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_music_span_singer_span = lv_spangroup_new_span(ui->screen_music_span_singer);
    lv_span_set_text(ui->screen_music_span_singer_span, "歌手");
    lv_style_set_text_color(&ui->screen_music_span_singer_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_music_span_singer_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_music_span_singer_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_music_span_singer, 120, 62);
    lv_obj_set_size(ui->screen_music_span_singer, 110, 22);

    //Write style state: LV_STATE_DEFAULT for &style_screen_music_span_singer_main_main_default
    static lv_style_t style_screen_music_span_singer_main_main_default;
    ui_init_style(&style_screen_music_span_singer_main_main_default);

    lv_style_set_border_width(&style_screen_music_span_singer_main_main_default, 0);
    lv_style_set_radius(&style_screen_music_span_singer_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_music_span_singer_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_music_span_singer_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_music_span_singer_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_music_span_singer_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_music_span_singer_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_music_span_singer_main_main_default, 0);
    lv_obj_add_style(ui->screen_music_span_singer, &style_screen_music_span_singer_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_music_span_singer);

    //Write codes screen_music_span_title
    ui->screen_music_span_title = lv_spangroup_create(ui->screen_music);
    lv_spangroup_set_align(ui->screen_music_span_title, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_music_span_title, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_music_span_title, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_music_span_title_span = lv_spangroup_new_span(ui->screen_music_span_title);
    lv_span_set_text(ui->screen_music_span_title_span, "歌名歌名歌名");
    lv_style_set_text_color(&ui->screen_music_span_title_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_music_span_title_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_music_span_title_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_music_span_title, 120, 31);
    lv_obj_set_size(ui->screen_music_span_title, 170, 22);

    //Write style state: LV_STATE_DEFAULT for &style_screen_music_span_title_main_main_default
    static lv_style_t style_screen_music_span_title_main_main_default;
    ui_init_style(&style_screen_music_span_title_main_main_default);

    lv_style_set_border_width(&style_screen_music_span_title_main_main_default, 0);
    lv_style_set_radius(&style_screen_music_span_title_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_music_span_title_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_music_span_title_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_music_span_title_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_music_span_title_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_music_span_title_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_music_span_title_main_main_default, 0);
    lv_obj_add_style(ui->screen_music_span_title, &style_screen_music_span_title_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_music_span_title);

    //Write codes screen_music_lyric
    ui->screen_music_lyric = lv_label_create(ui->screen_music);
    lv_label_set_text(ui->screen_music_lyric, "歌词加载中加载中加载中歌词加载中加载中加载中");
    lv_label_set_long_mode(ui->screen_music_lyric, LV_LABEL_LONG_DOT);
    lv_obj_set_pos(ui->screen_music_lyric, 41, 111);
    lv_obj_set_size(ui->screen_music_lyric, 230, 18);

    //Write style for screen_music_lyric, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_music_lyric, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_music_lyric, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_music_lyric, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_music_lyric, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_music_lyric, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_music_lyric, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_music_lyric, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_music_lyric, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_music_lyric, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_music_lyric, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_music_lyric, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_music_lyric, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_music_lyric, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_music_lyric, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_music_span_start
    ui->screen_music_span_start = lv_spangroup_create(ui->screen_music);
    lv_spangroup_set_align(ui->screen_music_span_start, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_music_span_start, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_music_span_start, LV_SPAN_MODE_BREAK);
    //create span
    ui->screen_music_span_start_span = lv_spangroup_new_span(ui->screen_music_span_start);
    lv_span_set_text(ui->screen_music_span_start_span, "0:00");
    lv_style_set_text_color(&ui->screen_music_span_start_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_music_span_start_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_music_span_start_span->style, &lv_font_Barlow__12);
    lv_obj_set_pos(ui->screen_music_span_start, 40, 139);
    lv_obj_set_size(ui->screen_music_span_start, 31, 11);

    //Write style state: LV_STATE_DEFAULT for &style_screen_music_span_start_main_main_default
    static lv_style_t style_screen_music_span_start_main_main_default;
    ui_init_style(&style_screen_music_span_start_main_main_default);

    lv_style_set_border_width(&style_screen_music_span_start_main_main_default, 0);
    lv_style_set_radius(&style_screen_music_span_start_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_music_span_start_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_music_span_start_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_music_span_start_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_music_span_start_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_music_span_start_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_music_span_start_main_main_default, 0);
    lv_obj_add_style(ui->screen_music_span_start, &style_screen_music_span_start_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_music_span_start);

    //Write codes screen_music_span_end
    ui->screen_music_span_end = lv_spangroup_create(ui->screen_music);
    lv_spangroup_set_align(ui->screen_music_span_end, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_music_span_end, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_music_span_end, LV_SPAN_MODE_BREAK);
    //create span
    ui->screen_music_span_end_span = lv_spangroup_new_span(ui->screen_music_span_end);
    lv_span_set_text(ui->screen_music_span_end_span, "0:00");
    lv_style_set_text_color(&ui->screen_music_span_end_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_music_span_end_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_music_span_end_span->style, &lv_font_Barlow__12);
    lv_obj_set_pos(ui->screen_music_span_end, 250, 139);
    lv_obj_set_size(ui->screen_music_span_end, 31, 11);

    //Write style state: LV_STATE_DEFAULT for &style_screen_music_span_end_main_main_default
    static lv_style_t style_screen_music_span_end_main_main_default;
    ui_init_style(&style_screen_music_span_end_main_main_default);

    lv_style_set_border_width(&style_screen_music_span_end_main_main_default, 0);
    lv_style_set_radius(&style_screen_music_span_end_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_music_span_end_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_music_span_end_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_music_span_end_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_music_span_end_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_music_span_end_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_music_span_end_main_main_default, 0);
    lv_obj_add_style(ui->screen_music_span_end, &style_screen_music_span_end_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_music_span_end);

    //Write codes screen_music_bar_volume
    ui->screen_music_bar_volume = lv_bar_create(ui->screen_music);
    lv_obj_set_style_anim_time(ui->screen_music_bar_volume, 1000, 0);
    lv_bar_set_mode(ui->screen_music_bar_volume, LV_BAR_MODE_NORMAL);
    lv_bar_set_range(ui->screen_music_bar_volume, 0, 100);
    lv_bar_set_value(ui->screen_music_bar_volume, 10, LV_ANIM_OFF);
    lv_obj_set_pos(ui->screen_music_bar_volume, 256, 81);
    lv_obj_set_size(ui->screen_music_bar_volume, 11, 103);
    lv_obj_add_flag(ui->screen_music_bar_volume, LV_OBJ_FLAG_HIDDEN);

    //Write style for screen_music_bar_volume, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_music_bar_volume, 60, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_music_bar_volume, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_music_bar_volume, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_music_bar_volume, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_music_bar_volume, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style for screen_music_bar_volume, Part: LV_PART_INDICATOR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_music_bar_volume, 143, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_music_bar_volume, lv_color_hex(0xffffff), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_music_bar_volume, LV_GRAD_DIR_NONE, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_music_bar_volume, 10, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    //Write codes screen_music_btn_play_pause
    ui->screen_music_btn_play_pause = lv_btn_create(ui->screen_music);
    ui->screen_music_btn_play_pause_label = lv_label_create(ui->screen_music_btn_play_pause);
    lv_label_set_text(ui->screen_music_btn_play_pause_label, "   " LV_SYMBOL_PLAY " ");
    lv_label_set_long_mode(ui->screen_music_btn_play_pause_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_music_btn_play_pause_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_music_btn_play_pause, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_music_btn_play_pause_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_music_btn_play_pause, 141, 184);
    lv_obj_set_size(ui->screen_music_btn_play_pause, 35, 35);

    //Write style for screen_music_btn_play_pause, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_music_btn_play_pause, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_music_btn_play_pause, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_music_btn_play_pause, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_music_btn_play_pause, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_music_btn_play_pause, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_music_btn_play_pause, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_music_btn_play_pause, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_music_btn_play_pause, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_music_btn_play_pause, &lv_font_Barlow__11, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_music_btn_play_pause, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_music_btn_play_pause, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_music_imgbtn_mode
    ui->screen_music_imgbtn_mode = lv_imgbtn_create(ui->screen_music);
    lv_obj_add_flag(ui->screen_music_imgbtn_mode, LV_OBJ_FLAG_CHECKABLE);
    lv_imgbtn_set_src(ui->screen_music_imgbtn_mode, LV_IMGBTN_STATE_RELEASED, NULL, &_play_random_alpha_20x20, NULL);
    lv_imgbtn_set_src(ui->screen_music_imgbtn_mode, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &_play_repeat_alpha_20x20, NULL);
    ui->screen_music_imgbtn_mode_label = lv_label_create(ui->screen_music_imgbtn_mode);
    lv_label_set_text(ui->screen_music_imgbtn_mode_label, "");
    lv_label_set_long_mode(ui->screen_music_imgbtn_mode_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_music_imgbtn_mode_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_music_imgbtn_mode, 0, LV_STATE_DEFAULT);
    lv_obj_set_pos(ui->screen_music_imgbtn_mode, 47, 192);
    lv_obj_set_size(ui->screen_music_imgbtn_mode, 20, 20);

    //Write style for screen_music_imgbtn_mode, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_text_color(ui->screen_music_imgbtn_mode, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_music_imgbtn_mode, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_music_imgbtn_mode, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_music_imgbtn_mode, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_music_imgbtn_mode, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_music_imgbtn_mode, true, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_music_imgbtn_mode, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style for screen_music_imgbtn_mode, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
    lv_obj_set_style_img_recolor_opa(ui->screen_music_imgbtn_mode, 0, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_img_opa(ui->screen_music_imgbtn_mode, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_color(ui->screen_music_imgbtn_mode, lv_color_hex(0xFF33FF), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_font(ui->screen_music_imgbtn_mode, &lv_font_montserratMedium_12, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_opa(ui->screen_music_imgbtn_mode, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(ui->screen_music_imgbtn_mode, 0, LV_PART_MAIN | LV_STATE_PRESSED);

    //Write style for screen_music_imgbtn_mode, Part: LV_PART_MAIN, State: LV_STATE_CHECKED.
    lv_obj_set_style_img_recolor_opa(ui->screen_music_imgbtn_mode, 0, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_img_opa(ui->screen_music_imgbtn_mode, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(ui->screen_music_imgbtn_mode, lv_color_hex(0xFF33FF), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_font(ui->screen_music_imgbtn_mode, &lv_font_montserratMedium_12, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(ui->screen_music_imgbtn_mode, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_width(ui->screen_music_imgbtn_mode, 0, LV_PART_MAIN | LV_STATE_CHECKED);

    //Write style for screen_music_imgbtn_mode, Part: LV_PART_MAIN, State: LV_IMGBTN_STATE_RELEASED.
    lv_obj_set_style_img_recolor_opa(ui->screen_music_imgbtn_mode, 0, LV_PART_MAIN | LV_IMGBTN_STATE_RELEASED);
    lv_obj_set_style_img_opa(ui->screen_music_imgbtn_mode, 255, LV_PART_MAIN | LV_IMGBTN_STATE_RELEASED);

    //Write codes screen_music_label_1
    ui->screen_music_label_1 = lv_label_create(ui->screen_music);
    lv_label_set_text(ui->screen_music_label_1, "Label");
    lv_label_set_long_mode(ui->screen_music_label_1, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_music_label_1, 31, 277);
    lv_obj_set_size(ui->screen_music_label_1, 100, 32);
    lv_obj_add_flag(ui->screen_music_label_1, LV_OBJ_FLAG_HIDDEN);

    //Write style for screen_music_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_music_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_music_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_music_label_1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_music_label_1, &lv_font_Barlow__15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_music_label_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_music_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_music_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_music_label_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_music_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_music_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_music_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_music_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_music_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_music_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //The custom code of screen_music.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_music);

    //Init events for screen.
    events_init_screen_music(ui);
}
