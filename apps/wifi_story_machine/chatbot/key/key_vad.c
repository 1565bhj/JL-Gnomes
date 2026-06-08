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

#ifdef TCFG_KEY_VAD_PORT
#if (TCFG_KEY_VAD_PORT != 0xff)

#define KEY_WAKE_SCAN_TIME     30

extern void aisp_wake(char index);//0:小飞小飞，8：配网模式
extern void key_vad_pcm_send_set_status(char start, char noice_not);
extern int ai_speaker_app(void);

static int key_vad_wake_enter = 0;
static char key_vad_wake_start = 0;

static void key_avd_wake_scan(void)
{
    int io_sta = gpio_read(TCFG_KEY_VAD_PORT);
    if (!io_sta) {
        key_vad_wake_enter++;
        if (key_vad_wake_enter >= 3 && !key_vad_wake_start && ai_speaker_app()) {
            key_vad_wake_start = 1;
            key_vad_pcm_send_set_status(1, 0);
        }
    } else {
        if (key_vad_wake_enter >= 3 && key_vad_wake_start) {
            key_vad_pcm_send_set_status(0, 0);
        }
        key_vad_wake_start = 0;
        key_vad_wake_enter = 0;
    }
}
static void key_vad_wake_scan_init(void)
{
    gpio_direction_input(TCFG_KEY_VAD_PORT);
    gpio_set_pull_up(TCFG_KEY_VAD_PORT, 1);
    gpio_set_pull_down(TCFG_KEY_VAD_PORT, 0);
    sys_timer_add_to_task("sys_timer", NULL, key_avd_wake_scan, KEY_WAKE_SCAN_TIME);
}
static void key_wake_init(void)
{
    if (production_test_io_get()) {
        return;
    }
    sys_timeout_add_to_task("sys_timer", NULL, key_vad_wake_scan_init, 1000);//1秒后再检测按键
}
late_initcall(key_wake_init);
#endif
#endif
