#include "system/includes.h"
#include "typedef.h"
#include "asm/pap.h"
#include "lcd_drive.h"
#include "lcd_config.h"

/*  ST7789P3驱动说明 该驱动测试时使用的 wl80 79系列
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

#if TCFG_LCD_ST7789P3_ENABLE

#define LCD_W     240
#define LCD_H     320

void ST7789P3_SetRange(u16 xs, u16 xe, u16 ys, u16 ye)
{
    /******UI每次发送数据都会调用开窗告诉屏幕要刷新那个区域***********/
    set_lcd_ui_x_y(xs, xe, ys, ye);
}
void ST7789P3_SetRange_1(u16 xs, u16 xe, u16 ys, u16 ye)
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

void ST7789P3_clear_screen(u32 color)
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

void ST7789P3_Fill(u8 *img, u16 len)
{
    lcd_interface_non_block_wait();
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);
}

void ST7789P3_SleepInMode(void)
{
    WriteCOM(0x10); //Sleep in
    lcd_delay(120); //Delay 120ms
}

void ST7789P3_SleepOutMode(void)
{
    WriteCOM(0x11); //Sleep out
    lcd_delay(120);  //Delay 120ms
}

void st7789_shown_image(u8 *buff, u16 x_addr, u16 y_addr, u16 width, u16 height)
{
    lcd_interface_non_block_wait();
    ST7789P3_SetRange(x_addr, y_addr, width, height);
    WriteDAT_DMA(buff, width * height * 2);
}

static void ST7789P3_set_direction(int dir)
{

}

static void ST7789P3_draw(u8 *map, u32 size)//获取Ui发送出来的数据
{
    ui_send_data_ready(map, size);
}

static void ST7789P3_draw_1(u8 *buf, u32 size, int width, int height)//获取camera发送出来的数据 //数据帧数以camera为基准
{
    camera_send_data_ready(buf, size, width, height);
}

static void ST7789P3_reset(void)
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
static void ST7789P3_init_code(void)
{
    os_time_dly(12);
    WriteCOM(0x11);
    os_time_dly(12);

    WriteCOM(0x36);
    WriteDAT_8(0x00);

    WriteCOM(0x3A);
    WriteDAT_8(0x05);

    WriteCOM(0xE7); //2 data lane
    WriteDAT_8(0x10);

    WriteCOM(0xB2);
    WriteDAT_8(0x1E);
    WriteDAT_8(0x1F);
    WriteDAT_8(0x00);
    WriteDAT_8(0x33);
    WriteDAT_8(0x33);

    WriteCOM(0xB7);
    WriteDAT_8(0x66);

    WriteCOM(0xBB);    //VCOM
    WriteDAT_8(0x12);//2E

    WriteCOM(0xC0);
    WriteDAT_8(0x2C);

    WriteCOM(0xC2);
    WriteDAT_8(0x01);

    WriteCOM(0xC3);
    WriteDAT_8(0x11);   //19  对比度

    WriteCOM(0xC4);
    WriteDAT_8(0x20);   //VDV, 0x20:0v

    WriteCOM(0xC6);
    WriteDAT_8(0x13);   //0x13:60Hz

    WriteCOM(0xD0);
    WriteDAT_8(0xA7);

    os_time_dly(1);                //ms

    WriteCOM(0xD0);
    WriteDAT_8(0xA4);
    WriteDAT_8(0xA1);

    WriteCOM(0xD6);
    WriteDAT_8(0xA1);   //sleep in?,gate???GND

    WriteCOM(0xE0);
    WriteDAT_8(0xF0);
    WriteDAT_8(0x08);
    WriteDAT_8(0x0C);
    WriteDAT_8(0x0B);
    WriteDAT_8(0x0A);
    WriteDAT_8(0x26);
    WriteDAT_8(0x31);
    WriteDAT_8(0x43);
    WriteDAT_8(0x48);
    WriteDAT_8(0x39);
    WriteDAT_8(0x15);
    WriteDAT_8(0x15);
    WriteDAT_8(0x31);
    WriteDAT_8(0x37);

    WriteCOM(0xE1);
    WriteDAT_8(0xF0);
    WriteDAT_8(0x04);
    WriteDAT_8(0x0A);
    WriteDAT_8(0x08);
    WriteDAT_8(0x08);
    WriteDAT_8(0x23);
    WriteDAT_8(0x31);
    WriteDAT_8(0x32);
    WriteDAT_8(0x47);
    WriteDAT_8(0x3B);
    WriteDAT_8(0x16);
    WriteDAT_8(0x16);
    WriteDAT_8(0x2E);
    WriteDAT_8(0x35);

    WriteCOM(0xE4);
    WriteDAT_8(0x1D);
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);   //?gate?????,bit4(TMG)??0


    WriteCOM(0x35);
    WriteDAT_8(0x00);

    WriteCOM(0x44);
    WriteDAT_8(0x00);


    WriteCOM(0x21);

    WriteCOM(0x29);

    WriteCOM(0x2A);     //Column Address Set
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);   //0
    WriteDAT_8(0x00);
    WriteDAT_8(0xEF);   //239

    WriteCOM(0x2B);     //Row Address Set
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);
    WriteDAT_8(0xEF);   //239

    WriteCOM(0x2C);

}

static void ST7789P3_led_ctrl(u8 status)
{
    //背光控制以及放在//lcd_te_driver.c 优化开机显示
    lcd_bl_pinstate(status);
}

void ST7789P3_test(void)
{
    ST7789P3_clear_screen(BLACK);
    lcd_delay(50);//每个屏幕可能会在刷新清屏了，但是屏幕芯片缓存还没刷到屏幕，需要加延时
    lcd_bl_pinstate(1);
//    while (1) {
//        os_time_dly(10);
//        ST7789P3_clear_screen(WHITE);
//        printf("LCD_ST7789P3_TSET_BLUE\n");
//        os_time_dly(100);
//        ST7789P3_clear_screen(DGREEN);
//        printf("LCD_ST7789P3_TSET_GRED\n");
//        os_time_dly(100);
//        ST7789P3_clear_screen(MAROON);
//        printf("LCD_ST7789P3_TSET_BRRED\n");
//        os_time_dly(100);
//        ST7789P3_clear_screen(BLUE);
//        printf("LCD_ST7789P3_TSET_YELLOW\n");
//        os_time_dly(100);
//        ST7789P3_clear_screen(CYAN);
//        os_time_dly(100);
//        ST7789P3_clear_screen(YELLOW);
//        os_time_dly(100);
//        ST7789P3_clear_screen(BLACK);
//        os_time_dly(100);
//        ST7789P3_clear_screen(RED);
//        os_time_dly(100);
//    }
}

static int ST7789P3_init(void)
{
    printf("LCD_ST7789P3 init_start\n");
    ST7789P3_reset();
    ST7789P3_init_code();
//    ST7789P3_set_direction(ROTATE_DEGREE_90);
#if USE_LCD_TE
    lcd_te_interrupt_init(1);
#endif
    ST7789P3_test();
    printf("LCD_ST7789P3 config succes\n");
    return 0;
}


REGISTER_LCD_DEV(LCD_ST7789P3) = {
    .name              = "ST7789P3",
    .lcd_width         = LCD_W,
    .lcd_height        = LCD_H,
    .color_format      = LCD_COLOR_RGB565,
    .column_addr_align = 1,
    .row_addr_align    = 1,
    .LCD_Init          = ST7789P3_init,
    .SetDrawArea       = ST7789P3_SetRange,
    .LCD_Draw          = ST7789P3_draw,
    .LCD_Draw_1        = ST7789P3_draw_1,
    .LCD_DrawToDev     = ST7789P3_Fill,//应用层直接到设备接口层，需要做好缓冲区共用互斥，慎用！
    .LCD_ClearScreen   = ST7789P3_clear_screen,
    .Reset             = ST7789P3_reset,
    .BackLightCtrl     = ST7789P3_led_ctrl,
};

#endif


