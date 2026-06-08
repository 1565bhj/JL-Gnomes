/*
* Copyright 2023 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/


/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include "lvgl.h"
#include "custom.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**
 * Create a demo application
 */

void custom_init(lv_ui *ui)
{
#ifdef APPLAYER_ENABLE
#else
    // setup_scr_screen_main(ui);
    setup_scr_screen_conversation(ui);
    setup_scr_screen_alarm(ui);
    setup_scr_screen_alarm_add(ui);
    setup_scr_screen_alarm_remove(ui);
    setup_scr_screen_alarm_set(ui);
    setup_scr_screen_timer(ui);
    setup_scr_screen_timer_add(ui);
    setup_scr_screen_timer_remove(ui);
#endif


}


