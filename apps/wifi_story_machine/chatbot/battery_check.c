#include "init.h"
#include "server/audio_server.h"
#include "server/server_core.h"
#include "app_config.h"
#include "action.h"
#include "system/app_core.h"
#include "event/key_event.h"
#include "fs/fs.h"
#include "asm/adc_api.h"
#include "system/timer.h"
#include "asm/efuse.h"
#include "asm/p33.h"
#include "asm/power_interface.h"
#include "asm/includes.h"
#include "app_power_manage.h"
#include "ai_uart_ctrol.h"

#define VBAT_LOW_NOTE_MAX_CNT   3 //低电量提示音提示次数
#define VBAT_LOW_NOTE_PERCENT   22 //低电量提示百分比，<=22%通知低电量


#ifdef TCFG_VBAT_CHECK_AD_PORT
static const u8 adc_port[12][2] = {
    {IO_PORTA_07, AD_CH_PA07},
    {IO_PORTA_08, AD_CH_PA08},
    {IO_PORTA_10, AD_CH_PA10},
    {IO_PORTB_01, AD_CH_PB01},
    {IO_PORTB_06, AD_CH_PB06},
    {IO_PORTB_07, AD_CH_PB07},
    {IO_PORTC_00, AD_CH_PC00},
    {IO_PORTC_01, AD_CH_PC01},
    {IO_PORTC_09, AD_CH_PC09},
    {IO_PORTC_10, AD_CH_PC10},
    {IO_PORTH_00, AD_CH_PH00},
    {IO_PORTH_03, AD_CH_PH03}
};
static char adc_channle = 0;
#endif
static unsigned char vbat_low_cnt = 0;
static unsigned char vbat_low_first_cnt = 0;
static unsigned char vbat_low_note_cnt = 0;
static unsigned char chargfull = 0;

#if TCFG_VBAT_CHECK_EN == 1
static int vbat_get_percent(void)
{
    int vbat_perc = 0;
#ifdef TCFG_VBAT_CHECK_AD_PORT
    u32 adcval = adc_get_value(adc_channle);
    int votg = 330 * adcval / 1024;
    int ad_vbat = (int)((float)votg / TCFG_VBAT_CHECK_AD_PORT_PPS);
    vbat_perc = (ad_vbat - LOW_POWER_OFF_VAL) * 100 / (420 - LOW_POWER_OFF_VAL);
    printf("vtg = %d\n", ad_vbat);
#else
    vbat_perc = get_vbat_percent();
#endif
    vbat_perc = vbat_perc > 100 ? 100 : vbat_perc;
    printf("vbat percent = %d%%\n", vbat_perc);
    return vbat_perc;
}
static int vbat_check(void)
{
    int vbat_perc = vbat_get_percent();
    if (vbat_perc == VBAT_LOW_NOTE_PERCENT) { //电量小于22%，提示充电
#ifdef CONFIG_UI_PLAY_EMOJI
        play_face_emoji(AI_UART_CMD_VBAT_LOW);// 低电量
#endif
#ifdef CONFIG_LVGL_UI_ENABLE
        int lv_demo_battery_percent_update(int percent, int charge_in, int charge_full);
        int sys_vbat_is_charging(void);
        if (!sys_vbat_is_charging()) {
            lv_demo_battery_percent_update(vbat_perc, 0, 0);
        }
#endif
        if (vbat_low_first_cnt < VBAT_LOW_NOTE_MAX_CNT) {
            if (!system_is_alarm_wakeup()) {
                music_play_res_file("ChargeMode.mp3");//电量低请充电
            }
        } else if (vbat_low_first_cnt >= 255) {
            vbat_low_first_cnt = VBAT_LOW_NOTE_MAX_CNT;
        }
        vbat_low_first_cnt++;
    } else if (vbat_perc > VBAT_LOW_NOTE_PERCENT) {
        vbat_low_first_cnt = 0;
#ifdef CONFIG_LVGL_UI_ENABLE
        int lv_demo_battery_percent_update(int percent, int charge_in, int charge_full);
        int sys_vbat_is_charging(void);
        if (!sys_vbat_is_charging()) {
            lv_demo_battery_percent_update(vbat_perc, 0, 0);
        }
#endif
    }
    if (vbat_perc <= VBAT_LOW_NOTE_PERCENT - 2) { //电量小于20%，提示充电
        vbat_low_cnt++;
    } else {
        if (vbat_low_note_cnt) {
#if (defined TCFG_LED_STATUES_VBAT_NET_EN && TCFG_LED_STATUES_VBAT_NET_EN == 1)
            extern void led_status_set(int status);//0初始化 1网络异常-快闪  2电池<20%-快闪  3网络正常和电池>=20%-常亮
            led_status_set(3);
#endif
        }
        vbat_low_cnt = 0;
        vbat_low_note_cnt = 0;
    }
    if (vbat_low_cnt > 4 || vbat_low_note_cnt) {
#if (defined TCFG_LED_STATUES_VBAT_NET_EN && TCFG_LED_STATUES_VBAT_NET_EN == 1)
        extern void led_status_set(int status);//0初始化 1网络异常-快闪  2电池<20%-快闪  3网络正常和电池>=20%-常亮
        led_status_set(2);
#endif
        //printf("->Vbat Low vbat percent = %d%%\n",vbat_perc);
        if (!system_is_alarm_wakeup()) {
            music_play_res_file("ChargeMode.mp3");//电量低请充电
        }
#ifdef CONFIG_UI_PLAY_EMOJI
        play_face_emoji(AI_UART_CMD_VBAT_LOW);// 低电量
#endif
#ifdef CONFIG_LVGL_UI_ENABLE
        int lv_demo_battery_percent_update(int percent, int charge_in, int charge_full);
        int sys_vbat_is_charging(void);
        if (!sys_vbat_is_charging()) {
            lv_demo_battery_percent_update(vbat_perc, 0, 0);
        }
#endif
        if (++vbat_low_note_cnt >= VBAT_LOW_NOTE_MAX_CNT) {
            struct key_event key = {0};
            key.type = KEY_EVENT_USER;
            key.action = KEY_EVENT_LONG;
            key.value = KEY_POWER;
            key_event_notify(KEY_EVENT_FROM_USER, &key);
        }
    }
    return vbat_perc;
}
int vbat_check_percent(void)
{
#ifdef TCFG_VBAT_CHECK_AD_PORT
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 2; j++) {
            if (adc_port[i][0] == TCFG_VBAT_CHECK_AD_PORT) {
                adc_channle = adc_port[i][1];
                adc_add_sample_ch(adc_channle);
                gpio_direction_input(TCFG_VBAT_CHECK_AD_PORT);
                gpio_set_pull_down(TCFG_VBAT_CHECK_AD_PORT, 0);
                gpio_set_pull_up(TCFG_VBAT_CHECK_AD_PORT, 0);
                gpio_set_die(TCFG_VBAT_CHECK_AD_PORT, 0);
            }
        }
    }
#endif
    return vbat_get_percent();
}
static int vbattery_check_init(void)
{
    if (production_io_is_enter() || is_production_test_enter(0)) {
        return 0;
    }
#ifdef TCFG_VBAT_CHECK_AD_PORT
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 2; j++) {
            if (adc_port[i][0] == TCFG_VBAT_CHECK_AD_PORT) {
                adc_channle = adc_port[i][1];
                adc_add_sample_ch(adc_channle);
                gpio_direction_input(TCFG_VBAT_CHECK_AD_PORT);
                gpio_set_pull_down(TCFG_VBAT_CHECK_AD_PORT, 0);
                gpio_set_pull_up(TCFG_VBAT_CHECK_AD_PORT, 0);
                gpio_set_die(TCFG_VBAT_CHECK_AD_PORT, 0);
            }
        }
    }
#endif
    void sys_vbat_charge_io_init(void);
    sys_timeout_add_to_task("sys_timer", NULL, sys_vbat_charge_io_init, 10 * 1000);
    return sys_timer_add_to_task("sys_timer", NULL, vbat_check, 10 * 1000);
}
late_initcall(vbattery_check_init);
#endif

int sys_get_vbat_percent(void)
{
#if TCFG_VBAT_CHECK_EN
    return vbat_get_percent();
#endif
    return 90;
}
int sys_vbat_is_charging(void)
{
#ifdef TCFG_CHARING_STATUS_PORT
    return gpio_read(TCFG_CHARING_STATUS_PORT);
#endif
    return 0;
}
int sys_vbat_is_chargfull(void)
{
#ifdef TCFG_VBAT_CHECK_EN
    return chargfull;
#endif
    return 0;
}
static void charge_status_check(void)
{
#ifdef TCFG_CHARING_STATUS_PORT
    int charging_status = gpio_read(TCFG_CHARING_STATUS_PORT);
    static unsigned char last_charging_status = 0;
    static unsigned char last_raw_status = 0;
    static unsigned char debounce_cnt = 0;
    static unsigned char already_notified_full = 0;
    static unsigned char dete_full_cnt = 0;
    static unsigned char dete_full_offline = 0;
    static unsigned short charging_cnt = 0;

#ifdef TCFG_CHARGFULL_STATUS_PORT
    unsigned char dete_full = gpio_read(TCFG_CHARGFULL_STATUS_PORT);
    if (dete_full && charging_status) {
        dete_full_cnt++;
        dete_full_offline = 0;
        if (dete_full_cnt > (1500 / 300)) { //大于1.5秒
            chargfull = true;
        }
    } else {
        dete_full_cnt = 0;
        dete_full_offline++;
        if (dete_full_offline > (1500 / 300)) {
            chargfull = 0;
        }
    }
#endif

    if (last_raw_status != charging_status) {
        debounce_cnt = 1;
        last_raw_status = charging_status;
    } else {
        debounce_cnt++;
    }
    if (debounce_cnt < (900 / 300)) {
        return ;
    }
    if (charging_status) {  // 高电平 = 正在充电
        if (last_charging_status != charging_status) {
            extern int auto_sleep_check_clear(void);
            auto_sleep_check_clear();
            if (!system_is_alarm_wakeup()) {
                music_play_res_file("Charging.mp3");
            }
#ifdef CONFIG_UI_PLAY_EMOJI
            play_face_emoji(AI_UART_CMD_VBAT_CHARGING);
#endif
            charging_cnt = 0;
        }
        // 如果已充满，且还没有播报过
        if (!already_notified_full && (chargfull || (charging_cnt % 50 == 0 ? sys_get_vbat_percent() : 0) >= 95)) {
            already_notified_full = true;
            printf("ChargeFull : %d , percent : %d\n", chargfull, sys_get_vbat_percent());
            if (!system_is_alarm_wakeup()) {
                music_play_res_file("ChargeFull.mp3");
            }
#ifdef CONFIG_LVGL_UI_ENABLE
            int lv_demo_battery_percent_update(int percent, int charge_in, int charge_full);
            lv_demo_battery_percent_update(sys_get_vbat_percent(), charging_status, chargfull);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
            play_face_emoji(AI_UART_CMD_VBAT_FULL);
#endif
        }
        if (!already_notified_full && ++charging_cnt % ((1000 / 300) * 30) == 0) {
#ifdef CONFIG_UI_PLAY_EMOJI
            play_face_emoji(AI_UART_CMD_VBAT_CHARGING);
#endif
        } else if (already_notified_full && ++charging_cnt % ((1000 / 300) * 30) == 0) {
#ifdef CONFIG_UI_PLAY_EMOJI
            play_face_emoji(AI_UART_CMD_VBAT_FULL);
#endif
        }
    } else {
        already_notified_full = false;
        chargfull = 0;
        charging_cnt = 0;
    }

    // 每次状态变化时更新电池显示
    if (last_charging_status != charging_status) {
#ifdef CONFIG_LVGL_UI_ENABLE
        int lv_demo_battery_percent_update(int percent, int charge_in, int charge_full);
        lv_demo_battery_percent_update(sys_get_vbat_percent(), charging_status, chargfull);
#endif
    }
    last_charging_status = charging_status;
#endif
}

void sys_vbat_charge_io_init(void)
{
#ifdef TCFG_CHARING_STATUS_PORT
    gpio_latch_en(TCFG_CHARING_STATUS_PORT, 0);
    gpio_direction_input(TCFG_CHARING_STATUS_PORT);
    gpio_set_die(TCFG_CHARING_STATUS_PORT, 1);
    gpio_set_pull_up(TCFG_CHARING_STATUS_PORT, 0);
    gpio_set_pull_down(TCFG_CHARING_STATUS_PORT, 0);
#endif
#ifdef TCFG_CHARGFULL_STATUS_PORT
    gpio_latch_en(TCFG_CHARGFULL_STATUS_PORT, 0);
    gpio_direction_input(TCFG_CHARGFULL_STATUS_PORT);
    gpio_set_die(TCFG_CHARGFULL_STATUS_PORT, 1);
    gpio_set_pull_up(TCFG_CHARGFULL_STATUS_PORT, 0);
    gpio_set_pull_down(TCFG_CHARGFULL_STATUS_PORT, 0);
#endif
#if (defined TCFG_CHARING_STATUS_PORT || defined TCFG_CHARGFULL_STATUS_PORT)
    sys_timer_add_to_task("sys_timer", NULL, charge_status_check, 300);
#endif
}
