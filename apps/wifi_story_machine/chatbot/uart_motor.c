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

#ifdef UART_MOTOR_ENABLE
static void *uart_hdl = NULL;
static u8 uart_buf[1024] __attribute__((aligned(32))); //用于串口接收缓存数据的循环buf
static u8 last_motor_dir = 0;
static u8 last_dir = 0;
static u8 last_dir_num = 0;
static u8 motor_dir_start = 0;
static int hw_time_id = 0;
static int hw_time_cnt = 0;
static int now_dir = 0;

#define MOTOR_DIR_CNT 12 //12个方向
//sys_hi_timer_add(NULL, one_sec_time_callback, 1000);
static int motor_check_time_callback(void)
{
    hw_time_cnt++;
}
int motor_dir_set_enable(char enable)
{
    motor_dir_start = enable;
    return motor_dir_start;
}
static int motor_check_direction_ctrol(char mode)
{
    if (mode == 0) { //刹车
        gpio_direction_output(TCFG_MOTOR_CTRLA_PORT, 0);
        gpio_direction_output(TCFG_MOTOR_CTRLB_PORT, 0);
    } else if (mode == 1) { //正转
        gpio_direction_output(TCFG_MOTOR_CTRLA_PORT, 1);
        gpio_direction_output(TCFG_MOTOR_CTRLB_PORT, 0);
    } else if (mode == 2) { //反转
        gpio_direction_output(TCFG_MOTOR_CTRLA_PORT, 0);
        gpio_direction_output(TCFG_MOTOR_CTRLB_PORT, 1);
    }
}
static int motor_check_new_dir_callback(void)
{
    hw_time_id = 0;
    motor_check_direction_ctrol(0);
}
static int motor_check_direction_start(char mode, int time_ms)
{
    motor_check_direction_ctrol(mode);
    hw_time_id = sys_hi_timeout_add(NULL, motor_check_new_dir_callback, time_ms);
}
static int motor_check_io_read(void)
{
    static char io_filter = 0;
    static int last_check = 0;
    unsigned char now_need_dir = 0;
    unsigned char dir = 1;
    int tm_ms = 0;
    int gpio = gpio_read(TCFG_MOTOR_CHECK_PORT);
    if (!gpio) {
        io_filter++;
        if (io_filter >= 3 && (timer_get_ms() - last_check) >= 3000) { //在12方向的0和360度处
            last_check = timer_get_ms();
            if (last_dir == 1) { //正转
                tm_ms = last_dir_num * hw_time_cnt / MOTOR_DIR_CNT;
                now_need_dir = last_dir_num;
            } else if (last_dir == 2) { //反转
                tm_ms = (MOTOR_DIR_CNT - last_dir_num) * hw_time_cnt / MOTOR_DIR_CNT;
                now_need_dir = (MOTOR_DIR_CNT - last_dir_num);
            } else {
                return 0;
            }
            if (hw_time_id) {
                sys_timer_modify(hw_time_id, tm_ms);
                printf("-> modify dir = %d , now_need = %d, time = %d ms \n", dir, now_need_dir, tm_ms);
            }
        }
    } else {
        io_filter = 0;
    }
    return 0;
}
static int motor_check_dir_set(int dir_num)
{
    unsigned char now_need_dir = 0;
    unsigned char dir = 1;
    int tm_ms = 0;

    if (hw_time_id || dir_num == last_dir_num) { //上一次正在执行
        return -1;
    }
    if (((int)dir_num - (int)last_dir_num) >= 0) { //正转
        now_need_dir = dir_num - last_dir_num;
        dir = 1;
    } else { //反转
        now_need_dir = last_dir_num - dir_num;
        dir = 2;
    }
    tm_ms = now_need_dir * hw_time_cnt / MOTOR_DIR_CNT;
    if (now_need_dir > (MOTOR_DIR_CNT / 2 + 1)) {
        dir = dir == 1 ? 2 : 1;
        now_need_dir = MOTOR_DIR_CNT - now_need_dir;
        tm_ms = now_need_dir * hw_time_cnt / MOTOR_DIR_CNT;
    }
    last_dir = dir;
    last_dir_num = dir_num;
    if (now_need_dir) {
        printf("-> dir = %d , now_need = %d, time = %d ms \n", dir, now_need_dir, tm_ms);
        motor_check_direction_start(dir, tm_ms);
    }
    return 0;
}
static int motor_check_direction_init(void)
{
    int user_motor_dir_read(void);
    last_motor_dir = user_motor_dir_read();
    if (last_motor_dir != 1 && last_motor_dir != 2) {
        last_motor_dir = 0;
    }
    gpio_direction_input(TCFG_MOTOR_CHECK_PORT);
    gpio_set_pull_down(TCFG_MOTOR_CHECK_PORT, 0);
    gpio_set_pull_up(TCFG_MOTOR_CHECK_PORT, 1);
    gpio_set_die(TCFG_MOTOR_CHECK_PORT, 1);
    if (!hw_time_id) {
        hw_time_cnt = 0;
        hw_time_id = sys_hi_timer_add(NULL, motor_check_time_callback, 2);
    }
    u8 dir = 0;
    u8 dir_start = 0;
    int cnt = 0;
    int timr_cnt = 0;
    sys_timeout_add(0, motor_check_direction_ctrol, 30000);//防止行程开关失效
    dir_start = dir = last_motor_dir == 1 ? 2 : 1;
    printf("->last_motor_dir = %d, dir = %d \n", last_motor_dir, dir);
    motor_check_direction_ctrol(dir);
    while (1) {
        int gpio = gpio_read(TCFG_MOTOR_CHECK_PORT);
        if (!gpio && dir == dir_start) {
            cnt++;
//            printf("-> dir start\n");
            if (cnt > 5) {
                cnt = 0;
                dir = dir == 1 ? 2 : 1;
                while (!gpio_read(TCFG_MOTOR_CHECK_PORT)) {
                    os_time_dly(1);
                }
                timr_cnt = hw_time_cnt;
            }
        } else if (!gpio) {
            cnt++;
//            printf("-> dir end\n");
            if (cnt > 5) {
                dir = dir == 1 ? 2 : 1;
                while (!gpio_read(TCFG_MOTOR_CHECK_PORT)) {
                    os_time_dly(1);
                }
                timr_cnt = hw_time_cnt - timr_cnt;
                motor_check_direction_ctrol(0);
                user_motor_dir_write(dir);
                break;
            }
        } else if (gpio) {
            cnt = 0;
        }
        os_time_dly(1);
        //taskYIELD();
    }
    if (hw_time_id) {
        sys_hi_timer_del(hw_time_id);
        hw_time_id = 0;
    }
    motor_check_direction_ctrol(0);
    hw_time_cnt = timr_cnt * 2;
    last_dir_num = 1;
    motor_dir_start = 1;
    //sys_timer_add_to_task("sys_timer", NULL, motor_check_io_read, 50);
    printf("->hw_time_cnt : %d , %d ms\n", hw_time_cnt, timr_cnt * 2);
}

static void uart_motor_task_main(void *priv)
{
    unsigned int cnt = 0;
    char name[32];

    os_time_dly(100);

    motor_check_direction_init();

#ifdef UART_MOTOR_ENABLE
    sprintf(name, "uart%d", UART_MOTOR_ENABLE);
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

    u32 parm = 1000;
    dev_ioctl(uart_hdl, UART_SET_RECV_TIMEOUT, (u32)parm); //设置超时时间

    /* 4 . 使能特殊串口,启动收发数据 */
    dev_ioctl(uart_hdl, UART_START, 0);

    unsigned char recv_buf[64];
    while (1) {
        /* 5 . 接收数据 */
        int len = dev_read(uart_hdl, recv_buf, 64);
        if (len <= 0) {
//            printf("\n  uart recv err len = %d\n", len);
            if (len == UART_CIRCULAR_BUFFER_WRITE_OVERLAY) {
                printf("\n UART_CIRCULAR_BUFFER_WRITE_OVERLAY err\n");
                dev_ioctl(uart_hdl, UART_FLUSH, 0); //如果由于用户长期不取走接收的数据导致循环buf接收回卷覆盖,因此直接冲掉循环buf所有数据重新接收
            } else if (len == UART_RECV_TIMEOUT) {
                puts("UART_RECV_TIMEOUT...\r\n");
            }
            continue;
        }
        printf("\n uart recv len = %d\n", len);
        put_buf(recv_buf, len);
        if (recv_buf[0] == 0xFE && recv_buf[2] == 0x0D && motor_dir_start) {
            // MOTOR_DIR_CNT
            if (music_buf_play_stop_staus() && music_play_stop_status() && !net_music_play_start_status()) { //播放音乐不转动
                motor_check_dir_set((int)recv_buf[1]);
            }
        }
        memset(recv_buf, 0, sizeof(recv_buf));
//        //把串口接收到的数据发送回去
//        dev_write(uart_hdl, recv_buf, len);
//        cnt += len;
//        len = sprintf(send_buf, "Rx_Cnt = %d\n", cnt);
//        //统计串口接收到的数据发送回去
//        dev_write(uart_hdl, send_buf, len);
    }
    dev_close(uart_hdl);
}
static int uart_motor_task_init(void)
{
    if (production_test_io_get()) {
        return 0;
    }
    os_task_create(uart_motor_task_main, NULL, 10, 1000, 0, "uart_motor_task");
    return 0;
}
late_initcall(uart_motor_task_init);
#endif
