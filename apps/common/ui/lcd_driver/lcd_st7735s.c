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

#if TCFG_LCD_ST7735S_ENABLE

void ST7735S_SetRange(u16 xs, u16 xe, u16 ys, u16 ye)
{
    /******UI每次发送数据都会调用开窗告诉屏幕要刷新那个区域***********/
    set_lcd_ui_x_y(xs, xe, ys, ye);
}
void ST7735S_SetRange_1(u16 xs, u16 xe, u16 ys, u16 ye)
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
static void ST7735S_lvgl_Fill(u16 xs, u16 xe, u16 ys, u16 ye, u8 *img)
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
void ST7735S_clear_screen(u32 color)
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

void ST7735S_Fill(u8 *img, u16 len)
{
    lcd_interface_non_block_wait();
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);
}

void ST7735S_SleepInMode(void)
{
    WriteCOM(0x10); //Sleep in
    lcd_delay(120); //Delay 120ms
}

void ST7735S_SleepOutMode(void)
{
    WriteCOM(0x11); //Sleep out
    lcd_delay(120);  //Delay 120ms
}

void ST7789_shown_image(u8 *buff, u16 x_addr, u16 y_addr, u16 width, u16 height)
{
    lcd_interface_non_block_wait();
    ST7735S_SetRange(x_addr, y_addr, width, height);
    WriteDAT_DMA(buff, width * height * 2);
}

static void ST7735S_set_direction(int dir)
{
    WriteCOM(0x36);    //扫描方向控制
    if (dir == ROTATE_DEGREE_0) { //
#if HORIZONTAL_SCREEN
        WriteDAT_8(0xA8);
#else
        WriteDAT_8(0x00);
#endif
    } else if (dir == ROTATE_DEGREE_180) { //翻转180
#if HORIZONTAL_SCREEN
        WriteDAT_8(0xc0);
#else
        WriteDAT_8(0x80);
#endif
    } else if (dir == ROTATE_DEGREE_90) { //翻转90
        WriteDAT_8(0x50);
    }
}

static void ST7735S_draw(u8 *map, u32 size)//获取Ui发送出来的数据
{
    ui_send_data_ready(map, size);
}

static void ST7735S_draw_1(u8 *buf, u32 size, int width, int height)//获取camera发送出来的数据 //数据帧数以camera为基准
{
    camera_send_data_ready(buf, size, width, height);
}

static void ST7735S_reset(void)
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
static void ST7735S_init_code(void)
{
#if 1
    lcd_delay(120);
    WriteCOM(0x11);
    lcd_delay(120);

    WriteCOM(0xb1);
    WriteDAT_8(0x02);
    WriteDAT_8(0x35);
    WriteDAT_8(0x36);

    WriteCOM(0xb2);
    WriteDAT_8(0x02);
    WriteDAT_8(0x35);
    WriteDAT_8(0x36);

    WriteCOM(0xb3);
    WriteDAT_8(0x02);
    WriteDAT_8(0x35);
    WriteDAT_8(0x36);
    WriteDAT_8(0x02);
    WriteDAT_8(0x35);
    WriteDAT_8(0x36);


    WriteCOM(0xb4);
    WriteDAT_8(0x00);

    WriteCOM(0xc0);
    WriteDAT_8(0xa2);
    WriteDAT_8(0x02);
    WriteDAT_8(0x84);


    WriteCOM(0xc1);
    WriteDAT_8(0xc5);


    WriteCOM(0xc2);
    WriteDAT_8(0x0d);
    WriteDAT_8(0x00);


    WriteCOM(0xc3);
    WriteDAT_8(0x8a);
    WriteDAT_8(0x2a);



    WriteCOM(0xc4);
    WriteDAT_8(0x8d);
    WriteDAT_8(0xee);

    WriteCOM(0xc5);
    WriteDAT_8(0x02);//12


    WriteCOM(0xe0);
    WriteDAT_8(0x12);
    WriteDAT_8(0x1c);
    WriteDAT_8(0x10);
    WriteDAT_8(0x18);
    WriteDAT_8(0x33);
    WriteDAT_8(0x2c);
    WriteDAT_8(0x25);
    WriteDAT_8(0x28);
    WriteDAT_8(0x28);
    WriteDAT_8(0x27);
    WriteDAT_8(0x2f);
    WriteDAT_8(0x3c);
    WriteDAT_8(0x00);
    WriteDAT_8(0x03);
    WriteDAT_8(0x03);
    WriteDAT_8(0x10);


    WriteCOM(0xe1);
    WriteDAT_8(0x12);
    WriteDAT_8(0x1c);
    WriteDAT_8(0x10);
    WriteDAT_8(0x18);
    WriteDAT_8(0x2d);
    WriteDAT_8(0x28);
    WriteDAT_8(0x23);
    WriteDAT_8(0x28);
    WriteDAT_8(0x28);
    WriteDAT_8(0x26);
    WriteDAT_8(0x2f);
    WriteDAT_8(0x3b);
    WriteDAT_8(0x00);
    WriteDAT_8(0x03);
    WriteDAT_8(0x03);
    WriteDAT_8(0x10);


    WriteCOM(0x2A);
    WriteDAT_8(0);
    WriteDAT_8(0);
    WriteDAT_8((u8)(LCD_W >> 8));
    WriteDAT_8((u8)LCD_W);
    WriteCOM(0x2B);
    WriteDAT_8(0);
    WriteDAT_8(0);
    WriteDAT_8((u8)(LCD_H >> 8));
    WriteDAT_8((u8)LCD_H);

    WriteCOM(0x3a);
    WriteDAT_8(0x05);

    WriteCOM(0x36);
    WriteDAT_8(0xC8);

    WriteCOM(0x29); //Display on
    WriteCOM(0x2C);
#else
    lcd_delay(120);

    WriteCOM(0x11); //sleep out and booter on
    lcd_delay(120);

    WriteCOM(0xB1); //
    WriteDAT_8(0x01);
    WriteDAT_8(0x2C);
    WriteDAT_8(0x2D);

    WriteCOM(0xB2); //
    WriteDAT_8(0x01);
    WriteDAT_8(0x2C);
    WriteDAT_8(0x2D);

    WriteCOM(0xB3); //
    WriteDAT_8(0x01);
    WriteDAT_8(0x2C);
    WriteDAT_8(0x2D);
    WriteDAT_8(0x01);
    WriteDAT_8(0x2C);
    WriteDAT_8(0x2D);

    WriteCOM(0xB4); //
    WriteDAT_8(0x07);

    //============ power control setting ==========================
    WriteCOM(0xC0); //
    WriteDAT_8(0xA2); //
    WriteDAT_8(0x02); //

    WriteDAT_8(0x84); //

    WriteCOM(0xC1); // set VCL,VGH,VGL,AVDD
    WriteDAT_8(0xC5);

    WriteCOM(0xC2); //
    WriteDAT_8(0x0A); //
    WriteDAT_8(0x00);

    WriteCOM(0xC3); //
    WriteDAT_8(0x8A); //
    WriteDAT_8(0x2A);

    WriteCOM(0xC4); //
    WriteDAT_8(0x8A); //
    WriteDAT_8(0xEE); //

    WriteCOM(0xC5); //
    WriteDAT_8(0x0E); //

    WriteCOM(0x36); //set VCOMH,VCOML voltage
    WriteDAT_8(0xC8); //VCOMH=3.275V
    WriteCOM(0x20);
    WriteCOM(0x21);

    //===== gamma"+"polarity correction characteristic setting ===================
    WriteCOM(0xE0);
    WriteDAT_8(0x02);
    WriteDAT_8(0x1c);
    WriteDAT_8(0x07);
    WriteDAT_8(0x12);
    WriteDAT_8(0x37);
    WriteDAT_8(0x32);
    WriteDAT_8(0x29);
    WriteDAT_8(0x2d);
    WriteDAT_8(0x29);
    WriteDAT_8(0x25);
    WriteDAT_8(0x2b);
    WriteDAT_8(0x39);
    WriteDAT_8(0x00);
    WriteDAT_8(0x01);
    WriteDAT_8(0x03);
    WriteDAT_8(0x10);

    //===== gamma"-"polarity correction characteristic setting ===================
    WriteCOM(0xE1);
    WriteDAT_8(0x03);
    WriteDAT_8(0x1d);
    WriteDAT_8(0x07);
    WriteDAT_8(0x06);
    WriteDAT_8(0x2e);
    WriteDAT_8(0x2c);
    WriteDAT_8(0x29);
    WriteDAT_8(0x2d);
    WriteDAT_8(0x2e);
    WriteDAT_8(0x2e);
    WriteDAT_8(0x37);
    WriteDAT_8(0x3f);
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);
    WriteDAT_8(0x02);
    WriteDAT_8(0x10);

    WriteCOM(0x20);     //
    WriteCOM(0x36);     // memory access control
    WriteDAT_8(0xcc);
    WriteCOM(0x3A);     //E0H or E1 Register enable or disabl
    WriteDAT_8(0x05);   //E0H or E1 Register enable
    WriteCOM(0x29);     //display on
#endif
}

static void ST7735S_led_ctrl(u8 status)
{
    //背光控制以及放在//lcd_te_driver.c 优化开机显示
    lcd_bl_pinstate(status);
}

void ST7735S_test(void)
{
    ST7735S_clear_screen(BLACK);
    lcd_delay(50);//每个屏幕可能会在刷新清屏了，但是屏幕芯片缓存还没刷到屏幕，需要加延时
    lcd_bl_pinstate(1);
//    while (1) {
//        os_time_dly(10);
//        ST7735S_clear_screen(WHITE);
//        printf("LCD_ST7735S_TSET_BLUE\n");
//        os_time_dly(100);
//        ST7735S_clear_screen(DGREEN);
//        printf("LCD_ST7735S_TSET_GRED\n");
//        os_time_dly(100);
//        ST7735S_clear_screen(MAROON);
//        printf("LCD_ST7735S_TSET_BRRED\n");
//        os_time_dly(100);
//        ST7735S_clear_screen(BLUE);
//        printf("LCD_ST7735S_TSET_YELLOW\n");
//        os_time_dly(100);
//        ST7735S_clear_screen(CYAN);
//        os_time_dly(100);
//        ST7735S_clear_screen(YELLOW);
//        os_time_dly(100);
//        ST7735S_clear_screen(BLACK);
//        os_time_dly(100);
//        ST7735S_clear_screen(RED);
//        os_time_dly(100);
//    }
}

static int ST7735S_init(void)
{
    printf("LCD_ST7735S init_start\n");
    ST7735S_reset();
    ST7735S_init_code();
    ST7735S_set_direction(ROTATE_DEGREE_0);
#if USE_LCD_TE
    lcd_te_interrupt_init(1);
#endif
    ST7735S_test();
    printf("LCD_ST7735S config succes\n");
    return 0;
}


REGISTER_LCD_DEV(LCD_ST7735S) = {
    .name              = "ST7735S",
    .lcd_width         = LCD_W,
    .lcd_height        = LCD_H,
    .color_format      = LCD_COLOR_RGB565,
    .column_addr_align = 1,
    .row_addr_align    = 1,
    .LCD_Init          = ST7735S_init,
    .SetDrawArea       = ST7735S_SetRange,
    .LCD_Draw          = ST7735S_draw,
    .LCD_Draw_1        = ST7735S_draw_1,
    .LCD_DrawToDev     = ST7735S_Fill,//应用层直接到设备接口层，需要做好缓冲区共用互斥，慎用！
    .LCD_Lvgl_Full     = ST7735S_lvgl_Fill,//LVGL发送数据接口
    .LCD_ClearScreen   = ST7735S_clear_screen,
    .Reset             = ST7735S_reset,
    .BackLightCtrl     = ST7735S_led_ctrl,
};

#endif


