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



void setup_scr_screen_conversation(lv_ui *ui)
{
    //Write codes screen_conversation
    ui->screen_conversation = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_conversation, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_conversation, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_conversation, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_conversation, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_conversation, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_conversation, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_bg_img_src(ui->screen_conversation, &_background_320x240, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->screen_conversation, "m:mnt/sdfile/app/uipackres/ui/bg1.bin", LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->screen_conversation, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_conversation_cont_frame
    ui->screen_conversation_cont_frame = lv_obj_create(ui->screen_conversation);
    lv_obj_set_pos(ui->screen_conversation_cont_frame, 8, 44);
    lv_obj_set_size(ui->screen_conversation_cont_frame, 304, 180);
    lv_obj_set_scrollbar_mode(ui->screen_conversation_cont_frame, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_conversation_cont_frame, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_conversation_cont_frame, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_conversation_cont_frame, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_conversation_cont_frame, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_conversation_cont_frame, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_conversation_cont_frame, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_conversation_cont_frame, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_conversation_cont_frame, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_conversation_cont_frame, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_conversation_cont_frame, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_conversation_cont_frame, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_conversation_cont_frame, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_conversation_span_listen_speak
    ui->screen_conversation_span_listen_speak = lv_spangroup_create(ui->screen_conversation);
    lv_spangroup_set_align(ui->screen_conversation_span_listen_speak, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_conversation_span_listen_speak, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_conversation_span_listen_speak, LV_SPAN_MODE_BREAK);
    //create span
    ui->screen_conversation_span_listen_speak_span = lv_spangroup_new_span(ui->screen_conversation_span_listen_speak);
    lv_span_set_text(ui->screen_conversation_span_listen_speak_span, "AI正在思考……");
    lv_style_set_text_color(&ui->screen_conversation_span_listen_speak_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_conversation_span_listen_speak_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_conversation_span_listen_speak_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_conversation_span_listen_speak, 107, 18);
    lv_obj_set_size(ui->screen_conversation_span_listen_speak, 131, 14);

    //Write style state: LV_STATE_DEFAULT for &style_screen_conversation_span_listen_speak_main_main_default
    static lv_style_t style_screen_conversation_span_listen_speak_main_main_default;
    ui_init_style(&style_screen_conversation_span_listen_speak_main_main_default);

    lv_style_set_border_width(&style_screen_conversation_span_listen_speak_main_main_default, 0);
    lv_style_set_radius(&style_screen_conversation_span_listen_speak_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_conversation_span_listen_speak_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_conversation_span_listen_speak_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_conversation_span_listen_speak_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_conversation_span_listen_speak_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_conversation_span_listen_speak_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_conversation_span_listen_speak_main_main_default, 0);
    lv_obj_add_style(ui->screen_conversation_span_listen_speak, &style_screen_conversation_span_listen_speak_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_conversation_span_listen_speak);

    //Write codes screen_conversation_cont_ai
    ui->screen_conversation_cont_ai = lv_obj_create(ui->screen_conversation);
    lv_obj_set_pos(ui->screen_conversation_cont_ai, 9, 55);
    lv_obj_set_size(ui->screen_conversation_cont_ai, 301, 49);
    lv_obj_set_scrollbar_mode(ui->screen_conversation_cont_ai, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_conversation_cont_ai, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_conversation_cont_ai, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_conversation_cont_ai, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_conversation_cont_ai, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_conversation_cont_ai, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_conversation_cont_ai, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_conversation_cont_ai, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_conversation_cont_ai, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_conversation_cont_ai, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_conversation_cont_ai, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_conversation_cont_ai, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_conversation_cont_ai, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_conversation_img_ai
    ui->screen_conversation_img_ai = lv_img_create(ui->screen_conversation_cont_ai);
    lv_obj_add_flag(ui->screen_conversation_img_ai, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_conversation_img_ai, &_ai_alpha_30x30);
    lv_img_set_pivot(ui->screen_conversation_img_ai, 10, 10);
    lv_img_set_angle(ui->screen_conversation_img_ai, 0);
    lv_obj_set_pos(ui->screen_conversation_img_ai, 8, 0);
    lv_obj_set_size(ui->screen_conversation_img_ai, 30, 30);

    //Write style for screen_conversation_img_ai, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_conversation_img_ai, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_conversation_img_ai, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_conversation_img_ai, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_conversation_img_ai, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_conversation_label_ai
    ui->screen_conversation_label_ai = lv_label_create(ui->screen_conversation_cont_ai);
    lv_label_set_text(ui->screen_conversation_label_ai, "你好，谁的歌曲专辑？");
    lv_label_set_long_mode(ui->screen_conversation_label_ai, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_conversation_label_ai, 63, 0);
    lv_obj_set_size(ui->screen_conversation_label_ai, 192, 30);

    //Write style for screen_conversation_label_ai, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_conversation_label_ai, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_conversation_label_ai, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_conversation_label_ai, lv_color_hex(0xcfcfcf), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_conversation_label_ai, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_conversation_label_ai, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_conversation_label_ai, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_conversation_label_ai, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_conversation_label_ai, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_conversation_label_ai, 53, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_conversation_label_ai, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_conversation_label_ai, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_conversation_label_ai, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_conversation_label_ai, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_conversation_label_ai, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_conversation_label_ai, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_conversation_label_ai, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_conversation_cont_user
    ui->screen_conversation_cont_user = lv_obj_create(ui->screen_conversation);
    lv_obj_set_pos(ui->screen_conversation_cont_user, 9, 104);
    lv_obj_set_size(ui->screen_conversation_cont_user, 301, 67);
    lv_obj_set_scrollbar_mode(ui->screen_conversation_cont_user, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_conversation_cont_user, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_conversation_cont_user, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_conversation_cont_user, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_conversation_cont_user, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_conversation_cont_user, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_conversation_cont_user, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_conversation_cont_user, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_conversation_cont_user, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_conversation_cont_user, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_conversation_cont_user, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_conversation_cont_user, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_conversation_cont_user, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_conversation_img_user
    ui->screen_conversation_img_user = lv_img_create(ui->screen_conversation_cont_user);
    lv_obj_add_flag(ui->screen_conversation_img_user, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_conversation_img_user, &_user_alpha_30x30);
    lv_img_set_pivot(ui->screen_conversation_img_user, 50, 50);
    lv_img_set_angle(ui->screen_conversation_img_user, 0);
    lv_obj_set_pos(ui->screen_conversation_img_user, 282, 5);
    lv_obj_set_size(ui->screen_conversation_img_user, 30, 30);

    //Write style for screen_conversation_img_user, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_conversation_img_user, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_conversation_img_user, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_conversation_img_user, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_conversation_img_user, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_conversation_label_user
    ui->screen_conversation_label_user = lv_label_create(ui->screen_conversation_cont_user);
    lv_label_set_text(ui->screen_conversation_label_user, "那当然是火爆已久的歌手周杰伦啦！");
    lv_label_set_long_mode(ui->screen_conversation_label_user, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_conversation_label_user, 4, -6);
    lv_obj_set_size(ui->screen_conversation_label_user, 256, 50);

    //Write style for screen_conversation_label_user, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_conversation_label_user, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_conversation_label_user, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_conversation_label_user, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_conversation_label_user, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_conversation_label_user, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_conversation_label_user, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_conversation_label_user, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_conversation_label_user, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_conversation_label_user, 79, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_conversation_label_user, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_conversation_label_user, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_conversation_label_user, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_conversation_label_user, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_conversation_label_user, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_conversation_label_user, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_conversation_label_user, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //The custom code of screen_conversation.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_conversation);

    //Init events for screen.
    events_init_screen_conversation(ui);
}
