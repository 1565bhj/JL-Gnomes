#include "app_config.h"
#include "system/includes.h"
#include "device/device.h"
#include "asm/spi.h"
#include "device/gpio.h"

#ifdef  USED_TM1629_SHOWN

#if (SXY_LL_YHH_BOARD)
#define TM_LED_BOARD_SHOWN_EN   0 //LED底板跑马灯使能
#define TM_SHOW_LEVEL_INIT  0x05
#elif (SXY_LL_XFX_BOARD)
#define TM_LED_BOARD_SHOWN_EN   1 //LED底板跑马灯使能
#define TM_SHOW_LEVEL_INIT  0x00
#else
#define TM_LED_BOARD_SHOWN_EN   0 //LED底板跑马灯使能
#define TM_SHOW_LEVEL_INIT  0x05
#endif

#define TM_TIME_TASK_NAME   "tm_time_task"
#define TM_DATA_CMD         0x44
#define TM_SHOW_ON_CMD      0x8F
#define TM_SHOW_OFF_CMD     0x80
#define TM_ADDR_CMD         0xC0
#define TM_SHOW_LEVEL_MAX   0x07
#define TM_SHOW_LEVEL_SETP  2//最大为2
#define TM_SHOW_ON_LEVEL0_CMD   0x88
#define TM_SHOW_F           0x71
#define TM_SHOW_U           0x3E
#define TM_SHOW_P           0x73

/*
0:s1-s7 = 0x3f
1:s1-s7 = 0x6
2:s1-s7 = 0x5b
3:s1-s7 = 0x4f
4:s1-s7 = 0x66
5:s1-s7 = 0x6d
6:s1-s7 = 0x7d
7:s1-s7 = 0x7
8:s1-s7 = 0x7f
9:s1-s7 = 0x6f

00:00
G4   G5    G8    G6    G7
0x6  0x8   0xe   0xa   0xc
*/
//0   1   2      3     4     5     6     7     8    9
static const unsigned char time_val[10] = {0x3f, 0x6, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x7, 0x7f, 0x6f};

static struct tm1629_info {
    void *spi_hdl;
    OS_MUTEX mutex;
    volatile unsigned char hour;
    volatile unsigned char min;
    volatile unsigned char sec;

    unsigned char led_board_breath;
    unsigned char led_g4s8_data;
    unsigned char led_board_cnt;
    unsigned short led_board_data;
    unsigned char bright_level;//亮度调节
    unsigned char bright_level_low_pwr;//低功耗亮度调节
    unsigned char face_smile_index;

    unsigned char ai;
    unsigned char bt;
    unsigned char tf;
    unsigned char update;
    unsigned char backlight;
    unsigned char face_smile_clear;

    unsigned char time_shown_off;
    unsigned char all_shown_off;
    unsigned char fft_shown_off;
    unsigned char shown_file;
    unsigned char shown_update;
    unsigned char shown_volume;

    unsigned char led_warm;
    unsigned char led_white;
    unsigned char led_g4s8;
    unsigned char led_board_mode;

    unsigned char led_board_last_mode;
    unsigned char led_board_bit;
} TM_INFO ALIGNED(4) = {0};

#define FFT_LED_H           5
#define FFT_LED_L           8
#define FFT_LED_ENERGY_MAX  FFT_LED_H

static struct tm1629_fft {
    unsigned char energy[FFT_LED_L];
} TM_FFT;

static const unsigned char fft_addr[FFT_LED_H]  = {0x04, 0x03, 0x02, 0x01, 0x00};

static const unsigned char fon_data[FFT_LED_H]  = {0x66, 0x99, 0xFF, 0x99, 0x66};
static const unsigned char foff_data[FFT_LED_H] = {0x00, 0x00, 0x99, 0xFF, 0x66};
static const unsigned char foff_data0[FFT_LED_H] = {0x00, 0x00, 0x00, 0x00, 0x00};

static const unsigned char fon_data1[FFT_LED_H]  = {0x18, 0x42, 0xA5, 0xA5, 0x42};
static const unsigned char fon_data2[FFT_LED_H]  = {0x3C, 0x18, 0x42, 0xE7, 0x42};
static const unsigned char fon_data3[FFT_LED_H]  = {0x24, 0x5A, 0x99, 0x99, 0x66};
static const unsigned char fon_data4[FFT_LED_H]  = {0x3C, 0x18, 0xE7, 0xA5, 0xFF};
static const unsigned char fon_data5[FFT_LED_H]  = {0x3C, 0x7E, 0xFF, 0xFF, 0x66};

//=================时间指令=====================//
static const char *tm_1629_time_open[] = {
    "打开时钟显示", "打开时间显示",
#ifdef CONFIG_KWS_ENGLISH
    "Turn on clock display", "Turn on time display",
#endif
    NULL,
};
static const char *tm_1629_time_close[] = {
    "关闭时钟显示", "关闭时间显示",
#ifdef CONFIG_KWS_ENGLISH
    "Turn off clock display", "Turn off time display",
#endif
    NULL,
};
static const char *tm_1629_shown_level_down[] = {
    "暗一点", "显示暗一点", "显示暗一些", "显示调暗一点", "显示调暗一些", "小夜灯暗一点",
#ifdef CONFIG_KWS_ENGLISH
    "Darker", "Display darker", "Display a bit darker", "Dim display a bit", "Dim display some", "Night light darker",
#endif
    NULL,
};
static const char *tm_1629_shown_level_up[] = {
    "亮一点", "显示亮一点", "显示亮一些", "显示调亮一点", "显示调亮一些", "小夜灯亮一点",
#ifdef CONFIG_KWS_ENGLISH
    "Brighter", "Display brighter", "Display a bit brighter", "Brighten display a bit", "Brighten display some", "Night light brighter",
#endif
    NULL,
};
static const char *tm_1629_shown_led_open[] = {
    "打开灯光", "开灯", "开启灯光", "打开小夜灯",
#ifdef CONFIG_KWS_ENGLISH
    "Turn on light", "Turn light on", "Enable light", "Turn on night light",
#endif
    NULL,
};
static const char *tm_1629_shown_led_close[] = {
    "关闭灯光", "关灯", "关闭小夜灯",
#ifdef CONFIG_KWS_ENGLISH
    "Turn off light", "Turn light off", "Turn off night light",
#endif
    NULL,
};
static const char *tm_1629_shown_led_breath_open[] = {
    "打开呼吸灯",
#ifdef CONFIG_KWS_ENGLISH
    "Turn on breath light",
#endif
    NULL,
};
static const char *tm_1629_shown_led_breath_close[] = {
    "关闭呼吸灯",
#ifdef CONFIG_KWS_ENGLISH
    "Turn off breath light",
#endif
    NULL,
};
static const char *tm_1629_shown_led_color[] = {
    "改变颜色", "颜色改变", "改变灯光颜色", "改变灯颜色",
    "切换颜色", "颜色切换", "切换灯光颜色", "切换灯颜色",
    "切换灯光",
#ifdef CONFIG_KWS_ENGLISH
    "Change color", "Color change", "Change light color", "Change lamp color",
    "Switch color", "Color switch", "Switch light color", "Switch lamp color",
    "Switch light",
#endif
    NULL,
};
static const char *tm_1629_shown_bottom_led_open[] = {
    "打开氛围灯", "打开脚底灯", "打开底座灯", "打开底座灯光",
#ifdef CONFIG_KWS_ENGLISH
    "Turn on ambient light", "Turn on foot light", "Turn on base light", "Turn on base lighting",
#endif
    NULL,
};
static const char *tm_1629_shown_bottom_led_close[] = {
    "关闭氛围灯", "关闭脚底灯", "关闭底座灯", "关闭底座灯光",
#ifdef CONFIG_KWS_ENGLISH
    "Turn off ambient light", "Turn off foot light", "Turn off base light", "Turn off base lighting",
#endif
    NULL,
};
enum {
    TM_1629_SHOWN_TIME = 0,//时间显示
    TM_1629_SHOWN_FREQ_SPEC,//频谱显示
    TM_1629_SHOWN_SMILE_FACE,//笑脸显示
    TM_1629_SHOWN_SMILE_FACE_CLEAR,//清空归零
    TM_1629_SHOWN_VOLUME,//音量显示
    TM_1629_SHOWN_FILE_NUM,//文件数量显示
    TM_1629_SHOWN_UPDATE,//升级进度条显示
    TM_1629_SHOWN_MODE,//模式显示
    TM_1629_SHOWN_LEVEL,//亮度等级式显示
    TM_1629_SHOWN_DATA_CLEAR,//清空配置的数据显示
    TM_1629_SHOWN_PD_TEST,//厂测测试
};
int led_eya_get_work(void);

static void tm_1629_shown_clear(void);
static void tm_1629_shown_smile_face_off(void);
void tm_1629_send_msg(int msg, int data);
void tm_1629_shown_face(int on);
void tm_1629_shown_led_auto(void);
int tm_1629_time_shown_level(char *word);
void tm_1629_shown_led_board_mode(char mode);
void tm_1629_shown_led_warm(char on);
void tm_1629_shown_led_white(char on);
int tm_1629_time_shown_board_led(char *word);

int tm_1629_time_shown(char *word)
{
    int close_type = 0;
    int i;

    for (i = 0; tm_1629_time_close[i] != NULL; i++) { //关闭时间显示
        if (strstr(word, tm_1629_time_close[i]) && strlen(word) <= strlen(tm_1629_time_close[i]) + 6) {
            close_type = 1;
            TM_INFO.time_shown_off = 1;
            puts("-> time_close\n");
            goto exit;
        }
    }
    for (i = 0; tm_1629_time_open[i] != NULL; i++) { //打开时间显示
        if (strstr(word, tm_1629_time_open[i]) && strlen(word) <= strlen(tm_1629_time_open[i]) + 6) {
            close_type = 1;
            TM_INFO.time_shown_off = 0;
            puts("-> time_open\n");
            goto exit;
        }
    }
    close_type = tm_1629_time_shown_level(word);
    if (!close_type) {
        close_type = tm_1629_time_shown_board_led(word);
    }
    return close_type;
exit:
    music_play_anser_OK();//好的
    music_play_waite();//等提示音播完了关闭设备
    return close_type;
}
void tm_1629_time_shown_bottom_led_ctrol(char open)
{
    if (open) {
        tm_1629_shown_led_board_mode(1);
    } else {
        tm_1629_shown_led_board_mode(0);
    }
}
int tm_1629_time_shown_board_led(char *word)
{
    int close_type = 0;
    int i;
#if (SXY_LL_XFX_BOARD)
    for (i = 0; tm_1629_shown_bottom_led_open[i] != NULL; i++) { //开启底座灯
        if (strstr(word, tm_1629_shown_bottom_led_open[i]) && strlen(word) <= strlen(tm_1629_shown_bottom_led_open[i]) + 6) {
            tm_1629_time_shown_bottom_led_ctrol(1);
            close_type = 1;
            puts("-> led_close\n");
            goto exit;
        }
    }
    for (i = 0; tm_1629_shown_bottom_led_close[i] != NULL; i++) { //关闭底座灯
        if (strstr(word, tm_1629_shown_bottom_led_close[i]) && strlen(word) <= strlen(tm_1629_shown_bottom_led_close[i]) + 6) {
            tm_1629_time_shown_bottom_led_ctrol(0);
            close_type = 1;
            puts("-> led_open\n");
            goto exit;
        }
    }
#endif
    for (i = 0; tm_1629_shown_led_open[i] != NULL; i++) { //开启灯光
        if (strstr(word, tm_1629_shown_led_open[i]) && strlen(word) <= strlen(tm_1629_shown_led_open[i]) + 6) {
#if (SXY_LL_YHH_BOARD)
#ifdef TCFG_LED_PWM0_PORT
            tm_light_open(0);
            close_type = 1;
#endif
#elif (SXY_LL_XFX_BOARD)
            tm_1629_shown_led_warm(1);
            tm_1629_shown_led_white(0);
            close_type = 1;
#endif
            puts("-> led_open\n");
            goto exit;
        }
    }
    for (i = 0; tm_1629_shown_led_close[i] != NULL; i++) { //关闭灯光
        if (strstr(word, tm_1629_shown_led_close[i]) && strlen(word) <= strlen(tm_1629_shown_led_close[i]) + 6) {
#if (SXY_LL_YHH_BOARD)
#ifdef TCFG_LED_PWM0_PORT
            tm_light_close(0);
            close_type = 1;
#endif
#elif (SXY_LL_XFX_BOARD)
            tm_1629_shown_led_warm(0);
            tm_1629_shown_led_white(0);
            close_type = 1;
#endif
            puts("-> led_close\n");
            goto exit;
        }
    }
    for (i = 0; tm_1629_shown_led_breath_open[i] != NULL; i++) { //打开呼吸灯
        if (strstr(word, tm_1629_shown_led_breath_open[i]) && strlen(word) <= strlen(tm_1629_shown_led_breath_open[i]) + 6) {
#if (SXY_LL_YHH_BOARD)
#ifdef TCFG_LED_PWM0_PORT
            tm_light_breath(1, 0);
            close_type = 1;
#endif
#endif
            puts("-> led_breath open\n");
            goto exit;
        }
    }
    for (i = 0; tm_1629_shown_led_breath_close[i] != NULL; i++) { //关闭呼吸灯
        if (strstr(word, tm_1629_shown_led_breath_close[i]) && strlen(word) <= strlen(tm_1629_shown_led_breath_close[i]) + 6) {
#if (SXY_LL_YHH_BOARD)
#ifdef TCFG_LED_PWM0_PORT
            tm_light_breath(0, 0);
            close_type = 1;
#endif
#endif
            puts("-> led_breath close\n");
            goto exit;
        }
    }
#if (SXY_LL_XFX_BOARD)
    for (i = 0; tm_1629_shown_led_color[i] != NULL; i++) { //灯光颜色
        if (strstr(word, tm_1629_shown_led_color[i]) && strlen(word) <= strlen(tm_1629_shown_led_color[i]) + 6) {
            tm_1629_shown_led_auto();
            close_type = 1;
            puts("-> led color\n");
            goto exit;
        }
    }
#endif
    return close_type;
exit:
    music_play_anser_OK();//好的
    music_play_waite();//等提示音播完了关闭设备
    return close_type;
}
int tm_1629_time_shown_level_low_power(char enter_low_pwr)
{
    TM_INFO.bright_level_low_pwr = enter_low_pwr ? 1 : 0;
    return 0;
}
int tm_1629_time_shown_level(char *word)
{
    int close_type = 0;
    int anser_OK = 1;
    int i;

    for (i = 0; tm_1629_shown_level_down[i] != NULL; i++) { //显示亮度减
        if (strstr(word, tm_1629_shown_level_down[i]) && strlen(word) <= strlen(tm_1629_shown_level_down[i]) + 6) {
            close_type = 1;
            if (TM_INFO.bright_level == 0) {
                anser_OK = 0;
                music_play_res_file("LowLight.mp3");
            }
            if ((int)TM_INFO.bright_level - TM_SHOW_LEVEL_SETP < 0) {
                TM_INFO.bright_level = 0;
            } else {
                TM_INFO.bright_level -= TM_SHOW_LEVEL_SETP;
            }
            user_data_bright_level_write(TM_INFO.bright_level);
            puts("-> bright_level down\n");
            goto exit;
        }
    }
    for (i = 0; tm_1629_shown_level_up[i] != NULL; i++) { //显示亮度加
        if (strstr(word, tm_1629_shown_level_up[i]) && strlen(word) <= strlen(tm_1629_shown_level_up[i]) + 6) {
            close_type = 1;
            int bl_max = ADDR_ALIGNE_BACK(TM_SHOW_LEVEL_MAX, TM_SHOW_LEVEL_SETP);
            if (TM_INFO.bright_level >= bl_max) {
                anser_OK = 0;
                music_play_res_file("HightLight.mp3");
            }
            TM_INFO.bright_level += TM_SHOW_LEVEL_SETP;
            TM_INFO.bright_level = TM_INFO.bright_level > bl_max ? bl_max : TM_INFO.bright_level;
            user_data_bright_level_write(TM_INFO.bright_level);
            puts("-> bright_level up\n");
            goto exit;
        }
    }
    return close_type;
exit:
    if (anser_OK) {
        music_play_anser_OK();//好的
    }
    music_play_waite();//等提示音播完了关闭设备
    return close_type;
}
int tm_1629_led_shown_bright_level_auto(void)
{
    int level = user_data_bright_level_read();//亮度等级
    if (level < 0 || level > TM_SHOW_LEVEL_MAX) {
        level = TM_SHOW_LEVEL_INIT;
        user_data_bright_level_write(level);
        TM_INFO.bright_level = level;
    } else {
        if (TM_INFO.fft_shown_off && TM_INFO.time_shown_off) {
            TM_INFO.bright_level = 0;
            TM_INFO.fft_shown_off = 0;
            TM_INFO.time_shown_off = 0;
        } else {
            TM_INFO.bright_level += TM_SHOW_LEVEL_SETP;
        }
        if (TM_INFO.bright_level > TM_SHOW_LEVEL_MAX) {
            TM_INFO.bright_level = 0;
            TM_INFO.fft_shown_off = 1;
            TM_INFO.time_shown_off = 1;
            //TM_INFO.all_shown_off = 1;//关闭所有显示
        } else {
            int bl_max = ADDR_ALIGNE_BACK(TM_SHOW_LEVEL_MAX, TM_SHOW_LEVEL_SETP);
            TM_INFO.bright_level = TM_INFO.bright_level > bl_max ? 0 : TM_INFO.bright_level;
            TM_INFO.fft_shown_off = 0;
            TM_INFO.time_shown_off = 0;
            //TM_INFO.all_shown_off = 0;
        }
        user_data_bright_level_write(TM_INFO.bright_level);
    }
    tm_1629_time_shown_level_low_power(0);
    if (TM_INFO.fft_shown_off) {
        tm_1629_shown_smile_face_off();
        tm_1629_send_msg(TM_1629_SHOWN_DATA_CLEAR, 0);
    } else {
        tm_1629_shown_face(0);
        tm_1629_send_msg(TM_1629_SHOWN_LEVEL, (TM_INFO.bright_level + 1) / TM_SHOW_LEVEL_SETP + 1);
    }
    printf("-> TM_INFO.bright_level = %d\n", TM_INFO.bright_level);
}
static int tm_1629_spi_init(void)
{
    /* 使用方法1：单次指定接收固定数据量。
    注意：此方法spi接收数据比较慢，因此对方主机的spi连续发送字节不能过快，否则丢数据（当数对方主机spi数据过快，请使用上述：用户指定接收块方法）
    */
    //1.打开spi设备
    gpio_direction_output(TCFG_TM1629_SPI_CS, 1);
    TM_INFO.spi_hdl = dev_open("spi2", NULL);
    if (!TM_INFO.spi_hdl) {
        printf("spi open err \n");
        return -1;
    }
    dev_ioctl(TM_INFO.spi_hdl, IOCTL_SPI_SET_IRQ_CPU_ID, (u32)1);//可以指定中断到核1执行
    dev_ioctl(TM_INFO.spi_hdl, IOCTL_SPI_SET_USE_SEM, 0);//等待数据时，用信号量等待，不用则为抢占查询硬件中断标记，建议应信号量等待
    return 0;
}
void tm_1629_spi_exit(void)
{
    if (TM_INFO.spi_hdl) {
        os_mutex_pend(&TM_INFO.mutex, 1000);
        tm_1629_shown_clear();
        dev_close(TM_INFO.spi_hdl);
        TM_INFO.spi_hdl = NULL;
        os_mutex_post(&TM_INFO.mutex);
    }
}
static char msb2lsb(char data)
{
    char pdat = 0;
    for (char i = 0; i < 8; i++) {
        pdat <<= 1;
        pdat |= (data & BIT(i)) ? 0x1 : 0;
    }
    return pdat;
}
static void tm_1629_shown_write_one_byte(char addr, char data)
{
    os_mutex_pend(&TM_INFO.mutex, 1000);
    if (TM_INFO.spi_hdl) {
        if (TM_INFO.all_shown_off) {
            data = 0;
        }
        gpio_direction_output(TCFG_TM1629_SPI_CS, 0);
        dev_ioctl(TM_INFO.spi_hdl, IOCTL_SPI_SEND_BYTE, msb2lsb(TM_DATA_CMD));//发送一个字节
        gpio_direction_output(TCFG_TM1629_SPI_CS, 1);
        delay(10);
        gpio_direction_output(TCFG_TM1629_SPI_CS, 0);
        dev_ioctl(TM_INFO.spi_hdl, IOCTL_SPI_SEND_BYTE, msb2lsb(TM_ADDR_CMD | addr));//发送一个字节
        dev_ioctl(TM_INFO.spi_hdl, IOCTL_SPI_SEND_BYTE, msb2lsb(data));//发送一个字节
        delay(10);
        gpio_direction_output(TCFG_TM1629_SPI_CS, 1);
        delay(10);
        gpio_direction_output(TCFG_TM1629_SPI_CS, 0);
        //dev_ioctl(TM_INFO.spi_hdl, IOCTL_SPI_SEND_BYTE, msb2lsb(TM_SHOW_ON_CMD));//发送一个字节
        if (TM_INFO.bright_level_low_pwr) {
            dev_ioctl(TM_INFO.spi_hdl, IOCTL_SPI_SEND_BYTE, msb2lsb(TM_SHOW_ON_LEVEL0_CMD | 0));//发送一个字节
        } else {
            dev_ioctl(TM_INFO.spi_hdl, IOCTL_SPI_SEND_BYTE, msb2lsb(TM_SHOW_ON_LEVEL0_CMD | TM_INFO.bright_level));//发送一个字节
        }
        gpio_direction_output(TCFG_TM1629_SPI_CS, 1);
        delay(10);
    }
    os_mutex_post(&TM_INFO.mutex);
}

static void tm_1629_shown_time(unsigned char hour, unsigned char min)
{
    unsigned char h_h = hour / 10;
    unsigned char h_l = hour % 10;
    unsigned char m_h = min / 10;
    unsigned char m_l = min % 10;
    unsigned char led_val = TM_INFO.led_warm ?  BIT(7) : 0;
    unsigned char led_val_w = TM_INFO.led_white ?  BIT(0) : 0;
    unsigned char led_g4s8 = TM_INFO.led_g4s8 ? BIT(7) : 0;
    TM_INFO.led_g4s8_data = time_val[h_h];

    if (TM_INFO.shown_file) {
        tm_1629_shown_write_one_byte(0x6, TM_SHOW_F | led_g4s8);
    } else if (TM_INFO.shown_update) {
        tm_1629_shown_write_one_byte(0x6, TM_SHOW_U | led_g4s8);
    } else {
        tm_1629_shown_write_one_byte(0x6, time_val[h_h] | led_g4s8);
    }
    if (TM_INFO.shown_update && h_l == 0) {
        tm_1629_shown_write_one_byte(0x8, TM_SHOW_P);
    } else {
        tm_1629_shown_write_one_byte(0x8, time_val[h_l]);
    }
    tm_1629_shown_write_one_byte(0xa, time_val[m_h]);
    tm_1629_shown_write_one_byte(0xc, time_val[m_l] | led_val);
    tm_1629_shown_write_one_byte(0xd, led_val_w);
    tm_1629_shown_write_one_byte(0xf, led_val_w);
}
static void tm_1629_shown_point(char on)
{
    char dat = 0;//point
    //printf("ai %d %d %d \n",TM_INFO.ai,TM_INFO.bt,TM_INFO.tf);
    if (on) {
        dat = 0x18;//point
        dat |= TM_INFO.ai ?  BIT(0) : 0;
        dat |= TM_INFO.bt ?  BIT(1) : 0;
        dat |= TM_INFO.tf ?  BIT(2) : 0;
        dat |= TM_INFO.led_warm ?  BIT(7) : 0;
        tm_1629_shown_write_one_byte(0xe, dat);
    } else {
        dat |= TM_INFO.ai ? BIT(0) : 0;
        dat |= TM_INFO.bt ? BIT(1) : 0;
        dat |= TM_INFO.tf ? BIT(2) : 0;
        dat |= TM_INFO.led_warm ?  BIT(7) : 0;
        tm_1629_shown_write_one_byte(0xe, dat);
    }
}
static void tm_1629_shown_board_led(void)
{
    int dat = 0;
    char led_g4s8;
    switch (TM_INFO.led_board_mode) {
    case 0://关闭
        if (TM_INFO.led_board_last_mode == TM_INFO.led_board_mode) {
            break;
        }
        TM_INFO.led_board_last_mode = TM_INFO.led_board_mode;
        TM_INFO.led_board_data = 0;
        led_g4s8 = 0;
        TM_INFO.led_g4s8 = 0;
        if (TM_INFO.shown_file) {
            tm_1629_shown_write_one_byte(0x6, TM_SHOW_F | led_g4s8);
        } else if (TM_INFO.shown_update) {
            tm_1629_shown_write_one_byte(0x6, TM_SHOW_U | led_g4s8);
        } else {
            tm_1629_shown_write_one_byte(0x6, TM_INFO.led_g4s8_data | led_g4s8);
        }
        tm_1629_shown_write_one_byte(0x7, 0);
        tm_1629_shown_write_one_byte(0x5, 0);
        break;
    case 1://跑马
        TM_INFO.led_board_last_mode = TM_INFO.led_board_mode;
        TM_INFO.led_board_cnt++;
        if (TM_INFO.led_board_cnt & 0x1) {
            break;
        }
        if (!TM_INFO.led_board_data) {
            TM_INFO.led_board_data = 0x3ff;
        } else {
            TM_INFO.led_board_data = 0x3ff & (~(1 << TM_INFO.led_board_bit));
        }

        led_g4s8 = (TM_INFO.led_board_data & BIT(0)) ? BIT(7) : 0;
        TM_INFO.led_g4s8 = led_g4s8 ? 1 : 0;

        if (TM_INFO.shown_file) {
            tm_1629_shown_write_one_byte(0x6, TM_SHOW_F | led_g4s8);
        } else if (TM_INFO.shown_update) {
            tm_1629_shown_write_one_byte(0x6, TM_SHOW_U | led_g4s8);
        } else {
            tm_1629_shown_write_one_byte(0x6, TM_INFO.led_g4s8_data | led_g4s8);
        }
        char led_g4s9s13 = (TM_INFO.led_board_data >> 1) & 0x1F;
        char led_g3s9s12 = (TM_INFO.led_board_data >> 6) & 0xF;

#if 0
        char tmp = led_g3s9s12;//LED不按顺序接法
        led_g3s9s12 = 0;
        for (int i = 0; i < 4; i++) {
            if (i == 0) {
                led_g3s9s12 |= (tmp & BIT(0)) ? BIT(3) : 0;
            } else if (i == 1) {
                led_g3s9s12 |= (tmp & BIT(1)) ? BIT(0) : 0;
            } else if (i == 2) {
                led_g3s9s12 |= (tmp & BIT(2)) ? BIT(1) : 0;
            } else if (i == 3) {
                led_g3s9s12 |= (tmp & BIT(3)) ? BIT(2) : 0;
            }
        }
#endif

        tm_1629_shown_write_one_byte(0x5, led_g3s9s12);//G3
        tm_1629_shown_write_one_byte(0x7, led_g4s9s13);//G4

        if (++TM_INFO.led_board_bit >= 10) {
            TM_INFO.led_board_bit = 0;
        }
        break;
    case 2://闪烁
        TM_INFO.led_board_last_mode = TM_INFO.led_board_mode;
        if (TM_INFO.led_board_breath % 10 == 0) {
            led_g4s8 = BIT(7);
            TM_INFO.led_g4s8 = 1;
            if (TM_INFO.shown_file) {
                tm_1629_shown_write_one_byte(0x6, TM_SHOW_F | led_g4s8);
            } else if (TM_INFO.shown_update) {
                tm_1629_shown_write_one_byte(0x6, TM_SHOW_U | led_g4s8);
            } else {
                tm_1629_shown_write_one_byte(0x6, TM_INFO.led_g4s8_data | led_g4s8);
            }
            tm_1629_shown_write_one_byte(0x5, 0x0F);//G3
            tm_1629_shown_write_one_byte(0x7, 0x1F);//G4
        } else if (TM_INFO.led_board_breath % 5 == 0) {
            led_g4s8 = 0;
            TM_INFO.led_g4s8 = 0;
            if (TM_INFO.shown_file) {
                tm_1629_shown_write_one_byte(0x6, TM_SHOW_F | led_g4s8);
            } else if (TM_INFO.shown_update) {
                tm_1629_shown_write_one_byte(0x6, TM_SHOW_U | led_g4s8);
            } else {
                tm_1629_shown_write_one_byte(0x6, TM_INFO.led_g4s8_data | led_g4s8);
            }
            tm_1629_shown_write_one_byte(0x5, 0x00);//G3
            tm_1629_shown_write_one_byte(0x7, 0x00);//G4
        }
        TM_INFO.led_board_breath++;
        break;
    }
}
static void tm_1629_shown_time_off(void)
{
    char led_val = TM_INFO.led_warm ?  BIT(7) : 0;
    char led_val_w = TM_INFO.led_white ?  BIT(0) : 0;
    char led_g4s8 = TM_INFO.led_g4s8 ? BIT(7) : 0;
    TM_INFO.led_g4s8_data = 0;
    if (TM_INFO.shown_file) {
        tm_1629_shown_write_one_byte(0x6, TM_SHOW_F | led_g4s8);
    } else if (TM_INFO.shown_update) {
        tm_1629_shown_write_one_byte(0x6, TM_SHOW_U | led_g4s8);
    } else {
        tm_1629_shown_write_one_byte(0x6, 0 | led_g4s8);
    }
    tm_1629_shown_write_one_byte(0x8, 0);
    tm_1629_shown_write_one_byte(0xa, 0);
    tm_1629_shown_write_one_byte(0xc, 0 | led_val);
    tm_1629_shown_write_one_byte(0xe, 0 | led_val);
    tm_1629_shown_write_one_byte(0xd, led_val_w);
    tm_1629_shown_write_one_byte(0xf, led_val_w);
    tm_1629_shown_point(0);
}
void tm_1629_time_check(int hour, int min, int sec)
{
    TM_INFO.hour = (unsigned char)hour;
    TM_INFO.min = (unsigned char)min;
    TM_INFO.sec = (unsigned char)sec;
}
void tm_1629_shown_ai(char on)
{
    TM_INFO.ai = on ? 1 : 0;
    tm_1629_send_msg(TM_1629_SHOWN_MODE, 0);
}
void tm_1629_shown_bt(char on)
{
    TM_INFO.bt = on ? 1 : 0;
    tm_1629_send_msg(TM_1629_SHOWN_MODE, 0);
}
void tm_1629_shown_tf(char on)
{
    TM_INFO.tf = on ? 1 : 0;
    tm_1629_send_msg(TM_1629_SHOWN_MODE, 0);
}
void tm_1629_shown_time_update(void)
{
    TM_INFO.update = 1;
}
void tm_1629_shown_time_update_rtc(void)
{
    TM_INFO.update = 2;
}
void tm_1629_shown_led_warm(char on)
{
    TM_INFO.led_warm = on ? 1 : 0;
}
void tm_1629_shown_led_white(char on)
{
    TM_INFO.led_white = on ? 1 : 0;
}
void tm_1629_shown_led_auto(void)
{
    if (!TM_INFO.led_warm && !TM_INFO.led_white) {
        TM_INFO.led_warm = 1;
    } else if (TM_INFO.led_warm) {
        TM_INFO.led_warm = 0;
        TM_INFO.led_white = 1;
    } else {
        TM_INFO.led_warm = 1;
        TM_INFO.led_white = 0;
    }
}
void tm_1629_shown_led_board_mode(char mode)
{
    TM_INFO.led_board_mode = (mode & 0x3);
}

void tm_1629_send_msg(int msg, int data)
{
    os_taskq_post(TM_TIME_TASK_NAME, 2, msg, data);
}
static void tm_1629_shown_init(void)
{
    for (char i = 0; i <= 0xf; i++) {
        tm_1629_shown_write_one_byte(i, 0);
    }
    int level = user_data_bright_level_read();//亮度等级
    if (level < 0 || level > TM_SHOW_LEVEL_MAX) {
        level = TM_SHOW_LEVEL_INIT;
        user_data_bright_level_write(level);
    }
    TM_INFO.bright_level = level;
}
static void tm_1629_shown_clear(void)
{
    for (char n = 0; n < 5; n++) {
        for (char i = 0; i <= 0xf; i++) {
            tm_1629_shown_write_one_byte(i, 0);
        }
    }
}
static void tm_1629_shown_check(void)
{
    for (char i = 0; i <= 0xf; i++) {
        tm_1629_shown_write_one_byte(i, 0xFF);
    }
}
static int one_sec_time_callback(void *p)
{
    tm_1629_send_msg(TM_1629_SHOWN_TIME, 0);
    return 0;
}
static void tm_1629_shown_fft(void)
{
    unsigned char data[FFT_LED_H] = {0};
    for (char j = 0; j < FFT_LED_H; j++) {
        for (char i = 0; i < FFT_LED_L; i++) {
            if (j <= TM_FFT.energy[i] && TM_FFT.energy[i] != 0) {
                data[j] |= BIT(i);
            }
        }
    }
    for (char i = 0; i < FFT_LED_H; i++) {
        tm_1629_shown_write_one_byte(fft_addr[i], data[i]);//g1 s1->18
    }
}
static void tm_1629_shown_smile_face_sleep(void)
{
    for (char i = 0; i < FFT_LED_H; i++) {
        tm_1629_shown_write_one_byte(fft_addr[i], foff_data[i]);//g1 s1->18
    }
}
static void tm_1629_shown_smile_face(char on)
{
    unsigned char *fdata = on == 1 ? fon_data : foff_data;

    //1-眨眼
//    TM_INFO.face_smile_index++;
//    if(!TM_INFO.face_smile_index){
//        TM_INFO.face_smile_index = rand()%6 + 1;
//    }
    if (TM_INFO.face_smile_clear && on) {
        TM_INFO.face_smile_clear = 0;
    }
    switch (TM_INFO.face_smile_index) {
    case 1:
        fdata = on == 1 ? fon_data : foff_data;
        break;
    case 2:
        fdata = on == 1 ? fon_data1 : foff_data;
        break;
    case 3:
        fdata = on == 1 ? fon_data2 : foff_data;
        break;
    case 4:
        fdata = on == 1 ? fon_data3 : foff_data;
        break;
    case 5:
        fdata = on == 1 ? fon_data4 : foff_data;
        break;
    case 6:
        fdata = on == 1 ? fon_data5 : foff_data;
        break;
    }
    for (char i = 0; i < FFT_LED_H; i++) {
        tm_1629_shown_write_one_byte(fft_addr[i], fdata[i]);//g1 s1->18
    }
}
static void tm_1629_shown_smile_face_off(void)
{
    for (char i = 0; i < FFT_LED_H; i++) {
        tm_1629_shown_write_one_byte(fft_addr[i], 0);//g1 s1->18
    }
}
static void tm_1629_shown_smile_face_clear(void)
{
    //TM_INFO.face_smile_index = 0;
    TM_INFO.face_smile_index = rand() % 6 + 1;
    if (TM_INFO.face_smile_index > 6) {
        TM_INFO.face_smile_index = 1;
    }
    TM_INFO.face_smile_clear = 1;
    //tm_1629_shown_smile_face_sleep();
}
void tm_1629_shown_fft_db(unsigned char *energy)
{
    memcpy(&TM_FFT.energy, energy, FFT_LED_L);
    //tm_1629_send_msg(TM_1629_SHOWN_FREQ_SPEC);
}
void tm_1629_shown_face(int on)
{
    tm_1629_send_msg(TM_1629_SHOWN_SMILE_FACE, on);
}
void tm_1629_shown_face_clear(void)
{
    tm_1629_send_msg(TM_1629_SHOWN_SMILE_FACE_CLEAR, 0);
}
void tm_1629_shown_set_volume(int vol)
{
    tm_1629_send_msg(TM_1629_SHOWN_VOLUME, vol);
}
void tm_1629_shown_set_dir_file_num(int fnum, char is_file)
{
    if (!is_file) {
        TM_INFO.shown_file = 1;
    } else {
        TM_INFO.shown_file = 0;
    }
    tm_1629_send_msg(TM_1629_SHOWN_FILE_NUM, fnum);
}
void tm_1629_shown_update_percent(int percent)
{
    if ((int)percent != (int)TM_INFO.shown_update) {
        TM_INFO.shown_update = percent;
        tm_1629_send_msg(TM_1629_SHOWN_UPDATE, percent);
    }
}
void tm_1629_shown_pd_test(void)
{
    tm_1629_send_msg(TM_1629_SHOWN_PD_TEST, 0);
}
short *net_music_get_audio_fft(int *len);
static int tm_time_task(void*p)
{
    char point = 0;
    int err, res;
    int msg[4];
    short *fft_db;
    int fft_freq_num = 0;
    int cnt = 0;
    int offset = 0;
    int cpy = FFT_LED_L;
    int face_on = 0;
    int volume = 0;
    int fnum = 0;
    int face_waite = 0;
    int no_fft_check = 0;
    char shown_data = 0;
    char shown_dp_test = 0;
    char dp_test_ai_back = 0;
    char dp_test_bt_back = 0;
    char dp_test_tf_back = 0;
    char temp;
    struct sys_time time = {0};

    os_mutex_create(&TM_INFO.mutex);
#if 0
    void *rtc_hdl = dev_open("rtc", NULL);
    if (rtc_hdl) {
        dev_ioctl(rtc_hdl, IOCTL_GET_SYS_TIME, (u32)&time);/* 获取时间信息 */
        dev_close(rtc_hdl);
    }
    printf("rtc time %02d:%02d:%02d\n", time.hour, time.min, time.sec);
    tm_1629_time_check(time.hour, time.min, time.sec);
#endif
    gpio_direction_output(TCFG_TM1629_POWER_PORT, 1);
    os_time_dly(10);
    tm_1629_spi_init();
    tm_1629_shown_init();
    tm_1629_shown_check();
    os_time_dly(200);
    tm_1629_shown_init();
    os_time_dly(50);
    tm_1629_shown_smile_face(0);
#if TM_LED_BOARD_SHOWN_EN
    tm_1629_shown_led_auto();
#endif
    sys_hi_timer_add(NULL, one_sec_time_callback, 1000);
    while (1) {
        //res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        res = os_taskq_accept(ARRAY_SIZE(msg), msg);
        if (res == OS_TASK_DEL_IDLE) {
            break;
        } else if (res == OS_TASKQ) {
            if (msg[0] == Q_USER) {
                face_on = msg[2];
                switch (msg[1]) {
                case TM_1629_SHOWN_TIME:
                    TM_INFO.sec++;
                    if (TM_INFO.sec >= 60) {
                        TM_INFO.sec = 0;
                        TM_INFO.min++;
                        if (TM_INFO.min >= 60) {
                            TM_INFO.min = 0;
                            TM_INFO.hour++;
                            if (TM_INFO.hour >= 24) {
                                TM_INFO.hour = 0;
                            }
                        }
                    }
                    memset(&time, 0, sizeof(struct sys_time));
                    if (TM_INFO.update == 1) {
                        if (utc_timer_update_get(&time) == 0) {
                            tm_1629_time_check(time.hour, time.min, time.sec);
                            TM_INFO.update = 0;
                        }
                    } else if (TM_INFO.update == 2) {
                        void *rtc_hdl = dev_open("rtc", NULL);
                        if (rtc_hdl) {
                            dev_ioctl(rtc_hdl, IOCTL_GET_SYS_TIME, (u32)&time);/* 获取时间信息 */
                            dev_close(rtc_hdl);
                        }
                        printf("rtc time %02d:%02d:%02d\n", time.hour, time.min, time.sec);
                        tm_1629_time_check(time.hour, time.min, time.sec);
                        TM_INFO.update = 0;
                    }
                    if (shown_data || shown_dp_test || TM_INFO.time_shown_off || TM_INFO.spi_hdl == NULL) {
                        break;
                    }
                    tm_1629_shown_time(TM_INFO.hour, TM_INFO.min);
                    tm_1629_shown_point(++point & 0x1);
                    break;
                case TM_1629_SHOWN_FREQ_SPEC:
                    break;
                case TM_1629_SHOWN_SMILE_FACE:
                    if (TM_INFO.fft_shown_off) {
                        tm_1629_shown_smile_face_off();
                    } else {
                        tm_1629_shown_smile_face(face_on);
                    }
                    break;
                case TM_1629_SHOWN_SMILE_FACE_CLEAR:
                    tm_1629_shown_smile_face_clear();
                    break;
                case TM_1629_SHOWN_VOLUME:
                    TM_INFO.shown_file = 0;
                    TM_INFO.shown_volume = 1;
                    point = 0;
                    shown_data = 1;
                    volume = msg[2];
                    tm_1629_shown_time(volume / 100, volume % 100); //千百十个位
                    tm_1629_shown_point(0);
                    break;
                case TM_1629_SHOWN_FILE_NUM:
                    TM_INFO.shown_volume = 0;
                    point = 0;
                    shown_data = 1;
                    fnum = msg[2];
                    tm_1629_shown_time((fnum / 100) % 10, fnum % 100);//百十个位
                    tm_1629_shown_point(0);
                    break;
                case TM_1629_SHOWN_UPDATE:
                    point = 0;
                    shown_data = 1;
                    //TM_INFO.shown_update = msg[2];
                    tm_1629_shown_time((TM_INFO.shown_update / 100) % 10, TM_INFO.shown_update % 100);//百十个位
                    tm_1629_shown_point(0);
                    break;
                case TM_1629_SHOWN_LEVEL:
                    TM_INFO.shown_volume = 0;
                    TM_INFO.shown_file = 0;
                    point = 0;
                    shown_data = 1;
                    fnum = msg[2];
                    tm_1629_shown_time((fnum / 100) % 10, fnum % 100);//百十个位
                    tm_1629_shown_point(0);
                    break;
                case TM_1629_SHOWN_MODE:
                    tm_1629_shown_point(0);
                    break;
                case TM_1629_SHOWN_DATA_CLEAR:
                    shown_data = 0;
                    if (TM_INFO.shown_file) {
                        TM_INFO.shown_file = 0;
                    }
                    if (TM_INFO.shown_volume) {
                        TM_INFO.shown_volume = 0;
                    }
                    if (TM_INFO.time_shown_off) {
                        tm_1629_shown_time_off();
                    } else {
                        tm_1629_shown_time(TM_INFO.hour, TM_INFO.min);
                        tm_1629_shown_point(++point & 0x1);
                    }
                    if (TM_INFO.fft_shown_off) {
                        tm_1629_shown_smile_face_off();
                    }
                    break;
                case TM_1629_SHOWN_PD_TEST:
                    TM_INFO.face_smile_index = 1;
                    shown_dp_test = 1;
                    cnt = 0;
                    dp_test_ai_back = TM_INFO.ai;
                    dp_test_bt_back = TM_INFO.bt;
                    dp_test_tf_back = TM_INFO.tf;
                    printf("TM_1629_SHOWN_PD_TEST\n");
                    break;
                }
            }
        }
        os_time_dly(10);
        if (TM_INFO.spi_hdl == NULL) {
            continue;
        }
        if (shown_data) {
            if (++shown_data > 30) {
                shown_data = 0;
                TM_INFO.shown_file = 0;
                TM_INFO.shown_update = 0;
                TM_INFO.shown_volume = 0;
                if (TM_INFO.time_shown_off) {
                    tm_1629_shown_time_off();
                } else {
                    tm_1629_shown_time(TM_INFO.hour, TM_INFO.min);
                    tm_1629_shown_point(++point & 0x1);
                }
                if (TM_INFO.fft_shown_off) {
                    tm_1629_shown_smile_face_off();
                }
            }
        } else if (shown_dp_test) { //厂测模式下显示
            if (++cnt % 5 == 0) {
                if (shown_dp_test > 30) {
                    shown_dp_test = 0;
                    TM_INFO.ai = dp_test_ai_back;
                    TM_INFO.bt = dp_test_bt_back;
                    TM_INFO.tf = dp_test_tf_back;
                    if (TM_INFO.time_shown_off) {
                        tm_1629_shown_time_off();
                    }
                    tm_1629_shown_smile_face(0);
                } else {
                    TM_INFO.ai = (shown_dp_test % 3) == 0 ? 1 : 0;
                    TM_INFO.bt = (shown_dp_test % 3) == 1 ? 1 : 0;
                    TM_INFO.tf = (shown_dp_test % 3) == 2 ? 1 : 0;
                    tm_1629_shown_smile_face(1);
                    tm_1629_shown_time((shown_dp_test - 1) % 10 * 11, (shown_dp_test - 1) % 10 * 11);
                    tm_1629_shown_point(shown_dp_test & 0x1);
                    if (++TM_INFO.face_smile_index > 6) {
                        TM_INFO.face_smile_index = 1;
                    }
                    shown_dp_test++;
                }
            }
            continue;
        } else if (TM_INFO.time_shown_off == 1 || TM_INFO.time_shown_off == 2) { //两次关闭时间显示
            tm_1629_shown_time_off();
            TM_INFO.time_shown_off++;
            if (TM_INFO.fft_shown_off) {
                tm_1629_shown_smile_face_off();
            }
        }
#if TM_LED_BOARD_SHOWN_EN
        tm_1629_shown_board_led();
#endif
        if (led_eya_get_work() || TM_INFO.all_shown_off || TM_INFO.fft_shown_off) {
            continue;
        }
        fft_db = net_music_get_audio_fft(&fft_freq_num);
        if (fft_db) {
get_fft_data:
#define FFT_ENERGY_MAX      64
            int max_energy = FFT_LED_ENERGY_MAX;
            int energy_step = FFT_ENERGY_MAX / FFT_LED_H;
            int fft_group = fft_freq_num / FFT_LED_L;
            int max_eng = 0, sum_eng = 0;
            unsigned char energy_num_data[FFT_LED_L] = {0};
            static unsigned char energy_num[FFT_LED_L] = {0};
            static unsigned char energy_num_back[FFT_LED_L] = {0};

#if 1
            //流水频谱
            if (cpy == FFT_LED_L || cpy == FFT_LED_L - 1) {
                memcpy(energy_num_back, energy_num, FFT_LED_L);
                for (int i = 0; i < FFT_LED_L; i++) {
                    max_eng = 0;
                    sum_eng = 0;
                    for (int j = 0; j < fft_group; j++) {
                        sum_eng += fft_db[i * FFT_LED_L + j];
                        if (max_eng < fft_db[i * FFT_LED_L + j]) {
                            max_eng = fft_db[i * FFT_LED_L + j];
                        }
                        if (max_eng > FFT_ENERGY_MAX) {
                            max_eng = FFT_ENERGY_MAX;
                        }
                    }
                    //                energy_num[i] = sum_eng / fft_group / energy_step;
                    energy_num[i] = max_eng / energy_step;
                }
            }

            int k = 0;
            if (FFT_LED_L - cpy > 0) {
                for (int i = cpy; i < FFT_LED_L; i++) {
                    energy_num_data[k++] = energy_num[i];//下一个
                }
            }
            for (int i = 0; i < cpy; i++) {
                energy_num_data[k++] = energy_num_back[i];//当前
            }
//            put_buf(energy_num_data, FFT_LED_L);
            cpy = --cpy <= 0 ? FFT_LED_L : cpy;
#else
            //固定位置频谱
            for (int i = 0; i < FFT_LED_L; i++) {
                max_eng = 0;
                sum_eng = 0;
                for (int j = 0; j < fft_group; j++) {
                    sum_eng += fft_db[i * FFT_LED_L + j];
                    if (max_eng < fft_db[i * FFT_LED_L + j]) {
                        max_eng = fft_db[i * FFT_LED_L + j];
                    }
                    if (max_eng > FFT_ENERGY_MAX) {
                        max_eng = FFT_ENERGY_MAX;
                    }
                }
//                energy_num_data[i] = sum_eng / fft_group / energy_step;
                energy_num_data[i] = max_eng / energy_step;
            }
#endif
            tm_1629_shown_fft_db(energy_num_data);
            tm_1629_shown_fft();
            face_waite = 0;
            no_fft_check = 0;
        } else {
#if defined CONFIG_BT_ENABLE && defined CONFIG_BT_MUSIC_MODE_ENABLE
            short *bt_music_get_audio_fft(int *len);
            fft_db = bt_music_get_audio_fft(&fft_freq_num);
            if (fft_db) {
                goto get_fft_data;
            }
#endif
#ifdef CONFIG_USB_DISK_MUSIC_MODE_ENABLE
            short *usbdisk_music_get_audio_fft(int *len);
            fft_db = usbdisk_music_get_audio_fft(&fft_freq_num);
            if (fft_db) {
                goto get_fft_data;
            }
#endif
#ifdef CONFIG_SD_MUSIC_MODE_ENABLE
            short *sd_music_get_audio_fft(int *len);
            fft_db = sd_music_get_audio_fft(&fft_freq_num);
            if (fft_db) {
                goto get_fft_data;
            }
#endif
#ifdef CONFIG_AUX_MUSIC_MODE_ENABLE
            short *aux_music_get_audio_fft(int *len);
            fft_db = aux_music_get_audio_fft(&fft_freq_num);
            if (fft_db) {
                goto get_fft_data;
            }
#endif
        }
        if (!fft_db) {
            no_fft_check++;
            if (no_fft_check == 10) {
                tm_1629_shown_smile_face(0);
            }
        }
//        printf("-> tm_1629_shown_time\n");
    }
}
int tm_time_init(void)
{
    os_task_create(tm_time_task, NULL, 10, 1600, 128, TM_TIME_TASK_NAME);
    return 0;
}
//late_initcall(tm_time_init);

#endif
