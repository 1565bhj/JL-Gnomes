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

#if CONFIG_LVGL_CONTENT_LIST_ENABLE

void setup_scr_screen_list(lv_ui *ui)
{
    //Write codes screen_list
    ui->screen_list = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_list, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_list, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_list, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_list, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_list, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_list, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_list_cont_1
    ui->screen_list_cont_1 = lv_obj_create(ui->screen_list);
    lv_obj_set_pos(ui->screen_list_cont_1, 296, 0);
    lv_obj_set_size(ui->screen_list_cont_1, 24, 240);
    lv_obj_set_scrollbar_mode(ui->screen_list_cont_1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(ui->screen_list_cont_1, LV_OBJ_FLAG_HIDDEN);

    //Write style for screen_list_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_radius(ui->screen_list_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_list_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_list_cont_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_list_cont_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_list_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_list_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_list_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_list_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_list_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_list_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_list_img_return
    ui->screen_list_img_return = lv_img_create(ui->screen_list);
    lv_obj_add_flag(ui->screen_list_img_return, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_list_img_return, &_return_alpha_22x22);
    lv_img_set_pivot(ui->screen_list_img_return, 50, 50);
    lv_img_set_angle(ui->screen_list_img_return, 0);
    lv_obj_set_pos(ui->screen_list_img_return, 20, 10);
    lv_obj_set_size(ui->screen_list_img_return, 22, 22);

    //Write style for screen_list_img_return, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_radius(ui->screen_list_img_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_list_img_return, true, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(ui->screen_list_img_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_list_img_return, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_list_label_name
    ui->screen_list_label_name = lv_label_create(ui->screen_list);
    lv_label_set_text(ui->screen_list_label_name, "分类");
    lv_label_set_long_mode(ui->screen_list_label_name, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_list_label_name, 50, 10);
    lv_obj_set_size(ui->screen_list_label_name, 117, 19);

    //Write style for screen_list_label_name, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_list_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_list_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_list_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_list_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_list_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_list_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_list_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_list_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_list_label_name, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_list_label_name, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_list_label_name, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_list_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_list_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_list_label_name, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_list_cont_content1
    ui->screen_list_cont_content1 = lv_obj_create(ui->screen_list);
    lv_obj_set_pos(ui->screen_list_cont_content1, 25, 40);
    lv_obj_set_size(ui->screen_list_cont_content1, 270, 195);
    lv_obj_set_scrollbar_mode(ui->screen_list_cont_content1, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_list_cont_content1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_list_cont_content1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_list_cont_content1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_list_cont_content1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_list_cont_content1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_list_cont_content1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_list_cont_content1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_list_cont_content1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_list_cont_content1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_list_study_english
    ui->screen_list_study_english = lv_img_create(ui->screen_list_cont_content1);
    lv_obj_add_flag(ui->screen_list_study_english, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_list_study_english, &_study_english_alpha_240x82);
    lv_img_set_pivot(ui->screen_list_study_english, 50, 50);
    lv_img_set_angle(ui->screen_list_study_english, 0);
    lv_obj_set_pos(ui->screen_list_study_english, 16, 8);
    lv_obj_set_size(ui->screen_list_study_english, 240, 82);

    //Write style for screen_list_study_english, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_radius(ui->screen_list_study_english, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_list_study_english, true, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(ui->screen_list_study_english, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_list_study_english, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_list_chinese_classics
    ui->screen_list_chinese_classics = lv_img_create(ui->screen_list_cont_content1);
    lv_obj_add_flag(ui->screen_list_chinese_classics, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_list_chinese_classics, &_study_chinese_alpha_240x82);
    lv_img_set_pivot(ui->screen_list_chinese_classics, 50, 50);
    lv_img_set_angle(ui->screen_list_chinese_classics, 0);
    lv_obj_set_pos(ui->screen_list_chinese_classics, 16, 111);
    lv_obj_set_size(ui->screen_list_chinese_classics, 240, 82);

    //Write style for screen_list_chinese_classics, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_radius(ui->screen_list_chinese_classics, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_list_chinese_classics, true, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(ui->screen_list_chinese_classics, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_list_chinese_classics, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_list_excellent_essay
    ui->screen_list_excellent_essay = lv_img_create(ui->screen_list_cont_content1);
    lv_obj_add_flag(ui->screen_list_excellent_essay, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_list_excellent_essay, &_excellent_essays_alpha_240x82);
    lv_img_set_pivot(ui->screen_list_excellent_essay, 50, 50);
    lv_img_set_angle(ui->screen_list_excellent_essay, 0);
    lv_obj_set_pos(ui->screen_list_excellent_essay, 16, 214);
    lv_obj_set_size(ui->screen_list_excellent_essay, 240, 82);

    //Write style for screen_list_excellent_essay, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_radius(ui->screen_list_excellent_essay, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_list_excellent_essay, true, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(ui->screen_list_excellent_essay, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_list_excellent_essay, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_list_study_QA
    ui->screen_list_study_QA = lv_img_create(ui->screen_list_cont_content1);
    lv_obj_add_flag(ui->screen_list_study_QA, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_list_study_QA, &_study_QA_alpha_240x82);
    lv_img_set_pivot(ui->screen_list_study_QA, 50, 50);
    lv_img_set_angle(ui->screen_list_study_QA, 0);
    lv_obj_set_pos(ui->screen_list_study_QA, 16, 317);
    lv_obj_set_size(ui->screen_list_study_QA, 240, 82);

    //Write style for screen_list_study_QA, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_radius(ui->screen_list_study_QA, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_list_study_QA, true, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(ui->screen_list_study_QA, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_list_study_QA, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_list_general_QA
    ui->screen_list_general_QA = lv_img_create(ui->screen_list_cont_content1);
    lv_obj_add_flag(ui->screen_list_general_QA, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_list_general_QA, &_general_QA_alpha_240x82);
    lv_img_set_pivot(ui->screen_list_general_QA, 50, 50);
    lv_img_set_angle(ui->screen_list_general_QA, 0);
    lv_obj_set_pos(ui->screen_list_general_QA, 16, 420);
    lv_obj_set_size(ui->screen_list_general_QA, 240, 82);

    //Write style for screen_list_general_QA, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_radius(ui->screen_list_general_QA, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_list_general_QA, true, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(ui->screen_list_general_QA, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_list_general_QA, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_list_cont_content2
    ui->screen_list_cont_content2 = lv_obj_create(ui->screen_list);
    lv_obj_set_pos(ui->screen_list_cont_content2, 405, 46);
    lv_obj_set_size(ui->screen_list_cont_content2, 295, 195);
    lv_obj_set_scrollbar_mode(ui->screen_list_cont_content2, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_list_cont_content2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_list_cont_content2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_list_cont_content2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_list_cont_content2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_list_cont_content2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_list_cont_content2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_list_cont_content2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_list_cont_content2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_list_cont_content2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_list_label_1
    ui->screen_list_label_1 = lv_label_create(ui->screen_list_cont_content2);
    lv_label_set_text(ui->screen_list_label_1, "唐诗三百首");
    lv_label_set_long_mode(ui->screen_list_label_1, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_list_label_1, 15, 11);
    lv_obj_set_size(ui->screen_list_label_1, 117, 51);

    //Write style for screen_list_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_list_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_list_label_1, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_list_label_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_list_label_1, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_list_label_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_list_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_list_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_list_label_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_list_label_1, 66, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_list_label_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_list_label_1, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_list_label_1, 14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_list_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_list_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_list_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_list_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_list_label_2
    ui->screen_list_label_2 = lv_label_create(ui->screen_list_cont_content2);
    lv_label_set_text(ui->screen_list_label_2, "唐诗三百首");
    lv_label_set_long_mode(ui->screen_list_label_2, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_list_label_2, 144, 11);
    lv_obj_set_size(ui->screen_list_label_2, 117, 51);

    //Write style for screen_list_label_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_list_label_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_list_label_2, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_list_label_2, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_list_label_2, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_list_label_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_list_label_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_list_label_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_list_label_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_list_label_2, 66, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_list_label_2, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_list_label_2, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_list_label_2, 14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_list_label_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_list_label_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_list_label_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_list_label_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_list_label_3
    ui->screen_list_label_3 = lv_label_create(ui->screen_list_cont_content2);
    lv_label_set_text(ui->screen_list_label_3, "唐诗三百首");
    lv_label_set_long_mode(ui->screen_list_label_3, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_list_label_3, 15, 75);
    lv_obj_set_size(ui->screen_list_label_3, 117, 51);

    //Write style for screen_list_label_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_list_label_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_list_label_3, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_list_label_3, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_list_label_3, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_list_label_3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_list_label_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_list_label_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_list_label_3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_list_label_3, 66, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_list_label_3, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_list_label_3, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_list_label_3, 14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_list_label_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_list_label_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_list_label_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_list_label_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_list_label_4
    ui->screen_list_label_4 = lv_label_create(ui->screen_list_cont_content2);
    lv_label_set_text(ui->screen_list_label_4, "唐诗三百首");
    lv_label_set_long_mode(ui->screen_list_label_4, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_list_label_4, 144, 75);
    lv_obj_set_size(ui->screen_list_label_4, 117, 51);

    //Write style for screen_list_label_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_list_label_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_list_label_4, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_list_label_4, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_list_label_4, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_list_label_4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_list_label_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_list_label_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_list_label_4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_list_label_4, 66, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_list_label_4, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_list_label_4, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_list_label_4, 14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_list_label_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_list_label_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_list_label_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_list_label_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_list_label_5
    ui->screen_list_label_5 = lv_label_create(ui->screen_list_cont_content2);
    lv_label_set_text(ui->screen_list_label_5, "唐诗三百首");
    lv_label_set_long_mode(ui->screen_list_label_5, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_list_label_5, 15, 139);
    lv_obj_set_size(ui->screen_list_label_5, 117, 51);

    //Write style for screen_list_label_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_list_label_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_list_label_5, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_list_label_5, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_list_label_5, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_list_label_5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_list_label_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_list_label_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_list_label_5, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_list_label_5, 66, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_list_label_5, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_list_label_5, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_list_label_5, 14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_list_label_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_list_label_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_list_label_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_list_label_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_list_label_6
    ui->screen_list_label_6 = lv_label_create(ui->screen_list_cont_content2);
    lv_label_set_text(ui->screen_list_label_6, "唐诗三百首");
    lv_label_set_long_mode(ui->screen_list_label_6, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_list_label_6, 144, 139);
    lv_obj_set_size(ui->screen_list_label_6, 117, 51);

    //Write style for screen_list_label_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_list_label_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_list_label_6, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_list_label_6, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_list_label_6, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_list_label_6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_list_label_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_list_label_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_list_label_6, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_list_label_6, 66, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_list_label_6, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_list_label_6, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_list_label_6, 18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_list_label_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_list_label_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_list_label_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_list_label_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Init events for screen.
    events_init_screen_list(ui);

    //Update current screen layout.
    lv_obj_update_layout(ui->screen_list);
}

#else

void setup_scr_screen_list(lv_ui *ui)
{
    (void)ui;
}

#endif /* CONFIG_LVGL_CONTENT_LIST_ENABLE */
