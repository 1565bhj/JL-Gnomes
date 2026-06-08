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


void setup_scr_screen_main(lv_ui *ui)
{
    //Write codes screen_main
    ui->screen_main = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_main, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_main, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_main, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_main, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_bg_img_src(ui->screen_main, &_background_320x240, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->screen_main, CONFIG_UI_RES_FILE_PATH"bg1.bin", LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->screen_main, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->screen_main, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_img_charging_battery
    ui->screen_main_img_charging_battery = lv_img_create(ui->screen_main);
    lv_obj_add_flag(ui->screen_main_img_charging_battery, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_main_img_charging_battery, &_charging_battery_alpha_22x14);
    lv_img_set_pivot(ui->screen_main_img_charging_battery, 50, 50);
    lv_img_set_angle(ui->screen_main_img_charging_battery, 0);
    lv_obj_set_pos(ui->screen_main_img_charging_battery, 255, -71);
    lv_obj_set_size(ui->screen_main_img_charging_battery, 22, 14);

    //Write style for screen_main_img_charging_battery, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_main_img_charging_battery, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_main_img_charging_battery, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_img_charging_battery, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_main_img_charging_battery, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_img_fully_battery
    ui->screen_main_img_fully_battery = lv_img_create(ui->screen_main);
    lv_obj_add_flag(ui->screen_main_img_fully_battery, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_main_img_fully_battery, &_fully_charged_battery_alpha_22x14);
    lv_img_set_pivot(ui->screen_main_img_fully_battery, 50, 50);
    lv_img_set_angle(ui->screen_main_img_fully_battery, 0);
    lv_obj_set_pos(ui->screen_main_img_fully_battery, 255, -27);
    lv_obj_set_size(ui->screen_main_img_fully_battery, 22, 14);

    //Write style for screen_main_img_fully_battery, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_main_img_fully_battery, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_main_img_fully_battery, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_img_fully_battery, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_main_img_fully_battery, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_img_no_alarm
    ui->screen_main_img_no_alarm = lv_img_create(ui->screen_main);
    lv_obj_add_flag(ui->screen_main_img_no_alarm, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_main_img_no_alarm, &_no_alarm_alpha_17x15);
    lv_img_set_pivot(ui->screen_main_img_no_alarm, 50, 50);
    lv_img_set_angle(ui->screen_main_img_no_alarm, 0);
    lv_obj_set_pos(ui->screen_main_img_no_alarm, ALARM_ICON_X, ALARM_ICON_Y);
    lv_obj_set_size(ui->screen_main_img_no_alarm, 17, 15);

    //Write style for screen_main_img_no_alarm, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_main_img_no_alarm, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_main_img_no_alarm, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_img_no_alarm, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_main_img_no_alarm, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_img_no_wifi
    ui->screen_main_img_no_wifi = lv_img_create(ui->screen_main);
    lv_obj_add_flag(ui->screen_main_img_no_wifi, LV_OBJ_FLAG_CLICKABLE);

    extern int lv_demo_system_net_interface(void);
    if (lv_demo_system_net_interface()) {
        lv_img_set_src(ui->screen_main_img_no_wifi, &_no_4g_alpha_18x16);
    } else {
        lv_img_set_src(ui->screen_main_img_no_wifi, &_no_wifi_alpha_18x16);
    }

    lv_img_set_pivot(ui->screen_main_img_no_wifi, 50, 50);
    lv_img_set_angle(ui->screen_main_img_no_wifi, 0);
    lv_obj_set_pos(ui->screen_main_img_no_wifi, WIFI_ICON_X, WIFI_ICON_Y);
    lv_obj_set_size(ui->screen_main_img_no_wifi, 18, 16);

    //Write style for screen_main_img_no_wifi, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_main_img_no_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_main_img_no_wifi, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_img_no_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_main_img_no_wifi, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_img_location
    ui->screen_main_img_location = lv_img_create(ui->screen_main);
    lv_obj_add_flag(ui->screen_main_img_location, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_main_img_location, &_location_alpha_15x18);
    lv_img_set_pivot(ui->screen_main_img_location, 50, 50);
    lv_img_set_angle(ui->screen_main_img_location, 0);
    lv_obj_set_pos(ui->screen_main_img_location, 10, 17);
    lv_obj_set_size(ui->screen_main_img_location, 15, 18);

    //Write style for screen_main_img_location, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_main_img_location, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_main_img_location, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_img_location, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_main_img_location, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_img_normol_battery
    ui->screen_main_img_normol_battery = lv_img_create(ui->screen_main);
    lv_obj_add_flag(ui->screen_main_img_normol_battery, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_main_img_normol_battery, &_normol_battery_alpha_22x14);
    lv_img_set_pivot(ui->screen_main_img_normol_battery, 50, 50);
    lv_img_set_angle(ui->screen_main_img_normol_battery, 0);
    lv_obj_set_pos(ui->screen_main_img_normol_battery, BATTERY_ICON_X, BATTERY_ICON_Y);
    lv_obj_set_size(ui->screen_main_img_normol_battery, 22, 14);

    //Write style for screen_main_img_normol_battery, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_main_img_normol_battery, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_main_img_normol_battery, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_img_normol_battery, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_main_img_normol_battery, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_img_no_BT
    ui->screen_main_img_no_BT = lv_img_create(ui->screen_main);
    lv_obj_add_flag(ui->screen_main_img_no_BT, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_main_img_no_BT, &_no_BT_alpha_14x16);
    lv_img_set_pivot(ui->screen_main_img_no_BT, 50, 50);
    lv_img_set_angle(ui->screen_main_img_no_BT, 0);
    lv_obj_set_pos(ui->screen_main_img_no_BT, BLUETOOTH_ICON_X, BLUETOOTH_ICON_Y);
    lv_obj_set_size(ui->screen_main_img_no_BT, 14, 16);

    //Write style for screen_main_img_no_BT, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_main_img_no_BT, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_main_img_no_BT, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_img_no_BT, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_main_img_no_BT, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_img_location_set
    ui->screen_main_img_location_set = lv_img_create(ui->screen_main);
    lv_obj_add_flag(ui->screen_main_img_location_set, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_main_img_location_set, &_location_set_alpha_15x18);
    lv_img_set_pivot(ui->screen_main_img_location_set, 50, 50);
    lv_img_set_angle(ui->screen_main_img_location_set, 0);
    lv_obj_set_pos(ui->screen_main_img_location_set, 29, -34);
    lv_obj_set_size(ui->screen_main_img_location_set, 15, 18);

    //Write style for screen_main_img_location_set, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_main_img_location_set, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_main_img_location_set, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_img_location_set, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_main_img_location_set, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_img_low_battery
    ui->screen_main_img_low_battery = lv_img_create(ui->screen_main);
    lv_obj_add_flag(ui->screen_main_img_low_battery, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_main_img_low_battery, &_low_battery_alpha_22x14);
    lv_img_set_pivot(ui->screen_main_img_low_battery, 50, 50);
    lv_img_set_angle(ui->screen_main_img_low_battery, 0);
    lv_obj_set_pos(ui->screen_main_img_low_battery, 255, -48);
    lv_obj_set_size(ui->screen_main_img_low_battery, 22, 14);

    //Write style for screen_main_img_low_battery, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_main_img_low_battery, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_main_img_low_battery, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_img_low_battery, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_main_img_low_battery, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_img_snow
//    ui->screen_main_img_snow = lv_img_create(ui->screen_main);
//    lv_obj_add_flag(ui->screen_main_img_snow, LV_OBJ_FLAG_CLICKABLE);
//    lv_img_set_src(ui->screen_main_img_snow, &_snow_alpha_25x20);
//    lv_img_set_pivot(ui->screen_main_img_snow, 50, 50);
//    lv_img_set_angle(ui->screen_main_img_snow, 0);
//    lv_obj_set_pos(ui->screen_main_img_snow, 34, 303);
//    lv_obj_set_size(ui->screen_main_img_snow, 25, 20);

//    //Write style for screen_main_img_snow, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
//    lv_obj_set_style_img_recolor_opa(ui->screen_main_img_snow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_img_opa(ui->screen_main_img_snow, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_radius(ui->screen_main_img_snow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_clip_corner(ui->screen_main_img_snow, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_img_windy
//    ui->screen_main_img_windy = lv_img_create(ui->screen_main);
//    lv_obj_add_flag(ui->screen_main_img_windy, LV_OBJ_FLAG_CLICKABLE);
//    lv_img_set_src(ui->screen_main_img_windy, &_windy_alpha_25x20);
//    lv_img_set_pivot(ui->screen_main_img_windy, 50, 50);
//    lv_img_set_angle(ui->screen_main_img_windy, 0);
//    lv_obj_set_pos(ui->screen_main_img_windy, 214, 303);
//    lv_obj_set_size(ui->screen_main_img_windy, 25, 20);

    //Write style for screen_main_img_windy, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
//    lv_obj_set_style_img_recolor_opa(ui->screen_main_img_windy, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_img_opa(ui->screen_main_img_windy, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_radius(ui->screen_main_img_windy, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_clip_corner(ui->screen_main_img_windy, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_img_wind
//    ui->screen_main_img_wind = lv_img_create(ui->screen_main);
//    lv_obj_add_flag(ui->screen_main_img_wind, LV_OBJ_FLAG_CLICKABLE);
//    lv_img_set_src(ui->screen_main_img_wind, &_wind_alpha_25x20);
//    lv_img_set_pivot(ui->screen_main_img_wind, 50, 50);
//    lv_img_set_angle(ui->screen_main_img_wind, 0);
//    lv_obj_set_pos(ui->screen_main_img_wind, 274, 303);
//    lv_obj_set_size(ui->screen_main_img_wind, 25, 20);

    //Write style for screen_main_img_wind, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
//    lv_obj_set_style_img_recolor_opa(ui->screen_main_img_wind, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_img_opa(ui->screen_main_img_wind, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_radius(ui->screen_main_img_wind, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_clip_corner(ui->screen_main_img_wind, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_img_sun
    ui->screen_main_img_sun = lv_img_create(ui->screen_main);
    lv_obj_add_flag(ui->screen_main_img_sun, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_main_img_sun, &_sun_alpha_25x20);
    lv_img_set_pivot(ui->screen_main_img_sun, 50, 50);
    lv_img_set_angle(ui->screen_main_img_sun, 0);
    lv_obj_set_pos(ui->screen_main_img_sun, WEATHER_ICON_X, WEATHER_ICON_Y);
    lv_obj_set_size(ui->screen_main_img_sun, 25, 20);

    //Write style for screen_main_img_sun, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_main_img_sun, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_main_img_sun, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_img_sun, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_main_img_sun, true, LV_PART_MAIN | LV_STATE_DEFAULT);

//    //Write codes screen_main_img_set_wifi
//    ui->screen_main_img_set_wifi = lv_img_create(ui->screen_main);
//    lv_obj_add_flag(ui->screen_main_img_set_wifi, LV_OBJ_FLAG_CLICKABLE);
//    lv_img_set_src(ui->screen_main_img_set_wifi, &_set_wifi_alpha_18x16);
//    lv_img_set_pivot(ui->screen_main_img_set_wifi, 50, 50);
//    lv_img_set_angle(ui->screen_main_img_set_wifi, 0);
//    lv_obj_set_pos(ui->screen_main_img_set_wifi, 229, -31);
//    lv_obj_set_size(ui->screen_main_img_set_wifi, 18, 16);
//
//    //Write style for screen_main_img_set_wifi, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
//    lv_obj_set_style_img_recolor_opa(ui->screen_main_img_set_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_img_opa(ui->screen_main_img_set_wifi, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_radius(ui->screen_main_img_set_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_clip_corner(ui->screen_main_img_set_wifi, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_img_set_alarm
    ui->screen_main_img_set_alarm = lv_img_create(ui->screen_main);
    lv_obj_add_flag(ui->screen_main_img_set_alarm, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_main_img_set_alarm, &_set_alarm_alpha_17x15);
    lv_img_set_pivot(ui->screen_main_img_set_alarm, 50, 50);
    lv_img_set_angle(ui->screen_main_img_set_alarm, 0);
    lv_obj_set_pos(ui->screen_main_img_set_alarm, 182, -31);
    lv_obj_set_size(ui->screen_main_img_set_alarm, 17, 15);

    //Write style for screen_main_img_set_alarm, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_main_img_set_alarm, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_main_img_set_alarm, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_img_set_alarm, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_main_img_set_alarm, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_img_rain
//    ui->screen_main_img_rain = lv_img_create(ui->screen_main);
//    lv_obj_add_flag(ui->screen_main_img_rain, LV_OBJ_FLAG_CLICKABLE);
//    lv_img_set_src(ui->screen_main_img_rain, &_rain_alpha_25x20);
//    lv_img_set_pivot(ui->screen_main_img_rain, 50, 50);
//    lv_img_set_angle(ui->screen_main_img_rain, 0);
//    lv_obj_set_pos(ui->screen_main_img_rain, 156, 303);
//    lv_obj_set_size(ui->screen_main_img_rain, 25, 20);

    //Write style for screen_main_img_rain, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
//    lv_obj_set_style_img_recolor_opa(ui->screen_main_img_rain, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_img_opa(ui->screen_main_img_rain, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_radius(ui->screen_main_img_rain, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_clip_corner(ui->screen_main_img_rain, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_img_overcast
//    ui->screen_main_img_overcast = lv_img_create(ui->screen_main);
//    lv_obj_add_flag(ui->screen_main_img_overcast, LV_OBJ_FLAG_CLICKABLE);
//    lv_img_set_src(ui->screen_main_img_overcast, &_overcast_alpha_25x20);
//    lv_img_set_pivot(ui->screen_main_img_overcast, 50, 50);
//    lv_img_set_angle(ui->screen_main_img_overcast, 0);
//    lv_obj_set_pos(ui->screen_main_img_overcast, 94, 303);
//    lv_obj_set_size(ui->screen_main_img_overcast, 25, 20);

//    //Write style for screen_main_img_overcast, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
//    lv_obj_set_style_img_recolor_opa(ui->screen_main_img_overcast, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_img_opa(ui->screen_main_img_overcast, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_radius(ui->screen_main_img_overcast, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//    lv_obj_set_style_clip_corner(ui->screen_main_img_overcast, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_span_time
    ui->screen_main_span_time = lv_spangroup_create(ui->screen_main);
    lv_spangroup_set_align(ui->screen_main_span_time, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_main_span_time, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_main_span_time, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_main_span_time_span = lv_spangroup_new_span(ui->screen_main_span_time);
    lv_span_set_text(ui->screen_main_span_time_span, "12:00");
    lv_style_set_text_color(&ui->screen_main_span_time_span->style, lv_color_hex(0xffffff));
    lv_style_set_text_decor(&ui->screen_main_span_time_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_main_span_time_span->style, &lv_font_Barlow__54);
    lv_obj_set_pos(ui->screen_main_span_time, 44, 49);
    lv_obj_set_size(ui->screen_main_span_time, 127, 47);

    //Write style state: LV_STATE_DEFAULT for &style_screen_main_span_time_main_main_default
    static lv_style_t style_screen_main_span_time_main_main_default;
    ui_init_style(&style_screen_main_span_time_main_main_default);

    lv_style_set_border_width(&style_screen_main_span_time_main_main_default, 0);
    lv_style_set_radius(&style_screen_main_span_time_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_main_span_time_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_main_span_time_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_main_span_time_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_main_span_time_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_main_span_time_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_main_span_time_main_main_default, 0);
    lv_obj_add_style(ui->screen_main_span_time, &style_screen_main_span_time_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_main_span_time);

    //Write codes screen_main_span_date
    ui->screen_main_span_date = lv_spangroup_create(ui->screen_main);
    lv_spangroup_set_align(ui->screen_main_span_date, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_main_span_date, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_main_span_date, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_main_span_date_span = lv_spangroup_new_span(ui->screen_main_span_date);
    lv_span_set_text(ui->screen_main_span_date_span, "2025/11/11");
    lv_style_set_text_color(&ui->screen_main_span_date_span->style, lv_color_hex(0xc5c5c5));
    lv_style_set_text_decor(&ui->screen_main_span_date_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_main_span_date_span->style, &lv_font_Barlow__16);
    lv_obj_set_pos(ui->screen_main_span_date, 196, 82);
    lv_obj_set_size(ui->screen_main_span_date, 81, 22);

    //Write style state: LV_STATE_DEFAULT for &style_screen_main_span_date_main_main_default
    static lv_style_t style_screen_main_span_date_main_main_default;
    ui_init_style(&style_screen_main_span_date_main_main_default);

    lv_style_set_border_width(&style_screen_main_span_date_main_main_default, 0);
    lv_style_set_radius(&style_screen_main_span_date_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_main_span_date_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_main_span_date_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_main_span_date_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_main_span_date_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_main_span_date_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_main_span_date_main_main_default, 0);
    lv_obj_add_style(ui->screen_main_span_date, &style_screen_main_span_date_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_main_span_date);

    //Write codes screen_main_span_week
    ui->screen_main_span_week = lv_spangroup_create(ui->screen_main);
    lv_spangroup_set_align(ui->screen_main_span_week, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_main_span_week, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_main_span_week, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_main_span_week_span = lv_spangroup_new_span(ui->screen_main_span_week);
    lv_span_set_text(ui->screen_main_span_week_span, "星期一");
    lv_style_set_text_color(&ui->screen_main_span_week_span->style, lv_color_hex(0xc5c5c5));
    lv_style_set_text_decor(&ui->screen_main_span_week_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_main_span_week_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_main_span_week, 196, 61);
    lv_obj_set_size(ui->screen_main_span_week, 55, 16);

    //Write style state: LV_STATE_DEFAULT for &style_screen_main_span_week_main_main_default
    static lv_style_t style_screen_main_span_week_main_main_default;
    ui_init_style(&style_screen_main_span_week_main_main_default);

    lv_style_set_border_width(&style_screen_main_span_week_main_main_default, 0);
    lv_style_set_radius(&style_screen_main_span_week_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_main_span_week_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_main_span_week_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_main_span_week_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_main_span_week_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_main_span_week_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_main_span_week_main_main_default, 0);
    lv_obj_add_style(ui->screen_main_span_week, &style_screen_main_span_week_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_main_span_week);

    //Write codes screen_main_span_weather
    ui->screen_main_span_weather = lv_spangroup_create(ui->screen_main);
    lv_spangroup_set_align(ui->screen_main_span_weather, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_main_span_weather, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_main_span_weather, LV_SPAN_MODE_EXPAND);
    //create span
    ui->screen_main_span_weather_span = lv_spangroup_new_span(ui->screen_main_span_weather);
    lv_span_set_text(ui->screen_main_span_weather_span, "暂无天气");
    lv_style_set_text_color(&ui->screen_main_span_weather_span->style, lv_color_hex(0xc5c5c5));
    lv_style_set_text_decor(&ui->screen_main_span_weather_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_main_span_weather_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_main_span_weather, WEATHER_ICON_X + 30, WEATHER_ICON_Y);
    lv_obj_set_size(ui->screen_main_span_weather, 98, 16);

    //Write style state: LV_STATE_DEFAULT for &style_screen_main_span_weather_main_main_default
    static lv_style_t style_screen_main_span_weather_main_main_default;
    ui_init_style(&style_screen_main_span_weather_main_main_default);

    lv_style_set_border_width(&style_screen_main_span_weather_main_main_default, 0);
    lv_style_set_radius(&style_screen_main_span_weather_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_main_span_weather_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_main_span_weather_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_main_span_weather_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_main_span_weather_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_main_span_weather_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_main_span_weather_main_main_default, 0);
    lv_obj_add_style(ui->screen_main_span_weather, &style_screen_main_span_weather_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_main_span_weather);

    //Write codes screen_main_span_direction
    ui->screen_main_span_direction = lv_spangroup_create(ui->screen_main);
    lv_spangroup_set_align(ui->screen_main_span_direction, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->screen_main_span_direction, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->screen_main_span_direction, LV_SPAN_MODE_FIXED);
    //create span
    ui->screen_main_span_direction_span = lv_spangroup_new_span(ui->screen_main_span_direction);
    lv_span_set_text(ui->screen_main_span_direction_span, "暂无城市");
    lv_style_set_text_color(&ui->screen_main_span_direction_span->style, lv_color_hex(0xc5c5c5));
    lv_style_set_text_decor(&ui->screen_main_span_direction_span->style, LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(&ui->screen_main_span_direction_span->style, &lv_font_MiSansDemibold_18);
    lv_obj_set_pos(ui->screen_main_span_direction, 29, 13);
    lv_obj_set_size(ui->screen_main_span_direction, 168, 22);

    //Write style state: LV_STATE_DEFAULT for &style_screen_main_span_direction_main_main_default
    static lv_style_t style_screen_main_span_direction_main_main_default;
    ui_init_style(&style_screen_main_span_direction_main_main_default);

    lv_style_set_border_width(&style_screen_main_span_direction_main_main_default, 0);
    lv_style_set_radius(&style_screen_main_span_direction_main_main_default, 0);
    lv_style_set_bg_opa(&style_screen_main_span_direction_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_main_span_direction_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_main_span_direction_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_main_span_direction_main_main_default, 0);
    lv_style_set_pad_left(&style_screen_main_span_direction_main_main_default, 0);
    lv_style_set_shadow_width(&style_screen_main_span_direction_main_main_default, 0);
    lv_obj_add_style(ui->screen_main_span_direction, &style_screen_main_span_direction_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->screen_main_span_direction);

    //Write codes screen_main_img_set_BT
    ui->screen_main_img_set_BT = lv_img_create(ui->screen_main);
    lv_obj_add_flag(ui->screen_main_img_set_BT, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->screen_main_img_set_BT, &_BT_alpha_14x16);
    lv_img_set_pivot(ui->screen_main_img_set_BT, 50, 50);
    lv_img_set_angle(ui->screen_main_img_set_BT, 0);
    lv_obj_set_pos(ui->screen_main_img_set_BT, 207, -31);
    lv_obj_set_size(ui->screen_main_img_set_BT, 14, 16);

    //Write style for screen_main_img_set_BT, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->screen_main_img_set_BT, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->screen_main_img_set_BT, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_img_set_BT, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_main_img_set_BT, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_cont
    ui->screen_main_cont = lv_obj_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_cont, 0, 107);
    lv_obj_set_size(ui->screen_main_cont, 320, 110);
    lv_obj_set_scrollbar_mode(ui->screen_main_cont, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_main_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_btn_music
    ui->screen_main_btn_music = lv_btn_create(ui->screen_main_cont);
    ui->screen_main_btn_music_label = lv_label_create(ui->screen_main_btn_music);
    lv_label_set_text(ui->screen_main_btn_music_label, "");
    lv_label_set_long_mode(ui->screen_main_btn_music_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_main_btn_music_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_main_btn_music, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_main_btn_music_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_main_btn_music, 156, -599);
    lv_obj_set_size(ui->screen_main_btn_music, 65, 65);

    //Write style for screen_main_btn_music, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_main_btn_music, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_main_btn_music, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_btn_music, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_btn_music, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->screen_main_btn_music, &_op_music_66x85, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->screen_main_btn_music, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->screen_main_btn_music, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_btn_music, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_btn_music, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_btn_music, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_btn_music, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_btn_conversation
    ui->screen_main_btn_conversation = lv_btn_create(ui->screen_main_cont);
    ui->screen_main_btn_conversation_label = lv_label_create(ui->screen_main_btn_conversation);
    lv_label_set_text(ui->screen_main_btn_conversation_label, "");
    lv_label_set_long_mode(ui->screen_main_btn_conversation_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_main_btn_conversation_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_main_btn_conversation, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_main_btn_conversation_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_main_btn_conversation, 85, -600);
    lv_obj_set_size(ui->screen_main_btn_conversation, 66, 85);

    //Write style for screen_main_btn_conversation, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_main_btn_conversation, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_main_btn_conversation, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_main_btn_conversation, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_main_btn_conversation, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_btn_conversation, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_btn_conversation, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->screen_main_btn_conversation, &_op_conversation_66x85, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->screen_main_btn_conversation, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->screen_main_btn_conversation, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_btn_conversation, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_btn_conversation, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_btn_conversation, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_btn_conversation, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_btn_alarm
    ui->screen_main_btn_alarm = lv_btn_create(ui->screen_main_cont);
    ui->screen_main_btn_alarm_label = lv_label_create(ui->screen_main_btn_alarm);
    lv_label_set_text(ui->screen_main_btn_alarm_label, "");
    lv_label_set_long_mode(ui->screen_main_btn_alarm_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_main_btn_alarm_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_main_btn_alarm, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_main_btn_alarm_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_main_btn_alarm, 156, -521);
    lv_obj_set_size(ui->screen_main_btn_alarm, 66, 85);

    //Write style for screen_main_btn_alarm, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_main_btn_alarm, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_main_btn_alarm, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_main_btn_alarm, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_main_btn_alarm, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_btn_alarm, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_btn_alarm, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->screen_main_btn_alarm, &_op_alarm_66x85, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->screen_main_btn_alarm, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->screen_main_btn_alarm, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_btn_alarm, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_btn_alarm, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_btn_alarm, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_btn_alarm, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_btn_timer
    ui->screen_main_btn_timer = lv_btn_create(ui->screen_main_cont);
    ui->screen_main_btn_timer_label = lv_label_create(ui->screen_main_btn_timer);
    lv_label_set_text(ui->screen_main_btn_timer_label, "");
    lv_label_set_long_mode(ui->screen_main_btn_timer_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_main_btn_timer_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_main_btn_timer, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_main_btn_timer_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_main_btn_timer, 82, -519);
    lv_obj_set_size(ui->screen_main_btn_timer, 66, 85);

    //Write style for screen_main_btn_timer, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_main_btn_timer, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_main_btn_timer, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_main_btn_timer, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_main_btn_timer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_btn_timer, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_btn_timer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->screen_main_btn_timer, &_op_timer_66x85, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->screen_main_btn_timer, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->screen_main_btn_timer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_btn_timer, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_btn_timer, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_btn_timer, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_btn_timer, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_label_1
    ui->screen_main_label_1 = lv_label_create(ui->screen_main);
    lv_label_set_text(ui->screen_main_label_1, "欢迎使用轻语AI智能设备，请使用“小语小语”唤醒我。");
    lv_label_set_long_mode(ui->screen_main_label_1, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_anim_speed(ui->screen_main_label_1, 10, LV_PART_MAIN);
    lv_obj_set_pos(ui->screen_main_label_1, 14, 209);
    lv_obj_set_size(ui->screen_main_label_1, 297, 22);

    //Write style for screen_main_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_1, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //The custom code of screen_main.



//Write codes screen_main_btn_podcast
    ui->screen_main_btn_podcast = lv_btn_create(ui->screen_main_cont);
    ui->screen_main_btn_podcast_label = lv_label_create(ui->screen_main_btn_podcast);
    lv_label_set_text(ui->screen_main_btn_podcast_label, "");
    lv_label_set_long_mode(ui->screen_main_btn_podcast_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_main_btn_podcast_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_main_btn_podcast, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_main_btn_podcast_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_main_btn_podcast, 154, -353);
    lv_obj_set_size(ui->screen_main_btn_podcast, 66, 85);

    //Write style for screen_main_btn_podcast, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_main_btn_podcast, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_main_btn_podcast, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_main_btn_podcast, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_main_btn_podcast, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_btn_podcast, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_btn_podcast, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->screen_main_btn_podcast, &_op_timer_66x85, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->screen_main_btn_podcast, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->screen_main_btn_podcast, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_btn_podcast, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_btn_podcast, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_btn_podcast, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_btn_podcast, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_btn_whack_game
    ui->screen_main_btn_whack_game = lv_btn_create(ui->screen_main_cont);
    ui->screen_main_btn_whack_game_label = lv_label_create(ui->screen_main_btn_whack_game);
    lv_label_set_text(ui->screen_main_btn_whack_game_label, "");
    lv_label_set_long_mode(ui->screen_main_btn_whack_game_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_main_btn_whack_game_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_main_btn_whack_game, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_main_btn_whack_game_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_main_btn_whack_game, 79, -358);
    lv_obj_set_size(ui->screen_main_btn_whack_game, 66, 85);

    //Write style for screen_main_btn_whack_game, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_main_btn_whack_game, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_main_btn_whack_game, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_main_btn_whack_game, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_main_btn_whack_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_btn_whack_game, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_btn_whack_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->screen_main_btn_whack_game, &_op_whack_game_66x85, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->screen_main_btn_whack_game, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->screen_main_btn_whack_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_btn_whack_game, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_btn_whack_game, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_btn_whack_game, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_btn_whack_game, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_btn_dice_game
    ui->screen_main_btn_dice_game = lv_btn_create(ui->screen_main_cont);
    ui->screen_main_btn_dice_game_label = lv_label_create(ui->screen_main_btn_dice_game);
    lv_label_set_text(ui->screen_main_btn_dice_game_label, "");
    lv_label_set_long_mode(ui->screen_main_btn_dice_game_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_main_btn_dice_game_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_main_btn_dice_game, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_main_btn_dice_game_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_main_btn_dice_game, 157, -432);
    lv_obj_set_size(ui->screen_main_btn_dice_game, 66, 85);

    //Write style for screen_main_btn_dice_game, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_main_btn_dice_game, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_main_btn_dice_game, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_main_btn_dice_game, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_main_btn_dice_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_btn_dice_game, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_btn_dice_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->screen_main_btn_dice_game, &_op_dice_game_66x85, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->screen_main_btn_dice_game, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->screen_main_btn_dice_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_btn_dice_game, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_btn_dice_game, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_btn_dice_game, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_btn_dice_game, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes screen_main_btn_draw_lots_game
    ui->screen_main_btn_draw_lots_game = lv_btn_create(ui->screen_main_cont);
    ui->screen_main_btn_draw_lots_game_label = lv_label_create(ui->screen_main_btn_draw_lots_game);
    lv_label_set_text(ui->screen_main_btn_draw_lots_game_label, "");
    lv_label_set_long_mode(ui->screen_main_btn_draw_lots_game_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_main_btn_draw_lots_game_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_main_btn_draw_lots_game, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_main_btn_draw_lots_game_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_main_btn_draw_lots_game, 81, -431);
    lv_obj_set_size(ui->screen_main_btn_draw_lots_game, 66, 85);

    //Write style for screen_main_btn_draw_lots_game, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_main_btn_draw_lots_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_main_btn_draw_lots_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_btn_draw_lots_game, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_btn_draw_lots_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->screen_main_btn_draw_lots_game, &_op_draw_lots_game_66x85, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->screen_main_btn_draw_lots_game, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->screen_main_btn_draw_lots_game, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_btn_draw_lots_game, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_btn_draw_lots_game, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_btn_draw_lots_game, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_btn_draw_lots_game, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Update current screen layout.
    lv_obj_update_layout(ui->screen_main);

    //Init events for screen.
    events_init_screen_main(ui);
}
