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



void setup_scr_screen_ring(lv_ui *ui)
{
    //Write codes screen_ring
    ui->screen_ring = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_ring, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_ring, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_ring, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_ring, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_ring, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_ring, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_ring_light
    ui->screen_ring_light = lv_img_create(ui->screen_ring);
    lv_obj_add_flag(ui->screen_ring_light, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_ring_light, &_ring_light_alpha_200x240);
    lv_img_set_pivot(ui->screen_ring_light, 100, 0);
    lv_img_set_angle(ui->screen_ring_light, 0);
    lv_obj_set_pos(ui->screen_ring_light, 60, -17);
    lv_obj_set_size(ui->screen_ring_light, 200, 240);

    //Write style for screen_ring_light, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_ring_light, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_ring_light, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_ring_light, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_ring_light, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_ring_ring
    ui->screen_ring_ring = lv_img_create(ui->screen_ring);
    lv_obj_add_flag(ui->screen_ring_ring, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_ring_ring, &_ring_alpha_100x100);
    lv_img_set_pivot(ui->screen_ring_ring, 50, 50);
    lv_img_set_angle(ui->screen_ring_ring, 0);
    lv_obj_set_pos(ui->screen_ring_ring, 110, 64);
    lv_obj_set_size(ui->screen_ring_ring, 100, 100);

    //Write style for screen_ring_ring, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_ring_ring, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_ring_ring, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_ring_ring, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_ring_ring, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //The custom code of screen_ring.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_ring);

    //Init events for screen.
    events_init_screen_ring(ui);
}
