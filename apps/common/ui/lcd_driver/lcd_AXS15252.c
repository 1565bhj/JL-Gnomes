#include "system/includes.h"
#include "typedef.h"
#include "asm/pap.h"
#include "lcd_drive.h"
#include "lcd_config.h"
#include "gpio.h"

#if TCFG_LCD_AXS15252_ENABLE


static void AXS15252_lvgl_Fill(u16 xs, u16 xe, u16 ys, u16 ye, u8 *img);
void AXS15252_SetRange(u16 xs, u16 xe, u16 ys, u16 ye)
{
    /******UI每次发送数据都会调用开窗告诉屏幕要刷新那个区域***********/
    set_lcd_ui_x_y(xs, xe, ys, ye);
}
void AXS15252_SetRange_1(u16 xs, u16 xe, u16 ys, u16 ye)
{
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
}

void AXS15252_clear_screen(u32 color)
{
//    lcd_interface_non_block_wait();
////    /* AXS15252 像素填充：0x2A=CASET(列地址) 0x2B=RASET(行地址) 0x2C=RAMWR(显存写) */
//    WriteCOM(0x2B);
//    WriteDAT_8(0);
//    WriteDAT_8(0);
//    WriteDAT_8((LCD_H - 1) >> 8);
//    WriteDAT_8(LCD_H - 1);
//    WriteCOM(0x2A);
//    WriteDAT_8(0);
//    WriteDAT_8(0);
//    WriteDAT_8((LCD_W - 1) >> 8);
//    WriteDAT_8(LCD_W - 1);
//    WriteCOM(0x2C);
    unsigned short w = LCD_H;
    unsigned short h = LCD_W;
    u8 *buf = malloc(w * h * 2);
    if (!buf) {
        printf("no men in %s \n", __func__);
        return;
    }
    for (u32 i = 0; i < w * h; i++) {
        buf[2 * i] = (color >> 8) & 0xff;
        buf[2 * i + 1] = color & 0xff;
    }
    WriteCOM(0x2B);
    WriteDAT_8(0);
    WriteDAT_8(0);
    WriteDAT_8((w - 1) >> 8);
    WriteDAT_8((w - 1));
    WriteCOM(0x2A);
    WriteDAT_8(0);
    WriteDAT_8(0);
    WriteDAT_8((h - 1) >> 8);
    WriteDAT_8((h - 1));
    lcd_interface_non_block_wait();
    WriteCOM(0x2c);
    WriteDAT_DMA(buf, w * h * 2);
    free(buf);
}

void AXS15252_no_wait_Fill(u8 *img, u32 len)
{
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);
}

void AXS15252_Fill(u8 *img, u16 len)
{
//    unsigned short w = LCD_W;
//    unsigned short h = LCD_H;
    unsigned short w = LCD_H;
    unsigned short h = LCD_W;
    WriteCOM(0x2B);
    WriteDAT_8(0);
    WriteDAT_8(0);
    WriteDAT_8((w - 1) >> 8);
    WriteDAT_8((w - 1));
    WriteCOM(0x2A);
    WriteDAT_8(0);
    WriteDAT_8(0);
    WriteDAT_8((h - 1) >> 8);
    WriteDAT_8((h - 1));
    lcd_interface_non_block_wait();
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);


}

void AXS15252_SleepInMode(void)
{
    WriteCOM(0x10); //Sleep in
    lcd_delay(120); //Delay 120ms
}

void AXS15252_SleepOutMode(void)
{
    WriteCOM(0x11); //Sleep out
    lcd_delay(120);  //Delay 120ms
}

static void AXS15252_set_direction(int dir)
{
    static short dir_angle = -1;
    if (dir_angle == dir) {
        return ;
    }
    dir_angle = dir;
    WriteCOM(0x36);    //扫描方向控制
    WriteDAT_8(0xA0);
//    if (dir == ROTATE_DEGREE_0) { //
//#if HORIZONTAL_SCREEN
//        WriteDAT_8(0x00);
//#else
//        WriteDAT_8(0xC0);
//#endif
//    } else if (dir == ROTATE_DEGREE_180) { //翻转180
//#if HORIZONTAL_SCREEN
//        WriteDAT_8(0xA0);
//#else
//        WriteDAT_8(0x00);
//#endif
////       AXS15252_SetRange_1(0, LCD_W - 1, 0, LCD_H - 1);
//    } else if (dir == ROTATE_DEGREE_90) { //翻转90  MX+MV
//        WriteDAT_8(0x60);
////        AXS15252_SetRange_1(0, LCD_W - 1, 0, LCD_H - 1);
//    }
}
static void AXS15252_set_direction_default(int dir)
{
    static short dir_angle_default = -1;
    if (dir_angle_default  == (short) -1) {
        dir_angle_default = dir;
    }
    AXS15252_set_direction(dir_angle_default);
}
static void AXS15252_draw(u8 *map, u32 size)//获取Ui发送出来的数据
{
    ui_send_data_ready(map, size);
}

static void AXS15252_draw_1(u8 *buf, u32 size, int width, int height)//获取camera发送出来的数据 //数据帧数以camera为基准
{
    camera_send_data_ready(buf, size, width, height);
}

static void AXS15252_reset(void)
{
    printf("reset \n");
    lcd_rst_pinstate(1);
    lcd_rs_pinstate(1);
    lcd_cs_pinstate(1);

    lcd_delay(120);
    lcd_rst_pinstate(1);
    lcd_delay(120);
    lcd_rst_pinstate(0);
    lcd_delay(100);
    lcd_rst_pinstate(1);
    lcd_delay(120);
}

static void AXS15252_init_code(void)
{


    WriteCOM(0xce);
    WriteDAT_8(0X5A); // 00
    WriteDAT_8(0XA5); // 01

    WriteCOM(0xa0);
    WriteDAT_8(0X01); // 00

    WriteCOM(0xa1);
    WriteDAT_8(0X00); // 00
    WriteDAT_8(0XDD); // 01
    WriteDAT_8(0X00); // 02
    WriteDAT_8(0X12); // 03
    WriteDAT_8(0X32); // 04
    WriteDAT_8(0X42); // 05
    WriteDAT_8(0XA2); // 06
    WriteDAT_8(0X52); // 07
    WriteDAT_8(0X61); // 08
    WriteDAT_8(0X60); // 09
    WriteDAT_8(0X05); // 10
    WriteDAT_8(0X40); // 11
    WriteDAT_8(0X00); // 12
    WriteDAT_8(0X00); // 13
    WriteDAT_8(0X00); // 14
    WriteDAT_8(0X00); // 15
    WriteDAT_8(0X00); // 16
    WriteDAT_8(0X00); // 17
    WriteDAT_8(0X00); // 18
    WriteDAT_8(0X00); // 19
    WriteDAT_8(0XEB); // 20
    WriteDAT_8(0X6B); // 21
    WriteDAT_8(0XBB); // 22
    WriteDAT_8(0X3B); // 23
    WriteDAT_8(0XA5); // 24
    WriteDAT_8(0X5A); // 25

    WriteCOM(0xa2);
    WriteDAT_8(0X00); // 00
    WriteDAT_8(0X00); // 01
    WriteDAT_8(0X03); // 02

    WriteCOM(0xa4);
    WriteDAT_8(0X00); // 00
    WriteDAT_8(0X00); // 01
    WriteDAT_8(0X22); // 02
    WriteDAT_8(0XD3); // 03

    WriteCOM(0xa5);
    WriteDAT_8(0X00); // 00

    WriteCOM(0xb1);
    WriteDAT_8(0X02); // 00
    WriteDAT_8(0X00); // 01
    WriteDAT_8(0X50); // 02

    WriteCOM(0xb2);
    WriteDAT_8(0X10); // 00
    WriteDAT_8(0X30); // 01
    WriteDAT_8(0X44); // 02
    WriteDAT_8(0X05); // 03
    WriteDAT_8(0X03); // 04
    WriteDAT_8(0X05); // 05
    WriteDAT_8(0X08); // 06
    WriteDAT_8(0X42); // 07
    WriteDAT_8(0X7F); // 08

    WriteCOM(0xb3);
    WriteDAT_8(0X11); // 00
    WriteDAT_8(0X09); // 01
    WriteDAT_8(0XAF); // 02
    WriteDAT_8(0X30); // 03
    WriteDAT_8(0X0D); // 04
    WriteDAT_8(0X10); // 05
    WriteDAT_8(0X03); // 06
    WriteDAT_8(0X22); // 07
    WriteDAT_8(0X00); // 08
    WriteDAT_8(0X00); // 09
    WriteDAT_8(0X02); // 10
    WriteDAT_8(0X22); // 11
    WriteDAT_8(0X4A); // 12
    WriteDAT_8(0X02); // 13
    WriteDAT_8(0X12); // 14
    WriteDAT_8(0X01); // 15
    WriteDAT_8(0X42); // 16
    WriteDAT_8(0X25); // 17
    WriteDAT_8(0X01); // 18
    WriteDAT_8(0X5C); // 19
    WriteDAT_8(0X65); // 20
    WriteDAT_8(0X17); // 21

    WriteCOM(0xb4);
    WriteDAT_8(0X02); // 00
    WriteDAT_8(0X50); // 01
    WriteDAT_8(0X06); // 02
    WriteDAT_8(0X01); // 03
    WriteDAT_8(0X30); // 04
    WriteDAT_8(0X10); // 05
    WriteDAT_8(0X80); // 06
    WriteDAT_8(0X6B); // 07
    WriteDAT_8(0X03); // 08
    WriteDAT_8(0X01); // 09
    WriteDAT_8(0X10); // 10
    WriteDAT_8(0X10); // 11
    WriteDAT_8(0X00); // 12
    WriteDAT_8(0X00); // 13
    WriteDAT_8(0X02); // 14

    WriteCOM(0xb5);
    WriteDAT_8(0X00); // 00
    WriteDAT_8(0X0A); // 01
    WriteDAT_8(0X20); // 02
    WriteDAT_8(0X14); // 03
    WriteDAT_8(0X50); // 04
    WriteDAT_8(0XB0); // 05
    WriteDAT_8(0X30); // 06
    WriteDAT_8(0X03); // 07


    WriteCOM(0xb6);
    WriteDAT_8(0X40); // 00
    WriteDAT_8(0X01); // 01
    WriteDAT_8(0XF0); // 02
    WriteDAT_8(0X10); // 03
    WriteDAT_8(0X40); // 04
    WriteDAT_8(0X40); // 05
    WriteDAT_8(0X01); // 06

    WriteCOM(0xb8);
    WriteDAT_8(0X00); // 00
    WriteDAT_8(0X35); // 01
    WriteDAT_8(0XCD); // 02
    WriteDAT_8(0X44); // 03
    WriteDAT_8(0X00); // 04
    WriteDAT_8(0X08); // 05
    WriteDAT_8(0X00); // 06
    WriteDAT_8(0X30); // 07
    WriteDAT_8(0X57); // 08
    WriteDAT_8(0X30); // 09
    WriteDAT_8(0X57); // 10
    WriteDAT_8(0X03); // 11
    WriteDAT_8(0X01); // 12
    WriteDAT_8(0X00); // 13
    WriteDAT_8(0X02); // 14
    WriteDAT_8(0X00); // 15
    WriteDAT_8(0X03); // 16
    WriteDAT_8(0X03); // 17
    WriteDAT_8(0X8D); // 18
    WriteDAT_8(0X60); // 19
    WriteDAT_8(0X01); // 20
    WriteDAT_8(0X83); // 21
    WriteDAT_8(0X03); // 22
    WriteDAT_8(0X00); // 23
    WriteDAT_8(0X00); // 24

    WriteCOM(0xb9);
    WriteDAT_8(0X10); // 00
    WriteDAT_8(0X32); // 01
    WriteDAT_8(0X54); // 02
    WriteDAT_8(0X76); // 03
    WriteDAT_8(0X98); // 04
    WriteDAT_8(0XBA); // 05
    WriteDAT_8(0XDC); // 06
    WriteDAT_8(0XFE); // 07
    WriteDAT_8(0X34); // 08
    WriteDAT_8(0XCB); // 09
    WriteDAT_8(0X20); // 10
    WriteDAT_8(0XBD); // 11
    WriteDAT_8(0X3E); // 12
    WriteDAT_8(0X02); // 13
    WriteDAT_8(0X35); // 14
    WriteDAT_8(0X35); // 15
    WriteDAT_8(0X00); // 16
    WriteDAT_8(0X36); // 17
    WriteDAT_8(0X36); // 18
    WriteDAT_8(0X00); // 19
    WriteDAT_8(0X20); // 20
    WriteDAT_8(0X50); // 21
    WriteDAT_8(0X20); // 22
    WriteDAT_8(0X50); // 23
    WriteDAT_8(0X20); // 24
    WriteDAT_8(0X50); // 25
    WriteDAT_8(0X20); // 26
    WriteDAT_8(0X03); // 27
    WriteDAT_8(0X00); // 28
    WriteDAT_8(0X00); // 29
    WriteDAT_8(0X4C); // 30

    WriteCOM(0xba);
    WriteDAT_8(0X28); // 00
    WriteDAT_8(0X1F); // 01
    WriteDAT_8(0X65); // 02
    WriteDAT_8(0X00); // 03

    WriteCOM(0xbb);
    WriteDAT_8(0X02); // 00
    WriteDAT_8(0X00); // 01
    WriteDAT_8(0X0A); // 02
    WriteDAT_8(0X08); // 03
    WriteDAT_8(0X0E); // 04
    WriteDAT_8(0X0C); // 05
    WriteDAT_8(0X1F); // 06
    WriteDAT_8(0X18); // 07
    WriteDAT_8(0X1F); // 08
    WriteDAT_8(0X1B); // 09
    WriteDAT_8(0X1A); // 10
    WriteDAT_8(0X1F); // 11

    WriteCOM(0xbc);
    WriteDAT_8(0X03); // 00
    WriteDAT_8(0X01); // 01
    WriteDAT_8(0X0B); // 02
    WriteDAT_8(0X09); // 03
    WriteDAT_8(0X0F); // 04
    WriteDAT_8(0X0D); // 05
    WriteDAT_8(0X1F); // 06
    WriteDAT_8(0X18); // 07
    WriteDAT_8(0X1F); // 08
    WriteDAT_8(0X1B); // 09
    WriteDAT_8(0X1A); // 10
    WriteDAT_8(0X1F); // 11

    WriteCOM(0xc0);
    WriteDAT_8(0XE0); // 00
    WriteDAT_8(0X03); // 01
    WriteDAT_8(0X00); // 02
    WriteDAT_8(0X00); // 03
    WriteDAT_8(0X00); // 04
    WriteDAT_8(0X00); // 05
    WriteDAT_8(0X50); // 06
    WriteDAT_8(0X04); // 07
    WriteDAT_8(0X01); // 08
    WriteDAT_8(0X80); // 09
    WriteDAT_8(0X06); // 10
    WriteDAT_8(0X06); // 11
    WriteDAT_8(0X28); // 12
    WriteDAT_8(0Xf0); // 13
    WriteDAT_8(0X10); // 14
    WriteDAT_8(0X40); // 15
    WriteDAT_8(0X06); // 16
    WriteDAT_8(0X06); // 17
    WriteDAT_8(0X06); // 18
    WriteDAT_8(0X06); // 19
    WriteDAT_8(0X00); // 20

    WriteCOM(0xc3);
    WriteDAT_8(0X03); // 00
    WriteDAT_8(0X00); // 01
    WriteDAT_8(0X00); // 02

    WriteCOM(0xc4);
    WriteDAT_8(0X05); // 00
    WriteDAT_8(0X4A); // 01
    WriteDAT_8(0X05); // 02
    WriteDAT_8(0X0A); // 03
    WriteDAT_8(0XE0); // 04
    WriteDAT_8(0X2E); // 05
    WriteDAT_8(0X00); // 06
    WriteDAT_8(0X12); // 07
    WriteDAT_8(0X12); // 08
    WriteDAT_8(0X22); // 09
    WriteDAT_8(0X00); // 10
    WriteDAT_8(0X52); // 11
    WriteDAT_8(0X11); // 12
    WriteDAT_8(0X00); // 13
    WriteDAT_8(0X00); // 14
    WriteDAT_8(0X00); // 15
    WriteDAT_8(0X00); // 16
    WriteDAT_8(0X00); // 17
    WriteDAT_8(0X00); // 18

    WriteCOM(0xd9);
    WriteDAT_8(0X42); // 00

    WriteCOM(0xe0);
    WriteDAT_8(0X15); // 00 vr0
    WriteDAT_8(0X30); // 01 vr1
    WriteDAT_8(0X6a); // 02 vr2
    WriteDAT_8(0X7a); // 03 vr3
    WriteDAT_8(0X00); // 04 vr 0/1/2/3
    WriteDAT_8(0X9a); // 05 vr4
    WriteDAT_8(0X9b); // 06 vr5
    WriteDAT_8(0Xb0); // 07 vr6
    WriteDAT_8(0Xc8); // 08 vr7
    WriteDAT_8(0X00); // 09 vr 4/5/6/7
    WriteDAT_8(0Xf0); // 10 vr8
    WriteDAT_8(0X01); // 11 vr9
    WriteDAT_8(0X25); // 12 vr10
    WriteDAT_8(0X60); // 13 vr11
    WriteDAT_8(0X15); // 14 vr 8/9/10/11
    WriteDAT_8(0X88); // 15 vr12
    WriteDAT_8(0Xa8); // 16 vr13
    WriteDAT_8(0Xc5); // 17 vr14
    WriteDAT_8(0Xde); // 18 vr15
    WriteDAT_8(0X55); // 19 vr 12/13/14/15

    WriteCOM(0xe1);
    WriteDAT_8(0Xf9); // 00 vr16
    WriteDAT_8(0Xfa); // 01 vr17
    WriteDAT_8(0X12); // 02 vr18
    WriteDAT_8(0X30); // 03 vr19
    WriteDAT_8(0X5a); // 04 vr16/17/18/19
    WriteDAT_8(0X51); // 05 vr20
    WriteDAT_8(0X78); // 06 vr21
    WriteDAT_8(0Xa0); // 07 vr22
    WriteDAT_8(0Xd0); // 08 vr23
    WriteDAT_8(0Xaa); // 09 vr20/21/22/23
    WriteDAT_8(0Xf0); // 10 vr24
    WriteDAT_8(0X10); // 11 vr25
    WriteDAT_8(0X20); // 12 vr26
    WriteDAT_8(0X28); // 13 vr27
    WriteDAT_8(0Xbf); // 14 vr24/25/26/27
    WriteDAT_8(0X48); // 15 vr28
    WriteDAT_8(0X50); // 16 vr29
    WriteDAT_8(0X33); // 17 vr28/29
    WriteDAT_8(0X64); // 18 vr30
    WriteDAT_8(0X70); // 19 vr31
    WriteDAT_8(0X75); // 20 vr32
    WriteDAT_8(0X8e); // 21 vr33
    WriteDAT_8(0XFF); // 22 vr30/31/32/33

    WriteCOM(0xe6);
    WriteDAT_8(0XF0); // 00
    WriteDAT_8(0X12); // 01
    WriteDAT_8(0X22); // 02
    WriteDAT_8(0X00); // 03
    WriteDAT_8(0X87); // 04
    WriteDAT_8(0X5F); // 05
    WriteDAT_8(0X87); // 06
    WriteDAT_8(0X41); // 07

    WriteCOM(0xe9);
    WriteDAT_8(0X05); // 00
    WriteDAT_8(0X00); // 01
    WriteDAT_8(0X00); // 02

    WriteCOM(0xf3);
    WriteDAT_8(0X13); // 00

    WriteCOM(0xf4);
    WriteDAT_8(0X00); // 00
    WriteDAT_8(0X00); // 01

    WriteCOM(0xce);
    WriteDAT_8(0X00); // 00
    WriteDAT_8(0X00); // 01

//    WriteCOM(0x2a);
//    WriteDAT_8(0x00);
//    WriteDAT_8(0x00);
//    WriteDAT_8((LCD_W - 1) >> 8);
//    WriteDAT_8(LCD_W - 1);
//    WriteCOM(0x2b);
//    WriteDAT_8(0x00);
//    WriteDAT_8(0x00);
//    WriteDAT_8((LCD_H - 1) >> 8);
//    WriteDAT_8(LCD_H - 1);
////    WriteCOM(0x2b);
////    WriteDAT_8(0x00);
////    WriteDAT_8(0x00);
////    WriteDAT_8((LCD_W - 1) >> 8);
////    WriteDAT_8(LCD_W - 1);
////    WriteCOM(0x2a);
////    WriteDAT_8(0x00);
////    WriteDAT_8(0x00);
////    WriteDAT_8((LCD_H - 1) >> 8);
////    WriteDAT_8(LCD_H - 1);


    lcd_delay(50);
    WriteCOM(0x11);
    lcd_delay(200);

    WriteCOM(0x29);
    lcd_delay(100);

    WriteCOM(0x36);
    WriteDAT_8(0X60); // 00

    lcd_delay(100);
}

static void AXS15252_lvgl_Fill(u16 xs, u16 xe, u16 ys, u16 ye, u8 *img)
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

static void AXS15252_led_ctrl(u8 status)
{
    //背光控制以及放在//lcd_te_driver.c 优化开机显示
    lcd_bl_pinstate(status);
}

void AXS15252_test(void)
{
    AXS15252_clear_screen(BLACK);
    AXS15252_clear_screen(BLACK);
    lcd_delay(50);//每个屏幕可能会在刷新清屏了，但是屏幕芯片缓存还没刷到屏幕，需要加延时
    lcd_bl_pinstate(1);
//    while (1) {
//         os_time_dly(100);
//         AXS15252_clear_screen(BLUE);
//         printf("LCD_AXS15252_TSET_BLUE\n");
//         os_time_dly(100);
//         AXS15252_clear_screen(DCYAN);
//         printf("LCD_AXS15252_TSET_GRED\n");
//         os_time_dly(100);
//         AXS15252_clear_screen(DGREEN);
//         printf("LCD_AXS15252_TSET_BRRED\n");
//         os_time_dly(100);
//         AXS15252_clear_screen(YELLOW);
//         printf("LCD_AXS15252_TSET_YELLOW\n");
//    }
}
int AXS15252_init(void)
{
    AXS15252_reset();
    printf("LCD_AXS15252 init_start\n");
#if USE_LCD_TE
    lcd_te_interrupt_init(1);
#endif
    AXS15252_init_code();
    AXS15252_set_direction_default(ROTATE_DEGREE_90);   /* 顺时针90度，0xE0 不镜像 */
    AXS15252_test();
    printf("LCD_AXS15252 config succes\n");
    return 0;
}


REGISTER_LCD_DEV(LCD_AXS15252) = {
    .name                   = "AXS15252",
    .lcd_width              = LCD_W,
    .lcd_height             = LCD_H,
    .color_format           = LCD_COLOR_RGB565,
    .column_addr_align      = 4, /* 与 LVGL rounder_cb 1x4 列对齐一致，供 lcd_drive 等使用 */
    .row_addr_align         = 1,
    .LCD_Init               = AXS15252_init,
    .SetDrawArea            = AXS15252_SetRange,
    .LCD_Draw               = AXS15252_draw,
    .LCD_Draw_1             = AXS15252_draw_1,
    .LCD_DrawToDev          = AXS15252_Fill,//应用层直接到设备接口层，需要做好缓冲区共用互斥，慎用！
    .LCD_Lvgl_Full          = AXS15252_lvgl_Fill,//LVGL发送数据接口
    .LCD_ClearScreen        = AXS15252_clear_screen,
    .Reset                  = AXS15252_reset,
    .BackLightCtrl          = AXS15252_led_ctrl,
    .set_dirction           = AXS15252_set_direction,
    .set_dirction_default   = AXS15252_set_direction_default,
};

#endif


