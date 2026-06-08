#include "system/includes.h"
#include "typedef.h"
#include "asm/pap.h"
#include "lcd_drive.h"
#include "lcd_config.h"

/*  GC9D01驱动说明 该驱动测试时使用的 wl80 79系列
 *  由于该IC推屏能力不够强 推屏的帧数较低 大概在25帧左右
 *  在推屏过程中需要使用TE屏幕帧中断 不然会有条纹
 *  由于GC9D01横屏配置无法调出没有条纹的配置
 *  所有只能使用竖屏加RGB旋转来实现UI横屏显示
 */

/* //pap的这个三个配置如下 在板级文件中进行修改
    .timing_setup   = 0,                    //具体看pap.h
    .timing_hold    = 0,                    //具体看pap.h
    .timing_width   = 1,                    //具体看pap.h
*/

#if TCFG_LCD_GC9D01_ENABLE


#define READ_ID     0x04
#define REGFLAG_DELAY 0x45

void GC9D01_SetRange(u16 xs, u16 xe, u16 ys, u16 ye)
{
    /******UI每次发送数据都会调用开窗告诉屏幕要刷新那个区域***********/
    set_lcd_ui_x_y(xs, xe, ys, ye);
}
void GC9D01_SetRange_1(u16 xs, u16 xe, u16 ys, u16 ye)
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

static void GC9D01_lvgl_Fill(u16 xs, u16 xe, u16 ys, u16 ye, u8 *img)
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

void GC9D01_clear_screen(u32 color)
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

void GC9D01_Fill(u8 *img, u16 len)
{
    lcd_interface_non_block_wait();
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);
}

void GC9D01_SleepInMode(void)
{
    WriteCOM(0x10); //Sleep in
    lcd_delay(120); //Delay 120ms
}

void GC9D01_SleepOutMode(void)
{
    WriteCOM(0x11); //Sleep out
    lcd_delay(120);  //Delay 120ms
}

void GC9D01_Shown_Image(u8 *buff, u16 x_addr, u16 y_addr, u16 width, u16 height)
{
    lcd_interface_non_block_wait();
    GC9D01_SetRange(x_addr, y_addr, width, height);
    WriteDAT_DMA(buff, width * height * 2);
}
static void GC9D01_SetMirror1(char enable)
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

static void GC9D01_set_direction(int dir)
{
//    WriteCOM(0x36);    //扫描方向控制
//    if (dir == ROTATE_DEGREE_0) { //
//#if HORIZONTAL_SCREEN
//        WriteDAT_8(0xA0);
//#else
//        WriteDAT_8(0x00);
//#endif
//        GC9D01_SetRange(0, LCD_W - 1, 0, LCD_H - 1);
//    } else if (dir == ROTATE_DEGREE_180) { //翻转180
//#if HORIZONTAL_SCREEN
//        WriteDAT_8(0xc0);
//#else
//        WriteDAT_8(0x80);
//#endif
//    } else if (dir == ROTATE_DEGREE_90) { //翻转90
//        WriteDAT_8(0x60);
//        GC9D01_SetRange(0, LCD_W - 1, 0, LCD_H - 1);
//  }
}

static void GC9D01_draw(u8 *map, u32 size)//获取Ui发送出来的数据
{
    ui_send_data_ready(map, size);
}

static void GC9D01_draw_1(u8 *buf, u32 size, int width, int height)//获取camera发送出来的数据 //数据帧数以camera为基准
{
    camera_send_data_ready(buf, size, width, height);
}

static void GC9D01_reset(void)
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
static void GC9D01_init_code(void *code, u8 cnt)
{
    //CMD2);
    lcd_delay(120);
    WriteCOM(0xFE);
    WriteCOM(0xEF);
    WriteCOM(0x80);
    WriteDAT_8(0xFF);
    WriteCOM(0x81);
    WriteDAT_8(0xFF);
    WriteCOM(0x82);
    WriteDAT_8(0xFF);
    WriteCOM(0x83);
    WriteDAT_8(0xFF);
    WriteCOM(0x84);
    WriteDAT_8(0xFF);
    WriteCOM(0x85);
    WriteDAT_8(0xFF);
    WriteCOM(0x86);
    WriteDAT_8(0xFF);
    WriteCOM(0x87);
    WriteDAT_8(0xFF);
    WriteCOM(0x88);
    WriteDAT_8(0xFF);
    WriteCOM(0x89);
    WriteDAT_8(0xFF);
    WriteCOM(0x8A);
    WriteDAT_8(0xFF);
    WriteCOM(0x8B);
    WriteDAT_8(0xFF);
    WriteCOM(0x8C);
    WriteDAT_8(0xFF);
    WriteCOM(0x8D);
    WriteDAT_8(0xFF);
    WriteCOM(0x8E);
    WriteDAT_8(0xFF);
    WriteCOM(0x8F);
    WriteDAT_8(0xFF);
    WriteCOM(0x3A);
    WriteDAT_8(0x05);
    WriteCOM(0xEC);
    WriteDAT_8(0x01);
    WriteCOM(0x74);
    WriteDAT_8(0x02);
    WriteDAT_8(0x0E);
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);
    WriteCOM(0x98);
    WriteDAT_8(0x3E);
    WriteCOM(0x99);
    WriteDAT_8(0x3E);

    WriteCOM(0xB5);
    WriteDAT_8(0x0D);
    WriteDAT_8(0x0D);

    WriteCOM(0x60);
    WriteDAT_8(0x38);
    WriteDAT_8(0x0F);
    WriteDAT_8(0x79);
    WriteDAT_8(0x67);

    WriteCOM(0x61);
    WriteDAT_8(0x38);
    WriteDAT_8(0x11);
    WriteDAT_8(0x79);
    WriteDAT_8(0x67);

    WriteCOM(0x64);
    WriteDAT_8(0x38);
    WriteDAT_8(0x17);
    WriteDAT_8(0x71);
    WriteDAT_8(0x5F);
    WriteDAT_8(0x79);
    WriteDAT_8(0x67);

    WriteCOM(0x65);
    WriteDAT_8(0x38);
    WriteDAT_8(0x13);
    WriteDAT_8(0x71);
    WriteDAT_8(0x5B);
    WriteDAT_8(0x79);
    WriteDAT_8(0x67);

    WriteCOM(0x6A);
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);

    WriteCOM(0x6C);
    WriteDAT_8(0x22);
    WriteDAT_8(0x02);
    WriteDAT_8(0x22);
    WriteDAT_8(0x02);
    WriteDAT_8(0x22);
    WriteDAT_8(0x22);
    WriteDAT_8(0x50);

    WriteCOM(0x6E);
    WriteDAT_8(0x03);
    WriteDAT_8(0x03);
    WriteDAT_8(0x01);
    WriteDAT_8(0x01);
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);
    WriteDAT_8(0x0f);
    WriteDAT_8(0x0f);
    WriteDAT_8(0x0d);
    WriteDAT_8(0x0d);
    WriteDAT_8(0x0b);
    WriteDAT_8(0x0b);
    WriteDAT_8(0x09);
    WriteDAT_8(0x09);
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);
    WriteDAT_8(0x0a);//
    WriteDAT_8(0x0a);//
    WriteDAT_8(0x0c);
    WriteDAT_8(0x0c);
    WriteDAT_8(0x0e);
    WriteDAT_8(0x0e);
    WriteDAT_8(0x10);
    WriteDAT_8(0x10);
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);
    WriteDAT_8(0x02);
    WriteDAT_8(0x02);
    WriteDAT_8(0x04);
    WriteDAT_8(0x04);

    WriteCOM(0xbf);
    WriteDAT_8(0x01);

    WriteCOM(0xF9);
    WriteDAT_8(0x40);

    WriteCOM(0x9b);
    WriteDAT_8(0x3b);
    WriteCOM(0x93);
    WriteDAT_8(0x33);
    WriteDAT_8(0x7f);
    WriteDAT_8(0x00);

    WriteCOM(0x7E);
    WriteDAT_8(0x30);

    WriteCOM(0x70);
    WriteDAT_8(0x0d);
    WriteDAT_8(0x02);
    WriteDAT_8(0x08);
    WriteDAT_8(0x0d);
    WriteDAT_8(0x02);
    WriteDAT_8(0x08);

    WriteCOM(0x71);
    WriteDAT_8(0x0d);
    WriteDAT_8(0x02);
    WriteDAT_8(0x08);

    WriteCOM(0x91);
    WriteDAT_8(0x0E);
    WriteDAT_8(0x09);

    WriteCOM(0xc3);
    WriteDAT_8(0x18);
    WriteCOM(0xc4);
    WriteDAT_8(0x18);
    WriteCOM(0xc9);
    WriteDAT_8(0x3c);

    WriteCOM(0xf0);
    WriteDAT_8(0x13);
    WriteDAT_8(0x15);
    WriteDAT_8(0x04);
    WriteDAT_8(0x05);
    WriteDAT_8(0x01);
    WriteDAT_8(0x38);

    WriteCOM(0xf2);
    WriteDAT_8(0x13);
    WriteDAT_8(0x15);
    WriteDAT_8(0x04);
    WriteDAT_8(0x05);
    WriteDAT_8(0x01);
    WriteDAT_8(0x34);

    WriteCOM(0xf1);
    WriteDAT_8(0x4b);
    WriteDAT_8(0xb8);
    WriteDAT_8(0x7b);
    WriteDAT_8(0x34);
    WriteDAT_8(0x35);
    WriteDAT_8(0xef);

    WriteCOM(0xf3);
    WriteDAT_8(0x47);
    WriteDAT_8(0xb4);
    WriteDAT_8(0x72);
    WriteDAT_8(0x34);
    WriteDAT_8(0x35);
    WriteDAT_8(0xda);

    WriteCOM(0x36);
    WriteDAT_8(0x00);

    WriteCOM(0x11);
    lcd_delay(200);

    WriteCOM(0x29);
    WriteCOM(0x2C);
}

static void GC9D01_led_ctrl(u8 status)
{
    //背光控制以及放在//lcd_te_driver.c 优化开机显示
    /*lcd_bl_pinstate(status);*/
}

void GC9D01_test(void)
{
    GC9D01_clear_screen(BLACK);
    lcd_delay(50);//每个屏幕可能会在刷新清屏了，但是屏幕芯片缓存还没刷到屏幕，需要加延时
    lcd_bl_pinstate(1);
//    while (1) {
//        os_time_dly(100);
//        GC9D01_clear_screen(BLUE);
//        printf("LCD_GC9D01_TSET_BLUE\n");
//        os_time_dly(100);
//        GC9D01_clear_screen(DCYAN);
//        printf("LCD_GC9D01_TSET_GRED\n");
//        os_time_dly(100);
//        GC9D01_clear_screen(DGREEN);
//        printf("LCD_GC9D01_TSET_BRRED\n");
//        os_time_dly(100);
//        GC9D01_clear_screen(YELLOW);
//        printf("LCD_GC9D01_TSET_YELLOW\n");
//    }
}

static int GC9D01_init(void)
{
    printf("LCD_GC9D01 init_start\n");
    GC9D01_reset();
    GC9D01_init_code(NULL, 0);
    GC9D01_set_direction(ROTATE_DEGREE_90);
#if USE_LCD_TE
    lcd_te_interrupt_init(1);
#endif
    GC9D01_test();
    printf("LCD_GC9D01 config succes\n");
    return 0;
}


REGISTER_LCD_DEV(LCD_GC9D01) = {
    .name              = "GC9D01",
    .lcd_width         = LCD_W,
    .lcd_height        = LCD_H,
    .color_format      = LCD_COLOR_RGB565,
    .column_addr_align = 1,
    .row_addr_align    = 1,
    .LCD_Init          = GC9D01_init,
    .SetDrawArea       = GC9D01_SetRange,
    .LCD_Mirror1       = GC9D01_SetMirror1,
    .LCD_Draw          = GC9D01_draw,
    .LCD_Draw_1        = GC9D01_draw_1,
    .LCD_DrawToDev     = GC9D01_Fill,//应用层直接到设备接口层，需要做好缓冲区共用互斥，慎用！
    .LCD_Lvgl_Full     = GC9D01_lvgl_Fill,//LVGL发送数据接口
    .LCD_ClearScreen   = GC9D01_clear_screen,
    .Reset             = GC9D01_reset,
    .BackLightCtrl     = GC9D01_led_ctrl,
};

#endif


