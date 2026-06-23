/*
* Copyright 2025 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and be bound by, such license terms.  If you do not agree to be bound by the applicable license
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
    ui->screen_option = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_option, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_option, LV_SCROLLBAR_MODE_OFF);

    lv_obj_set_style_bg_opa(ui->screen_option, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->screen_option, CONFIG_UI_RES_FILE_PATH"bg1.bin", LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->screen_option, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->screen_option, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui->screen_option_img_return = lv_img_create(ui->screen_option);
    lv_obj_add_flag(ui->screen_option_img_return, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_option_img_return, &_return_alpha_22x22);
    lv_img_set_pivot(ui->screen_option_img_return, 11, 11);
    lv_img_set_angle(ui->screen_option_img_return, 0);
    lv_obj_set_pos(ui->screen_option_img_return, 20, 13);
    lv_obj_set_size(ui->screen_option_img_return, 22, 22);

    lv_obj_set_style_img_recolor_opa(ui->screen_option_img_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_option_img_return, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_option_img_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_option_img_return, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui->screen_option_op_name = lv_label_create(ui->screen_option);
    lv_label_set_text(ui->screen_option_op_name, "名称");
    lv_label_set_long_mode(ui->screen_option_op_name, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_option_op_name, 116, 15);
    lv_obj_set_size(ui->screen_option_op_name, 84, 22);

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

    lv_obj_update_layout(ui->screen_option);

    events_init_screen_option(ui);
}
