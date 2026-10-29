#include "system/includes.h"
#include "typedef.h"
#include "asm/pap.h"
#include "app_config.h"
#include "lcd_drive.h"
#include "lcd_config.h"

/* //pap的这个三个配置如下 在板级文件中进行修改
    .timing_setup   = 0,                    //具体看pap.h
    .timing_hold    = 0,                    //具体看pap.h
    .timing_width   = 1,                    //具体看pap.h
*/

#if TCFG_LCD_ST7735SV_ENABLE

void ST7735SV_SetRange(u16 xs, u16 xe, u16 ys, u16 ye)
{
    /******UI每次发送数据都会调用开窗告诉屏幕要刷新那个区域***********/
    set_lcd_ui_x_y(xs, xe, ys, ye);
}
void ST7735SV_SetRange_1(u16 xs, u16 xe, u16 ys, u16 ye)
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
static void ST7735SV_lvgl_Fill(u16 xs, u16 xe, u16 ys, u16 ye, u8 *img)
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
void ST7735SV_clear_screen(u32 color)
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

void ST7735SV_Fill(u8 *img, u16 len)
{
    lcd_interface_non_block_wait();
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);
}

static int ST7735SV_Sleep(char on)
{
    return on;//确保LCD屏幕支持睡眠指令再进行修改
    if (on) {
        WriteCOM(0x28);
        WriteCOM(0x10); //Sleep in
        lcd_delay(120); //Delay 120ms
    } else {
        WriteCOM(0x11); //Sleep out
        lcd_delay(120);  //Delay 120ms
        WriteCOM(0x29);
    }
    return on;
}

void ST7789_shown_image(u8 *buff, u16 x_addr, u16 y_addr, u16 width, u16 height)
{
    lcd_interface_non_block_wait();
    ST7735SV_SetRange(x_addr, y_addr, width, height);
    WriteDAT_DMA(buff, width * height * 2);
}

static void ST7735SV_set_direction(int dir)
{
//    WriteCOM(0x36); // 扫描方向控制
//    if (dir == ROTATE_DEGREE_0) {
//        //
//#if HORIZONTAL_SCREEN
//        WriteDAT_8(0xA8);
//#else
//        WriteDAT_8(0xC8);
//#endif
//    } else if (dir == ROTATE_DEGREE_180) {
//        // 翻转180
//#if HORIZONTAL_SCREEN
//        WriteDAT_8(0xc0);
//#else
//        WriteDAT_8(0x80);
//#endif
//    } else if (dir == ROTATE_DEGREE_90) {
//        // 翻转90
//        WriteDAT_8(0x50);
//    }
}


static void ST7735SV_draw(u8 *map, u32 size) // 获取Ui发送出来的数据
{
    ui_send_data_ready(map, size);
}

static void ST7735SV_draw_1(u8 *buf, u32 size, int width, int height) // 获取camera发送出来的数据 //数据帧数以camera为基准
{
    camera_send_data_ready(buf, size, width, height);
}

static void ST7735SV_reset(void)
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
static void ST7735SV_init_code(void)
{

    lcd_delay(120); // 延时120ms

    WriteCOM(0x11); // Sleep out

    lcd_delay(120); // lcd_delay 120ms

    WriteCOM(0xB1);
    WriteDAT_8(0x05);
    WriteDAT_8(0x3C);
    WriteDAT_8(0x3C);

    WriteCOM(0xB2);
    WriteDAT_8(0x05);
    WriteDAT_8(0x3C);
    WriteDAT_8(0x3C);

    WriteCOM(0xB3);
    WriteDAT_8(0x05);
    WriteDAT_8(0x3C);
    WriteDAT_8(0x3C);
    WriteDAT_8(0x05);
    WriteDAT_8(0x3C);
    WriteDAT_8(0x3C);

    WriteCOM(0xB4); // Dot inversion
    WriteDAT_8(0x03);

    WriteCOM(0xC0);
    WriteDAT_8(0xA4);
    WriteDAT_8(0x04);
    WriteDAT_8(0x84);

    WriteCOM(0xC1);
    WriteDAT_8(0xC8);

    WriteCOM(0xC2);
    WriteDAT_8(0x0D);
    WriteDAT_8(0x00);

    WriteCOM(0xC3);
    WriteDAT_8(0x8D);
    WriteDAT_8(0x2A);

    WriteCOM(0xC4);
    WriteDAT_8(0x8D);
    WriteDAT_8(0xEE);

    WriteCOM(0xC5);
    WriteDAT_8(0x1D);

    WriteCOM(0xE0);
    WriteDAT_8(0x0C);
    WriteDAT_8(0x0A);
    WriteDAT_8(0x06);
    WriteDAT_8(0x00);
    WriteDAT_8(0x1A);
    WriteDAT_8(0x11);
    WriteDAT_8(0x0B);
    WriteDAT_8(0x0A);
    WriteDAT_8(0x0B);
    WriteDAT_8(0x0C);
    WriteDAT_8(0x19);
    WriteDAT_8(0x33);
    WriteDAT_8(0x00);
    WriteDAT_8(0x06);
    WriteDAT_8(0x02);
    WriteDAT_8(0x10);

    WriteCOM(0xE1);
    WriteDAT_8(0x0D);
    WriteDAT_8(0x0F);
    WriteDAT_8(0x03);
    WriteDAT_8(0x00);
    WriteDAT_8(0x11);
    WriteDAT_8(0x0A);
    WriteDAT_8(0x06);
    WriteDAT_8(0x08);
    WriteDAT_8(0x09);
    WriteDAT_8(0x0D);
    WriteDAT_8(0x1C);
    WriteDAT_8(0x3A);
    WriteDAT_8(0x00);
    WriteDAT_8(0x09);
    WriteDAT_8(0x07);
    WriteDAT_8(0x10);

    WriteCOM(0x2A); // Set Column Address
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);
    WriteDAT_8(0x7F);

    WriteCOM(0x2B); // Set Page Address
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);
    WriteDAT_8(0x7F);

    WriteCOM(0x35);
    WriteDAT_8(0x00);

    WriteCOM(0x3A); // 65k mode
    WriteDAT_8(0x05);

    WriteCOM(0x36);
    WriteDAT_8(0xC8);

    WriteCOM(0x29); // Display on
    lcd_delay(120); // lcd_delay 120ms
}

static void ST7735SV_led_ctrl(u8 status)
{
    // 背光控制以及放在//lcd_te_driver.c 优化开机显示
    lcd_bl_pinstate(status);
}

void ST7735SV_color_set_screen(u16 xs, u16 ys, u16 xe, u16 ye, u16 color)
{
    WriteCOM(0x2A);
    WriteDAT_8(xs);
    WriteDAT_8(xs);
    WriteDAT_8(xe);
    WriteDAT_8(xe);
    WriteCOM(0x2B);
    WriteDAT_8(ys);
    WriteDAT_8(ys);
    WriteDAT_8(ye);
    WriteDAT_8(ye);

    WriteCOM(0x2c);
    u8 *buf = malloc((xe - xs) * (ye - ys) * 2);
    if (buf) {
        for (u32 i = 0; i < (xe - xs) * (ye - ys); i++) {
            buf[2 * i] = (color >> 8) & 0xff;
            buf[2 * i + 1] = color & 0xff;
        }
        WriteDAT_DMA(buf, (xe - xs) * (ye - ys) * 2);
        free(buf);
    }
}

void ST7735SV_test(void)
{
    ST7735SV_clear_screen(BLACK);
    lcd_delay(50);//每个屏幕可能会在刷新清屏了，但是屏幕芯片缓存还没刷到屏幕，需要加延时
    lcd_bl_pinstate(1);

#if 0
    while (1) {
        os_time_dly(10);
        ST7735SV_clear_screen(WHITE);
        printf("LCD_ST7735SV_TSET_BLUE\n");
        os_time_dly(12);
        ST7735SV_clear_screen(DGREEN);
        printf("LCD_ST7735SV_TSET_GRED\n");
        os_time_dly(12);
        ST7735SV_clear_screen(MAROON);
        printf("LCD_ST7735SV_TSET_BRRED\n");
        os_time_dly(12);
        ST7735SV_clear_screen(BLUE);
        printf("LCD_ST7735SV_TSET_YELLOW\n");
        os_time_dly(12);
        ST7735SV_clear_screen(CYAN);
        os_time_dly(12);
        ST7735SV_clear_screen(YELLOW);
        os_time_dly(12);
        ST7735SV_clear_screen(BLACK);
        os_time_dly(12);
        ST7735SV_clear_screen(RED);
        os_time_dly(12);
    }
#endif
#if 0
    while (1) {
        os_time_dly(100);
        ST7735SV_color_set_screen(0, 0, 30 - 1, 40 - 1, RED);
        os_time_dly(100);
        ST7735SV_color_set_screen(30, 40, 60 - 1, 80 - 1, BLUE);
        os_time_dly(100);
        ST7735SV_color_set_screen(60, 80, 90, 128, YELLOW);
    }
#endif
}

static int ST7735SV_init(void)
{
    lcd_bl_pinstate(lcd_bk_on);
    printf("LCD_ST7735SV init_start\n");
    ST7735SV_reset();
    ST7735SV_init_code();
    ST7735SV_set_direction(ROTATE_DEGREE_0);
#if USE_LCD_TE
    lcd_te_interrupt_init(1);
#endif
    ST7735SV_test();
    printf("LCD_ST7735SV config succes\n");
    return 0;
}

REGISTER_LCD_DEV(LCD_ST7735SV) = {
    .name               = "ST7735SV",
    .lcd_width          = LCD_W,
    .lcd_height         = LCD_H,
    .color_format       = LCD_COLOR_RGB565,
    .column_addr_align  = 1,
    .row_addr_align     = 1,
    .LCD_Init           = ST7735SV_init,
    .SetDrawArea        = ST7735SV_SetRange,
    .LCD_Draw           = ST7735SV_draw,
    .LCD_Draw_1         = ST7735SV_draw_1,
    .LCD_DrawToDev      = ST7735SV_Fill,      // 应用层直接到设备接口层，需要做好缓冲区共用互斥，慎用！
    .LCD_Lvgl_Full      = ST7735SV_lvgl_Fill, // LVGL发送数据接口
    .LCD_ClearScreen    = ST7735SV_clear_screen,
    .LCD_Sleep          = ST7735SV_Sleep,
    .Reset              = ST7735SV_reset,
    .BackLightCtrl      = ST7735SV_led_ctrl,
};

#endif
