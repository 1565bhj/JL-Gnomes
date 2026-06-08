#include "app_config.h"
#include "system/includes.h"
#include "asm/clock.h"
#include "asm/irq.h"
#include "asm/gpio.h"
#include "asm/spi.h"
#include "printer_stepper_motor.h"
#include "printer_test.h"

//=================================以下宏定义，需要结合实际板子进行改动===============================//
// 使用的硬件SPI
#define JL_SPI          JL_SPI2 //与spi设备对应：JL_SPI0 JL_SPI1 JL_SPI2

/* 硬件要求每次只打 96 点(12 字节)，每 48 字节分 4 次发送，非当前组补零 */
#define PRINTER_CHUNK_BYTES  12


// IO位宏定义
#define LATCH_PIN               2           // 锁存IO - PC2
#define HEATER_PIN              0           // 加热IO - PA0
#define CLK_PIN                 3           // 锁存IO - PA3
#define DATA_PIN                4           // 加热IO - PA4

/* 时序参数（单位 us）—— 参考打印头手册表4 电气参数
 * 加热能量 Eo(25°C) = 0.14 mJ/dot → ts = 1.49ms（R=90Ω, VH=3.3V）
 * 每点电流 Io = 32.3 mA/dot
 * 推荐打印速度 = 2.5 ms/line */
#define LATCH_TIME              2           /* 锁存脉冲宽度 2us */
#define HEATER_ON_US            6500        /* 加热时间：手册 1.49ms + 温度余量 ≈ 2ms */
#define CHUNK_GAP_US            150         /* chunk 间冷却间隔 0.5ms */
#define EXTRA_FEED_INTERVAL_US  2000        /* 额外走纸：每次定时器间隔 2ms */
#define EXTRA_FEED_COUNT        1200        /* 额外走纸次数，总时长 = INTERVAL * COUNT = 1s，可调 */

// 数据和时钟的IO初始化操作宏定义
#define CLK_IO_INIT()       {JL_PORTA->DIR &=~BIT(CLK_PIN);JL_PORTA->PU &=~BIT(CLK_PIN);JL_PORTA->PD &=~BIT(CLK_PIN);JL_PORTA->OUT &=~BIT(CLK_PIN);}
#define DATA_IO_INIT()      {JL_PORTA->DIR &=~BIT(DATA_PIN);JL_PORTA->PU &=~BIT(DATA_PIN);JL_PORTA->PD &=~BIT(DATA_PIN);JL_PORTA->OUT &=~BIT(DATA_PIN);}

// 数据锁存IO操作宏定义
#define LATCH_IO_INIT()     {JL_PORTC->DIR &=~BIT(LATCH_PIN);JL_PORTC->PU |= BIT(LATCH_PIN);JL_PORTC->PD &=~BIT(LATCH_PIN);JL_PORTC->OUT |= BIT(LATCH_PIN);}
#define LATCH_SET(x)        {if(x){JL_PORTC->OUT |= BIT(LATCH_PIN);}else{JL_PORTC->OUT &= ~BIT(LATCH_PIN);}}

// 加热IO初始化操作宏定义
#define HEATER_IO_INIT()    {JL_PORTA->DIR &=~BIT(HEATER_PIN);JL_PORTA->PU &=~BIT(HEATER_PIN);JL_PORTA->PD |= BIT(HEATER_PIN);JL_PORTA->OUT &= ~BIT(HEATER_PIN);}
#define HEATER_SET(x)       {if(x){JL_PORTA->OUT |= BIT(HEATER_PIN);}else{JL_PORTA->OUT &= ~BIT(HEATER_PIN);}}

//=================================以上宏定义，需要结合实际板子进行改动===============================//

// 状态定义
#define STATE_IDLE       0           // 空闲状态
#define STATE_LATCH      1           // 锁存状态
#define STATE_HEATER     2           // 加热状态
#define STATE_END        3           // 结束状态
#define STATE_MOTOR_STOP 4           // 电机停止状态
#define STATE_EXTRA_FEED 5           // 额外走纸状态
// IO操作宏定义

// 全局变量
static SEC_USED(.volatile_ram) volatile u8 timer_state = STATE_IDLE;
static SEC_USED(.volatile_ram) volatile u8 spi_trigger = 0;

/* 分块发送用：每次向硬件送 48 字节，其中只有当前 12 字节为数据，其余补零以降低电流 */
static SEC_USED(.volatile_ram) u8 spi_oneline_buf[PRINTER_ONELINE_POINT_DATA_BYTES] ALIGNED(32);
static SEC_USED(.volatile_ram) volatile u32 spi_buffer_cnt = 0;

static SEC_USED(.volatile_ram) void *spi_hdl = NULL;
static SEC_USED(.volatile_ram) volatile char *spi_send_buf = 0;
static SEC_USED(.volatile_ram) volatile u32 spi_send_one_size = 0;
static SEC_USED(.volatile_ram) volatile u32 spi_send_all_size = 0;
static SEC_USED(.volatile_ram) volatile u32 spi_send_offset = 0;

static SEC_USED(.volatile_ram) JL_TIMER_TypeDef *TMR = JL_TIMER4;    // 选择定时器4
static SEC_USED(.volatile_ram) u8 timer_irq = IRQ_TIMER4_IDX;        // 选择定时器4中断
static SEC_USED(.volatile_ram) u32 timer_clk = 0;
static SEC_USED(.volatile_ram) volatile u32 extra_feed_cnt = 0;

//设置一行字节数
static void SEC_USED(.volatile_ram_code) printer_set_send_buf_once_size(int size)
{
    spi_send_one_size = size;
}

//打印一行数据：每一行都发送整行48字节数据，但只有12字节(96个点)是有效数据
static int SEC_USED(.volatile_ram_code) printer_send_buf_one_line(void)
{
    int size = 0;
    if (spi_oneline_buf) {
        memset(spi_oneline_buf, 0, spi_send_one_size);
        if (spi_buffer_cnt >= PRINTER_CHUNK_BYTES && spi_buffer_cnt < spi_send_one_size) {
            size = MIN(PRINTER_CHUNK_BYTES, spi_send_one_size - spi_buffer_cnt);
            memcpy(spi_oneline_buf + spi_buffer_cnt, spi_send_buf + spi_send_offset + spi_buffer_cnt, size);
            asm volatile("csync;");
            JL_SPI->ADR = (u32)spi_oneline_buf;//配置硬件接收地址
            JL_SPI->CNT = (u32)spi_send_one_size;//写该寄存器则正式启动硬件接
            spi_buffer_cnt += size;
            asm volatile("csync;");
            return size;
        }
    }
    return 0;
}

//打印一帧数据
static int SEC_USED(.volatile_ram_code) printer_send_buf_next(void)
{
    int size = 0;
    int remain = 0;
    spi_send_offset += spi_buffer_cnt;
    spi_buffer_cnt = 0;
    if (spi_oneline_buf) {
        memset(spi_oneline_buf, 0, spi_send_one_size);
        if (spi_send_buf && spi_send_offset < spi_send_all_size) {//检查每一行
            size = MIN(PRINTER_CHUNK_BYTES, spi_send_one_size - spi_buffer_cnt);
            memcpy(spi_oneline_buf, spi_send_buf + spi_send_offset, size);
            asm volatile("csync;");
            JL_SPI->ADR = (u32)spi_oneline_buf;//配置硬件接收地址
            JL_SPI->CNT = (u32)spi_send_one_size;//写该寄存器则正式启动硬件接
            spi_buffer_cnt += size;
        } else {
            spi_send_all_size = 0;
            spi_send_offset = 0;
        }
    } else if (spi_send_buf && spi_send_offset < spi_send_all_size) {
        size = MIN(spi_send_all_size - spi_send_offset, spi_send_one_size);
        JL_SPI->ADR = (u32)spi_send_buf + (u32)spi_send_offset;//配置硬件接收地址
        JL_SPI->CNT = (u32)size;//写该寄存器则正式启动硬件接
        asm volatile("csync;");
        spi_send_offset += size;
    } else {
        spi_send_all_size = 0;
        spi_send_offset = 0;
    }
    asm volatile("csync;");
    return spi_send_offset;
}

// 定时器配置函数
static void SEC_USED(.volatile_ram_code) timer_cfg(u32 freq, u32 us)
{
    u8 timer_index[16] =  {0, 4, 1, 5, 2,  6,  3,  7,   8,   12,  9,    13,   10,   14,   11,    15};
    u32 timer_table[16] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};
    u32 clock = timer_clk;
    u8 psc = 0;
    u32 prd = 0;
    u32 ts = us / (1000 * 1000);  // 计算秒数
    u32 tu = us % (1000 * 1000);  // 计算剩余微秒数
    float tp = 0;

    // 频率范围检查
    if (freq >= clock) {
        freq = clock;
    } else if (freq <= 1) {
        freq = 1;
        if (ts) {
            tp = (float)tu / (1000 * 1000);
        }
    }

    // 计算周期值
    prd = clock / freq;

    // 如果周期值超过65535，调整预分频器
    if (prd > 65535) {
        for (psc = 0; psc < 16; psc++) {
            prd = (u32)(clock / (timer_table[psc]) / freq);
            if (prd <= 65535) {
                break;
            } else if (psc == 15) {
                prd = 65535;  // 最大周期值
                break;
            }
        }
    }

    // 计算最终周期值
    prd = ts ? (prd * ts + tp * prd) : prd;
    psc = timer_index[psc];

    // 配置定时器
    TMR->CON = 0;          // 重置控制寄存器
    TMR->CNT = 0;          // 重置计数器
    TMR->CON |= BIT(14);   // 清除中断标志
    TMR->PRD = prd;        // 设置周期值
    TMR->CON |= psc << 4;  // 设置预分频器
    TMR->CON |= BIT(0);     // 启动定时器
}
static void SEC_USED(.volatile_ram_code) timer_set(u32 us)//设置时间，该函数可以在中断调用重新设置定时器值
{
    u32 freq = 1000000 / us;
    timer_cfg(freq, us);
}
static void SEC_USED(.volatile_ram_code) timer_freq_set(u32 freq)//设置频率，该函数可以在中断调用重新设置定时器值
{
    timer_cfg(freq, 0);
}
// 定时器中断处理函数
static ___interrupt SEC_USED(.volatile_ram_code) void timer_isr(void)
{
    char no_paper = printer_no_paper_detect();
    if (TMR->CON & BIT(15)) {
        TMR->CON |= BIT(14);  // 清除中断标志
        TMR->CON = 0;     // 启动定时器
        // 根据当前状态处理
        switch (timer_state) {
        case STATE_LATCH:
            if (no_paper) {
                extra_feed_cnt = EXTRA_FEED_COUNT;
                timer_state = STATE_EXTRA_FEED;
                timer_set(EXTRA_FEED_INTERVAL_US);
                LATCH_SET(1);
                HEATER_SET(0);
            } else  if (spi_trigger) {
                // 锁存IO恢复高电平
                LATCH_SET(1);

                // 切换到加热等待状态
                timer_state = STATE_HEATER;
                spi_trigger = 0;

                // 启动1ms定时器
                timer_set(HEATER_ON_US);
            }
            break;

        case STATE_HEATER:
            if (no_paper) {
                extra_feed_cnt = EXTRA_FEED_COUNT;
                timer_state = STATE_EXTRA_FEED;
                timer_set(EXTRA_FEED_INTERVAL_US);
                LATCH_SET(1);
                HEATER_SET(0);
            } else {
                // 加热IO恢复低电平
                HEATER_SET(0);

                // // 返回空闲状态
                timer_state = STATE_END;
                timer_set(CHUNK_GAP_US);
            }
            break;
        case STATE_END:
            timer_state = STATE_IDLE;
            TMR->CON = 0;  // 停止定时器
            // 连续发送数据
            if (no_paper || !printer_send_buf_one_line()) {
                if (no_paper || !printer_send_buf_next()) {
                    extra_feed_cnt = EXTRA_FEED_COUNT;
                    timer_state = STATE_EXTRA_FEED;
                    timer_set(EXTRA_FEED_INTERVAL_US);
                    LATCH_SET(1);
                    HEATER_SET(0);
                }
            }
            break;
        case STATE_EXTRA_FEED:
            if (--extra_feed_cnt > 0) {
                timer_set(EXTRA_FEED_INTERVAL_US);
            } else {
                TMR->CON = 0;
                printer_motor_emergency_stop();
                timer_state = STATE_IDLE;
                LATCH_SET(1);
                HEATER_SET(0);
            }
            break;
        case STATE_IDLE:
        default:
            // 空闲状态，停止定时器
            LATCH_SET(1);
            HEATER_SET(0);
            TMR->CON = 0;
            break;
        }
    }
}

void SEC_USED(.volatile_ram_code)  printer_spi_dev_stop(void)
{
    TMR->CON = 0;
    JL_SPI->CON |= BIT(14);
    JL_SPI->CON &= ~BIT(0);

    LATCH_SET(1);
    HEATER_SET(0);

    spi_send_buf = 0;
    spi_send_all_size = 0;
    spi_send_offset = 0;
    spi_trigger = 0;
    extra_feed_cnt = 0;
    timer_state = STATE_IDLE;
}

// SPI中断处理函数
static ___interrupt SEC_USED(.volatile_ram_code) void spi_irq_cb(void)
{
    JL_SPI->CON |= BIT(14);  // 清除中断标记

    // 1. 设置状态
    timer_state = STATE_LATCH;
    spi_trigger = 1;
    asm volatile("csync;");

    // 1. SPI传输完成，设置锁存为低电平，加热为高电平
    LATCH_SET(0);     // 锁存IO输出低电平
    HEATER_SET(1);    // 加热IO输出高电平

    // 3. 启动定时器
    timer_set(LATCH_TIME);
}


// IO初始化函数
void printer_io_init(void)
{
    // 初始化锁存IO为输出，默认高电平
    LATCH_IO_INIT();
    LATCH_SET(1);

    HEATER_IO_INIT();
    CLK_IO_INIT();
    DATA_IO_INIT();
}
// 系统初始化函数
int printer_spi_dev_init(void)
{
    char dev_name[16];
    char dev_id = 1;
    if (spi_hdl) {
        printf("spi open redo \n");
        return 0;
    }
    // IO初始化
    printer_io_init();
    printer_set_send_buf_once_size(PRINTER_ONELINE_POINT_DATA_BYTES);

    timer_clk = clk_get("timer");                  // 获取定时器的时钟源
    request_irq(timer_irq, 3, timer_isr, 0);       // 注册中断函数和中断优先级
    switch ((int)JL_SPI) {
    case (int)JL_SPI1:
        dev_id = 1;
        break;
    case (int)JL_SPI2:
        dev_id = 2;
        break;
    default :
        break;
    }
    sprintf(dev_name, "spi%d", dev_id);
    if (!spi_hdl) {
        //1.打开spi设备
        spi_hdl = dev_open(dev_name, NULL);
    }
    if (!spi_hdl) {
        printf("spi open err \n");
        return -1;
    }
    //2.注册中断函数
    dev_ioctl(spi_hdl, IOCTL_SPI_SET_IRQ_FUNC, (u32)spi_irq_cb);

    //3.配置接收模式，开启中断，开启SPI
    JL_SPI->CON &= ~(BIT(10) | BIT(11) | BIT(2)); //1bit模式
    //JL_SPI->CON |= BIT(12);//接收模式
    JL_SPI->CON &= ~BIT(12); //发送模式
    JL_SPI->CON |= BIT(13);//开启中断
    //JL_SPI->CON |= BIT(0);//开启SPI

    // 初始化状态
    timer_state = STATE_IDLE;
    spi_trigger = 0;
    return 0;
}

int printer_is_printf(void)
{
    return timer_state != STATE_IDLE;
}

int printer_send_buf(uint8_t *buf, int len)
{
    int size = 0;
    if (!buf || len <= 0) {
        return -1;
    }
    if (printer_no_paper_detect()) {
        printf("err no_paper \n");
        return -1;
    }
    /* 按行发送，长度应为 PRINTER_ONELINE_POINT_DATA_BYTES 的整数倍 */
    if (len % PRINTER_ONELINE_POINT_DATA_BYTES) {
        len = (len / PRINTER_ONELINE_POINT_DATA_BYTES) * PRINTER_ONELINE_POINT_DATA_BYTES;
    }
    if (len <= 0) {
        return -1;
    }
    while (spi_trigger) {
        asm volatile("csync;");
        asm volatile("nop;");
    }
    if (printer_is_printf()) {
        JL_SPI->CON |= BIT(14);  // 清除中断标记
        JL_SPI->CON &= ~BIT(0);//关闭SPI
        TMR->CON = 0;//定时器关闭
        LATCH_IO_INIT();
        HEATER_IO_INIT();
    }
    JL_SPI->CON |= BIT(14);  // 清除中断标记
    JL_SPI->CON |= BIT(0);//开启SPI
    asm volatile("csync;");

    // 初始化状态
    timer_state = STATE_IDLE;

    spi_trigger = 0;
    spi_send_buf = buf;
    spi_send_all_size = len;

    spi_send_offset = 0;
    spi_buffer_cnt = 0;
    printer_send_buf_next();
    printf("printf start\n");
    return 0;
}

int printer_test_print_image(char *img, int len)
{
    if (printer_spi_dev_init() || !img || len <= 0) {
        printf("printer_spi_dev_init err\n");
        return -1;
    }
    if (!printer_no_paper_detect()) {
        printer_motor_init();
        printer_motor_rotate_dir_default();
        printer_motor_run(1);
        if (printer_send_buf(img, len)) {
            printer_motor_emergency_stop();
        }
        printf("printer_test_print_image done\n");
    } else {
        music_play_res_file("Nopaper.mp3");
    }
    return 0;
}


#if 0
int printer_test_print_image1(void)
{
    if (printer_spi_dev_init()) {
        printf("printer_spi_dev_init err\n");
        return -1;
    }
    if (!printer_no_paper_detect()) {
        printer_motor_init();
        printer_motor_rotate_dir_default();
        printer_motor_run(1);
        if (printer_send_buf(test_file, sizeof(test_file))) {
            printer_motor_emergency_stop();
        }
        printf("printer_test_print_image done\n");
    } else {
        music_play_res_file("Nopaper.mp3");
    }
    return 0;
}

void printer_jpg_print_test(void)
{
    int file_size = 0;
    char *jpg_buf = NULL;
    int jpg_size = 0;
    char *printf_buf = NULL;
    int printf_size = 0;
    int out_lines = 0;

    FILE *fd = fopen(CONFIG_VOICE_PROMPT_FILE_PATH"test.jpg", "r");
    if (fd) {
        file_size = flen(fd);
        jpg_size = file_size;
        jpg_buf = malloc(jpg_size);
        if (jpg_buf) {
            fread(jpg_buf, 1, jpg_size, fd);
        }
        printf("--->jpg_size : %d \n", jpg_size);
        fclose(fd);
        if (jpg_buf) {
            printf_size = 35 * 1024;
            printf_buf = malloc(printf_size);
            if (printf_buf) {
                printf_size = printer_jpg_print(jpg_buf, jpg_size, printf_buf, printf_size);
                printer_test_print_image(printf_buf, printf_size);
                free(printf_buf);
            }
            free(jpg_buf);
        }
    }
}

#endif

