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
/* 必须在 gui_guider.h 之前：头文件里 lv_ui 与 CONFIG_LVGL_* 一致，否则 typedef 阶段宏为 0、此处又为 1 会成员不匹配 */
#include "lvgl_develop.h"
#include "gui_guider.h"
#include "widgets_init.h"

/*
 * 可选界面由 custom/lvgl_develop.h 中宏控制；本文件与各 setup_scr_screen_*.c 内条件编译一致。
 * CONFIG_LVGL_OPTION_WIFI_ENABLE   — WiFi 列表 / 密码页
 * CONFIG_LVGL_AGENT_ENABLE         — 智能体选择页
 * CONFIG_LVGL_CALCULATOR_ENABLE  — 计算器页
 * CONFIG_LVGL_CONTENT_LIST_ENABLE  — 内容列表 / 详情（教学）
 * CONFIG_LVGL_SLEEP_MUSIC_ENABLE   — 睡眠音乐页
 * （播客、定时休眠等见 lvgl_develop.h 其余项；对应 setup_scr 接入后再于此处增加调用。）
 */

#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "gg_external_data.h"
#endif

void ui_init_style(lv_style_t * style)
{
    if (style->prop_cnt > 1) {
        lv_style_reset(style);
    } else {
        lv_style_init(style);
    }
}

void ui_load_scr_animation(lv_ui *ui, lv_obj_t ** new_scr, bool new_scr_del, bool * old_scr_del, ui_setup_scr_t setup_scr,
                           lv_scr_load_anim_t anim_type, uint32_t time, uint32_t delay, bool is_clean, bool auto_del)
{
    lv_obj_t *act_scr = lv_scr_act();

#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
    if (auto_del) {
        gg_edata_task_clear(act_scr);
    }
#endif
    if (auto_del && is_clean) {
        lv_obj_clean(act_scr);
    }
    if (new_scr_del) {
        setup_scr(ui);
    }
    lv_scr_load_anim(*new_scr, anim_type, time, delay, auto_del);
    *old_scr_del = auto_del;
}

void ui_animation(void * var, int32_t duration, int32_t delay, int32_t start_value, int32_t end_value, lv_anim_path_cb_t path_cb,
                  uint16_t repeat_cnt, uint32_t repeat_delay, uint32_t playback_time, uint32_t playback_delay,
                  lv_anim_exec_xcb_t exec_cb, lv_anim_start_cb_t start_cb, lv_anim_ready_cb_t ready_cb, lv_anim_deleted_cb_t deleted_cb)
{
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, var);
    lv_anim_set_exec_cb(&anim, exec_cb);
    lv_anim_set_values(&anim, start_value, end_value);
    lv_anim_set_time(&anim, duration);
    lv_anim_set_delay(&anim, delay);
    lv_anim_set_path_cb(&anim, path_cb);
    lv_anim_set_repeat_count(&anim, repeat_cnt);
    lv_anim_set_repeat_delay(&anim, repeat_delay);
    lv_anim_set_playback_time(&anim, playback_time);
    lv_anim_set_playback_delay(&anim, playback_delay);
    if (start_cb) {
        lv_anim_set_start_cb(&anim, start_cb);
    }
    if (ready_cb) {
        lv_anim_set_ready_cb(&anim, ready_cb);
    }
    if (deleted_cb) {
        lv_anim_set_deleted_cb(&anim, deleted_cb);
    }
    lv_anim_start(&anim);
}

void init_scr_del_flag(lv_ui *ui)
{

    ui->screen_main_del = true;
    ui->screen_option_del = true;
    ui->screen_music_del = true;
    ui->screen_conversation_del = true;
    ui->screen_alarm_del = true;
    ui->screen_alarm_add_del = true;
    ui->screen_alarm_set_del = true;
    ui->screen_alarm_remove_del = true;
    ui->screen_timer_del = true;
    ui->screen_timer_add_del = true;
    ui->screen_timer_remove_del = true;
    ui->screen_dropdown_del = true;
    ui->screen_ring_del = true;
    ui->screen_tomato_del = true;
    ui->screen_net_del = true;
#if CONFIG_LVGL_OPTION_WIFI_ENABLE
    ui->screen_wifi_show_del = true;
    ui->screen_wifi_psw_del = true;
#endif
#if CONFIG_LVGL_AGENT_ENABLE
    ui->screen_agent_del = true;
#endif
#if CONFIG_LVGL_CALCULATOR_ENABLE
    ui->screen_calculator_del = true;
#endif
#if CONFIG_LVGL_CONTENT_LIST_ENABLE
    ui->screen_list_del = true;
    ui->screen_content_del = true;
#endif
#if CONFIG_LVGL_SLEEP_MUSIC_ENABLE
    ui->screen_sleep_music_del = true;
#endif
#if defined(CONFIG_LVGL_DRAW_LOTS_GAME_ENABLE) && CONFIG_LVGL_DRAW_LOTS_GAME_ENABLE
    ui->screen_draw_lots_game_del = true;
#endif
#if defined(CONFIG_LVGL_DICE_GAME_ENABLE) && CONFIG_LVGL_DICE_GAME_ENABLE
    ui->screen_dice_game_del = true;
#endif
#if defined(CONFIG_LVGL_MOUSE_GAME_ENABLE) && CONFIG_LVGL_MOUSE_GAME_ENABLE
    ui->screen_whack_game_del = true;
#endif
}

void setup_ui(lv_ui *ui)
{
    init_scr_del_flag(ui);
    init_keyboard(ui);

    /* 以下为各业务屏；可选屏与 lvgl_develop.h 宏一一对应 */
    setup_scr_screen_main(ui);
    setup_scr_screen_option(ui);
    setup_scr_screen_dropdown(ui);
    setup_scr_screen_music(ui);
    setup_scr_screen_conversation(ui);

    setup_scr_screen_alarm(ui);
    setup_scr_screen_alarm_add(ui);
    setup_scr_screen_alarm_remove(ui);
    setup_scr_screen_alarm_set(ui);
    setup_scr_screen_timer(ui);
    setup_scr_screen_timer_add(ui);
    setup_scr_screen_timer_remove(ui);
    setup_scr_screen_ring(ui);
    setup_scr_screen_tomato(ui);
#if CONFIG_LVGL_OPTION_WIFI_ENABLE
    setup_scr_screen_wifi_show(ui);
    setup_scr_screen_wifi_psw(ui);
#endif
    setup_scr_screen_net(ui);
#if CONFIG_LVGL_AGENT_ENABLE
    setup_scr_screen_agent(ui);
#endif
#if CONFIG_LVGL_CALCULATOR_ENABLE
    setup_scr_screen_calculator(ui);
#endif
#if CONFIG_LVGL_CONTENT_LIST_ENABLE
    setup_scr_screen_list(ui);
    setup_scr_screen_content(ui);
#endif
#if CONFIG_LVGL_SLEEP_MUSIC_ENABLE
    setup_scr_screen_sleep_music(ui);
#endif
#if defined(CONFIG_LVGL_DRAW_LOTS_GAME_ENABLE) && CONFIG_LVGL_DRAW_LOTS_GAME_ENABLE
    setup_scr_screen_draw_lots_game(ui);
#endif
#if defined(CONFIG_LVGL_DICE_GAME_ENABLE) && CONFIG_LVGL_DICE_GAME_ENABLE
    setup_scr_screen_dice_game(ui);
#endif
#if defined(CONFIG_LVGL_MOUSE_GAME_ENABLE) && CONFIG_LVGL_MOUSE_GAME_ENABLE
    setup_scr_screen_whack_game(ui);
#endif
    lv_scr_load(ui->screen_main);
}


void init_keyboard(lv_ui *ui)
{

    ui->g_kb_top_layer = lv_keyboard_create(lv_layer_top());
    lv_obj_add_event_cb(ui->g_kb_top_layer, kb_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(ui->g_kb_top_layer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_text_font(ui->g_kb_top_layer, &lv_font_MiSansDemibold_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    /* WiFi 密码页软键盘：白字深灰键面，与黑底密码页一致 */
    lv_obj_set_style_text_color(ui->g_kb_top_layer, lv_color_hex(0xffffff), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->g_kb_top_layer, lv_color_hex(0x3a3a3a), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->g_kb_top_layer, 255, LV_PART_ITEMS | LV_STATE_DEFAULT);
}
