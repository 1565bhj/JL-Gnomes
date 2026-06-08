#include "gSensor_manage.h"
#include "SC7A20H.h"
#include <math.h>
#include "generic/typedef.h"
#include "system/includes.h" // 包含系统头文件，避免printf.h缺失
#include "asm/clock.h"
#include "asm/adc_api.h"
#include "timer.h"
#include "asm/efuse.h"
#include "asm/p33.h"
#include "asm/power_interface.h"
#include "device/gpio.h"
#include "system/includes.h"
#include "app_config.h"
//#include "common/ntc/ntc.h"
#include "ai_uart_ctrol.h"

#include "system/includes.h"
#include "fs/fs.h"

// 添加json和http相关头文件
#include "json_c/json.h"
#include "json_c/json_tokener.h"



#ifdef CONFIG_SC7A20H_GSENSOR_ENABLE

// 添加上报锁定变量
static bool g_water_reporting = false;
static unsigned int g_last_report_time = 0;
#define REPORT_LOCK_INTERVAL 3000 // 上报锁定时间(毫秒)

// 低通滤波频率定义
#define LPF_CUTOFF_800HZ  0x00  // CTRL_REG2: 00xx xxxx
#define LPF_CUTOFF_200HZ  0x40  // CTRL_REG2: 01xx xxxx
#define LPF_CUTOFF_100HZ  0x80  // CTRL_REG2: 10xx xxxx
#define LPF_CUTOFF_50HZ   0xC0  // CTRL_REG2: 11xx xxxx

// 加速阈值
#define SHAKE_THRESHOLD             0.02 //检测摇换最低角度阈值

// 用于存储当前传感器的灵敏度设置
static unsigned char g_current_sc7a20h_sensitivity = G_SENSITY_LOW;

// 内部延时函数
static void sl_delay(unsigned char sl_i)
{
    unsigned int sl_j = 10;
    sl_j = sl_j * 1000 * sl_i;
    while (sl_j--);
}

// SC7A20H传感器命令发送
unsigned char sc7a20h_sensor_command(unsigned char register_address, unsigned char function_command)
{
    gravity_sensor_command(WRITE_COMMAND_FOR_SC7A20H, register_address, function_command);
    return 0;
}

// SC7A20H传感器数据获取
unsigned char sc7a20h_sensor_get_data(unsigned char register_address)
{
    return _gravity_sensor_get_data(WRITE_COMMAND_FOR_SC7A20H, READ_COMMAND_FOR_SC7A20H, register_address);
}

// 设置低通滤波频率
void sc7a20h_set_lpf_cutoff(unsigned char cutoff_freq)
{
    unsigned char reg_value;

    // 读取当前CTRL_REG2值
    reg_value = sc7a20h_sensor_get_data(SC7A20H_CTRL_REG2);

    // 清除原有滤波设置位(bit6-7)
    reg_value &= 0x3F;

    // 设置新的滤波频率
    reg_value |= cutoff_freq;

    // 写入寄存器
    sc7a20h_sensor_command(SC7A20H_CTRL_REG2, reg_value);

    printf("低通滤波已设置，截止频率: ");
    switch (cutoff_freq) {
    case LPF_CUTOFF_800HZ:
        printf("800Hz\n");
        break;
    case LPF_CUTOFF_200HZ:
        printf("200Hz\n");
        break;
    case LPF_CUTOFF_100HZ:
        printf("100Hz\n");
        break;
    case LPF_CUTOFF_50HZ:
        printf("50Hz\n");
        break;
    default:
        printf("未知\n");
    }
}
static uint8_t shake_begin_flag = 0;
static uint8_t shake_end_flag = 0;
static bool shake_flag = 0;
static bool is_shake_flag = false;
extern bool music_start;
static bool die_shake_flag = false;
static bool draw_lots_shake_flag = false;
static bool draw_lots_executed = false;

void qian_mode_sc7a20h_enable(int enbable)
{
    draw_lots_shake_flag = enbable;
    printf("draw_lots_shake_flag = %d\n", draw_lots_shake_flag);
}
void qian_mode_restore(void)
{
    draw_lots_executed = false;
}
// 获取传感器加速度数据并处理摇晃检测逻辑
// 在文件开头添加一个全局变量来标记是否正在播放表情
static bool is_playing_emoji = false;
void qian_delay_6s(void)
{
    draw_lots_executed = false;
    is_playing_emoji = false;
    qian_mode_sc7a20h_enable(1);
}

double sqrt_binary(double x)
{
    if (x < 0) {
        return -1;
    }
    if (x == 0 || x == 1) {
        return x;
    }

    double low, high;

    // 确定搜索范围
    if (x < 1) {
        low = x;
        high = 1;
    } else {
        low = 1;
        high = x;
    }

    double mid;
    double epsilon = 1e-7;

    while (high - low > epsilon) {
        mid = (low + high) / 2.0;

        if (mid * mid > x) {
            high = mid;
        } else {
            low = mid;
        }
    }

    return (low + high) / 2.0;
}

#define SAMPLE_NUM  10
// 结构体定义
typedef struct {
    float x, y, z;
} Acceleration;

typedef struct {
    Acceleration buffer[SAMPLE_NUM];
    int size;
    int count;
    float sum_x, sum_y, sum_z;  // 用于快速计算
} sample_data_type;
static sample_data_type sample_data ALIGNED(4) = {0};

// 添加样本到缓冲区（优化版本，维护总和）
void add_sample(sample_data_type *buf, Acceleration *acc)
{
    if (buf->count >= SAMPLE_NUM) {
        // 需要移除最旧的样本
        buf->count = SAMPLE_NUM;
        memcpy(&buf->buffer[0], &buf->buffer[1], sizeof(Acceleration) * (SAMPLE_NUM - 1));
        memcpy(&buf->buffer[SAMPLE_NUM - 1], acc, sizeof(Acceleration));
//        buf->buffer[SAMPLE_NUM - 1].x = acc->x;
//        buf->buffer[SAMPLE_NUM - 1].y = acc->y;
//        buf->buffer[SAMPLE_NUM - 1].z = acc->z;
    } else {
        // 添加新样本
        memcpy(&buf->buffer[buf->count], acc, sizeof(Acceleration));
//        buf->buffer[buf->count].x = acc->x;
//        buf->buffer[buf->count].y = acc->y;
//        buf->buffer[buf->count].z = acc->z;
        buf->count++;
    }
}

// 计算滑动窗口内加速度的方差
float calculate_variance(sample_data_type *buf)
{
    buf->sum_x = buf->sum_y = buf->sum_z = 0.0;
    // 计算均值（使用预计算的总和）
    for (int i = 0; i < SAMPLE_NUM; i++) {
        buf->sum_x += buf->buffer[i].x;
        buf->sum_y += buf->buffer[i].y;
        buf->sum_z += buf->buffer[i].z;
    }

    float mean_x = buf->sum_x / SAMPLE_NUM;
    float mean_y = buf->sum_y / SAMPLE_NUM;
    float mean_z = buf->sum_z / SAMPLE_NUM;

    // 计算方差
    float variance = 0;
    for (int i = 0; i < SAMPLE_NUM; i++) {
        float diff_x = buf->buffer[i].x - mean_x;
        float diff_y = buf->buffer[i].y - mean_y;
        float diff_z = buf->buffer[i].z - mean_z;
        variance += (diff_x * diff_x + diff_y * diff_y + diff_z * diff_z);
    }

    return variance / SAMPLE_NUM;
}
// 计算滑动窗口内加速度的方差
float calculate_variance_x(sample_data_type *buf)
{
    buf->sum_x = 0;
    // 计算均值（使用预计算的总和）
    for (int i = 0; i < SAMPLE_NUM; i++) {
        buf->sum_x += buf->buffer[i].x;
    }
    float mean_x = buf->sum_x / SAMPLE_NUM;
    float variance = 0;
    for (int i = 0; i < SAMPLE_NUM; i++) {
        float diff_x = buf->buffer[i].x - mean_x;
        variance += (diff_x * diff_x);
    }
    return variance / SAMPLE_NUM;
}
// 计算滑动窗口内加速度的方差
float calculate_variance_y(sample_data_type *buf)
{
    buf->sum_y = 0;
    // 计算均值（使用预计算的总和）
    for (int i = 0; i < SAMPLE_NUM; i++) {
        buf->sum_y += buf->buffer[i].y;
    }
    float mean_y = buf->sum_y / SAMPLE_NUM;
    float variance = 0;
    for (int i = 0; i < SAMPLE_NUM; i++) {
        float diff_y = buf->buffer[i].y - mean_y;
        variance += (diff_y * diff_y);
    }
    return variance / SAMPLE_NUM;
}
// 计算滑动窗口内加速度的方差
float calculate_variance_z(sample_data_type *buf)
{
    buf->sum_z = 0;
    // 计算均值（使用预计算的总和）
    for (int i = 0; i < SAMPLE_NUM; i++) {
        buf->sum_z += buf->buffer[i].z;
    }
    float mean_z = buf->sum_z / SAMPLE_NUM;
    float variance = 0;
    for (int i = 0; i < SAMPLE_NUM; i++) {
        float diff_z = buf->buffer[i].z - mean_z;
        variance += (diff_z * diff_z);
    }
    return variance / SAMPLE_NUM;
}

// 计算总加速度（矢量模长，减去重力）
float calculate_total_acceleration(float x, float y, float z)
{
    float magnitude = sqrt_binary(x * x + y * y + z * z);; //sqrt(x*x + y*y + z*z);
//    float magnitude = sqrt(x*x + y*y + z*z);
    return magnitude;//fabs(magnitude - 1.0);
}

// 检测摇晃的主函数
bool detect_shaking(sample_data_type *buf, Acceleration *current, float val)
{

    //一阶低通滤波器（RC滤波器），公式：当前输出 = α × 当前输入 + (1-α) × 上一次输出
    static float x_g = 0;
    static float y_g = 0;
    static float z_g = 0;
#define ALPHA 0.2f  // 滤波系数，越小滤波效果越强
    x_g = ALPHA * current->x + (1 - ALPHA) * x_g;
    y_g = ALPHA * current->y + (1 - ALPHA) * y_g;
    z_g = ALPHA * current->z + (1 - ALPHA) * z_g;

    //获取低通滤波数据
    current->x = x_g;
    current->y = y_g;
    current->z = z_g;

    //样本窗口添加
    add_sample(buf, current);

    if (buf->count < SAMPLE_NUM) {
        return false;
    }

    //计算三个方向方差
    float variance_x = calculate_variance_x(buf);
    float variance_y = calculate_variance_y(buf);
    float variance_z = calculate_variance_z(buf);

    //计算动态两个方向（移动平面）加速度
    float dynamic_acc_xy = calculate_total_acceleration(variance_x, variance_y, 0);
    float dynamic_acc_yz = calculate_total_acceleration(0, variance_y, variance_z);
    float dynamic_acc_xz = calculate_total_acceleration(variance_x, 0, variance_z);
    float dynamic_acc_xyz = calculate_total_acceleration(variance_x, variance_y, variance_z);

//    printf("---> xy=%.5f yz=%.5f xz=%.5f , x=%0.5f y=%0.5f z=%0.5f\n",
//           dynamic_acc_xy,dynamic_acc_yz,dynamic_acc_xz,
//           variance_x, variance_y, variance_z
//           );
    if (dynamic_acc_xy > val || dynamic_acc_yz > val || dynamic_acc_xz > val || dynamic_acc_xyz > val) {
        return true;
    }
    return false;
}
// 修改sc7a20h_get_acceleration函数中的相关代码
void sc7a20h_get_acceleration(unsigned int cnt)
{
    // 读取三轴加速度数据
    signed short acc_x = 0, acc_y = 0, acc_z = 0;

    // 读取原始数据
    unsigned char acc_x_lsb, acc_x_msb;
    unsigned char acc_y_lsb, acc_y_msb;
    unsigned char acc_z_lsb, acc_z_msb;

    static float last_pitch_x = 0;
    static float last_roll_y  = 0;
    static float last_theta_z = 0;
    static bool shake_detected = false;

    static unsigned int last_shake_time = 0;

    acc_x_lsb = sc7a20h_sensor_get_data(SC7A20H_OUT_X_L);
    acc_x_msb = sc7a20h_sensor_get_data(SC7A20H_OUT_X_H);

    acc_y_lsb = sc7a20h_sensor_get_data(SC7A20H_OUT_Y_L);
    acc_y_msb = sc7a20h_sensor_get_data(SC7A20H_OUT_Y_H);

    acc_z_lsb = sc7a20h_sensor_get_data(SC7A20H_OUT_Z_L);
    acc_z_msb = sc7a20h_sensor_get_data(SC7A20H_OUT_Z_H);

    if (cnt < 50) {//过滤前10秒，等待10秒后开始时间稳定性检测...
        return;
    }

    // 12位数据处理
    acc_x = (signed short)((acc_x_msb << 8) + acc_x_lsb) >> 4;
    acc_y = (signed short)((acc_y_msb << 8) + acc_y_lsb) >> 4;
    acc_z = (signed short)((acc_z_msb << 8) + acc_z_lsb) >> 4;

    if (draw_lots_shake_flag) {
        // 添加静态标志变量，确保只执行一次
        if (!draw_lots_executed) {
            printf("draw_lots_executed = %d\n", draw_lots_executed);
#ifdef CONFIG_UI_PLAY_EMOJI
            play_face_emoji(0, 200);
            printf("play_face_emoji(0, 200);play_face_emoji(0, 200);play_face_emoji(0, 200);\n");
#endif
            draw_lots_executed = true;
        }
    }
//    printf("acc_x %d, acc_y %d,acc_z %d  --------------------\r\n", acc_x, acc_y, acc_z);

    if ((acc_x == 0) && (acc_y == 0) && (acc_z == 0)) {
        //putchar('Z');
        return;
    }

    extern bool is_any_music_playing(void);

    // 只有不在对话页面时才更新摇晃标志
    extern volatile int keyworld_start;
    if (net_update_request() || keyworld_start) {
//       printf("检查在对话页面\n");printf("关闭陀螺仪\n");
        //putchar('R');
        return;
    }
    // 根据当前分辨率转换单位为g
    float resolution;
    switch (g_current_sc7a20h_sensitivity) {
    case G_SENSITY_HIGH:
        resolution = 4096.0f; // ±2g，4096 LSB/g
        break;
    case G_SENSITY_MEDIUM:
        resolution = 2048.0f; // ±4g
        break;
    case G_SENSITY_LOW:
        resolution = 512.0f;  // ±16g
        break;
    default:
        resolution = 4096.0f;
    }

    float acc_x_g = (float)acc_x / resolution * 1.0;
    float acc_y_g = (float)acc_y / resolution * 1.0;
    float acc_z_g = (float)acc_z / resolution * 1.0;

    // 计算倾斜角度
    float pitch_x = atan2(acc_y_g, sqrt(acc_x_g * acc_x_g + acc_z_g * acc_z_g)) * 180 / M_PI;
    float roll_y  = atan2(acc_x_g, sqrt(acc_y_g * acc_y_g + acc_z_g * acc_z_g)) * 180 / M_PI;
    float theta_z = atan2(sqrt(acc_x_g * acc_x_g + acc_y_g * acc_y_g), acc_z_g) * 180 / M_PI;
    //上次运动角度差
    float dax = 0;
    float day = 0;
    float daz = 0;

    // 获取角度差值
    dax = fabs(pitch_x - last_pitch_x);
    day = fabs(roll_y - last_roll_y);
    daz = fabs(theta_z - last_theta_z);

    Acceleration current ALIGNED(4) = {
        .x = acc_x_g,
        .y = acc_y_g,
        .z = acc_z_g
    };
#define SHAKE_CNT   3
    if (detect_shaking(&sample_data, &current, SHAKE_THRESHOLD)) {
        shake_begin_flag++;
        if (!shake_flag) {
            shake_flag = 1;
        }
        shake_end_flag = 0;
    } else {
noshake:
        shake_end_flag++;
        if (shake_end_flag >= SHAKE_CNT) {
            shake_begin_flag = 0;
        }
    }
    int start_play_time = 0;
//    printf("shake_begin_flag = %d , %d , %d\r\n", shake_begin_flag, shake_end_flag, shake_flag);
    if (shake_flag == 1 && shake_begin_flag == SHAKE_CNT) {
        printf("========Shake started\n");
        shake_begin_flag = 0;
        shake_end_flag = 0;
        shake_flag = 2;

        // 触发相应的事件
#ifdef CONFIG_LVGL_GAMES_ENABLE
        if (lv_demo_is_draw_lots_game_page()) {
#ifdef CONFIG_UI_ONLY_EYE
            play_face_emoji(AI_UART_CMD_EMOJI_DRAW_LOTS_SUCCESS);
#endif
        } else if (lv_demo_is_dice_game_page()) {
            // 骰子游戏处理
            die_shake_flag = true;
        } else {
#ifdef CONFIG_UI_ONLY_EYE
            play_face_emoji(AI_UART_CMD_EMOJI_SHAKE);
            os_time_dly(100);
#endif
        } else
#endif
            // 如果在骰子游戏页面，不播放表情29
            if (!keyworld_start) {
#ifdef CONFIG_UI_PLAY_EMOJI
                play_face_emoji(AI_UART_CMD_EMOJI_DIZZY);
                os_time_dly(100);
#endif
            }

    } else if (shake_flag == 2 && shake_end_flag >= SHAKE_CNT) {
        printf("Shake end\n");
        shake_begin_flag = 0;
        shake_end_flag = 0;
        shake_flag = 0;
        die_shake_flag = false;
    }

    // 更新上次角度值
    last_pitch_x = pitch_x;
    last_roll_y = roll_y;
    last_theta_z = theta_z;

//    printf("pitch_x: %.2f, roll_y: %.2f, theta_z: %.2f \r\n",
//           pitch_x, roll_y, theta_z);
}


// 传感器任务函数
static void sc7a20h_sensor_task(void *p)
{
    printf("sc7a20h_init\r\n");

    // // 初始化时从flash读取历史喝水量，但不做日期检查
    // cnt_all_dwv = user_water_intake_read();
    // printf("初始化时从flash读取历史喝水量: %d ml (日期检查将在任务中进行)\n", cnt_all_dwv);
    unsigned char sc7a20h_config(void);
    os_time_dly(100); // 延迟10秒
    sc7a20h_config();
    // 配置为高灵敏度模式
    sc7a20h_resolution_range(G_SENSITY_MEDIUM);

    // 任务启动时首先执行一次日期检查和重置
    // 但先等待系统充分初始化，确保时间已经更新
    bool date_checked = false;
    unsigned char time_retry_count = 0;

    // 使用时间稳定性检测
    struct sys_time last_time = {0};
    unsigned char stable_count = 0;
    unsigned int cnt = 0;
    while (1) {
        cnt++;
        // 正常的传感器数据处理
        sc7a20h_get_acceleration(cnt);
        os_time_dly(5);
    }
}

// 启动传感器任务
void sc7a20h_sensor_task_init(void)
{
    int ret = thread_fork("sc7a20h_sensor_task", 10, 4 * 1024, 0, 0, sc7a20h_sensor_task, NULL);
    if (ret == 0) {
        printf("SC7A20H传感器任务创建成功!\n");
    } else {
        printf("SC7A20H传感器任务创建失败! err:%d\n", ret);
    }
}

// 传感器初始化
void sc7a20h_init(void)
{
    printf("===== sc7a20h_init called =====\n");

    // 初始化完成后启动传感器任务
    sc7a20h_sensor_task_init();
}

// 传感器灵敏度设置
void sc7a20h_resolution_range(unsigned char range)
{
    g_current_sc7a20h_sensitivity = range; // 更新当前的灵敏度设置

    // 根据SC7A20H传感器的特性配置分辨率
    switch (range) {
    case G_SENSITY_HIGH:
        // ±2g模式
        sc7a20h_sensor_command(SC7A20H_CTRL_REG4, 0x00);
        break;
    case G_SENSITY_MEDIUM:
        // ±4g模式
        sc7a20h_sensor_command(SC7A20H_CTRL_REG4, 0x10);
        break;
    case G_SENSITY_LOW:
        // ±8g或±16g模式
        sc7a20h_sensor_command(SC7A20H_CTRL_REG4, 0x30);
        break;
    default:
        // 默认±2g模式
        sc7a20h_sensor_command(SC7A20H_CTRL_REG4, 0x00);
        break;
    }
}

// 设置工作模式
void sc7a20h_work_mode(unsigned char work_mode)
{
    switch (work_mode) {
    case G_NORMAL_MODE:
        // 正常模式
        sc7a20h_sensor_command(SC7A20H_CTRL_REG1, 0x57); // 开启XYZ轴，100Hz刷新率
        break;
    case G_LOW_POWER_MODE:
        // 低功耗模式
        sc7a20h_sensor_command(SC7A20H_CTRL_REG1, 0x47); // 开启XYZ轴，低功耗模式
        break;
    case G_SUSPEND_MODE:
        // 挂起模式
        sc7a20h_sensor_command(SC7A20H_CTRL_REG1, 0x00); // 关闭所有轴
        break;
    }
}

// 传感器检查函数
char sc7a20h_check(void)
{
    unsigned char reg_value1 = 0;
    unsigned char reg_value2 = 0;

    reg_value1 = sc7a20h_sensor_get_data(SC7A20H_WHO_AM_I);
    reg_value2 = sc7a20h_sensor_get_data(SC7A20H_VERSION);

    printf("SC7A20H_Check,reg_value1=0x%x reg_value2=0x%x!\r\n", reg_value1, reg_value2);

    if ((reg_value1 == 0x11) && (reg_value2 == 0x28)) {
        return 0;    // 检测成功返回0
    } else {
        return -1;    // 检测失败返回-1
    }
}

// 配置传感器
unsigned char sc7a20h_config(void)
{
    if (sc7a20h_check() != 0) {
        return 0; // 检测失败
    }

    // 配置电源模式
    sc7a20h_sensor_command(SC7A20H_CTRL_REG1, 0x00); // 关闭传感器电源
    sl_delay(5);

    // 软复位传感器
    sc7a20h_sensor_command(SC7A20H_CTRL_REG2, 0x40); // 软复位
    sl_delay(20);

    // 基本配置 - 正常模式
    sc7a20h_sensor_command(SC7A20H_CTRL_REG1, 0x57); // 开启XYZ轴，100Hz刷新率
//    sc7a20h_sensor_command(SC7A20H_CTRL_REG2, 0x00); // 开启XYZ轴，100Hz刷新率
//     sc7a20h_sensor_command(SC7A20H_CTRL_REG4, 0x88); // ±2g，高字节在高地址
    sc7a20h_sensor_command(SC7A20H_CTRL_REG4, 0x00); // ±2g，高字节在高地址
    // 设置低通滤波 - 50Hz截止频率
    sc7a20h_set_lpf_cutoff(LPF_CUTOFF_50HZ);

    sl_delay(10);

    return 1; // 配置成功
}

// 配置传感器灵敏度
int sc7a20h_gravity_sensity(unsigned char gsid)
{
    if (gsid == G_SENSOR_SCAN) {
        return 0;
    }

    if (gsid == G_SENSOR_HIGH) {
        sc7a20h_work_mode(G_NORMAL_MODE);
        sc7a20h_resolution_range(G_SENSITY_HIGH);
        // 添加高精度模式下的低通滤波设置
        sc7a20h_set_lpf_cutoff(LPF_CUTOFF_50HZ);
    }

    if (gsid == G_SENSOR_MEDIUM) {
        sc7a20h_work_mode(G_NORMAL_MODE);
        sc7a20h_resolution_range(G_SENSITY_MEDIUM);
        // 添加中等精度模式下的低通滤波设置
        sc7a20h_set_lpf_cutoff(LPF_CUTOFF_100HZ);
    }

    if (gsid == G_SENSOR_LOW) {
        sc7a20h_work_mode(G_NORMAL_MODE);
        sc7a20h_resolution_range(G_SENSITY_LOW);
        // 添加低精度模式下的低通滤波设置
        sc7a20h_set_lpf_cutoff(LPF_CUTOFF_200HZ);
    }

    if (gsid == G_SENSOR_CLOSE) {
        sc7a20h_init();
        sc7a20h_resolution_range(G_SENSITY_LOW);
        sc7a20h_work_mode(G_SUSPEND_MODE);
    }

    if (gsid == G_SENSOR_LOW_POWER_MODE) { // 低功耗
        sc7a20h_work_mode(G_LOW_POWER_MODE);
        sc7a20h_resolution_range(G_SENSITY_MEDIUM);
        // 低功耗模式下使用更高的截止频率以减少处理
        sc7a20h_set_lpf_cutoff(LPF_CUTOFF_200HZ);
    }

    return 0;
}

bool gsensor_is_device_shaking(void)
{
    return die_shake_flag;
}


// 注册传感器操作接口
_G_SENSOR_INTERFACE sc7a20h_ops = {
    .logo                    = "SC7A20H",
    .gravity_sensor_check    = sc7a20h_check,
    .gravity_sensor_init     = sc7a20h_init,
    .gravity_sensor_sensity  = sc7a20h_gravity_sensity,
};

REGISTER_GRAVITY_SENSOR(sc7a20h)
.gsensor_ops = &sc7a20h_ops,
};

#endif // CONFIG_SC7A20H_GSENSOR_ENABLE
