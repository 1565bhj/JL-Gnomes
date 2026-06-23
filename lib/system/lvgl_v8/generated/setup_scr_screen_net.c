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



void setup_scr_screen_net(lv_ui *ui)
{
    //Write codes screen_net
    ui->screen_net = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_net, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_net, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_net, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_net, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_net, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_net, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_net_img_return
    ui->screen_net_img_return = lv_img_create(ui->screen_net);
    lv_obj_add_flag(ui->screen_net_img_return, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_net_img_return, &_return_alpha_22x22);
    lv_img_set_pivot(ui->screen_net_img_return, 50, 50);
    lv_img_set_angle(ui->screen_net_img_return, 0);
    lv_obj_set_pos(ui->screen_net_img_return, 20, 10);
    lv_obj_set_size(ui->screen_net_img_return, 22, 22);
    lv_obj_set_style_img_recolor_opa(ui->screen_net_img_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_net_img_return, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_net_img_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_net_img_return, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_net_net_name
    ui->screen_net_net_name = lv_spangroup_create(ui->screen_net);
    lv_spangroup_set_align(ui->screen_net_net_name, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_net_net_name, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_net_net_name, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_net_net_name_span = lv_spangroup_new_span(ui->screen_net_net_name);
    lv_span_set_text(ui->screen_net_net_name_span, "配网请扫码下载APP");
    lv_style_set_text_color(&ui->screen_net_net_name_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_net_net_name_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_net_net_name_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_net_net_name, 90, 10);
    lv_obj_set_size(ui->screen_net_net_name, 171, 20);

    //Write style state: LV_STATE_DEFAULT for &style_screen_net_net_name_main_main_default
    static lv_style_t style_screen_net_net_name_main_main_default;
    ui_init_style(&style_screen_net_net_name_main_main_default);

    lv_style_set_border_width(&style_screen_net_net_name_main_main_default, 0);
    lv_style_set_radius(&style_screen_net_net_name_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_net_net_name_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_net_net_name_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_net_net_name_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_net_net_name_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_net_net_name_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_net_net_name_main_main_default, 0);
    lv_obj_add_style(ui->screen_net_net_name, &style_screen_net_net_name_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_net_net_name);

    //Write codes screen_net_img_ios
    ui->screen_net_img_ios = lv_img_create(ui->screen_net);
    lv_obj_add_flag(ui->screen_net_img_ios, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_net_img_ios, &_ios_alpha_100x100);
    lv_img_set_pivot(ui->screen_net_img_ios, 50, 50);
    lv_img_set_angle(ui->screen_net_img_ios, 0);
    lv_obj_set_pos(ui->screen_net_img_ios, 115, 82);
    lv_obj_set_size(ui->screen_net_img_ios, 100, 100);

    //Write style for screen_net_img_ios, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_net_img_ios, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_net_img_ios, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_net_img_ios, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_net_img_ios, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //The custom code of screen_net.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_net);

}
