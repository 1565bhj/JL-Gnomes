/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"

#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif


static void screen_main_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED: {
            screen_main_event_setup();
            break;
        }
    default:
        break;
    }
}

void events_init_screen_main(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_main, screen_main_event_handler, LV_EVENT_ALL, ui);
}

static void screen_option_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED: {
            scrollicon();
            break;
        }
    default:
        break;
    }
}

void events_init_screen_option(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_option, screen_option_event_handler, LV_EVENT_ALL, ui);
}

static void screen_music_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED: {
            music_event_setup();
            break;
        }
    default:
        break;
    }
}

void events_init_screen_music(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_music, screen_music_event_handler, LV_EVENT_ALL, ui);
}

static void screen_conversation_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED: {
            conversation_event_setup();
            break;
        }
    default:
        break;
    }
}

void events_init_screen_conversation(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_conversation, screen_conversation_event_handler, LV_EVENT_ALL, ui);
}

static void screen_alarm_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED: {
            alarm_event_setup();
            break;
        }
    default:
        break;
    }
}

void events_init_screen_alarm(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_alarm, screen_alarm_event_handler, LV_EVENT_ALL, ui);
}

static void screen_alarm_add_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED: {
            alarm_add_event_setup();
            break;
        }
    default:
        break;
    }
}

void events_init_screen_alarm_add(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_alarm_add, screen_alarm_add_event_handler, LV_EVENT_ALL, ui);
}

static void screen_alarm_set_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED: {
            alarm_set_event_setup();
            break;
        }
    default:
        break;
    }
}

static void screen_alarm_set_btn_7_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED: {
            lv_obj_set_style_bg_opa(guider_ui.screen_alarm_set_btn_7, 255, LV_PART_MAIN);
            break;
        }
    default:
        break;
    }
}

void events_init_screen_alarm_set(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_alarm_set, screen_alarm_set_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_alarm_set_btn_7, screen_alarm_set_btn_7_event_handler, LV_EVENT_ALL, ui);
}

static void screen_alarm_remove_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED: {
            alarm_del_event_setup();
            break;
        }
    default:
        break;
    }
}

void events_init_screen_alarm_remove(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_alarm_remove, screen_alarm_remove_event_handler, LV_EVENT_ALL, ui);
}

static void screen_timer_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED: {
            timer_event_setup();
            break;
        }
    default:
        break;
    }
}

void events_init_screen_timer(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_timer, screen_timer_event_handler, LV_EVENT_ALL, ui);
}

static void screen_timer_add_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED: {
            timer_add_event_setup();
            break;
        }
    default:
        break;
    }
}

void events_init_screen_timer_add(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_timer_add, screen_timer_add_event_handler, LV_EVENT_ALL, ui);
}

static void screen_timer_remove_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED: {
            timer_del_event_setup();
            break;
        }
    default:
        break;
    }
}

void events_init_screen_timer_remove(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_timer_remove, screen_timer_remove_event_handler, LV_EVENT_ALL, ui);
}

static void screen_dropdown_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED: {
            dropdown_event_setup();
            break;
        }
    default:
        break;
    }
}

void events_init_screen_dropdown(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_dropdown, screen_dropdown_event_handler, LV_EVENT_ALL, ui);
}

static void screen_story_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED: {
            story_event_setup();
            break;
        }
    default:
        break;
    }
}

void events_init_screen_story(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_story, screen_story_event_handler, LV_EVENT_ALL, ui);
}


void events_init(lv_ui *ui)
{

}
