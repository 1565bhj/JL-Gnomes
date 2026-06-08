/*
* Copyright 2025 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "app_config.h"

typedef struct {

    lv_obj_t *screen_main;
    bool screen_main_del;
    lv_obj_t *screen_main_img_charging_battery;
    lv_obj_t *screen_main_img_fully_battery;
    lv_obj_t *screen_main_img_no_alarm;
    lv_obj_t *screen_main_img_no_wifi;
    lv_obj_t *screen_main_img_location;
    lv_obj_t *screen_main_img_normol_battery;
    lv_obj_t *screen_main_img_no_BT;
    lv_obj_t *screen_main_img_location_set;
    lv_obj_t *screen_main_img_low_battery;
    lv_obj_t *screen_main_img_snow;
    lv_obj_t *screen_main_img_windy;
    lv_obj_t *screen_main_img_wind;
    lv_obj_t *screen_main_img_sun;
    lv_obj_t *screen_main_img_set_wifi;
    lv_obj_t *screen_main_img_set_alarm;
    lv_obj_t *screen_main_img_rain;
    lv_obj_t *screen_main_img_overcast;
    lv_obj_t *screen_main_span_time;
    lv_span_t *screen_main_span_time_span;
    lv_obj_t *screen_main_span_date;
    lv_span_t *screen_main_span_date_span;
    lv_obj_t *screen_main_span_week;
    lv_span_t *screen_main_span_week_span;
    lv_obj_t *screen_main_span_weather;
    lv_span_t *screen_main_span_weather_span;
    lv_obj_t *screen_main_span_direction;
    lv_span_t *screen_main_span_direction_span;
    lv_obj_t *screen_main_img_set_BT;
    lv_obj_t *screen_main_cont;
    lv_obj_t *screen_main_btn_music;
    lv_obj_t *screen_main_btn_music_label;
    lv_obj_t *screen_main_btn_conversation;
    lv_obj_t *screen_main_btn_conversation_label;
    lv_obj_t *screen_main_btn_alarm;
    lv_obj_t *screen_main_btn_alarm_label;
    lv_obj_t *screen_main_btn_timer;
    lv_obj_t *screen_main_btn_timer_label;
    lv_obj_t *screen_main_label_1;
//    lv_obj_t *screen_main_btn_bird_game;
//    lv_obj_t *screen_main_btn_bird_game_label;
    lv_obj_t *screen_main_btn_whack_game;
    lv_obj_t *screen_main_btn_whack_game_label;
    lv_obj_t *screen_main_btn_dice_game;
    lv_obj_t *screen_main_btn_dice_game_label;
    lv_obj_t *screen_main_btn_draw_lots_game;
    lv_obj_t *screen_main_btn_draw_lots_game_label;
    lv_obj_t *screen_main_btn_podcast;
    lv_obj_t *screen_main_btn_podcast_label;

    lv_obj_t *screen_option;
    bool screen_option_del;
    lv_obj_t *screen_option_option_music;
    lv_obj_t *screen_option_option_timer;
    lv_obj_t *screen_option_option_set;
    lv_obj_t *screen_option_option_conversation;
    lv_obj_t *screen_option_option_alarm;
    lv_obj_t *screen_option_op_name;
    lv_obj_t *screen_music;
    bool screen_music_del;
    lv_obj_t *screen_music_img_tape;
    lv_obj_t *screen_music_img_bar;
    lv_obj_t *screen_music_img_volume;
    lv_obj_t *screen_music_img_next;
    lv_obj_t *screen_music_img_prev;
    lv_obj_t *screen_music_slider;

    lv_obj_t *screen_music_label_title;
    lv_obj_t *screen_music_lyric;

    lv_obj_t *screen_music_span_start;
    lv_span_t *screen_music_span_start_span;
    lv_obj_t *screen_music_span_end;
    lv_span_t *screen_music_span_end_span;
    lv_obj_t *screen_music_bar_volume;
    lv_obj_t *screen_music_btn_play_pause;
    lv_obj_t *screen_music_btn_play_pause_label;
    lv_obj_t *screen_music_imgbtn_mode;
    lv_obj_t *screen_music_imgbtn_mode_label;
    lv_obj_t *screen_music_label_1;
    lv_obj_t *screen_conversation;
    bool screen_conversation_del;
    lv_obj_t *screen_conversation_cont_frame;
    lv_obj_t *screen_conversation_span_listen_speak;
    lv_span_t *screen_conversation_span_listen_speak_span;
    lv_obj_t *screen_conversation_cont_ai;
    lv_obj_t *screen_conversation_img_ai;
    lv_obj_t *screen_conversation_label_ai;
    lv_obj_t *screen_conversation_cont_user;
    lv_obj_t *screen_conversation_img_user;
    lv_obj_t *screen_conversation_label_user;
    lv_obj_t *screen_alarm;
    bool screen_alarm_del;
    lv_obj_t *screen_alarm_alarm_add;
    lv_obj_t *screen_alarm_alarm_del;
    lv_obj_t *screen_alarm_name;
    lv_span_t *screen_alarm_name_span;
    lv_obj_t *screen_alarm_cont;
    lv_obj_t *screen_alarm_cont_3;
    lv_obj_t *screen_alarm_repeat3;
    lv_span_t *screen_alarm_repeat3_span;
    lv_obj_t *screen_alarm_sw_3;
    lv_obj_t *screen_alarm_time3;
    lv_span_t *screen_alarm_time3_span;
    lv_obj_t *screen_alarm_cont_2;
    lv_obj_t *screen_alarm_repeat2;
    lv_span_t *screen_alarm_repeat2_span;
    lv_obj_t *screen_alarm_sw_2;
    lv_obj_t *screen_alarm_time2;
    lv_span_t *screen_alarm_time2_span;
    lv_obj_t *screen_alarm_cont_1;
    lv_obj_t *screen_alarm_repeat1;
    lv_span_t *screen_alarm_repeat1_span;
    lv_obj_t *screen_alarm_sw_1;
    lv_obj_t *screen_alarm_time1;
    lv_span_t *screen_alarm_time1_span;
    lv_obj_t *screen_alarm_add;
    bool screen_alarm_add_del;
    lv_obj_t *screen_alarm_add_roller_min;
    lv_obj_t *screen_alarm_add_roller_hour;
    lv_obj_t *screen_alarm_add_yes;
    lv_obj_t *screen_alarm_add_add_alarm_name;
    lv_span_t *screen_alarm_add_add_alarm_name_span;
    lv_obj_t *screen_alarm_add_return;
    lv_obj_t *screen_alarm_add_cont_repeat;
    lv_obj_t *screen_alarm_add_repeat_name;
    lv_span_t *screen_alarm_add_repeat_name_span;
    lv_obj_t *screen_alarm_add_enter;
    lv_obj_t *screen_alarm_add_dot;
    lv_span_t *screen_alarm_add_dot_span;
    lv_obj_t *screen_alarm_set;
    bool screen_alarm_set_del;
    lv_obj_t *screen_alarm_set_yes;
    lv_obj_t *screen_alarm_set_repeat_name;
    lv_span_t *screen_alarm_set_repeat_name_span;
    lv_obj_t *screen_alarm_set_return;
    lv_obj_t *screen_alarm_set_cont_circle;
    lv_obj_t *screen_alarm_set_circle;
    lv_span_t *screen_alarm_set_circle_span;
    lv_obj_t *screen_alarm_set_p_circle;
    lv_obj_t *screen_alarm_set_p_circle_label;
    lv_obj_t *screen_alarm_set_cont_once;
    lv_obj_t *screen_alarm_set_once;
    lv_span_t *screen_alarm_set_once_span;
    lv_obj_t *screen_alarm_set_p_once;
    lv_obj_t *screen_alarm_set_p_once_label;
    lv_obj_t *screen_alarm_set_cont_set;
    lv_obj_t *screen_alarm_set_data_set;
    lv_span_t *screen_alarm_set_data_set_span;
    lv_obj_t *screen_alarm_set_btn_7;
    lv_obj_t *screen_alarm_set_btn_7_label;
    lv_obj_t *screen_alarm_set_btn_2;
    lv_obj_t *screen_alarm_set_btn_2_label;
    lv_obj_t *screen_alarm_set_btn_3;
    lv_obj_t *screen_alarm_set_btn_3_label;
    lv_obj_t *screen_alarm_set_btn_4;
    lv_obj_t *screen_alarm_set_btn_4_label;
    lv_obj_t *screen_alarm_set_btn_5;
    lv_obj_t *screen_alarm_set_btn_5_label;
    lv_obj_t *screen_alarm_set_btn_1;
    lv_obj_t *screen_alarm_set_btn_1_label;
    lv_obj_t *screen_alarm_set_btn_6;
    lv_obj_t *screen_alarm_set_btn_6_label;
    lv_obj_t *screen_alarm_set_p_set;
    lv_obj_t *screen_alarm_set_p_set_label;
    lv_obj_t *screen_alarm_remove;
    bool screen_alarm_remove_del;
    lv_obj_t *screen_alarm_remove_choice;
    lv_span_t *screen_alarm_remove_choice_span;
    lv_obj_t *screen_alarm_remove_return;
    lv_obj_t *screen_alarm_remove_cont;
    lv_obj_t *screen_alarm_remove_cont_2;
    lv_obj_t *screen_alarm_remove_imgbtn_del3;
    lv_obj_t *screen_alarm_remove_imgbtn_del3_label;
    lv_obj_t *screen_alarm_remove_repeat2;
    lv_span_t *screen_alarm_remove_repeat2_span;
    lv_obj_t *screen_alarm_remove_time2;
    lv_span_t *screen_alarm_remove_time2_span;
    lv_obj_t *screen_alarm_remove_cont_3;
    lv_obj_t *screen_alarm_remove_imgbtn_del2;
    lv_obj_t *screen_alarm_remove_imgbtn_del2_label;
    lv_obj_t *screen_alarm_remove_repeat3;
    lv_span_t *screen_alarm_remove_repeat3_span;
    lv_obj_t *screen_alarm_remove_time3;
    lv_span_t *screen_alarm_remove_time3_span;
    lv_obj_t *screen_alarm_remove_cont_1;
    lv_obj_t *screen_alarm_remove_repeat1;
    lv_span_t *screen_alarm_remove_repeat1_span;
    lv_obj_t *screen_alarm_remove_time1;
    lv_span_t *screen_alarm_remove_time1_span;
    lv_obj_t *screen_alarm_remove_imgbtn_del1;
    lv_obj_t *screen_alarm_remove_imgbtn_del1_label;
    lv_obj_t *screen_alarm_remove_imgbtn_choice;
    lv_obj_t *screen_alarm_remove_imgbtn_choice_label;
    lv_obj_t *screen_alarm_remove_img_del;
    lv_obj_t *screen_timer;
    bool screen_timer_del;
    lv_obj_t *screen_timer_img_add;
    lv_obj_t *screen_timer_span_timer;
    lv_span_t *screen_timer_span_timer_span;
    lv_obj_t *screen_timer_img_del;
    lv_obj_t *screen_timer_cont;
    lv_obj_t *screen_timer_cont_3;
    lv_obj_t *screen_timer_sw_3;
    lv_obj_t *screen_timer_time3;
    lv_span_t *screen_timer_time3_span;
    lv_obj_t *screen_timer_cont_2;
    lv_obj_t *screen_timer_sw_2;
    lv_obj_t *screen_timer_time2;
    lv_span_t *screen_timer_time2_span;
    lv_obj_t *screen_timer_cont_1;
    lv_obj_t *screen_timer_sw_1;
    lv_obj_t *screen_timer_time1;
    lv_span_t *screen_timer_time1_span;
    lv_obj_t *screen_timer_add;
    bool screen_timer_add_del;
    lv_obj_t *screen_timer_add_img_return;
    lv_obj_t *screen_timer_add_add_timer_name;
    lv_span_t *screen_timer_add_add_timer_name_span;
    lv_obj_t *screen_timer_add_img_yes;
    lv_obj_t *screen_timer_add_roller_min;
    lv_obj_t *screen_timer_add_roller_hour;
    lv_obj_t *screen_timer_add_roller_sec;
    lv_obj_t *screen_timer_add_dot1;
    lv_span_t *screen_timer_add_dot1_span;
    lv_obj_t *screen_timer_add_dot2;
    lv_span_t *screen_timer_add_dot2_span;
    lv_obj_t *screen_timer_add_hour;
    lv_span_t *screen_timer_add_hour_span;
    lv_obj_t *screen_timer_add_min;
    lv_span_t *screen_timer_add_min_span;
    lv_obj_t *screen_timer_add_sec;
    lv_span_t *screen_timer_add_sec_span;
    lv_obj_t *screen_timer_add_min5;
    lv_obj_t *screen_timer_add_min5_label;
    lv_obj_t *screen_timer_add_min10;
    lv_obj_t *screen_timer_add_min10_label;
    lv_obj_t *screen_timer_add_min25;
    lv_obj_t *screen_timer_add_min25_label;
    lv_obj_t *screen_timer_remove;
    bool screen_timer_remove_del;
    lv_obj_t *screen_timer_remove_cont;
    lv_obj_t *screen_timer_remove_cont_3;
    lv_obj_t *screen_timer_remove_time_3;
    lv_span_t *screen_timer_remove_time_3_span;
    lv_obj_t *screen_timer_remove_imgbtn_del3;
    lv_obj_t *screen_timer_remove_imgbtn_del3_label;
    lv_obj_t *screen_timer_remove_cont_2;
    lv_obj_t *screen_timer_remove_imgbtn_del2;
    lv_obj_t *screen_timer_remove_imgbtn_del2_label;
    lv_obj_t *screen_timer_remove_time_2;
    lv_span_t *screen_timer_remove_time_2_span;
    lv_obj_t *screen_timer_remove_cont_1;
    lv_obj_t *screen_timer_remove_time_1;
    lv_span_t *screen_timer_remove_time_1_span;
    lv_obj_t *screen_timer_remove_imgbtn_del1;
    lv_obj_t *screen_timer_remove_imgbtn_del1_label;
    lv_obj_t *screen_timer_remove_choice;
    lv_span_t *screen_timer_remove_choice_span;
    lv_obj_t *screen_timer_remove_return;
    lv_obj_t *screen_timer_remove_imgbtn_choice;
    lv_obj_t *screen_timer_remove_imgbtn_choice_label;
    lv_obj_t *screen_timer_remove_img_del;
    lv_obj_t *screen_dropdown;
    bool screen_dropdown_del;
    lv_obj_t *screen_dropdown_max_volumn;
    lv_obj_t *screen_dropdown_min_light;
    lv_obj_t *screen_dropdown_max_light;
    lv_obj_t *screen_dropdown_min_volumn;
    lv_obj_t *screen_dropdown_cont;
    lv_obj_t *screen_dropdown_imgbtn_mic;
    lv_obj_t *screen_dropdown_imgbtn_mic_label;
    lv_obj_t *screen_dropdown_imgbtn_speaker;
    lv_obj_t *screen_dropdown_imgbtn_speaker_label;
    lv_obj_t *screen_dropdown_imgbtn_BT;
    lv_obj_t *screen_dropdown_imgbtn_BT_label;
    lv_obj_t *screen_dropdown_img_charging_battery;
    lv_obj_t *screen_dropdown_img_low_battery;
    lv_obj_t *screen_dropdown_img_set_alarm;
    lv_obj_t *screen_dropdown_img_set_BT;
    lv_obj_t *screen_dropdown_img_set_wifi;
    lv_obj_t *screen_dropdown_img_fully_battery;
    lv_obj_t *screen_dropdown_img_normal_battery;
    lv_obj_t *screen_dropdown_img_no_wifi;
    lv_obj_t *screen_dropdown_img_no_BT;
    lv_obj_t *screen_dropdown_img_no_alarm;
    lv_obj_t *screen_dropdown_time;
    lv_span_t *screen_dropdown_time_span;
    lv_obj_t *screen_dropdown_volumn_bar;
    lv_obj_t *screen_dropdown_light_bar;

    lv_obj_t *screen_draw_lots_game;
    bool screen_draw_lots_game_del;

    lv_obj_t *screen_dice_game;
    bool screen_dice_game_del;
    lv_obj_t *screen_dice_game_img_enter_game;
    lv_obj_t *screen_dice_game_btn_enter_game;
    lv_obj_t *screen_dice_game_btn_enter_game_label;
    lv_obj_t *screen_dice_game_img_goal;
    lv_obj_t *screen_dice_game_img_shake;
    lv_obj_t *screen_dice_game_die1;
    lv_obj_t *screen_dice_game_die2;
    lv_obj_t *screen_dice_game_die4;
    lv_obj_t *screen_dice_game_die6;
    lv_obj_t *screen_dice_game_die3;
    lv_obj_t *screen_dice_game_die5;
    lv_obj_t *screen_dice_game_pdie5;
    lv_obj_t *screen_dice_game_pdie3;
    lv_obj_t *screen_dice_game_pdie6;
    lv_obj_t *screen_dice_game_pdie4;
    lv_obj_t *screen_dice_game_pdie2;
    lv_obj_t *screen_dice_game_pdie1;
    lv_obj_t *screen_dice_game_score;
    lv_span_t *screen_dice_game_score_span;
    lv_obj_t *screen_dice_game_fail;
    lv_obj_t *screen_dice_game_btn_fail_again;
    lv_obj_t *screen_dice_game_btn_fail_again_label;
    lv_obj_t *screen_dice_game_black_bg;
    lv_obj_t *screen_dice_game_btn_vic_again;
    lv_obj_t *screen_dice_game_btn_vic_again_label;
    lv_obj_t *screen_dice_game_victory;
    lv_obj_t *screen_bird_game;
    bool screen_bird_game_del;
    lv_obj_t *screen_whack_game;
    bool screen_whack_game_del;
    lv_obj_t *screen_whack_game_img_countdown;
    lv_obj_t *screen_whack_game_cave1;
    lv_obj_t *screen_whack_game_cave3;
    lv_obj_t *screen_whack_game_cave5;
    lv_obj_t *screen_whack_game_cave4;
    lv_obj_t *screen_whack_game_cave6;
    lv_obj_t *screen_whack_game_cave2;
    lv_obj_t *screen_whack_game_mouse;
    lv_obj_t *screen_whack_game_mouse2;
    lv_obj_t *screen_whack_game_black_bg;
    lv_obj_t *screen_whack_game_btn_again;
    lv_obj_t *screen_whack_game_btn_again_label;
    lv_obj_t *screen_whack_game_img_score;
    lv_obj_t *screen_whack_game_score_num;
    lv_span_t *screen_whack_game_score_num_span;
    lv_obj_t *screen_whack_game_time;
    lv_span_t *screen_whack_game_time_span;
    lv_obj_t *screen_whack_game_img_enter_bg;
    lv_obj_t *screen_whack_game_btn_enter_game;
    lv_obj_t *screen_whack_game_btn_enter_game_label;

    lv_obj_t *screen_podcast;
    bool screen_podcast_del;
    lv_obj_t *screen_podcast_next;
    lv_obj_t *screen_podcast_last;
    lv_obj_t *screen_podcast_slider;
    lv_obj_t *screen_podcast_time_start;
    lv_span_t *screen_podcast_time_start_span;
    lv_obj_t *screen_podcast_time_end;
    lv_span_t *screen_podcast_time_end_span;
    lv_obj_t *screen_podcast_cont;
    lv_obj_t *screen_podcast_img_music;
    lv_obj_t *screen_podcast_img_tape_right;
    lv_obj_t *screen_podcast_img_tape;
    lv_obj_t *screen_podcast_img_tape_left;
    lv_obj_t *screen_podcast_img_bar;
    lv_obj_t *screen_podcast_podcast_title;
    lv_obj_t *screen_podcast_btn_play_pause;
    lv_obj_t *screen_podcast_btn_play_pause_label;

    lv_obj_t *screen_ring;
    bool screen_ring_del;
    lv_obj_t *screen_ring_light;
    lv_obj_t *screen_ring_ring;

    lv_obj_t *screen_tomato;
    bool screen_tomato_del;
    lv_obj_t *screen_tomato_img_add;
    lv_obj_t *screen_tomato_img_reset;
    lv_obj_t *screen_tomato_circle;
    lv_obj_t *screen_tomato_time;
    lv_obj_t *screen_tomato_hide_part;
} lv_ui;

typedef void (*ui_setup_scr_t)(lv_ui * ui);

void ui_init_style(lv_style_t * style);

void ui_load_scr_animation(lv_ui *ui, lv_obj_t ** new_scr, bool new_scr_del, bool * old_scr_del, ui_setup_scr_t setup_scr,
                           lv_scr_load_anim_t anim_type, uint32_t time, uint32_t delay, bool is_clean, bool auto_del);

void ui_animation(void * var, int32_t duration, int32_t delay, int32_t start_value, int32_t end_value, lv_anim_path_cb_t path_cb,
                  uint16_t repeat_cnt, uint32_t repeat_delay, uint32_t playback_time, uint32_t playback_delay,
                  lv_anim_exec_xcb_t exec_cb, lv_anim_start_cb_t start_cb, lv_anim_ready_cb_t ready_cb, lv_anim_deleted_cb_t deleted_cb);


void init_scr_del_flag(lv_ui *ui);

void setup_ui(lv_ui *ui);

void init_keyboard(lv_ui *ui);

extern lv_ui guider_ui;


void setup_scr_screen_main(lv_ui *ui);
void setup_scr_screen_option(lv_ui *ui);
void setup_scr_screen_music(lv_ui *ui);
void setup_scr_screen_conversation(lv_ui *ui);
void setup_scr_screen_alarm(lv_ui *ui);
void setup_scr_screen_alarm_add(lv_ui *ui);
void setup_scr_screen_alarm_set(lv_ui *ui);
void setup_scr_screen_alarm_remove(lv_ui *ui);
void setup_scr_screen_timer(lv_ui *ui);
void setup_scr_screen_timer_add(lv_ui *ui);
void setup_scr_screen_timer_remove(lv_ui *ui);
void setup_scr_screen_dropdown(lv_ui *ui);
void setup_scr_screen_story(lv_ui *ui);
void setup_scr_screen_podcast(lv_ui *ui);


LV_IMG_DECLARE(_charging_battery_alpha_22x14);
LV_IMG_DECLARE(_fully_charged_battery_alpha_22x14);
LV_IMG_DECLARE(_no_alarm_alpha_17x15);
LV_IMG_DECLARE(_no_wifi_alpha_18x16);
LV_IMG_DECLARE(_no_4g_alpha_18x16);
LV_IMG_DECLARE(_location_alpha_15x18);
LV_IMG_DECLARE(_normol_battery_alpha_22x14);
LV_IMG_DECLARE(_no_BT_alpha_14x16);
LV_IMG_DECLARE(_location_set_alpha_15x18);
LV_IMG_DECLARE(_low_battery_alpha_22x14);
LV_IMG_DECLARE(_snow_alpha_25x20);
LV_IMG_DECLARE(_windy_alpha_25x20);
LV_IMG_DECLARE(_wind_alpha_25x20);
LV_IMG_DECLARE(_sun_alpha_25x20);
LV_IMG_DECLARE(_set_wifi_alpha_18x16);
LV_IMG_DECLARE(_set_4g_alpha_18x16);
LV_IMG_DECLARE(_set_alarm_alpha_17x15);
LV_IMG_DECLARE(_rain_alpha_25x20);
LV_IMG_DECLARE(_overcast_alpha_25x20);
LV_IMG_DECLARE(_BT_alpha_14x16);


LV_IMG_DECLARE(_op_music_66x85);
LV_IMG_DECLARE(_op_conversation_66x85);
LV_IMG_DECLARE(_op_alarm_66x85);
LV_IMG_DECLARE(_op_timer_66x85);
LV_IMG_DECLARE(_op_podcast_66x85);
LV_IMG_DECLARE(_op_whack_game_66x85);
LV_IMG_DECLARE(_op_dice_game_66x85);
LV_IMG_DECLARE(_op_draw_lots_game_66x85);


LV_IMG_DECLARE(_music_option_alpha_100x100);
LV_IMG_DECLARE(_timer_option_alpha_100x100);
LV_IMG_DECLARE(_op_set_alpha_100x100);
LV_IMG_DECLARE(_conversation_option_alpha_100x100);
LV_IMG_DECLARE(_alarm_option_alpha_100x100);
LV_IMG_DECLARE(_tape_alpha_62x62);
LV_IMG_DECLARE(_tape_bar_alpha_35x68);
LV_IMG_DECLARE(_volume_alpha_20x20);
LV_IMG_DECLARE(_next_alpha_14x14);
LV_IMG_DECLARE(_last_alpha_14x14);
LV_IMG_DECLARE(_play_random_alpha_20x20);
LV_IMG_DECLARE(_play_repeat_alpha_20x20);
LV_IMG_DECLARE(_user_alpha_30x30);
LV_IMG_DECLARE(_ai_alpha_30x30);
LV_IMG_DECLARE(_add_alpha_30x30);
LV_IMG_DECLARE(_clear_alpha_30x30);
LV_IMG_DECLARE(_yes_alpha_30x30);
LV_IMG_DECLARE(_return_alpha_22x22);
LV_IMG_DECLARE(_enter_alpha_24x17);
LV_IMG_DECLARE(_yes_alpha_30x30);
LV_IMG_DECLARE(_return_alpha_22x22);
LV_IMG_DECLARE(_return_alpha_22x22);
LV_IMG_DECLARE(_un_press_alpha_20x20);
LV_IMG_DECLARE(_alarm_del_alpha_20x20);
LV_IMG_DECLARE(_un_press_alpha_20x20);
LV_IMG_DECLARE(_alarm_del_alpha_20x20);
LV_IMG_DECLARE(_un_press_alpha_20x20);
LV_IMG_DECLARE(_alarm_del_alpha_20x20);
LV_IMG_DECLARE(_all_choice_alpha_22x22);
LV_IMG_DECLARE(_all_choice_press_alpha_22x22);
LV_IMG_DECLARE(_trash_alpha_30x30);
LV_IMG_DECLARE(_add_alpha_30x30);
LV_IMG_DECLARE(_clear_alpha_30x30);
LV_IMG_DECLARE(_return_alpha_22x22);
LV_IMG_DECLARE(_yes_alpha_30x30);
LV_IMG_DECLARE(_un_press_alpha_20x20);
LV_IMG_DECLARE(_alarm_del_alpha_20x20);
LV_IMG_DECLARE(_un_press_alpha_20x20);
LV_IMG_DECLARE(_alarm_del_alpha_20x20);
LV_IMG_DECLARE(_un_press_alpha_20x20);
LV_IMG_DECLARE(_alarm_del_alpha_20x20);
LV_IMG_DECLARE(_return_alpha_22x22);
LV_IMG_DECLARE(_all_choice_alpha_22x22);
LV_IMG_DECLARE(_all_choice_press_alpha_22x22);
LV_IMG_DECLARE(_trash_alpha_30x30);
LV_IMG_DECLARE(_volume_alpha_30x30);
LV_IMG_DECLARE(_min_light_alpha_25x25);
LV_IMG_DECLARE(_max_light_alpha_30x30);
LV_IMG_DECLARE(_min_volumn_alpha_25x25);
LV_IMG_DECLARE(_mic_alpha_25x30);
LV_IMG_DECLARE(_no_mic_alpha_25x30);
LV_IMG_DECLARE(_max_volumn_alpha_30x30);
LV_IMG_DECLARE(_no_volumn_alpha_30x30);
LV_IMG_DECLARE(_no_BT2_alpha_40x30);
LV_IMG_DECLARE(_BT2_alpha_40x30);
LV_IMG_DECLARE(_charging_battery_alpha_22x14);
LV_IMG_DECLARE(_low_battery_alpha_22x14);
LV_IMG_DECLARE(_set_alarm_alpha_17x15);
LV_IMG_DECLARE(_BT_alpha_14x16);
LV_IMG_DECLARE(_set_wifi_alpha_18x16);
LV_IMG_DECLARE(_fully_charged_battery_alpha_22x14);
LV_IMG_DECLARE(_normol_battery_alpha_22x14);
LV_IMG_DECLARE(_no_wifi_alpha_18x16);
LV_IMG_DECLARE(_no_BT_alpha_14x16);
LV_IMG_DECLARE(_no_alarm_alpha_17x15);


LV_IMG_DECLARE(_next2_alpha_50x50);
LV_IMG_DECLARE(_last2_alpha_50x50);
LV_IMG_DECLARE(_groove_alpha_228x145);
LV_IMG_DECLARE(_story_alpha_80x80);
LV_IMG_DECLARE(_story_alpha_100x100);
LV_IMG_DECLARE(_story_alpha_80x80);
LV_IMG_DECLARE(_tape_bar_alpha_47x98);

LV_IMG_DECLARE(_bg2_100x39);

LV_IMG_DECLARE(_blog_alpha_80x80);
LV_IMG_DECLARE(_blog_alpha_100x100);
LV_IMG_DECLARE(_blog_alpha_80x80);
LV_IMG_DECLARE(_tape_bar_alpha_47x98);

LV_IMG_DECLARE(_bg2_100x39);
LV_IMG_DECLARE(_next2_alpha_50x50);
LV_IMG_DECLARE(_last2_alpha_50x50);

LV_IMG_DECLARE(_shunxu_alpha_20x20);
LV_IMG_DECLARE(_BT_alpha_20x20);

LV_IMG_DECLARE(_ring_alpha_100x100);

LV_IMG_DECLARE(_ring_light_alpha_200x240);
LV_IMG_DECLARE(_refresh_alpha_22x22);
LV_IMG_DECLARE(_hide_part_alpha_52x51);

LV_FONT_DECLARE(lv_font_Barlow__54)
LV_FONT_DECLARE(lv_font_Barlow__16)
LV_FONT_DECLARE(lv_font_MiSansDemibold_18)
LV_FONT_DECLARE(lv_font_MiSansDemibold_24)
LV_FONT_DECLARE(lv_font_MiSansDemibold_12)
LV_FONT_DECLARE(lv_font_Barlow__12)
LV_FONT_DECLARE(lv_font_Barlow__11)
LV_FONT_DECLARE(lv_font_Barlow__15)
LV_FONT_DECLARE(lv_font_Barlow__24)
LV_FONT_DECLARE(lv_font_Barlow__28)
LV_FONT_DECLARE(lv_font_Barlow__18)
LV_FONT_DECLARE(lv_font_Barlow__40)

#ifdef __cplusplus
}
#endif
#endif
