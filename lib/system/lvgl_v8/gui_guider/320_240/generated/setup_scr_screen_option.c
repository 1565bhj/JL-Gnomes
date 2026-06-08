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



void setup_scr_screen_option(lv_ui *ui)
{
    //Write codes screen_option
    ui->screen_option = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_option, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_option, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_option, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_option, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->screen_option, &_background_320x240, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->screen_option, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->screen_option, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_option_option_music
    ui->screen_option_option_music = lv_img_create(ui->screen_option);
    lv_obj_add_flag(ui->screen_option_option_music, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_option_option_music, &_music_option_alpha_100x100);
    lv_img_set_pivot(ui->screen_option_option_music, 50, 50);
    lv_img_set_angle(ui->screen_option_option_music, 0);
    lv_obj_set_pos(ui->screen_option_option_music, -212, -210);
    lv_obj_set_size(ui->screen_option_option_music, 100, 100);

    //Write style for screen_option_option_music, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_option_option_music, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_option_option_music, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_option_option_music, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_option_option_music, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_option_option_timer
    ui->screen_option_option_timer = lv_img_create(ui->screen_option);
    lv_obj_add_flag(ui->screen_option_option_timer, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_option_option_timer, &_timer_option_alpha_100x100);
    lv_img_set_pivot(ui->screen_option_option_timer, 50, 50);
    lv_img_set_angle(ui->screen_option_option_timer, 0);
    lv_obj_set_pos(ui->screen_option_option_timer, 285, -210);
    lv_obj_set_size(ui->screen_option_option_timer, 100, 100);

    //Write style for screen_option_option_timer, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_option_option_timer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_option_option_timer, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_option_option_timer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_option_option_timer, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_option_option_set
    ui->screen_option_option_set = lv_img_create(ui->screen_option);
    lv_obj_add_flag(ui->screen_option_option_set, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_option_option_set, &_op_set_alpha_100x100);
    lv_img_set_pivot(ui->screen_option_option_set, 50, 50);
    lv_img_set_angle(ui->screen_option_option_set, 0);
    lv_obj_set_pos(ui->screen_option_option_set, -77, -210);
    lv_obj_set_size(ui->screen_option_option_set, 100, 100);

    //Write style for screen_option_option_set, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_option_option_set, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_option_option_set, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_option_option_set, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_option_option_set, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_option_option_conversation
    ui->screen_option_option_conversation = lv_img_create(ui->screen_option);
    lv_obj_add_flag(ui->screen_option_option_conversation, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_option_option_conversation, &_conversation_option_alpha_100x100);
    lv_img_set_pivot(ui->screen_option_option_conversation, 50, 50);
    lv_img_set_angle(ui->screen_option_option_conversation, 0);
    lv_obj_set_pos(ui->screen_option_option_conversation, 41, -210);
    lv_obj_set_size(ui->screen_option_option_conversation, 100, 100);

    //Write style for screen_option_option_conversation, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_option_option_conversation, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_option_option_conversation, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_option_option_conversation, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_option_option_conversation, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_option_option_alarm
    ui->screen_option_option_alarm = lv_img_create(ui->screen_option);
    lv_obj_add_flag(ui->screen_option_option_alarm, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_option_option_alarm, &_alarm_option_alpha_100x100);
    lv_img_set_pivot(ui->screen_option_option_alarm, 50, 50);
    lv_img_set_angle(ui->screen_option_option_alarm, 0);
    lv_obj_set_pos(ui->screen_option_option_alarm, 165, -210);
    lv_obj_set_size(ui->screen_option_option_alarm, 100, 100);

    //Write style for screen_option_option_alarm, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_option_option_alarm, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_option_option_alarm, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_option_option_alarm, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_option_option_alarm, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_option_op_name
    ui->screen_option_op_name = lv_label_create(ui->screen_option);
    lv_label_set_text(ui->screen_option_op_name, "名称");
    lv_label_set_long_mode(ui->screen_option_op_name, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_option_op_name, 118, 32);
    lv_obj_set_size(ui->screen_option_op_name, 84, 22);

    //Write style for screen_option_op_name, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_option_op_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_option_op_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_option_op_name, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_option_op_name, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_option_op_name, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_option_op_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_option_op_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_option_op_name, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_option_op_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_option_op_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_option_op_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_option_op_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_option_op_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_option_op_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //The custom code of screen_option.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_option);

    //Init events for screen.
    events_init_screen_option(ui);
}
