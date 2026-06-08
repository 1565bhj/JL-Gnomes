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



void setup_scr_screen_dropdown(lv_ui *ui)
{
    //Write codes screen_dropdown
    ui->screen_dropdown = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_dropdown, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_dropdown, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_dropdown, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_dropdown, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_dropdown, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_dropdown, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dropdown_max_volumn
    ui->screen_dropdown_max_volumn = lv_img_create(ui->screen_dropdown);
    lv_obj_add_flag(ui->screen_dropdown_max_volumn, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dropdown_max_volumn, &_volume_alpha_30x30);
    lv_img_set_pivot(ui->screen_dropdown_max_volumn, 50, 50);
    lv_img_set_angle(ui->screen_dropdown_max_volumn, 0);
    lv_obj_set_pos(ui->screen_dropdown_max_volumn, 250, 137);
    lv_obj_set_size(ui->screen_dropdown_max_volumn, 30, 30);

    //Write style for screen_dropdown_max_volumn, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_max_volumn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dropdown_max_volumn, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dropdown_max_volumn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dropdown_max_volumn, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dropdown_min_light
    ui->screen_dropdown_min_light = lv_img_create(ui->screen_dropdown);
    lv_obj_add_flag(ui->screen_dropdown_min_light, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dropdown_min_light, &_min_light_alpha_25x25);
    lv_img_set_pivot(ui->screen_dropdown_min_light, 50, 50);
    lv_img_set_angle(ui->screen_dropdown_min_light, 0);
    lv_obj_set_pos(ui->screen_dropdown_min_light, 37, 183);
    lv_obj_set_size(ui->screen_dropdown_min_light, 25, 25);

    //Write style for screen_dropdown_min_light, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_min_light, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dropdown_min_light, 77, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dropdown_min_light, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dropdown_min_light, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dropdown_max_light
    ui->screen_dropdown_max_light = lv_img_create(ui->screen_dropdown);
    lv_obj_add_flag(ui->screen_dropdown_max_light, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dropdown_max_light, &_max_light_alpha_30x30);
    lv_img_set_pivot(ui->screen_dropdown_max_light, 50, 50);
    lv_img_set_angle(ui->screen_dropdown_max_light, 0);
    lv_obj_set_pos(ui->screen_dropdown_max_light, 250, 182);
    lv_obj_set_size(ui->screen_dropdown_max_light, 30, 30);

    //Write style for screen_dropdown_max_light, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_max_light, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dropdown_max_light, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dropdown_max_light, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dropdown_max_light, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dropdown_min_volumn
    ui->screen_dropdown_min_volumn = lv_img_create(ui->screen_dropdown);
    lv_obj_add_flag(ui->screen_dropdown_min_volumn, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dropdown_min_volumn, &_min_volumn_alpha_25x25);
    lv_img_set_pivot(ui->screen_dropdown_min_volumn, 50, 50);
    lv_img_set_angle(ui->screen_dropdown_min_volumn, 0);
    lv_obj_set_pos(ui->screen_dropdown_min_volumn, 37, 137);
    lv_obj_set_size(ui->screen_dropdown_min_volumn, 25, 25);

    //Write style for screen_dropdown_min_volumn, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_min_volumn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dropdown_min_volumn, 79, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dropdown_min_volumn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dropdown_min_volumn, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dropdown_cont
    ui->screen_dropdown_cont = lv_obj_create(ui->screen_dropdown);
    lv_obj_set_pos(ui->screen_dropdown_cont, 34, 52);
    lv_obj_set_size(ui->screen_dropdown_cont, 249, 70);
    lv_obj_set_scrollbar_mode(ui->screen_dropdown_cont, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_dropdown_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_dropdown_cont, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_dropdown_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_dropdown_cont, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_dropdown_cont, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dropdown_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_dropdown_cont, 39, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_dropdown_cont, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_dropdown_cont, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_dropdown_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_dropdown_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_dropdown_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_dropdown_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_dropdown_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dropdown_imgbtn_mic
    ui->screen_dropdown_imgbtn_mic = lv_imgbtn_create(ui->screen_dropdown_cont);
    lv_obj_add_flag(ui->screen_dropdown_imgbtn_mic, LV_OBJ_FLAG_CHECKABLE);
    lv_imgbtn_set_src(ui->screen_dropdown_imgbtn_mic, LV_IMGBTN_STATE_RELEASED, NULL, &_mic_alpha_25x30, NULL);
    lv_imgbtn_set_src(ui->screen_dropdown_imgbtn_mic, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &_no_mic_alpha_25x30, NULL);
    ui->screen_dropdown_imgbtn_mic_label = lv_label_create(ui->screen_dropdown_imgbtn_mic);
    lv_label_set_text(ui->screen_dropdown_imgbtn_mic_label, "");
    lv_label_set_long_mode(ui->screen_dropdown_imgbtn_mic_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_dropdown_imgbtn_mic_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_dropdown_imgbtn_mic, 0, LV_STATE_DEFAULT);
    lv_obj_set_pos(ui->screen_dropdown_imgbtn_mic, 25, 19);
    lv_obj_set_size(ui->screen_dropdown_imgbtn_mic, 25, 30);

    //Write style for screen_dropdown_imgbtn_mic, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_text_color(ui->screen_dropdown_imgbtn_mic, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_dropdown_imgbtn_mic, &lv_font_montserratMedium_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_dropdown_imgbtn_mic, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_dropdown_imgbtn_mic, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dropdown_imgbtn_mic, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dropdown_imgbtn_mic, true, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_dropdown_imgbtn_mic, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style for screen_dropdown_imgbtn_mic, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_imgbtn_mic, 0, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_img_opa(ui->screen_dropdown_imgbtn_mic, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_color(ui->screen_dropdown_imgbtn_mic, lv_color_hex(0xFF33FF), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_font(ui->screen_dropdown_imgbtn_mic, &lv_font_montserratMedium_12, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_opa(ui->screen_dropdown_imgbtn_mic, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(ui->screen_dropdown_imgbtn_mic, 0, LV_PART_MAIN | LV_STATE_PRESSED);

    //Write style for screen_dropdown_imgbtn_mic, Part: LV_PART_MAIN, State: LV_STATE_CHECKED.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_imgbtn_mic, 0, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_img_opa(ui->screen_dropdown_imgbtn_mic, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(ui->screen_dropdown_imgbtn_mic, lv_color_hex(0xFF33FF), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_font(ui->screen_dropdown_imgbtn_mic, &lv_font_montserratMedium_12, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(ui->screen_dropdown_imgbtn_mic, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_width(ui->screen_dropdown_imgbtn_mic, 0, LV_PART_MAIN | LV_STATE_CHECKED);

    //Write style for screen_dropdown_imgbtn_mic, Part: LV_PART_MAIN, State: LV_IMGBTN_STATE_RELEASED.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_imgbtn_mic, 0, LV_PART_MAIN | LV_IMGBTN_STATE_RELEASED);
    lv_obj_set_style_img_opa(ui->screen_dropdown_imgbtn_mic, 255, LV_PART_MAIN | LV_IMGBTN_STATE_RELEASED);

    //Write codes screen_dropdown_imgbtn_speaker
    ui->screen_dropdown_imgbtn_speaker = lv_imgbtn_create(ui->screen_dropdown_cont);
    lv_obj_add_flag(ui->screen_dropdown_imgbtn_speaker, LV_OBJ_FLAG_CHECKABLE);
    lv_imgbtn_set_src(ui->screen_dropdown_imgbtn_speaker, LV_IMGBTN_STATE_RELEASED, NULL, &_max_volumn_alpha_30x30, NULL);
    lv_imgbtn_set_src(ui->screen_dropdown_imgbtn_speaker, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &_no_volumn_alpha_30x30, NULL);
    ui->screen_dropdown_imgbtn_speaker_label = lv_label_create(ui->screen_dropdown_imgbtn_speaker);
    lv_label_set_text(ui->screen_dropdown_imgbtn_speaker_label, "");
    lv_label_set_long_mode(ui->screen_dropdown_imgbtn_speaker_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_dropdown_imgbtn_speaker_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_dropdown_imgbtn_speaker, 0, LV_STATE_DEFAULT);
    lv_obj_set_pos(ui->screen_dropdown_imgbtn_speaker, 111, 18);
    lv_obj_set_size(ui->screen_dropdown_imgbtn_speaker, 30, 30);

    //Write style for screen_dropdown_imgbtn_speaker, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_text_color(ui->screen_dropdown_imgbtn_speaker, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_dropdown_imgbtn_speaker, &lv_font_montserratMedium_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_dropdown_imgbtn_speaker, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_dropdown_imgbtn_speaker, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dropdown_imgbtn_speaker, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dropdown_imgbtn_speaker, true, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_dropdown_imgbtn_speaker, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style for screen_dropdown_imgbtn_speaker, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_imgbtn_speaker, 0, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_img_opa(ui->screen_dropdown_imgbtn_speaker, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_color(ui->screen_dropdown_imgbtn_speaker, lv_color_hex(0xFF33FF), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_font(ui->screen_dropdown_imgbtn_speaker, &lv_font_montserratMedium_12, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_opa(ui->screen_dropdown_imgbtn_speaker, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(ui->screen_dropdown_imgbtn_speaker, 0, LV_PART_MAIN | LV_STATE_PRESSED);

    //Write style for screen_dropdown_imgbtn_speaker, Part: LV_PART_MAIN, State: LV_STATE_CHECKED.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_imgbtn_speaker, 0, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_img_opa(ui->screen_dropdown_imgbtn_speaker, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(ui->screen_dropdown_imgbtn_speaker, lv_color_hex(0xFF33FF), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_font(ui->screen_dropdown_imgbtn_speaker, &lv_font_montserratMedium_12, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(ui->screen_dropdown_imgbtn_speaker, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_width(ui->screen_dropdown_imgbtn_speaker, 0, LV_PART_MAIN | LV_STATE_CHECKED);

    //Write style for screen_dropdown_imgbtn_speaker, Part: LV_PART_MAIN, State: LV_IMGBTN_STATE_RELEASED.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_imgbtn_speaker, 0, LV_PART_MAIN | LV_IMGBTN_STATE_RELEASED);
    lv_obj_set_style_img_opa(ui->screen_dropdown_imgbtn_speaker, 255, LV_PART_MAIN | LV_IMGBTN_STATE_RELEASED);

    //Write codes screen_dropdown_imgbtn_BT
    ui->screen_dropdown_imgbtn_BT = lv_imgbtn_create(ui->screen_dropdown_cont);
    lv_obj_add_flag(ui->screen_dropdown_imgbtn_BT, LV_OBJ_FLAG_CHECKABLE);
    lv_imgbtn_set_src(ui->screen_dropdown_imgbtn_BT, LV_IMGBTN_STATE_RELEASED, NULL, &_no_BT2_alpha_40x30, NULL);
    lv_imgbtn_set_src(ui->screen_dropdown_imgbtn_BT, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &_BT2_alpha_40x30, NULL);
    ui->screen_dropdown_imgbtn_BT_label = lv_label_create(ui->screen_dropdown_imgbtn_BT);
    lv_label_set_text(ui->screen_dropdown_imgbtn_BT_label, "");
    lv_label_set_long_mode(ui->screen_dropdown_imgbtn_BT_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_dropdown_imgbtn_BT_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_dropdown_imgbtn_BT, 0, LV_STATE_DEFAULT);
    lv_obj_set_pos(ui->screen_dropdown_imgbtn_BT, 185, 17);
    lv_obj_set_size(ui->screen_dropdown_imgbtn_BT, 40, 30);

    //Write style for screen_dropdown_imgbtn_BT, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_text_color(ui->screen_dropdown_imgbtn_BT, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_dropdown_imgbtn_BT, &lv_font_montserratMedium_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_dropdown_imgbtn_BT, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_dropdown_imgbtn_BT, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dropdown_imgbtn_BT, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dropdown_imgbtn_BT, true, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_dropdown_imgbtn_BT, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style for screen_dropdown_imgbtn_BT, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_imgbtn_BT, 0, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_img_opa(ui->screen_dropdown_imgbtn_BT, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_color(ui->screen_dropdown_imgbtn_BT, lv_color_hex(0xFF33FF), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_font(ui->screen_dropdown_imgbtn_BT, &lv_font_montserratMedium_12, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_opa(ui->screen_dropdown_imgbtn_BT, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(ui->screen_dropdown_imgbtn_BT, 0, LV_PART_MAIN | LV_STATE_PRESSED);

    //Write style for screen_dropdown_imgbtn_BT, Part: LV_PART_MAIN, State: LV_STATE_CHECKED.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_imgbtn_BT, 0, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_img_opa(ui->screen_dropdown_imgbtn_BT, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(ui->screen_dropdown_imgbtn_BT, lv_color_hex(0xFF33FF), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_font(ui->screen_dropdown_imgbtn_BT, &lv_font_montserratMedium_12, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(ui->screen_dropdown_imgbtn_BT, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_width(ui->screen_dropdown_imgbtn_BT, 0, LV_PART_MAIN | LV_STATE_CHECKED);

    //Write style for screen_dropdown_imgbtn_BT, Part: LV_PART_MAIN, State: LV_IMGBTN_STATE_RELEASED.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_imgbtn_BT, 0, LV_PART_MAIN | LV_IMGBTN_STATE_RELEASED);
    lv_obj_set_style_img_opa(ui->screen_dropdown_imgbtn_BT, 255, LV_PART_MAIN | LV_IMGBTN_STATE_RELEASED);

    //Write codes screen_dropdown_img_charging_battery
    ui->screen_dropdown_img_charging_battery = lv_img_create(ui->screen_dropdown);
    lv_obj_add_flag(ui->screen_dropdown_img_charging_battery, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dropdown_img_charging_battery, &_charging_battery_alpha_22x14);
    lv_img_set_pivot(ui->screen_dropdown_img_charging_battery, 50, 50);
    lv_img_set_angle(ui->screen_dropdown_img_charging_battery, 0);
    lv_obj_set_pos(ui->screen_dropdown_img_charging_battery, 255, -71);
    lv_obj_set_size(ui->screen_dropdown_img_charging_battery, 22, 14);

    //Write style for screen_dropdown_img_charging_battery, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_img_charging_battery, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dropdown_img_charging_battery, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dropdown_img_charging_battery, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dropdown_img_charging_battery, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dropdown_img_low_battery
    ui->screen_dropdown_img_low_battery = lv_img_create(ui->screen_dropdown);
    lv_obj_add_flag(ui->screen_dropdown_img_low_battery, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dropdown_img_low_battery, &_low_battery_alpha_22x14);
    lv_img_set_pivot(ui->screen_dropdown_img_low_battery, 50, 50);
    lv_img_set_angle(ui->screen_dropdown_img_low_battery, 0);
    lv_obj_set_pos(ui->screen_dropdown_img_low_battery, 255, -48);
    lv_obj_set_size(ui->screen_dropdown_img_low_battery, 22, 14);

    //Write style for screen_dropdown_img_low_battery, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_img_low_battery, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dropdown_img_low_battery, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dropdown_img_low_battery, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dropdown_img_low_battery, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dropdown_img_set_alarm
    ui->screen_dropdown_img_set_alarm = lv_img_create(ui->screen_dropdown);
    lv_obj_add_flag(ui->screen_dropdown_img_set_alarm, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dropdown_img_set_alarm, &_set_alarm_alpha_17x15);
    lv_img_set_pivot(ui->screen_dropdown_img_set_alarm, 50, 50);
    lv_img_set_angle(ui->screen_dropdown_img_set_alarm, 0);
    lv_obj_set_pos(ui->screen_dropdown_img_set_alarm, 181, -31);
    lv_obj_set_size(ui->screen_dropdown_img_set_alarm, 17, 15);

    //Write style for screen_dropdown_img_set_alarm, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_img_set_alarm, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dropdown_img_set_alarm, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dropdown_img_set_alarm, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dropdown_img_set_alarm, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dropdown_img_set_BT
    ui->screen_dropdown_img_set_BT = lv_img_create(ui->screen_dropdown);
    lv_obj_add_flag(ui->screen_dropdown_img_set_BT, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dropdown_img_set_BT, &_BT_alpha_14x16);
    lv_img_set_pivot(ui->screen_dropdown_img_set_BT, 50, 50);
    lv_img_set_angle(ui->screen_dropdown_img_set_BT, 0);
    lv_obj_set_pos(ui->screen_dropdown_img_set_BT, 207, -31);
    lv_obj_set_size(ui->screen_dropdown_img_set_BT, 14, 16);

    //Write style for screen_dropdown_img_set_BT, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_img_set_BT, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dropdown_img_set_BT, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dropdown_img_set_BT, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dropdown_img_set_BT, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dropdown_img_set_wifi
    ui->screen_dropdown_img_set_wifi = lv_img_create(ui->screen_dropdown);
    lv_obj_add_flag(ui->screen_dropdown_img_set_wifi, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dropdown_img_set_wifi, &_set_wifi_alpha_18x16);
    lv_img_set_pivot(ui->screen_dropdown_img_set_wifi, 50, 50);
    lv_img_set_angle(ui->screen_dropdown_img_set_wifi, 0);
    lv_obj_set_pos(ui->screen_dropdown_img_set_wifi, 231, -31);
    lv_obj_set_size(ui->screen_dropdown_img_set_wifi, 18, 16);

    //Write style for screen_dropdown_img_set_wifi, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_img_set_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dropdown_img_set_wifi, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dropdown_img_set_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dropdown_img_set_wifi, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dropdown_img_fully_battery
    ui->screen_dropdown_img_fully_battery = lv_img_create(ui->screen_dropdown);
    lv_obj_add_flag(ui->screen_dropdown_img_fully_battery, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dropdown_img_fully_battery, &_fully_charged_battery_alpha_22x14);
    lv_img_set_pivot(ui->screen_dropdown_img_fully_battery, 50, 50);
    lv_img_set_angle(ui->screen_dropdown_img_fully_battery, 0);
    lv_obj_set_pos(ui->screen_dropdown_img_fully_battery, 255, -26);
    lv_obj_set_size(ui->screen_dropdown_img_fully_battery, 22, 14);

    //Write style for screen_dropdown_img_fully_battery, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_img_fully_battery, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dropdown_img_fully_battery, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dropdown_img_fully_battery, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dropdown_img_fully_battery, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dropdown_img_normal_battery
    ui->screen_dropdown_img_normal_battery = lv_img_create(ui->screen_dropdown);
    lv_obj_add_flag(ui->screen_dropdown_img_normal_battery, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dropdown_img_normal_battery, &_normol_battery_alpha_22x14);
    lv_img_set_pivot(ui->screen_dropdown_img_normal_battery, 50, 50);
    lv_img_set_angle(ui->screen_dropdown_img_normal_battery, 0);
    lv_obj_set_pos(ui->screen_dropdown_img_normal_battery, 259, 20);
    lv_obj_set_size(ui->screen_dropdown_img_normal_battery, 22, 14);

    //Write style for screen_dropdown_img_normal_battery, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_img_normal_battery, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dropdown_img_normal_battery, 221, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dropdown_img_normal_battery, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dropdown_img_normal_battery, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dropdown_img_no_wifi
    ui->screen_dropdown_img_no_wifi = lv_img_create(ui->screen_dropdown);
    lv_obj_add_flag(ui->screen_dropdown_img_no_wifi, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dropdown_img_no_wifi, &_no_wifi_alpha_18x16);
    lv_img_set_pivot(ui->screen_dropdown_img_no_wifi, 50, 50);
    lv_img_set_angle(ui->screen_dropdown_img_no_wifi, 0);
    lv_obj_set_pos(ui->screen_dropdown_img_no_wifi, 231, 20);
    lv_obj_set_size(ui->screen_dropdown_img_no_wifi, 18, 16);

    //Write style for screen_dropdown_img_no_wifi, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_img_no_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dropdown_img_no_wifi, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dropdown_img_no_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dropdown_img_no_wifi, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dropdown_img_no_BT
    ui->screen_dropdown_img_no_BT = lv_img_create(ui->screen_dropdown);
    lv_obj_add_flag(ui->screen_dropdown_img_no_BT, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dropdown_img_no_BT, &_no_BT_alpha_14x16);
    lv_img_set_pivot(ui->screen_dropdown_img_no_BT, 50, 50);
    lv_img_set_angle(ui->screen_dropdown_img_no_BT, 0);
    lv_obj_set_pos(ui->screen_dropdown_img_no_BT, 207, 19);
    lv_obj_set_size(ui->screen_dropdown_img_no_BT, 14, 16);

    //Write style for screen_dropdown_img_no_BT, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_img_no_BT, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dropdown_img_no_BT, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dropdown_img_no_BT, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dropdown_img_no_BT, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dropdown_img_no_alarm
    ui->screen_dropdown_img_no_alarm = lv_img_create(ui->screen_dropdown);
    lv_obj_add_flag(ui->screen_dropdown_img_no_alarm, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_dropdown_img_no_alarm, &_no_alarm_alpha_17x15);
    lv_img_set_pivot(ui->screen_dropdown_img_no_alarm, 50, 50);
    lv_img_set_angle(ui->screen_dropdown_img_no_alarm, 0);
    lv_obj_set_pos(ui->screen_dropdown_img_no_alarm, 181, 20);
    lv_obj_set_size(ui->screen_dropdown_img_no_alarm, 17, 15);

    //Write style for screen_dropdown_img_no_alarm, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_dropdown_img_no_alarm, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_dropdown_img_no_alarm, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dropdown_img_no_alarm, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_dropdown_img_no_alarm, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_dropdown_time
    ui->screen_dropdown_time = lv_spangroup_create(ui->screen_dropdown);
    lv_spangroup_set_align(ui->screen_dropdown_time, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_dropdown_time, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_dropdown_time, LV_SPAN_MODE_BREAK);
    //create span
    ui->screen_dropdown_time_span = lv_spangroup_new_span(ui->screen_dropdown_time);
    lv_span_set_text(ui->screen_dropdown_time_span, "12:12");
    lv_style_set_text_color(&ui->screen_dropdown_time_span->style, lv_color_hex(0xc8c8c8));
    lv_style_set_text_decor(&ui->screen_dropdown_time_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_dropdown_time_span->style, &lv_font_Barlow__18);
    lv_obj_set_pos(ui->screen_dropdown_time, 42, 19);
    lv_obj_set_size(ui->screen_dropdown_time, 46, 16);

    //Write style state: LV_STATE_DEFAULT for &style_screen_dropdown_time_main_main_default
    static lv_style_t style_screen_dropdown_time_main_main_default;
    ui_init_style(&style_screen_dropdown_time_main_main_default);

    lv_style_set_border_width(&style_screen_dropdown_time_main_main_default, 0);
    lv_style_set_radius(&style_screen_dropdown_time_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_dropdown_time_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_dropdown_time_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_dropdown_time_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_dropdown_time_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_dropdown_time_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_dropdown_time_main_main_default, 0);
    lv_obj_add_style(ui->screen_dropdown_time, &style_screen_dropdown_time_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_dropdown_time);

    //Write codes screen_dropdown_volumn_bar
    ui->screen_dropdown_volumn_bar = lv_bar_create(ui->screen_dropdown);
    lv_obj_set_style_anim_time(ui->screen_dropdown_volumn_bar, 1000, 0);
    lv_bar_set_mode(ui->screen_dropdown_volumn_bar, LV_BAR_MODE_NORMAL);
    lv_bar_set_range(ui->screen_dropdown_volumn_bar, 0, 100);
    lv_bar_set_value(ui->screen_dropdown_volumn_bar, 11, LV_ANIM_OFF);
    lv_obj_set_pos(ui->screen_dropdown_volumn_bar, 70, 146);
    lv_obj_set_size(ui->screen_dropdown_volumn_bar, 171, 10);

    //Write style for screen_dropdown_volumn_bar, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_dropdown_volumn_bar, 61, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_dropdown_volumn_bar, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_dropdown_volumn_bar, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dropdown_volumn_bar, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_dropdown_volumn_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style for screen_dropdown_volumn_bar, Part: LV_PART_INDICATOR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_dropdown_volumn_bar, 218, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_dropdown_volumn_bar, lv_color_hex(0xffffff), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_dropdown_volumn_bar, LV_GRAD_DIR_NONE, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dropdown_volumn_bar, 10, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    //Write codes screen_dropdown_light_bar
    ui->screen_dropdown_light_bar = lv_bar_create(ui->screen_dropdown);
    lv_obj_set_style_anim_time(ui->screen_dropdown_light_bar, 1000, 0);
    lv_bar_set_mode(ui->screen_dropdown_light_bar, LV_BAR_MODE_NORMAL);
    lv_bar_set_range(ui->screen_dropdown_light_bar, 0, 100);
    lv_bar_set_value(ui->screen_dropdown_light_bar, 10, LV_ANIM_OFF);
    lv_obj_set_pos(ui->screen_dropdown_light_bar, 70, 192);
    lv_obj_set_size(ui->screen_dropdown_light_bar, 171, 10);

    //Write style for screen_dropdown_light_bar, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_dropdown_light_bar, 60, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_dropdown_light_bar, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_dropdown_light_bar, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dropdown_light_bar, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_dropdown_light_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style for screen_dropdown_light_bar, Part: LV_PART_INDICATOR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_dropdown_light_bar, 217, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_dropdown_light_bar, lv_color_hex(0xffffff), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_dropdown_light_bar, LV_GRAD_DIR_NONE, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_dropdown_light_bar, 10, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    //The custom code of screen_dropdown.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_dropdown);

    //Init events for screen.
    events_init_screen_dropdown(ui);
}
