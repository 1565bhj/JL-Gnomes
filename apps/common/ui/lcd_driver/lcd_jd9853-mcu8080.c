#include "system/includes.h"
#include "typedef.h"
#include "asm/pap.h"
#include "lcd_drive.h"
#include "lcd_config.h"
#include "gpio.h"

#if TCFG_LCD_JD9853_ENABLE

//#if HORIZONTAL_SCREEN /*横屏*/
//#define LCD_W     320
//#define LCD_H     240
//#else                 /*竖屏*/
//#define LCD_W     240
//#define LCD_H     320
//#endif

static void JD9853_lvgl_Fill(u16 xs, u16 xe, u16 ys, u16 ye, u8 *img);
void jd9853_SetRange(u16 xs, u16 xe, u16 ys, u16 ye)
{
    /******UI每次发送数据都会调用开窗告诉屏幕要刷新那个区域***********/
    set_lcd_ui_x_y(xs, xe, ys, ye);
}
void jd9853_SetRange_1(u16 xs, u16 xe, u16 ys, u16 ye)
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

void jd9853_clear_screen(u32 color)
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

void jd9853_no_wait_Fill(u8 *img, u32 len)
{
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);
}

void jd9853_Fill(u8 *img, u16 len)
{
    lcd_interface_non_block_wait();
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);
}

void jd9853_SleepInMode(void)
{
    WriteCOM(0x10); //Sleep in
    lcd_delay(120); //Delay 120ms
}

void jd9853_SleepOutMode(void)
{
    WriteCOM(0x11); //Sleep out
    lcd_delay(120);  //Delay 120ms
}

static void jd9853_set_direction(u8 dir)
{
    WriteCOM(0x36);    //扫描方向控制

    if (dir == ROTATE_DEGREE_0) { //
#if HORIZONTAL_SCREEN
        WriteDAT_8(0xA0);
#else
        WriteDAT_8(0x00);
#endif
        jd9853_SetRange(0, LCD_W - 1, 0, LCD_H - 1);
    } else if (dir == ROTATE_DEGREE_180) { //翻转180
#if HORIZONTAL_SCREEN
        WriteDAT_8(0xc0);
#else
        WriteDAT_8(0x80);
#endif
    }
}

static void jd9853_draw(u8 *map, u32 size)//获取Ui发送出来的数据
{
    ui_send_data_ready(map, size);
}

static void jd9853_draw_1(u8 *buf, u32 size, int width, int height)//获取camera发送出来的数据 //数据帧数以camera为基准
{
    camera_send_data_ready(buf, size, width, height);
}

static void jd9853_reset(void)
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
    lcd_delay(10);
}

static void jd9853_init_code(void)
{
    os_time_dly(100);
    WriteCOM(0xDF);
    WriteDAT_8(0x98);
    WriteDAT_8(0x53);
    WriteCOM(0xDE);
    WriteDAT_8(0x00);
    WriteCOM(0xB2);
    WriteDAT_8(0x40);
    WriteCOM(0xB7);
    WriteDAT_8(0x00);
    WriteDAT_8(0x33);
    WriteDAT_8(0x00);
    WriteDAT_8(0x5B);
    WriteCOM(0xBB);
    WriteDAT_8(0x4C);
    WriteDAT_8(0x2F);
    WriteDAT_8(0x55);
    WriteDAT_8(0x73);
    WriteDAT_8(0x6F);
    WriteDAT_8(0xF0);
    WriteCOM(0xBC);
    WriteDAT_8(0x77);
    WriteCOM(0xC1);
    WriteDAT_8(0x12);
    WriteCOM(0xC3);
    WriteDAT_8(0x7D);
    WriteDAT_8(0x08);
    WriteDAT_8(0x14);
    WriteDAT_8(0x06);
    WriteDAT_8(0xC9);
    WriteDAT_8(0x72);
    WriteDAT_8(0x6C);
    WriteDAT_8(0x77);
    WriteCOM(0xC4);
    WriteDAT_8(0x00);//00=60Hz 04=55Hz 08=50Hz
    WriteDAT_8(0x00);
    WriteDAT_8(0xA0);//LN=320  Line
    WriteDAT_8(0x79);
    WriteDAT_8(0x0A);
    WriteDAT_8(0x0E);
    WriteDAT_8(0x16);
    WriteDAT_8(0x79);
    WriteDAT_8(0x0A);
    WriteDAT_8(0x0E);
    WriteDAT_8(0x16);
    WriteDAT_8(0x82);

    //SET_R_Gamma2.2
    WriteCOM(0xC8); //G2.5
    WriteDAT_8(0x3F);   //0x3F
    WriteDAT_8(0x38);   //0x37
    WriteDAT_8(0x33);   //0x31
    WriteDAT_8(0x30);   //0x2E
    WriteDAT_8(0x35);   //0x32
    WriteDAT_8(0x3B);   //0x37
    WriteDAT_8(0x38);   //0x34
    WriteDAT_8(0x3A);   //0x37
    WriteDAT_8(0x39);   //0x36
    WriteDAT_8(0x3B);   //0x38
    WriteDAT_8(0x3C);   //0x37
    WriteDAT_8(0x39);   //0x34
    WriteDAT_8(0x38);   //0x32
    WriteDAT_8(0x34);   //0x2E
    WriteDAT_8(0x01);   //0x00
    WriteDAT_8(0x01);   //0x01
    WriteDAT_8(0x3F);   //0x3F
    WriteDAT_8(0x38);   //0x37
    WriteDAT_8(0x33);   //0x32
    WriteDAT_8(0x30);   //0x2E
    WriteDAT_8(0x35);   //0x32
    WriteDAT_8(0x3B);   //0x38
    WriteDAT_8(0x38);   //0x35
    WriteDAT_8(0x3A);   //0x37
    WriteDAT_8(0x39);   //0x35
    WriteDAT_8(0x3B);   //0x38
    WriteDAT_8(0x3C);   //0x37
    WriteDAT_8(0x39);   //0x34
    WriteDAT_8(0x38);   //0x32
    WriteDAT_8(0x34);   //0x2E
    WriteDAT_8(0x01);   //0x00
    WriteDAT_8(0x01);   //0x01
    WriteCOM(0xD0);
    WriteDAT_8(0x04);
    WriteDAT_8(0x06);
    WriteDAT_8(0x6A);
    WriteDAT_8(0x0F);
    WriteDAT_8(0x00);
    WriteCOM(0xD7);
    WriteDAT_8(0x00);//2DL_OPT
    WriteDAT_8(0x30);
    WriteCOM(0xE6);
    WriteDAT_8(0x10);
    WriteCOM(0xDE);
    WriteDAT_8(0x01);
    WriteCOM(0xBB);
    WriteDAT_8(0x04);
    WriteCOM(0xD7);
    WriteDAT_8(0x12);
    WriteCOM(0xB7);
    WriteDAT_8(0x03);
    WriteDAT_8(0x13);
    WriteDAT_8(0xEF);
    WriteDAT_8(0x3B);
    WriteDAT_8(0x3B);
    WriteCOM(0xC1);
    WriteDAT_8(0x14);
    WriteDAT_8(0x15);
    WriteDAT_8(0xC0);
    WriteCOM(0xC2);
    WriteDAT_8(0x06);
    WriteDAT_8(0x3A);
    WriteCOM(0xC4);
    WriteDAT_8(0x72);
    WriteDAT_8(0x12);
    WriteCOM(0xBE);
    WriteDAT_8(0x00);
    WriteCOM(0xDE);
    WriteDAT_8(0x02);
    WriteCOM(0xE5);
    WriteDAT_8(0x00);
    WriteDAT_8(0x02);
    WriteDAT_8(0x00);
    WriteCOM(0xE5);
    WriteDAT_8(0x01);
    WriteDAT_8(0x02);
    WriteDAT_8(0x00);
    WriteCOM(0xDE);
    WriteDAT_8(0x00);
    WriteCOM(0x35);
    WriteDAT_8(0x00);
    WriteCOM(0x36);
    WriteDAT_8(0x00);//00=FW正扫 ; 03=BW反扫
    WriteCOM(0x3A);
    WriteDAT_8(0x05);//06=RGB666；05=RGB565

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

    WriteCOM(0x11);
    os_time_dly(14);

    WriteCOM(0xDE);
    WriteDAT_8(0x02);
    WriteCOM(0xE5);
    WriteDAT_8(0x00);
    WriteDAT_8(0x02);
    WriteDAT_8(0x00);
    WriteCOM(0xDE);
    WriteDAT_8(0x00);
    WriteCOM(0x29);
    os_time_dly(12);
    WriteCOM(0x2C);
}

static void JD9853_lvgl_Fill(u16 xs, u16 xe, u16 ys, u16 ye, u8 *img)
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

static void jd9853_led_ctrl(u8 status)
{
    //背光控制以及放在//lcd_te_driver.c 优化开机显示
    lcd_bl_pinstate(status);
}

void jd9853_test(void)
{
//    printf("-> LCD_W = %d , %d \n",LCD_W,LCD_H);
    jd9853_clear_screen(BLACK);
    lcd_bl_pinstate(lcd_bk_on);
//    while (1) {
//        jd9853_clear_screen(WHITE);
//        printf("LCD_jd9853_TSET_WHITE\n");
//        os_time_dly(100);
//        jd9853_clear_screen(BLACK);
//        printf("LCD_jd9853_TSET_BLACK\n");
//        os_time_dly(100);
//        jd9853_clear_screen(RED);
//        printf("LCD_jd9853_TSET_RED\n");
//         os_time_dly(100);
//        jd9853_clear_screen(BLUE);
//        printf("LCD_jd9853_TSET_BLUE\n");
//        os_time_dly(100);
//        os_time_dly(100);
//        jd9853_clear_screen(DCYAN);
//        printf("LCD_jd9853_TSET_GRED\n");
//        os_time_dly(100);
//        jd9853_clear_screen(DGREEN);
//        printf("LCD_jd9853_TSET_BRRED\n");
//        os_time_dly(100);
//        jd9853_clear_screen(YELLOW);
//        printf("LCD_jd9853_TSET_YELLOW\n");
//    }
}

int jd9853_init(void)
{
    jd9853_reset();
#if TCFG_TOUCH_FT6236_ENABLE
    extern int FT6236_task_init(void);
    FT6236_task_init();
#endif
    printf("LCD_jd9853 init_start\n");
    jd9853_init_code();
    jd9853_set_direction(ROTATE_DEGREE_0);
#if USE_LCD_TE
    init_TE(jd9853_Fill);
#endif
    jd9853_test();
    printf("LCD_jd9853 config succes\n");
    return 0;
}


REGISTER_LCD_DEV(LCD_jd9853) = {
    .name              = "jd9853",
    .lcd_width         = LCD_W,
    .lcd_height        = LCD_H,
    .color_format      = LCD_COLOR_RGB565,
    .column_addr_align = 1,
    .row_addr_align    = 1,
    .LCD_Init          = jd9853_init,
    .SetDrawArea       = jd9853_SetRange,
    .LCD_Draw          = jd9853_draw,
    .LCD_Draw_1        = jd9853_draw_1,
    .LCD_DrawToDev     = jd9853_no_wait_Fill,//应用层直接到设备接口层，需要做好缓冲区共用互斥，慎用！
    .LCD_Lvgl_Full     = JD9853_lvgl_Fill,//LVGL发送数据接口
    .LCD_ClearScreen   = jd9853_clear_screen,
    .Reset             = jd9853_reset,
    .BackLightCtrl     = jd9853_led_ctrl,
};

#endif


