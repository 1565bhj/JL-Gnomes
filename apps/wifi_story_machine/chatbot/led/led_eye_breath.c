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

#ifdef TCFG_LED_PWM0_PORT
#if (TCFG_LED_PWM0_PORT != -1)

#define TM_BREATH_LIGHE_SPEED   3 //2-5

enum {
    TM_LIGHT_INIT = 0,
    TM_LIGHT_OPEN,
    TM_LIGHT_CLOSE,
    TM_LIGHT_BREATH_OPEN,
    TM_LIGHT_BREATH_CLOSE,

    TM_LIGHT_EYE_WAKE,//唤醒
    TM_LIGHT_EYE_SLEEP,//休眠
    TM_LIGHT_EYE_WORK,//正在执行
};

static void *pwm_dev_handler = NULL;
static struct pwm_platform_data tm_led_pwm = {0};
static int tm_light_pwm_last = 0;
static int tm_light_pwm = 0;
static char tm_light_eye_step = 0;

static int tm_light_send_msg(int message, char notice);

void led_eya_wake(char start)
{
    if (start) {
        tm_light_send_msg(TM_LIGHT_EYE_WAKE, 0);
    }
    tm_light_send_msg(TM_LIGHT_EYE_WORK, 0);
}
void led_eya_sleep(void)
{
    tm_light_send_msg(TM_LIGHT_EYE_SLEEP, 0);
}
int led_eya_get_work(void)
{
    return tm_light_eye_step == TM_LIGHT_EYE_WORK;
}
static void tm_led_pwm_release(void)
{
    gpio_latch_en(TCFG_LED_PWM0_PORT, 0);
}
static int tm_led_pwm_open(void)
{
    int ret;
    u32 duty;
    u32 channel;

    memset(&tm_led_pwm, 0, sizeof(tm_led_pwm));
    if (pwm_dev_handler) {
        dev_close(pwm_dev_handler);
    }
    gpio_latch_en(TCFG_LED_PWM0_PORT, 0);
    gpio_direction_output(TCFG_LED_PWM0_PORT, 0);
    gpio_set_pull_down(TCFG_LED_PWM0_PORT, 0);
    gpio_set_pull_up(TCFG_LED_PWM0_PORT, 0);
    gpio_latch_en(TCFG_LED_PWM0_PORT, 1);

    pwm_dev_handler = dev_open("pwm0", &tm_led_pwm);
    if (!pwm_dev_handler) {
        printf("open pwm err !!!\n\n");
        return -1;
    }
    gpio_set_hd(TCFG_LED_PWM0_PORT, 1);
    sys_timeout_add_to_task("sys_timer", NULL, tm_led_pwm_release, 100); //释放IO
    printf("-> tm_led_pwm.pwm_ch = 0x%x \n", tm_led_pwm.pwm_ch);
    return 0;
}
static int tm_led_pwm_close(void)
{
    if (pwm_dev_handler) {
        dev_ioctl(pwm_dev_handler, PWM_STOP, (u32)&tm_led_pwm);//PWM停止
        dev_close(pwm_dev_handler);
        pwm_dev_handler = NULL;
        return 0;
    }
    return -1;
}
static int my_pow(int a, int b)
{
    int i = 1;
    while (b--) {
        i *= a;
    }
    return i;
}
int tm_led_pwm_write(int duty)
{
    if (pwm_dev_handler) {
        tm_led_pwm.duty = duty;
        dev_write(pwm_dev_handler, (void *)&tm_led_pwm, 0);
    }
    return pwm_dev_handler ? 0 : -1;
}

int tm_light_pwm_decinc(int inc, int notice)
{
    if (!tm_light_pwm || !pwm_dev_handler) {
        if (notice) {
            music_play_light_anser(3);
        }
        return 0;
    }
    if (inc == (int)0xFF) {
        tm_light_pwm += 25;
        if (tm_light_pwm > 100) {
            tm_light_pwm = 25;
        }
    } else {
        tm_light_pwm += inc ? 25 : (-25);
        if (tm_light_pwm > 100) {
            tm_light_pwm = 100;
            if (notice) {
                music_play_light_anser(2);
            }
        } else if (tm_light_pwm < 25) {
            tm_light_pwm = 25;
            if (notice) {
                music_play_light_anser(1);
            }
        } else {
            if (notice) {
                music_play_light_anser(0);
            }
        }
    }
    tm_light_pwm_last = tm_light_pwm;
    tm_led_pwm_write(tm_light_pwm);
    return 0;
}

int tm_light_pwm_init(char notice)
{
    if (tm_light_pwm_last) {
        tm_light_pwm = tm_light_pwm_last;
    } else if (!tm_light_pwm) {
        tm_light_pwm = 100;
    }
    tm_light_pwm_last = tm_light_pwm;
    tm_led_pwm_open();
    tm_led_pwm_write(tm_light_pwm);
    printf("->tm_light_pwm_init00 : %d\n", notice);
    if (notice) {
        printf("->tm_light_pwm_init11 : %d\n", notice);
        music_play_light_anser(0);
    }
    return 0;
}
int tm_light_pwm_uninit(char notice)
{
    if (tm_light_pwm) {
        tm_light_pwm = 0;
    }
    int err = tm_led_pwm_close();
    if (notice) {
        if (!err) {
            music_play_light_anser(0);
        } else {
            music_play_light_anser(3);
        }
    }
    return 0;
}
int tm_light_pwm_auto(void)
{
    if (tm_light_pwm) {
        tm_light_pwm_uninit(0);
        tm_light_send_msg(TM_LIGHT_CLOSE, 0);
        return 0;
    } else {
        tm_light_pwm_init(0);
        return 1;
    }
}
int tm_light_pwm_auto_decinc_openclose(void)
{
    int ret = 0;
    if (!pwm_dev_handler) {
        tm_light_pwm_init(0);
        ret = 1;
    } else {
        switch (tm_light_pwm) {
        case 100:
            tm_light_pwm = 70;
            tm_light_pwm_last = tm_light_pwm;
            tm_led_pwm_write(tm_light_pwm);
            ret = 2;
            break;
        case 70:
            tm_light_pwm = 30;
            tm_light_pwm_last = tm_light_pwm;
            tm_led_pwm_write(tm_light_pwm);
            ret = 3;
            break;
        case 30:
            tm_light_pwm = 0;
            tm_light_pwm_last = tm_light_pwm;
            tm_light_pwm_uninit(0);
            ret = 0;
            break;
        default:
            tm_light_pwm_init(0);
            ret = 1;
            break;
        }

    }
    return ret;
}
#define TM_LIGHT_NAME     "tm_light_task"
static int tm_light_send_msg(int message, char notice)
{
    int ret;
    char retry = 0;

    do {
        ret = os_taskq_post(TM_LIGHT_NAME, 2, message, notice);
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

void tm_light_open(char notice)
{
    puts("->tm_light_open\n");
    tm_light_send_msg(TM_LIGHT_OPEN, notice);
}
void tm_light_close(char notice)
{
    puts("->tm_light_close\n");
    tm_light_send_msg(TM_LIGHT_CLOSE, notice);
}
void tm_light_breath(char open, char notice)
{
    puts("->tm_light_breath\n");
    tm_light_send_msg(open ? TM_LIGHT_BREATH_OPEN : TM_LIGHT_BREATH_CLOSE, notice);
}
static void tm_led_open_auto_set(char note);
struct msg_type {
    int msg[2];
};
static int tm_light_eye_task(void *priv, struct msg_type *msg_type)
{
    int err, res;
    int msg[4];
    int cnt = 0;
    int accept = 0;
    int wake_step = 0;

    res = OS_TASKQ;
    msg[0] = Q_USER;
    msg[1] = (int)priv;
    while (1) {
        if (res == OS_TASKQ) {
            if (msg[0] == Q_USER) {
                switch (msg[1]) {
                case TM_LIGHT_EYE_WAKE:
                    tm_light_pwm_init(0);
                    puts("->led_eya_wake\n");
                    break;
                case TM_LIGHT_EYE_WORK:
                    puts("->led_eya_work\n");
                    tm_light_eye_step = TM_LIGHT_EYE_WAKE;
                    wake_step = tm_light_eye_step;
                    cnt = 0;
                    break;
                case TM_LIGHT_EYE_SLEEP:
                    puts("->led_eya_stop\n");
                    tm_light_eye_step = TM_LIGHT_EYE_SLEEP;
                    wake_step = tm_light_eye_step;
                    cnt = 0;
                    break;
                default:
                    cnt = 0;
                    wake_step = 0;
                    if (msg_type) {
                        msg_type->msg[0] = msg[1];
                        msg_type->msg[1] = msg[2];
                    }
                    return msg[1];
                }
            }
        }
        if (wake_step == TM_LIGHT_EYE_WAKE) {
            cnt++;
            if (cnt == 1) {
                tm_led_pwm_write(0);
            } else if (cnt == 10) {
                tm_led_pwm_write(tm_light_pwm ?  tm_light_pwm : 100);
            } else if (cnt == 20) {
                tm_led_pwm_write(0);
            } else if (cnt == 30) {
                tm_led_pwm_write(tm_light_pwm ?  tm_light_pwm : 100);
                cnt = 0;
                wake_step = 0;
            }
        } else if (wake_step == TM_LIGHT_EYE_SLEEP) {
            tm_led_pwm_write(0);
            cnt = 0;
            wake_step = 0;
            break;
        }
        os_time_dly(1);
        res = os_taskq_accept(ARRAY_SIZE(msg), msg);
        //res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        if (res == OS_TASK_DEL_IDLE) {
            break;
        }
    }
    return 0;
}


static int tm_light_task(void *priv)
{
    int err, res;
    int msg[4];
    int light_breath = 0;
    int light_duty = 0;
    int cnt = 0;
    char duty_add = 0;
    int notice = 0;

#if (defined TCFG_LED_PWM0_PORT_POWER_ON_OPEN && TCFG_LED_PWM0_PORT_POWER_ON_OPEN == 1)
    tm_led_open_auto_set(0);
#ifdef TCFG_PWM1_PORT
    tm_led_open_auto_set(0);
#endif
#endif
    while (1) {
        memset(msg, 0xFF, sizeof(msg));
        if (light_breath) {
            res = os_taskq_accept(ARRAY_SIZE(msg), msg);
        } else {
            res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        }
        if (res == OS_TASK_DEL_IDLE) {
            break;
        } else if (res == OS_TASKQ) {
            if (msg[0] == Q_USER) {
__msg_do:
                notice = msg[2];
                switch (msg[1]) {
                case TM_LIGHT_OPEN:
                    puts("->TM_LIGHT_OPEN : %d\n");
                    tm_light_pwm_init(notice);
                    light_breath = 0;
                    light_duty = 100;
                    break;
                case TM_LIGHT_CLOSE:
                    puts("->TM_LIGHT_CLOSE\n");
                    tm_light_pwm_uninit(notice);
                    light_breath = 0;
                    light_duty = 0;
                    break;
                case TM_LIGHT_BREATH_OPEN:
                    puts("->TM_LIGHT_BREATH_OPEN\n");
                    tm_light_pwm_init(notice);
                    light_duty = tm_light_pwm_last;
                    duty_add = 1;
                    light_breath = 1;
                    break;
                case TM_LIGHT_BREATH_CLOSE:
                    puts("->TM_LIGHT_BREATH_CLOSE\n");
                    tm_light_pwm_uninit(notice);
                    light_duty = 0;
                    duty_add = 0;
                    light_breath = 0;
                    break;
                case TM_LIGHT_EYE_WAKE:
                    puts("->TM_LIGHT_EYE_WAKE\n");
                    struct msg_type ret_msg = {0};
                    if (tm_light_eye_task(TM_LIGHT_EYE_WAKE, &ret_msg)) {
                        msg[2] = ret_msg.msg[1];
                        msg[1] = ret_msg.msg[0];
                        goto __msg_do;
                    }
                    if (light_duty || light_breath) {
                        tm_led_pwm_write(light_duty);
                        light_duty = light_breath ? (light_duty + 1) : light_duty;
                    }
                    break;
                default:
                    break;
                }
            }
        } else {
            if (light_breath) {
                if (light_duty >= 100) {
                    duty_add = 0;
                } else if (light_duty <= 2) {
                    duty_add = 1;
                }
                if (cnt++ % TM_BREATH_LIGHE_SPEED == 0) {
                    if (duty_add) {
                        light_duty += 1;
                    } else {
                        light_duty -= 1;
                    }
                    tm_led_pwm_write(light_duty);
                }
                os_time_dly(1);
//                taskYIELD();
            }
        }
    }
}
void tm_led_open_pwm_auto(void)
{
#ifndef TCFG_LED_PWM0_PORT_SHORT_PRESS_AUTO
    static u8 tm_long_cnt = 0;
    if (tm_long_cnt++ & 0x1) { //按键长按时间比较短，两次按键通知调用1次灯光控制
        tm_light_pwm_decinc((int)0xFF, 0);
    }
    //tm_light_pwm_decinc((int)0xFF, 0);
#endif
}
static void tm_led_open_auto_set(char note)
{
    static char touch_light_open = 0;
    static char touch_light_mode = 0;
#ifdef TCFG_LED_PWM0_PORT_SHORT_PRESS_AUTO
    int ret = tm_light_pwm_auto_decinc_openclose();
    if (note) {
        if (ret == 1) { //open
            music_play_res_file("LightMode.mp3");
        } else if (ret == 0) { //close
            music_play_res_file("LightClose.mp3");
        }
    }
#else
    if (touch_light_open == 0) {
        touch_light_open = tm_light_pwm_auto() ? 1 : 2;//打开灯光：open = 1, close = 2
#ifdef TCFG_PWM1_PORT
        touch_light_mode = touch_light_open == 1 ? 1 : 4;//打开灯光：open 1开始， close 4开始
#elif (defined TCFG_LED_PWM0_PORT)
        touch_light_mode = 2;//打开灯光：open 2开始
#endif
        if (touch_light_open == 1) { //open
            if (note) {
                music_play_res_file("LightMode.mp3");
            }
        } else { //close
            if (note) {
                music_play_res_file("LightClose.mp3");
            }
#if (defined TCFG_LED_PWM0_PORT && !defined TCFG_PWM1_PORT)
            touch_light_open = 0;//没有球体地球仪灯，则关闭灯光，下一次就是开启灯光
#endif
        }
    } else {
        switch (++touch_light_mode) {
#ifdef TCFG_PWM1_PORT
        case 2://打开球体灯
            tm_light_earth_ctrol(1, 0);
            if (note) {
                music_play_res_file("EearthMode.mp3");
            }
            break;
#endif
        case 3://打开呼吸灯
            tm_light_breath(1, 0);
            if (note) {
                music_play_res_file("BreathMode.mp3");
            }
            touch_light_open = 0;
            break;
#ifdef TCFG_PWM1_PORT
        case 5://关闭球体灯
            tm_light_earth_ctrol(0, 0);
            if (note) {
                music_play_res_file("EearthClose.mp3");
            }
            touch_light_open = 0;
            break;
#endif
        default:
            break;
        }
    }
#endif
}
void tm_led_open_auto(void)
{
    tm_led_open_auto_set(1);
}
static void tm_light_task_create(void *priv)
{
    if (production_io_is_enter() || is_production_test_enter(0)) {
        return;
    }
    gpio_latch_en(TCFG_LED_PWM0_PORT, 0);
    gpio_direction_output(TCFG_LED_PWM0_PORT, 0);
    gpio_latch_en(TCFG_LED_PWM0_PORT, 1);
    if (thread_fork(TM_LIGHT_NAME, 10, 1024, 256, NULL, tm_light_task, NULL) != OS_NO_ERR) {
        puts("tm_light_task create err\n");
    }
}
late_initcall(tm_light_task_create);
#endif
#endif
