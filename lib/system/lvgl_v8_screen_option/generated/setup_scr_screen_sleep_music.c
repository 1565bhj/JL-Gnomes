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

#if CONFIG_LVGL_SLEEP_MUSIC_ENABLE

#if GUIDER_UI_SLEEP_MUSIC_DURATION_LABELS
/** 睡眠音乐屏顶部「30分钟 / 90分钟」label，与本 setup 同属一文件 */
static void setup_scr_screen_sleep_music_create_duration_labels(lv_ui *ui)
{
    lv_obj_t *scr;

    if (ui == NULL || ui->screen_sleep_music == NULL) {
        return;
    }
    scr = ui->screen_sleep_music;

    ui->screen_sleep_music_label_30min = lv_label_create(scr);
    lv_label_set_text(ui->screen_sleep_music_label_30min, "30分钟");
    lv_label_set_long_mode(ui->screen_sleep_music_label_30min, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_sleep_music_label_30min, 158, 13);
    lv_obj_set_size(ui->screen_sleep_music_label_30min, 63, 24);
    lv_obj_set_style_border_width(ui->screen_sleep_music_label_30min, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_sleep_music_label_30min, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_sleep_music_label_30min, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_sleep_music_label_30min, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_sleep_music_label_30min, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_sleep_music_label_30min, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_sleep_music_label_30min, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_sleep_music_label_30min, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_sleep_music_label_30min, 65, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_sleep_music_label_30min, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_sleep_music_label_30min, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_sleep_music_label_30min, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_sleep_music_label_30min, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_sleep_music_label_30min, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_sleep_music_label_30min, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_sleep_music_label_30min, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui->screen_sleep_music_label_90min = lv_label_create(scr);
    lv_label_set_text(ui->screen_sleep_music_label_90min, "90分钟");
    lv_label_set_long_mode(ui->screen_sleep_music_label_90min, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_sleep_music_label_90min, 232, 13);
    lv_obj_set_size(ui->screen_sleep_music_label_90min, 63, 24);
    lv_obj_set_style_border_width(ui->screen_sleep_music_label_90min, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_sleep_music_label_90min, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_sleep_music_label_90min, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_sleep_music_label_90min, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_sleep_music_label_90min, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_sleep_music_label_90min, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_sleep_music_label_90min, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_sleep_music_label_90min, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_sleep_music_label_90min, 65, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_sleep_music_label_90min, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_sleep_music_label_90min, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_sleep_music_label_90min, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_sleep_music_label_90min, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_sleep_music_label_90min, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_sleep_music_label_90min, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_sleep_music_label_90min, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
}
#endif

void setup_scr_screen_sleep_music(lv_ui *ui)
{
    //Write codes screen_sleep_music
    ui->screen_sleep_music = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_sleep_music, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_sleep_music, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_sleep_music, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_sleep_music, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_sleep_music, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_sleep_music, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_sleep_music_span_name
    ui->screen_sleep_music_span_name = lv_spangroup_create(ui->screen_sleep_music);
    lv_spangroup_set_align(ui->screen_sleep_music_span_name, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_sleep_music_span_name, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_sleep_music_span_name, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_sleep_music_span_name_span = lv_spangroup_new_span(ui->screen_sleep_music_span_name);
    lv_span_set_text(ui->screen_sleep_music_span_name_span, "睡眠音乐");
    lv_style_set_text_color(&ui->screen_sleep_music_span_name_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_sleep_music_span_name_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_sleep_music_span_name_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_sleep_music_span_name, 47, 11);
    lv_obj_set_size(ui->screen_sleep_music_span_name, 85, 17);

    //Write style state: LV_STATE_DEFAULT for &style_screen_sleep_music_span_name_main_main_default
    static lv_style_t style_screen_sleep_music_span_name_main_main_default;
    ui_init_style(&style_screen_sleep_music_span_name_main_main_default);

    lv_style_set_border_width(&style_screen_sleep_music_span_name_main_main_default, 0);
    lv_style_set_radius(&style_screen_sleep_music_span_name_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_sleep_music_span_name_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_sleep_music_span_name_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_sleep_music_span_name_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_sleep_music_span_name_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_sleep_music_span_name_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_sleep_music_span_name_main_main_default, 0);
    lv_obj_add_style(ui->screen_sleep_music_span_name, &style_screen_sleep_music_span_name_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_sleep_music_span_name);

    //Write codes screen_sleep_music_img_return
    ui->screen_sleep_music_img_return = lv_img_create(ui->screen_sleep_music);
    lv_obj_add_flag(ui->screen_sleep_music_img_return, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_sleep_music_img_return, &_return_alpha_22x22);
    lv_img_set_pivot(ui->screen_sleep_music_img_return, 50, 50);
    lv_img_set_angle(ui->screen_sleep_music_img_return, 0);
    lv_obj_set_pos(ui->screen_sleep_music_img_return, 20, 10);
    lv_obj_set_size(ui->screen_sleep_music_img_return, 22, 22);

    //Write style for screen_sleep_music_img_return, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_sleep_music_img_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_sleep_music_img_return, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_sleep_music_img_return, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_sleep_music_img_return, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_sleep_music_cont
    ui->screen_sleep_music_cont = lv_obj_create(ui->screen_sleep_music);
    lv_obj_set_pos(ui->screen_sleep_music_cont, 16, 44);
    lv_obj_set_size(ui->screen_sleep_music_cont, 300, 200);
    lv_obj_set_scrollbar_mode(ui->screen_sleep_music_cont, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_sleep_music_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_sleep_music_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_sleep_music_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_sleep_music_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_sleep_music_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_sleep_music_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_sleep_music_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_sleep_music_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_sleep_music_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_sleep_music_label_5
    ui->screen_sleep_music_label_5 = lv_label_create(ui->screen_sleep_music_cont);
    lv_label_set_text(ui->screen_sleep_music_label_5, "雨声");
    lv_label_set_long_mode(ui->screen_sleep_music_label_5, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_sleep_music_label_5, 22, 103);
    lv_obj_set_size(ui->screen_sleep_music_label_5, 110, 40);

    //Write style for screen_sleep_music_label_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_radius(ui->screen_sleep_music_label_5, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_sleep_music_label_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_sleep_music_label_5, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_sleep_music_label_5, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_sleep_music_label_5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_sleep_music_label_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_sleep_music_label_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_sleep_music_label_5, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_sleep_music_label_5, 65, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_sleep_music_label_5, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_sleep_music_label_5, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_sleep_music_label_5, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_sleep_music_label_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_sleep_music_label_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_sleep_music_label_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_sleep_music_label_5, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_sleep_music_label_6
    ui->screen_sleep_music_label_6 = lv_label_create(ui->screen_sleep_music_cont);
    lv_label_set_text(ui->screen_sleep_music_label_6, "海浪");
    lv_label_set_long_mode(ui->screen_sleep_music_label_6, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_sleep_music_label_6, 153, 103);
    lv_obj_set_size(ui->screen_sleep_music_label_6, 110, 40);

    //Write style for screen_sleep_music_label_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_sleep_music_label_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_sleep_music_label_6, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_sleep_music_label_6, 65, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_sleep_music_label_6, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_sleep_music_label_6, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_sleep_music_label_6, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_sleep_music_label_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_sleep_music_label_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_sleep_music_label_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_sleep_music_label_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_sleep_music_label_6, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_sleep_music_label_6, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_sleep_music_label_6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_sleep_music_label_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_sleep_music_label_6, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_sleep_music_label_6, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_sleep_music_label_7
    ui->screen_sleep_music_label_7 = lv_label_create(ui->screen_sleep_music_cont);
    lv_label_set_text(ui->screen_sleep_music_label_7, "电波");
    lv_label_set_long_mode(ui->screen_sleep_music_label_7, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_sleep_music_label_7, 22, 151);
    lv_obj_set_size(ui->screen_sleep_music_label_7, 110, 40);

    //Write style for screen_sleep_music_label_7, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_sleep_music_label_7, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_sleep_music_label_7, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_sleep_music_label_7, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_sleep_music_label_7, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_sleep_music_label_7, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_sleep_music_label_7, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_sleep_music_label_7, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_sleep_music_label_7, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_sleep_music_label_7, 65, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_sleep_music_label_7, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_sleep_music_label_7, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_sleep_music_label_7, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_sleep_music_label_7, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_sleep_music_label_7, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_sleep_music_label_7, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_sleep_music_label_7, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_sleep_music_label_8
    ui->screen_sleep_music_label_8 = lv_label_create(ui->screen_sleep_music_cont);
    lv_label_set_text(ui->screen_sleep_music_label_8, "泉水");
    lv_label_set_long_mode(ui->screen_sleep_music_label_8, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_sleep_music_label_8, 153, 151);
    lv_obj_set_size(ui->screen_sleep_music_label_8, 110, 40);

    //Write style for screen_sleep_music_label_8, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_sleep_music_label_8, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_sleep_music_label_8, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_sleep_music_label_8, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_sleep_music_label_8, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_sleep_music_label_8, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_sleep_music_label_8, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_sleep_music_label_8, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_sleep_music_label_8, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_sleep_music_label_8, 65, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_sleep_music_label_8, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_sleep_music_label_8, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_sleep_music_label_8, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_sleep_music_label_8, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_sleep_music_label_8, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_sleep_music_label_8, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_sleep_music_label_8, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_sleep_music_label_4
    ui->screen_sleep_music_label_4 = lv_label_create(ui->screen_sleep_music_cont);
    lv_label_set_text(ui->screen_sleep_music_label_4, "沙滩");
    lv_label_set_long_mode(ui->screen_sleep_music_label_4, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_sleep_music_label_4, 153, 54);
    lv_obj_set_size(ui->screen_sleep_music_label_4, 110, 40);

    //Write style for screen_sleep_music_label_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_sleep_music_label_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_sleep_music_label_4, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_sleep_music_label_4, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_sleep_music_label_4, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_sleep_music_label_4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_sleep_music_label_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_sleep_music_label_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_sleep_music_label_4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_sleep_music_label_4, 65, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_sleep_music_label_4, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_sleep_music_label_4, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_sleep_music_label_4, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_sleep_music_label_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_sleep_music_label_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_sleep_music_label_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_sleep_music_label_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_sleep_music_label_3
    ui->screen_sleep_music_label_3 = lv_label_create(ui->screen_sleep_music_cont);
    lv_label_set_text(ui->screen_sleep_music_label_3, "雪声");
    lv_label_set_long_mode(ui->screen_sleep_music_label_3, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_sleep_music_label_3, 22, 55);
    lv_obj_set_size(ui->screen_sleep_music_label_3, 110, 40);

    //Write style for screen_sleep_music_label_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_sleep_music_label_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_sleep_music_label_3, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_sleep_music_label_3, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_sleep_music_label_3, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_sleep_music_label_3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_sleep_music_label_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_sleep_music_label_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_sleep_music_label_3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_sleep_music_label_3, 65, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_sleep_music_label_3, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_sleep_music_label_3, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_sleep_music_label_3, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_sleep_music_label_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_sleep_music_label_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_sleep_music_label_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_sleep_music_label_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_sleep_music_label_2
    ui->screen_sleep_music_label_2 = lv_label_create(ui->screen_sleep_music_cont);
    lv_label_set_text(ui->screen_sleep_music_label_2, "风声");
    lv_label_set_long_mode(ui->screen_sleep_music_label_2, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_sleep_music_label_2, 153, 6);
    lv_obj_set_size(ui->screen_sleep_music_label_2, 110, 40);

    //Write style for screen_sleep_music_label_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_sleep_music_label_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_sleep_music_label_2, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_sleep_music_label_2, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_sleep_music_label_2, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_sleep_music_label_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_sleep_music_label_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_sleep_music_label_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_sleep_music_label_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_sleep_music_label_2, 65, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_sleep_music_label_2, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_sleep_music_label_2, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_sleep_music_label_2, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_sleep_music_label_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_sleep_music_label_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_sleep_music_label_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_sleep_music_label_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_sleep_music_label_1
    ui->screen_sleep_music_label_1 = lv_label_create(ui->screen_sleep_music_cont);
    lv_label_set_text(ui->screen_sleep_music_label_1, "空灵");
    lv_label_set_long_mode(ui->screen_sleep_music_label_1, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_sleep_music_label_1, 22, 6);
    lv_obj_set_size(ui->screen_sleep_music_label_1, 110, 40);

    //Write style for screen_sleep_music_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_sleep_music_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_sleep_music_label_1, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_sleep_music_label_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_sleep_music_label_1, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_sleep_music_label_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_sleep_music_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_sleep_music_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_sleep_music_label_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_sleep_music_label_1, 65, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_sleep_music_label_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_sleep_music_label_1, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_sleep_music_label_1, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_sleep_music_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_sleep_music_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_sleep_music_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_sleep_music_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //The custom code of screen_sleep_music.
#if GUIDER_UI_SLEEP_MUSIC_DURATION_LABELS
    setup_scr_screen_sleep_music_create_duration_labels(ui);
#endif

    //Init events for screen.
    events_init_screen_sleep_music(ui);
    //Update current screen layout.
    lv_obj_update_layout(ui->screen_sleep_music);

}
#endif
