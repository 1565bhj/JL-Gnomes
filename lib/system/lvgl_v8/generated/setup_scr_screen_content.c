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

void setup_scr_screen_content(lv_ui *ui)
{
    //Write codes screen_content
    ui->screen_content = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_content, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_content, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_content, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_content, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_content, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_content, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_content_label_name
    ui->screen_content_label_name = lv_label_create(ui->screen_content);
    lv_label_set_text(ui->screen_content_label_name, "三年级文言文");
    lv_label_set_long_mode(ui->screen_content_label_name, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_content_label_name, 0, 10);
    lv_obj_set_size(ui->screen_content_label_name, 296, 20);

    //Write style for screen_content_label_name, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_content_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_content_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_content_label_name, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_content_label_name, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_content_label_name, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_content_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_content_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_content_label_name, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_content_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_content_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_content_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_content_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_content_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_content_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_content_img_return
    ui->screen_content_img_return = lv_img_create(ui->screen_content);
    lv_obj_add_flag(ui->screen_content_img_return, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_content_img_return, &_return_alpha_22x22);
    lv_img_set_pivot(ui->screen_content_img_return, 50, 50);
    lv_img_set_angle(ui->screen_content_img_return, 0);
    lv_obj_set_pos(ui->screen_content_img_return, 20, 10);
    lv_obj_set_size(ui->screen_content_img_return, 22, 22);

    //Write style for screen_content_img_return, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_content_img_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_content_img_return, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_content_img_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_content_img_return, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_content_cont_1
    ui->screen_content_cont_1 = lv_obj_create(ui->screen_content);
    lv_obj_set_pos(ui->screen_content_cont_1, 296, 0);
    lv_obj_set_size(ui->screen_content_cont_1, 24, 240);
    lv_obj_set_scrollbar_mode(ui->screen_content_cont_1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(ui->screen_content_cont_1, LV_OBJ_FLAG_HIDDEN);

    //Write style for screen_content_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_content_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_content_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_content_cont_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_content_cont_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_content_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_content_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_content_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_content_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_content_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_content_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_content_label_last_page
    ui->screen_content_label_last_page = lv_label_create(ui->screen_content);
    lv_label_set_text(ui->screen_content_label_last_page, "上一页");
    lv_label_set_long_mode(ui->screen_content_label_last_page, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_content_label_last_page, 54, 8);
    lv_obj_set_size(ui->screen_content_label_last_page, 65, 26);

    //Write style for screen_content_label_last_page, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_content_label_last_page, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_content_label_last_page, 128, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_content_label_last_page, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_content_label_last_page, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_content_label_last_page, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_content_label_last_page, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_content_label_last_page, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_content_label_last_page, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_content_label_last_page, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_content_label_last_page, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_content_label_last_page, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_content_label_last_page, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_content_label_last_page, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_content_label_last_page, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_content_label_last_page, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_content_label_last_page, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_content_label_last_page, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    /* 默认隐藏，只在 screen_content_detail（最终内容页）显示时由 custom 代码 unhide */
    lv_obj_add_flag(ui->screen_content_label_last_page, LV_OBJ_FLAG_HIDDEN);

    //Write codes screen_content_label_next_page
    ui->screen_content_label_next_page = lv_label_create(ui->screen_content);
    lv_label_set_text(ui->screen_content_label_next_page, "下一页");
    lv_label_set_long_mode(ui->screen_content_label_next_page, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_content_label_next_page, 239, 8);
    lv_obj_set_size(ui->screen_content_label_next_page, 65, 26);

    //Write style for screen_content_label_next_page, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_content_label_next_page, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_content_label_next_page, 128, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_content_label_next_page, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_content_label_next_page, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_content_label_next_page, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_content_label_next_page, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_content_label_next_page, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_content_label_next_page, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_content_label_next_page, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_content_label_next_page, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_content_label_next_page, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_content_label_next_page, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_content_label_next_page, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_content_label_next_page, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_content_label_next_page, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_content_label_next_page, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_content_label_next_page, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    /* 默认隐藏，只在 screen_content_detail（最终内容页）显示时由 custom 代码 unhide */
    lv_obj_add_flag(ui->screen_content_label_next_page, LV_OBJ_FLAG_HIDDEN);

    //Write codes screen_content_label_repeat
    ui->screen_content_label_repeat = lv_label_create(ui->screen_content);
    lv_label_set_text(ui->screen_content_label_repeat, "重读");
    lv_label_set_long_mode(ui->screen_content_label_repeat, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_content_label_repeat, 156, 8);
    lv_obj_set_size(ui->screen_content_label_repeat, 46, 26);

    //Write style for screen_content_label_repeat, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_content_label_repeat, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_content_label_repeat, 128, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_content_label_repeat, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_content_label_repeat, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_content_label_repeat, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_content_label_repeat, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_content_label_repeat, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_content_label_repeat, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_content_label_repeat, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_content_label_repeat, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_content_label_repeat, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_content_label_repeat, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_content_label_repeat, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_content_label_repeat, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_content_label_repeat, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_content_label_repeat, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_content_label_repeat, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    /* 默认隐藏，只在 screen_content_detail（最终内容页）显示时由 custom 代码 unhide */
    lv_obj_add_flag(ui->screen_content_label_repeat, LV_OBJ_FLAG_HIDDEN);

    //Update current screen layout.
    lv_obj_update_layout(ui->screen_content);

    events_init_screen_content(ui);
}

#else

void setup_scr_screen_content(lv_ui *ui)
{
    (void)ui;
}

#endif /* CONFIG_LVGL_CONTENT_LIST_ENABLE */
