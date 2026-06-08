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
#include "device/gpio.h"

#ifdef CONFIG_WIFI_ENABLE
#include "wifi/wifi_connect.h"
#endif

#ifdef TIMER_UART_CALIBRATION_ENABLE
extern int utc_timer_update_get(struct sys_time *time);
static void *uart_hdl = NULL;
static char time_update_check = 0;
static u8 uart_buf[1024] __attribute__((aligned(32))); //用于串口接收缓存数据的循环buf


//发送校验和
static unsigned char data_check_sum(unsigned char *data, int len)
{
    int i;
    unsigned int sum = 0;
    for (i = 0; i < len; i++) {
        sum += data[i];
    }
    sum = sum & 0xFF;
    return sum;
}
//接收校验和
static int recv_data_check_sum(unsigned char *data, int len)
{
    int i;
    unsigned int sum = 0;
    unsigned int check_sum = data[len - 2];
    for (i = 0; i < len - 2; i++) {
        sum += data[i];
    }
    sum = sum & 0xFF;
    return check_sum == sum ? 0 : -1;
}

static int time_update_send_to_uart(void)
{
    unsigned char tmp_data[16];
#ifdef TIMER_UART_V11_ENABLE
    int data_len = 9;
#elif (defined TIMER_UART_V2_ENABLE)
    int data_len = 13;
#else
    int data_len = 8;
#endif
    struct sys_time gtime = {0};
    if (utc_timer_update_get(&gtime) != 0) {
        get_sys_time(&gtime);
    }
    if (gtime.year) {
#ifdef TIMER_UART_V11_ENABLE
        tmp_data[0] = 0x55;
        tmp_data[1] = 0x1;
        tmp_data[2] = 4;
        tmp_data[3] = 0;
        tmp_data[4] = gtime.hour;
        tmp_data[5] = gtime.min;
        tmp_data[6] = gtime.sec;
        tmp_data[7] = data_check_sum(tmp_data, 7);
        tmp_data[8] = 0xA5;
#elif (defined TIMER_UART_V2_ENABLE)
        time_t t = time(NULL);
        struct tm timeinfo = *localtime(&t);
        int weekday = timeinfo.tm_wday; // tm_w

        printf("-> time : %d-%02d-%02d %02d:%02d:%02d %d\n",
               timeinfo.tm_year,
               timeinfo.tm_mon,
               timeinfo.tm_mday,
               timeinfo.tm_hour,
               timeinfo.tm_min,
               timeinfo.tm_sec,
               timeinfo.tm_wday);

        tmp_data[0] = 0x55;
        tmp_data[1] = 0x9;
        tmp_data[2] = 0;
        tmp_data[3] = gtime.year / 100;
        tmp_data[4] = gtime.year % 100;
        tmp_data[5] = gtime.month;
        tmp_data[6] = gtime.day;
        tmp_data[7] = weekday;
        tmp_data[8] = gtime.hour;
        tmp_data[9] = gtime.min;
        tmp_data[10] = gtime.sec;
        tmp_data[11] = data_check_sum(tmp_data, 11);
        tmp_data[12] = 0xA5;
#else
        tmp_data[0] = 0x55;
        tmp_data[1] = 4;
        tmp_data[2] = 0;
        tmp_data[3] = gtime.hour;
        tmp_data[4] = gtime.min;
        tmp_data[5] = gtime.sec;
        tmp_data[6] = data_check_sum(tmp_data, 6);
        tmp_data[7] = 0xA5;
#endif
        if (uart_hdl) {
            puts("uart time check\n");
            put_buf(tmp_data, data_len);
            dev_write(uart_hdl, tmp_data, data_len);
            os_time_dly(50);
            dev_write(uart_hdl, tmp_data, data_len);
        }
        return 0;
    } else {
        puts("uart time check err\n");
    }
    return -1;
}
void time_uart_calibration_update(void)
{
    time_update_check = 1;
    printf("->time_update_check\n");
}
static void uart_task_main(void *priv)
{
    unsigned int cnt = 0;
    char name[32];
#ifdef TIMER_UART_CALIBRATION_ENABLE
    sprintf(name, "uart%d", TIMER_UART_CALIBRATION_ENABLE);
    uart_hdl = dev_open(name, NULL);
#else
    uart_hdl = dev_open("uart2", NULL);
#endif
    if (!uart_hdl) {
        printf("open uart err !!!\n");
        return ;
    }
    /* 1 . 设置串口接收缓存数据的循环buf地址 */
    dev_ioctl(uart_hdl, UART_SET_CIRCULAR_BUFF_ADDR, (int)uart_buf);

    /* 1 . 设置串口接收缓存数据的循环buf长度 */
    dev_ioctl(uart_hdl, UART_SET_CIRCULAR_BUFF_LENTH, sizeof(uart_buf));

    /* 3 . 设置接收数据为阻塞方式,需要非阻塞可以去掉,建议加上超时设置 */
    dev_ioctl(uart_hdl, UART_SET_RECV_BLOCK, 1);

    /* u32 parm = 1000; */
    /* dev_ioctl(uart_hdl, UART_SET_RECV_TIMEOUT, (u32)parm); //超时设置 */

#ifdef TIMER_UART_TX_PORT
#ifdef TIMER_UART_CALIBRATION_ENABLE
#if TIMER_UART_CALIBRATION_ENABLE == 0
#define UART_DATA_EXTERN()  extern const struct uart_platform_data uart0_data;
#define UART_DATA_TX()  uart0_data.tx_pin
#define UART_DATA_OUTCH()  uart0_data.output_channel
#define UART_DATA_OUTCH_MAP_SET()  0
#elif TIMER_UART_CALIBRATION_ENABLE == 1
#define UART_DATA_EXTERN()  extern const struct uart_platform_data uart1_data;
#define UART_DATA_TX()  uart1_data.tx_pin
#define UART_DATA_OUTCH()  uart1_data.output_channel
#define UART_DATA_OUTCH_MAP_SET()  1
#elif TIMER_UART_CALIBRATION_ENABLE == 2
#define UART_DATA_EXTERN()  extern const struct uart_platform_data uart2_data;
#define UART_DATA_TX()  uart2_data.tx_pin
#define UART_DATA_OUTCH()  uart2_data.output_channel
#define UART_DATA_OUTCH_MAP_SET()  7
#endif
#endif
    UART_DATA_EXTERN();

//    if (UART_DATA_TX() != (u8)0xFF) {
//        gpio_set_hd(UART_DATA_TX(), 1);
//    }
    char out, die;
    out = die = 0;
    switch (UART_DATA_OUTCH()) {
    case OUTPUT_CHANNEL0:
        out = die = 0;
        JL_IOMAP->CON1 &= ~(0xF << 8);
        JL_IOMAP->CON1 |= (UART_DATA_OUTCH_MAP_SET() << 8);
        break;
    case OUTPUT_CHANNEL1:
        out = 0;
        die = 1;
        JL_IOMAP->CON3 &= ~(0xF << 20);
        JL_IOMAP->CON3 |= (UART_DATA_OUTCH_MAP_SET() << 20);
        break;
    case OUTPUT_CHANNEL2:
        out = 1;
        die = 0;
        JL_IOMAP->CON3 &= ~(0xF << 24);
        JL_IOMAP->CON3 |= (UART_DATA_OUTCH_MAP_SET() << 24);
        break;
    case OUTPUT_CHANNEL3:
        out = 1;
        die = 1;
        JL_IOMAP->CON3 &= ~(0xF << 28);
        JL_IOMAP->CON3 |= (UART_DATA_OUTCH_MAP_SET() << 28);
        break;
    }
    gpio_set_pull_down(TIMER_UART_TX_PORT, 1);
    gpio_set_pull_up(TIMER_UART_TX_PORT, 1);
    gpio_direction_output(TIMER_UART_TX_PORT, out);
    gpio_set_die(TIMER_UART_TX_PORT, die);
#endif

    /* 4 . 使能特殊串口,启动收发数据 */
    dev_ioctl(uart_hdl, UART_START, 0);

    sys_timeout_add_to_task("sys_timer", NULL, time_uart_calibration_update, 15 * 1000); //50ms检测1次
    sys_timeout_add_to_task("sys_timer", NULL, time_uart_calibration_update, 20 * 1000); //50ms检测1次
    sys_timeout_add_to_task("sys_timer", NULL, time_uart_calibration_update, 30 * 1000); //50ms检测1次
    while (1) {
        if (++cnt % (60 * 60) == 0) { //60分钟校准一次时间
            time_update_check = 1;
        }
        if (time_update_check) {
            if (time_update_send_to_uart() == 0) {
                time_update_check = 0;
                printf("-> timer uart send ok\n");
            }
        }
        os_time_dly(100);
    }
    dev_close(uart_hdl);
}
static int uart_task_init(void)
{
    if (production_test_io_get()) {
        return 0;
    }
    os_task_create(uart_task_main, NULL, 10, 1000, 0, "uart_task");
    return 0;
}
late_initcall(uart_task_init);
#endif
