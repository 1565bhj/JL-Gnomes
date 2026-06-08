#include "app_config.h"
#include "system/includes.h"
#include "device/device.h"
#include <time.h>
#include <sys/time.h>
#include "uart.h"
#include "syscfg/syscfg_id.h"
#include "event/key_event.h"
#include "event/net_event.h"
#include "os/os_api.h"
#include "asm/port_waked_up.h"
#include "asm/gpio.h"
#include "asm/p33.h"
#include "asm/adc_api.h"

#ifdef TCFG_AD_VOLUME_PORT
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
static u16 last_adc_volume = 0;
static u16 adc_channle = 0;
static int ad_volume_scan(void)
{
    u32 adcval = adc_get_value(adc_channle);
    u32 adc_volume = adcval * MAX_VOLUME_VALUE / 1024;

    adc_volume = adc_volume > MAX_VOLUME_VALUE ? MAX_VOLUME_VALUE : adc_volume;
    adc_volume = MAX_VOLUME_VALUE - adc_volume;//音量反计算

    adc_volume = adc_volume < MIN_VOLUME_VALUE ? MIN_VOLUME_VALUE : adc_volume;
    adc_volume = adc_volume > MAX_VOLUME_VALUE ? MAX_VOLUME_VALUE : adc_volume;

//    printf("-> adc %d , %d \n",adcval, adc_volume);
    if (!last_adc_volume) {
        last_adc_volume = adc_volume;
    }
    int diff = last_adc_volume >= adc_volume ? (last_adc_volume - adc_volume) : (adc_volume - last_adc_volume);
    if (last_adc_volume != adc_volume && diff >= 5) {
//        printf("-> adc_volume ch = %d, %d , %d \n",adc_channle, adcval, adc_volume);
        struct key_event key = {0};
        key.type = KEY_EVENT_USER;
        key.action = KEY_EVENT_CLICK;
        if (adc_volume < last_adc_volume) {
            key.value = KEY_VOLUME_DEC;
        } else {
            key.value = KEY_VOLUME_INC;
        }
        last_adc_volume = adc_volume;
        key_event_notify(KEY_EVENT_FROM_USER, &key);
    }
    return 0;
}
static int ad_volume_init(void)
{
    if (production_io_is_enter() || is_production_test_enter(0)) {
        return 0;
    }
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 2; j++) {
            if (adc_port[i][0] == TCFG_AD_VOLUME_PORT) {
                adc_channle = adc_port[i][1];
                adc_add_sample_ch(adc_channle);
                gpio_direction_input(TCFG_AD_VOLUME_PORT);
                gpio_set_pull_down(TCFG_AD_VOLUME_PORT, 0);
                gpio_set_pull_up(TCFG_AD_VOLUME_PORT, 0);
                gpio_set_die(TCFG_AD_VOLUME_PORT, 0);
                sys_hi_timer_add(NULL, ad_volume_scan, 100);
                return 0;
            }
        }
    }
    return 0;
}
late_initcall(ad_volume_init);
#endif
