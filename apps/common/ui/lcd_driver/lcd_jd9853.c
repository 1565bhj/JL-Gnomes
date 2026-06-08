#include "system/includes.h"
#include "typedef.h"
#include "asm/pap.h"
#include "lcd_drive.h"
#include "lcd_config.h"
#include "gpio.h"

#if TCFG_LCD_JD9853_ENABLE

#define DEFAULT_ROTATE ROTATE_DEGREE_270
static short dir_angle = -1;
static void JD9853_lvgl_Fill(u16 xs, u16 xe, u16 ys, u16 ye, u8 *img);
void JD9853_SetRange(u16 xs, u16 xe, u16 ys, u16 ye)
{
    /******UI每次发送数据都会调用开窗告诉屏幕要刷新那个区域***********/
    set_lcd_ui_x_y(xs, xe, ys, ye);
}
void JD9853_SetRange_1(u16 xs, u16 xe, u16 ys, u16 ye)
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

void JD9853_clear_screen(u32 color)
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

void JD9853_no_wait_Fill(u8 *img, u32 len)
{
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);
}

void JD9853_Fill(u8 *img, u16 len)
{
    unsigned short w = LCD_W - 1;
    unsigned short h = LCD_H - 1;
    if (dir_angle != (short) -1 && dir_angle != DEFAULT_ROTATE) {
        h = LCD_W - 1;
        w = LCD_H - 1;
    }
    WriteCOM(0x2A);
    WriteDAT_8(0);
    WriteDAT_8(0);
    WriteDAT_8(w >> 8);
    WriteDAT_8(w);
    WriteCOM(0x2B);
    WriteDAT_8(0);
    WriteDAT_8(0);
    WriteDAT_8(h >> 8);
    WriteDAT_8(h);

    lcd_interface_non_block_wait();
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);
}

void JD9853_SleepInMode(void)
{
    WriteCOM(0x10); //Sleep in
    lcd_delay(120); //Delay 120ms
}

void JD9853_SleepOutMode(void)
{
    WriteCOM(0x11); //Sleep out
    lcd_delay(120);  //Delay 120ms
}

static void JD9853_set_direction(int dir)
{
    if (dir_angle == dir) {
        return ;
    }
    dir_angle = dir;
    WriteCOM(0x36);    // MADCTL 扫描方向控制
    if (dir == ROTATE_DEGREE_0 || dir == 0) {
        WriteDAT_8(0x00);
    } else if (dir == ROTATE_DEGREE_90  || dir == 90) {
        WriteDAT_8(0x60);
    } else if (dir == ROTATE_DEGREE_180 || dir == 180) {
        WriteDAT_8(0x40);
    } else if (dir == ROTATE_DEGREE_270 || dir == 270) {
        WriteDAT_8(0xA0);
    }
}
static void JD9853_set_direction_default(int dir)
{
    JD9853_set_direction(DEFAULT_ROTATE);
}
static void JD9853_draw(u8 *map, u32 size)//获取Ui发送出来的数据
{
    ui_send_data_ready(map, size);
}

static void JD9853_draw_1(u8 *buf, u32 size, int width, int height)//获取camera发送出来的数据 //数据帧数以camera为基准
{
    camera_send_data_ready(buf, size, width, height);
}

static void JD9853_reset(void)
{
    printf("reset \n");
    lcd_rst_pinstate(1);
    lcd_rs_pinstate(1);
    lcd_cs_pinstate(1);

    lcd_rst_pinstate(1);
    lcd_delay(120);
    lcd_rst_pinstate(0);
    lcd_delay(10);
    lcd_rst_pinstate(1);
    lcd_delay(50);
}

static void JD9853_init_code(void)
{
    lcd_delay(50);
    WriteCOM(0xDF);
    WriteDAT_8(0x98);
    WriteDAT_8(0x53);

    WriteCOM(0xDE);
    WriteDAT_8(0x00);

    WriteCOM(0xB2);
    WriteDAT_8(0x16);

    WriteCOM(0xB7);
    WriteDAT_8(0x00);
    WriteDAT_8(0x2D);
    WriteDAT_8(0x00);
    WriteDAT_8(0x55);

    WriteCOM(0xBB);
    WriteDAT_8(0x0A);
    WriteDAT_8(0x2F);
    WriteDAT_8(0x55);
    WriteDAT_8(0x73);
    WriteDAT_8(0x63);
    WriteDAT_8(0xF0);

    WriteCOM(0xC0);
    WriteDAT_8(0x22);
    WriteDAT_8(0xA2);

    WriteCOM(0xC1);
    WriteDAT_8(0x12);

    WriteCOM(0xC3);
    WriteDAT_8(0x7E);
    WriteDAT_8(0x08);
    WriteDAT_8(0x0A);
    WriteDAT_8(0x0C);
    WriteDAT_8(0xC4);
    WriteDAT_8(0x73);
    WriteDAT_8(0x22);
    WriteDAT_8(0x77);

    WriteCOM(0xC4);
    WriteDAT_8(0x00);//00=60Hz 04=55Hz 08=50Hz
    WriteDAT_8(0x00);
    WriteDAT_8(0xA0);//LN=320  Line
    WriteDAT_8(0x79);
    WriteDAT_8(0x0A);
    WriteDAT_8(0x0B);
    WriteDAT_8(0x16);
    WriteDAT_8(0x79);
    WriteDAT_8(0x0A);
    WriteDAT_8(0x0B);
    WriteDAT_8(0x16);
    WriteDAT_8(0x82);

    //SET_R_Gamma2.2
    WriteCOM(0xC8); //G2.2
    WriteDAT_8(0x3F);   //0x3F
    WriteDAT_8(0x32);   //0x32
    WriteDAT_8(0x29);   //0x29
    WriteDAT_8(0x24);   //0x24
    WriteDAT_8(0x25);   //0x25
    WriteDAT_8(0x27);   //0x27
    WriteDAT_8(0x22);   //0x22
    WriteDAT_8(0x22);   //0x22
    WriteDAT_8(0x20);   //0x20
    WriteDAT_8(0x1D);   //0x1D
    WriteDAT_8(0x1B);   //0x1B
    WriteDAT_8(0x10);   //0x10
    WriteDAT_8(0x0D);   //0x0D
    WriteDAT_8(0x07);   //0x07
    WriteDAT_8(0x05);   //0x05
    WriteDAT_8(0x00);   //0x00
    WriteDAT_8(0x3F);   //0x3F
    WriteDAT_8(0x32);   //0x32
    WriteDAT_8(0x29);   //0x29
    WriteDAT_8(0x24);   //0x24
    WriteDAT_8(0x25);   //0x25
    WriteDAT_8(0x27);   //0x27
    WriteDAT_8(0x22);   //0x22
    WriteDAT_8(0x22);   //0x22
    WriteDAT_8(0x20);   //0x20
    WriteDAT_8(0x1D);   //0x1D
    WriteDAT_8(0x1B);   //0x1B
    WriteDAT_8(0x10);   //0x10
    WriteDAT_8(0x0D);   //0x0D
    WriteDAT_8(0x07);   //0x07
    WriteDAT_8(0x05);   //0x05
    WriteDAT_8(0x00);   //0x00

    WriteCOM(0xD0);
    WriteDAT_8(0x04);
    WriteDAT_8(0x04);
    WriteDAT_8(0x6C);
    WriteDAT_8(0x1C);
    WriteDAT_8(0x03);

    WriteCOM(0xD7);
    WriteDAT_8(0x20);//2DL_OPT
    WriteDAT_8(0x30);

    WriteCOM(0xE6);
    WriteDAT_8(0x14);

    WriteCOM(0xDE);
    WriteDAT_8(0x01);

    WriteCOM(0xBB);
    WriteDAT_8(0x04);
    WriteCOM(0xD7);
    WriteDAT_8(0x12);

    WriteCOM(0xB7);
    WriteDAT_8(0x03);
    WriteDAT_8(0x13);
    WriteDAT_8(0xE5);
    WriteDAT_8(0x38);
    WriteDAT_8(0x38);

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

    WriteCOM(0x2A);
    WriteDAT_8(0);
    WriteDAT_8(0);
    WriteDAT_8((LCD_W - 1) >> 8);
    WriteDAT_8((LCD_W - 1));
    WriteCOM(0x2B);
    WriteDAT_8(0);
    WriteDAT_8(0);
    WriteDAT_8((LCD_H - 1) >> 8);
    WriteDAT_8((LCD_H - 1));

    WriteCOM(0x11);
    lcd_delay(200);//至少200ms

    WriteCOM(0xDE);
    WriteDAT_8(0x02);

    WriteCOM(0xE5);
    WriteDAT_8(0x00);
    WriteDAT_8(0x02);
    WriteDAT_8(0x00);

    WriteCOM(0xDE);
    WriteDAT_8(0x00);

    WriteCOM(0x35);
    WriteDAT_8(0x00);

    WriteCOM(0x29);
    lcd_delay(20);
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

static void JD9853_led_ctrl(u8 status)
{
    //背光控制以及放在//lcd_te_driver.c 优化开机显示
    lcd_bl_pinstate(status);
}

void JD9853_test(void)
{
    JD9853_clear_screen(BLACK);
    lcd_delay(50);//每个屏幕可能会在刷新清屏了，但是屏幕芯片缓存还没刷到屏幕，需要加延时
    lcd_bl_pinstate(1);
}

int JD9853_init(void)
{
    JD9853_reset();
    printf("LCD_JD9853 init_start\n");
#if USE_LCD_TE
    lcd_te_interrupt_init(1);
#endif
    JD9853_init_code();
    JD9853_set_direction_default(DEFAULT_ROTATE);
    JD9853_test();
    printf("LCD_JD9853 config succes\n");
    return 0;
}


REGISTER_LCD_DEV(LCD_JD9853) = {
    .name                   = "JD9853",
    .lcd_width              = LCD_W,
    .lcd_height             = LCD_H,
    .color_format           = LCD_COLOR_RGB565,
    .column_addr_align      = 1,
    .row_addr_align         = 1,
    .LCD_Init               = JD9853_init,
    .SetDrawArea            = JD9853_SetRange,
    .LCD_Draw               = JD9853_draw,
    .LCD_Draw_1             = JD9853_draw_1,
    .LCD_DrawToDev          = JD9853_Fill,//应用层直接到设备接口层，需要做好缓冲区共用互斥，慎用！
    .LCD_Lvgl_Full          = JD9853_lvgl_Fill,//LVGL发送数据接口
    .LCD_ClearScreen        = JD9853_clear_screen,
    .Reset                  = JD9853_reset,
    .BackLightCtrl          = JD9853_led_ctrl,
    .set_dirction           = JD9853_set_direction,
    .set_dirction_default   = JD9853_set_direction_default,
};

#endif


