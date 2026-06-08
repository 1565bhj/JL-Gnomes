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

#ifdef TOUCH_WAKUP_PORT_ENABLE
extern volatile unsigned long jiffies;

extern void *port_wakeup_reg(PORT_EVENT_E event, unsigned int gpio, PORT_EDGE_E edge, void (*handler)(void));
static char touch_key               SEC_USED(.sram) = 0;
static char touch_key_data          SEC_USED(.sram) = 0;
static char touch_key_continue      SEC_USED(.sram) = 0;
static int touch_interval_start     SEC_USED(.sram) = 0;
static int touch_interval           SEC_USED(.sram) = 0;

/*
CLK:    ____   _   _
_______|    |_| |_| |________
DATA-KAY1:   _
____________| |______________
DATA-KAY2:     _
______________| |____________
DATA-KAY3:       _
________________| |__________
DATA-KAY4:         _
__________________| |________
*/
enum {
    TOUCH_KEY_TYPE_INIT = 0,
    TOUCH_KEY_TYPE_CLICK,
    TOUCH_KEY_TYPE_LONG,
};
#define WKUP_ENABLE(x)       JL_WAKEUP->CON0 |= BIT(x)
#define WKUP_DISABLE(x)      JL_WAKEUP->CON0 &= (~BIT(x))
#define WKUP_EDGE(x,edg)     JL_WAKEUP->CON1 = ((JL_WAKEUP->CON1 & (~BIT(x))) | (edg? BIT(x): 0))
static void handler_isr(void) SEC_USED(.volatile_ram_code) ALIGNE(4)
{
    ++touch_key;
    WKUP_DISABLE(EVENT_PB1);
    if (touch_key <= 4) {
        WKUP_EDGE(EVENT_PB1, (touch_key & 0x1) ? EDGE_POSITIVE : EDGE_NEGATIVE);
    } else {
        WKUP_EDGE(EVENT_PB1, EDGE_NEGATIVE);
    }
    if (touch_key >= 5) {
        touch_key = 0;
        touch_interval = jiffies;
    } else if (touch_key == 1 && touch_key_data == 0) {
        touch_interval_start = jiffies;
    }
    WKUP_ENABLE(EVENT_PB1);
    if (p33_rx_1byte(P3_PR_IN) & 0x1) { //PB0、读出数据1
        touch_key_data = touch_key >= 5 ? 4 : touch_key;
//        putchar('0' + touch_key_data);
    }
}
static void wkport_press_scan(void)
{
    int key;
    if (touch_interval && (jiffies - touch_interval) > 50 / 10) { //1为10ms，超过50ms则视按键释放
        key = touch_key_data;
        touch_key_data = 0;
        if (touch_interval - touch_interval_start > 1000 / 10) { //超过1s为长按
            os_taskq_post("wkport_task", 2, TOUCH_KEY_TYPE_LONG, key);
        } else {
            os_taskq_post("wkport_task", 2, TOUCH_KEY_TYPE_CLICK, key);
        }
        touch_interval = 0;
        touch_interval_start = 0;
    }
}
static void wkport_task(void *priv)
{
    int cnt = 0;
    int err, res;
    int msg[4];
    int key_val, key_msg;

    os_time_dly(100);
    gpio_direction_input(IO_PORTB_00);
    gpio_set_pull_down(IO_PORTB_00, 1);
    gpio_set_pull_up(IO_PORTB_00, 0);
    gpio_set_die(IO_PORTB_00, 1);

    //IO中断注册
    port_wakeup_reg(EVENT_PB1, IO_PORTB_01, EDGE_NEGATIVE, handler_isr);
    sys_timer_add_to_task("wkport_task", NULL, wkport_press_scan, 50);//50ms检测1次
    while (1) {
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        if (res == OS_TASK_DEL_IDLE) {
            break;
        } else if (res == OS_TASKQ) {
            if (msg[0] == Q_USER) {
                key_msg = msg[1];
                key_val = msg[2];
                struct key_event key = {0};
                key.type = KEY_EVENT_USER;
                key.action = key_msg == TOUCH_KEY_TYPE_CLICK ? KEY_EVENT_CLICK : KEY_EVENT_LONG;
                switch (key_val) {
                case 1:
                    key.value = KEY_VOLUME_DEC;
                    break;
                case 2:
                    key.value = KEY_MODE;
                    break;
                case 3:
                    key.value = KEY_OK;
                    break;
                case 4:
                    key.value = KEY_VOLUME_INC;
                    break;
                }
                key_event_notify(KEY_EVENT_FROM_USER, &key);
                printf("-> touch_key_data %s = %d\n", key_msg == TOUCH_KEY_TYPE_CLICK ? "clock" : "long", key_val);
            }
        }
    }
}
static int wkport_init(void)
{
    if (production_test_io_get()) {
        return 0;
    }
    os_task_create(wkport_task, NULL, 10, 1000, 256, "wkport_task");
    return 0;
}
late_initcall(wkport_init);
#endif
