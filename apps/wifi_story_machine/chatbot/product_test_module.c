#include "init.h"
#include "system/includes.h"
#include "asm/gpio.h"
#include "app_config.h"
#include "storage_device.h"
#include "generic/log.h"
#include "btstack/avctp_user.h"
#include "btstack/le/ble_api.h"
#include "btstack/btstack_task.h"
#include "btctrler/btctrler_task.h"
#include "btcontroller_modules.h"
#include "app_music.h"
#include "bt_common.h"
#include "a2dp_media_codec.h"
#include "btstack/bluetooth.h"
#include "btstack/btstack_error.h"
#include "bt_ble/bt_emitter.h"
#include "event/bt_event.h"
#include "event/key_event.h"
#include "os/os_api.h"
#include "event/key_event.h"
#include "event/device_event.h"
#include "event/net_event.h"
#include "fs/fs.h"
#include "asm/pwm.h"
#include "device/device.h"
#include "system/app_core.h"
#include "action.h"
#include "ai_uart_ctrol.h"

#ifndef PRODUCTION_TEST_PORT
#define PRODUCTION_TEST_PORT IO_PORTC_05
#endif

#ifdef CONFIG_KWS_XIAOYUXIAOYU
#define KWS_NAME "XiaoYu.mp3"
#elif (defined CONFIG_KWS_ENGLISH)
#define KWS_NAME "HelloUU.mp3"
#else
#define KWS_NAME "XiaoFei.mp3"
#endif // CONFIG_KWS_XIAOYUXIAOYU

int music_play_res_file(const char *name);
int music_play_waite(void);

static volatile char product_enter_test = 0;
enum {
    PD_TEST_ERR = BIT(31),
    PD_TEST_OK = 0,
    PD_TEST_LED = BIT(0),
    PD_TEST_KEY = BIT(1),
    PD_TEST_IR = BIT(2),
    PD_TEST_MIC = BIT(3),
    PD_TEST_DAC = BIT(4),
    PD_TEST_NET = BIT(5),
    PD_TEST_WIFI = BIT(6),
    PD_TEST_BLE = BIT(7),
    PD_TEST_SD = BIT(8),
    PD_TEST_UDISK = BIT(9),
    PD_TEST_AUX = BIT(10),
    PD_TEST_IO = BIT(11),
    PD_TEST_MIC1 = BIT(12),
    PD_TEST_MIC2 = BIT(13),
    PD_TEST_BT = BIT(14),

    PD_TEST_LED_OK = BIT(16),
    PD_TEST_KEY_OK = BIT(17),
    PD_TEST_IR_OK = BIT(18),
    PD_TEST_MIC_OK = BIT(19),
    PD_TEST_DAC_OK = BIT(20),
    PD_TEST_NET_OK = BIT(21),
    PD_TEST_WIFI_OK = BIT(22),
    PD_TEST_BLE_OK = BIT(23),
    PD_TEST_SD_OK = BIT(24),
    PD_TEST_UDISK_OK = BIT(25),
    PD_TEST_AUX_OK = BIT(26),
    PD_TEST_IO_OK = BIT(27),
    PD_TEST_MIC1_OK = BIT(28),
    PD_TEST_MIC2_OK = BIT(29),
};

static struct pd_test {
    unsigned int step;
    char app_name[32];
} PD_TEST;

int is_production_test_enter(char wake)
{
    if (wake) {
        if (wake == 1) {
            if (PD_TEST.step & PD_TEST_MIC) {
                PD_TEST.step |= PD_TEST_MIC_OK;
            }
            if (PD_TEST.step & PD_TEST_MIC1) {
                PD_TEST.step |= PD_TEST_MIC1_OK;
            }
            if (PD_TEST.step & PD_TEST_MIC2) {
                PD_TEST.step |= PD_TEST_MIC2_OK;
            }
        }
        if (wake == 2 && (PD_TEST.step & PD_TEST_IR)) {
            PD_TEST.step |= PD_TEST_IR_OK;
        }
        if (wake == 3 && ((PD_TEST.step & PD_TEST_WIFI) || (PD_TEST.step & PD_TEST_NET))) {
            if (PD_TEST.step & PD_TEST_WIFI) {
                PD_TEST.step |= PD_TEST_WIFI_OK;
            } else {
                PD_TEST.step |= PD_TEST_NET_OK;
            }
        }
    }
    return product_enter_test;
}

int is_module_production_test_enter(void)
{
    return product_enter_test & BIT(0);
}

void production_test_mic_enter(char mic)
{
    mic &= 0x3;
    product_enter_test |= BIT(3 + mic);
}

void production_test_mic_clear(char mic)
{
    mic &= 0x3;
    product_enter_test &= ~BIT(3 + mic);
}

int is_production_test_mic1_enter(void)
{
    return product_enter_test & BIT(3);
}

int is_production_test_mic2_enter(void)
{
    return product_enter_test & BIT(4);
}

int is_production_test_aec_enter(void)
{
    return product_enter_test & BIT(5);
}

void is_production_test_clear(void)
{
    product_enter_test = 0;
}

int production_io_init(void)
{
#ifdef PRODUCTION_TEST_ENABLE
    gpio_direction_input(PRODUCTION_TEST_PORT);
    gpio_set_pull_down(PRODUCTION_TEST_PORT, 0);
    gpio_set_pull_up(PRODUCTION_TEST_PORT, 1);
    gpio_set_die(PRODUCTION_TEST_PORT, 1);
#endif
#ifdef PRODUCTION_ALL_TEST_ENABLE
#ifdef PRODUCTION_ALL_TEST_PORT
#if TCFG_ADKEY_ENABLE
#if PRODUCTION_ALL_TEST_PORT == TCFG_ADKEY_PORT
#error "err in PRODUCTION_ALL_TEST_PORT = TCFG_ADKEY_PORT"
#endif
#endif
    gpio_direction_input(PRODUCTION_ALL_TEST_PORT);
    gpio_set_pull_down(PRODUCTION_ALL_TEST_PORT, 0);
    gpio_set_pull_up(PRODUCTION_ALL_TEST_PORT, 1);
    gpio_set_die(PRODUCTION_ALL_TEST_PORT, 1);
#endif
#endif
}

//#if (defined PRODUCTION_TEST_ENABLE || defined PRODUCTION_ALL_TEST_ENABLE)
//late_initcall(production_io_init);
//#endif // PRODUCTION_TEST_ENABLE

int production_test_io_get(void)
{
#ifdef PRODUCTION_TEST_ENABLE
    return !gpio_read(PRODUCTION_TEST_PORT);
#else
    return 0;
#endif
}

int production_all_test_get(void)
{
#if (defined PRODUCTION_ALL_TEST_ENABLE && defined PRODUCTION_ALL_TEST_PORT)
    return !gpio_read(PRODUCTION_ALL_TEST_PORT);
#else
    return 0;
#endif
}

int production_io_is_enter(void)
{
    int io = (production_test_io_get() | production_all_test_get());
    printf("->production_io = %d, %d %d \n", io, production_test_io_get(), production_all_test_get());
    return io;//(production_test_io_get() | production_all_test_get());
}

static int music_respon(char *name)
{
    music_play_waite();
    if (music_play_res_file(name) != 0) {
        return -1;
    }
    music_play_waite();
    return 0;
}

static int music_result(int ok)
{
    if (ok) {
        return music_respon("TestOK.mp3");
    }
    return 0;
}

int production_test_key(void)
{
#ifdef PRODUCTION_KEY_TEST_ENABLE
#if (defined TCFG_ADKEY_ENABLE || defined TCFG_IOKEY_ENABLE)
#if (TCFG_ADKEY_ENABLE || TCFG_IOKEY_ENABLE)
    PD_TEST.step |= PD_TEST_KEY;
    int to = 30 * 100;
    puts("->KEY test\n");
    music_play_res_file("TestStart.mp3");
    while (--to) {
        if (PD_TEST.step & PD_TEST_KEY_OK) {
            PD_TEST.step &= ~(PD_TEST_KEY | PD_TEST_KEY_OK);
            printf("-> PD_TEST_KEY_OK \n");
            break;
        }
        os_time_dly(1);
    }
    return to ? 0 : -1;
#endif
#endif
#endif
    return 0;
}

static const char PWMCH_H_table[] = {
    IO_PORTA_03, IO_PORTA_07, IO_PORTC_07, IO_PORTH_00, IO_PORTC_00, IO_PORTH_03, IO_PORTB_02, IO_PORTB_06
};

static const char PWMCH_L_table[] = {
    IO_PORTA_04, IO_PORTA_08, IO_PORTC_08, IO_PORTH_01, IO_PORTC_02, IO_PORTH_07, IO_PORTB_03, IO_PORTB_07
};

static int pwm_ch_port(int port, int *port0, int *port1)
{
    int p0 = -1;
    int p1 = -1;
    int n = 0;
    for (int i = 0, p; i < 32; i++) {
        p = port & BIT(i);
        if (p && i <= 7) {
            n++;
            if (p0 == -1) {
                p0 = PWMCH_H_table[i];
            } else if (p1 == -1) {
                p1 = PWMCH_H_table[i];
            }
        } else if (p && i >= 10 && i <= 17) {
            n++;
            if (p0 == -1) {
                p0 = PWMCH_L_table[i];
            } else if (p1 == -1) {
                p1 = PWMCH_L_table[i];
            }
        }
    }
    if (port0) {
        *port0 = p0;
    }
    if (port1) {
        *port1 = p1;
    }
    return p0;
}

void production_test_led(char always_on)
{
    int t = 10;
    int led_pwmch_p0 = -1;
    int led_pwmch_p1 = -1;
    int led_pwm0_p = -1;
    int led_status_p = -1;
    int led_eys_r_p = -1;
    int led_eys_l_p = -1;
    char led_open = 1;
#if (defined PRODUCTION_LED_TEST_ENABLE)
#define GPIO_SET(p,v) if((int)(p) != (int)-1){gpio_latch_en((p),0); gpio_direction_output((p),(v));}
#ifdef TCFG_LED_PWMCH_PORT
    pwm_ch_port(TCFG_LED_PWMCH_PORT, &led_pwmch_p0, &led_pwmch_p1);
#endif // TCFG_LED_PWMCH_PORT
#ifdef TCFG_LED_PWM0_PORT
    led_pwm0_p = TCFG_LED_PWM0_PORT;
#endif // TCFG_LED_PWM0_PORT
#ifdef TCFG_LED_STATUES_PORT
    led_status_p = TCFG_LED_STATUES_PORT;
#endif // TCFG_LED_STATUES_PORT
#ifdef TCFG_LED_EYA_R_PORT
    led_eys_r_p = TCFG_LED_EYA_R_PORT;
#endif // TCFG_LED_EYA_R_PORT
#ifdef TCFG_LED_EYA_L_PORT
    led_eys_l_p = TCFG_LED_EYA_L_PORT;
#endif // TCFG_LED_EYA_L_PORT
    if (led_pwmch_p0 != -1 || led_pwmch_p1 != -1 || led_pwm0_p != -1 || led_status_p != -1 || led_eys_r_p != -1 || led_eys_l_p) {
        puts("->LED test\n");
        PD_TEST.step |= PD_TEST_LED;
        if (always_on) {
            led_open = 1;
            GPIO_SET(led_pwmch_p0, led_open);
            GPIO_SET(led_pwmch_p1, led_open);
            GPIO_SET(led_pwm0_p, led_open);
            GPIO_SET(led_status_p, led_open);
            GPIO_SET(led_eys_r_p, led_open);
            GPIO_SET(led_eys_l_p, led_open);
            os_time_dly(t);
        } else {
            led_open = 1;
            GPIO_SET(led_pwmch_p0, led_open);
            GPIO_SET(led_pwmch_p1, led_open);
            GPIO_SET(led_pwm0_p, led_open);
            GPIO_SET(led_status_p, led_open);
            GPIO_SET(led_eys_r_p, led_open);
            GPIO_SET(led_eys_l_p, led_open);
            os_time_dly(t);
            led_open = 0;
            GPIO_SET(led_pwmch_p0, led_open);
            GPIO_SET(led_pwmch_p1, led_open);
            GPIO_SET(led_pwm0_p, led_open);
            GPIO_SET(led_status_p, led_open);
            GPIO_SET(led_eys_r_p, led_open);
            GPIO_SET(led_eys_l_p, led_open);
            os_time_dly(t);
            led_open = 1;
            GPIO_SET(led_pwmch_p0, led_open);
            GPIO_SET(led_pwmch_p1, led_open);
            GPIO_SET(led_pwm0_p, led_open);
            GPIO_SET(led_status_p, led_open);
            GPIO_SET(led_eys_r_p, led_open);
            GPIO_SET(led_eys_l_p, led_open);
            os_time_dly(t);
            led_open = 0;
            GPIO_SET(led_pwmch_p0, led_open);
            GPIO_SET(led_pwmch_p1, led_open);
            GPIO_SET(led_pwm0_p, led_open);
            GPIO_SET(led_status_p, led_open);
            GPIO_SET(led_eys_r_p, led_open);
            GPIO_SET(led_eys_l_p, led_open);
            os_time_dly(t);
            PD_TEST.step |= PD_TEST_LED_OK;
        }
    }
#endif
}

void production_test_timer_led(void)
{
#ifdef USED_TM1629_SHOWN
    void tm_1629_shown_pd_test(void);
    PD_TEST.step |= PD_TEST_LED;
    tm_1629_shown_pd_test();
    PD_TEST.step |= PD_TEST_LED_OK;
#endif
}

void production_test_ir(void)
{
#if (defined IRARC_UART_ENABLE && defined PRODUCTION_IR_TEST_ENABLE)
    PD_TEST.step |= PD_TEST_IR;
    void irarc_arc_lrc_learning(char first, char note);
    int irarc_production_test_send(void);
    void irarc_aurc_exit(char note);
    puts("->IR test\n");
    irarc_arc_lrc_learning(1, 0);
#endif
#if (defined TCFG_IRKEY_ENABLE && defined PRODUCTION_IR_TEST_ENABLE)
    PD_TEST.step |= PD_TEST_IR;
    puts("->IR test\n");
#endif
}

static void connet_wifi(void)
{
#ifdef PRODUCTION_ALL_TEST_WIFI_SSID
    wifi_enter_sta_mode(PRODUCTION_ALL_TEST_WIFI_SSID, PRODUCTION_ALL_TEST_WIFI_PWD);
#endif
}

static void connet_bt(void)
{
#ifdef CONFIG_BT_ENABLE
    if (PD_TEST.step & PD_TEST_BT) {
        bt_music_volume_init();
        bt_connection_enable();
    }
#endif
}
void production_test_lte_net_wifi(void)
{
    int net_ch = 0;
#if (defined CONFIG_WIFI_ENABLE && defined CONFIG_LTE_PHY_ENABLE)
    int sys_net_channel_read(void);
    net_ch = sys_net_channel_read();
    if (net_ch < 0 || (net_ch != 0 && net_ch != 1)) {
        sys_net_channel_write(NET_CH_SELECT_WIFI);
        net_ch = sys_net_channel_read();
    }
#elif (defined CONFIG_LTE_PHY_ENABLE)
    net_ch = 1;
#endif

#if (defined CONFIG_WIFI_ENABLE && defined PRODUCTION_WIFI_TEST_ENABLE)
    extern int wireless_net_init(void);
    printf("->wifi net_ch = %d \n", net_ch);
    if (net_ch == 0) {
        puts("->WIFI test\n");
        PD_TEST.step |= PD_TEST_WIFI;
        wireless_net_init();
    }
#endif

#if (defined CONFIG_LTE_PHY_ENABLE && defined PRODUCTION_NET_TEST_ENABLE)
    extern int lte_net_init(void);
    printf("->lte net_ch = %d \n", net_ch);
    if (net_ch == 1) {
        puts("->4G LTE test\n");
#ifdef LTE_POWER_ONOFF_PORT
        extern int lte_power_control(void *on);
        lte_power_control(1);
#endif
        PD_TEST.step |= PD_TEST_NET;
        lte_net_init();
    } else {
        lte_power_control(0);
    }
#endif

#if (defined CONFIG_WIFI_ENABLE && defined PRODUCTION_WIFI_TEST_ENABLE)
    if (net_ch == 0) {
        sys_timeout_add_to_task("sys_timer", NULL, connet_wifi, 1000);
    }
#endif
    sys_timeout_add_to_task("sys_timer", NULL, connet_bt, 1000);
}
void production_test_lcd(void)
{
#if (defined PRODUCTION_LCD_TEST_ENABLE && defined CONFIG_UI_ENABLE)
    void lcd_product_test(void);
    thread_fork("lcd_test", 12, 1000, 0, 0, lcd_product_test, NULL);
#endif
}
void production_test_mic(void)
{
#if (defined PRODUCTION_MIC_TEST_ENABLE)
    char mic1_test_err = 0;
    int aisp_open(u16 sample_rate);
    void aisp_close(void);
    puts("->MIC test\n");
    aisp_open(16000);
    os_time_dly(50);

    int aisp_mic_gain_suspend(void);
    int aisp_mic_gain_resum(void);
    aisp_mic_gain_suspend();

    production_test_mic_enter(0);
    PD_TEST.step |= PD_TEST_MIC1;
    int to = 5;
    while (--to) {
#ifdef CONFG_NO_KW_ENABLE
        music_play_res_file("Sin.mp3");
#else
        music_play_res_file(KWS_NAME);
#endif
        music_play_waite();
        os_time_dly(80);
        if (PD_TEST.step & PD_TEST_MIC1_OK) {
            PD_TEST.step &= ~(PD_TEST_MIC1 | PD_TEST_MIC1_OK);
            printf("-> PD_TEST_MIC_OK \n");
            break;
        }
    }
    production_test_mic_clear(0);
    if (PD_TEST.step & PD_TEST_MIC1) {
        mic1_test_err = true;
    }

#if AUDIO_RECORD_MIC_COUNT == 2
    production_test_mic_enter(1);
    PD_TEST.step |= PD_TEST_MIC2;
    to = 5;
    while (--to) {
#ifdef CONFG_NO_KW_ENABLE
        music_play_res_file("Sin.mp3");
#else
        music_play_res_file(KWS_NAME);
#endif
        music_play_waite();
        os_time_dly(80);
        if (PD_TEST.step & PD_TEST_MIC2_OK) {
            PD_TEST.step &= ~(PD_TEST_MIC2 | PD_TEST_MIC2_OK);
            printf("-> PD_TEST_MIC_OK \n");
            break;
        }
    }
    production_test_mic_clear(1);
#endif

    aisp_mic_gain_resum();

    if (PD_TEST.step & PD_TEST_MIC2) {
        if (mic1_test_err) {
            if (music_respon("TestMIC1.mp3")) {
                music_respon("TestMIC.mp3");
            }
        }
        if (music_respon("TestMIC2.mp3")) {
            music_respon("TestMIC.mp3");
        }
        music_respon("TestErr.mp3");
    } else if (mic1_test_err) {
        if (music_respon("TestMIC1.mp3")) {
            music_respon("TestMIC.mp3");
        }
        music_respon("TestErr.mp3");
    }
#endif
}

void production_test_dac(void)
{
#if (defined PRODUCTION_DAC_TEST_ENABLE)
    puts("->DAC test\n");
    music_respon("PdTestLR.mp3");
#endif
}

static void ble_test_stop(void)
{
#ifdef CONFIG_BT_ENABLE
    if ((PD_TEST.step & PD_TEST_BT)) {
        bt_music_play_set_stop();
        bt_connection_disable();
    } else {
#ifdef PRODUCTION_BT_TEST_ENABLE
        bt_music_play_set_stop();
        bt_connection_disable();
#endif
    }
#endif
}

void production_test_ble(void)
{
#if (defined CONFIG_BT_ENABLE && defined PRODUCTION_BT_TEST_ENABLE)
    extern int bt_music_play_set_stop(void);
    puts("->BLE test\n");
    os_time_dly(120);
    PD_TEST.step |= PD_TEST_BLE;
    bt_music_volume_init();
    bt_connection_enable();
#endif
}

void production_test_sd(void)
{
#if (defined CONFIG_SD_MUSIC_MODE_ENABLE && defined PRODUCTION_SD_TEST_ENABLE)
    PD_TEST.step |= PD_TEST_SD;
    puts("->SD test\n");
#endif
}

void production_test_udisk(void)
{
#if (defined CONFIG_USB_DISK_MUSIC_MODE_ENABLE && defined PRODUCTION_UDISK_TEST_ENABLE)
    PD_TEST.step |= PD_TEST_UDISK;
    puts("->USBDISK test\n");
#endif

}

void production_test_aux(void)
{
#if (defined CONFIG_AUX_MUSIC_MODE_ENABLE && defined PRODUCTION_AUX_TEST_ENABLE)
    extern int aux_det_is_ok(void);
    extern int audio_app_mode_switch_set(char *name, char note);
    PD_TEST.step |= PD_TEST_AUX;
    puts("->AUX test\n");
#endif
}

void production_test_restore_factory_settings(void)//恢复出厂设置
{
#if (defined CONFIG_WIFI_ENABLE || defined CONFIG_LTE_PHY_ENABLE)
    void system_restore_factory_settings(void);//wifi恢复出厂设置
    puts("->restore_factory_settings \n");
    system_restore_factory_settings();
#endif
}

static int product_test_key_event_handler(struct key_event *key)
{
    if (product_enter_test & BIT(0)) {//模组厂测不需要按键功能
        return true;
    }

    switch (key->action) {
    case KEY_EVENT_LONG:
        if (key->value == KEY_OK || key->value == KEY_POWER || key->value == KEY_MODE) {
#if (defined TCFG_VBAT_CHECK_EN && TCFG_VBAT_CHECK_EN == 1)
            extern void sys_power_poweroff(void);
            extern int music_play_waite(void);
            music_play_stop_all();
            music_play_res_file("PwrOff.mp3");
            music_play_waite();
            sys_power_poweroff();
#else
            music_play_stop_all();
            music_play_res_file("SysReset.mp3");
            music_play_waite();
            system_reset();
#endif
        }
    case KEY_EVENT_CLICK:
    case KEY_EVENT_DOUBLE_CLICK:
    case KEY_EVENT_TRIPLE_CLICK:
        if (PD_TEST.step & PD_TEST_KEY) {
            PD_TEST.step |= PD_TEST_KEY_OK;
        } else if (PD_TEST.step & PD_TEST_IR) {
            PD_TEST.step |= PD_TEST_IR_OK;
        }
        break;
    case KEY_EVENT_FOURTH_CLICK:
        if (key->value == KEY_OK) { //4G和WiFi切换
#if (defined CONFIG_WIFI_ENABLE && defined CONFIG_LTE_PHY_ENABLE)
            int sys_net_channel_read(void);
            int net_ch = sys_net_channel_read();
            if (net_ch < 0) {
                sys_net_channel_write(NET_CH_SELECT_WIFI);
                net_ch = sys_net_channel_read();
            } else {
                net_ch = net_ch ? 0 : 1;
                sys_net_channel_write(net_ch);
                if (sys_net_channel_read() == net_ch) {
                    extern void sys_power_poweroff(void);
                    extern int music_play_waite(void);
                    aisp_all_pause(1);
                    music_play_waite();
                    music_play_res_file("SysReset.mp3");
                    music_play_waite();
                    system_reset();
                }
            }
#endif
        }
        if (PD_TEST.step & PD_TEST_KEY) {
            PD_TEST.step |= PD_TEST_KEY_OK;
        } else if (PD_TEST.step & PD_TEST_IR) {
            PD_TEST.step |= PD_TEST_IR_OK;
        }
        break;
    case KEY_EVENT_FIRTH_CLICK:;//五连击
#ifdef CONFIG_WFT_ENBALE
        extern int dev_iot_uart_task_init(void);
        dev_iot_uart_task_init();
#endif
        break;
    default:
        break;
    }
    return true;
}

static int product_test_device_event_handler(struct sys_event *sys_eve)
{
    struct device_event *device_eve = (struct device_event *)sys_eve->payload;
    int ret = true;
    extern int audio_app_mode_switch_set(char *name, char note);
    /* SD卡插拔处理 */
    if (sys_eve->from == DEVICE_EVENT_FROM_SD) {
        switch (device_eve->event) {
        case DEVICE_EVENT_IN://SD卡插入
            printf("->SD IN : %s\n", device_eve->arg);
            if (PD_TEST.step & PD_TEST_SD) {
                PD_TEST.step |= PD_TEST_SD_OK;
            }
            break;
        case DEVICE_EVENT_OUT://SD卡拔出，释放资源f
            break;
        default :
            ret = false;
            break;
        }
#if TCFG_UDISK_ENABLE
        /* U盘插拔处理 */
    } else if (sys_eve->from == DEVICE_EVENT_FROM_USB_HOST && !strncmp((const char *)device_eve->value, "udisk", 5)) {
        switch (device_eve->event) {
        case DEVICE_EVENT_IN:
#ifdef CONFIG_USB_DISK_MUSIC_MODE_ENABLE
            if (PD_TEST.step & PD_TEST_UDISK) {
                PD_TEST.step |= PD_TEST_UDISK_OK;
            }
#endif
            break;
        case DEVICE_EVENT_OUT:
            ret = false;
            break;
        default :
            ret = false;
            break;
        }
#endif

#ifdef CONFIG_AUX_MUSIC_MODE_ENABLE
        /* AUX插拔处理 */
    } else if (sys_eve->from == DEVICE_EVENT_FROM_LINEIN) {
        switch (device_eve->event) {
        case DEVICE_EVENT_IN:
            puts("->AUX IN\n");
            if (PD_TEST.step & PD_TEST_AUX) {
                PD_TEST.step |= PD_TEST_AUX_OK;
            }
            break;
        case DEVICE_EVENT_OUT:
            puts("->AUX OUT\n");
            break;
        default :
            ret = false;
            break;
        }
#endif
    }
    return ret;
}

static int product_test_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        return product_test_key_event_handler((struct key_event *)event->payload);
    case SYS_NET_EVENT:
        ;
        extern int ai_net_event_handler(struct net_event * event);
        return ai_net_event_handler((struct net_event *)event->payload);
#ifdef CONFIG_BT_ENABLE
    case SYS_BT_EVENT:
        ;
        struct bt_event *bt = (struct bt_event *)event->payload;
        if ((bt->event != BT_STATUS_INIT_OK && (PD_TEST.step & PD_TEST_BLE || PD_TEST.step & PD_TEST_WIFI)) || (PD_TEST.step & PD_TEST_BT)) {
            extern int app_music_bt_event_handler(struct sys_event * event);
            return app_music_bt_event_handler(event);
        }
        return false;
#endif
    default:
        return false;
    }
}

static int product_test_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;
    case APP_STA_START:
        switch (it->action) {
        case ACTION_HOME_MAIN:
            break;
        case ACTION_MUSIC_PLAY_MAIN:
            break;
        case ACTION_MUSIC_PLAY_VOICE_PROMPT:
            if (!strcmp((const char *)it->data, "BtSucceed.mp3")) {
                if (PD_TEST.step & PD_TEST_BLE) {
                    PD_TEST.step |= PD_TEST_BLE_OK;
                }
            }
            break;
        default:
            break;
        }
        break;
    case APP_STA_PAUSE:
        break;
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
        break;
    case APP_STA_DESTROY:
        break;
    }

    return 0;
}

static const struct application_operation product_test_ops = {
    .state_machine  = product_test_state_machine,
    .event_handler  = product_test_event_handler,
};

REGISTER_APPLICATION(product_test) = {
    .name   = "product_test",
    .ops    = &product_test_ops,
    .state  = APP_STA_DESTROY,
};

void product_function_app_start(void)
{
    struct intent it;
    struct application *app = get_current_app();
    if (app) {
        init_intent(&it);
        it.name = app->name;
        it.action = ACTION_STOP;    //退出当前模式
        start_app(&it);
    }
    init_intent(&it);
    it.name = "product_test";
    it.action = ACTION_HOME_MAIN;
    start_app(&it);
}

#ifdef TCFG_ADKEY_PORT
#if TCFG_ADKEY_PORT == IO_PORTB_01
#define KEY_PORT    (u8)-1
#endif
#endif

#ifdef TCFG_IOKEY_PORT
#if TCFG_IOKEY_PORT == IO_PORTB_01
#define KEY_PORT    (u8)-1
#endif
#endif

#ifndef KEY_PORT
#define KEY_PORT    IO_PORTB_01
#endif

static const u8 product_io_test_group[6][8] = {
    {IO_PORT_PR_01,     IO_PORT_PR_00,      IO_PORTB_00,    KEY_PORT,      IO_PORTB_03, (u8) -1, (u8) -1, (u8) -1},
    //{IO_PORT_USB_DPB,   IO_PORT_USB_DMB,    IO_PORTB_00,    IO_PORTB_01,    IO_PORTB_03,    (u8)-1,      (u8)-1, (u8)-1},
    {IO_PORTA_00,       IO_PORTA_05,        IO_PORTA_06,    IO_PORTA_07,    IO_PORTA_08,    IO_PORTA_09, (u8) -1, (u8) -1},
    {IO_PORTH_00,       IO_PORTH_01,        IO_PORTH_02,    IO_PORTH_03, (u8) -1, (u8) -1, (u8) -1, (u8) -1},
    {IO_PORTH_04,       IO_PORTH_07, (u8) -1, (u8) -1, (u8) -1, (u8) -1, (u8) -1, (u8) -1},
    {IO_PORTC_00,       IO_PORTC_01,        IO_PORTC_02,    IO_PORTC_03,    IO_PORTC_04, (u8) -1, (u8) -1, (u8) -1},
    {IO_PORTC_07,       IO_PORTC_08,        IO_PORTC_09,    IO_PORTC_10, (u8) -1, (u8) -1, (u8) -1, (u8) -1},
};

static int product_all_io_set_init(void)
{
    u32 IO_PORT;
    JL_PORT_FLASH_TypeDef *gpio[] = {JL_PORTA, JL_PORTB, JL_PORTC, JL_PORTD, JL_PORTE, JL_PORTF, JL_PORTG, JL_PORTH};
    for (u8 p = 0; p < 8; ++p) {
        //flash sdram PD PE PF PG口不能进行配置,由内部完成控制
        if (gpio[p] == JL_PORTD || gpio[p] == JL_PORTE || gpio[p] == JL_PORTF || gpio[p] == JL_PORTG) {
            continue;
        }
        for (u8 i = 0; i < IO_GROUP_NUM; ++i) {
            IO_PORT = IO_PORTA_00 + p * IO_GROUP_NUM + i;
#ifdef TCFG_DEBUG_PORT
            if (IO_PORT == TCFG_DEBUG_PORT) {
                continue;
            }
#endif
            gpio_latch_en(IO_PORT, 0);
            gpio_set_pull_up(IO_PORT, 0);
            gpio_set_pull_down(IO_PORT, 1);
            gpio_direction_input(IO_PORT);
            gpio_set_die(IO_PORT, 1);
        }
    }
    for (u8 i = IO_PORT_PR_00; i <= IO_PORT_PR_03; ++i) {
#ifdef TCFG_DEBUG_PORT
        if (IO_PORT == TCFG_DEBUG_PORT) {
            continue;
        }
#endif
        gpio_latch_en(i, 0);
        gpio_set_pull_up(i, 0);
        gpio_set_pull_down(i, 1);
        gpio_direction_input(i);
        gpio_set_die(i, 1);
    }
    for (u8 i = IO_PORT_USB_DPA; i <= IO_PORT_USB_DMB; ++i) {
#ifdef TCFG_DEBUG_PORT
        if (IO_PORT == TCFG_DEBUG_PORT) {
            continue;
        }
#endif
#ifdef PRODUCTION_YIERD_UART_CMD_ENABLE
        if ((i == IO_PORT_USB_DPA) || (i == IO_PORT_USB_DMA)) {
            continue;
        }
#endif
        gpio_latch_en(i, 0);
        gpio_set_pull_up(i, 0);
        gpio_set_pull_down(i, 1);
        gpio_direction_input(i);
        gpio_set_die(i, 1);
    }
    p33_and_1byte(P3_PINR_CON, ~BIT(0));//长按4s/8s关闭
    memset(JL_PWM, 0, sizeof(JL_MCPWM_TypeDef));
    JL_CTM->CON0 &= ~BIT(0);
    JL_GPCNT->CON &= ~BIT(0);
    JL_IR->RFLT_CON &= ~BIT(0);
    JL_IIC->CON0 &= ~BIT(0);
    JL_SD0->CON1 &= ~BIT(0);
    JL_SD1->CON1 &= ~BIT(0);
    JL_EMI->CON0 &= ~BIT(0);
    JL_IMD->CON0 &= ~BIT(0);
    JL_PAP->CON &= ~BIT(0);
    JL_ISC0->COM_CON &= ~BIT(0);
    JL_ISC1->COM_CON &= ~BIT(0);
    JL_RDEC->CON &= ~BIT(0);
    JL_SPI1->CON &= ~BIT(0);
    JL_SPI2->CON &= ~BIT(0);
    JL_SPI3->CON &= ~BIT(0);
}

static int product_all_io_test(void)
{
    int i, j, k = 0;
    int ret = 0;
    int check_num = 0;
    int check_ok_num = 0;
    int check_err = 0;

    PD_TEST.step |= PD_TEST_IO;
    product_all_io_set_init();
    for (i = 0; i < 6; i++) {
        check_num = 0;
        check_ok_num = 0;
        for (j = 0; j < 8; j++) {

#ifdef PRODUCTION_YIERD_UART_CMD_ENABLE
            if ((product_io_test_group[i][j] == IO_PORT_USB_DPA) || (product_io_test_group[i][j] == IO_PORT_USB_DMA)) {
                continue;
            }
#endif

#ifdef TCFG_DEBUG_PORT
            if (product_io_test_group[i][j] != (u8) -1 && product_io_test_group[i][j] != TCFG_DEBUG_PORT) {
#else
            if (product_io_test_group[i][j] != (u8) -1) {
#endif
                check_num++;
                for (k = 0; k < 8; k++) {
                    if (product_io_test_group[i][k] != (u8) -1 && product_io_test_group[i][k] != product_io_test_group[i][j]) {
#ifdef TCFG_DEBUG_PORT
                        if (product_io_test_group[i][k] == TCFG_DEBUG_PORT) {
                            continue;
                        }
#endif
                        gpio_direction_input(product_io_test_group[i][k]);
                        gpio_set_pull_up(product_io_test_group[i][k], 0);
                        gpio_set_pull_down(product_io_test_group[i][k], 1);
                        gpio_set_die(product_io_test_group[i][k], 1);
                    }
                }
                gpio_direction_output(product_io_test_group[i][j], 1);

                int IO_G = product_io_test_group[i][j] / IO_GROUP_NUM;
                int IO_P = product_io_test_group[i][j] % IO_GROUP_NUM;
                if (product_io_test_group[i][j] >= IO_PORT_PR_00 && product_io_test_group[i][j] <= IO_PORT_PR_03) {
                    printf("SET 1 -> PR%02d \n", product_io_test_group[i][j] - IO_PORT_PR_00);
                } else if (product_io_test_group[i][j] >= IO_PORT_USB_DPA && product_io_test_group[i][j] <= IO_PORT_USB_DMB) {
                    if (product_io_test_group[i][j] <= IO_PORT_USB_DMA) {
                        printf("SET 1 -> FUSB_%s \n", (product_io_test_group[i][j] == IO_PORT_USB_DPA) ? "DP" : "DN");
                    } else {
                        printf("SET 1 -> HUSB_%s \n", (product_io_test_group[i][j] == IO_PORT_USB_DPB) ? "DP" : "DN");
                    }
                } else {
                    printf("SET 1 -> P%c%02d \n", 'A' + IO_G, IO_P);
                }

                delay_us(2000);
                for (k = 0; k < 8; k++) {
                    if (product_io_test_group[i][k] != (u8) -1 && product_io_test_group[i][k] != product_io_test_group[i][j]) {
#ifdef TCFG_DEBUG_PORT
                        if (product_io_test_group[i][k] == TCFG_DEBUG_PORT) {
                            continue;
                        }
#endif
                        int io_status = gpio_read(product_io_test_group[i][k]);
                        //printf("io_status = %d , io = %d \n",io_status,product_io_test_group[i][k]);
                        if (io_status) {
                            check_ok_num++;
                            //printf("check_ok num = %d , io = %d \n",check_ok_num,product_io_test_group[i][k]);
                        } else {
                            IO_G = product_io_test_group[i][k] / IO_GROUP_NUM;
                            IO_P = product_io_test_group[i][k] % IO_GROUP_NUM;
                            if (product_io_test_group[i][k] >= IO_PORT_PR_00 && product_io_test_group[i][k] <= IO_PORT_PR_03) {
                                printf("check_IO_err -> PR%02d \n", product_io_test_group[i][k] - IO_PORT_PR_00);
                            } else if (product_io_test_group[i][k] >= IO_PORT_USB_DPA && product_io_test_group[i][k] <= IO_PORT_USB_DMB) {
                                if (product_io_test_group[i][k] <= IO_PORT_USB_DMA) {
                                    printf("check_IO_err -> FUSB_%s \n", (product_io_test_group[i][k] == IO_PORT_USB_DPA) ? "DP" : "DN");
                                } else {
                                    printf("check_IO_err -> HUSB_%s \n", (product_io_test_group[i][k] == IO_PORT_USB_DPB) ? "DP" : "DN");
                                }
                            } else {
                                printf("check_IO_err -> P%c%02d \n", 'A' + IO_G, IO_P);
                            }
                        }
                    }
                }
            }
        }
        if (check_num && ((check_num - 1) * check_num) != check_ok_num) {
            check_err++;
            printf("->check_IO_err i = %d , check_num = %d , check_ok_num = %d \n", i, check_num, check_ok_num);
        }
    }
    if (!check_err) {
        PD_TEST.step &= ~(PD_TEST_IO | PD_TEST_IO_OK);
        printf("-> PD_TEST_IO_OK \n");
    }
    return check_err;
}

static int product_function_test_task(void *priv)
{
    int cnt = 0;
    int to = 60 * 10; //60秒超时
    if ((int)priv == 1) {
        PD_TEST.step |= PD_TEST_BT;
        goto __func_test;
    } else if ((int)priv == 2) {//进入模组厂测，开启经典蓝牙
        PD_TEST.step |= PD_TEST_BT;
    }

#ifdef PRODUCTION_TEST_ENABLE
    if (production_test_io_get() || (product_enter_test & BIT(0))) {
        extern void dac_mute_control_init(void);
        extern void dac_mute_control(char enable, char force);
        dac_mute_control_init();
        dac_mute_control(0, 0);
        product_enter_test |= BIT(0);

#ifdef PRODUCTION_YIERD_UART_CMD_ENABLE
        extern int ai_at_report_audio_test_result(int status);
        extern int ai_at_report_io_test_result(int status);
        extern int ai_at_test_finish(void);
        extern u16 ai_at_get_test_code(void);
        while (!ai_at_is_test_start()) {
            os_time_dly(50);
        }
        u16 tcode = ai_at_get_test_code();
#endif

#ifdef CONFIG_BT_ENABLE
        if (PD_TEST.step & PD_TEST_BT) {
            bt_ble_module_init();
        }
#endif
        music_play_waite();
        music_play_res_file("TestProdEnt.mp3");
        music_play_waite();

#ifdef CONFIG_BT_ENABLE
        if (PD_TEST.step & PD_TEST_BT) {
            bt_music_volume_init();
            bt_connection_enable();
        }
#endif
        /******************************** IO TEST **********************************/
#ifdef PRODUCTION_YIERD_UART_CMD_ENABLE
        if (tcode & 0x0001)
#endif
        {
            int io_check_err = product_all_io_test();

            if (!io_check_err) {
#ifdef PRODUCTION_YIERD_UART_CMD_ENABLE
                music_respon("TestIO.mp3");
                music_result(1);
                ai_at_report_io_test_result(true);
#endif
                printf("[TEST] start IO TEST PASS\n");
            } else {
                music_respon("TestIO.mp3");
                music_respon("TestErr.mp3");
#ifdef PRODUCTION_YIERD_UART_CMD_ENABLE
                ai_at_report_io_test_result(false);
#endif
            }
        }
        /************************** MIC TEST  **********************************/
#ifdef PRODUCTION_YIERD_UART_CMD_ENABLE
        if (tcode & 0x0002)
#endif
        {
            music_play_res_file("PdTestLR.mp3");
            music_play_waite();
            os_time_dly(10);

            extern int ai_speaker_mode_init(void);
            ai_speaker_mode_init();

            production_test_lcd();
            production_test_mic();
            char test_mic_ok = 0;
            if ((PD_TEST.step & PD_TEST_MIC2) && (PD_TEST.step & PD_TEST_MIC1)) {
                test_mic_ok = -3;
            } else if (PD_TEST.step & PD_TEST_MIC2) {
                test_mic_ok = -2;
            } else if (PD_TEST.step & PD_TEST_MIC1) {
                test_mic_ok = -1;
            }
#ifdef PRODUCTION_YIERD_UART_CMD_ENABLE
            if (test_mic_ok != 0) {
                ai_at_report_audio_test_result(test_mic_ok);
            } else {
                ai_at_report_audio_test_result(true);
            }
#endif
            char test_io_ok = (PD_TEST.step & (~PD_TEST_BT)) == PD_TEST_OK ? 1 : 0;
            music_result(test_io_ok);
            if (!test_io_ok) {
                if (PD_TEST.step & PD_TEST_IO) {
                    music_respon("TestIO.mp3");
                    music_respon("TestErr.mp3");
                }
            }
#ifdef PRODUCTION_YIERD_UART_CMD_ENABLE
            ai_at_test_finish();
#endif
        }
    }
#endif

__func_test:
#ifdef PRODUCTION_ALL_TEST_ENABLE
    if (production_all_test_get() || (product_enter_test & BIT(1))) {
        extern void dac_mute_control_init(void);
        extern void dac_mute_control(char enable, char force);
        dac_mute_control_init();
        dac_mute_control(0, 0);
        product_enter_test |= BIT(1);
        int net_ch = 0;
#ifdef CONFIG_LTE_PHY_ENABLE
        net_ch = sys_net_channel_read();
#endif

        music_play_waite();
        music_play_res_file("TestProdEnt.mp3");

        production_test_lcd();
        if (net_ch != 1) {
            production_test_led(0);
            production_test_timer_led();
            music_play_waite();
            if (production_test_key()) {
                goto __err;
            }
            production_test_ir();
            production_test_dac();
            production_test_sd();
            production_test_udisk();
            production_test_aux();
        }
        production_test_lte_net_wifi();
        printf("->1net_ch = %d\n", net_ch);
        if (net_ch != 1) {
            production_test_ble();
            production_test_mic();
        }
        to = 30 * 10; //20秒超时
        while (--to && (PD_TEST.step & (~PD_TEST_BT))) {
            if (PD_TEST.step & PD_TEST_WIFI || PD_TEST.step & PD_TEST_NET) {
                if (sys_connect_net_success()) {
                    if (PD_TEST.step & PD_TEST_WIFI) {
                        PD_TEST.step |= PD_TEST_WIFI_OK;
                    }
                    if (PD_TEST.step & PD_TEST_NET) {
                        PD_TEST.step |= PD_TEST_NET_OK;
                    }
                }
            }
            for (int i = 0; i < 31; i++) {
                int pd = PD_TEST.step & BIT(i);
                switch (pd) {
                case PD_TEST_LED:
                    if (PD_TEST.step & PD_TEST_LED_OK) {
                        PD_TEST.step &= ~(PD_TEST_LED | PD_TEST_LED_OK);
                        printf("-> PD_TEST_LED_OK \n");
                    }
                    break;
                case PD_TEST_KEY:
                    if (PD_TEST.step & PD_TEST_KEY_OK) {
                        PD_TEST.step &= ~(PD_TEST_KEY | PD_TEST_KEY_OK);
                        printf("-> PD_TEST_KEY_OK \n");
                    }
                    break;
                case PD_TEST_IR:
                    if (PD_TEST.step & PD_TEST_IR_OK) {
                        PD_TEST.step &= ~(PD_TEST_IR | PD_TEST_IR_OK);
                        printf("-> PD_TEST_IR_OK \n");
                    }
                    break;
                case PD_TEST_MIC:
                    if (PD_TEST.step & PD_TEST_MIC_OK) {
                        PD_TEST.step &= ~(PD_TEST_MIC | PD_TEST_MIC_OK);
                        printf("-> PD_TEST_MIC_OK \n");
                    }
                    break;
                case PD_TEST_MIC1:
                case PD_TEST_MIC2:
                    if (PD_TEST.step == PD_TEST_MIC1 || PD_TEST.step == PD_TEST_MIC2 || PD_TEST.step == (PD_TEST_MIC1 | PD_TEST_MIC2)) {
                        goto __err;
                    }
                    break;
                case PD_TEST_DAC:
                    if (PD_TEST.step & PD_TEST_DAC_OK) {
                        PD_TEST.step &= ~(PD_TEST_DAC | PD_TEST_DAC_OK);
                        printf("-> PD_TEST_DAC_OK \n");
                    }
                    break;
                case PD_TEST_NET:
                    if (PD_TEST.step & PD_TEST_NET_OK) {
                        PD_TEST.step &= ~(PD_TEST_NET | PD_TEST_NET_OK);
                        printf("-> PD_TEST_NET_OK \n");
                    }
                    break;
                case PD_TEST_WIFI:
                    if (PD_TEST.step & PD_TEST_WIFI_OK) {
                        PD_TEST.step &= ~(PD_TEST_WIFI | PD_TEST_WIFI_OK);
                        printf("-> PD_TEST_WIFI_OK \n");
                    }
                    break;
                case PD_TEST_BLE:
                    if (PD_TEST.step & PD_TEST_BLE_OK) {
                        PD_TEST.step &= ~(PD_TEST_BLE | PD_TEST_BLE_OK);
                        printf("-> PD_TEST_BLE_OK \n");
                    }
                    break;
                case PD_TEST_SD:
                    if (PD_TEST.step & PD_TEST_SD_OK) {
                        PD_TEST.step &= ~(PD_TEST_SD | PD_TEST_SD_OK);
                        printf("-> PD_TEST_SD_OK \n");
                    }
                    break;
                case PD_TEST_UDISK:
                    if (PD_TEST.step & PD_TEST_UDISK_OK) {
                        PD_TEST.step &= ~(PD_TEST_UDISK | PD_TEST_UDISK_OK);
                        printf("-> PD_TEST_UDISK_OK \n");
                    }
                    break;
                case PD_TEST_AUX:
                    if (PD_TEST.step & PD_TEST_AUX_OK) {
                        PD_TEST.step &= ~(PD_TEST_AUX | PD_TEST_AUX_OK);
                        printf("-> PD_TEST_AUX_OK \n");
                    }
                    break;
                }
            }
            os_time_dly(10);
        }
__err:
        ;
        char test_ok = (PD_TEST.step & (~PD_TEST_BT)) == PD_TEST_OK ? 1 : 0;
        music_result(test_ok);
        if (!test_ok) {
            if (PD_TEST.step & PD_TEST_KEY) {
                music_respon("TestKey.mp3");
                music_respon("TestErr.mp3");
            }
            if (PD_TEST.step & PD_TEST_IR) {
                music_respon("TestIR.mp3");
                music_respon("TestErr.mp3");
            }
            if (PD_TEST.step & PD_TEST_MIC) {
                music_respon("TestMIC.mp3");
                music_respon("TestErr.mp3");
            }
            if (PD_TEST.step & PD_TEST_DAC) {
                music_respon("TestDac.mp3");
                music_respon("TestErr.mp3");
            }
            if (PD_TEST.step & PD_TEST_NET) {
                music_respon("TestNet.mp3");
                music_respon("TestErr.mp3");
            }
            if (PD_TEST.step & PD_TEST_WIFI) {
                music_respon("TestWiFi.mp3");
                music_respon("TestErr.mp3");
            }
            if (PD_TEST.step & PD_TEST_BLE) {
                music_respon("TestBLE.mp3");
                music_respon("TestErr.mp3");
            }
            if (PD_TEST.step & PD_TEST_SD) {
                music_respon("TestSD.mp3");
                music_respon("TestErr.mp3");
            }
            if (PD_TEST.step & PD_TEST_UDISK) {
                music_respon("TestUdisk.mp3");
                music_respon("TestErr.mp3");
            }
            if (PD_TEST.step & PD_TEST_AUX) {
                music_respon("TestAUX.mp3");
                music_respon("TestErr.mp3");
            }
        }
        printf("->PD_TEST.step = 0x%x\n", PD_TEST.step);
#if (defined CONFIG_WIFI_ENABLE && defined CONFIG_LTE_PHY_ENABLE && defined PRODUCTION_NET_TEST_ENABLE)
        printf("->2net_ch = %d\n", net_ch);
        if (net_ch == 0) {
            if (test_ok) {
                music_respon("TestWiFi.mp3");
                music_play_waite();
                music_respon("TestOK.mp3");
            }
#ifdef PRODUCTION_ALL_TEST_PORT
            sys_net_channel_write(NET_CH_SELECT_LTE);
            wifi_sta_info_clear();
            music_play_waite();
            music_play_res_file("SysReset.mp3");
            music_play_waite();
            system_reset();
#endif
        }
#endif
#if (defined CONFIG_BT_ENABLE && defined PRODUCTION_BT_TEST_ENABLE)
        ble_test_stop();
#endif
        if ((PD_TEST.step & PD_TEST_BT)) {
            sys_timeout_add_to_task("sys_timer", NULL, ble_test_stop, 30 * 1000);//1分钟后断开蓝牙
        }
        //production_test_restore_factory_settings();
        int sh_test_result = 30;
        production_test_led(test_ok);
        while (!test_ok && --sh_test_result) {
            production_test_led(test_ok);
        }
        is_production_test_clear();
        extern int audio_app_mode_switch_set(char *name, char note);
        if (PD_TEST.app_name[0] != 0) {
            audio_app_mode_switch_set(PD_TEST.app_name, 0);
        }
        memset(PD_TEST.app_name, 0, sizeof(PD_TEST.app_name));
    }
#endif
    return 0;
}

int product_test_task_init(void *priv)
{
    struct application *app = get_current_app();

    if (app && app->name) {
        strcpy(PD_TEST.app_name, app->name);
    } else {
        memset(PD_TEST.app_name, 0, sizeof(PD_TEST.app_name));
    }
    product_function_app_start();
    return thread_fork("pdfunc_test", 10, 1600, 0, 0, product_function_test_task, priv);
}

void enter_product_func_test(void *priv)
{
    product_enter_test |= BIT(1);
    product_test_task_init(priv);
}

void module_enter_product_func_test(void *priv)
{
    product_enter_test |= BIT(0);
    product_test_task_init(priv);
}
