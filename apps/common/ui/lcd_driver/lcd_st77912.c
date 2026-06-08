

#include "system/includes.h"
#include "typedef.h"
#include "asm/pap.h"
#include "lcd_drive.h"
#include "lcd_config.h"

/*  ST77912驱动说明 该驱动测试时使用的 wl80 79系列
 *  由于该IC推屏能力不够强 推屏的帧数较低 大概在25帧左右
 *  在推屏过程中需要使用TE屏幕帧中断 不然会有条纹
 *  由于ST77912横屏配置无法调出没有条纹的配置
 *  所有只能使用竖屏加RGB旋转来实现UI横屏显示
 */

/* //pap的这个三个配置如下 在板级文件中进行修改
    .timing_setup   = 0,                    //具体看pap.h
    .timing_hold    = 0,                    //具体看pap.h
    .timing_width   = 1,                    //具体看pap.h
*/

#if TCFG_LCD_ST77912_ENABLE


#define READ_ID     0x04
#define REGFLAG_DELAY 0x45

void ST77912_SetRange(u16 xs, u16 xe, u16 ys, u16 ye)
{
    /******UI每次发送数据都会调用开窗告诉屏幕要刷新那个区域***********/
    set_lcd_ui_x_y(xs, xe, ys, ye);
}
void ST77912_SetRange_1(u16 xs, u16 xe, u16 ys, u16 ye)
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

static void ST77912_lvgl_Fill(u16 xs, u16 xe, u16 ys, u16 ye, u8 *img)
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

void ST77912_clear_screen(u32 color)
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

void ST77912_Fill(u8 *img, u16 len)
{
    lcd_interface_non_block_wait();
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);
}

void ST77912_SleepInMode(void)
{
    WriteCOM(0x10); //Sleep in
    lcd_delay(120); //Delay 120ms
}

void ST77912_SleepOutMode(void)
{
    WriteCOM(0x11); //Sleep out
    lcd_delay(120);  //Delay 120ms
}

void ST77912_Shown_Image(u8 *buff, u16 x_addr, u16 y_addr, u16 width, u16 height)
{
    lcd_interface_non_block_wait();
    ST77912_SetRange(x_addr, y_addr, width, height);
    WriteDAT_DMA(buff, width * height * 2);
}
static void ST77912_SetMirror1(char enable)
{
#ifdef CONFIG_UI_MIRROR_EYE
    WriteCOM(0x36);//扫描方向控制
    if (enable) {
        WriteDAT_8(0x40);
    } else {
        WriteDAT_8(0x00);
    }
#endif
}

static void ST77912_set_direction(int dir)
{
//    WriteCOM(0x36);    //扫描方向控制
//    if (dir == ROTATE_DEGREE_0) { //
//#if HORIZONTAL_SCREEN
//        WriteDAT_8(0xA0);
//#else
//        WriteDAT_8(0x00);
//#endif
//        ST77912_SetRange(0, LCD_W - 1, 0, LCD_H - 1);
//    } else if (dir == ROTATE_DEGREE_180) { //翻转180
//#if HORIZONTAL_SCREEN
//        WriteDAT_8(0xc0);
//#else
//        WriteDAT_8(0x80);
//#endif
//    } else if (dir == ROTATE_DEGREE_90) { //翻转90
//        WriteDAT_8(0x60);
//        ST77912_SetRange(0, LCD_W - 1, 0, LCD_H - 1);
//  }
}

static void ST77912_draw(u8 *map, u32 size)//获取Ui发送出来的数据
{
    ui_send_data_ready(map, size);
}

static void ST77912_draw_1(u8 *buf, u32 size, int width, int height)//获取camera发送出来的数据 //数据帧数以camera为基准
{
    camera_send_data_ready(buf, size, width, height);
}

static void ST77912_reset(void)
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
static void ST77912_init_code(void *code, u8 cnt)
{
    //CMD2);
    WriteCOM(0xF0);
    WriteDAT_8(0x01);
    WriteCOM(0xF1);
    WriteDAT_8(0x01);
    WriteCOM(0x7A);
    WriteDAT_8(0x83);
    WriteCOM(0xB0);
    WriteDAT_8(0x5E);
    WriteCOM(0xB1);
    WriteDAT_8(0x55);
    WriteCOM(0xB2);
    WriteDAT_8(0x24);
    WriteCOM(0xB4);
    WriteDAT_8(0xA7);
    WriteCOM(0xB5);
    WriteDAT_8(0x54);
    WriteCOM(0xB6);
    WriteDAT_8(0x8B);
    WriteCOM(0xB7);
    WriteDAT_8(0x50);
    WriteCOM(0xBA);
    WriteDAT_8(0x00);
    WriteCOM(0xBB);
    WriteDAT_8(0x08);
    WriteCOM(0xBC);
    WriteDAT_8(0x08);
    WriteCOM(0xBD);
    WriteDAT_8(0x00);
    WriteCOM(0xC0);
    WriteDAT_8(0x80);
    WriteCOM(0xC1);
    WriteDAT_8(0x08);
    WriteCOM(0xC2);
    WriteDAT_8(0x54);
    WriteCOM(0xC3);
    WriteDAT_8(0x80);
    WriteCOM(0xC4);
    WriteDAT_8(0x08);
    WriteCOM(0xC5);
    WriteDAT_8(0x54);
    WriteCOM(0xC6);
    WriteDAT_8(0xA9);
    WriteCOM(0xC7);
    WriteDAT_8(0x41);
    WriteCOM(0xC8);
    WriteDAT_8(0x51);
    WriteCOM(0xC9);
    WriteDAT_8(0xA9);
    WriteCOM(0xCA);
    WriteDAT_8(0x41);
    WriteCOM(0xCB);
    WriteDAT_8(0x51);
    WriteCOM(0xD0);
    WriteDAT_8(0x80);
    WriteCOM(0xD1);
    WriteDAT_8(0xF0);
    WriteCOM(0xD2);
    WriteDAT_8(0xF0);
    WriteCOM(0xF5);
    WriteDAT_8(0x00);
    WriteDAT_8(0xA5);
    WriteCOM(0xDD);
    WriteDAT_8(0x36);
    WriteCOM(0xDE);
    WriteDAT_8(0x36);
    WriteCOM(0xF0);
    WriteDAT_8(0x02);
    WriteCOM(0xF1);
    WriteDAT_8(0x01);
    WriteCOM(0xE0);
    WriteDAT_8(0xF0);
    WriteDAT_8(0x16);
    WriteDAT_8(0x1C);
    WriteDAT_8(0x0A);
    WriteDAT_8(0x0A);
    WriteDAT_8(0x06);
    WriteDAT_8(0x3E);
    WriteDAT_8(0x33);
    WriteDAT_8(0x53);
    WriteDAT_8(0x07);
    WriteDAT_8(0x14);
    WriteDAT_8(0x13);
    WriteDAT_8(0x31);
    WriteDAT_8(0x35);
    WriteCOM(0xE1);
    WriteDAT_8(0xF0);
    WriteDAT_8(0x16);
    WriteDAT_8(0x1C);
    WriteDAT_8(0x0A);
    WriteDAT_8(0x0A);
    WriteDAT_8(0x06);
    WriteDAT_8(0x3E);
    WriteDAT_8(0x33);
    WriteDAT_8(0x53);
    WriteDAT_8(0x07);
    WriteDAT_8(0x14);
    WriteDAT_8(0x13);
    WriteDAT_8(0x31);
    WriteDAT_8(0x35);
    //GIP);
    WriteCOM(0xF0);
    WriteDAT_8(0x10);
    WriteCOM(0xF3);
    WriteDAT_8(0x10);
    WriteCOM(0xE0);
    WriteDAT_8(0x0B);
    WriteCOM(0xE1);
    WriteDAT_8(0x00);
    WriteCOM(0xE2);
    WriteDAT_8(0x00);
    WriteCOM(0xE3);
    WriteDAT_8(0x00);
    WriteCOM(0xE4);
    WriteDAT_8(0xE0);
    WriteCOM(0xE5);
    WriteDAT_8(0x06);
    WriteCOM(0xE6);
    WriteDAT_8(0x21);
    WriteCOM(0xE7);
    WriteDAT_8(0x80);
    WriteCOM(0xE8);
    WriteDAT_8(0x0A);
    WriteCOM(0xE9);
    WriteDAT_8(0x00);
    WriteCOM(0xEA);
    WriteDAT_8(0x04);
    WriteCOM(0xEB);
    WriteDAT_8(0x00);
    WriteCOM(0xEC);
    WriteDAT_8(0x00);
    WriteCOM(0xED);
    WriteDAT_8(0x24);
    WriteCOM(0xEE);
    WriteDAT_8(0x00);
    WriteCOM(0xEF);
    WriteDAT_8(0x00);
    WriteCOM(0xF8);
    WriteDAT_8(0xFF);
    WriteCOM(0xF9);
    WriteDAT_8(0x00);
    WriteCOM(0xFA);
    WriteDAT_8(0x00);
    WriteCOM(0xFB);
    WriteDAT_8(0x30);
    WriteCOM(0xFC);
    WriteDAT_8(0x00);
    WriteCOM(0xFD);
    WriteDAT_8(0x00);
    WriteCOM(0xFE);
    WriteDAT_8(0x00);
    WriteCOM(0xFF);
    WriteDAT_8(0x00);
    WriteCOM(0x60);
    WriteDAT_8(0x40);
    WriteCOM(0x61);
    WriteDAT_8(0x08);
    WriteCOM(0x62);
    WriteDAT_8(0x00);
    WriteCOM(0x63);
    WriteDAT_8(0x41);
    WriteCOM(0x64);
    WriteDAT_8(0xED);
    WriteCOM(0x65);
    WriteDAT_8(0x00);
    WriteCOM(0x66);
    WriteDAT_8(0x40);
    WriteCOM(0x67);
    WriteDAT_8(0x00);
    WriteCOM(0x68);
    WriteDAT_8(0x00);
    WriteCOM(0x69);
    WriteDAT_8(0x40);
    WriteCOM(0x6A);
    WriteDAT_8(0x00);
    WriteCOM(0x6B);
    WriteDAT_8(0x00);
    WriteCOM(0x70);
    WriteDAT_8(0x40);
    WriteCOM(0x71);
    WriteDAT_8(0x07);
    WriteCOM(0x72);
    WriteDAT_8(0x00);
    WriteCOM(0x73);
    WriteDAT_8(0x41);
    WriteCOM(0x74);
    WriteDAT_8(0xEC);
    WriteCOM(0x75);
    WriteDAT_8(0x00);
    WriteCOM(0x76);
    WriteDAT_8(0x40);
    WriteCOM(0x77);
    WriteDAT_8(0x00);
    WriteCOM(0x78);
    WriteDAT_8(0x00);
    WriteCOM(0x79);
    WriteDAT_8(0x40);
    WriteCOM(0x7A);
    WriteDAT_8(0x00);
    WriteCOM(0x7B);
    WriteDAT_8(0x00);
    WriteCOM(0x80);
    WriteDAT_8(0x48);
    WriteCOM(0x81);
    WriteDAT_8(0x00);
    WriteCOM(0x82);
    WriteDAT_8(0x0A);
    WriteCOM(0x83);
    WriteDAT_8(0x01);
    WriteCOM(0x84);
    WriteDAT_8(0xEA);
    WriteCOM(0x85);
    WriteDAT_8(0x00);
    WriteCOM(0x86);
    WriteDAT_8(0x00);
    WriteCOM(0x87);
    WriteDAT_8(0x00);
    WriteCOM(0x88);
    WriteDAT_8(0x48);
    WriteCOM(0x89);
    WriteDAT_8(0x00);
    WriteCOM(0x8A);
    WriteDAT_8(0x0C);
    WriteCOM(0x8B);
    WriteDAT_8(0x01);
    WriteCOM(0x8C);
    WriteDAT_8(0xEC);
    WriteCOM(0x8D);
    WriteDAT_8(0x00);
    WriteCOM(0x8E);
    WriteDAT_8(0x00);
    WriteCOM(0x8F);
    WriteDAT_8(0x00);
    WriteCOM(0x90);
    WriteDAT_8(0x48);
    WriteCOM(0x91);
    WriteDAT_8(0x00);
    WriteCOM(0x92);
    WriteDAT_8(0x0E);
    WriteCOM(0x93);
    WriteDAT_8(0x01);
    WriteCOM(0x94);
    WriteDAT_8(0xEE);
    WriteCOM(0x95);
    WriteDAT_8(0x00);
    WriteCOM(0x96);
    WriteDAT_8(0x00);
    WriteCOM(0x97);
    WriteDAT_8(0x00);
    WriteCOM(0x98);
    WriteDAT_8(0x48);
    WriteCOM(0x99);
    WriteDAT_8(0x00);
    WriteCOM(0x9A);
    WriteDAT_8(0x10);
    WriteCOM(0x9B);
    WriteDAT_8(0x01);
    WriteCOM(0x9C);
    WriteDAT_8(0xF0);
    WriteCOM(0x9D);
    WriteDAT_8(0x00);
    WriteCOM(0x9E);
    WriteDAT_8(0x00);
    WriteCOM(0x9F);
    WriteDAT_8(0x00);
    WriteCOM(0xA0);
    WriteDAT_8(0x48);
    WriteCOM(0xA1);
    WriteDAT_8(0x00);
    WriteCOM(0xA2);
    WriteDAT_8(0x09);
    WriteCOM(0xA3);
    WriteDAT_8(0x01);
    WriteCOM(0xA4);
    WriteDAT_8(0xE9);
    WriteCOM(0xA5);
    WriteDAT_8(0x00);
    WriteCOM(0xA6);
    WriteDAT_8(0x00);
    WriteCOM(0xA7);
    WriteDAT_8(0x00);
    WriteCOM(0xA8);
    WriteDAT_8(0x48);
    WriteCOM(0xA9);
    WriteDAT_8(0x00);
    WriteCOM(0xAA);
    WriteDAT_8(0x0B);
    WriteCOM(0xAB);
    WriteDAT_8(0x01);
    WriteCOM(0xAC);
    WriteDAT_8(0xEB);
    WriteCOM(0xAD);
    WriteDAT_8(0x00);
    WriteCOM(0xAE);
    WriteDAT_8(0x00);
    WriteCOM(0xAF);
    WriteDAT_8(0x00);
    WriteCOM(0xB0);
    WriteDAT_8(0x48);
    WriteCOM(0xB1);
    WriteDAT_8(0x00);
    WriteCOM(0xB2);
    WriteDAT_8(0x0D);
    WriteCOM(0xB3);
    WriteDAT_8(0x01);
    WriteCOM(0xB4);
    WriteDAT_8(0xED);
    WriteCOM(0xB5);
    WriteDAT_8(0x00);
    WriteCOM(0xB6);
    WriteDAT_8(0x00);
    WriteCOM(0xB7);
    WriteDAT_8(0x00);
    WriteCOM(0xB8);
    WriteDAT_8(0x48);
    WriteCOM(0xB9);
    WriteDAT_8(0x00);
    WriteCOM(0xBA);
    WriteDAT_8(0x0F);
    WriteCOM(0xBB);
    WriteDAT_8(0x01);
    WriteCOM(0xBC);
    WriteDAT_8(0xEF);
    WriteCOM(0xBD);
    WriteDAT_8(0x00);
    WriteCOM(0xBE);
    WriteDAT_8(0x00);
    WriteCOM(0xBF);
    WriteDAT_8(0x00);
    WriteCOM(0xC0);
    WriteDAT_8(0x88);
    WriteCOM(0xC1);
    WriteDAT_8(0x99);
    WriteCOM(0xC2);
    WriteDAT_8(0x01);
    WriteCOM(0xC3);
    WriteDAT_8(0xAA);
    WriteCOM(0xC4);
    WriteDAT_8(0xBB);
    WriteCOM(0xC5);
    WriteDAT_8(0x74);
    WriteCOM(0xC6);
    WriteDAT_8(0x65);
    WriteCOM(0xC7);
    WriteDAT_8(0x56);
    WriteCOM(0xC8);
    WriteDAT_8(0x47);
    WriteCOM(0xC9);
    WriteDAT_8(0x10);
    WriteCOM(0xD0);
    WriteDAT_8(0x88);
    WriteCOM(0xD1);
    WriteDAT_8(0x99);
    WriteCOM(0xD2);
    WriteDAT_8(0x01);
    WriteCOM(0xD3);
    WriteDAT_8(0xAA);
    WriteCOM(0xD4);
    WriteDAT_8(0xBB);
    WriteCOM(0xD5);
    WriteDAT_8(0x74);
    WriteCOM(0xD6);
    WriteDAT_8(0x65);
    WriteCOM(0xD7);
    WriteDAT_8(0x56);
    WriteCOM(0xD8);
    WriteDAT_8(0x47);
    WriteCOM(0xD9);
    WriteDAT_8(0x10);
    //Test);
    WriteCOM(0xF0);
    WriteDAT_8(0x08);
    WriteCOM(0xF2);
    WriteDAT_8(0x08);
    WriteCOM(0x71);
    WriteDAT_8(0x03);
    WriteCOM(0x73);
    WriteDAT_8(0x30);
    WriteCOM(0x76);
    WriteDAT_8(0x00);
    WriteCOM(0x78);
    WriteDAT_8(0x33);
    WriteCOM(0x79);
    WriteDAT_8(0x01);
    WriteCOM(0x7B);
    WriteDAT_8(0xFA);
    WriteCOM(0x7E);
    WriteDAT_8(0x16);
    WriteCOM(0x86);
    WriteDAT_8(0x55);
    WriteCOM(0x89);
    WriteDAT_8(0x61);
    WriteCOM(0x8A);
    WriteDAT_8(0x00);
    WriteCOM(0xF0);
    WriteDAT_8(0x01);
    WriteCOM(0xF1);
    WriteDAT_8(0x01);
    WriteCOM(0xA0);
    WriteDAT_8(0x0B);
    WriteCOM(0xA3);
    WriteDAT_8(0x2A);
    WriteCOM(0xA5);
    WriteDAT_8(0xC3);
    lcd_delay(1);
    WriteCOM(0xA3);
    WriteDAT_8(0x2B);
    WriteCOM(0xA5);
    WriteDAT_8(0xC3);
    lcd_delay(1);
    WriteCOM(0xA3);
    WriteDAT_8(0x2C);
    WriteCOM(0xA5);
    WriteDAT_8(0xC3);
    lcd_delay(1);
    WriteCOM(0xA3);
    WriteDAT_8(0x2D);
    WriteCOM(0xA5);
    WriteDAT_8(0xC3);
    lcd_delay(1);
    WriteCOM(0xA3);
    WriteDAT_8(0x2E);
    WriteCOM(0xA5);
    WriteDAT_8(0xC3);
    lcd_delay(1);
    WriteCOM(0xA3);
    WriteDAT_8(0x2F);
    WriteCOM(0xA5);
    WriteDAT_8(0xC3);
    lcd_delay(1);
    WriteCOM(0xA3);
    WriteDAT_8(0x30);
    WriteCOM(0xA5);
    WriteDAT_8(0xC3);
    lcd_delay(1);
    WriteCOM(0xA3);
    WriteDAT_8(0x31);
    WriteCOM(0xA5);
    WriteDAT_8(0xC3);
    lcd_delay(1);
    WriteCOM(0xA3);
    WriteDAT_8(0x32);
    WriteCOM(0xA5);
    WriteDAT_8(0xC3);
    lcd_delay(1);
    WriteCOM(0xA3);
    WriteDAT_8(0x33);
    WriteCOM(0xA5);
    WriteDAT_8(0xC3);
    lcd_delay(1);
    WriteCOM(0xA0);
    WriteDAT_8(0x09);
    WriteCOM(0xF0);
    WriteDAT_8(0x00);
    WriteCOM(0xF1);
    WriteDAT_8(0x10);
    WriteCOM(0xF2);
    WriteDAT_8(0x84);
    WriteCOM(0xF3);
    WriteDAT_8(0x01);
    WriteCOM(0x21);
    WriteCOM(0x3A);
    WriteDAT_8(0x05);
    WriteCOM(0x35);
    WriteDAT_8(0x00);
    WriteCOM(0x11);
    lcd_delay(12);
    WriteCOM(0x29);
    lcd_delay(10);

}

static void ST77912_led_ctrl(u8 status)
{
    //背光控制以及放在//lcd_te_driver.c 优化开机显示
    /*lcd_bl_pinstate(status);*/
}

void ST77912_test(void)
{
    ST77912_clear_screen(BLACK);
    lcd_delay(50);//每个屏幕可能会在刷新清屏了，但是屏幕芯片缓存还没刷到屏幕，需要加延时
    lcd_bl_pinstate(1);
//    while (1) {
//        os_time_dly(100);
//        ST77912_clear_screen(BLUE);
//        printf("LCD_ST77912_TSET_BLUE\n");
//        os_time_dly(100);
//        ST77912_clear_screen(DCYAN);
//        printf("LCD_ST77912_TSET_GRED\n");
//        os_time_dly(100);
//        ST77912_clear_screen(DGREEN);
//        printf("LCD_ST77912_TSET_BRRED\n");
//        os_time_dly(100);
//        ST77912_clear_screen(YELLOW);
//        printf("LCD_ST77912_TSET_YELLOW\n");
//    }
}

static int ST77912_init(void)
{
    printf("LCD_ST77912 init_start\n");
    ST77912_reset();
    ST77912_init_code(NULL, 0);
    ST77912_set_direction(ROTATE_DEGREE_90);
#if USE_LCD_TE
    lcd_te_interrupt_init(1);
#endif
    ST77912_test();
    printf("LCD_ST77912 config succes\n");
    return 0;
}


REGISTER_LCD_DEV(LCD_ST77912) = {
    .name              = "ST77912",
    .lcd_width         = LCD_W,
    .lcd_height        = LCD_H,
    .color_format      = LCD_COLOR_RGB565,
    .column_addr_align = 1,
    .row_addr_align    = 1,
    .LCD_Init          = ST77912_init,
    .SetDrawArea       = ST77912_SetRange,
    .LCD_Mirror1       = ST77912_SetMirror1,
    .LCD_Draw          = ST77912_draw,
    .LCD_Draw_1        = ST77912_draw_1,
    .LCD_DrawToDev     = ST77912_Fill,//应用层直接到设备接口层，需要做好缓冲区共用互斥，慎用！
    .LCD_Lvgl_Full     = ST77912_lvgl_Fill,//LVGL发送数据接口
    .LCD_ClearScreen   = ST77912_clear_screen,
    .Reset             = ST77912_reset,
    .BackLightCtrl     = ST77912_led_ctrl,
};

#endif


