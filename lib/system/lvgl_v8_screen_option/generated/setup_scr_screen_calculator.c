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
#include "lvgl_develop.h"

#if CONFIG_LVGL_CALCULATOR_ENABLE

void setup_scr_screen_calculator(lv_ui *ui)
{
    //Write codes screen_calculator
    ui->screen_calculator = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_calculator, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_calculator, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_calculator, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_calculator, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_calculator, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_calculator, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_calculator_btnm_calculate
    ui->screen_calculator_btnm_calculate = lv_btnmatrix_create(ui->screen_calculator);
    static const char *screen_calculator_btnm_calculate_text_map[] = {"C", "÷", "×", "←", "\n", "7", "8", "9", "－", "\n", "4", "5", "6", "+", "\n", "1", "2", "3", "=", "\n", "%", "0", ".", "=", "",};
    lv_btnmatrix_set_map(ui->screen_calculator_btnm_calculate, screen_calculator_btnm_calculate_text_map);
    lv_obj_set_pos(ui->screen_calculator_btnm_calculate, 1, 52);
    lv_obj_set_size(ui->screen_calculator_btnm_calculate, 320, 189);

    //Write style for screen_calculator_btnm_calculate, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_calculator_btnm_calculate, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_calculator_btnm_calculate, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_calculator_btnm_calculate, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_calculator_btnm_calculate, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_calculator_btnm_calculate, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_calculator_btnm_calculate, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_calculator_btnm_calculate, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui->screen_calculator_btnm_calculate, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui->screen_calculator_btnm_calculate, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style for screen_calculator_btnm_calculate, Part: LV_PART_ITEMS, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_calculator_btnm_calculate, 1, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_calculator_btnm_calculate, 161, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_calculator_btnm_calculate, lv_color_hex(0xffffff), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_calculator_btnm_calculate, LV_BORDER_SIDE_FULL, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_calculator_btnm_calculate, lv_color_hex(0xffffff), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_calculator_btnm_calculate, &lv_font_MiSansDemibold_18, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_calculator_btnm_calculate, 255, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_calculator_btnm_calculate, 5, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_calculator_btnm_calculate, 54, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_calculator_btnm_calculate, lv_color_hex(0xffffff), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_calculator_btnm_calculate, LV_GRAD_DIR_NONE, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_calculator_btnm_calculate, 4, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui->screen_calculator_btnm_calculate, lv_color_hex(0xffffff), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui->screen_calculator_btnm_calculate, 117, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui->screen_calculator_btnm_calculate, 0, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(ui->screen_calculator_btnm_calculate, 0, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(ui->screen_calculator_btnm_calculate, 0, LV_PART_ITEMS | LV_STATE_DEFAULT);

    //Write codes screen_calculator_label_calculate
    ui->screen_calculator_label_calculate = lv_label_create(ui->screen_calculator);
    lv_label_set_text(ui->screen_calculator_label_calculate, "0");
    lv_label_set_long_mode(ui->screen_calculator_label_calculate, LV_LABEL_LONG_CLIP);
    lv_obj_set_pos(ui->screen_calculator_label_calculate, 15, 20);
    lv_obj_set_size(ui->screen_calculator_label_calculate, 292, 26);

    //Write style for screen_calculator_label_calculate, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_calculator_label_calculate, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_calculator_label_calculate, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_calculator_label_calculate, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_calculator_label_calculate, &lv_font_MiSansDemibold_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_calculator_label_calculate, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_calculator_label_calculate, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_calculator_label_calculate, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_calculator_label_calculate, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_calculator_label_calculate, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_calculator_label_calculate, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_calculator_label_calculate, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_calculator_label_calculate, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_calculator_label_calculate, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_calculator_label_calculate, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_calculator_img_return
    ui->screen_calculator_img_return = lv_img_create(ui->screen_calculator);
    lv_obj_add_flag(ui->screen_calculator_img_return, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_calculator_img_return, &_return_alpha_20x20);
    lv_img_set_pivot(ui->screen_calculator_img_return, 50, 50);
    lv_img_set_angle(ui->screen_calculator_img_return, 0);
    lv_obj_set_pos(ui->screen_calculator_img_return, 12, 10);
    lv_obj_set_size(ui->screen_calculator_img_return, 20, 20);

    //Write style for screen_calculator_img_return, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_calculator_img_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_calculator_img_return, 128, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_calculator_img_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_calculator_img_return, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //The custom code of screen_calculator.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_calculator);

    //Init events for screen.
    events_init_screen_calculator(ui);
}

#else

void setup_scr_screen_calculator(lv_ui *ui)
{
    (void)ui;
}

#endif
