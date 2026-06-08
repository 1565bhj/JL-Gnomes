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

#ifdef TCFG_TOUCH_KEY_PORT
#if (TCFG_TOUCH_KEY_PORT != -1)

#define TOUCH_KEY_SCAN_TIME     30

extern void tm_light_open(char notice);
extern void tm_light_close(char notice);
extern void tm_light_breath(char open, char notice);
extern void tm_light_earth_ctrol(char open, char notice);
extern int tm_light_pwm_decinc(int inc, int notice);
extern int music_play_res_file(const char *name);
extern void aisp_wake(char index);//0:小飞小飞，8：配网模式
extern void system_restore_factory_settings(void);

static int touch_key_enter = 0;
static char touch_done = 0;
static char touch_net_cfg = 0;
static char touch_reset_cfg = 0;
static char touch_light_open = 0;
static char touch_light_mode = 0;

static void tm_touch_key_scan(void)
{
    int io_sta = gpio_read(TCFG_TOUCH_KEY_PORT);
    if (!io_sta) {
        touch_key_enter++;
//        if(touch_key_enter >= (1500 / TOUCH_KEY_SCAN_TIME) && !touch_done){
//#ifndef TCFG_LED_PWM0_PORT_SHORT_PRESS_AUTO
//            tm_light_pwm_decinc((int)0xFF, 0);
//#endif
//            touch_done = 1;
//        }
        if (touch_key_enter >= (5000 / TOUCH_KEY_SCAN_TIME) && !touch_net_cfg) {
            touch_net_cfg = 1;
            touch_done = 1;
            aisp_wake(8);
        }
        if (touch_key_enter >= (8000 / TOUCH_KEY_SCAN_TIME) && !touch_reset_cfg) {
            touch_reset_cfg = 1;
            music_play_stop_all();
            system_restore_factory_settings();
            music_play_res_file("SysRstFaSet.mp3");
            sys_timeout_add(NULL, system_reset, 2000);
        }
    } else if (touch_key_enter > 1) {
//#ifdef TCFG_LED_PWM0_PORT_SHORT_PRESS_AUTO
//        int tm_light_pwm_auto_decinc_openclose(void);
//        if(touch_key_enter && !touch_done){
//            int ret = tm_light_pwm_auto_decinc_openclose();
//            if(ret == 1){//open
//                music_play_res_file("LightMode.mp3");
//            }else if(ret == 0){//close
//                music_play_res_file("LightClose.mp3");
//            }
//        }
//#else
//        if(touch_key_enter && !touch_done){
//            if(!music_buf_play_stop_staus()){
//                aisp_clear(0);
//            }else{
//                if(touch_light_open == 0){
//                    touch_light_open = tm_light_pwm_auto() ? 1 : 2;//打开灯光：open = 1, close = 2
//#ifdef TCFG_PWM1_PORT
//                    touch_light_mode = touch_light_open == 1 ? 1 : 4;//打开灯光：open 1开始， close 4开始
//#elif (defined TCFG_LED_PWM0_PORT)
//                    touch_light_mode = 2;//打开灯光：open 2开始
//#endif
//                    if(touch_light_open == 1){//open
//                        music_play_res_file("LightMode.mp3");
//                    }else{//close
//                        music_play_res_file("LightClose.mp3");
//#if (defined TCFG_LED_PWM0_PORT && !defined TCFG_PWM1_PORT)
//                        touch_light_open = 0;//没有球体地球仪灯，则关闭灯光，下一次就是开启灯光
//#endif
//                    }
//                }else{
//                    switch(++touch_light_mode){
//#ifdef TCFG_PWM1_PORT
//                    case 2://打开球体灯
//                        tm_light_earth_ctrol(1, 0);
//                        music_play_res_file("EearthMode.mp3");
//                        break;
//#endif
//                    case 3://打开呼吸灯
//                        tm_light_breath(1, 0);
//                        music_play_res_file("BreathMode.mp3");
//                        touch_light_open = 0;
//                        break;
//#ifdef TCFG_PWM1_PORT
//                    case 5://关闭球体灯
//                        tm_light_earth_ctrol(0, 0);
//                        music_play_res_file("EearthClose.mp3");
//                        touch_light_open = 0;
//                        break;
//#endif
//                    default:
//                        break;
//                    }
//                }
//            }
//        }
//#endif
        touch_key_enter = 0;
        touch_done = 0;
        touch_net_cfg = 0;
    } else {
        touch_key_enter = 0;
    }
}
static void tm_touch_key_scan_init(void)
{
    gpio_direction_input(TCFG_TOUCH_KEY_PORT);
    gpio_set_pull_down(TCFG_TOUCH_KEY_PORT, 0);
    gpio_set_pull_up(TCFG_TOUCH_KEY_PORT, 1);
    sys_timer_add_to_task("sys_timer", NULL, tm_touch_key_scan, TOUCH_KEY_SCAN_TIME);
}
static void tm_touch_key_init(void)
{
    if (production_test_io_get()) {
        return;
    }
    sys_timeout_add_to_task("sys_timer", NULL, tm_touch_key_scan_init, 1000);//1秒后再检测按键
}
late_initcall(tm_touch_key_init);
#endif
#endif
