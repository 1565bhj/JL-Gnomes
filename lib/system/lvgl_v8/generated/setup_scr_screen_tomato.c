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



void setup_scr_screen_tomato(lv_ui *ui)
{
    //Write codes screen_tomato
    ui->screen_tomato = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_tomato, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_tomato, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_tomato, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_tomato, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_tomato, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_tomato, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_tomato_img_add
    ui->screen_tomato_img_add = lv_img_create(ui->screen_tomato);
    lv_obj_add_flag(ui->screen_tomato_img_add, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_tomato_img_add, &_add_alpha_30x30);
    lv_img_set_pivot(ui->screen_tomato_img_add, 50,50);
    lv_img_set_angle(ui->screen_tomato_img_add, 0);
    lv_obj_set_pos(ui->screen_tomato_img_add, 255, 12);
    lv_obj_set_size(ui->screen_tomato_img_add, 30, 30);

    //Write style for screen_tomato_img_add, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_tomato_img_add, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_tomato_img_add, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_tomato_img_add, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_tomato_img_add, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_tomato_img_reset
    ui->screen_tomato_img_reset = lv_img_create(ui->screen_tomato);
    lv_obj_add_flag(ui->screen_tomato_img_reset, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_tomato_img_reset, &_refresh_alpha_22x22);
    lv_img_set_pivot(ui->screen_tomato_img_reset, 50,50);
    lv_img_set_angle(ui->screen_tomato_img_reset, 0);
    lv_obj_set_pos(ui->screen_tomato_img_reset, 35, 16);
    lv_obj_set_size(ui->screen_tomato_img_reset, 22, 22);

    //Write style for screen_tomato_img_reset, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_tomato_img_reset, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_tomato_img_reset, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_tomato_img_reset, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_tomato_img_reset, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_tomato_circle
    ui->screen_tomato_circle = lv_img_create(ui->screen_tomato);
    lv_obj_add_flag(ui->screen_tomato_circle, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_tomato_circle, CONFIG_UI_RES_FILE_PATH"circle.bin");
    lv_img_set_pivot(ui->screen_tomato_circle, 50,50);
    lv_img_set_angle(ui->screen_tomato_circle, 0);
    lv_obj_set_pos(ui->screen_tomato_circle, 47, 7);
    lv_obj_set_size(ui->screen_tomato_circle, 225, 225);

    //Write style for screen_tomato_circle, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_tomato_circle, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_tomato_circle, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_tomato_circle, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_tomato_circle, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_tomato_time
    ui->screen_tomato_time = lv_label_create(ui->screen_tomato);
    lv_label_set_text(ui->screen_tomato_time, "00:00");
    lv_label_set_long_mode(ui->screen_tomato_time, LV_LABEL_LONG_CLIP);
    lv_obj_set_pos(ui->screen_tomato_time, 108, 98);
    lv_obj_set_size(ui->screen_tomato_time, 105, 47);

    //Write style for screen_tomato_time, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_tomato_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_tomato_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_tomato_time, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_tomato_time, &lv_font_Barlow__40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_tomato_time, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_tomato_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_tomato_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_tomato_time, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_tomato_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_tomato_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_tomato_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_tomato_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_tomato_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_tomato_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_tomato_hide_part
    ui->screen_tomato_hide_part = lv_img_create(ui->screen_tomato);
    lv_obj_add_flag(ui->screen_tomato_hide_part, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_tomato_hide_part, &_hide_part_alpha_52x51);
    lv_img_set_pivot(ui->screen_tomato_hide_part, 50,50);
    lv_img_set_angle(ui->screen_tomato_hide_part, 0);
    lv_obj_set_pos(ui->screen_tomato_hide_part, -8, -82);
    lv_obj_set_size(ui->screen_tomato_hide_part, 52, 51);

    //Write style for screen_tomato_hide_part, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_tomato_hide_part, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_tomato_hide_part, 230, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_tomato_hide_part, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_tomato_hide_part, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of screen_tomato.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_tomato);

    //Init events for screen.
    events_init_screen_tomato(ui);
}
