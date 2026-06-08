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

#ifdef TCFG_KEY_WAKE_AISP_PORT
#if (TCFG_KEY_WAKE_AISP_PORT != 0xff)

#define KEY_WAKE_SCAN_TIME     30

extern void aisp_wake(char index);//0:小飞小飞，8：配网模式

static int key_wake_enter = 0;
static char key_wake_done = 0;
static char key_wake_net_cfg = 0;
static char key_wake_light_open = 0;
static char key_wake_light_mode = 0;

static void key_wake_scan(void)
{
    int io_sta = gpio_read(TCFG_KEY_WAKE_AISP_PORT);
    if (!io_sta) {
        key_wake_enter++;
        if (key_wake_enter >= (5000 / KEY_WAKE_SCAN_TIME) && !key_wake_net_cfg) {
            key_wake_net_cfg = 1;
            aisp_wake(8);//按键进入配网
        }
    } else {
        if (key_wake_enter >= 2 && !key_wake_done && !key_wake_net_cfg) {
            aisp_wake(0);//按键唤醒对话
        }
        key_wake_enter = 0;
        key_wake_done = 0;
        key_wake_net_cfg = 0;
    }
}
static void key_wake_scan_init(void)
{
    gpio_direction_input(TCFG_KEY_WAKE_AISP_PORT);
    gpio_set_pull_up(TCFG_KEY_WAKE_AISP_PORT, 1);
    gpio_set_pull_down(TCFG_KEY_WAKE_AISP_PORT, 0);
    sys_timer_add_to_task("sys_timer", NULL, key_wake_scan, KEY_WAKE_SCAN_TIME);
}
static void key_wake_init(void)
{
    if (production_test_io_get()) {
        return;
    }
    sys_timeout_add_to_task("sys_timer", NULL, key_wake_scan_init, 1000);//1秒后再检测按键
}
late_initcall(key_wake_init);
#endif
#endif
