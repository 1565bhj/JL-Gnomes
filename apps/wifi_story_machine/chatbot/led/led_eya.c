#include "system/includes.h"
#include "asm/gpio.h"
#include "app_config.h"
#include "storage_device.h"
#include "generic/log.h"
#include "os/os_api.h"
#include "event/key_event.h"
#include "event/device_event.h"
#include "event/net_event.h"
#include "fs/fs.h"
#include "asm/pwm.h"
#include "device/device.h"

#ifdef LED_EYA_OPEN
static int led_eys_send_msg(int message, char notice);
extern void tm_1629_shown_face(int on);
static volatile char led_eya_step = 0;
void led_eya_wake(char start)
{
    if (start) {
        led_eys_send_msg(2, 0);
    }
    led_eys_send_msg(1, 0);
}
void led_eya_sleep(void)
{
    led_eys_send_msg(2, 0);
}
int led_eya_get_work(void)
{
    return led_eya_step == 1;
}
#define LED_EYA_NAME     "led_eya_task"
static int led_eys_send_msg(int message, char notice)
{
    int ret;
    char retry = 0;
    do {
        ret = os_taskq_post(LED_EYA_NAME, 2, message, notice);
        if (ret != OS_NO_ERR) {
            if (ret != OS_Q_FULL) {
                return -1;
            }
            os_time_dly(5);
            retry++;
        } else {
            break;
        }
    } while (retry < 5);
    if (retry == 5) {
        printf("warnning : music_buf OS_Q_FULL\n");
    }
    return ret;
}

static int led_eya_task(void *priv)
{
    int err, res;
    int msg[4];
    int cnt = 0;
    int accept = 0;
    int wake_step = 0;

    while (1) {
        res = os_taskq_accept(ARRAY_SIZE(msg), msg);
        //res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        if (res == OS_TASK_DEL_IDLE) {
            break;
        } else if (res == OS_TASKQ) {
            if (msg[0] == Q_USER) {
                switch (msg[1]) {
                case 1:
                    puts("->led_eya_wake\n");
                    led_eya_step = 1;
                    wake_step = led_eya_step;
                    cnt = 0;
                    break;
                case 2:
                    puts("->led_eya_stop\n");
                    led_eya_step = 2;
                    wake_step = led_eya_step;
                    cnt = 0;
                    break;
                case 3:
#ifdef USED_TM1629_EAY
                    tm_1629_shown_face_clear(0);
#endif
                    continue;
                default:
                    break;
                }
            }
        }
        if (wake_step == 1) {
            cnt++;
            if (cnt == 1) {
#ifdef TCFG_LED_EYA_R_PORT
#if (TCFG_LED_EYA_R_PORT != -1)
                gpio_direction_output(TCFG_LED_EYA_R_PORT, !LED_EYA_OPEN);
                gpio_direction_output(TCFG_LED_EYA_L_PORT, !LED_EYA_OPEN);
#endif
#endif
#ifdef USED_TM1629_EAY
                tm_1629_shown_face(0);
#endif
#ifdef  USED_WS2812B_SHOWN
                ws2812_shown_face(0);
#endif
            } else if (cnt == 10) {
#ifdef TCFG_LED_EYA_R_PORT
#if (TCFG_LED_EYA_R_PORT != -1)
                gpio_direction_output(TCFG_LED_EYA_R_PORT, LED_EYA_OPEN);
                gpio_direction_output(TCFG_LED_EYA_L_PORT, LED_EYA_OPEN);
#endif
#endif
#ifdef USED_TM1629_EAY
                tm_1629_shown_face(1);
#endif
#ifdef  USED_WS2812B_SHOWN
                ws2812_shown_face(1);
#endif
            } else if (cnt == 20) {
#ifdef TCFG_LED_EYA_R_PORT
#if (TCFG_LED_EYA_R_PORT != -1)
                gpio_direction_output(TCFG_LED_EYA_R_PORT, !LED_EYA_OPEN);
                gpio_direction_output(TCFG_LED_EYA_L_PORT, !LED_EYA_OPEN);
#endif
#endif
#ifdef USED_TM1629_EAY
                tm_1629_shown_face(0);
#endif
#ifdef  USED_WS2812B_SHOWN
                ws2812_shown_face(0);
#endif
            } else if (cnt == 30) {
#ifdef TCFG_LED_EYA_R_PORT
#if (TCFG_LED_EYA_R_PORT != -1)
                gpio_direction_output(TCFG_LED_EYA_R_PORT, LED_EYA_OPEN);
                gpio_direction_output(TCFG_LED_EYA_L_PORT, LED_EYA_OPEN);
#endif
#endif
#ifdef USED_TM1629_EAY
                tm_1629_shown_face(1);
#endif
#ifdef  USED_WS2812B_SHOWN
                ws2812_shown_face(1);
#endif
                cnt = 0;
                wake_step = 0;
            }
        } else if (wake_step == 2) {
#ifdef TCFG_LED_EYA_R_PORT
#if (TCFG_LED_EYA_R_PORT != -1)
            gpio_direction_output(TCFG_LED_EYA_R_PORT, !LED_EYA_OPEN);
            gpio_direction_output(TCFG_LED_EYA_L_PORT, !LED_EYA_OPEN);
#endif
#endif
#ifdef USED_TM1629_EAY
            tm_1629_shown_face(0);
            tm_1629_shown_face_clear(0);
#endif
#ifdef  USED_WS2812B_SHOWN
            ws2812_shown_face(0);
#endif
#if (defined TCFG_LED_STATUES_VBAT_NET_EN && TCFG_LED_STATUES_VBAT_NET_EN == 1)
            extern void led_status_set(int status);//0初始化 1网络异常-快闪  2电池<20%-快闪  3网络正常和电池>=20%-常亮
            led_status_set(3);
#endif
            cnt = 0;
            wake_step = 0;
        }
        os_time_dly(1);
    }
}

static void led_eya_task_create(void *priv)
{
    if (production_test_io_get()) {
        return;
    }
#ifdef TCFG_LED_EYA_R_PORT
#if (TCFG_LED_EYA_R_PORT != -1)
    gpio_direction_output(TCFG_LED_EYA_R_PORT, !LED_EYA_OPEN);
    gpio_direction_output(TCFG_LED_EYA_L_PORT, !LED_EYA_OPEN);
    gpio_set_hd(TCFG_LED_EYA_R_PORT, 1);
    gpio_set_hd(TCFG_LED_EYA_L_PORT, 1);
#endif
#endif
    if (thread_fork(LED_EYA_NAME, 10, 512, 128, NULL, led_eya_task, NULL) != OS_NO_ERR) {
        puts("led_eya_task create err\n");
    }
}
#if (defined TCFG_LED_EYA_R_PORT || defined USED_TM1629_EAY)
late_initcall(led_eya_task_create);
#endif
#endif
