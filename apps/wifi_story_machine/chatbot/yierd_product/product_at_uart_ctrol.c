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

#ifdef CONFIG_WIFI_ENABLE
#include "wifi/wifi_connect.h"
#endif

#ifdef PRODUCTION_YIERD_UART_CMD_ENABLE

static void *at_uart_hdl = NULL;

static u8 at_uart_buf[1024] __attribute__((aligned(32))); //用于串口接收缓存数据的循环buf


void ai_at_uart_send_data(char *buf, int len)
{
    if (at_uart_hdl) {
        dev_write(at_uart_hdl, buf, len);
    }
}

static int ai_at_uart_cmd_recv(char *buf, int len)
{
    extern int ai_at_handle(char* buf, int len);
    if (ai_at_handle(buf, len) != 0) {
        return 1;
    }
    return -1;
}

static void ai_at_uart_task_main(void *priv)
{
    unsigned int cnt = 0;
    unsigned char recv_buf[64] ALIGNED(32);
    char name[32];
    at_uart_hdl = dev_open("uart1", NULL);
    if (!at_uart_hdl) {
        printf("open uart1 err !!!\n");
        return ;
    }
    /* 1 . 设置串口接收缓存数据的循环buf地址 */
    dev_ioctl(at_uart_hdl, UART_SET_CIRCULAR_BUFF_ADDR, (int)at_uart_buf);

    /* 1 . 设置串口接收缓存数据的循环buf长度 */
    dev_ioctl(at_uart_hdl, UART_SET_CIRCULAR_BUFF_LENTH, sizeof(at_uart_buf));

    /* 3 . 设置接收数据为阻塞方式,需要非阻塞可以去掉,建议加上超时设置 */
    dev_ioctl(at_uart_hdl, UART_SET_RECV_BLOCK, 1);

    u32 parm = 1000;
    dev_ioctl(at_uart_hdl, UART_SET_RECV_TIMEOUT, (u32)parm); //超时设置

    /* 4 . 使能特殊串口,启动收发数据 */
    dev_ioctl(at_uart_hdl, UART_START, 0);
    while (1) {
        /* 5 . 接收数据 */
        memset(recv_buf, 0, 64);
        int len = dev_read(at_uart_hdl, recv_buf, 64);
        if (len <= 0) {
            //printf("\n  uart recv err len = %d\n", len);
            if (len == UART_CIRCULAR_BUFFER_WRITE_OVERLAY) {
                printf("\n UART_CIRCULAR_BUFFER_WRITE_OVERLAY err\n");
                dev_ioctl(at_uart_hdl, UART_FLUSH, 0); //如果由于用户长期不取走接收的数据导致循环buf接收回卷覆盖,因此直接冲掉循环buf所有数据重新接收
            } else if (len == UART_RECV_TIMEOUT) {
                //puts("UART_RECV_TIMEOUT...\r\n");
            }
            continue;
        }
        printf("\n uart1 recv len = %d\n", len);
        put_buf(recv_buf, len);
        ai_at_uart_cmd_recv(recv_buf, len);//接收到指令
    }
    dev_close(at_uart_hdl);
}
static int ai_at_uart_task_init(void)
{
    if (production_test_io_get()) {
        os_task_create(ai_at_uart_task_main, NULL, 10, 1000, 0, "ai_at_uart_task_main");
    }
    return 0;
}
late_initcall(ai_at_uart_task_init);
#endif
