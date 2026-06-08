#include "system/includes.h"
#include "asm/port_waked_up.h"
#include "typedef.h"
#include "os/os_api.h"
#include "asm/iic.h"
#include "device/iic.h"
#include "device/device.h"
#include "app_config.h"
#include "gpio.h"
#include "ui_api.h"
#include "touch_event.h"
#include "sys_common.h"
#include "lcd_config.h"
#if TCFG_TOUCH_AXS15252_ENABLE
#if 1
#define log_info(x, ...)    printf("\n[AXS15252_touch]>" x " \n", ## __VA_ARGS__)
#else
#define log_info(...)
#endif
/* ---------- AXS15252 触摸上报协议（与 axs_platform.c / axs_core.h 一致）---------- */
#define AXS15252_MAX_POINTS         1
#define AXS15252_POINT_ONE_LEN      6
#define AXS15252_BUF_HEAD_LEN       2
#define AXS15252_REPORT_LEN         (AXS15252_MAX_POINTS * AXS15252_POINT_ONE_LEN + AXS15252_BUF_HEAD_LEN)
/* 触摸事件（buf 内 point 字节高 2 bit） */
#define AXS15252_EV_DOWN            0
#define AXS15252_EV_UP              1
#define AXS15252_EV_CONTACT         2
/* 与芯片通信：写首字节 0x76，读首字节 0x77（8bit 从机地址习惯） */
#define AXS15252_TP_WRCMD           0x76
#define AXS15252_TP_RDCMD           0x77
/* 与 touch_panel.c 一致：buf[1] 高半字节非 0 时丢弃本帧坐标；1=开启 */
#ifndef AXS15252_ESD_BUF1_CHECK
#define AXS15252_ESD_BUF1_CHECK     0
#endif
/* 读触摸 FIFO：先发 11 字节命令，再读 rd_len 字节 */
static const u8 axs15252_read_cmd_template[11] = {
    0x5A, 0xA5, 0xB5, 0xAB, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00
};
typedef struct {
    u8 gesture;
    u8 point_num;
    u8 touch_id;
    u8 event;
    u16 x;
    u16 y;
    u8 pressed;
} axs15252_touch_info_t;
struct touch_hdl_axs15252 {
    u16 x;
    u16 y;
    u8 status;
};
static OS_SEM touch_sem_axs15252;
static void my_touch_axs15252_task(void *priv);
int AXS15252_touch_task_init(void);
extern int ui_touch_msg_post(struct touch_event *event);
#define AXS15252_TP_CHIP_ID_REG     0x00
static void *iic_axs15252 = NULL;
/* 可选：与屏分辨率一致时在 lcd_config.h 中定义 LCD_WIDTH / LCD_HEIGHT */
#if defined(LCD_WIDTH) && defined(LCD_HEIGHT)
#define AXS15252_X_MAX  LCD_WIDTH
#define AXS15252_Y_MAX  LCD_HEIGHT
#else
#define AXS15252_X_MAX  4095
#define AXS15252_Y_MAX  4095
#endif
/*-----------------------------------------------------------------------------
 * I2C：写若干字节 -> STOP -> 短延时 -> 读若干字节（与 axs_soft_i2c 时序一致）
 *---------------------------------------------------------------------------*/
static int axs15252_write_bytes_read_bytes(const u8 *wt_buf, u16 wt_len, u8 *rd_buf, u16 rd_len)
{
    u16 i;
    if (!iic_axs15252 || !wt_buf || !rd_buf || wt_len == 0 || rd_len == 0) {
        return -1;
    }
    dev_ioctl(iic_axs15252, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic_axs15252, IIC_IOCTL_TX_WITH_START_BIT, AXS15252_TP_WRCMD)) {
        log_info("iic TX WRCMD fail %d", __LINE__);
        goto err_stop;
    }
    for (i = 0; i < wt_len - 1; i++) {
        if (dev_ioctl(iic_axs15252, IIC_IOCTL_TX, wt_buf[i])) {
            log_info("iic TX data fail %d", __LINE__);
            goto err_stop;
        }
    }
    if (dev_ioctl(iic_axs15252, IIC_IOCTL_TX_WITH_STOP_BIT, wt_buf[wt_len - 1])) {
        log_info("iic TX last+STOP fail %d", __LINE__);
        goto err_stop;
    }
    dev_ioctl(iic_axs15252, IIC_IOCTL_STOP, 0);
    /* 写停到读起之间的间隔，芯片需要时间准备数据（参考 STM32 驱动约几十 us） */
    delay(10);
    dev_ioctl(iic_axs15252, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic_axs15252, IIC_IOCTL_TX_WITH_START_BIT, AXS15252_TP_RDCMD)) {
        log_info("iic TX RDCMD fail %d", __LINE__);
        goto err_stop;
    }
    for (i = 0; i < rd_len - 1; i++) {
        if (dev_ioctl(iic_axs15252, IIC_IOCTL_RX_WITH_ACK, (u32)&rd_buf[i])) {
            log_info("iic RX ack fail %d", __LINE__);
            goto err_stop;
        }
    }
    if (dev_ioctl(iic_axs15252, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)&rd_buf[rd_len - 1])) {
        log_info("iic RX stop fail %d", __LINE__);
        goto err_stop;
    }
    dev_ioctl(iic_axs15252, IIC_IOCTL_STOP, 0);
    return 0;
err_stop:
    dev_ioctl(iic_axs15252, IIC_IOCTL_STOP, 0);
    return -1;
}
/**
 * 读触摸上报缓冲并解析为 axs15252_touch_info_t。
 * @return 0 成功；-1 I2C/参数错误；-2 本帧为 ESD/状态位异常（与原厂 touch_panel 一致可丢弃坐标）
 */
int axs15252_read_touch_info(axs15252_touch_info_t *info)
{
    u8 buf[AXS15252_REPORT_LEN];
    u8 read_cmd[11];
    u16 x, y;
    u8 event;
    u8 pn;
    if (!info) {
        return -1;
    }
    {
        unsigned int k;
        for (k = 0; k < sizeof(read_cmd); k++) {
            read_cmd[k] = axs15252_read_cmd_template[k];
        }
    }
    read_cmd[6] = (u8)(AXS15252_REPORT_LEN >> 8);
    read_cmd[7] = (u8)(AXS15252_REPORT_LEN & 0xff);
    if (axs15252_write_bytes_read_bytes(read_cmd, sizeof(read_cmd), buf, AXS15252_REPORT_LEN) != 0) {
        return -1;
    }
    info->gesture = buf[0];
    info->point_num = buf[1];
    /* 手势帧：不解析 XY（与 touch_panel.c 分支一致） */
    if (info->gesture != 0) {
        info->touch_id = 0;
        info->event = AXS15252_EV_UP;
        info->x = 0;
        info->y = 0;
        info->pressed = 0;
        return 0;
    }
#if AXS15252_ESD_BUF1_CHECK
    /* 与 touch_panel 一致：buf[1] 高半字节非 0 表示普通 ESD 状态上报 */
    if (buf[1] >> 4) {
        return -2;
    }
#endif
    event = (u8)(buf[2] >> 6);
    x = (u16)((buf[2] & 0x0Fu) << 8) | buf[3];
    y = (u16)((buf[4] & 0x0Fu) << 8) | buf[5];
    info->touch_id = (u8)(buf[4] >> 4);
    info->event = event;
    if (x >= AXS15252_X_MAX) {
        x = (u16)(AXS15252_X_MAX - 1);
    }
    if (y >= AXS15252_Y_MAX) {
        y = (u16)(AXS15252_Y_MAX - 1);
    }
    info->x = x;
    info->y = y;
    pn = (u8)(buf[1] & 0x0Fu);
    if (pn == 0 || event == AXS15252_EV_UP) {
        info->pressed = 0;
    } else {
        info->pressed = (event == AXS15252_EV_DOWN || event == AXS15252_EV_CONTACT) ? 1 : 0;
    }
    return 0;
}
static unsigned char axs15252_write_reg(uint16_t regID, uint8_t regDat)
{
    u8 ret = 1;
    dev_ioctl(iic_axs15252, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic_axs15252, IIC_IOCTL_TX_WITH_START_BIT, AXS15252_TP_WRCMD)) {
        ret = 0;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(10);
    if (dev_ioctl(iic_axs15252, IIC_IOCTL_TX, regID & 0xff)) {
        ret = 0;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(10);
    if (dev_ioctl(iic_axs15252, IIC_IOCTL_TX_WITH_STOP_BIT, regDat)) {
        ret = 0;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(10);
exit:
    dev_ioctl(iic_axs15252, IIC_IOCTL_STOP, 0);
    return ret;
}
static unsigned char axs15252_read_reg(u16 regID, u8 *regDat)
{
    u8 ret = 1;
    dev_ioctl(iic_axs15252, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic_axs15252, IIC_IOCTL_TX_WITH_START_BIT, AXS15252_TP_WRCMD)) {
        ret = 0;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(10);
    if (dev_ioctl(iic_axs15252, IIC_IOCTL_TX, regID & 0xff)) {
        ret = 0;
        log_info("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(10);
    if (dev_ioctl(iic_axs15252, IIC_IOCTL_TX_WITH_START_BIT, AXS15252_TP_RDCMD)) {
        ret = 0;
        goto exit;
    }
    delay(10);
    dev_ioctl(iic_axs15252, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)regDat);
exit:
    dev_ioctl(iic_axs15252, IIC_IOCTL_STOP, 0);
    return ret;
}
static int get_axs15252_chip_id(void)
{
    log_info(">>>>>>>>>>>hello touch AXS15252<<<<<<<<<<<");
    return -1;
}
/**
 * 内部：读 I2C 并更新 *x/*y/*press；press 为 1 表示按下/移动。
 * 返回 1 有有效按下/接触，0 抬起或无触点或读失败。
 */
static char get_axs15252_xy_raw(u16 *x, u16 *y)
{
    axs15252_touch_info_t ti;
    int r;
    if (!x || !y) {
        return 0;
    }
    r = axs15252_read_touch_info(&ti);
    if (r != 0) {
        return 0;
    }
    *x = ti.x;
    *y = ti.y;
    return (char)ti.pressed;
}
static char get_axs15252_status_reg(u16 addr)
{
    char status = 0;
    axs15252_read_reg(addr, (u8 *)&status);
    return status;
}
static u16 touch_x = 0;
static u16 touch_y = 0;
static u8 touch_status = 0;
static void axs15252_interrupt(void)
{
    os_sem_post(&touch_sem_axs15252);
}
/**
 * 获取最近一次中断任务更新后的坐标与按压状态（非线程安全，与历史 API 兼容）。
 */
void axs15252_get_xy(short *x, short *y, char *press)
{
    if (x) {
        *x = (short)touch_x;
    }
    if (y) {
        *y = (short)touch_y;
    }
    if (press) {
        *press = (char)touch_status;
    }
}
/**
 * 立即从 TP 读一帧并写入 *x/*y/*press；成功返回 0。
 */
int axs15252_fetch_xy_now(short *x, short *y, char *press)
{
    axs15252_touch_info_t ti;
    int r = axs15252_read_touch_info(&ti);
    if (r != 0) {
        if (press) {
            *press = 0;
        }
        return r;
    }
    if (x) {
        *x = (short)ti.x;
    }
    if (y) {
        *y = (short)ti.y;
    }
    if (press) {
        *press = (char)ti.pressed;
    }
    return 0;
}
int check_axs15252(void)
{
    iic_axs15252 = dev_open("iic0", NULL);
    extern const struct ui_devices_cfg ui_cfg_data;
    static const struct ui_lcd_platform_data *pdata;
    pdata = (struct ui_lcd_platform_data *)ui_cfg_data.private_data;
    gpio_direction_output(pdata->touch_reset_pin, 1);
    os_time_dly(20);
    gpio_direction_output(pdata->touch_reset_pin, 0);
    os_time_dly(10);
    gpio_direction_output(pdata->touch_reset_pin, 1);
    os_time_dly(60);
    os_sem_create(&touch_sem_axs15252, 0);
    port_wakeup_reg(EVENT_IO_0, pdata->touch_int_pin, EDGE_NEGATIVE, axs15252_interrupt);
    AXS15252_touch_task_init();
    printf(">>>>>>>>>>>>hello AXS15252 touch (W=0x%02X R=0x%02X)<<<<<<<<<<<\n",
           AXS15252_TP_WRCMD, AXS15252_TP_RDCMD);
    return 0;
}
static int AXS15252_init(void)
{
    printf("[AXS15252_touch]>>>>>start!!!\n");
    while (1) {
        axs15252_touch_info_t ti = {0};
        if (!os_sem_pend(&touch_sem_axs15252, 50)) {
            if (axs15252_read_touch_info(&ti) == 0) {
                touch_x = ti.x;
                touch_y = ti.y;
                touch_status = ti.pressed;
            } else {
                touch_status = 0;
            }
        } else {
            touch_status = 0;
        }
//        printf("[AXS15252] touch_status: %d, x: %d, y: %d, gesture: 0x%02X\n",
//               touch_status, touch_x, touch_y, ti.gesture);
    }
    return 0;
}
static void my_touch_axs15252_task(void *priv)
{
    AXS15252_init();
}
int AXS15252_touch_task_init(void)
{
    return thread_fork("axs15252_touch", 20, 1024, 0, NULL, my_touch_axs15252_task, NULL);
}
#endif /* TCFG_TOUCH_AXS15252_ENABLE */

