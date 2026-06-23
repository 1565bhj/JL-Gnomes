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

#if CONFIG_LVGL_AGENT_ENABLE

void setup_scr_screen_agent(lv_ui *ui)
{
    //Write codes screen_agent
    ui->screen_agent = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_agent, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_agent, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_agent, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_agent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_agent, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_agent, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->screen_agent, CONFIG_UI_RES_FILE_PATH"bg1.bin", LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->screen_agent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->screen_agent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_agent_img_return
    ui->screen_agent_img_return = lv_img_create(ui->screen_agent);
    lv_obj_add_flag(ui->screen_agent_img_return, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_agent_img_return, &_return_alpha_20x20);
    lv_img_set_pivot(ui->screen_agent_img_return, 50, 50);
    lv_img_set_angle(ui->screen_agent_img_return, 0);
    lv_obj_set_pos(ui->screen_agent_img_return, 12, 10);
    lv_obj_set_size(ui->screen_agent_img_return, 20, 20);

    //Write style for screen_agent_img_return, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_agent_img_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_agent_img_return, 128, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_agent_img_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_agent_img_return, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_agent_label_title
    ui->screen_agent_label_title = lv_label_create(ui->screen_agent);
    lv_label_set_text(ui->screen_agent_label_title, "选择智能体");
    lv_label_set_long_mode(ui->screen_agent_label_title, LV_LABEL_LONG_CLIP);
    lv_obj_set_pos(ui->screen_agent_label_title, 0, 8);
    lv_obj_set_size(ui->screen_agent_label_title, 320, 22);
    lv_obj_set_style_text_align(ui->screen_agent_label_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_agent_label_title, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_agent_label_title, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_screen_agent_label_title_main_main_default
    static lv_style_t style_screen_agent_label_title_main_main_default;
    ui_init_style(&style_screen_agent_label_title_main_main_default);

    lv_style_set_border_width(&style_screen_agent_label_title_main_main_default, 0);
    lv_style_set_radius(&style_screen_agent_label_title_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_agent_label_title_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_agent_label_title_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_agent_label_title_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_agent_label_title_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_agent_label_title_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_agent_label_title_main_main_default, 0);
    lv_obj_add_style(ui->screen_agent_label_title, &style_screen_agent_label_title_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_agent_cont_6
    ui->screen_agent_cont_6 = lv_obj_create(ui->screen_agent);
    lv_obj_set_pos(ui->screen_agent_cont_6, 222, 143);
    lv_obj_set_size(ui->screen_agent_cont_6, 75, 92);
    lv_obj_set_scrollbar_mode(ui->screen_agent_cont_6, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_agent_cont_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_agent_cont_6, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_agent_cont_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_agent_cont_6, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_agent_cont_6, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_agent_cont_6, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_agent_cont_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_agent_cont_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_agent_cont_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_agent_cont_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_agent_cont_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_agent_cont_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_agent_agent_6
    ui->screen_agent_agent_6 = lv_img_create(ui->screen_agent_cont_6);
    lv_obj_add_flag(ui->screen_agent_agent_6, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_agent_agent_6, &_agent_6_alpha_66x66);
    lv_img_set_pivot(ui->screen_agent_agent_6, 50, 50);
    lv_img_set_angle(ui->screen_agent_agent_6, 0);
    lv_obj_set_pos(ui->screen_agent_agent_6, 3, 2);
    lv_obj_set_size(ui->screen_agent_agent_6, 66, 66);

    //Write style for screen_agent_agent_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_agent_agent_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_agent_agent_6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_agent_agent_6, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_agent_agent_6, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_agent_label_name_6
    ui->screen_agent_label_name_6 = lv_label_create(ui->screen_agent_cont_6);
    lv_label_set_text(ui->screen_agent_label_name_6, "技术老师");
    lv_label_set_long_mode(ui->screen_agent_label_name_6, LV_LABEL_LONG_DOT);
    lv_obj_set_pos(ui->screen_agent_label_name_6, 0, 70);
    lv_obj_set_size(ui->screen_agent_label_name_6, 75, 22);
    lv_obj_set_style_text_align(ui->screen_agent_label_name_6, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_agent_label_name_6, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_agent_label_name_6, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_screen_agent_label_name_6_main_main_default
    static lv_style_t style_screen_agent_label_name_6_main_main_default;
    ui_init_style(&style_screen_agent_label_name_6_main_main_default);

    lv_style_set_border_width(&style_screen_agent_label_name_6_main_main_default, 0);
    lv_style_set_radius(&style_screen_agent_label_name_6_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_agent_label_name_6_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_agent_label_name_6_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_agent_label_name_6_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_agent_label_name_6_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_agent_label_name_6_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_agent_label_name_6_main_main_default, 0);
    lv_obj_add_style(ui->screen_agent_label_name_6, &style_screen_agent_label_name_6_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_agent_cont_5
    ui->screen_agent_cont_5 = lv_obj_create(ui->screen_agent);
    lv_obj_set_pos(ui->screen_agent_cont_5, 122, 143);
    lv_obj_set_size(ui->screen_agent_cont_5, 75, 92);
    lv_obj_set_scrollbar_mode(ui->screen_agent_cont_5, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_agent_cont_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_agent_cont_5, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_agent_cont_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_agent_cont_5, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_agent_cont_5, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_agent_cont_5, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_agent_cont_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_agent_cont_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_agent_cont_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_agent_cont_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_agent_cont_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_agent_cont_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_agent_agent_5
    ui->screen_agent_agent_5 = lv_img_create(ui->screen_agent_cont_5);
    lv_obj_add_flag(ui->screen_agent_agent_5, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_agent_agent_5, &_agent_5_alpha_66x66);
    lv_img_set_pivot(ui->screen_agent_agent_5, 50, 50);
    lv_img_set_angle(ui->screen_agent_agent_5, 0);
    lv_obj_set_pos(ui->screen_agent_agent_5, 3, 2);
    lv_obj_set_size(ui->screen_agent_agent_5, 66, 66);

    //Write style for screen_agent_agent_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_agent_agent_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_agent_agent_5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_agent_agent_5, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_agent_agent_5, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_agent_label_name_5
    ui->screen_agent_label_name_5 = lv_label_create(ui->screen_agent_cont_5);
    lv_label_set_text(ui->screen_agent_label_name_5, "化学老师");
    lv_label_set_long_mode(ui->screen_agent_label_name_5, LV_LABEL_LONG_DOT);
    lv_obj_set_pos(ui->screen_agent_label_name_5, 0, 70);
    lv_obj_set_size(ui->screen_agent_label_name_5, 75, 22);
    lv_obj_set_style_text_align(ui->screen_agent_label_name_5, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_agent_label_name_5, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_agent_label_name_5, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_screen_agent_label_name_5_main_main_default
    static lv_style_t style_screen_agent_label_name_5_main_main_default;
    ui_init_style(&style_screen_agent_label_name_5_main_main_default);

    lv_style_set_border_width(&style_screen_agent_label_name_5_main_main_default, 0);
    lv_style_set_radius(&style_screen_agent_label_name_5_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_agent_label_name_5_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_agent_label_name_5_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_agent_label_name_5_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_agent_label_name_5_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_agent_label_name_5_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_agent_label_name_5_main_main_default, 0);
    lv_obj_add_style(ui->screen_agent_label_name_5, &style_screen_agent_label_name_5_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_agent_cont_4
    ui->screen_agent_cont_4 = lv_obj_create(ui->screen_agent);
    lv_obj_set_pos(ui->screen_agent_cont_4, 23, 143);
    lv_obj_set_size(ui->screen_agent_cont_4, 75, 92);
    lv_obj_set_scrollbar_mode(ui->screen_agent_cont_4, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_agent_cont_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_agent_cont_4, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_agent_cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_agent_cont_4, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_agent_cont_4, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_agent_cont_4, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_agent_cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_agent_cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_agent_cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_agent_cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_agent_cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_agent_cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_agent_agent_4
    ui->screen_agent_agent_4 = lv_img_create(ui->screen_agent_cont_4);
    lv_obj_add_flag(ui->screen_agent_agent_4, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_agent_agent_4, &_agent_4_alpha_66x66);
    lv_img_set_pivot(ui->screen_agent_agent_4, 50, 50);
    lv_img_set_angle(ui->screen_agent_agent_4, 0);
    lv_obj_set_pos(ui->screen_agent_agent_4, 3, 3);
    lv_obj_set_size(ui->screen_agent_agent_4, 66, 66);

    //Write style for screen_agent_agent_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_agent_agent_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_agent_agent_4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_agent_agent_4, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_agent_agent_4, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_agent_label_name_4
    ui->screen_agent_label_name_4 = lv_label_create(ui->screen_agent_cont_4);
    lv_label_set_text(ui->screen_agent_label_name_4, "体育老师");
    lv_label_set_long_mode(ui->screen_agent_label_name_4, LV_LABEL_LONG_DOT);
    lv_obj_set_pos(ui->screen_agent_label_name_4, 0, 70);
    lv_obj_set_size(ui->screen_agent_label_name_4, 75, 22);
    lv_obj_set_style_text_align(ui->screen_agent_label_name_4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_agent_label_name_4, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_agent_label_name_4, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_screen_agent_label_name_4_main_main_default
    static lv_style_t style_screen_agent_label_name_4_main_main_default;
    ui_init_style(&style_screen_agent_label_name_4_main_main_default);

    lv_style_set_border_width(&style_screen_agent_label_name_4_main_main_default, 0);
    lv_style_set_radius(&style_screen_agent_label_name_4_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_agent_label_name_4_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_agent_label_name_4_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_agent_label_name_4_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_agent_label_name_4_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_agent_label_name_4_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_agent_label_name_4_main_main_default, 0);
    lv_obj_add_style(ui->screen_agent_label_name_4, &style_screen_agent_label_name_4_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_agent_cont_3
    ui->screen_agent_cont_3 = lv_obj_create(ui->screen_agent);
    lv_obj_set_pos(ui->screen_agent_cont_3, 222, 44);
    lv_obj_set_size(ui->screen_agent_cont_3, 75, 92);
    lv_obj_set_scrollbar_mode(ui->screen_agent_cont_3, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_agent_cont_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_agent_cont_3, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_agent_cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_agent_cont_3, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_agent_cont_3, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_agent_cont_3, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_agent_cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_agent_cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_agent_cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_agent_cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_agent_cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_agent_cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_agent_agent_3
    ui->screen_agent_agent_3 = lv_img_create(ui->screen_agent_cont_3);
    lv_obj_add_flag(ui->screen_agent_agent_3, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_agent_agent_3, &_agent_3_alpha_66x66);
    lv_img_set_pivot(ui->screen_agent_agent_3, 50, 50);
    lv_img_set_angle(ui->screen_agent_agent_3, 0);
    lv_obj_set_pos(ui->screen_agent_agent_3, 3, 2);
    lv_obj_set_size(ui->screen_agent_agent_3, 66, 66);

    //Write style for screen_agent_agent_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_agent_agent_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_agent_agent_3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_agent_agent_3, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_agent_agent_3, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_agent_label_name_3
    ui->screen_agent_label_name_3 = lv_label_create(ui->screen_agent_cont_3);
    lv_label_set_text(ui->screen_agent_label_name_3, "英语老师");
    lv_label_set_long_mode(ui->screen_agent_label_name_3, LV_LABEL_LONG_DOT);
    lv_obj_set_pos(ui->screen_agent_label_name_3, 0, 70);
    lv_obj_set_size(ui->screen_agent_label_name_3, 75, 22);
    lv_obj_set_style_text_align(ui->screen_agent_label_name_3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_agent_label_name_3, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_agent_label_name_3, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_screen_agent_label_name_3_main_main_default
    static lv_style_t style_screen_agent_label_name_3_main_main_default;
    ui_init_style(&style_screen_agent_label_name_3_main_main_default);

    lv_style_set_border_width(&style_screen_agent_label_name_3_main_main_default, 0);
    lv_style_set_radius(&style_screen_agent_label_name_3_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_agent_label_name_3_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_agent_label_name_3_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_agent_label_name_3_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_agent_label_name_3_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_agent_label_name_3_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_agent_label_name_3_main_main_default, 0);
    lv_obj_add_style(ui->screen_agent_label_name_3, &style_screen_agent_label_name_3_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_agent_cont_2
    ui->screen_agent_cont_2 = lv_obj_create(ui->screen_agent);
    lv_obj_set_pos(ui->screen_agent_cont_2, 122, 44);
    lv_obj_set_size(ui->screen_agent_cont_2, 75, 92);
    lv_obj_set_scrollbar_mode(ui->screen_agent_cont_2, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_agent_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_agent_cont_2, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_agent_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_agent_cont_2, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_agent_cont_2, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_agent_cont_2, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_agent_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_agent_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_agent_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_agent_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_agent_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_agent_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_agent_agent_2
    ui->screen_agent_agent_2 = lv_img_create(ui->screen_agent_cont_2);
    lv_obj_add_flag(ui->screen_agent_agent_2, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_agent_agent_2, &_agent_2_alpha_66x66);
    lv_img_set_pivot(ui->screen_agent_agent_2, 50, 50);
    lv_img_set_angle(ui->screen_agent_agent_2, 0);
    lv_obj_set_pos(ui->screen_agent_agent_2, 3, 2);
    lv_obj_set_size(ui->screen_agent_agent_2, 66, 66);

    //Write style for screen_agent_agent_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_agent_agent_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_agent_agent_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_agent_agent_2, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_agent_agent_2, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_agent_label_name_2
    ui->screen_agent_label_name_2 = lv_label_create(ui->screen_agent_cont_2);
    lv_label_set_text(ui->screen_agent_label_name_2, "数学老师");
    lv_label_set_long_mode(ui->screen_agent_label_name_2, LV_LABEL_LONG_DOT);
    lv_obj_set_pos(ui->screen_agent_label_name_2, 0, 70);
    lv_obj_set_size(ui->screen_agent_label_name_2, 75, 22);
    lv_obj_set_style_text_align(ui->screen_agent_label_name_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_agent_label_name_2, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_agent_label_name_2, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_screen_agent_label_name_2_main_main_default
    static lv_style_t style_screen_agent_label_name_2_main_main_default;
    ui_init_style(&style_screen_agent_label_name_2_main_main_default);

    lv_style_set_border_width(&style_screen_agent_label_name_2_main_main_default, 0);
    lv_style_set_radius(&style_screen_agent_label_name_2_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_agent_label_name_2_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_agent_label_name_2_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_agent_label_name_2_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_agent_label_name_2_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_agent_label_name_2_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_agent_label_name_2_main_main_default, 0);
    lv_obj_add_style(ui->screen_agent_label_name_2, &style_screen_agent_label_name_2_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_agent_cont_1
    ui->screen_agent_cont_1 = lv_obj_create(ui->screen_agent);
    lv_obj_set_pos(ui->screen_agent_cont_1, 23, 44);
    lv_obj_set_size(ui->screen_agent_cont_1, 75, 92);
    lv_obj_set_scrollbar_mode(ui->screen_agent_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_agent_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_agent_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_agent_cont_1, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_agent_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_agent_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_agent_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_agent_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_agent_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_agent_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_agent_agent_1
    ui->screen_agent_agent_1 = lv_img_create(ui->screen_agent_cont_1);
    lv_obj_add_flag(ui->screen_agent_agent_1, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_agent_agent_1, &_agent_1_alpha_66x66);
    lv_img_set_pivot(ui->screen_agent_agent_1, 50, 50);
    lv_img_set_angle(ui->screen_agent_agent_1, 0);
    lv_obj_set_pos(ui->screen_agent_agent_1, 3, 2);
    lv_obj_set_size(ui->screen_agent_agent_1, 66, 66);

    //Write style for screen_agent_agent_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_agent_agent_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_agent_agent_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_agent_agent_1, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_agent_agent_1, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_agent_label_name_1
    ui->screen_agent_label_name_1 = lv_label_create(ui->screen_agent_cont_1);
    lv_label_set_text(ui->screen_agent_label_name_1, "语文老师");
    lv_label_set_long_mode(ui->screen_agent_label_name_1, LV_LABEL_LONG_DOT);
    lv_obj_set_pos(ui->screen_agent_label_name_1, 0, 70);
    lv_obj_set_size(ui->screen_agent_label_name_1, 75, 22);
    lv_obj_set_style_text_align(ui->screen_agent_label_name_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_agent_label_name_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_agent_label_name_1, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_screen_agent_label_name_1_main_main_default
    static lv_style_t style_screen_agent_label_name_1_main_main_default;
    ui_init_style(&style_screen_agent_label_name_1_main_main_default);

    lv_style_set_border_width(&style_screen_agent_label_name_1_main_main_default, 0);
    lv_style_set_radius(&style_screen_agent_label_name_1_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_agent_label_name_1_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_agent_label_name_1_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_agent_label_name_1_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_agent_label_name_1_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_agent_label_name_1_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_agent_label_name_1_main_main_default, 0);
    lv_obj_add_style(ui->screen_agent_label_name_1, &style_screen_agent_label_name_1_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_agent_img_choose
    ui->screen_agent_img_choose = lv_img_create(ui->screen_agent);
    lv_obj_add_flag(ui->screen_agent_img_choose, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_agent_img_choose, &_choose_alpha_66x66);
    lv_img_set_pivot(ui->screen_agent_img_choose, 50, 50);
    lv_img_set_angle(ui->screen_agent_img_choose, 0);
    lv_obj_set_pos(ui->screen_agent_img_choose, 458, 30);
    lv_obj_set_size(ui->screen_agent_img_choose, 66, 66);

    //Write style for screen_agent_img_choose, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_agent_img_choose, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_agent_img_choose, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_agent_img_choose, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_agent_img_choose, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //The custom code of screen_agent.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_agent);

    //Init events for screen.
    events_init_screen_agent(ui);
}

#else

void setup_scr_screen_agent(lv_ui *ui)
{
    (void)ui;
}

#endif
