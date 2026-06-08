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

#ifdef TCFG_LED_STATUES_PORT
#if (TCFG_LED_STATUES_PORT != -1)

static void *pwm_dev_handler = NULL;
static struct pwm_platform_data led_pwm = {0};

static int light_pwm_last = 0;
static int light_pwm = 0;

static const char *open_light[] = {
    "开灯",
    "打开灯光",
    NULL,
};
static const char *close_light[] = {
    "关灯",
    "关闭灯光",
    NULL,
};
static const char *pwm_light[] = {
    "暗一点", "灯光暗一点",
    "亮一点", "灯光亮一点",
    NULL,
};
static void led_pwm_release(void)
{
    gpio_latch_en(TCFG_LED_STATUES_PORT, 0);//解锁IO
}
int led_pwm_open(void)
{
    int ret;
    u32 duty;
    u32 channel;

    memset(&led_pwm, 0, sizeof(led_pwm));
    if (pwm_dev_handler) {
        dev_close(pwm_dev_handler);
    }

    gpio_direction_output(TCFG_LED_STATUES_PORT, 0);
    gpio_latch_en(TCFG_LED_STATUES_PORT, 1);//锁定IO

    pwm_dev_handler = dev_open("pwm0", &led_pwm);//
    if (!pwm_dev_handler) {
        printf("open pwm err !!!\n\n");
        gpio_latch_en(TCFG_LED_STATUES_PORT, 0);
        gpio_direction_output(TCFG_LED_STATUES_PORT, 1);
        return -1;
    }
    gpio_set_hd(TCFG_LED_STATUES_PORT, 1);//设置强驱
    sys_timeout_add_to_task("sys_timer", NULL, led_pwm_release, 100); //释放IO
    return 0;
}
void led_pwm_close(void)
{
    if (pwm_dev_handler) {
        dev_ioctl(pwm_dev_handler, PWM_STOP, (u32)&led_pwm);//PWM停止
        dev_close(pwm_dev_handler);
        pwm_dev_handler = NULL;
    } else {
        gpio_direction_output(TCFG_LED_STATUES_PORT, 0);
    }
}
int led_pwm_write(int duty)
{
    if (pwm_dev_handler) {
        led_pwm.duty = duty;
        dev_write(pwm_dev_handler, (void *)&led_pwm, 0);
    }
    return pwm_dev_handler ? 0 : -1;
}

static void led_shown(void)
{
    static char led_status;
    if (++led_status & 0x1) {
        gpio_direction_output(TCFG_LED_STATUES_PORT, 0);
    } else {
        gpio_direction_output(TCFG_LED_STATUES_PORT, 1);
    }
}

void led_init(void)
{
#if (!defined TCFG_LED_STATUES_VBAT_NET_EN || TCFG_LED_STATUES_VBAT_NET_EN == 0)
    sys_timer_add_to_task("sys_timer", NULL, led_shown, 1000);
#endif
}


int light_pwm_decinc(int inc, int notice)
{
    if (!light_pwm || !pwm_dev_handler) {
        return 0;
    }
    light_pwm += inc ? 25 : (-25);
    if (light_pwm > 100) {
        light_pwm = 100;
        if (notice) {
            music_play_light_anser(2);
        }
    } else if (light_pwm < 25) {
        light_pwm = 25;
        if (notice) {
            music_play_light_anser(1);
        }
    } else {
        if (notice) {
            music_play_light_anser(0);
        }
    }
    light_pwm_last = light_pwm;
    led_pwm_write(light_pwm);
    return 0;
}
int light_pwm_init(char notice)
{
    if (light_pwm_last) {
        light_pwm = light_pwm_last;
    } else if (!light_pwm) {
        light_pwm = 100;
    }
    light_pwm_last = light_pwm;
    led_pwm_open();
    led_pwm_write(light_pwm);
    if (notice) {
        music_play_light_anser(0);
    }
    return 0;
}
int light_pwm_uninit(char notice)
{
    if (light_pwm) {
        light_pwm = 0;
    }
    led_pwm_close();
    if (notice) {
        music_play_light_anser(0);
    }
    return 0;
}
int light_pwm_auto(void)
{
    if (light_pwm) {
        light_pwm_uninit(0);
//        light_pwm = 0;
//        led_pwm_close();
    } else {
        light_pwm_init(0);
//        led_pwm_open();
//        light_pwm = 50;
    }
    return 0;
}
int light_pwm_instruction_word_callback(char *word)
{
#if (!defined TCFG_LED_STATUES_VBAT_NET_EN || TCFG_LED_STATUES_VBAT_NET_EN == 0)
    for (int i = 0; open_light[i] != NULL; i++) { //开灯
        if (!strcmp(word, open_light[i])) {
            light_pwm_init(1);
            puts("-> open light \n");
            return 1;
        }
    }
    for (int i = 0; close_light[i] != NULL; i++) { //关灯
        if (!strcmp(word, close_light[i])) {
            light_pwm_uninit(1);
            puts("-> close light \n");
            return 1;
        }
    }
    for (int i = 0; pwm_light[i] != NULL; i++) { //暗一点、亮一点
        if (!strcmp(word, pwm_light[i])) {
            light_pwm_decinc(i >= 2 ? 1 : 0, 1);
            printf("-> pwm = %d\n", light_pwm);
            break;
        }
    }
#endif
    return 0;
}

#if (defined TCFG_LED_STATUES_VBAT_NET_EN && TCFG_LED_STATUES_VBAT_NET_EN == 1)
/*
需求：
绿色快烁:网络异常，配网成功后转成绿色长亮(电量大于20%)。
绿色长亮:网络正常、电池电量大于等于 20%
绿色慢烁:电池电量低于 20%
红色长亮:充电红色长亮，充满灯熄灭
语音提示规则:网络连接异常，给予语音播报提示，提示内容“网络异常，请检查网络配置.电池电量低于 20%，给与语音播报提示，提示内容
   “电池电量低于 20%，请及时充电”，电池电量低于20%，则每10秒钟提醒一次，连续提醒5次没有进行充电则进行关机。

不改硬件：把蓝色灯换绿色灯即可
*/
void led_status_set(int status)//0初始化 1网络异常-快闪  2电池<20%-快闪  3网络正常和电池>=20%-常亮
{
    os_taskq_post("led_status", 1, status);
}
static void led_status_task(void)
{
    int err, res;
    int msg[4];
    int cnt = 0;
    int low_power_cnt = 0;
    int blink = 0;
    int status = 0;
    int io_status = 0;

    gpio_latch_en(TCFG_LED_STATUES_PORT, 0);
    gpio_direction_output(TCFG_LED_STATUES_PORT, 1);
    extern int sys_connect_net_success(void);
    if (!sys_connect_net_success()) {
        status = 1;
    }
    while (1) {
//        if(cnt++ % 20 == 0){
//            if (!sys_connect_net_success()) {
//                status = 1;
//            }else if(status != 2){
//                status = 3;
//            }
//        }
        res = os_taskq_accept(ARRAY_SIZE(msg), msg);
        //res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        if (res == OS_TASK_DEL_IDLE) {
            break;
        } else if (res == OS_TASKQ) {
            if (msg[0] == Q_USER) {
                switch (msg[1]) {
                case 1://快闪优先级最高
                    status = msg[1];
                    break;
                case 2://电量低于20%优先级第二
                    if (status != 1) {
                        status = msg[1];
                    }
                    break;
                case 3://正常优先级最低
                    if (status != 2) {
                        status = msg[1];
                    }
                    break;
                }
                io_status = 0;
                low_power_cnt = 0;
            }
        }
        switch (status) {
        case 1:
            //puts("->net err\n");
            gpio_direction_output(TCFG_LED_STATUES_PORT, io_status);
            io_status = !io_status;
            break;
        case 2:
            //puts("->vbat < 20%\n");
            if (low_power_cnt++ % 3 == 0) {
                gpio_direction_output(TCFG_LED_STATUES_PORT, io_status);
                io_status = !io_status;
            }
            break;
        case 3:
            //puts("->all normal\n");
#if (defined TCFG_LED_STATUES_NORMAL_BLINK && TCFG_LED_STATUES_NORMAL_BLINK == 1)
            if (blink++ % 5 == 0) {
                io_status = !io_status;
                gpio_direction_output(TCFG_LED_STATUES_PORT, io_status);
            }
#else
            if (io_status != 1) {
                gpio_direction_output(TCFG_LED_STATUES_PORT, 1);
                io_status = 1;
            }
#endif
            break;

        default:
            break;
        }
        os_time_dly(20);
    }
}
static void led_status_init(void)
{
    if (production_io_is_enter() || is_production_test_enter(0)) {
        return;
    }
    if (thread_fork("led_status", 10, 512, 64, NULL, led_status_task, NULL) != OS_NO_ERR) {
        puts("led_eya_task create err\n");
    }
}
late_initcall(led_status_init);
#endif
#endif
#endif
