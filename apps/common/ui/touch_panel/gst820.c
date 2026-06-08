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

#if TCFG_TOUCH_GST820_ENABLE

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
int GST820_task_init(void);

extern int ui_touch_msg_post(struct touch_event *event);

//I2C读写命令
#define GST820_WRCMD            0X2A        //写命令
#define GST820_RDCMD            0X2B        //读命令

//GST820 部分寄存器定义
#define GT_CTRL_REG             0X8040      //GST820控制寄存器
#define GT_CFGS_REG             0X8047      //GST820配置起始地址寄存器
#define GT_CHECK_REG            0X80FF      //GST820校验和寄存器

#define GT_PID_REG0             0X8140      //GST820产品ID寄存器
#define GT_PID_REG1             0X8141      //GST820产品ID寄存器
#define GT_PID_REG2             0X8142      //GST820产品ID寄存器

#define GT_GSTID_REG            0X814E      //GST820当前检测到的触摸情况
#define GT_TP1_REG              0X8150      //第一个触摸点数据地址 //每个点4四个地址控制GT_TP1_X_L_REG
#define GT_TP2_REG              0X8158      //第二个触摸点数据地址
#define GT_TP3_REG              0X8160      //第三个触摸点数据地址
#define GT_TP4_REG              0X8168      //第四个触摸点数据地址
#define GT_TP5_REG              0X8170      //第五个触摸点数据地址

static void *iic = NULL;
static unsigned char wrGST820Reg(unsigned int regID, unsigned char regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, GST820_WRCMD)) {
        ret = 0;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
//    delay(1000);
//    if (dev_ioctl(iic, IIC_IOCTL_TX, regID >> 8)) {
//        ret = 0;
//        log_info("iic write err!!! line : %d \n", __LINE__);
//        goto exit;
//    }
    delay(1000);
    if (dev_ioctl(iic, IIC_IOCTL_TX, regID & 0xff)) {
        ret = 0;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(1000);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regDat)) {
        ret = 0;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(1000);

exit:
    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;
}

static unsigned char rdGST820Reg(u16 regID, u8 *regDat)
{
    u8 ret = 1;
    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, GST820_WRCMD)) {
        ret = 0;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
//    delay(100);
//    if (dev_ioctl(iic, IIC_IOCTL_TX, regID >> 8)) {
//        ret = 0;;
//        log_info("iic write err!!! line : %d \n", __LINE__);
//        goto exit;
//    }
    delay(100);
    if (dev_ioctl(iic, IIC_IOCTL_TX, regID & 0xff)) {
        ret = 0;;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }

    delay(100);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, GST820_RDCMD)) {
        ret = 0;
        goto exit;
    }
    delay(100);
    dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)regDat);
exit:
    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;
}

int get_GST820_pid(void)
{
    u8 temp = 0;
    rdGST820Reg(GT_PID_REG0, &temp);
    if (temp != 57) { // 9
        return 1;
    }
    rdGST820Reg(GT_PID_REG1, &temp);
    if (temp != 49) { // 1
        return 1;
    }
    rdGST820Reg(GT_PID_REG2, &temp);
    if (temp != 49) { // 1
        return 1;
    }
    log_info(">>>>>>>>>>>hell touch GST820<<<<<<<<<<<");
    return 0;
}

static char get_GST820_xy(u16 addr, u16 *x, u16 *y)
{
    u8 buf[4] = {0};
    char press = 0;
    rdGST820Reg(addr, &buf[0]);
    rdGST820Reg(addr + 1, &buf[1]);
    rdGST820Reg(addr + 2, &buf[2]);
    rdGST820Reg(addr + 3, &buf[3]);
    press = buf[0] >> 4;
    *x = ((buf[0] & 0xF) << 8) | buf[1];
    *y = ((buf[2] & 0xF) << 8) | buf[3];
    return press;
}
static char get_GST820_status(u16 addr)
{
    char stauts = 0;
    rdGST820Reg(addr, &stauts);
    return stauts;
}
static u16 touch_x = 0;
static u16 touch_y = 0;
static u8 touch_status = 0;

static void GST820_interrupt(void)
{
    os_sem_post(&touch_sem);
    //printf("\n [ERROR] %s -[yuyu] %d\n", __FUNCTION__, __LINE__);

    //IIC读取坐标值
    //get_GST820_xy(GT_TP1_REG, &touch_x, &touch_y);
}
void GST820_get_xy(short *x, short *y, char *press)
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

int check_GST820(void)
{
    iic = dev_open("iic0", NULL);
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

//    if (get_GST820_pid()) {
//        log_info("[err]>>>>>GST820 err!!!");
//        dev_close(iic);
//        return 1;
//    }
    os_sem_create(&touch_sem, 0);
    //注册中断注意触摸用的事件0 屏幕TE用的事件1
    port_wakeup_reg(EVENT_IO_0, pdata->touch_int_pin, EDGE_NEGATIVE, GST820_interrupt);
    GST820_task_init();
    printf(">>>>>>>>>>>>hello GST820<<<<<<<<<<<");
    return 0;

}
static int GST820_init(void)
{
    u8 status = 0;

    printf("[GST820]>>>>>GST820 start!!!");
    while (1) {
        if (!os_sem_pend(&touch_sem, 50)) {
            touch_status = get_GST820_xy(0x3, &touch_x, &touch_y) == 0x8;
        } else {//500ms没有触摸则强制清空触摸状态
            touch_status = 0;
        }
        //status = get_GST820_status(0x0);
        //touch_status = get_GST820_xy(0x3, &touch_x, &touch_y) == 0x8;
        //printf("[GST820]>>>>>touch_status: %d, touch_x: %d, touch_y: %d\n", touch_status, touch_x, touch_y);
    }

    return 0;
}

static void my_touch_test_task(void *priv)
{
    /*=====TE中断配置=====*/
    //check_GST820();
    GST820_init();
}

int GST820_task_init(void)
{
    return thread_fork("my_touch_test_task", 20, 1024, 0, NULL, my_touch_test_task, NULL);
}

#endif

