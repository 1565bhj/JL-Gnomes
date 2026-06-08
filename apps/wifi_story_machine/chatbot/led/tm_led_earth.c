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

#ifdef TCFG_PWM1_PORT
#if (TCFG_PWM1_PORT != -1)
static void *pwm_dev_handler = NULL;
static struct pwm_platform_data led_pwm = {0};

static int light_pwm_last = 0;
static int light_pwm = 0;

static void led_pwm_release(void)
{
    gpio_latch_en(TCFG_PWM1_PORT, 0);//解锁IO
}
static int led_pwm_open(void)
{
    int ret;
    u32 duty;
    u32 channel;

    gpio_latch_en(TCFG_PWM1_PORT, 0);//解锁IO
    gpio_direction_output(TCFG_PWM1_PORT, 0);//设置IO
    gpio_latch_en(TCFG_PWM1_PORT, 1);//锁定IO

    memset(&led_pwm, 0, sizeof(led_pwm));
    if (pwm_dev_handler) {
        dev_close(pwm_dev_handler);
    }

    pwm_dev_handler = dev_open("pwm1", &led_pwm);
    if (!pwm_dev_handler) {
        printf("open pwm err !!!\n\n");
        return -1;
    }
    gpio_set_hd(TCFG_PWM1_PORT, 1);
    sys_timeout_add_to_task("sys_timer", NULL, led_pwm_release, 100); //释放IO
    return 0;
}
static void led_pwm_close(void)
{
    if (pwm_dev_handler) {
        dev_ioctl(pwm_dev_handler, PWM_STOP, (u32)&led_pwm);//PWM停止
        dev_close(pwm_dev_handler);
        pwm_dev_handler = NULL;
        gpio_latch_en(TCFG_PWM1_PORT, 0);//解锁IO
        gpio_direction_output(TCFG_PWM1_PORT, 0);//设置IO
        gpio_latch_en(TCFG_PWM1_PORT, 1);//锁定IO，防止其他程序控制到这个引脚，使得无线充电模块异常短路
    }
}
static int tm_light_earth_init(char notice)
{
    led_pwm_open();
    if (notice) {
        music_play_light_anser(0);
    }
    return 0;
}
static int tm_light_earth_uninit(char notice)
{
    led_pwm_close();
    if (notice) {
        music_play_light_anser(0);
    }
    return 0;
}

void tm_light_earth_ctrol(char open, char notice)
{
    if (open) {
        puts("->tm_light_earth open\n");
        tm_light_earth_init(notice);
    } else {
        tm_light_earth_uninit(notice);
        puts("->tm_light_earth close\n");
    }
}
#endif
#endif
