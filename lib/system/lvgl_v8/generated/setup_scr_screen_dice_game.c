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
#include "app_config.h"

void setup_scr_screen_dice_game(lv_ui *ui)
{
    //Write codes screen_dice_game
    ui->screen_dice_game = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_dice_game, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_dice_game, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_dice_game, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_dice_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->screen_dice_game, CONFIG_UI_RES_FILE_PATH"dice_bg.bin", LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->screen_dice_game, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->screen_dice_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dice_game_img_enter_game
    ui->screen_dice_game_img_enter_game = lv_img_create(ui->screen_dice_game);
    lv_obj_add_flag(ui->screen_dice_game_img_enter_game, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dice_game_img_enter_game, CONFIG_UI_RES_FILE_PATH"en_d_bg.bin");
    lv_img_set_pivot(ui->screen_dice_game_img_enter_game, 50, 50);
    lv_img_set_angle(ui->screen_dice_game_img_enter_game, 0);
    lv_obj_set_pos(ui->screen_dice_game_img_enter_game, -407, 21);
    lv_obj_set_size(ui->screen_dice_game_img_enter_game, 320, 240);

    //Write style for screen_dice_game_img_enter_game, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dice_game_img_enter_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dice_game_img_enter_game, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dice_game_img_enter_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dice_game_img_enter_game, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dice_game_btn_enter_game
    ui->screen_dice_game_btn_enter_game = lv_btn_create(ui->screen_dice_game);
    ui->screen_dice_game_btn_enter_game_label = lv_label_create(ui->screen_dice_game_btn_enter_game);
    lv_label_set_text(ui->screen_dice_game_btn_enter_game_label, "");
    lv_label_set_long_mode(ui->screen_dice_game_btn_enter_game_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_dice_game_btn_enter_game_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_dice_game_btn_enter_game, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_dice_game_btn_enter_game_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_dice_game_btn_enter_game, -328, 183);
    lv_obj_set_size(ui->screen_dice_game_btn_enter_game, 168, 61);

    //Write style for screen_dice_game_btn_enter_game, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_dice_game_btn_enter_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_dice_game_btn_enter_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dice_game_btn_enter_game, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_dice_game_btn_enter_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->screen_dice_game_btn_enter_game, CONFIG_UI_RES_FILE_PATH"en_game.bin", LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->screen_dice_game_btn_enter_game, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->screen_dice_game_btn_enter_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_dice_game_btn_enter_game, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_dice_game_btn_enter_game, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_dice_game_btn_enter_game, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_dice_game_btn_enter_game, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dice_game_img_goal
    ui->screen_dice_game_img_goal = lv_img_create(ui->screen_dice_game);
    lv_obj_add_flag(ui->screen_dice_game_img_goal, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dice_game_img_goal, CONFIG_UI_RES_FILE_PATH"d_score.bin");
    lv_img_set_pivot(ui->screen_dice_game_img_goal, 50, 50);
    lv_img_set_angle(ui->screen_dice_game_img_goal, 0);
    lv_obj_set_pos(ui->screen_dice_game_img_goal, 21, 27);
    lv_obj_set_size(ui->screen_dice_game_img_goal, 115, 41);

    //Write style for screen_dice_game_img_goal, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dice_game_img_goal, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dice_game_img_goal, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dice_game_img_goal, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dice_game_img_goal, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dice_game_img_shake
    ui->screen_dice_game_img_shake = lv_img_create(ui->screen_dice_game);
    lv_obj_add_flag(ui->screen_dice_game_img_shake, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dice_game_img_shake, CONFIG_UI_RES_FILE_PATH"shake.bin");
    lv_img_set_pivot(ui->screen_dice_game_img_shake, 50, 50);
    lv_img_set_angle(ui->screen_dice_game_img_shake, 0);
    lv_obj_set_pos(ui->screen_dice_game_img_shake, 155, 17);
    lv_obj_set_size(ui->screen_dice_game_img_shake, 141, 56);

    //Write style for screen_dice_game_img_shake, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dice_game_img_shake, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dice_game_img_shake, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dice_game_img_shake, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dice_game_img_shake, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dice_game_die1
    ui->screen_dice_game_die1 = lv_img_create(ui->screen_dice_game);
    lv_obj_add_flag(ui->screen_dice_game_die1, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dice_game_die1, CONFIG_UI_RES_FILE_PATH"die1.bin");
    lv_img_set_pivot(ui->screen_dice_game_die1, 50, 50);
    lv_img_set_angle(ui->screen_dice_game_die1, 0);
    lv_obj_set_pos(ui->screen_dice_game_die1, 40, 107);
    lv_obj_set_size(ui->screen_dice_game_die1, 100, 100);

    //Write style for screen_dice_game_die1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dice_game_die1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dice_game_die1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dice_game_die1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dice_game_die1, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dice_game_die2
    ui->screen_dice_game_die2 = lv_img_create(ui->screen_dice_game);
    lv_obj_add_flag(ui->screen_dice_game_die2, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dice_game_die2, CONFIG_UI_RES_FILE_PATH"die2.bin");
    lv_img_set_pivot(ui->screen_dice_game_die2, 50, 50);
    lv_img_set_angle(ui->screen_dice_game_die2, 0);
    lv_obj_set_pos(ui->screen_dice_game_die2, 178, 107);
    lv_obj_set_size(ui->screen_dice_game_die2, 100, 100);

    //Write style for screen_dice_game_die2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dice_game_die2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dice_game_die2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dice_game_die2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dice_game_die2, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dice_game_die4
    ui->screen_dice_game_die4 = lv_img_create(ui->screen_dice_game);
    lv_obj_add_flag(ui->screen_dice_game_die4, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dice_game_die4, CONFIG_UI_RES_FILE_PATH"die4.bin");
    lv_img_set_pivot(ui->screen_dice_game_die4, 50, 50);
    lv_img_set_angle(ui->screen_dice_game_die4, 0);
    lv_obj_set_pos(ui->screen_dice_game_die4, 99, -127);
    lv_obj_set_size(ui->screen_dice_game_die4, 100, 100);

    //Write style for screen_dice_game_die4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dice_game_die4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dice_game_die4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dice_game_die4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dice_game_die4, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dice_game_die6
    ui->screen_dice_game_die6 = lv_img_create(ui->screen_dice_game);
    lv_obj_add_flag(ui->screen_dice_game_die6, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dice_game_die6, CONFIG_UI_RES_FILE_PATH"die6.bin");
    lv_img_set_pivot(ui->screen_dice_game_die6, 50, 50);
    lv_img_set_angle(ui->screen_dice_game_die6, 0);
    lv_obj_set_pos(ui->screen_dice_game_die6, 332, -133);
    lv_obj_set_size(ui->screen_dice_game_die6, 100, 100);

    //Write style for screen_dice_game_die6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dice_game_die6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dice_game_die6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dice_game_die6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dice_game_die6, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dice_game_die3
    ui->screen_dice_game_die3 = lv_img_create(ui->screen_dice_game);
    lv_obj_add_flag(ui->screen_dice_game_die3, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dice_game_die3, CONFIG_UI_RES_FILE_PATH"die3.bin");
    lv_img_set_pivot(ui->screen_dice_game_die3, 50, 50);
    lv_img_set_angle(ui->screen_dice_game_die3, 0);
    lv_obj_set_pos(ui->screen_dice_game_die3, -22, -127);
    lv_obj_set_size(ui->screen_dice_game_die3, 100, 100);

    //Write style for screen_dice_game_die3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dice_game_die3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dice_game_die3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dice_game_die3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dice_game_die3, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dice_game_die5
    ui->screen_dice_game_die5 = lv_img_create(ui->screen_dice_game);
    lv_obj_add_flag(ui->screen_dice_game_die5, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dice_game_die5, CONFIG_UI_RES_FILE_PATH"die5.bin");
    lv_img_set_pivot(ui->screen_dice_game_die5, 50, 50);
    lv_img_set_angle(ui->screen_dice_game_die5, 0);
    lv_obj_set_pos(ui->screen_dice_game_die5, 216, -127);
    lv_obj_set_size(ui->screen_dice_game_die5, 100, 100);

    //Write style for screen_dice_game_die5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dice_game_die5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dice_game_die5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dice_game_die5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dice_game_die5, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dice_game_pdie5
    ui->screen_dice_game_pdie5 = lv_img_create(ui->screen_dice_game);
    lv_obj_add_flag(ui->screen_dice_game_pdie5, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dice_game_pdie5, CONFIG_UI_RES_FILE_PATH"pdie5.bin");
    lv_img_set_pivot(ui->screen_dice_game_pdie5, 50, 50);
    lv_img_set_angle(ui->screen_dice_game_pdie5, 0);
    lv_obj_set_pos(ui->screen_dice_game_pdie5, 213, -245);
    lv_obj_set_size(ui->screen_dice_game_pdie5, 100, 100);

    //Write style for screen_dice_game_pdie5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dice_game_pdie5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dice_game_pdie5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dice_game_pdie5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dice_game_pdie5, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dice_game_pdie3
    ui->screen_dice_game_pdie3 = lv_img_create(ui->screen_dice_game);
    lv_obj_add_flag(ui->screen_dice_game_pdie3, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dice_game_pdie3, CONFIG_UI_RES_FILE_PATH"pdie3.bin");
    lv_img_set_pivot(ui->screen_dice_game_pdie3, 50, 50);
    lv_img_set_angle(ui->screen_dice_game_pdie3, 0);
    lv_obj_set_pos(ui->screen_dice_game_pdie3, -25, -248);
    lv_obj_set_size(ui->screen_dice_game_pdie3, 100, 100);

    //Write style for screen_dice_game_pdie3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dice_game_pdie3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dice_game_pdie3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dice_game_pdie3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dice_game_pdie3, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dice_game_pdie6
    ui->screen_dice_game_pdie6 = lv_img_create(ui->screen_dice_game);
    lv_obj_add_flag(ui->screen_dice_game_pdie6, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dice_game_pdie6, CONFIG_UI_RES_FILE_PATH"pdie6.bin");
    lv_img_set_pivot(ui->screen_dice_game_pdie6, 50, 50);
    lv_img_set_angle(ui->screen_dice_game_pdie6, 0);
    lv_obj_set_pos(ui->screen_dice_game_pdie6, 338, -245);
    lv_obj_set_size(ui->screen_dice_game_pdie6, 100, 100);

    //Write style for screen_dice_game_pdie6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dice_game_pdie6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dice_game_pdie6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dice_game_pdie6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dice_game_pdie6, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dice_game_pdie4
    ui->screen_dice_game_pdie4 = lv_img_create(ui->screen_dice_game);
    lv_obj_add_flag(ui->screen_dice_game_pdie4, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dice_game_pdie4, CONFIG_UI_RES_FILE_PATH"pdie4.bin");
    lv_img_set_pivot(ui->screen_dice_game_pdie4, 50, 50);
    lv_img_set_angle(ui->screen_dice_game_pdie4, 0);
    lv_obj_set_pos(ui->screen_dice_game_pdie4, 96, -248);
    lv_obj_set_size(ui->screen_dice_game_pdie4, 100, 100);

    //Write style for screen_dice_game_pdie4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dice_game_pdie4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dice_game_pdie4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dice_game_pdie4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dice_game_pdie4, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dice_game_pdie2
    ui->screen_dice_game_pdie2 = lv_img_create(ui->screen_dice_game);
    lv_obj_add_flag(ui->screen_dice_game_pdie2, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dice_game_pdie2, CONFIG_UI_RES_FILE_PATH"pdie2.bin");
    lv_img_set_pivot(ui->screen_dice_game_pdie2, 50, 50);
    lv_img_set_angle(ui->screen_dice_game_pdie2, 0);
    lv_obj_set_pos(ui->screen_dice_game_pdie2, 192, 283);
    lv_obj_set_size(ui->screen_dice_game_pdie2, 100, 100);

    //Write style for screen_dice_game_pdie2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dice_game_pdie2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dice_game_pdie2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dice_game_pdie2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dice_game_pdie2, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dice_game_pdie1
    ui->screen_dice_game_pdie1 = lv_img_create(ui->screen_dice_game);
    lv_obj_add_flag(ui->screen_dice_game_pdie1, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dice_game_pdie1, CONFIG_UI_RES_FILE_PATH"pdie1.bin");
    lv_img_set_pivot(ui->screen_dice_game_pdie1, 50, 50);
    lv_img_set_angle(ui->screen_dice_game_pdie1, 0);
    lv_obj_set_pos(ui->screen_dice_game_pdie1, 51, 283);
    lv_obj_set_size(ui->screen_dice_game_pdie1, 100, 100);

    //Write style for screen_dice_game_pdie1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dice_game_pdie1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dice_game_pdie1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dice_game_pdie1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dice_game_pdie1, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dice_game_score
    ui->screen_dice_game_score = lv_spangroup_create(ui->screen_dice_game);
    lv_spangroup_set_align(ui->screen_dice_game_score, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_dice_game_score, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_dice_game_score, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_dice_game_score_span = lv_spangroup_new_span(ui->screen_dice_game_score);
    lv_span_set_text(ui->screen_dice_game_score_span, "目标:12点");
    lv_style_set_text_color(&ui->screen_dice_game_score_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_dice_game_score_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_dice_game_score_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_dice_game_score, 51, 38);
    lv_obj_set_size(ui->screen_dice_game_score, 62, 19);

    //Write style state: LV_STATE_DEFAULT for &style_screen_dice_game_score_main_main_default
    static lv_style_t style_screen_dice_game_score_main_main_default;
    ui_init_style(&style_screen_dice_game_score_main_main_default);

    lv_style_set_border_width(&style_screen_dice_game_score_main_main_default, 0);
    lv_style_set_radius(&style_screen_dice_game_score_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_dice_game_score_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_dice_game_score_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_dice_game_score_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_dice_game_score_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_dice_game_score_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_dice_game_score_main_main_default, 0);
    lv_obj_add_style(ui->screen_dice_game_score, &style_screen_dice_game_score_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_dice_game_score);

    //Write codes screen_dice_game_fail
    ui->screen_dice_game_fail = lv_img_create(ui->screen_dice_game);
    lv_obj_add_flag(ui->screen_dice_game_fail, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dice_game_fail, CONFIG_UI_RES_FILE_PATH"fail.bin");
    lv_img_set_pivot(ui->screen_dice_game_fail, 50, 50);
    lv_img_set_angle(ui->screen_dice_game_fail, 0);
    lv_obj_set_pos(ui->screen_dice_game_fail, 573, 8);
    lv_obj_set_size(ui->screen_dice_game_fail, 137, 86);

    //Write style for screen_dice_game_fail, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dice_game_fail, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dice_game_fail, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dice_game_fail, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dice_game_fail, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dice_game_btn_fail_again
    ui->screen_dice_game_btn_fail_again = lv_btn_create(ui->screen_dice_game);
    ui->screen_dice_game_btn_fail_again_label = lv_label_create(ui->screen_dice_game_btn_fail_again);
    lv_label_set_text(ui->screen_dice_game_btn_fail_again_label, "");
    lv_label_set_long_mode(ui->screen_dice_game_btn_fail_again_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_dice_game_btn_fail_again_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_dice_game_btn_fail_again, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_dice_game_btn_fail_again_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_dice_game_btn_fail_again, 560, 117);
    lv_obj_set_size(ui->screen_dice_game_btn_fail_again, 168, 61);

    //Write style for screen_dice_game_btn_fail_again, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_dice_game_btn_fail_again, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_dice_game_btn_fail_again, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dice_game_btn_fail_again, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_dice_game_btn_fail_again, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->screen_dice_game_btn_fail_again, CONFIG_UI_RES_FILE_PATH"fail_again.bin", LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->screen_dice_game_btn_fail_again, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->screen_dice_game_btn_fail_again, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_dice_game_btn_fail_again, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_dice_game_btn_fail_again, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_dice_game_btn_fail_again, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_dice_game_btn_fail_again, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dice_game_black_bg
    ui->screen_dice_game_black_bg = lv_img_create(ui->screen_dice_game);
    lv_obj_add_flag(ui->screen_dice_game_black_bg, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dice_game_black_bg, CONFIG_UI_RES_FILE_PATH"black_bg.bin");
    lv_img_set_pivot(ui->screen_dice_game_black_bg, 50, 50);
    lv_img_set_angle(ui->screen_dice_game_black_bg, 0);
    lv_obj_set_pos(ui->screen_dice_game_black_bg, 482, -237);
    lv_obj_set_size(ui->screen_dice_game_black_bg, 320, 240);

    //Write style for screen_dice_game_black_bg, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dice_game_black_bg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dice_game_black_bg, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dice_game_black_bg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dice_game_black_bg, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dice_game_btn_vic_again
    ui->screen_dice_game_btn_vic_again = lv_btn_create(ui->screen_dice_game);
    ui->screen_dice_game_btn_vic_again_label = lv_label_create(ui->screen_dice_game_btn_vic_again);
    lv_label_set_text(ui->screen_dice_game_btn_vic_again_label, "");
    lv_label_set_long_mode(ui->screen_dice_game_btn_vic_again_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_dice_game_btn_vic_again_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_dice_game_btn_vic_again, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_dice_game_btn_vic_again_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_dice_game_btn_vic_again, 558, -99);
    lv_obj_set_size(ui->screen_dice_game_btn_vic_again, 168, 61);

    //Write style for screen_dice_game_btn_vic_again, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_dice_game_btn_vic_again, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_dice_game_btn_vic_again, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dice_game_btn_vic_again, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_dice_game_btn_vic_again, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->screen_dice_game_btn_vic_again, CONFIG_UI_RES_FILE_PATH"win_again.bin", LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->screen_dice_game_btn_vic_again, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->screen_dice_game_btn_vic_again, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_dice_game_btn_vic_again, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_dice_game_btn_vic_again, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_dice_game_btn_vic_again, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_dice_game_btn_vic_again, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dice_game_victory
    ui->screen_dice_game_victory = lv_img_create(ui->screen_dice_game);
    lv_obj_add_flag(ui->screen_dice_game_victory, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dice_game_victory, CONFIG_UI_RES_FILE_PATH"win.bin");
    lv_img_set_pivot(ui->screen_dice_game_victory, 50, 50);
    lv_img_set_angle(ui->screen_dice_game_victory, 0);
    lv_obj_set_pos(ui->screen_dice_game_victory, 573, -204);
    lv_obj_set_size(ui->screen_dice_game_victory, 137, 86);

    //Write style for screen_dice_game_victory, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dice_game_victory, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dice_game_victory, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dice_game_victory, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dice_game_victory, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //The custom code of screen_dice_game.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_dice_game);

    //Init events for screen.
    events_init_screen_dice_game(ui);
}
