#include "system/includes.h"
#include "typedef.h"
#include "asm/pap.h"
#include "lcd_drive.h"
#include "lcd_config.h"
#include "gpio.h"

#if TCFG_LCD_NV3031A_ENABLE

static void NV3031A_lvgl_Fill(u16 xs, u16 xe, u16 ys, u16 ye, u8 *img);
void NV3031A_SetRange(u16 xs, u16 xe, u16 ys, u16 ye)
{
    /******UI每次发送数据都会调用开窗告诉屏幕要刷新那个区域***********/
    set_lcd_ui_x_y(xs, xe, ys, ye);
}
void NV3031A_SetRange_1(u16 xs, u16 xe, u16 ys, u16 ye)
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

void NV3031A_clear_screen(u32 color)
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

void NV3031A_no_wait_Fill(u8 *img, u32 len)
{
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);
}

void NV3031A_Fill(u8 *img, u16 len)
{
//    WriteCOM(0x2A);
//    WriteDAT_8(0);
//    WriteDAT_8(0);
//    WriteDAT_8((LCD_H - 1) >> 8);
//    WriteDAT_8((LCD_H - 1));
//    WriteCOM(0x2B);
//    WriteDAT_8(0);
//    WriteDAT_8(0);
//    WriteDAT_8((LCD_W - 1) >> 8);
//    WriteDAT_8((LCD_W - 1));

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

    lcd_interface_non_block_wait();
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);
}

void NV3031A_SleepInMode(void)
{
    WriteCOM(0x10); //Sleep in
    lcd_delay(120); //Delay 120ms
}

void NV3031A_SleepOutMode(void)
{
    WriteCOM(0x11); //Sleep out
    lcd_delay(120);  //Delay 120ms
}

static void NV3031A_set_direction(int dir)
{
    static short dir_angle = -1;
    if (dir_angle == dir) {
        return ;
    }
    dir_angle = dir;
    WriteCOM(0x36);    //扫描方向控制
    if (dir == ROTATE_DEGREE_0) { //
#if HORIZONTAL_SCREEN
        WriteDAT_8(0x00);
#else
        WriteDAT_8(0xC0);
#endif
    } else if (dir == ROTATE_DEGREE_180) { //翻转180
#if HORIZONTAL_SCREEN
        WriteDAT_8(0x00);
#else
        WriteDAT_8(0xA0);
#endif
    } else if (dir == ROTATE_DEGREE_90) { //翻转90
        WriteDAT_8(0x60);
    }
}
static void NV3031A_set_direction_default(int dir)
{
    static short dir_angle_default = -1;
    if (dir_angle_default  == (short) -1) {
        dir_angle_default = dir;
    }
    NV3031A_set_direction(dir_angle_default);
}
static void NV3031A_draw(u8 *map, u32 size)//获取Ui发送出来的数据
{
    ui_send_data_ready(map, size);
}

static void NV3031A_draw_1(u8 *buf, u32 size, int width, int height)//获取camera发送出来的数据 //数据帧数以camera为基准
{
    camera_send_data_ready(buf, size, width, height);
}

static void NV3031A_reset(void)
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

static void NV3031A_init_code(void)
{
    os_time_dly(5);
    WriteCOM(0xFD);
    WriteDAT_8(0x06);
    WriteDAT_8(0x08);
    WriteCOM(0x61);
    WriteDAT_8(0x07);
    WriteDAT_8(0x07);
    WriteCOM(0x73);
    WriteDAT_8(0x70);
    WriteCOM(0x73);
    WriteDAT_8(0x00); //07
    //bias
    WriteCOM(0x62);//
    WriteDAT_8(0x00);
    WriteDAT_8(0x44);
    WriteDAT_8(0x40);
    WriteCOM(0x63);//
    WriteDAT_8(0x41);//
    WriteDAT_8(0x07);//
    WriteDAT_8(0x12);//
    WriteDAT_8(0x12);//
    //VSP
    WriteCOM(0x65);//Pump1=4.7MHz //PUMP1 VSP
    WriteDAT_8(0x09);//D6-5:pump1_clk[1:0] clamp 28 2b
    WriteDAT_8(0x10);//6.26
    WriteDAT_8(0x21);
    //VSN
    WriteCOM(0x66); //pump=2 AVCL
    WriteDAT_8(0x09); //clamp 08 0b 09
    WriteDAT_8(0x10); //10
    WriteDAT_8(0x21);
    //add source_neg_time
    WriteCOM(0x67);//pump_sel
    WriteDAT_8(0x21);//21 20
    WriteDAT_8(0x40);
    WriteCOM(0x68);//gamma vap/van
    WriteDAT_8(0x90);//78-90-af 90
    WriteDAT_8(0x30);// 4c
    WriteDAT_8(0x25);//
    WriteDAT_8(0x21);//
    WriteCOM(0xb1);//frame rate
    WriteDAT_8(0x0f);//0x0f fr_h[5:0]
    WriteDAT_8(0x02);//0x02 fr_v[4:0]
    WriteDAT_8(0x01);//0x04 fr_div[2:0]
    WriteCOM(0xB4);
    WriteDAT_8(0x01); //1dot
    WriteCOM(0xB5);////porch
    WriteDAT_8(0x02);//0x02 vfp[6:0]
    WriteDAT_8(0x02);//0x02 vbp[6:0]
    WriteDAT_8(0x0a);//0x0A hfp[6:0]
    WriteDAT_8(0x14);//0x14 hbp[6:0]
    //48
    //NV3031A-B
    WriteCOM(0xB6); //display function
    WriteDAT_8(0x44); //rev sm
    WriteDAT_8(0x01); //gs norblack
    WriteDAT_8(0x9f);
    WriteDAT_8(0x00);
    WriteDAT_8(0x02);
    WriteCOM(0xdf);//
    WriteDAT_8(0x11);//gofc_gamma_en_sel=1
    ////gamma_test1 A1#_wangly
    //GAMMA---------------------------------/////////////
    WriteCOM(0xe0); //gmama set 2.2
    WriteDAT_8(0x05); //PKP0[4:0]//60
    WriteDAT_8(0x05); //PKP1[4:0]//56
    WriteDAT_8(0x0D); //PKP2[4:0]//45
    WriteDAT_8(0x0E); //PKP3[4:0]//37
    WriteDAT_8(0x0D); //PKP4[4:0]29
    WriteDAT_8(0x10); //PKP5[4:0]21
    WriteDAT_8(0x13); //PKP6[4:0]7
    WriteDAT_8(0x16); //PKP6[4:0]3
    WriteCOM(0xe3);
    WriteDAT_8(0x16); //PKN0[4:0]3
    WriteDAT_8(0x13); //PKN1[4:0] 7
    WriteDAT_8(0x10); //PKN2[4:0] 21
    WriteDAT_8(0x0D); //PKN3[4:0]29
    WriteDAT_8(0x0E); //PKN4[4:0]37
    WriteDAT_8(0x0D); //PKN5[4:0]45
    WriteDAT_8(0x06); //PKN6[4:0]56
    WriteDAT_8(0x06); //PKN6[4:0]60
    WriteCOM(0xe1);
    WriteDAT_8(0x10); //PRP0[6:0]51
    WriteDAT_8(0x5A); //PRP1[6:0]15
    WriteCOM(0xe4);
    WriteDAT_8(0x5A); //PRN0[6:0]15
    WriteDAT_8(0x11); //PRN1[6:0]51
    WriteCOM(0xe2);
    WriteDAT_8(0x06); //VRP0[5:0]63
    WriteDAT_8(0x06); //VRP1[5:0]62
    WriteDAT_8(0x04); //VRP2[5:0]61
    WriteDAT_8(0x27); //VRP3[5:0]2
    WriteDAT_8(0x2B); //VRP4[5:0]1
    WriteDAT_8(0x3f); //VRP5[5:0]0
    WriteCOM(0xe5);
    WriteDAT_8(0x3f); //VRN0[5:0]0
    WriteDAT_8(0x2C); //VRN1[5:0]1
    WriteDAT_8(0x27); //VRN2[5:0]2
    WriteDAT_8(0x04); //VRN3[5:0]61
    WriteDAT_8(0x07); //VRN4[5:0]62
    WriteDAT_8(0x06); //VRN5[5:0]63
    //GAMMA---------------------------------/////////////
    WriteCOM(0xE6);
    WriteDAT_8(0x00);
    WriteDAT_8(0xff);//SC_EN_START[7:0] f0
    WriteCOM(0xE7);
    WriteDAT_8(0x01);//CS_START[3:0] 01
    WriteDAT_8(0x04);//scdt_inv_sel cs_vp_en
    WriteDAT_8(0x03);//CS1_WIDTH[7:0] 12
    //49
    //NV3031A-B
    WriteDAT_8(0x03);//CS2_WIDTH[7:0] 12
    WriteDAT_8(0x00);//PREC_START[7:0] 06
    WriteDAT_8(0x12);//PREC_WIDTH[7:0] 12
    WriteCOM(0xE8); //source
    WriteDAT_8(0x00); //VCMP_OUT_EN 81-vcmp/vref_output pad
    WriteDAT_8(0x70); //chopper_sel[6:4]
    WriteDAT_8(0x00); //gchopper_sel[6:4] 60
    ////gate
    WriteCOM(0xEc);
    WriteDAT_8(0x52);//47 43
    WriteCOM(0xF1);
    WriteDAT_8(0x01);//te_pol tem_extend 00 01 03
    WriteDAT_8(0x01);
    WriteDAT_8(0x02);
    WriteCOM(0xF6);
    WriteDAT_8(0x01);
    WriteDAT_8(0x30);//epf[1:0]
    WriteDAT_8(0x00);
    WriteDAT_8(0x00);//SPI2L: 40
    WriteCOM(0xfd);
    WriteDAT_8(0xfa);
    WriteDAT_8(0xfc);
    WriteCOM(0x3a);
    WriteDAT_8(0x55);//SH 0x66
    WriteCOM(0x35);
    WriteDAT_8(0x00);
    WriteCOM(0x36);//bgr_[3]
    WriteDAT_8(0x00);//c0
    WriteCOM(0x11); // exit sleep
    lcd_delay(200);
    WriteCOM(0x29); // display on
    lcd_delay(20);
}


static void NV3031A_lvgl_Fill(u16 xs, u16 xe, u16 ys, u16 ye, u8 *img)
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

static void NV3031A_led_ctrl(u8 status)
{
    //背光控制以及放在//lcd_te_driver.c 优化开机显示
    lcd_bl_pinstate(status);
}

void NV3031A_test(void)
{
    NV3031A_clear_screen(BLACK);
    lcd_delay(50);//每个屏幕可能会在刷新清屏了，但是屏幕芯片缓存还没刷到屏幕，需要加延时
    lcd_bl_pinstate(1);
}

int NV3031A_init(void)
{
    NV3031A_reset();
    printf("LCD_NV3031A init_start\n");
#if USE_LCD_TE
    lcd_te_interrupt_init(1);
#endif
    NV3031A_init_code();
    NV3031A_set_direction_default(ROTATE_DEGREE_90);
    NV3031A_test();
    printf("LCD_NV3031A config succes\n");
    return 0;
}


REGISTER_LCD_DEV(LCD_NV3031A) = {
    .name                   = "NV3031A",
    .lcd_width              = LCD_W,
    .lcd_height             = LCD_H,
    .color_format           = LCD_COLOR_RGB565,
    .column_addr_align      = 1,
    .row_addr_align         = 1,
    .LCD_Init               = NV3031A_init,
    .SetDrawArea            = NV3031A_SetRange,
    .LCD_Draw               = NV3031A_draw,
    .LCD_Draw_1             = NV3031A_draw_1,
    .LCD_DrawToDev          = NV3031A_Fill,//应用层直接到设备接口层，需要做好缓冲区共用互斥，慎用！
    .LCD_Lvgl_Full          = NV3031A_lvgl_Fill,//LVGL发送数据接口
    .LCD_ClearScreen        = NV3031A_clear_screen,
    .Reset                  = NV3031A_reset,
    .BackLightCtrl          = NV3031A_led_ctrl,
    .set_dirction           = NV3031A_set_direction,
    .set_dirction_default   = NV3031A_set_direction_default,
};

#endif


