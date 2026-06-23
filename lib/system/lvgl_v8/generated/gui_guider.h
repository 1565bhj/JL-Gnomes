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
#include "lvgl_develop.h"

typedef struct {

    lv_obj_t *screen_main;
    bool screen_main_del;
    lv_obj_t *screen_main_img_charging_battery;
    lv_obj_t *screen_main_img_fully_battery;
    lv_obj_t *screen_main_img_no_alarm;
    lv_obj_t *screen_main_img_no_wifi;
    lv_obj_t *screen_main_img_location;
    lv_obj_t *screen_main_img_normol_battery;
    lv_obj_t *screen_main_img_normal_fully_battery;
    lv_obj_t *screen_main_img_no_BT;
    lv_obj_t *screen_main_img_nomute;
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
    lv_obj_t *screen_main_btn_content;
    lv_obj_t *screen_main_btn_content_label;
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
    lv_obj_t *screen_music_img_return;
    lv_obj_t *screen_conversation;
    bool screen_conversation_del;
    lv_obj_t *screen_conversation_img_return;
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
    lv_obj_t *screen_alarm_img_return;
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
    lv_obj_t *screen_timer_img_return;
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
    lv_obj_t *screen_dropdown_img_return;
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
    lv_obj_t *screen_dropdown_img_normal_fully_battery;
    lv_obj_t *screen_dropdown_img_no_wifi;
    lv_obj_t *screen_dropdown_img_no_BT;
    lv_obj_t *screen_dropdown_img_no_alarm;
    lv_obj_t *screen_dropdown_img_nomute;
    lv_obj_t *screen_dropdown_time;
    lv_span_t *screen_dropdown_time_span;
    lv_obj_t *screen_dropdown_volumn_bar;
    lv_obj_t *screen_dropdown_light_bar;
    lv_obj_t *screen_dropdown_label_version;


    lv_obj_t *screen_ring;
    bool screen_ring_del;
    lv_obj_t *screen_ring_img_return;
    lv_obj_t *screen_ring_light;
    lv_obj_t *screen_ring_ring;

    lv_obj_t *screen_tomato;
    bool screen_tomato_del;
    lv_obj_t *screen_tomato_img_return;
    lv_obj_t *screen_tomato_img_add;
    lv_obj_t *screen_tomato_img_reset;
    lv_obj_t *screen_tomato_circle;
    lv_obj_t *screen_tomato_time;
    lv_obj_t *screen_tomato_hide_part;

#if CONFIG_LVGL_OPTION_WIFI_ENABLE
    lv_obj_t *screen_wifi_show;
    bool screen_wifi_show_del;
    lv_obj_t *screen_wifi_show_cont_wifi_disconnected;
    lv_obj_t *screen_wifi_show_span_net;
    lv_span_t *screen_wifi_show_span_net_span;
    lv_obj_t *screen_wifi_show_cont_wifi1;
    lv_obj_t *screen_wifi_show_span_wifi_name1;
    lv_span_t *screen_wifi_show_span_wifi_name1_span;
    lv_obj_t *screen_wifi_show_img_wifi1;
    lv_obj_t *screen_wifi_show_img_lock1;
    lv_obj_t *screen_wifi_show_cont_wifi2;
    lv_obj_t *screen_wifi_show_span_wifi_name2;
    lv_span_t *screen_wifi_show_span_wifi_name2_span;
    lv_obj_t *screen_wifi_show_img_wifi2;
    lv_obj_t *screen_wifi_show_img_lock2;
    lv_obj_t *screen_wifi_show_cont_wifi;
    lv_obj_t *screen_wifi_show_span_connect_suc;
    lv_span_t *screen_wifi_show_span_connect_suc_span;
    lv_obj_t *screen_wifi_show_cont_wifi_success;
    lv_obj_t *screen_wifi_show_span_wifi_success;
    lv_span_t *screen_wifi_show_span_wifi_success_span;
    lv_obj_t *screen_wifi_show_img_suceess;
    lv_obj_t *screen_wifi_show_img_wifi_success;
    lv_obj_t *screen_wifi_show_cont_title;
    lv_obj_t *screen_wifi_show_img_return;
    lv_obj_t *screen_wifi_show_cont_QR_code;
    lv_obj_t *screen_wifi_show_img_QR_code;
    lv_obj_t *screen_wifi_show_label_QR_code;
    lv_obj_t *screen_wifi_show_cont_refresh;
    lv_obj_t *screen_wifi_show_img_refresh;
    lv_obj_t *screen_wifi_show_label_refresh;
    lv_obj_t *screen_wifi_show_span_wifi;
    lv_span_t *screen_wifi_show_span_wifi_span;
    lv_obj_t *screen_wifi_show_cont_wifi_choose;
    lv_obj_t *screen_wifi_show_img_wifi;
    lv_obj_t *screen_wifi_show_span_wifi_name;
    lv_span_t *screen_wifi_show_span_wifi_name_span;
    lv_obj_t *screen_wifi_show_span_wifi_status;
    lv_span_t *screen_wifi_show_span_wifi_status_span;
    lv_obj_t *screen_wifi_show_btn_del;
    lv_obj_t *screen_wifi_show_btn_del_label;
    lv_obj_t *screen_wifi_show_img_return_choose;

    lv_obj_t *screen_wifi_psw;
    bool screen_wifi_psw_del;
    lv_obj_t *screen_wifi_psw_ta_input;
    lv_obj_t *screen_wifi_psw_img_return;
    lv_obj_t *screen_wifi_psw_span_wifi_name;
    lv_span_t *screen_wifi_psw_span_wifi_name_span;
    lv_obj_t *screen_wifi_psw_span_tip;
    lv_span_t *screen_wifi_psw_span_tip_span;
    lv_obj_t *screen_wifi_psw_btn_connect;
    lv_obj_t *screen_wifi_psw_btn_connect_label;
    lv_obj_t *screen_wifi_psw_btn_del;
    lv_obj_t *screen_wifi_psw_btn_del_label;
    lv_obj_t *screen_wifi_psw_cont_err;
    lv_obj_t *screen_wifi_psw_img_err;
    lv_obj_t *screen_wifi_psw_span_tip_err;
    lv_span_t *screen_wifi_psw_span_tip_err_span;
    lv_obj_t *screen_wifi_psw_btn_know;
    lv_obj_t *screen_wifi_psw_btn_know_label;
    lv_obj_t *screen_wifi_psw_cont_connecting;
    lv_obj_t *screen_wifi_psw_spangroup_1;
    lv_span_t *screen_wifi_psw_spangroup_1_span;
    lv_obj_t *screen_wifi_psw_spinner_wifi;
#if CONFIG_LVGL_WIFI_PASSWORD_IMGBTN_ENABLE
    lv_obj_t *screen_wifi_psw_imgbtn_psw;
    lv_obj_t *screen_wifi_psw_imgbtn_psw_label;
#endif
#endif
    lv_obj_t *g_kb_top_layer;
    lv_obj_t *screen_net;
    bool screen_net_del;
    lv_obj_t *screen_net_img_return;
    lv_obj_t *screen_net_net_name;
    lv_span_t *screen_net_net_name_span;
    lv_obj_t *screen_net_img_ios;

#if CONFIG_LVGL_AGENT_ENABLE
    lv_obj_t *screen_agent;
    bool screen_agent_del;
    lv_obj_t *screen_agent_img_return;
    lv_obj_t *screen_agent_label_title;
    lv_obj_t *screen_agent_cont_6;
    lv_obj_t *screen_agent_agent_6;
    lv_obj_t *screen_agent_label_name_6;
    lv_obj_t *screen_agent_cont_5;
    lv_obj_t *screen_agent_agent_5;
    lv_obj_t *screen_agent_label_name_5;
    lv_obj_t *screen_agent_cont_4;
    lv_obj_t *screen_agent_agent_4;
    lv_obj_t *screen_agent_label_name_4;
    lv_obj_t *screen_agent_cont_3;
    lv_obj_t *screen_agent_agent_3;
    lv_obj_t *screen_agent_label_name_3;
    lv_obj_t *screen_agent_cont_2;
    lv_obj_t *screen_agent_agent_2;
    lv_obj_t *screen_agent_label_name_2;
    lv_obj_t *screen_agent_cont_1;
    lv_obj_t *screen_agent_agent_1;
    lv_obj_t *screen_agent_label_name_1;
    lv_obj_t *screen_agent_img_choose;
#endif
#if CONFIG_LVGL_CALCULATOR_ENABLE
    lv_obj_t *screen_calculator;
    bool screen_calculator_del;
    lv_obj_t *screen_calculator_btnm_calculate;
    lv_obj_t *screen_calculator_label_calculate;
    lv_obj_t *screen_calculator_img_return;
#endif
#if CONFIG_LVGL_CONTENT_LIST_ENABLE
    lv_obj_t *screen_list;
    bool screen_list_del;
    lv_obj_t *screen_list_cont_1;
    lv_obj_t *screen_list_img_return;
    lv_obj_t *screen_list_label_name;
    lv_obj_t *screen_list_cont_content1;
    lv_obj_t *screen_list_study_english;
    lv_obj_t *screen_list_chinese_classics;
    lv_obj_t *screen_list_excellent_essay;
    lv_obj_t *screen_list_study_QA;
    lv_obj_t *screen_list_general_QA;
    lv_obj_t *screen_list_cont_content2;
    lv_obj_t *screen_list_label_1;
    lv_obj_t *screen_list_label_2;
    lv_obj_t *screen_list_label_3;
    lv_obj_t *screen_list_label_4;
    lv_obj_t *screen_list_label_5;
    lv_obj_t *screen_list_label_6;
    lv_obj_t *screen_content;
    bool screen_content_del;
    lv_obj_t *screen_content_label_name;
    lv_obj_t *screen_content_img_return;
    lv_obj_t *screen_content_cont_1;
    lv_obj_t *screen_content_label_last_page;
    lv_obj_t *screen_content_label_next_page;
    lv_obj_t *screen_content_label_repeat;
#endif
#if CONFIG_LVGL_SLEEP_MUSIC_ENABLE
    lv_obj_t *screen_sleep_music;
    bool screen_sleep_music_del;
    lv_obj_t *screen_sleep_music_span_name;
    lv_span_t *screen_sleep_music_span_name_span;
    lv_obj_t *screen_sleep_music_img_return;
    lv_obj_t *screen_sleep_music_cont;
    lv_obj_t *screen_sleep_music_label_5;
    lv_obj_t *screen_sleep_music_label_6;
    lv_obj_t *screen_sleep_music_label_7;
    lv_obj_t *screen_sleep_music_label_8;
    lv_obj_t *screen_sleep_music_label_4;
    lv_obj_t *screen_sleep_music_label_3;
    lv_obj_t *screen_sleep_music_label_2;
    lv_obj_t *screen_sleep_music_label_1;
#if GUIDER_UI_SLEEP_MUSIC_DURATION_LABELS
    lv_obj_t *screen_sleep_music_label_30min;
    lv_obj_t *screen_sleep_music_label_90min;
#endif
#endif

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

#if CONFIG_LVGL_OPTION_WIFI_ENABLE
void setup_scr_screen_wifi_show(lv_ui *ui);
void setup_scr_screen_wifi_psw(lv_ui *ui);
#endif
#if CONFIG_LVGL_CONTENT_LIST_ENABLE
void setup_scr_screen_list(lv_ui *ui);
void setup_scr_screen_content(lv_ui *ui);
#endif
#if CONFIG_LVGL_AGENT_ENABLE
void setup_scr_screen_agent(lv_ui *ui);
#endif
#if CONFIG_LVGL_CALCULATOR_ENABLE
void setup_scr_screen_calculator(lv_ui *ui);
#endif
#if CONFIG_LVGL_SLEEP_MUSIC_ENABLE
void setup_scr_screen_sleep_music(lv_ui *ui);
#endif

LV_IMG_DECLARE(_charging_battery_alpha_22x14);
LV_IMG_DECLARE(_fully_charged_battery_alpha_22x14);
LV_IMG_DECLARE(_no_alarm_alpha_17x15);
LV_IMG_DECLARE(_no_wifi_alpha_18x16);
LV_IMG_DECLARE(_no_4g_alpha_18x16);
LV_IMG_DECLARE(_location_alpha_15x18);
LV_IMG_DECLARE(_normol_battery_alpha_22x14);
LV_IMG_DECLARE(_nomute_alpha_17x18);
LV_IMG_DECLARE(_normal_fully_battery_alpha_22x14);
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

LV_IMG_DECLARE(_choose_bg_320x240);
LV_IMG_DECLARE(_return_alpha_20x20);
LV_IMG_DECLARE(_agent_6_alpha_66x66);
LV_IMG_DECLARE(_agent_5_alpha_66x66);
LV_IMG_DECLARE(_agent_4_alpha_66x66);
LV_IMG_DECLARE(_agent_3_alpha_66x66);
LV_IMG_DECLARE(_agent_2_alpha_66x66);
LV_IMG_DECLARE(_agent_1_alpha_66x66);
LV_IMG_DECLARE(_choose_alpha_66x66);

LV_IMG_DECLARE(_op_music_alpha_66x85);
LV_IMG_DECLARE(_op_conversation_alpha_66x85);
LV_IMG_DECLARE(_op_alarm_alpha_66x85);
LV_IMG_DECLARE(_op_timer_alpha_66x85);
LV_IMG_DECLARE(_op_content_alpha_66x85);
LV_IMG_DECLARE(_op_wifi_alpha_66x85);
LV_IMG_DECLARE(_op_sleep_music_alpha_66x85);
LV_IMG_DECLARE(_op_agent_alpha_66x85);
LV_IMG_DECLARE(_op_calculator_alpha_66x85);
#if defined(CONFIG_LVGL_GAMES_ENABLE) && CONFIG_LVGL_GAMES_ENABLE && \
    defined(CONFIG_LVGL_MOUSE_GAME_ENABLE) && CONFIG_LVGL_MOUSE_GAME_ENABLE
LV_IMG_DECLARE(_op_whack_game_66x85);
#endif
LV_IMG_DECLARE(_ios_alpha_100x100);

LV_IMG_DECLARE(_set_wifi_alpha_16x14);
LV_IMG_DECLARE(_lock_alpha_18x16);
LV_IMG_DECLARE(_return_alpha_20x20);
LV_IMG_DECLARE(_wifi_err_alpha_80x80);
LV_IMG_DECLARE(_connect_success_alpha_20x20);
#if CONFIG_LVGL_WIFI_PASSWORD_IMGBTN_ENABLE
LV_IMG_DECLARE(_close_alpha_20x20);
LV_IMG_DECLARE(_open_alpha_20x20);
#endif
LV_IMG_DECLARE(_QR_code_alpha_30x30);
LV_IMG_DECLARE(_refresh_alpha_15x15);
LV_IMG_DECLARE(_set_wifi_alpha_80x62);

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

/* screen_list 分类入口图（generated/images） */
LV_IMG_DECLARE(_study_english_alpha_240x82);
LV_IMG_DECLARE(_study_chinese_alpha_240x82);
LV_IMG_DECLARE(_excellent_essays_alpha_240x82);
LV_IMG_DECLARE(_study_QA_alpha_240x82);
LV_IMG_DECLARE(_general_QA_alpha_240x82);

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
LV_FONT_DECLARE(lv_font_MiSansDemibold_14)
LV_FONT_DECLARE(lv_font_Barlow__12)
LV_FONT_DECLARE(lv_font_Barlow__11)
LV_FONT_DECLARE(lv_font_Barlow__15)
LV_FONT_DECLARE(lv_font_Barlow__24)
LV_FONT_DECLARE(lv_font_Barlow__28)
LV_FONT_DECLARE(lv_font_Barlow__18)
LV_FONT_DECLARE(lv_font_Barlow__40)
LV_FONT_DECLARE(lv_font_Barlow__72)
LV_FONT_DECLARE(lv_font_Barlow__80)

#ifdef __cplusplus
}
#endif
#endif
