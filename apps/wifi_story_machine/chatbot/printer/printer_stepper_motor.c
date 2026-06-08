#include "app_config.h"
#include "system/includes.h"
#include "asm/clock.h"
#include "asm/irq.h"
#include "asm/gpio.h"
#include "asm/spi.h"
#include "printer_stepper_motor.h"

//=================================以下宏定义，需要结合实际板子进行改动===============================//
// 宏定义
#define INTA1_PIN         6   // PA6 - A相正极
#define INTA2_PIN         9   // PA9 - A相负极
#define INTB1_PIN         1   // PA1 - B相正极
#define INTB2_PIN         2   // PA2 - B相负极
#define SLEEP_PIN         7   // PA7 - 睡眠IO
#define PAPER_DETECT_PIN  0   // PA3 - 缺纸检测IO
#define MOTOR_VDD_PIN     5   // PA5 - 电机和加热头供电使能

// 步进电机参数宏定义（定时器中断驱动，固定 10ms 一步）
#define HALF_STEP_MODE    1           // 1=半步模式
#define STEP_INTERVAL_US  6500        // 每步间隔 6.5ms（定时器周期，单位 us）
#define MOTOR_PRINT_DIR   0           // 电机的默认打印方向

// 步进电机IO初始化宏定义
#define INTA1_IO_INIT()     {JL_PORTA->DIR &= ~BIT(INTA1_PIN); JL_PORTA->HD |= BIT(INTA1_PIN); JL_PORTA->OUT &= ~BIT(INTA1_PIN);}
#define INTA2_IO_INIT()     {JL_PORTA->DIR &= ~BIT(INTA2_PIN); JL_PORTA->HD |= BIT(INTA2_PIN); JL_PORTA->OUT &= ~BIT(INTA2_PIN);}
#define INTB1_IO_INIT()     {JL_PORTA->DIR &= ~BIT(INTB1_PIN); JL_PORTA->HD |= BIT(INTB1_PIN); JL_PORTA->OUT &= ~BIT(INTB1_PIN);}
#define INTB2_IO_INIT()     {JL_PORTA->DIR &= ~BIT(INTB2_PIN); JL_PORTA->HD |= BIT(INTB2_PIN); JL_PORTA->OUT &= ~BIT(INTB2_PIN);}

#define INTA1_SET(x)        {if(x){JL_PORTA->OUT |= BIT(INTA1_PIN);}else{JL_PORTA->OUT &= ~BIT(INTA1_PIN);}}
#define INTA2_SET(x)        {if(x){JL_PORTA->OUT |= BIT(INTA2_PIN);}else{JL_PORTA->OUT &= ~BIT(INTA2_PIN);}}
#define INTB1_SET(x)        {if(x){JL_PORTA->OUT |= BIT(INTB1_PIN);}else{JL_PORTA->OUT &= ~BIT(INTB1_PIN);}}
#define INTB2_SET(x)        {if(x){JL_PORTA->OUT |= BIT(INTB2_PIN);}else{JL_PORTA->OUT &= ~BIT(INTB2_PIN);}}

// 步进电机驱动芯片的休眠IO操作宏定义
#define SLEEP_IO_INIT()     {JL_PORTA->DIR &= ~BIT(SLEEP_PIN); JL_PORTA->HD |= BIT(SLEEP_PIN); JL_PORTA->OUT &= ~BIT(SLEEP_PIN);}
#define SLEEP_SET(x)        {if(x){JL_PORTA->OUT |= BIT(SLEEP_PIN);}else{JL_PORTA->OUT &= ~BIT(SLEEP_PIN);}}

// 步进电机和加热体的电压供电引脚
#define MOTOR_VDD_IO_INIT() {JL_PORTA->DIR &= ~BIT(MOTOR_VDD_PIN); JL_PORTA->HD |= BIT(MOTOR_VDD_PIN); JL_PORTA->OUT &= ~BIT(MOTOR_VDD_PIN);}
#define MOTOR_VDD_SET(x)    {if(x){JL_PORTA->OUT |= BIT(MOTOR_VDD_PIN);}else{JL_PORTA->OUT &= ~BIT(MOTOR_VDD_PIN);}}


// 缺纸检测 IO 初始化：普通输入模式
#define PAPER_DETECT_IO_INIT()  {JL_PORTC->DIR |=  BIT(PAPER_DETECT_PIN);JL_PORTC->DIE |=  BIT(PAPER_DETECT_PIN);JL_PORTC->PU  &= ~BIT(PAPER_DETECT_PIN);JL_PORTC->PD  &= ~BIT(PAPER_DETECT_PIN);}
// 读取缺纸状态：1=缺纸(高电平)  0=有纸(低电平)
#define NO_PAPER_DETECT_READ()  (!!(JL_PORTC->IN & BIT(PAPER_DETECT_PIN)))

//=================================以上宏定义，需要结合实际板子进行改动===============================//

// 定时器相关定义，请结合实际应用更改，默认不改（电机使用JL_TIMER3，加热棒使用JL_TIMER4，分开好控制电机速度和加热速度）
static SEC_USED(.volatile_ram) JL_TIMER_TypeDef *TMR = JL_TIMER3;  // 选择定时器3
static SEC_USED(.volatile_ram) u8 timer_irq = IRQ_TIMER3_IDX;      // 定时器中断号
static SEC_USED(.volatile_ram) u32 timer_clk = 0;

#define AWINLINE __attribute__((always_inline))
#define TSEC SEC_USED(.volatile_ram_code)

//==============================================================================
// 1-2相驱动步序表（半步模式，8 步循环）
// 数组索引: [步数][0=INTA1, 1=INTA2, 2=INTB1, 3=INTB2]
// 定时器每 6.5ms 触发一次，每周期切到下一步
//==============================================================================
static SEC_USED(.volatile_ram) u8 step_sequence_half[8][4] = {
    {1, 1, 0, 1},   // 步0: A+  (0°)
    {0, 1, 0, 1},   // 步1: A+ B+ (45°)
    {0, 1, 1, 1},   // 步2: B+ (90°)
    {0, 1, 1, 0},   // 步3: A- B+ (135°)
    {1, 1, 1, 0},   // 步4: A- (180°)
    {1, 0, 1, 0},   // 步5: A- B- (225°)
    {1, 0, 1, 1},   // 步6: B- (270°)
    {1, 0, 0, 1}    // 步7: A+ B- (315°)
};

// 整步模式步序表（四相八拍，作为备选）
static SEC_USED(.volatile_ram) u8 step_sequence_full[4][4] = {
    {1, 0, 1, 0},  // 步1: A+ B+ (45°)
    {1, 0, 0, 1},  // 步2: A+ B- (315°)
    {0, 1, 0, 1},  // 步3: A- B- (225°)
    {0, 1, 1, 0}   // 步4: A- B+ (135°)
};

//==============================================================================
// 步进电机状态结构体
//==============================================================================
typedef struct {
    volatile u8 current_step;       // 当前步数: 半步模式0-7, 整步模式0-3
    volatile u8 direction;          // 方向: 0=正转, 1=反转
    volatile u8 running;            // 运行状态: 0=停止, 1=运行中
    volatile u8 init;               // IO初始化状态
    volatile u8 half_step;          // 驱动模式: 1=半步模式, 0=整步模式
    volatile u32 speed;             // 速度: 定时器时间，单位us
} StepperMotor_t;

static SEC_USED(.volatile_ram) StepperMotor_t stepper;

//==============================================================================
// 定时器配置函数（原样保留）
//==============================================================================
static AWINLINE TSEC void timer_cfg(u32 freq, u32 us)
{
    u8 timer_index[16] =  {0, 4, 1, 5, 2,  6,  3,  7,   8,   12,  9,    13,   10,   14,   11,    15};
    u32 timer_table[16] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};
    u32 clock = timer_clk;
    u8 psc = 0;
    u32 prd = 0;
    u32 ts = us / (1000 * 1000);
    u32 tu = us % (1000 * 1000);
    float tp = 0;

    if (freq >= clock) {
        freq = clock;
    } else if (freq <= 1) {
        freq = 1;
        if (ts) {
            tp = (float)tu / (1000 * 1000);
        }
    }

    prd = clock / freq;
    if (prd > 65535) {
        for (psc = 0; psc < 16; psc++) {
            prd = (u32)(clock / (timer_table[psc]) / freq);
            if (prd <= 65535) {
                break;
            } else if (psc == 15) {
                prd = 65535;
                break;
            }
        }
    }
    prd = ts ? (prd * ts + tp * prd) : prd;
    psc = timer_index[psc];

    TMR->CON = 0;
    TMR->CNT = 0;
    TMR->CON |= BIT(14);
    TMR->PRD = prd;
    TMR->CON |= psc << 4;
    TMR->CON |= BIT(0);
}

//==============================================================================
// 设置定时器时间
//==============================================================================
static void TSEC timer_set(u32 us)
{
    u32 freq = 1000000 / us;
    timer_cfg(freq, us);
}

//==============================================================================
// 设置定时器频率
//==============================================================================
static void TSEC timer_freq_set(u32 freq)
{
    timer_cfg(freq, 0);
}

//==============================================================================
// 设置步进电机一步输出（根据当前模式）
//==============================================================================
static void TSEC stepper_set_output(void)
{
    u8 step = stepper.current_step;

    if (stepper.half_step) {
        // 半步模式 (0-7)
        step &= 0x07;  // 确保范围0-7
        INTA1_SET(step_sequence_half[step][0]);
        INTA2_SET(step_sequence_half[step][1]);
        INTB1_SET(step_sequence_half[step][2]);
        INTB2_SET(step_sequence_half[step][3]);
        stepper.current_step = stepper.direction ? (step == 0 ? 0x07 : (--step)) : ++step;
    } else {
        // 整步模式 (0-3)
        step &= 0x03;  // 确保范围0-3
        INTA1_SET(step_sequence_full[step][0]);
        INTA2_SET(step_sequence_full[step][1]);
        INTB1_SET(step_sequence_full[step][2]);
        INTB2_SET(step_sequence_full[step][3]);
        stepper.current_step = stepper.direction ? (step == 0 ? 0x03 : (--step)) : ++step;
    }
}

//==============================================================================
// 停止电机（释放所有线圈）
//==============================================================================
static void TSEC stepper_stop_motor(void)
{
    // 所有输出置低电平，关闭所有MOSFET
    INTA1_SET(0);
    INTA2_SET(0);
    INTB1_SET(0);
    INTB2_SET(0);

    SLEEP_SET(0);//休眠电机驱动
}

//==============================================================================
// 紧急停止电机
//==============================================================================
void TSEC printer_motor_emergency_stop(void)
{
    stepper.running = 0;
    TMR->CON &= ~BIT(0);  // 停止定时器
    asm volatile("csync;");
    stepper_stop_motor();  // 释放电机
}

//==============================================================================
// 缺纸检测相关函数
//==============================================================================
int TSEC printer_no_paper_detect(void)
{
    return NO_PAPER_DETECT_READ();
}

//==============================================================================
// 定时器中断处理函数
//==============================================================================
static ___interrupt TSEC void timer_isr(void)
{
    if (TMR->CON & BIT(15)) {
        TMR->CON |= BIT(14);  // 清除中断标志
        // 步进电机控制
        if (stepper.running) {
            // 更新到下一步 设置IO输出
            stepper_set_output();
        }
    }
}

//==============================================================================
// 打印机步进电机初始化
//==============================================================================
void printer_motor_test_init(void);
int printer_motor_init(void)
{
    // 初始化IO
    INTA1_IO_INIT();
    INTA2_IO_INIT();
    INTB1_IO_INIT();
    INTB2_IO_INIT();
    PAPER_DETECT_IO_INIT();
    MOTOR_VDD_IO_INIT()
    SLEEP_IO_INIT();

    SLEEP_SET(1);
    MOTOR_VDD_SET(1);

    // 初始状态：所有输出低电平
    stepper_stop_motor();

    // 初始化步进电机状态
    stepper.current_step = 0;
    stepper.direction = 0;
    stepper.running = 0;
    stepper.speed = STEP_INTERVAL_US;
    stepper.half_step = HALF_STEP_MODE;  // 使用半步模式

    // 获取定时器时钟源
    timer_clk = clk_get("timer");

    // 注册定时器中断
    request_irq(timer_irq, 3, timer_isr, 0);

    /* 初始化完成后自检：正转 1.5s → 反转 1.5s */
    printer_motor_test_init();

    SLEEP_SET(0);//休眠电机驱动
    return 0;
}
//==============================================================================
// 打印机步进电机速度控制
// @param interval_us: 定时器时间，控制速度
//==============================================================================
int printer_motor_set_speed(int interval_us)
{
    stepper.speed = interval_us;
    TMR->CON &= ~BIT(0);
    asm volatile("csync;");
    if (stepper.running) {
        timer_set(stepper.speed);
    }
    return 0;
}

//==============================================================================
// 打印机步进电机运行控制
// @param run: 1-启动电机, 0-停止电机
//==============================================================================
int printer_motor_run(char run)
{
    if (run) {
        SLEEP_SET(1);//开启电机驱动
        if (!stepper.running) {
            stepper.running = 1;
            timer_set(stepper.speed);
        }
    } else {
        stepper.running = 0;
        // 停止定时器
        TMR->CON &= ~BIT(0);
        // 释放电机线圈
        stepper_stop_motor();
    }

    return 0;
}

//==============================================================================
// 设置电机方向
// @param dir: 0-正转, 1=反转
//==============================================================================
void printer_motor_set_direction(u8 dir)
{
    if (dir <= 1) {
        stepper.direction = dir;
    }
}

//==============================================================================
// 设置驱动模式
// @param half_mode: 1-半步模式, 0-整步模式
//==============================================================================
void printer_motor_set_mode(u8 half_mode)
{
    if (half_mode <= 1) {
        stepper.half_step = half_mode;
        // 如果电机正在运行，更新输出
        if (stepper.running) {
            stepper_set_output();
        }
    }
}

//==============================================================================
// 获取电机运行状态
//==============================================================================
int printer_motor_is_running(void)
{
    return stepper.running;
}

//==============================================================================
// 电机转动指定圈数
// @param dir: 方向(0-正转, 1-反转)
//==============================================================================
void printer_motor_rotate_dir(u8 dir)
{
    stepper.direction = dir;
    printer_motor_run(1);
}

//==============================================================================
// 电机转动默认方向
//==============================================================================
void printer_motor_rotate_dir_default(void)
{
    printer_motor_set_direction(MOTOR_PRINT_DIR);
}


//==============================================================================
// 初始化完成后电机自检：正转 1.5s → 停 → 反转 1.5s
//==============================================================================
void printer_motor_test_init(void)
{
    if (stepper.init) {
        return ;
    }
    stepper.init = true;
#define PRINTER_MOTOR_TEST_TICKS_1S 100
    int speed = stepper.speed;
    /* 正转 1.5 秒 */
    printer_motor_set_direction(MOTOR_PRINT_DIR);
    if (speed) {
        printer_motor_set_speed(speed / 2);//速度比初始化块一倍
    }
    printer_motor_run(1);
    os_time_dly(PRINTER_MOTOR_TEST_TICKS_1S * 3 / 2);
    printer_motor_run(0);
    os_time_dly(5);

    /* 反转 1.5 秒 */
    printer_motor_set_direction(!MOTOR_PRINT_DIR);
    if (speed) {
        printer_motor_set_speed(speed / 2);//速度比初始化块一倍
    }
    printer_motor_run(1);
    os_time_dly(PRINTER_MOTOR_TEST_TICKS_1S);
    printer_motor_run(0);
    printer_motor_set_direction(MOTOR_PRINT_DIR);
    stepper.speed = speed;//恢复初始化的速度
    printf("[printer_motor] init test done: fwd 1.5s + rev 1.5s\n");
}

