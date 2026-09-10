#include "system/includes.h"
#include "asm/port_waked_up.h"
#include "typedef.h"
#include "os/os_api.h"
#include "asm/iic.h"
#include "device/iic.h"
#include "device/device.h"
#include "app_config.h"
#include "system/includes.h"
#include "gpio.h"
#include "ui_api.h"
#include "touch_event.h"
#include "sys_common.h"
#include "lcd_config.h"

#if TCFG_TOUCH_GST328_ENABLE

#if 1
#define log_info(x, ...)    printf("\n[touch]>" x " \n", ## __VA_ARGS__)
#else
#define log_info(...)
#endif

struct touch_hdl {
    u16 x;
    u16 y;
    u8 status;
};

static OS_SEM touch_sem;

static void my_touch_test_task(void *priv);
int GST328_task_init(void);

extern int ui_touch_msg_post(struct touch_event *event);

//I2C读写命令
#define GST328_WRCMD            0x34        //写命令
#define GST328_RDCMD            0x35        //读命令

#define GT_NORMAL_MODE          0XD109      //GST328正常模式
#define GT_ASYNC_CMD            0XD000AB    //GST328同步命令

#define GT_GSTID_REG            0XD000      //GST328当前检测到的触摸情况
#define GT_TP1_REG              0XD001      //第一个触摸点数据地址
#define GT_TP2_REG              0XD002      //第二个触摸点数据地址
#define GT_TP3_REG              0XD003      //第三个触摸点数据地址
#define GT_TP4_REG              0XD004      //第四个触摸点数据地址
#define GT_TP5_REG              0XD005      //第五个触摸点数据地址
#define GT_TP5_REG              0XD006      //第五个触摸点数据地址
#define GT_TP5_REG              0XD007      //第五个触摸点数据地址

static void *iic = NULL;
static unsigned char wrGST328Reg(u16 regID, unsigned char regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, GST328_WRCMD)) {
        ret = 0;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(100);
    if (dev_ioctl(iic, IIC_IOCTL_TX, (regID >> 8) & 0xFF)) {
        ret = 0;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(100);
    if (dev_ioctl(iic, IIC_IOCTL_TX, regID & 0xFF)) {
        ret = 0;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(100);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regDat)) {
        ret = 0;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(100);

exit:
    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;
}
static unsigned char wrGST328Reg24(u32 regID, unsigned char regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, GST328_WRCMD)) {
        ret = 0;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(100);
    if (dev_ioctl(iic, IIC_IOCTL_TX, (regID >> 16) & 0xFF)) {
        ret = 0;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(100);
    if (dev_ioctl(iic, IIC_IOCTL_TX, (regID >> 8) & 0xFF)) {
        ret = 0;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(100);
    if (dev_ioctl(iic, IIC_IOCTL_TX, regID & 0xFF)) {
        ret = 0;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(100);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regDat)) {
        ret = 0;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(100);

exit:
    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;
}

static unsigned char rdGST328Reg(u16 regID, u8 *regDat)
{
    u8 ret = 1;
    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, GST328_WRCMD)) {
        ret = 0;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(100);
    if (dev_ioctl(iic, IIC_IOCTL_TX, (regID >> 8) & 0xFF)) {
        ret = 0;;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(100);
    if (dev_ioctl(iic, IIC_IOCTL_TX, regID & 0xff)) {
        ret = 0;;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }

    delay(100);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, GST328_RDCMD)) {
        ret = 0;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(100);
    dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)regDat);
exit:
    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;
}

static int GST328_get_pid(void)
{
//    u8 temp = 0;
//    rdGST328Reg(GT_PID_REG0, &temp);
//    if (temp != 57) { // 9
//        return 1;
//    }
//    rdGST328Reg(GT_PID_REG1, &temp);
//    if (temp != 49) { // 1
//        return 1;
//    }
//    rdGST328Reg(GT_PID_REG2, &temp);
//    if (temp != 49) { // 1
//        return 1;
//    }
//    log_info(">>>>>>>>>>>hell touch GST328<<<<<<<<<<<");
    return 0;
}

static char get_GST328_xy(u16 addr, u16 *x, u16 *y)
{
    u8 buf[8] = {0};
    char press = 0;
    rdGST328Reg(addr, &buf[0]);
    rdGST328Reg(addr + 1, &buf[1]);
    rdGST328Reg(addr + 2, &buf[2]);
    rdGST328Reg(addr + 3, &buf[3]);
    rdGST328Reg(addr + 4, &buf[4]);
    rdGST328Reg(addr + 5, &buf[5]);
    rdGST328Reg(addr + 6, &buf[6]);
    rdGST328Reg(addr + 7, &buf[7]);

    press = buf[0] & 0xF;
    *x = (buf[1] << 4) | ((buf[3] >> 4) & 0xF);
    *y = (buf[2] << 4) | (buf[3] & 0xF);

    wrGST328Reg24(GT_ASYNC_CMD, 0);
    return press;
}
static char get_GST328_status(u16 addr)
{
    char stauts = 0;
    rdGST328Reg(addr, &stauts);
    return stauts;
}
static u16 touch_x = 0;
static u16 touch_y = 0;
static u8 touch_status = 0;
static u8 touch_sleep = 0;

static void GST328_interrupt(void)
{
    os_sem_post(&touch_sem);
}
static void GST328_get_xy(unsigned short *x, unsigned short *y, char *press)
{
    if (x) {
        *x = touch_x;
    }
    if (y) {
        *y = touch_y;
    }
    if (press) {
        *press = touch_status;
    }
}

static int GST328_check(void*iic_dev_name)
{
    if (!iic_dev_name) {
        printf("err in %s iic_dev_name\n", __func__);
        return -1;
    }
    iic = dev_open(iic_dev_name, NULL);
    extern const struct ui_devices_cfg ui_cfg_data;
    static const struct ui_lcd_platform_data *pdata;
    pdata = (struct ui_lcd_platform_data *)ui_cfg_data.private_data;

    gpio_direction_output(pdata->touch_reset_pin, 1);
    os_time_dly(20);
    gpio_direction_output(pdata->touch_reset_pin, 0);
    os_time_dly(10);
    gpio_direction_output(pdata->touch_reset_pin, 1);
    os_time_dly(20);
    //gpio_direction_output(pdata->touch_reset_pin, 0);

//    if (get_GST328_pid()) {
//        log_info("[err]>>>>>GST328 err!!!");
//        dev_close(iic);
//        return 1;
//    }
    os_sem_create(&touch_sem, 0);
    //注册中断注意触摸用的事件0 屏幕TE用的事件1
    port_wakeup_reg(EVENT_IO_0, pdata->touch_int_pin, EDGE_NEGATIVE, GST328_interrupt);
    printf(">>>>>>>>>>>>hello GST328<<<<<<<<<<<");
    return 0;

}
static int GST328_init(void)
{
    int ret = 0;
    u8 touch_sleep_cb = 0;
    printf("[GST328]>>>>>GST328 start!!!");
    if (!wrGST328Reg(GT_NORMAL_MODE, 0)) {//设置正常模式
        os_time_dly(10);
        wrGST328Reg(GT_NORMAL_MODE, 0);//设置正常模式
    }
    while (1) {
        ret = os_sem_pend(&touch_sem, 50);
        if (!ret && touch_sleep && !touch_sleep_cb) {
            touch_sleep_cb = true;
            lcd_touch_irq_wakeup_callback();
            continue;
        }
        if (touch_sleep) {
            continue;
        }
        touch_sleep_cb = false;
        if (!ret) {
            touch_status = get_GST328_xy(GT_GSTID_REG, &touch_x, &touch_y) == 6;
            //printf("[GST328]>>>>>touch_status: %d, touch_x: %d, touch_y: %d\n", touch_status, touch_x, touch_y);
        } else {//500ms没有触摸则强制清空触摸状态
            touch_status = 0;
        }
        //printf("[GST328]>>>>>touch_status: %d, touch_x: %d, touch_y: %d\n", touch_status, touch_x, touch_y);
    }
    return 0;
}

static int GST328_sleep(char sleep)
{
    touch_sleep = sleep;
    return touch_sleep;
}

static int GST328_task_init(void)
{
    return thread_fork("touch_task", 20, 1024, 0, NULL, GST328_init, NULL);
}

REGISTER_LCD_TOUCH_DEV(GST328) = {
    .name                           = "GST328",
    .iic_dev_name                   = "iic0",
    .LCD_Touch_check                = GST328_check,
    .LCD_Touch_init                 = GST328_task_init,
    .LCD_Touch_get                  = GST328_get_xy,
    .LCD_Touch_sleep                = GST328_sleep,
    .LCD_Touch_enable               = NULL,
    .LCD_Touch_get_id               = GST328_get_pid,
};
#endif

