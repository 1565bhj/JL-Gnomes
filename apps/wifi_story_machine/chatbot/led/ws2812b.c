#include "app_config.h"
#include "system/includes.h"
#include "device/device.h"
#include "asm/spi.h"
#include "asm/gpio.h"
#include "asm/uart.h"
#include "device/gpio.h"
#include "uart.h"

#ifdef USED_WS2812B_SHOWN

//注意：硬件需要加反相器输出到WS2812的DIN引脚
#define WS2812B_SHOWN_LED_FFT   1 //1开启FFT显示
#define WS2812B_SHOWN_TASK_NAME "ws_task"

#define COLOR_RGB_VALUE_MAXC    255 //最大颜色值（默认最大255）
#define COLOR_RGB_STEP          5 //七彩颜色步长(每个灯珠一样颜色)
#define COLOR_RGB_SPEED         10 //七彩颜色速度(每个灯珠一样颜色)
#define ONE_COLOR_AUTO_CHANGE   0   //单色自动切换颜色，MODE_ONE_COLOR_CYC有效
#define COLOR_RAINBOW_RGB_SPEED     10 //七彩颜色速度(每个灯珠不同样颜色)
#define GRADIENT_SPEED 8        // 渐变速度（1-255，值越大越快）

#ifndef WS2812B_SHOWN_LED_NUM
#define WS2812B_SHOWN_LED_NUM 12
#endif /* WS2812B_SHOWN_LED_NUM */

#define FFT_LED_L   WS2812B_SHOWN_LED_NUM           //列-频率区间
#define FFT_LED_H   20          //行-能量值

#define RGB_STEP    5
// 每个LED的渐变状态
typedef struct {
    uint8_t current_color_index;  // 当前颜色索引
    uint8_t next_color_index;     // 下一个颜色索引
    uint16_t blend_factor;        // 混合因子 (0-255)
    uint8_t direction;            // 渐变方向 (0:正向, 1:反向)
} led_gradient_state_t;
static struct ws2812_info {
    void *spi_hdl;
    void *uart_hdl;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    char breath_addr;
    char breath;
    char inc;
    char on;
    char percent;
    OS_MUTEX mutex;
    led_gradient_state_t led_states[WS2812B_SHOWN_LED_NUM];
    uint8_t rgb_data[WS2812B_SHOWN_LED_NUM * 3];
    int current_mode;
    int one_color_mode;
    void (*current_mode_func)(void);
} WS_INFO;

static struct ws2812_fft {
    unsigned char addr;
    unsigned char energy[FFT_LED_L];
} WS_FFT;

enum {
    WS2812_SHOWN_INIT = 0,
    WS2812_SHOWN_FREQ_SPEC,
    WS2812_SHOWN_SMILE_FACE,        //笑脸显示
    WS2812_SHOWN_SMILE_FACE_CLEAR,  //清空归零示
    WS2812_SHOWN_AUTO_LED,          //LED自动模式打开显示
};
typedef enum rgb_type {
    MODE_BLACK  = 0x000000,
    MODE_RED    = COLOR_RGB_VALUE_MAXC << 16,///0xFF0000,
    MODE_YELLOW = (COLOR_RGB_VALUE_MAXC << 16) | (COLOR_RGB_VALUE_MAXC << 8), //0xFFFF00,
    MODE_GREEN  = COLOR_RGB_VALUE_MAXC << 8,///0x00FF00,
    MODE_CYAN   = (COLOR_RGB_VALUE_MAXC << 8) | COLOR_RGB_VALUE_MAXC, //0x00FFFF,
    MODE_BLUE   = COLOR_RGB_VALUE_MAXC,//0x0000FF,
    MODE_PUR    = (COLOR_RGB_VALUE_MAXC << 16) | COLOR_RGB_VALUE_MAXC, //0xFF00FF,
    MODE_WIHTE  = (COLOR_RGB_VALUE_MAXC << 16) | (COLOR_RGB_VALUE_MAXC << 8) | COLOR_RGB_VALUE_MAXC, //0xFFFFFF,
} RGB_TYPE;
// 彩虹颜色定义 (红黄绿青蓝紫)
#define RAINBOW_COLORS_COUNT 6
static const uint8_t rainbow_colors[RAINBOW_COLORS_COUNT][3] = {
    {COLOR_RGB_VALUE_MAXC, 0, 0},     // 红
    {COLOR_RGB_VALUE_MAXC, COLOR_RGB_VALUE_MAXC, 0},   // 黄
    {0, COLOR_RGB_VALUE_MAXC, 0},     // 绿
    {0, COLOR_RGB_VALUE_MAXC, COLOR_RGB_VALUE_MAXC},   // 青
    {0, 0, COLOR_RGB_VALUE_MAXC},     // 蓝
    {COLOR_RGB_VALUE_MAXC, 0, COLOR_RGB_VALUE_MAXC}    // 紫
};
// 模式枚举
typedef enum {
    MODE_NOME = 0,//静态炫彩渐变
    MODE_STATIC_RAINBOW = 1,//静态炫彩渐变
    MODE_FLOWING_RAINBOW = 2,//流水炫彩渐变
    MODE_ALL_GRADIENT = 3, //静态单色逐色炫彩渐变
    MODE_ONE_COLOR_GRADIENT = 4,//静态单色逐色炫彩
    MODE_ONE_COLOR_CYC = 5,//单色自动循环显示
} LedMode;

#ifdef WS2812B_HW_UART_NUM
static unsigned char ws_rgb_buf[8 * WS2812B_SHOWN_LED_NUM] sec(.sram) ALIGNED(32) = {0};
//用于串口接收缓存数据的循环buf
static u8 uart_buf[1024] __attribute__((aligned(32)));
#else
static unsigned char ws_rgb_buf[9 * WS2812B_SHOWN_LED_NUM] sec(.sram) ALIGNED(32) = {0};
#endif /*  WS2812B_HW_UART_NUM */

int led_eya_get_work(void);
static void ws2812_shown_clear(void);

#ifdef WS2812B_HW_UART_NUM
static int ws2812_uart_init(void)
{
    //1.打开uart设备
    char name[16];
    sprintf(name, "uart%d", WS2812B_HW_UART_NUM);
    WS_INFO.uart_hdl = dev_open(name, NULL);
    if (!WS_INFO.uart_hdl) {
        printf("uart open err \n");
        return -1;
    }

    /* 1 . 设置串口接收缓存数据的循环buf地址 */
    dev_ioctl(WS_INFO.uart_hdl, UART_SET_CIRCULAR_BUFF_ADDR, (int)uart_buf);

    /* 1 . 设置串口接收缓存数据的循环buf长度 */
    dev_ioctl(WS_INFO.uart_hdl, UART_SET_CIRCULAR_BUFF_LENTH, sizeof(uart_buf));

    /* 3 . 设置接收数据为阻塞方式,需要非阻塞可以去掉,建议加上超时设置 */
    dev_ioctl(WS_INFO.uart_hdl, UART_SET_RECV_BLOCK, 1);

    u32 parm = 1000;
    dev_ioctl(WS_INFO.uart_hdl, UART_SET_RECV_TIMEOUT, (u32)parm); //设置超时时间

#ifdef WS2812B_HW_UART_NUM
#if WS2812B_HW_UART_NUM == 0
#define UART_DATA_EXTERN()  extern const struct uart_platform_data uart0_data;
#define UART_DATA_TX()  uart0_data.tx_pin
#define UART_DATA_OUTCH()  uart0_data.output_channel
#define UART_DATA_OUTCH_MAP_SET()  0
#elif WS2812B_HW_UART_NUM == 1
#define UART_DATA_EXTERN()  extern const struct uart_platform_data uart1_data;
#define UART_DATA_TX()  uart1_data.tx_pin
#define UART_DATA_OUTCH()  uart1_data.output_channel
#define UART_DATA_OUTCH_MAP_SET()  1
#elif WS2812B_HW_UART_NUM == 2
#define UART_DATA_EXTERN()  extern const struct uart_platform_data uart2_data;
#define UART_DATA_TX()  uart2_data.tx_pin
#define UART_DATA_OUTCH()  uart2_data.output_channel
#define UART_DATA_OUTCH_MAP_SET()  7
#endif
#endif
    UART_DATA_EXTERN();

    if (UART_DATA_TX() != (u8)0xFF) {
        gpio_set_hd(UART_DATA_TX(), 1);//WS2812需要开启强驱，防止部分的反相器识别不到
    }
#ifdef WS2812B_HW_UART_TX_PORT
    char out, die;
    out = die = 0;
    switch (UART_DATA_OUTCH()) {
    case OUTPUT_CHANNEL0:
        out = die = 0;
        JL_IOMAP->CON1 &= ~(0xF << 8);
        JL_IOMAP->CON1 |= (UART_DATA_OUTCH_MAP_SET() << 8);
        break;
    case OUTPUT_CHANNEL1:
        out = 0;
        die = 1;
        JL_IOMAP->CON3 &= ~(0xF << 20);
        JL_IOMAP->CON3 |= (UART_DATA_OUTCH_MAP_SET() << 20);
        break;
    case OUTPUT_CHANNEL2:
        out = 1;
        die = 0;
        JL_IOMAP->CON3 &= ~(0xF << 24);
        JL_IOMAP->CON3 |= (UART_DATA_OUTCH_MAP_SET() << 24);
        break;
    case OUTPUT_CHANNEL3:
        out = 1;
        die = 1;
        JL_IOMAP->CON3 &= ~(0xF << 28);
        JL_IOMAP->CON3 |= (UART_DATA_OUTCH_MAP_SET() << 28);
        break;
    }
    gpio_set_pull_down(WS2812B_HW_UART_TX_PORT, 1);
    gpio_set_pull_up(WS2812B_HW_UART_TX_PORT, 1);
    gpio_direction_output(WS2812B_HW_UART_TX_PORT, out);
    gpio_set_die(WS2812B_HW_UART_TX_PORT, die);
#endif

    /* 4 . 使能特殊串口,启动收发数据 */
    dev_ioctl(WS_INFO.uart_hdl, UART_START, 0);
    return 0;
}

void ws2812_uart_exit(void)
{
    if (WS_INFO.uart_hdl) {
        os_mutex_pend(&WS_INFO.mutex, 1000);
        ws2812_shown_clear();
        dev_close(WS_INFO.uart_hdl);
        WS_INFO.uart_hdl = NULL;
        os_mutex_post(&WS_INFO.mutex);
    }
}
static int ws2812_uart_get_byte(unsigned char r, unsigned char g, unsigned char b, unsigned char *buf) // WS2812使用GRB顺序
{
//  taskENTER_CRITICAL();//为了计时准确要进临界区
    unsigned char uart_byte;
    int cnt = 0;
    uart_byte = ((g & 0x80) ? 0x2 : 0x3) | (((g & 0x40) ? 0x4 : 0x6) << 2) | (((g & 0x20) ? 0x4 : 0x6) << 5);
    if (buf) {
        *buf++ = uart_byte;
        cnt++;
    }
    //USART1_TX_Data(uart_byte);//1串口发送1字节

    uart_byte = ((g & 0x10) ? 0x2 : 0x3) | (((g & 0x08) ? 0x4 : 0x6) << 2) | (((g & 0x04) ? 0x4 : 0x6) << 5);
    if (buf) {
        *buf++ = uart_byte;
        cnt++;
    }
    //USART1_TX_Data(uart_byte);//2串口发送1字节

    uart_byte = ((g & 0x02) ? 0x2 : 0x3) | (((g & 0x01) ? 0x4 : 0x6) << 2) | (((r & 0x80) ? 0x4 : 0x6) << 5);
    if (buf) {
        *buf++ = uart_byte;
        cnt++;
    }
    //USART1_TX_Data(uart_byte);//3串口发送1字节

    uart_byte = ((r & 0x40) ? 0x2 : 0x3) | (((r & 0x20) ? 0x4 : 0x6) << 2) | (((r & 0x10) ? 0x4 : 0x6) << 5);
    if (buf) {
        *buf++ = uart_byte;
        cnt++;
    }
    //USART1_TX_Data(uart_byte);//4串口发送1字节

    uart_byte = ((r & 0x08) ? 0x2 : 0x3) | (((r & 0x04) ? 0x4 : 0x6) << 2) | (((r & 0x02) ? 0x4 : 0x6) << 5);
    if (buf) {
        *buf++ = uart_byte;
        cnt++;
    }
    //USART1_TX_Data(uart_byte);//5串口发送1字节

    uart_byte = ((r & 0x01) ? 0x2 : 0x3) | (((b & 0x80) ? 0x4 : 0x6) << 2) | (((b & 0x40) ? 0x4 : 0x6) << 5);
    if (buf) {
        *buf++ = uart_byte;
        cnt++;
    }
    //USART1_TX_Data(uart_byte);//6串口发送1字节

    uart_byte = ((b & 0x20) ? 0x2 : 0x3) | (((b & 0x10) ? 0x4 : 0x6) << 2) | (((b & 0x08) ? 0x4 : 0x6) << 5);
    if (buf) {
        *buf++ = uart_byte;
        cnt++;
    }
    //USART1_TX_Data(uart_byte);//7串口发送1字节

    uart_byte = ((b & 0x04) ? 0x2 : 0x3) | (((b & 0x02) ? 0x4 : 0x6) << 2) | (((b & 0x01) ? 0x4 : 0x6) << 5);
    if (buf) {
        *buf++ = uart_byte;
        cnt++;
    }
    //USART1_TX_Data(uart_byte);//8串口发送1字节

//  taskEXIT_CRITICAL();//为了计时准确要进临界区
    return cnt;
}

#else
static int ws2812_spi_init(void)
{
    /* 使用方法1：单次指定接收固定数据量。
     * 注意：此方法spi接收数据比较慢，因此对方主机的spi连续发送字节不能过快，否则丢数据
     * 当数对方主机spi数据过快，请使用上述：用户指定接收块方法.
     */
    //1.打开spi设备
    char name[16];
    sprintf(name, "spi%d", WS2812B_HW_SPI_NUM);
    WS_INFO.spi_hdl = dev_open(name, NULL);
    if (!WS_INFO.spi_hdl) {
        printf("spi open err \n");
        return -1;
    }
    gpio_set_hd(IO_PORTC_10, 1);
    dev_ioctl(WS_INFO.spi_hdl, IOCTL_SPI_SET_IRQ_CPU_ID, (u32)1);   //可以指定中断到核1执行
    dev_ioctl(WS_INFO.spi_hdl, IOCTL_SPI_SET_USE_SEM, 0);   //等待数据时，用信号量等待，不用则为抢占查询硬件中断标记，建议应信号量等待
    dev_ioctl(WS_INFO.spi_hdl, IOCTL_SPI_SEND_BYTE, 0); //发送一个0字节，初始化SPI-DO为低电平
    return 0;
}

void ws2812_spi_exit(void)
{
    if (WS_INFO.spi_hdl) {
        os_mutex_pend(&WS_INFO.mutex, 1000);
        ws2812_shown_clear();
        dev_close(WS_INFO.spi_hdl);
        WS_INFO.spi_hdl = NULL;
        os_mutex_post(&WS_INFO.mutex);
    }
}

static int ws2812_spi_get_byte(unsigned char r, unsigned char g, unsigned char b, unsigned char *buf) // WS2812使用GRB顺序
{
    unsigned char s_byte;
    int cnt = 0;
    s_byte = (((g & 0x80) ? 0x6 : 0x4) << 5) | (((g & 0x40) ? 0x6 : 0x4) << 2) | ((g & 0x20) ? 0x3 : 0x2);
    if (buf) {
        *buf++ = s_byte;
        cnt++;
    }

    s_byte = (((g & 0x10) ? 0x6 : 0x4) << 4) | (((g & 0x08) ? 0x6 : 0x4) << 1) | 0x1;
    if (buf) {
        *buf++ = s_byte;
        cnt++;
    }

    s_byte = (((g & 0x04) ? 0x2 : 0x0) << 6) | (((g & 0x02) ? 0x6 : 0x4) << 3) | ((g & 0x01) ? 0x6 : 0x4);
    if (buf) {
        *buf++ = s_byte;
        cnt++;
    }

    s_byte = (((r & 0x80) ? 0x6 : 0x4) << 5) | (((r & 0x40) ? 0x6 : 0x4) << 2) | (((r & 0x20) ? 0x3 : 0x2));
    if (buf) {
        *buf++ = s_byte;
        cnt++;
    }

    s_byte = (((r & 0x10) ? 0x6 : 0x4) << 4) | (((r & 0x08) ? 0x6 : 0x4) << 1) | 0x1;
    if (buf) {
        *buf++ = s_byte;
        cnt++;
    }

    s_byte = (((r & 0x04) ? 0x2 : 0x0) << 6) | (((r & 0x02) ? 0x6 : 0x4) << 3) | ((r & 0x01) ? 0x6 : 0x4);
    if (buf) {
        *buf++ = s_byte;
        cnt++;
    }

    s_byte = (((b & 0x80) ? 0x6 : 0x4) << 5) | (((b & 0x40) ? 0x6 : 0x4) << 2) | ((b & 0x20) ? 0x3 : 0x2);
    if (buf) {
        *buf++ = s_byte;
        cnt++;
    }

    s_byte = (((b & 0x10) ? 0x6 : 0x4) << 4) | (((b & 0x08) ? 0x6 : 0x4) << 1) | 0x1;
    if (buf) {
        *buf++ = s_byte;
        cnt++;
    }

    s_byte = (((b & 0x04) ? 0x2 : 0x0) << 6) | (((b & 0x02) ? 0x6 : 0x4) << 3) | ((b & 0x01) ? 0x6 : 0x4);
    if (buf) {
        *buf++ = s_byte;
        cnt++;
    }
    return cnt;
}
#endif

void ws2812_init(void)
{
#ifdef WS2812B_HW_UART_NUM
    ws2812_uart_init();
#else
    ws2812_spi_init();
#endif
}
void ws2812_exit(void)
{
#ifdef WS2812B_HW_UART_NUM
    ws2812_uart_exit();
#else
    ws2812_spi_exit();
#endif
}

static void ws2812_shown_write_rgb(unsigned char r, unsigned char g, unsigned char b)
{
    unsigned char *buf = ws_rgb_buf;
    int ret = 0;

#ifdef WS2812B_HW_UART_NUM
    ret = ws2812_uart_get_byte(r, g, b, buf);
    os_mutex_pend(&WS_INFO.mutex, 1000);
    if (WS_INFO.uart_hdl) {
        dev_write(WS_INFO.uart_hdl, ws_rgb_buf, sizeof(ws_rgb_buf));
    }
    os_mutex_post(&WS_INFO.mutex);
#else
    ret = ws2812_spi_get_byte(r, g, b, buf);
    os_mutex_pend(&WS_INFO.mutex, 1000);
    if (WS_INFO.spi_hdl) {
        dev_write(WS_INFO.spi_hdl, ws_rgb_buf, 9);
        os_time_dly(2);
        dev_write(WS_INFO.spi_hdl, ws_rgb_buf, 9);
    }
    os_mutex_post(&WS_INFO.mutex);
#endif // WS2812B_HW_UART_NUM
}

static void ws2812_shown_fft(char cyc)
{
    unsigned char r, g, b;
    unsigned char *buf = ws_rgb_buf;
    memset(ws_rgb_buf, 0, sizeof(ws_rgb_buf));
    cyc = cyc <= 0 ? 1 : cyc;
    do {
        for (char i = 0, rd = WS_FFT.addr; i < FFT_LED_L; i++/*, rd++*/) {
            //        rd = rand() % FFT_LED_L;
            switch (rd % 9) {
            case 0:
                r = WS_FFT.energy[i];
                g = 0;
                b = 0;
                break;
            case 1:
                g = WS_FFT.energy[i];
                r = 0;
                b = 0;
                break;
            case 2:
                b = WS_FFT.energy[i];
                r = 0;
                g = 0;
                break;
            case 3:
                r = WS_FFT.energy[i];
                g = WS_FFT.energy[i];
                b = 0;
                break;
            case 4:
                g = 0;
                r = WS_FFT.energy[i];
                b = WS_FFT.energy[i];
                break;
            case 5:
                b = WS_FFT.energy[i];
                r = 0;
                g = WS_FFT.energy[i];
                break;
            case 6:
                r = WS_FFT.energy[i] / 2;
                g = WS_FFT.energy[i] / 2;
                b = 0;
                break;
            case 7:
                g = 0;
                r = WS_FFT.energy[i] / 2;
                b = WS_FFT.energy[i] / 2;
                break;
            case 8:
                b = WS_FFT.energy[i] / 2;
                r = 0;
                g = WS_FFT.energy[i] / 2;
                break;
            }
#ifdef WS2812B_HW_UART_NUM
            buf += ws2812_uart_get_byte(r, g, b, buf);
#else
            buf += ws2812_spi_get_byte(r, g, b, buf);
#endif
        }

        os_mutex_pend(&WS_INFO.mutex, 1000);
        if (WS_INFO.spi_hdl) {
            dev_write(WS_INFO.spi_hdl, ws_rgb_buf, sizeof(ws_rgb_buf));
        } else if (WS_INFO.uart_hdl) {
            dev_write(WS_INFO.uart_hdl, ws_rgb_buf, sizeof(ws_rgb_buf));
        }
        os_mutex_post(&WS_INFO.mutex);
        if (--cyc) {
            os_time_dly(2);
        }
    } while (cyc);

    WS_FFT.addr++;
    WS_FFT.addr = WS_FFT.addr >= FFT_LED_L ? 0 : WS_FFT.addr;
}

static void ws2812_shown_clear(void)
{
    uint16_t i = 0;
    os_mutex_pend(&WS_INFO.mutex, 1000);
    char *rgb_buffer = ws_rgb_buf;
#ifdef WS2812B_HW_UART_NUM
    while (rgb_buffer < (ws_rgb_buf + sizeof(ws_rgb_buf))) {
        rgb_buffer += ws2812_uart_get_byte(0, 0, 0, rgb_buffer);
        i += 3;
    }
#else
    while (rgb_buffer < (ws_rgb_buf + sizeof(ws_rgb_buf))) {
        rgb_buffer += ws2812_uart_get_byte(0, 0, 0, rgb_buffer);
        i += 3;
    }
#endif
    if (WS_INFO.spi_hdl) {
        dev_write(WS_INFO.spi_hdl, ws_rgb_buf, sizeof(ws_rgb_buf));
    } else if (WS_INFO.uart_hdl) {
        dev_write(WS_INFO.uart_hdl, ws_rgb_buf, sizeof(ws_rgb_buf));
    }
    os_mutex_post(&WS_INFO.mutex);
}

static void ws2812_shown_fft_init(void)
{
    for (char i = 0, rd; i < FFT_LED_L; i++) {
        WS_FFT.energy[i] = 50;
    }
    ws2812_shown_fft(0);
    ws2812_shown_clear();
}

void ws2812_shown_fft_db(unsigned char *energy)
{
    memcpy(&WS_FFT.energy, energy, FFT_LED_L);
}

static void ws2812_shown_face_on_off(char on)
{
    unsigned char energy_num_data[FFT_LED_L] = {0};
    if (on) {
        for (int i = 0; i < FFT_LED_L; i++) {
            energy_num_data[i] = rand() % FFT_LED_H;
        }
        ws2812_shown_fft_db(energy_num_data);
        ws2812_shown_fft(0);
    } else {
        for (int n = 0; n < 5; n++) {
            for (int i = 0; i < FFT_LED_L; i++) {
                energy_num_data[i] = 0;
            }
            ws2812_shown_fft_db(energy_num_data);
            ws2812_shown_fft(0);
            os_time_dly(2);
        }
    }
}

static void ws2812_shown_face_auto_led(char on)
{
    unsigned char energy_num_data[FFT_LED_L] = {0};
    if (on) {
        unsigned char enn = rand() % COLOR_RGB_VALUE_MAXC;
        while (enn < 128) {
            enn = rand() % COLOR_RGB_VALUE_MAXC;
        }
        for (int i = 0; i < FFT_LED_L; i++) {
            energy_num_data[i] = enn;
        }
        ws2812_shown_fft_db(energy_num_data);
        ws2812_shown_fft(2);
    } else {
        for (int n = 0; n < 5; n++) {
            for (int i = 0; i < FFT_LED_L; i++) {
                energy_num_data[i] = 0;
            }
            ws2812_shown_fft_db(energy_num_data);
            ws2812_shown_fft(0);
            os_time_dly(2);
        }
    }
}

static void ws2812_shown_face_breath_led(char on)
{
    unsigned char r, g, b;
    unsigned char *buf = ws_rgb_buf;
    int energy = COLOR_RGB_VALUE_MAXC;
    memset(ws_rgb_buf, 0, sizeof(ws_rgb_buf));
    int cyc = 1;
    if (!on) {
        ws2812_shown_face_auto_led(0);
        //sys_timeout_add(NULL, ws2812_shown_face_auto_led, 1000);
    } else {
        do {
            if (WS_INFO.r <= 0 && WS_INFO.g <= 0  && WS_INFO.b <= 0) {
__rgb_init:;
                int rd = WS_INFO.breath_addr;
                WS_INFO.breath = 100;
                switch (rd % 9) {
                case 0:
                    r = energy;
                    g = 0;
                    b = 0;
                    break;
                case 1:
                    g = energy;
                    r = 0;
                    b = 0;
                    break;
                case 2:
                    b = energy;
                    r = 0;
                    g = 0;
                    break;
                case 3:
                    r = energy;
                    g = energy;
                    b = 0;
                    break;
                case 4:
                    g = 0;
                    r = energy;
                    b = energy;
                    break;
                case 5:
                    b = energy;
                    r = 0;
                    g = energy;
                    break;
                case 6:
                    r = energy / 2;
                    g = energy / 2;
                    b = 0;
                    break;
                case 7:
                    g = 0;
                    r = energy / 2;
                    b = energy / 2;
                    break;
                case 8:
                    b = energy / 2;
                    r = 0;
                    g = energy / 2;
                    break;
                }
                WS_INFO.r = r;
                WS_INFO.g = g;
                WS_INFO.b = b;
                WS_INFO.breath_addr++;
                WS_INFO.breath_addr = WS_INFO.breath_addr >= 9 ? 0 : WS_INFO.breath_addr;
            } else {
                WS_INFO.breath -= 5;
                if (WS_INFO.breath <= 0) {
                    WS_INFO.breath = 0;
                }
            }
            r = WS_INFO.r * WS_INFO.breath / 100;
            g = WS_INFO.g * WS_INFO.breath / 100;
            b = WS_INFO.b * WS_INFO.breath / 100;

            if (r <= 0 && g <= 0  && b <= 0) {
                WS_INFO.r = 0;
                WS_INFO.g = 0;
                WS_INFO.b = 0;
                goto __rgb_init;
            }

            for (char i = 0; i < FFT_LED_L; i++) {
#ifdef WS2812B_HW_UART_NUM
                buf += ws2812_uart_get_byte(r, g, b, buf);
#else
                buf += ws2812_spi_get_byte(r, g, b, buf);
#endif
            }

            printf("rgb0 = %d %d %d , %d\n", r, g, b, WS_INFO.breath_addr);
            os_mutex_pend(&WS_INFO.mutex, 1000);
            if (WS_INFO.spi_hdl) {
                printf("rgb1 = %d %d %d\n", ws_rgb_buf[0], ws_rgb_buf[1], ws_rgb_buf[2]);
                dev_write(WS_INFO.spi_hdl, ws_rgb_buf, sizeof(ws_rgb_buf));
            } else if (WS_INFO.uart_hdl) {
                dev_write(WS_INFO.uart_hdl, ws_rgb_buf, sizeof(ws_rgb_buf));
            }
            os_mutex_post(&WS_INFO.mutex);
            if (--cyc) {
                os_time_dly(2);
            }
        } while (cyc);
    }
}

// 更新所有LED
static void WS2812_Update(void)
{
    uint16_t i = 0;
    os_mutex_pend(&WS_INFO.mutex, 1000);
    char *rgb_buffer = ws_rgb_buf;
#ifdef WS2812B_HW_UART_NUM
    while (rgb_buffer < (ws_rgb_buf + sizeof(ws_rgb_buf))) {
        rgb_buffer += ws2812_uart_get_byte(WS_INFO.rgb_data[i], WS_INFO.rgb_data[i + 1], WS_INFO.rgb_data[i + 2], rgb_buffer);
        i += 3;
    }
#else
    while (rgb_buffer < (ws_rgb_buf + sizeof(ws_rgb_buf))) {
        rgb_buffer += ws2812_uart_get_byte(WS_INFO.rgb_data[i], WS_INFO.rgb_data[i + 1], WS_INFO.rgb_data[i + 2], rgb_buffer);
        i += 3;
    }
#endif
    if (WS_INFO.spi_hdl) {
        dev_write(WS_INFO.spi_hdl, ws_rgb_buf, sizeof(ws_rgb_buf));
    } else if (WS_INFO.uart_hdl) {
        dev_write(WS_INFO.uart_hdl, ws_rgb_buf, sizeof(ws_rgb_buf));
    }
    os_mutex_post(&WS_INFO.mutex);
    //WS2812_Send_Reset();
}

// 设置单个LED颜色
static void set_led_color(uint16_t index, uint8_t r, uint8_t g, uint8_t b)
{
    if (index >= WS2812B_SHOWN_LED_NUM) {
        return;
    }

    WS_INFO.rgb_data[index * 3] = r;     // 红色
    WS_INFO.rgb_data[index * 3 + 1] = g; // 绿色
    WS_INFO.rgb_data[index * 3 + 2] = b; // 蓝色
}
// 颜色混合函数
static uint8_t color_blend(uint8_t color1, uint8_t color2, uint16_t factor)
{
    // factor范围: 0-255, 0=完全color1, 255=完全color2
    uint16_t result = (color1 * (COLOR_RGB_VALUE_MAXC - factor) + color2 * factor) / COLOR_RGB_VALUE_MAXC;
    return (uint8_t)(result > COLOR_RGB_VALUE_MAXC ? COLOR_RGB_VALUE_MAXC : result);
}

// 设置初始颜色分布（从左到右的红黄绿青蓝紫）
static void ws2812_set_initial_color_distribution(void)
{
    for (uint16_t i = 0; i < WS2812B_SHOWN_LED_NUM; i++) {
        // 计算每个LED应该显示的颜色索引
        uint8_t color_index = (i * RAINBOW_COLORS_COUNT) / WS2812B_SHOWN_LED_NUM;

        // 设置LED的初始颜色
        set_led_color(i,
                      rainbow_colors[color_index][0],
                      rainbow_colors[color_index][1],
                      rainbow_colors[color_index][2]);
    }
    WS2812_Update();
}

// 初始化LED渐变状态
static void ws2812_init_led_gradient_states(void)
{
    for (uint16_t i = 0; i < WS2812B_SHOWN_LED_NUM; i++) {
        // 每个LED从不同的颜色开始，创建彩虹分布效果
        WS_INFO.led_states[i].current_color_index = (i * RAINBOW_COLORS_COUNT) / WS2812B_SHOWN_LED_NUM;
        WS_INFO.led_states[i].next_color_index = (WS_INFO.led_states[i].current_color_index + 1) % RAINBOW_COLORS_COUNT;
        WS_INFO.led_states[i].blend_factor = 0;
        WS_INFO.led_states[i].direction = 0; // 正向渐变
    }
}
// 单向渐变版本（只从当前颜色渐变到下一个颜色）
static void update_led_gradient_simple(uint16_t led_index)
{
    if (led_index >= WS2812B_SHOWN_LED_NUM) {
        return;
    }

    led_gradient_state_t *state = &WS_INFO.led_states[led_index];

    // 更新混合因子（单向渐变）
    state->blend_factor += GRADIENT_SPEED;

    if (state->blend_factor >= COLOR_RGB_VALUE_MAXC) {
        // 渐变完成，切换到下一个颜色对
        state->blend_factor = 0;
        state->current_color_index = state->next_color_index;
        state->next_color_index = (state->next_color_index + 1) % RAINBOW_COLORS_COUNT;
    }

    // 计算当前颜色
    uint8_t r = color_blend(rainbow_colors[state->current_color_index][0],
                            rainbow_colors[state->next_color_index][0],
                            state->blend_factor);

    uint8_t g = color_blend(rainbow_colors[state->current_color_index][1],
                            rainbow_colors[state->next_color_index][1],
                            state->blend_factor);

    uint8_t b = color_blend(rainbow_colors[state->current_color_index][2],
                            rainbow_colors[state->next_color_index][2],
                            state->blend_factor);

    // 设置LED颜色
    set_led_color(led_index, r, g, b);
}
// 更新所有LED的渐变
static void ws2812_update_all_gradient(void)
{
    if (WS_INFO.current_mode != MODE_ALL_GRADIENT) {
        WS_INFO.current_mode = MODE_ALL_GRADIENT;
        // 设置初始颜色分布（从左到右的红黄绿青蓝紫）
        ws2812_set_initial_color_distribution();
        // 初始化LED渐变状态
        ws2812_init_led_gradient_states();
    }
    for (uint16_t i = 0; i < WS2812B_SHOWN_LED_NUM; i++) {
        update_led_gradient_simple(i);
    }
    WS2812_Update();
    os_time_dly(COLOR_RGB_SPEED);
}
// 静态彩虹渐变模式
static void ws2812_rainbow_static(void)
{
    WS_INFO.current_mode = MODE_STATIC_RAINBOW;
    for (uint16_t i = 0; i < WS2812B_SHOWN_LED_NUM; i++) {
        // 计算颜色索引
        float position = (float)i / WS2812B_SHOWN_LED_NUM * RAINBOW_COLORS_COUNT;
        int color_index1 = (int)position % RAINBOW_COLORS_COUNT;
        int color_index2 = (color_index1 + 1) % RAINBOW_COLORS_COUNT;
        float ratio = position - (int)position;

        // 在两个颜色之间插值
        uint8_t r = (uint8_t)(rainbow_colors[color_index1][0] * (1 - ratio) +
                              rainbow_colors[color_index2][0] * ratio);
        uint8_t g = (uint8_t)(rainbow_colors[color_index1][1] * (1 - ratio) +
                              rainbow_colors[color_index2][1] * ratio);
        uint8_t b = (uint8_t)(rainbow_colors[color_index1][2] * (1 - ratio) +
                              rainbow_colors[color_index2][2] * ratio);

        set_led_color(i, r, g, b);
    }
    WS2812_Update();
    os_time_dly(COLOR_RAINBOW_RGB_SPEED * 10);
}
// 彩虹流水效果
static void ws2812_rainbow_flow(void)
{
    static uint8_t offset = 0;
    if (WS_INFO.current_mode != MODE_FLOWING_RAINBOW) {
        WS_INFO.current_mode = MODE_FLOWING_RAINBOW;
        offset = 0;
    }

    for (uint16_t i = 0; i < WS2812B_SHOWN_LED_NUM; i++) {
        // 计算颜色索引（带偏移量）
        uint8_t color_index = (i + offset) % RAINBOW_COLORS_COUNT;
        set_led_color(i,
                      rainbow_colors[color_index][0],
                      rainbow_colors[color_index][1],
                      rainbow_colors[color_index][2]);
    }

    WS2812_Update();
    offset = (offset + 1) % RAINBOW_COLORS_COUNT;
    os_time_dly(COLOR_RAINBOW_RGB_SPEED);
}
static void ws2812_shown_one_color_auto(void)//单独颜色，亮度设置：brightness_percent 0 -100
{
    if (WS_INFO.current_mode != MODE_ONE_COLOR_CYC) {
        WS_INFO.current_mode = MODE_ONE_COLOR_CYC;
    }
    WS_INFO.one_color_mode = WS_INFO.one_color_mode ? WS_INFO.one_color_mode : MODE_RED;
    char *rgb_buffer = ws_rgb_buf;
    unsigned char color_r = 0;
    unsigned char color_g = 0;
    unsigned char color_b = 0;

#define BRIGHT_PERCENT(v,p) ((v)*(p)/100)
    switch (WS_INFO.one_color_mode) {
    case MODE_WIHTE:
        color_r = BRIGHT_PERCENT(COLOR_RGB_VALUE_MAXC, WS_INFO.percent);
        color_g = BRIGHT_PERCENT(COLOR_RGB_VALUE_MAXC, WS_INFO.percent);
        color_b = BRIGHT_PERCENT(COLOR_RGB_VALUE_MAXC, WS_INFO.percent);
        break;
    case MODE_BLACK:
        color_r = BRIGHT_PERCENT(0, WS_INFO.percent);
        color_g = BRIGHT_PERCENT(0, WS_INFO.percent);
        color_b = BRIGHT_PERCENT(0, WS_INFO.percent);
        break;
    case MODE_RED:
        color_r = BRIGHT_PERCENT(COLOR_RGB_VALUE_MAXC, WS_INFO.percent);
        color_g = BRIGHT_PERCENT(0, WS_INFO.percent);
        color_b = BRIGHT_PERCENT(0, WS_INFO.percent);
        WS_INFO.one_color_mode = ONE_COLOR_AUTO_CHANGE ? MODE_YELLOW : WS_INFO.one_color_mode;
        break;
    case MODE_YELLOW:
        color_r = BRIGHT_PERCENT(COLOR_RGB_VALUE_MAXC, WS_INFO.percent);
        color_g = BRIGHT_PERCENT(COLOR_RGB_VALUE_MAXC, WS_INFO.percent);
        color_b = BRIGHT_PERCENT(0, WS_INFO.percent);
        WS_INFO.one_color_mode = ONE_COLOR_AUTO_CHANGE ? MODE_GREEN : WS_INFO.one_color_mode;
        break;
    case MODE_GREEN:
        color_r = BRIGHT_PERCENT(0, WS_INFO.percent);
        color_g = BRIGHT_PERCENT(COLOR_RGB_VALUE_MAXC, WS_INFO.percent);
        color_b = BRIGHT_PERCENT(0, WS_INFO.percent);
        WS_INFO.one_color_mode = ONE_COLOR_AUTO_CHANGE ? MODE_CYAN : WS_INFO.one_color_mode;
        break;
    case MODE_CYAN:
        color_r = BRIGHT_PERCENT(0, WS_INFO.percent);
        color_g = BRIGHT_PERCENT(COLOR_RGB_VALUE_MAXC, WS_INFO.percent);
        color_b = BRIGHT_PERCENT(COLOR_RGB_VALUE_MAXC, WS_INFO.percent);
        WS_INFO.one_color_mode = ONE_COLOR_AUTO_CHANGE ? MODE_BLUE : WS_INFO.one_color_mode;
        break;
    case MODE_BLUE:
        color_r = BRIGHT_PERCENT(0, WS_INFO.percent);
        color_g = BRIGHT_PERCENT(0, WS_INFO.percent);
        color_b = BRIGHT_PERCENT(COLOR_RGB_VALUE_MAXC, WS_INFO.percent);
        WS_INFO.one_color_mode = ONE_COLOR_AUTO_CHANGE ? MODE_PUR : WS_INFO.one_color_mode;
        break;
    case MODE_PUR:
        color_r = BRIGHT_PERCENT(COLOR_RGB_VALUE_MAXC, WS_INFO.percent);
        color_g = BRIGHT_PERCENT(0, WS_INFO.percent);
        color_b = BRIGHT_PERCENT(COLOR_RGB_VALUE_MAXC, WS_INFO.percent);
        WS_INFO.one_color_mode = ONE_COLOR_AUTO_CHANGE ? MODE_RED : WS_INFO.one_color_mode;
        break;
    }
    for (uint16_t i = 0; i < WS2812B_SHOWN_LED_NUM; i++) {
        set_led_color(i, color_r, color_g, color_b);
    }
    WS2812_Update();
    os_time_dly(COLOR_RGB_SPEED);
}
static void ws2812_shown_one_color_gradient(void)//所有灯珠绚丽彩灯
{
    char *rgb_buffer = ws_rgb_buf;
    static unsigned char color_r = COLOR_RGB_VALUE_MAXC;//初始化为红色
    static unsigned char color_g = 0;//初始化为0
    static unsigned char color_b = 0;//初始化为0
    static unsigned int last_mode = MODE_RED;//初始化为红色,color_r = 255

    if (WS_INFO.current_mode != MODE_ONE_COLOR_GRADIENT) {
        WS_INFO.current_mode = MODE_ONE_COLOR_GRADIENT;
        color_r = COLOR_RGB_VALUE_MAXC;
        color_g = color_b = 0;
    }
    switch (last_mode) {
    case MODE_WIHTE:
        color_r += COLOR_RGB_STEP;
        if (color_r >= (COLOR_RGB_VALUE_MAXC - COLOR_RGB_STEP)) {
            color_r = COLOR_RGB_VALUE_MAXC;
            last_mode = MODE_RED;
        }
        break;
    case MODE_RED:
        color_g += COLOR_RGB_STEP;
        if (color_g >= (COLOR_RGB_VALUE_MAXC - COLOR_RGB_STEP)) {
            color_g = COLOR_RGB_VALUE_MAXC;
            last_mode = MODE_YELLOW;
        }
        break;
    case MODE_YELLOW:
        color_r -= COLOR_RGB_STEP;
        if (color_r <= COLOR_RGB_STEP) {
            color_r = 0;
            last_mode = MODE_GREEN;
        }
        break;
    case MODE_GREEN:
        color_b += COLOR_RGB_STEP;
        if (color_b >= (COLOR_RGB_VALUE_MAXC - COLOR_RGB_STEP)) {
            color_b = COLOR_RGB_VALUE_MAXC;
            last_mode = MODE_CYAN;
        }
        break;
    case MODE_CYAN:
        color_g -= COLOR_RGB_STEP;
        if (color_g <= COLOR_RGB_STEP) {
            color_g = 0;
            last_mode = MODE_BLUE;
        }
        break;
    case MODE_BLUE:
        color_r += COLOR_RGB_STEP;
        if (color_r >= (COLOR_RGB_VALUE_MAXC - COLOR_RGB_STEP)) {
            color_r = COLOR_RGB_VALUE_MAXC;
            last_mode = MODE_PUR;
        }
    case MODE_PUR:
        color_b -= COLOR_RGB_STEP;
        if (color_b <= COLOR_RGB_STEP) {
            color_b = 0;
            last_mode = MODE_RED;
        }
        break;
    }
    for (uint16_t i = 0; i < WS2812B_SHOWN_LED_NUM; i++) {
        set_led_color(i, color_r, color_g, color_b);
    }
    WS2812_Update();
    os_time_dly(COLOR_RGB_SPEED);
}
/*
 应用层使用的API，设置RGB灯的模式
 对应模式mode：
    MODE_STATIC_RAINBOW = 1,//静态炫彩渐变
    MODE_FLOWING_RAINBOW = 2,//流水炫彩渐变
    MODE_ALL_GRADIENT = 3, //静态单色逐色炫彩渐变
    MODE_ONE_COLOR_GRADIENT = 4,//静态单色逐色炫彩
    MODE_ONE_COLOR_CYC = 5,//单色自动循环显示
 对应的percent：亮度百分比，只有模式mode=MODE_ONE_COLOR_CYC才有效
*/
void ws2812_shown_mode(int mode, int percent)
{
    WS_INFO.percent = (char)percent;
    switch (mode) {
    case MODE_FLOWING_RAINBOW:
        ws2812_rainbow_flow();
        WS_INFO.current_mode_func = ws2812_rainbow_flow;
        break;
    case MODE_STATIC_RAINBOW:
        ws2812_rainbow_static();
        WS_INFO.current_mode_func = ws2812_rainbow_static;
        break;
    case MODE_ALL_GRADIENT:
        ws2812_update_all_gradient();
        WS_INFO.current_mode_func = ws2812_update_all_gradient;
        break;
    case MODE_ONE_COLOR_GRADIENT:
        ws2812_shown_one_color_gradient();
        WS_INFO.current_mode_func = ws2812_shown_one_color_gradient;
        break;
    case MODE_ONE_COLOR_CYC:
        ws2812_shown_one_color_auto();
        WS_INFO.current_mode_func = ws2812_shown_one_color_auto;
        break;
    default:
        WS_INFO.current_mode_func = NULL;
        break;
    }
    WS_INFO.current_mode = mode;
}
/*
 应用层使用的API，设置RGB灯的单色模式
 对应模式mode：
    MODE_BLACK  = 0x000000,//黑色
    MODE_RED    = 0xFF0000,//红色
    MODE_YELLOW = 0xFFFF00,//黄色
    MODE_GREEN  = 0x00FF00,//绿色
    MODE_CYAN   = 0x00FFFF,//青色
    MODE_BLUE   = 0x0000FF,//蓝色
    MODE_PUR    = 0xFF00FF,//紫色
    MODE_WIHTE  = 0xFFFFFF,//白色
 对应的percent：亮度百分比，只有模式mode=MODE_ONE_COLOR_CYC才有效
*/
void ws2812_shown_one_color_mode(int mode, int percent)
{
    WS_INFO.one_color_mode = mode;
    WS_INFO.percent = (char)percent;
}
/*
 应用层使用的API，设置RGB灯的单色模式自动循环
 对应的percent：亮度百分比，只有模式MODE_ONE_COLOR_CYC才有效
*/
void ws2812_shown_one_color_auto_mode(void)
{
    if (!WS_INFO.one_color_mode) {
        WS_INFO.one_color_mode = MODE_RED;
    } else {
        switch (WS_INFO.one_color_mode) {
        case MODE_RED:
            WS_INFO.one_color_mode = MODE_YELLOW;
            break;
        case MODE_YELLOW:
            WS_INFO.one_color_mode = MODE_GREEN;
            break;
        case MODE_GREEN:
            WS_INFO.one_color_mode = MODE_CYAN;
            break;
        case MODE_CYAN:
            WS_INFO.one_color_mode = MODE_BLUE;
            break;
        case MODE_BLUE:
            WS_INFO.one_color_mode = MODE_PUR;
            break;
        case MODE_PUR:
            WS_INFO.one_color_mode = MODE_RED;
            break;
        }
    }
}

void ws2812_shown_face(int on)
{
    os_taskq_post(WS2812B_SHOWN_TASK_NAME, 2, WS2812_SHOWN_SMILE_FACE, on);
}

void ws2812_shown_auto_led(int on)
{
    os_taskq_post(WS2812B_SHOWN_TASK_NAME, 2, WS2812_SHOWN_AUTO_LED, on);
}

void ws2812_shown_auto_led_ctrl(char on)
{
    if (on <= 1) {
        WS_INFO.on = on ? 1 : 0;
    } else {
        if (!WS_INFO.on) {
            WS_INFO.on = 1;
        } else {
            WS_INFO.on = 0;
        }
    }
    if (!WS_INFO.on) {
        sys_timeout_add_to_task("sys_timer", 0, ws2812_shown_auto_led, 1000);
    }
    ws2812_shown_auto_led(WS_INFO.on);
}

short *net_music_get_audio_fft(int *len);
static int ws_shown_task(void*p)
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
    int face_auto_on = 0;
    unsigned int face_auto_on_cnt = 0;
    int face_waite = 0;
    char temp;
    unsigned char energy_num_data[FFT_LED_L] = {0};
    static unsigned char energy_num[FFT_LED_L] = {0};
    static unsigned char energy_num_back[FFT_LED_L] = {0};

    os_mutex_create(&WS_INFO.mutex);
    ws2812_init();
    ws2812_shown_clear();
#ifdef WS2812B_FFT_EFFECT_ENABLE
    ws2812_shown_fft_init();
    ws2812_shown_auto_led_ctrl(2);
#else
//    ws2812_shown_mode(MODE_STATIC_RAINBOW, 100);//所有模式的灯
    ws2812_shown_mode(MODE_FLOWING_RAINBOW, 100);//所有模式的灯
//    ws2812_shown_mode(MODE_ALL_GRADIENT, 100);//所有模式的灯
//    ws2812_shown_mode(MODE_ONE_COLOR_GRADIENT, 100);//所有模式的灯
//    ws2812_shown_mode(MODE_ONE_COLOR_CYC, 100);//所有模式的灯
#endif
    os_time_dly(200);
    while (1) {
        //res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        res = os_taskq_accept(ARRAY_SIZE(msg), msg);
#ifndef WS2812B_FFT_EFFECT_ENABLE
        if (WS_INFO.current_mode_func) {
            WS_INFO.current_mode_func();
        } else {
            os_time_dly(10);
        }
#else
        if (res == OS_TASK_DEL_IDLE) {
            break;
        } else if (res == OS_TASKQ) {
            if (msg[0] == Q_USER) {
                face_on = msg[2];
                switch (msg[1]) {
                case WS2812_SHOWN_SMILE_FACE:
                    ws2812_shown_face_on_off(face_on);
                    face_auto_on = 0;
                    face_auto_on_cnt = 0;
                    break;
                case WS2812_SHOWN_AUTO_LED:
//                    ws2812_shown_face_auto_led(face_on);
                    ws2812_shown_face_breath_led(face_auto_on);
                    face_auto_on = face_on;
                    face_auto_on_cnt = 0;
                    break;
                }
            }
        }
        os_time_dly(10);
//        if(face_auto_on && (face_auto_on_cnt++ % 50) == 0){
//            ws2812_shown_face_auto_led(face_auto_on);
//        }
        if (face_auto_on && (face_auto_on_cnt++ % 6) == 0) {
            ws2812_shown_face_breath_led(face_auto_on);
        }
        if (led_eya_get_work()) {
            continue;
        }
#if WS2812B_SHOWN_LED_FFT
        fft_db = net_music_get_audio_fft(&fft_freq_num);
        if (fft_db) {
get_fft_data:
            face_auto_on = 0;
            face_auto_on_cnt = 0;
            cnt = 0;

#define FFT_ENERGY_MAX      64
            char division = FFT_ENERGY_MAX > FFT_LED_H ? 1 : 0; //除法
            int energy_step = division ? (FFT_ENERGY_MAX / FFT_LED_H) : ((FFT_LED_H + 1) / FFT_ENERGY_MAX);
            int fft_group = fft_freq_num / FFT_LED_L;
            int max_eng = 0, sum_eng = 0;

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
                    if (!division) {
                        max_eng = max_eng * energy_step;
                        max_eng = max_eng > FFT_LED_H ? FFT_LED_H : max_eng;
                        energy_num[i] = max_eng;
                    } else {
                        energy_num[i] = max_eng / energy_step;
                    }
                }
            }

            int k = 0;
            if (FFT_LED_L - cpy > 0) {
                for (int i = cpy; i < FFT_LED_L; i++) {
                    energy_num_data[k++] = energy_num[i]; //下一个
                }
            }
            for (int i = 0; i < cpy; i++) {
                energy_num_data[k++] = energy_num_back[i]; //当前
            }
            //put_buf(energy_num_data, FFT_LED_L);
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
//            int fft_num = 0;
//            for(int i = 0; i < 3 /*FFT_ENERGY_MAX*/; i++){
//                if(fft_db[i] > max_eng){
//                    max_eng = fft_db[i];
//                }
//                fft_num++;
//            }
//            max_eng = max_eng > FFT_ENERGY_MAX ? FFT_ENERGY_MAX : max_eng;
//            int maxengy = max_eng * COLOR_RGB_VALUE_MAXC / (FFT_ENERGY_MAX);
//            for(int i = 0; i < FFT_LED_L; i++){
//                energy_num_data[i] = maxengy;
//            }

            ws2812_shown_fft_db(energy_num_data);
            ws2812_shown_fft(0);
            face_waite = 0;
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
#endif
        cnt++;
        if (cnt >= 100 && cnt <= 110) { //1秒清空显示
            memset(energy_num_data, 0, sizeof(energy_num_data));
            ws2812_shown_fft_db(energy_num_data);
            ws2812_shown_fft(0);
            ws2812_shown_clear();
        }
#endif
    }
}
int ws_shown_init(void)
{
    os_task_create(ws_shown_task, NULL, 10, 1600, 64, WS2812B_SHOWN_TASK_NAME);
    return 0;
}
//late_initcall(ws_shown_init);

#endif
