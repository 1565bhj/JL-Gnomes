#include "system/includes.h"
#include "asm/gpio.h"
#include "app_config.h"
#include "storage_device.h"
#include "generic/log.h"
#include "os/os_api.h"
#include "event/key_event.h"
#include "event/device_event.h"
#include "event/net_event.h"
#include "fs/fs.h"
#include "asm/pwm.h"
#include "device/device.h"
#include "ArcLib.h"
#include "syscfg/syscfg_id.h"

#ifdef IRARC_UART_ENABLE
static OS_MUTEX mutex;
extern int utc_timer_update_get(struct sys_time *time);
static u8 uart_buf[1024] __attribute__((aligned(4))); //用于串口接收缓存数据的循环buf
static u8 lrc_learning[240] __attribute__((aligned(4))); //用于接收空调码
static void *uart_hdl = NULL;
static char ARC_learn = 0;//1：空调学习，2：家电学习
static char ARC_learn_step = 0;//模式：1、电源 2、模式

#define HT16XX_TYPE             0x1
#define HT16XX_LEARN_SIZE       208

struct irarc_pararam { // 空调控制变量
    unsigned char buf[HT16XX_LEARN_SIZE];//空调码
    unsigned char arc_dev;//1空调设备，0普通家电设备
    unsigned int ARC_group;
} IR_ADR_PAR;

struct irarc_info { // 空调控制变量
    unsigned char ADR_MODE;
    unsigned char ADR_TMP;
    unsigned char ADR_FANLEV;
    unsigned char ADR_FANDIR;
    unsigned char ADR_AFANDIR;
    unsigned char ADR_MFANDIR;
    unsigned char ADR_Z0;
    unsigned char ADR_TIMEON;
    unsigned char ADR_TIMEOFF;
    unsigned char ADR_KEYVAL;
    unsigned char ADR_SYSFLAG;
    unsigned char ADR_Z1;
    unsigned char ADR_TMSTATE3;
    unsigned char ADR_Z2;
} IR_ADR_VAR;

extern int is_production_test_enter(char wake);
extern void ArcLibInit(void);
extern void Read_ARC_IR_Data(int ARC_group, unsigned char ADR_VAR[14], unsigned char ic_type);
extern void Arc_Identification(unsigned const char * p_arc_len_buffer, unsigned int * group_number, unsigned char ic_type);;

int ir_product_test_ok(void)
{
    return ARC_learn == 0;
}
static int irarc_write_data(unsigned char *data, int len)
{
    unsigned char start_cmd = 0;
    dev_write(uart_hdl, &start_cmd, 1);//1个字节0唤醒命令
    os_time_dly(1);
    dev_write(uart_hdl, data, len);
}
static int irarc_data_init(void)
{
    Read_ARC_IR_Data(IR_ADR_PAR.ARC_group, ADR_VAR, HT16XX_TYPE);//按键处理，包括数据解析，红外数据分析输出
    memcpy(&IR_ADR_VAR, ADR_VAR, 14);

    IR_ADR_VAR.ADR_MODE = ARC_MODE_COOL; //模式
    IR_ADR_VAR.ADR_TMP = ARC_TMP_25; //温度
    IR_ADR_VAR.ADR_FANLEV = ARC_FANLEV_1; //自动风量
    IR_ADR_VAR.ADR_FANDIR = ARC_FANDIR_1; //自动风向
    IR_ADR_VAR.ADR_AFANDIR = ARC_AFANDIR_1; //自动风向
    IR_ADR_VAR.ADR_MFANDIR = ARC_MFANDIR_1; //手动风向
    IR_ADR_VAR.ADR_Z0 = 0;
    IR_ADR_VAR.ADR_TIMEON = 0; //定时开时间
    IR_ADR_VAR.ADR_TIMEOFF = 0; //定时关时间
    IR_ADR_VAR.ADR_KEYVAL = ARC_NO_KEY; //键值信息
    IR_ADR_VAR.ADR_SYSFLAG = 0x1; //功能标志
    IR_ADR_VAR.ADR_Z1 = 0;
    IR_ADR_VAR.ADR_TMSTATE3 = 0; //定时状态
    IR_ADR_VAR.ADR_Z2 = 0;
    return 0;
}
static unsigned char irarc_data_sum(unsigned char *data, int len)
{
    unsigned char sum = 0;
    while (len--) {
        sum += *data++;
    }
    return sum;
}
static unsigned char irarc_data_sum_check(unsigned char *data)
{
    unsigned char sum = 0;
    int len = *(data + 2);
    unsigned char sum_check = *(data + len + 3);
    len += 3;
    while (len--) {
        sum += *data++;
    }
    if (sum_check != sum) {
        printf("-> irarc_data_sum_check err , sum = 0x%x, 0x%x\n", sum, sum_check);
    }
    return sum_check == sum;
}
void irarc_arc_lrc_learning(char first, char note)//空调开始学习
{
    int len = 0;
    printf("-> %s arc_dev = %d\n", __func__, IR_ADR_PAR.arc_dev);
    if (note && !is_production_test_enter(0)) {
        music_play_res_file("IrLearn.mp3");
    }
    if (ARC_learn == 2) {
        irarc_aurc_exit(0);
        os_time_dly(10);
    }
    os_mutex_pend(&mutex, 60000);
    ARC_learn = 1;
    if (first) {
        ARC_learn_step = 0;
    }
    IR_ADR_PAR.arc_dev = 1;
    memset(lrc_learning, 0, sizeof(lrc_learning));
    lrc_learning[0] = 0x16;
    lrc_learning[1] = 0x0c;
    lrc_learning[2] = len;
    lrc_learning[3 + len] = irarc_data_sum(lrc_learning, 3 + len);
    lrc_learning[3 + len + 1] = 0x08;
    irarc_write_data(lrc_learning, len + 3 + 1 + 1);
    os_mutex_post(&mutex);
    put_buf(lrc_learning, len + 3 + 1 + 1);
}
static void irarc_arc_lrc_send(char *buf)//空调码发送命令
{
    int len = HT16XX_LEARN_SIZE;
    printf("-> %s arc_dev = %d\n", __func__, IR_ADR_PAR.arc_dev);
    memset(lrc_learning, 0, sizeof(lrc_learning));
    lrc_learning[0] = 0x16;
    lrc_learning[1] = 0x0d;
    lrc_learning[2] = len;
    memcpy(&lrc_learning[3], buf ? buf : ArcIRdataBuff, len);
    lrc_learning[3 + len] = irarc_data_sum(lrc_learning, 3 + len);
    lrc_learning[3 + len + 1] = 0x08;
    irarc_write_data(lrc_learning, len + 3 + 1 + 1);
    put_buf(lrc_learning, HT16XX_LEARN_SIZE + 5);
}
void irarc_urc_lrc_learning(char first, char note)//普通家电开始学习
{
    int len = 0;
    if (note && !is_production_test_enter(0)) {
        music_play_res_file("IrLearn.mp3");
    }
    if (ARC_learn == 1) {
        irarc_aurc_exit(0);
        os_time_dly(10);
    }
    os_mutex_pend(&mutex, 60000);
    ARC_learn = 2;
    if (first) {
        ARC_learn_step = 0;
    }
    IR_ADR_PAR.arc_dev = 0;
    printf("-> %s arc_dev = %d\n", __func__, IR_ADR_PAR.arc_dev);
    memset(lrc_learning, 0, sizeof(lrc_learning));
    lrc_learning[0] = 0x16;
    lrc_learning[1] = 0x06;
    lrc_learning[2] = len;
    lrc_learning[3 + len] = irarc_data_sum(lrc_learning, 3 + len);
    lrc_learning[3 + len + 1] = 0x08;
    irarc_write_data(lrc_learning, len + 3 + 1 + 1);
    os_mutex_post(&mutex);
    put_buf(lrc_learning, len + 3 + 1 + 1);
}
static void irarc_urc_lrc_send(char *buf)//普通家电码发送命令
{
    printf("-> %s arc_dev = %d\n", __func__, IR_ADR_PAR.arc_dev);
    int len = HT16XX_LEARN_SIZE;
    memset(lrc_learning, 0, sizeof(lrc_learning));
    lrc_learning[0] = 0x16;
    lrc_learning[1] = 0x07;
    lrc_learning[2] = len;
    memcpy(&lrc_learning[3], buf ? buf : ArcIRdataBuff, len);
    lrc_learning[3 + len] = irarc_data_sum(lrc_learning, 3 + len);
    lrc_learning[3 + len + 1] = 0x08;
    irarc_write_data(lrc_learning, len + 3 + 1 + 1);
    put_buf(lrc_learning, HT16XX_LEARN_SIZE + 5);
}
void irarc_aurc_exit(char note)
{
    unsigned char *pbuf = malloc(400);
    if (note && !is_production_test_enter(0)) {
        music_play_res_file("ExtLearn.mp3");
    }
    os_mutex_pend(&mutex, 60000);
    ARC_learn = 0;
    ARC_learn_step = 0;
    IR_ADR_PAR.arc_dev = 0;
    if (pbuf) {
        memset(pbuf, 0, 400);
        dev_write(uart_hdl, pbuf, 400);//400个0退出学习或者发码
        free(pbuf);
    } else {
        unsigned char data = 0;
        for (int i = 0; i < 400; i++) {
            dev_write(uart_hdl, &data, 1);//400个0退出学习或者发码
        }
    }
    os_mutex_post(&mutex);
    printf("-> %s arc_dev = %d\n", __func__, IR_ADR_PAR.arc_dev);
}
static int irarc_data_read(void)
{
    if (syscfg_read(CFG_USER_IRARC_INDEX, &IR_ADR_PAR, sizeof(IR_ADR_PAR)) < 0) { //没有配置空调
        IR_ADR_PAR.ARC_group = 0xFFFF;
        music_play_res_file("ArcNotFond.mp3");
        printf("-> err no ARC_group\n");
        return -1;
    }
    return 0;
}
static int irurc_data_read(char index)
{
    if (syscfg_read(CFG_USER_IRARC_KEY0 + index, &IR_ADR_PAR, sizeof(IR_ADR_PAR)) < 0) { //没有配置空调
        IR_ADR_PAR.ARC_group = 0xFFFF;
        music_play_res_file("ArcNotFond.mp3");
        printf("-> err no ARC_group\n");
        return -1;
    }
    printf("-> %s arc_dev = %d\n", __func__, IR_ADR_PAR.arc_dev);
    return 0;
}
static void irarc_urc_command_send(char index)//发送命令
{
    printf("-> %s arc_dev = %d\n", __func__, IR_ADR_PAR.arc_dev);
    if (IR_ADR_PAR.arc_dev) {
        memcpy(ADR_VAR, &IR_ADR_VAR, sizeof(ADR_VAR));
        Read_ARC_IR_Data(IR_ADR_PAR.ARC_group, ADR_VAR, HT16XX_TYPE);
        if (ArcIRdataBuff[0] != 0) {
            irarc_arc_lrc_send(NULL);
        } else { //不存在空调
            printf("-> Read_ARC_IR_Data err \n");
        }
    } else {
        printf("-> irurc_data_read \n");
        if (!irurc_data_read(index)) {
            irarc_urc_lrc_send(&IR_ADR_PAR.buf);
        } else { //不存在空调
            printf("-> irurc_data_read err \n");
        }
    }
}

int irarc_open(void)//打开空调
{
    int err = 0;
    printf("-> %s %d\n", __func__, __LINE__);
    os_mutex_pend(&mutex, 60000);
    if (IR_ADR_VAR.ADR_MODE == 0) {
        irarc_data_init();
    }
    if (!irarc_data_read()) {
        err = 1;
        printf("-> %s arc_dev = %d\n", __func__, IR_ADR_PAR.arc_dev);
        if (IR_ADR_PAR.arc_dev) {
            if (IR_ADR_PAR.ARC_group != 0xFFFF) {
                IR_ADR_VAR.ADR_KEYVAL = 0X00;//（开关键）
                IR_ADR_VAR.ADR_SYSFLAG &= 0x20;//00100000B;// (清除灯光以外的所有功能标志)
                IR_ADR_VAR.ADR_SYSFLAG |= 0X01;//（或上开机标志）
                irarc_urc_command_send(0);
            }
        } else {
            irarc_urc_command_send(0);
        }
    }
ext:
    os_mutex_post(&mutex);
    return err;
}
void irarc_close(void)//关闭空调
{
    printf("-> %s %d\n", __func__, __LINE__);
    os_mutex_pend(&mutex, 60000);
    if (!irarc_data_read()) {
        printf("-> IR_ADR_PAR.arc_dev = %d\n", IR_ADR_PAR.arc_dev);
        if (IR_ADR_PAR.arc_dev) {
            IR_ADR_VAR.ADR_KEYVAL = 0X00;// （开关键）
            IR_ADR_VAR.ADR_SYSFLAG &= 0x20;//00100000B;// (清除灯光以外的所有功能标志)
            irarc_urc_command_send(0);
        } else {
            irarc_urc_command_send(0);
        }
    }
    os_mutex_post(&mutex);
}
int irarc_cold(void)//制冷模式
{
    printf("-> %s %d\n", __func__, __LINE__);
    int err = 0;
    os_mutex_pend(&mutex, 60000);
    if (!irarc_data_read()) {
        printf("-> IR_ADR_PAR.arc_dev = %d\n", IR_ADR_PAR.arc_dev);
        if (IR_ADR_PAR.arc_dev) {
            IR_ADR_VAR.ADR_MODE = ARC_MODE_COOL;// （制冷模式）
            IR_ADR_VAR.ADR_KEYVAL = ARC_KEY_MODE;// （模式键）
            irarc_urc_command_send(1);
        } else {
            irarc_urc_command_send(1);
        }
        err = 1;
    }
    os_mutex_post(&mutex);
    return err;
}
int irarc_heating(void)//制热模式
{
    printf("-> %s %d\n", __func__, __LINE__);
    int err = 0;
    os_mutex_pend(&mutex, 60000);
    if (!irarc_data_read()) {
        printf("-> IR_ADR_PAR.arc_dev = %d\n", IR_ADR_PAR.arc_dev);
        if (IR_ADR_PAR.arc_dev) {
            IR_ADR_VAR.ADR_MODE = ARC_MODE_HEAT;// （制热模式）
            IR_ADR_VAR.ADR_KEYVAL = ARC_KEY_MODE;// （模式键）
            irarc_urc_command_send(2);
        } else {
            irarc_urc_command_send(2);
        }
        err = 1;
    }
    os_mutex_post(&mutex);
    return err;
}
int irarc_dehumidification(void)//除湿模式
{
    printf("-> %s %d\n", __func__, __LINE__);
    int err = 0;
    os_mutex_pend(&mutex, 60000);
    if (!irarc_data_read()) {
        printf("-> IR_ADR_PAR.arc_dev = %d\n", IR_ADR_PAR.arc_dev);
        if (IR_ADR_PAR.arc_dev) {
            IR_ADR_VAR.ADR_MODE = ARC_MODE_DHMD;//除湿模式
            IR_ADR_VAR.ADR_KEYVAL = ARC_KEY_MODE;// （模式键）
            irarc_urc_command_send(3);
        } else {
            irarc_urc_command_send(3);
        }
        err = 1;
    }
    os_mutex_post(&mutex);
    return err;
}
int irarc_energy_saving(void)//节能模式
{
    printf("-> %s %d\n", __func__, __LINE__);
    int err = 0;
    os_mutex_pend(&mutex, 60000);
    if (!irarc_data_read()) {
        printf("-> IR_ADR_PAR.arc_dev = %d\n", IR_ADR_PAR.arc_dev);
        if (IR_ADR_PAR.arc_dev) {
            IR_ADR_VAR.ADR_KEYVAL = ARC_KEY_ECONOMIC;//（经济键）
            IR_ADR_VAR.ADR_SYSFLAG &= 0xEF;//11101111B;// (清除强力标志)
            IR_ADR_VAR.ADR_SYSFLAG |= 0x80;//10000000B;// （或上经济标志）
            irarc_urc_command_send(4);
        } else {
            irarc_urc_command_send(4);
        }
        err = 1;
    }
    os_mutex_post(&mutex);
    return err;
}
int irarc_air_suplly(void)//送风模式
{
    printf("-> %s %d\n", __func__, __LINE__);
    int err = 0;
    os_mutex_pend(&mutex, 60000);
    if (!irarc_data_read()) {
        printf("-> IR_ADR_PAR.arc_dev = %d\n", IR_ADR_PAR.arc_dev);
        if (IR_ADR_PAR.arc_dev) {
            IR_ADR_VAR.ADR_MODE = ARC_MODE_FAN;// （送风模式）
            IR_ADR_VAR.ADR_KEYVAL = ARC_KEY_MODE;// （模式键）
            irarc_urc_command_send(5);
        } else {
            irarc_urc_command_send(5);
        }
        err = 1;
    }
    os_mutex_post(&mutex);
    return err;
}
int irarc_sleep(void)//睡眠模式
{
    printf("-> %s %d\n", __func__, __LINE__);
    int err = 0;
    os_mutex_pend(&mutex, 60000);
    if (!irarc_data_read()) {
        printf("-> IR_ADR_PAR.arc_dev = %d\n", IR_ADR_PAR.arc_dev);
        if (IR_ADR_PAR.arc_dev) {
            IR_ADR_VAR.ADR_KEYVAL = ARC_KEY_SLEEP;//（睡眠键）
            IR_ADR_VAR.ADR_SYSFLAG &= 0xEF;//11101111B;// (清除强力标志)
            IR_ADR_VAR.ADR_SYSFLAG |= 0x02;//00000010B;// （或上睡眠标志）
            irarc_urc_command_send(6);
        } else {
            irarc_urc_command_send(6);
        }
        err = 1;
    }
    os_mutex_post(&mutex);
    return err;
}
int irarc_powerful(void)//强力模式
{
    printf("-> %s %d\n", __func__, __LINE__);
    int err = 0;
    os_mutex_pend(&mutex, 60000);
    if (!irarc_data_read() && IR_ADR_PAR.arc_dev) {
        printf("-> IR_ADR_PAR.arc_dev = %d\n", IR_ADR_PAR.arc_dev);
        IR_ADR_VAR.ADR_SYSFLAG &= 0x3F;//00111111B;// (清除空清/经济标志）
        IR_ADR_VAR.ADR_SYSFLAG |= 0x10;//00010000B;// （或上强力标志）
        IR_ADR_VAR.ADR_KEYVAL = ARC_KEY_STRONG;//（强力键）
        irarc_urc_command_send(7);
        err = 1;
    }
    os_mutex_post(&mutex);
    return err;
}
int irarc_temperature_inc(void)//升高温度
{
    printf("-> %s %d\n", __func__, __LINE__);
    int err = 0;
    os_mutex_pend(&mutex, 60000);
    if (!irarc_data_read()) {
        if (IR_ADR_PAR.arc_dev) {
            IR_ADR_VAR.ADR_TMP++;// （温度加 1）
            IR_ADR_VAR.ADR_KEYVAL = ARC_KEY_TUP;// （温度+键）
            irarc_urc_command_send(7);
        } else {
            irarc_urc_command_send(7);
        }
        err = 1;
    }
    os_mutex_post(&mutex);
    return err;
}
int irarc_temperature_dec(void)//降低温度
{
    printf("-> %s %d\n", __func__, __LINE__);
    int err = 0;
    os_mutex_pend(&mutex, 60000);
    if (!irarc_data_read()) {
        if (IR_ADR_PAR.arc_dev) {
            IR_ADR_VAR.ADR_TMP--;// （温度减 1）
            IR_ADR_VAR.ADR_KEYVAL = ARC_KEY_TDOWN;// （温度+键）
            irarc_urc_command_send(8);
        } else {
            irarc_urc_command_send(8);
        }
        err = 1;
    }
    os_mutex_post(&mutex);
    return err;
}
int irarc_max_wind(void)//最大风量
{
    printf("-> %s %d\n", __func__, __LINE__);
    int err = 0;
    os_mutex_pend(&mutex, 60000);
    if (!irarc_data_read()) {
        if (IR_ADR_PAR.arc_dev) {
            IR_ADR_VAR.ADR_FANLEV = ARC_FANLEV_4;// （高风量）
            IR_ADR_VAR.ADR_KEYVAL = ARC_KEY_FANLEV;// （风量键）
            irarc_urc_command_send(9);
        } else {
            irarc_urc_command_send(9);
        }
        err = 1;
    }
    os_mutex_post(&mutex);
    return err;
}
int irarc_min_wind(void)//最小风量
{
    printf("-> %s %d\n", __func__, __LINE__);
    int err = 0;
    os_mutex_pend(&mutex, 60000);
    if (!irarc_data_read()) {
        if (IR_ADR_PAR.arc_dev) {
            IR_ADR_VAR.ADR_FANLEV = ARC_FANLEV_1;// （低风量）
            IR_ADR_VAR.ADR_KEYVAL = ARC_KEY_FANLEV;// （风量键）
            irarc_urc_command_send(10);
        } else {
            irarc_urc_command_send(10);
        }
        err = 1;
    }
    os_mutex_post(&mutex);
    return err;
}
int irarc_temperature_set(int tmp)//设置温度 16 - 32
{
    printf("-> %s %d\n", __func__, __LINE__);
    int err = 0;
    os_mutex_pend(&mutex, 60000);
    if (!irarc_data_read()) {
        /*
        注：如果之前的温度小于新设定温度赋“温度+键”，如果之前的温度大于新设定温度赋“温度-键”，
        如果之前的温度等于新设定温度付“无效键”
        */
        if (tmp >= 16 && tmp <= 32) {
            if (tmp < IR_ADR_VAR.ADR_TMP) {
                IR_ADR_VAR.ADR_KEYVAL = ARC_KEY_TUP;// （温度+键）
            } else if (tmp > IR_ADR_VAR.ADR_TMP) {
                IR_ADR_VAR.ADR_KEYVAL = ARC_KEY_TDOWN;//（温度-键）
            } else {
                IR_ADR_VAR.ADR_KEYVAL = ARC_NO_KEY;// (无效键)
            }
            IR_ADR_VAR.ADR_TMP = tmp - ARC_TMP_16;//
            irarc_urc_command_send(0xFF);
        }
        err = 1;
    }
    os_mutex_post(&mutex);
    return err;
}
int irarc_swing_left_and_right(void)//左右摆动
{
    printf("-> %s %d\n", __func__, __LINE__);
    int err = 0;
    os_mutex_pend(&mutex, 60000);
    if (!irarc_data_read()) {
        if (IR_ADR_VAR.ADR_AFANDIR < 9) {// （风向值无法做成统一，调节风向时要 0—9 范围测试）
            IR_ADR_VAR.ADR_AFANDIR++;
        } else {
            IR_ADR_VAR.ADR_AFANDIR = 0;
        }
        IR_ADR_VAR.ADR_FANDIR = IR_ADR_VAR.ADR_AFANDIR;// （ADR_FANDIR 同步变化）
        IR_ADR_VAR.ADR_KEYVAL = ARC_KEY_AFANDIR;//（自动风向键）
        irarc_urc_command_send(0xFF);
        err = 1;
    }
    os_mutex_post(&mutex);
    return err;
}
int irarc_swing_auto(void)//自动风向
{
    printf("-> %s %d\n", __func__, __LINE__);
    int err = 0;
    os_mutex_pend(&mutex, 60000);
    if (!irarc_data_read()) {
        if (IR_ADR_VAR.ADR_AFANDIR < 9) {// （风向值无法做成统一，调节风向时要 0—9 范围测试）
            IR_ADR_VAR.ADR_AFANDIR ++;
        } else {
            IR_ADR_VAR.ADR_AFANDIR = 0;
        }
        IR_ADR_VAR.ADR_FANDIR = IR_ADR_VAR.ADR_AFANDIR;// （ADR_FANDIR 同步变化）
        IR_ADR_VAR.ADR_MFANDIR = 0;// （清除手动风向）
        IR_ADR_VAR.ADR_KEYVAL = ARC_KEY_AFANDIR;//（自动风向键）
        irarc_urc_command_send(0xFF);
        err = 1;
    }
    os_mutex_post(&mutex);
    return err;
}
int irarc_swing_up_and_down(void)//上下摆动
{
    printf("-> %s %d\n", __func__, __LINE__);
    int err = 0;
    os_mutex_pend(&mutex, 60000);
    if (!irarc_data_read()) {
        if (IR_ADR_VAR.ADR_MFANDIR < 9) {// （风向值无法做成统一，调节风向时要 0—9 范围测试）
            IR_ADR_VAR.ADR_MFANDIR ++;
        } else {
            IR_ADR_VAR.ADR_MFANDIR = 0;
        }
        IR_ADR_VAR.ADR_FANDIR = IR_ADR_VAR.ADR_MFANDIR;//（ADR_FANDIR 同步变化）
        IR_ADR_VAR.ADR_AFANDIR = 0;//（清除自动风向）
        IR_ADR_VAR.ADR_KEYVAL = ARC_KEY_MFANDIR;//（手动风向键）
        irarc_urc_command_send(0xFF);
        err = 1;
    }
    os_mutex_post(&mutex);
    return err;
}
int irarc_swing_manual(void)//手动风向
{
    printf("-> %s %d\n", __func__, __LINE__);
    int err = 0;
    os_mutex_pend(&mutex, 60000);
    if (!irarc_data_read()) {
        if (IR_ADR_VAR.ADR_MFANDIR < 9) {// （风向值无法做成统一，调节风向时要 0—9 范围测试）
            IR_ADR_VAR.ADR_MFANDIR ++;
        } else {
            IR_ADR_VAR.ADR_MFANDIR = 0;
        }
        IR_ADR_VAR.ADR_FANDIR = IR_ADR_VAR.ADR_MFANDIR;//（ADR_FANDIR 同步变化）
        IR_ADR_VAR.ADR_AFANDIR = 0;//（清除自动风向）
        IR_ADR_VAR.ADR_KEYVAL = ARC_KEY_MFANDIR;//（手动风向键）
        irarc_urc_command_send(0xFF);
        err = 1;
    }
    os_mutex_post(&mutex);
    return err;
}
void irarc_timing_open(int times)//定时打开
{
    printf("-> %s %d\n", __func__, __LINE__);
    os_mutex_pend(&mutex, 60000);
    if (!irarc_data_read()) {
        if (times <= 12) {
            IR_ADR_VAR.ADR_TIMEON = times;// （times小时）
            IR_ADR_VAR.ADR_KEYVAL = ARC_KEY_TIMEON;// （定开键）
            IR_ADR_VAR.ADR_SYSFLAG = 0x08;//00001000B;// （置定时标志，清除开机标志）
            IR_ADR_VAR.ADR_TMSTATE3 = 1;// （有定开）
            irarc_urc_command_send(0xFF);
        }
    }
    os_mutex_post(&mutex);
}
void irarc_timing_close(int times)//定时关闭
{
    printf("-> %s %d\n", __func__, __LINE__);
    os_mutex_pend(&mutex, 60000);
    if (!irarc_data_read()) {
        if (times <= 12) {
            IR_ADR_VAR.ADR_TIMEON = times;// （times小时）
            IR_ADR_VAR.ADR_KEYVAL = ARC_KEY_TIMEOFF;// （定关键）
            IR_ADR_VAR.ADR_SYSFLAG |= 0x08;//00001000B;// （置定时标志，清除开机标志）
            IR_ADR_VAR.ADR_TMSTATE3 = 2;// （有定关）
            irarc_urc_command_send(0xFF);
        }
    }
    os_mutex_post(&mutex);
}
void irarc_timing_exit(void)//关闭定时
{
    printf("-> %s %d\n", __func__, __LINE__);
    os_mutex_pend(&mutex, 60000);
    if (!irarc_data_read()) {
        IR_ADR_VAR.ADR_KEYVAL = ARC_KEY_TIMECL;// （取消定时键）
        irarc_urc_command_send(0xFF);
    }
    os_mutex_post(&mutex);
}

int irarc_production_test_send(void)
{
    IR_ADR_PAR.arc_dev = 1;
    IR_ADR_PAR.ARC_group = 0x1;
    irarc_data_init();
    IR_ADR_VAR.ADR_KEYVAL = 0X00;//（开关键）
    IR_ADR_VAR.ADR_SYSFLAG &= 0x20;//00100000B;// (清除灯光以外的所有功能标志)
    IR_ADR_VAR.ADR_SYSFLAG |= 0X01;//（或上开机标志）
    irarc_urc_command_send(0);
    return 0;
}

static void uart_task_main(void *priv)
{
#define RECV_SIZE   240
    unsigned int cnt = 0;
    unsigned char *recv_buf = malloc(RECV_SIZE);
    unsigned int grup[10];
    int len;

    uart_hdl = dev_open("uart0", NULL);
    if (!uart_hdl || !recv_buf) {
        printf("open uart err !!!\n");
        return ;
    }
    /* 1 . 设置串口接收缓存数据的循环buf地址 */
    dev_ioctl(uart_hdl, UART_SET_CIRCULAR_BUFF_ADDR, (int)uart_buf);

    /* 1 . 设置串口接收缓存数据的循环buf长度 */
    dev_ioctl(uart_hdl, UART_SET_CIRCULAR_BUFF_LENTH, sizeof(uart_buf));

    /* 3 . 设置接收数据为阻塞方式,需要非阻塞可以去掉,建议加上超时设置 */
    dev_ioctl(uart_hdl, UART_SET_RECV_BLOCK, 1);

    u32 parm = 1000;
    dev_ioctl(uart_hdl, UART_SET_RECV_TIMEOUT, (u32)parm); //超时设置

    /* 4 . 使能特殊串口,启动收发数据 */
    dev_ioctl(uart_hdl, UART_START, 0);

    os_mutex_create(&mutex);

    ArcLibInit();

    irarc_aurc_exit(0);

    while (1) {
        /* 5 . 接收数据 */
        len = dev_read(uart_hdl, recv_buf, RECV_SIZE);
        if (len <= 0) {
//            printf("\n  uart recv err len = %d\n", len);
            if (len == UART_CIRCULAR_BUFFER_WRITE_OVERLAY) {
                printf("\n UART_CIRCULAR_BUFFER_WRITE_OVERLAY err\n");
                dev_ioctl(uart_hdl, UART_FLUSH, 0); //如果由于用户长期不取走接收的数据导致循环buf接收回卷覆盖,因此直接冲掉循环buf所有数据重新接收
            } else if (len == UART_RECV_TIMEOUT) {
//                puts("UART_RECV_TIMEOUT...\r\n");
            }
            continue;
        } else {
            printf("->uart recv  len = %d\n", len);
//            put_buf(recv_buf, len);
            if (recv_buf[0] == 0x16 && (recv_buf[1] == 0x8c || recv_buf[1] == 0x86) && recv_buf[2] == HT16XX_LEARN_SIZE && len > HT16XX_LEARN_SIZE) { //学习空调OK
                os_mutex_pend(&mutex, 60000);
                memset(grup, 0, sizeof(grup));
                put_buf(&recv_buf[3], HT16XX_LEARN_SIZE);
                if (irarc_data_sum_check(recv_buf)) {
                    Arc_Identification(&recv_buf[3], &grup, HT16XX_TYPE);
                    if (is_production_test_enter(2) && IR_ADR_PAR.arc_dev) {
                        for (int i = 0; i < 2; i++) {
                            if (grup[0] != 0xFFFF) {
                                IR_ADR_PAR.ARC_group = grup[0];
                                irarc_data_init();
                                if (IR_ADR_PAR.arc_dev && ArcIRdataBuff[0] != 0) {
                                    IR_ADR_VAR.ADR_KEYVAL = 0X00;//（开关键）
                                    IR_ADR_VAR.ADR_SYSFLAG &= 0x20;//00100000B;// (清除灯光以外的所有功能标志)
                                    IR_ADR_VAR.ADR_SYSFLAG |= 0X01;//（或上开机标志）
                                    irarc_urc_command_send(0);
                                } else {
                                    irarc_urc_lrc_send(&recv_buf[3]);
                                }
                            } else {
                                irarc_urc_lrc_send(&recv_buf[3]);
                            }
                        }
                        irarc_aurc_exit(0);
                        continue;
                    }
                    if (grup[0] != 0xFFFF && IR_ADR_PAR.arc_dev) {
                        printf("-> grup[0] = 0x%x \n", grup[0]);
                        IR_ADR_PAR.ARC_group = grup[0];
                        irarc_data_init();
                        if (ArcIRdataBuff[0] != 0) {
                            syscfg_write(CFG_USER_IRARC_INDEX, &IR_ADR_PAR, sizeof(IR_ADR_PAR));
                        } else {
                            printf("err in ArcIRdataBuff \n");
                        }
                    } else {
                        if (ARC_learn_step > 10) {
                            ARC_learn_step = 10;
                        }
                        memcpy(&IR_ADR_PAR.buf, &recv_buf[3], HT16XX_LEARN_SIZE);
                        syscfg_write(CFG_USER_IRARC_INDEX, &IR_ADR_PAR, sizeof(IR_ADR_PAR));
                        syscfg_write(CFG_USER_IRARC_KEY0 + ARC_learn_step, &IR_ADR_PAR, sizeof(IR_ADR_PAR));
                        printf("err no found in grup \n");
                    }
                    printf("send ARC_learn_step = %d\n", ARC_learn_step);
                    ARC_learn_step++;
                    music_play_res_file("IrLearnOk.mp3");
                }
                os_mutex_post(&mutex);
                printf("-> IR_ADR_PAR.arc_dev = %d\n", IR_ADR_PAR.arc_dev);
                if (IR_ADR_PAR.arc_dev) {
                    irarc_arc_lrc_learning(0, 0);
                } else {
                    irarc_urc_lrc_learning(0, 0);
                }
                //irarc_aurc_exit(1);
            } else if (recv_buf[0] == 0x16 && (recv_buf[1] == 0x8d || recv_buf[1] == 0x87)) {
                //irarc_aurc_exit(1);
                printf("send command ok\n");
            } else if (recv_buf[0] == 0x16 && (recv_buf[1] == 0xcc || recv_buf[1] == 0xc6)) {
                if (IR_ADR_PAR.arc_dev) {
                    irarc_arc_lrc_learning(0, 0);
                } else {
                    irarc_urc_lrc_learning(0, 0);
                }
                printf("err in start learning, redo\n");
            }
        }
    }
    dev_close(uart_hdl);
}
static int uart_task_init(void)
{
    if (production_test_io_get()) {
        return 0;
    }
    os_task_create(uart_task_main, NULL, 10, 1000, 0, "ir_uart_task");
    return 0;
}
late_initcall(uart_task_init);
#endif
