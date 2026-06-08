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



void setup_scr_screen_whack_game(lv_ui *ui)
{
    //Write codes screen_whack_game
    ui->screen_whack_game = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_whack_game, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_whack_game, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_whack_game, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_whack_game, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_whack_game, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_whack_game, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->screen_whack_game, CONFIG_UI_RES_FILE_PATH"mouse_bg.bin", LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->screen_whack_game, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->screen_whack_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_whack_game_img_countdown
    ui->screen_whack_game_img_countdown = lv_img_create(ui->screen_whack_game);
    lv_obj_add_flag(ui->screen_whack_game_img_countdown, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_whack_game_img_countdown, CONFIG_UI_RES_FILE_PATH"countdown.bin");
    lv_img_set_pivot(ui->screen_whack_game_img_countdown, 50, 50);
    lv_img_set_angle(ui->screen_whack_game_img_countdown, 0);
    lv_obj_set_pos(ui->screen_whack_game_img_countdown, 98, 6);
    lv_obj_set_size(ui->screen_whack_game_img_countdown, 124, 53);

    //Write style for screen_whack_game_img_countdown, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_whack_game_img_countdown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_whack_game_img_countdown, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_whack_game_img_countdown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_whack_game_img_countdown, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_whack_game_cave1
    ui->screen_whack_game_cave1 = lv_img_create(ui->screen_whack_game);
    lv_obj_add_flag(ui->screen_whack_game_cave1, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_whack_game_cave1, CONFIG_UI_RES_FILE_PATH"cave.bin");
    lv_img_set_pivot(ui->screen_whack_game_cave1, 50, 50);
    lv_img_set_angle(ui->screen_whack_game_cave1, 0);
    lv_obj_set_pos(ui->screen_whack_game_cave1, 24, 79);
    lv_obj_set_size(ui->screen_whack_game_cave1, 71, 63);

    //Write style for screen_whack_game_cave1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_whack_game_cave1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_whack_game_cave1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_whack_game_cave1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_whack_game_cave1, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_whack_game_cave3
    ui->screen_whack_game_cave3 = lv_img_create(ui->screen_whack_game);
    lv_obj_add_flag(ui->screen_whack_game_cave3, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_whack_game_cave3, CONFIG_UI_RES_FILE_PATH"cave.bin");
    lv_img_set_pivot(ui->screen_whack_game_cave3, 50, 50);
    lv_img_set_angle(ui->screen_whack_game_cave3, 0);
    lv_obj_set_pos(ui->screen_whack_game_cave3, 95, 124);
    lv_obj_set_size(ui->screen_whack_game_cave3, 71, 63);

    //Write style for screen_whack_game_cave3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_whack_game_cave3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_whack_game_cave3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_whack_game_cave3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_whack_game_cave3, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_whack_game_cave5
    ui->screen_whack_game_cave5 = lv_img_create(ui->screen_whack_game);
    lv_obj_add_flag(ui->screen_whack_game_cave5, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_whack_game_cave5, CONFIG_UI_RES_FILE_PATH"cave.bin");
    lv_img_set_pivot(ui->screen_whack_game_cave5, 50, 50);
    lv_img_set_angle(ui->screen_whack_game_cave5, 0);
    lv_obj_set_pos(ui->screen_whack_game_cave5, 166, 174);
    lv_obj_set_size(ui->screen_whack_game_cave5, 71, 63);

    //Write style for screen_whack_game_cave5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_whack_game_cave5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_whack_game_cave5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_whack_game_cave5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_whack_game_cave5, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_whack_game_cave4
    ui->screen_whack_game_cave4 = lv_img_create(ui->screen_whack_game);
    lv_obj_add_flag(ui->screen_whack_game_cave4, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_whack_game_cave4, CONFIG_UI_RES_FILE_PATH"cave.bin");
    lv_img_set_pivot(ui->screen_whack_game_cave4, 50, 50);
    lv_img_set_angle(ui->screen_whack_game_cave4, 0);
    lv_obj_set_pos(ui->screen_whack_game_cave4, 169, 69);
    lv_obj_set_size(ui->screen_whack_game_cave4, 71, 63);

    //Write style for screen_whack_game_cave4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_whack_game_cave4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_whack_game_cave4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_whack_game_cave4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_whack_game_cave4, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_whack_game_cave6
    ui->screen_whack_game_cave6 = lv_img_create(ui->screen_whack_game);
    lv_obj_add_flag(ui->screen_whack_game_cave6, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_whack_game_cave6, CONFIG_UI_RES_FILE_PATH"cave.bin");
    lv_img_set_pivot(ui->screen_whack_game_cave6, 50, 50);
    lv_img_set_angle(ui->screen_whack_game_cave6, 0);
    lv_obj_set_pos(ui->screen_whack_game_cave6, 241, 124);
    lv_obj_set_size(ui->screen_whack_game_cave6, 71, 63);

    //Write style for screen_whack_game_cave6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_whack_game_cave6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_whack_game_cave6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_whack_game_cave6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_whack_game_cave6, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_whack_game_cave2
    ui->screen_whack_game_cave2 = lv_img_create(ui->screen_whack_game);
    lv_obj_add_flag(ui->screen_whack_game_cave2, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_whack_game_cave2, CONFIG_UI_RES_FILE_PATH"cave.bin");
    lv_img_set_pivot(ui->screen_whack_game_cave2, 50, 50);
    lv_img_set_angle(ui->screen_whack_game_cave2, 0);
    lv_obj_set_pos(ui->screen_whack_game_cave2, 13, 168);
    lv_obj_set_size(ui->screen_whack_game_cave2, 71, 63);

    //Write style for screen_whack_game_cave2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_whack_game_cave2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_whack_game_cave2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_whack_game_cave2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_whack_game_cave2, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_whack_game_mouse
    ui->screen_whack_game_mouse = lv_img_create(ui->screen_whack_game);
    lv_obj_add_flag(ui->screen_whack_game_mouse, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_whack_game_mouse, CONFIG_UI_RES_FILE_PATH"mouse1.bin");
    lv_img_set_pivot(ui->screen_whack_game_mouse, 50, 50);
    lv_img_set_angle(ui->screen_whack_game_mouse, 0);
    lv_obj_set_pos(ui->screen_whack_game_mouse, -238, 211);
    lv_obj_set_size(ui->screen_whack_game_mouse, 56, 58);

    //Write style for screen_whack_game_mouse, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_whack_game_mouse, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_whack_game_mouse, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_whack_game_mouse, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_whack_game_mouse, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_whack_game_mouse2
    ui->screen_whack_game_mouse2 = lv_img_create(ui->screen_whack_game);
    lv_obj_add_flag(ui->screen_whack_game_mouse2, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_whack_game_mouse2, CONFIG_UI_RES_FILE_PATH"mouse2.bin");
    lv_img_set_pivot(ui->screen_whack_game_mouse2, 50, 50);
    lv_img_set_angle(ui->screen_whack_game_mouse2, 0);
    lv_obj_set_pos(ui->screen_whack_game_mouse2, -320, 208);
    lv_obj_set_size(ui->screen_whack_game_mouse2, 56, 58);

    //Write style for screen_whack_game_mouse2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_whack_game_mouse2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_whack_game_mouse2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_whack_game_mouse2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_whack_game_mouse2, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_whack_game_black_bg
    ui->screen_whack_game_black_bg = lv_img_create(ui->screen_whack_game);
    lv_obj_add_flag(ui->screen_whack_game_black_bg, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_whack_game_black_bg, CONFIG_UI_RES_FILE_PATH"black_bg.bin");
    lv_img_set_pivot(ui->screen_whack_game_black_bg, 50, 50);
    lv_img_set_angle(ui->screen_whack_game_black_bg, 0);
    lv_obj_set_pos(ui->screen_whack_game_black_bg, 505, 85);
    lv_obj_set_size(ui->screen_whack_game_black_bg, 320, 240);

    //Write style for screen_whack_game_black_bg, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_whack_game_black_bg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_whack_game_black_bg, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_whack_game_black_bg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_whack_game_black_bg, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_whack_game_btn_again
    ui->screen_whack_game_btn_again = lv_btn_create(ui->screen_whack_game);
    ui->screen_whack_game_btn_again_label = lv_label_create(ui->screen_whack_game_btn_again);
    lv_label_set_text(ui->screen_whack_game_btn_again_label, "");
    lv_label_set_long_mode(ui->screen_whack_game_btn_again_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_whack_game_btn_again_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_whack_game_btn_again, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_whack_game_btn_again_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_whack_game_btn_again, 585, 227);
    lv_obj_set_size(ui->screen_whack_game_btn_again, 168, 61);

    //Write style for screen_whack_game_btn_again, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_whack_game_btn_again, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_whack_game_btn_again, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_whack_game_btn_again, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_whack_game_btn_again, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->screen_whack_game_btn_again, CONFIG_UI_RES_FILE_PATH"win_again.bin", LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->screen_whack_game_btn_again, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->screen_whack_game_btn_again, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_whack_game_btn_again, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_whack_game_btn_again, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_whack_game_btn_again, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_whack_game_btn_again, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_whack_game_img_score
    ui->screen_whack_game_img_score = lv_img_create(ui->screen_whack_game);
    lv_obj_add_flag(ui->screen_whack_game_img_score, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_whack_game_img_score, CONFIG_UI_RES_FILE_PATH"m_score.bin");
    lv_img_set_pivot(ui->screen_whack_game_img_score, 50, 50);
    lv_img_set_angle(ui->screen_whack_game_img_score, 0);
    lv_obj_set_pos(ui->screen_whack_game_img_score, 563, 110);
    lv_obj_set_size(ui->screen_whack_game_img_score, 59, 71);

    //Write style for screen_whack_game_img_score, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_whack_game_img_score, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_whack_game_img_score, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_whack_game_img_score, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_whack_game_img_score, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_whack_game_score_num
    ui->screen_whack_game_score_num = lv_spangroup_create(ui->screen_whack_game);
    lv_spangroup_set_align(ui->screen_whack_game_score_num, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_whack_game_score_num, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_whack_game_score_num, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_whack_game_score_num_span = lv_spangroup_new_span(ui->screen_whack_game_score_num);
    lv_span_set_text(ui->screen_whack_game_score_num_span, "得分:32");
    lv_style_set_text_color(&ui->screen_whack_game_score_num_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_whack_game_score_num_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_whack_game_score_num_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_whack_game_score_num, 632, 143);
    lv_obj_set_size(ui->screen_whack_game_score_num, 96, 21);

    //Write style state: LV_STATE_DEFAULT for &style_screen_whack_game_score_num_main_main_default
    static lv_style_t style_screen_whack_game_score_num_main_main_default;
    ui_init_style(&style_screen_whack_game_score_num_main_main_default);

    lv_style_set_border_width(&style_screen_whack_game_score_num_main_main_default, 0);
    lv_style_set_radius(&style_screen_whack_game_score_num_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_whack_game_score_num_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_whack_game_score_num_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_whack_game_score_num_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_whack_game_score_num_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_whack_game_score_num_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_whack_game_score_num_main_main_default, 0);
    lv_obj_add_style(ui->screen_whack_game_score_num, &style_screen_whack_game_score_num_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_whack_game_score_num);

    //Write codes screen_whack_game_time
    ui->screen_whack_game_time = lv_spangroup_create(ui->screen_whack_game);
    lv_spangroup_set_align(ui->screen_whack_game_time, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_whack_game_time, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_whack_game_time, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_whack_game_time_span = lv_spangroup_new_span(ui->screen_whack_game_time);
    lv_span_set_text(ui->screen_whack_game_time_span, "30");
    lv_style_set_text_color(&ui->screen_whack_game_time_span->style, lv_color_hex(0xE87F53));
    lv_style_set_text_decor(&ui->screen_whack_game_time_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_whack_game_time_span->style, &lv_font_Barlow__18);
    lv_obj_set_pos(ui->screen_whack_game_time, 176, 24);
    lv_obj_set_size(ui->screen_whack_game_time, 26, 19);

    //Write style state: LV_STATE_DEFAULT for &style_screen_whack_game_time_main_main_default
    static lv_style_t style_screen_whack_game_time_main_main_default;
    ui_init_style(&style_screen_whack_game_time_main_main_default);

    lv_style_set_border_width(&style_screen_whack_game_time_main_main_default, 0);
    lv_style_set_radius(&style_screen_whack_game_time_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_whack_game_time_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_whack_game_time_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_whack_game_time_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_whack_game_time_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_whack_game_time_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_whack_game_time_main_main_default, 0);
    lv_obj_add_style(ui->screen_whack_game_time, &style_screen_whack_game_time_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_whack_game_time);
    //Write codes screen_whack_game_img_enter_bg
    ui->screen_whack_game_img_enter_bg = lv_img_create(ui->screen_whack_game);
    lv_obj_add_flag(ui->screen_whack_game_img_enter_bg, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_whack_game_img_enter_bg, CONFIG_UI_RES_FILE_PATH"en_m_bg.bin");
    lv_img_set_pivot(ui->screen_whack_game_img_enter_bg, 50, 50);
    lv_img_set_angle(ui->screen_whack_game_img_enter_bg, 0);
    lv_obj_set_pos(ui->screen_whack_game_img_enter_bg, -425, -147);
    lv_obj_set_size(ui->screen_whack_game_img_enter_bg, 320, 240);

    //Write style for screen_whack_game_img_enter_bg, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_whack_game_img_enter_bg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_whack_game_img_enter_bg, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_whack_game_img_enter_bg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_whack_game_img_enter_bg, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_whack_game_btn_enter_game
    ui->screen_whack_game_btn_enter_game = lv_btn_create(ui->screen_whack_game);
    ui->screen_whack_game_btn_enter_game_label = lv_label_create(ui->screen_whack_game_btn_enter_game);
    lv_label_set_text(ui->screen_whack_game_btn_enter_game_label, "");
    lv_label_set_long_mode(ui->screen_whack_game_btn_enter_game_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_whack_game_btn_enter_game_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_whack_game_btn_enter_game, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_whack_game_btn_enter_game_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_whack_game_btn_enter_game, -347, 6);
    lv_obj_set_size(ui->screen_whack_game_btn_enter_game, 168, 61);

    //Write style for screen_whack_game_btn_enter_game, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_whack_game_btn_enter_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_whack_game_btn_enter_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_whack_game_btn_enter_game, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_whack_game_btn_enter_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->screen_whack_game_btn_enter_game, CONFIG_UI_RES_FILE_PATH"en_game.bin", LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->screen_whack_game_btn_enter_game, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->screen_whack_game_btn_enter_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_whack_game_btn_enter_game, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_whack_game_btn_enter_game, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_whack_game_btn_enter_game, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_whack_game_btn_enter_game, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //The custom code of screen_whack_game.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_whack_game);

    //Init events for screen.
    events_init_screen_whack_game(ui);
}
