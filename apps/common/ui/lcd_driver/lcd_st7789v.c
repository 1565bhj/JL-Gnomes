

#include "system/includes.h"
#include "typedef.h"
#include "asm/pap.h"
#include "lcd_drive.h"
#include "lcd_config.h"

/*  ST7789V驱动说明 该驱动测试时使用的 wl80 79系列
 *  由于该IC推屏能力不够强 推屏的帧数较低 大概在25帧左右
 *  在推屏过程中需要使用TE屏幕帧中断 不然会有条纹
 *  由于ST7789V横屏配置无法调出没有条纹的配置
 *  所有只能使用竖屏加RGB旋转来实现UI横屏显示
 */

/* //pap的这个三个配置如下 在板级文件中进行修改
    .timing_setup   = 0,                    //具体看pap.h
    .timing_hold    = 0,                    //具体看pap.h
    .timing_width   = 1,                    //具体看pap.h
*/

#if TCFG_LCD_ST7789V_ENABLE


#define READ_ID     0x04
#define REGFLAG_DELAY 0x45

void ST7789V_SetRange(u16 xs, u16 xe, u16 ys, u16 ye)
{
    /******UI每次发送数据都会调用开窗告诉屏幕要刷新那个区域***********/
    set_lcd_ui_x_y(xs, xe, ys, ye);
}
void ST7789V_SetRange_1(u16 xs, u16 xe, u16 ys, u16 ye)
{
    WriteCOM(0x2A);
    WriteDAT_8(ys >> 8);
    WriteDAT_8(ys);
    WriteDAT_8(ye >> 8);
    WriteDAT_8(ye);
    WriteCOM(0x2B);
    WriteDAT_8(xs >> 8);
    WriteDAT_8(xs);
    WriteDAT_8(xe >> 8);
    WriteDAT_8(xe);

}

static void ST7789V_lvgl_Fill(u16 xs, u16 xe, u16 ys, u16 ye, u8 *img)
{
    u32 len = 0;
    lcd_interface_non_block_wait();
    len = (xe + 1 - xs) * (ye + 1 - ys) * 2;
    WriteCOM(0x2A);
    WriteDAT_8(xs >> 8);
    WriteDAT_8(xs);
    WriteDAT_8(xe >> 8);
    WriteDAT_8(xe);
    WriteCOM(0x2B);
    WriteDAT_8(ys >> 8);
    WriteDAT_8(ys);
    WriteDAT_8(ye >> 8);
    WriteDAT_8(ye);
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);
}

void ST7789V_clear_screen(u32 color)
{
    lcd_interface_non_block_wait();
    WriteCOM(0x2c);

    u8 *buf = malloc(LCD_W * LCD_H * 2);
    if (!buf) {
        printf("no men in %s \n", __func__);
        return;
    }
    for (u32 i = 0; i < LCD_W * LCD_H; i++) {
        buf[2 * i] = (color >> 8) & 0xff;
        buf[2 * i + 1] = color & 0xff;
    }
    WriteDAT_DMA(buf, LCD_W * LCD_H * 2);
    free(buf);
}

void ST7789V_Fill(u8 *img, u16 len)
{
    lcd_interface_non_block_wait();
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);
}

void ST7789V_SleepInMode(void)
{
    WriteCOM(0x10); //Sleep in
    lcd_delay(120); //Delay 120ms
}

void ST7789V_SleepOutMode(void)
{
    WriteCOM(0x11); //Sleep out
    lcd_delay(120);  //Delay 120ms
}

void st7789_shown_image(u8 *buff, u16 x_addr, u16 y_addr, u16 width, u16 height)
{
    lcd_interface_non_block_wait();
    ST7789V_SetRange(x_addr, y_addr, width, height);
    WriteDAT_DMA(buff, width * height * 2);
}

static void ST7789V_set_direction(int dir)
{
    WriteCOM(0x36);    //扫描方向控制
    if (dir == ROTATE_DEGREE_0) { //
#if HORIZONTAL_SCREEN
        WriteDAT_8(0xA0);
#else
        WriteDAT_8(0x00);
#endif
        ST7789V_SetRange(0, LCD_W - 1, 0, LCD_H - 1);
    } else if (dir == ROTATE_DEGREE_180) { //翻转180
#if HORIZONTAL_SCREEN
        WriteDAT_8(0xc0);
#else
        WriteDAT_8(0x80);
#endif
    } else if (dir == ROTATE_DEGREE_90) { //翻转90
        WriteDAT_8(0x60);
        ST7789V_SetRange(0, LCD_W - 1, 0, LCD_H - 1);
    }
}

static void ST7789V_draw(u8 *map, u32 size)//获取Ui发送出来的数据
{
    ui_send_data_ready(map, size);
}

static void ST7789V_draw_1(u8 *buf, u32 size, int width, int height)//获取camera发送出来的数据 //数据帧数以camera为基准
{
    camera_send_data_ready(buf, size, width, height);
}

static void ST7789V_reset(void)
{
    printf("reset \n");
    lcd_rst_pinstate(1);
    lcd_rs_pinstate(1);
    lcd_cs_pinstate(1);

    lcd_rst_pinstate(1);
    lcd_delay(60);
    lcd_rst_pinstate(0);
    lcd_delay(10);
    lcd_rst_pinstate(1);
    lcd_delay(100);
}
static void ST7789V_init_code(void *code, u8 cnt)
{
    WriteCOM(0x01);
    WriteDAT_8(0x00);

    lcd_delay(120);                //ms

    WriteCOM(0x11);
    WriteDAT_8(0x00);

    lcd_delay(120);                //ms

    WriteCOM(0x35);
    WriteDAT_8(0x00);

    WriteCOM(0x36);
    WriteDAT_8(0x00);

    WriteCOM(0x3A);
    WriteDAT_8(0x05);

    WriteCOM(0xB2);
    WriteDAT_8(0x0C);
    WriteDAT_8(0x0C);
    WriteDAT_8(0x00);
    WriteDAT_8(0x33);
    WriteDAT_8(0x33);

    WriteCOM(0xB7);
    WriteDAT_8(0x75);   //VGH=14.97V, VGL=-10.43V

    WriteCOM(0xBB);     //VCOM
    WriteDAT_8(0x38);

    WriteCOM(0xC0);
    WriteDAT_8(0x2C);

    WriteCOM(0xC2);
    WriteDAT_8(0x01);

    WriteCOM(0xC3);     //GVDD
    WriteDAT_8(0x13);

    WriteCOM(0xC4);
    WriteDAT_8(0x20);

    WriteCOM(0xC6);
    WriteDAT_8(0x0F);

    WriteCOM(0xD0);
    WriteDAT_8(0xA4);
    WriteDAT_8(0xA1);

    WriteCOM(0xd6);
    WriteDAT_8(0xa1);

#if HORIZONTAL_SCREEN
    WriteCOM(0x2A);
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);//Start_Y=0
    WriteDAT_8(0x01);
    WriteDAT_8(0x40);//End_Y=320
    WriteCOM(0x2B);
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);//Start_X=0
    WriteDAT_8(0x00);
    WriteDAT_8(0xF0);//End_X=240
#else
    WriteCOM(0x2A);
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);//Start_X=0
    WriteDAT_8(0x00);
    WriteDAT_8(0xF0);//End_X=240
    WriteCOM(0x2B);
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);//Start_Y=0
    WriteDAT_8(0x01);
    WriteDAT_8(0x40);//End_Y=320
#endif
//    WriteCOM(0x11);
//    lcd_delay(140);

    WriteCOM(0xE0);
    WriteDAT_8(0xD0);
    WriteDAT_8(0x0D);
    WriteDAT_8(0x17);
    WriteDAT_8(0x11);
    WriteDAT_8(0x11);
    WriteDAT_8(0x09);
    WriteDAT_8(0x3B);
    WriteDAT_8(0x45);
    WriteDAT_8(0x47);
    WriteDAT_8(0x0A);
    WriteDAT_8(0x12);
    WriteDAT_8(0x10);
    WriteDAT_8(0x17);
    WriteDAT_8(0x18);

    WriteCOM(0xE1);
    WriteDAT_8(0xD0);
    WriteDAT_8(0x07);
    WriteDAT_8(0x0E);
    WriteDAT_8(0x10);
    WriteDAT_8(0x0F);
    WriteDAT_8(0x1B);
    WriteDAT_8(0x36);
    WriteDAT_8(0x43);
    WriteDAT_8(0x4A);
    WriteDAT_8(0x39);
    WriteDAT_8(0x19);
    WriteDAT_8(0x19);
    WriteDAT_8(0x1C);
    WriteDAT_8(0x22);

    WriteCOM(0x29);
    lcd_delay(20);
    WriteCOM(0x2C);
    lcd_delay(20);
}

static void ST7789V_led_ctrl(u8 status)
{
    //背光控制以及放在//lcd_te_driver.c 优化开机显示
    /*lcd_bl_pinstate(status);*/
}

void ST7789V_test(void)
{
    ST7789V_clear_screen(BLACK);
    lcd_delay(50);//每个屏幕可能会在刷新清屏了，但是屏幕芯片缓存还没刷到屏幕，需要加延时
    lcd_bl_pinstate(1);
//    while (1) {
//        os_time_dly(100);
//        ST7789V_clear_screen(BLUE);
//        printf("LCD_ST7789V_TSET_BLUE\n");
//        os_time_dly(100);
//        ST7789V_clear_screen(DCYAN);
//        printf("LCD_ST7789V_TSET_GRED\n");
//        os_time_dly(100);
//        ST7789V_clear_screen(DGREEN);
//        printf("LCD_ST7789V_TSET_BRRED\n");
//        os_time_dly(100);
//        ST7789V_clear_screen(YELLOW);
//        printf("LCD_ST7789V_TSET_YELLOW\n");
//    }
}

static int ST7789V_init(void)
{
    printf("LCD_ST7789V init_start\n");
    ST7789V_reset();
    ST7789V_init_code(NULL, 0);
    ST7789V_set_direction(ROTATE_DEGREE_90);
#if USE_LCD_TE
    lcd_te_interrupt_init(1);
#endif
    ST7789V_test();
    printf("LCD_ST7789V config succes\n");
    return 0;
}


REGISTER_LCD_DEV(LCD_ST7789V) = {
    .name              = "ST7789V",
    .lcd_width         = LCD_W,
    .lcd_height        = LCD_H,
    .color_format      = LCD_COLOR_RGB565,
    .column_addr_align = 1,
    .row_addr_align    = 1,
    .LCD_Init          = ST7789V_init,
    .SetDrawArea       = ST7789V_SetRange,
    .LCD_Draw          = ST7789V_draw,
    .LCD_Draw_1        = ST7789V_draw_1,
    .LCD_DrawToDev     = ST7789V_Fill,//应用层直接到设备接口层，需要做好缓冲区共用互斥，慎用！
    .LCD_Lvgl_Full     = ST7789V_lvgl_Fill,//LVGL发送数据接口
    .LCD_ClearScreen   = ST7789V_clear_screen,
    .Reset             = ST7789V_reset,
    .BackLightCtrl     = ST7789V_led_ctrl,
};

#endif


