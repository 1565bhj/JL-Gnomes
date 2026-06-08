/***************************************************************************
*****************************************************************************/
#ifndef _ARC_LIB_H
#define _ARC_LIB_H
/*****************************************************************************/


#define ADR_VAR_LEN     14
extern unsigned char    ADR_VAR[ADR_VAR_LEN];       //空调状态变量数组
/* 空调变量数组ADR_VAR[14]说明 */
//#define   ADR_MODE        0       //模式------------》变化范围"0~4"对应模式详见"模式常量"定义
//#define   ADR_TMP         1       //温度------------》变化范围"0~16"对应温度"16~32"
//#define   ADR_FANLEV      2       //风量------------》变化范围"0~3"对应风量"自动，低，中，高"
//#define   ADR_FANDIR      3       //风向------------》按自动风向键清0，按手动风向键加1（在1~8之间变化）（用于自动风向和手动风向数据在同一位置）
//#define   ADR_AFANDIR     4       //自动风向----》按自动风向键改变（在0~8之间变化），按手动风向键不变（用于自动风向和手动风向数据在不同位置）
//#define   ADR_MFANDIR     5       //手动风向----》按手动风向键改变（在0~8之间变化），按自动风向键不变（用于自动风向和手动风向数据在不同位置,或只有一个风向键的时候）
//#define   ADR_CKHOUR      6       //时钟小时
//#define   ADR_TIMEON      7       //定开时间（小时）----》变化范围"0~12"对应定开时间"0~12"小时（只要用于定开定关数据在不同位置的时候）
//#define   ADR_TIMEOFF     8       //定关时间（小时）----》变化范围"0~12"对应定关时间"0~12"小时（只要用于定开定关数据在不同位置的时候）
//#define   ADR_KEYVAL      9       //键值------------》详见"按键常量"定义,空键:0FFH
//#define   ADR_SYSFLAG     10      //功能标志---》详见"功能标志"常量定义
//#define   ADR_TMSTATE7    11      //定时状态（7种），详见"定时状态常量（7态）"
//#define   ADR_TMSTATE3    12      //定时状态（3种），详见"定时状态常量（3态）"
//#define   ADR_CKMIN       13      //时钟分钟
/**************************************************************************************************************
*空调变量 取值定义*
***************************************************************************************************************/
/*ARC 温度常量*/
#define ARC_TMP_L       0       //最低温度为0+16摄氏度
#define ARC_TMP_H       16      //最高温度为16+16摄氏度
/////
#define ARC_TMP_16  0  //16摄氏度
#define ARC_TMP_17  1  //17摄氏度
#define ARC_TMP_18  2  //18摄氏度
#define ARC_TMP_19  3  //19摄氏度
#define ARC_TMP_20  4  //20摄氏度
#define ARC_TMP_21  5  //21摄氏度
#define ARC_TMP_22  6  //22摄氏度
#define ARC_TMP_23  7  //23摄氏度
#define ARC_TMP_24  8  //24摄氏度
#define ARC_TMP_25  9  //25摄氏度
#define ARC_TMP_26  10  //26摄氏度
#define ARC_TMP_27  11  //27摄氏度
#define ARC_TMP_28  12  //28摄氏度
#define ARC_TMP_29  13  //29摄氏度
#define ARC_TMP_30  14  //30摄氏度
#define ARC_TMP_31  15  //31摄氏度
#define ARC_TMP_32  16  //32摄氏度

/*ARC 模式常量 ADR_MODE*/
#define ARC_MODE_AUTO   0       //自动
#define ARC_MODE_COOL   1       //制冷
#define ARC_MODE_DHMD   2       //抽湿
#define ARC_MODE_FAN    3       //送风
#define ARC_MODE_HEAT   4       //制热
/*ARC ADR_FANDIR风向常量*/
#define ARC_FANDIR_1    0       //自动风向
#define ARC_FANDIR_2    1       //手动1
#define ARC_FANDIR_3    2       //手动2
#define ARC_FANDIR_4    3       //手动3
#define ARC_FANDIR_5    4       //手动4
#define ARC_FANDIR_6    5       //手动5
#define ARC_FANDIR_7    6       //手动6
#define ARC_FANDIR_8    7       //手动7
#define ARC_FANDIR_9    8       //手动8
#define ARC_FANDIR_10   9       //手动9
/*ARC  ADR_AFANDIR 自动风向常量*/
#define ARC_AFANDIR_1   0       //自动风向1
#define ARC_AFANDIR_2   1       //自动风向2
#define ARC_AFANDIR_3   2       //自动风向3
#define ARC_AFANDIR_4   3       //自动风向4
#define ARC_AFANDIR_5   4       //自动风向5
#define ARC_AFANDIR_6   5       //自动风向6
#define ARC_AFANDIR_7   6       //自动风向7
#define ARC_AFANDIR_8   7       //自动风向8
#define ARC_AFANDIR_9   8       //自动风向9
#define ARC_AFANDIR_10  9       //自动风向10
/*ARC  ADR_MFANDIR 手动风向常量*/
#define ARC_MFANDIR_1   0       //手动风向1
#define ARC_MFANDIR_2   1       //手动风向2
#define ARC_MFANDIR_3   2       //手动风向3
#define ARC_MFANDIR_4   3       //手动风向4
#define ARC_MFANDIR_5   4       //手动风向5
#define ARC_MFANDIR_6   5       //手动风向6
#define ARC_MFANDIR_7   6       //手动风向7
#define ARC_MFANDIR_8   7       //手动风向8
#define ARC_MFANDIR_9   8       //手动风向9
#define ARC_MFANDIR_10  9       //手动风向10

/*ARC ADR_FANLEV 风量常量*/
#define ARC_FANLEV_1    0       //风量1 自动
#define ARC_FANLEV_2    1       //风量2 低风
#define ARC_FANLEV_3    2       //风量3 中风
#define ARC_FANLEV_4    3       //风量4 高风

/*ARC 开关机常量*/
#define ARC_POWER_ON        0
#define ARC_POWER_OFF       1


/*ARC (ADR_KEYVAL)按键常量*/
#define DAT_KEY_NUM_MAX     0x10
#define ARC_NO_KEY          0x0FF   //空键
#define ARC_KEY_POWER       0   //电源
#define ARC_KEY_TUP         1   //温度+
#define ARC_KEY_TDOWN       2   //温度-
#define ARC_KEY_MODE        3   //模式
#define ARC_KEY_FANLEV      4   //风量
#define ARC_KEY_AFANDIR     5   //自动风向
#define ARC_KEY_MFANDIR     6   //手动风向
#define ARC_KEY_TIMEON      7   //定开
#define ARC_KEY_TIMEOFF     8   //定关
//ARC_KEY_TIME      EQU 7   //定时设置
#define ARC_KEY_TIMECL      9   //取消定时
#define ARC_KEY_SLEEP       10  //睡眠
#define ARC_KEY_HEAT        11  //电辅热
#define ARC_KEY_STRONG      12  //强力
#define ARC_KEY_LIGHT       13  //灯光
#define ARC_KEY_AIRCLEAR    14  //空清
#define ARC_KEY_ECONOMIC    15  //经济
#define ARC_KEY_CLOCK       0x10        //时钟

/*ADR_SYSFLAG(功能标志)常量定义*/
#define SYSFLAG_POWER   0x01    //00000001B     //开关机标志----》开关=1，关机=0
#define ADSF_POWER      0               //开关机标志位置
#define SYSFLAG_SLEEP   0x02    //00000010B     //睡眠标志--------》睡眠开=1，睡眠关=0
#define ADSF_SLEEP      1               //睡眠标志位置
#define SYSFLAG_HEAT    0x04    //00000100B     //电辅热标志----》电辅热开=1，电辅热关=0
#define ADSF_HEAT       2               //电辅热标志位置
#define SYSFLAG_TIME    0x08    //00001000B     //定时标志-------》 有定时= 1，没有定时 = 0
#define ADSF_TIME       3               //定时标志位置
#define SYSFLAG_STRONG   0x10   //00010000B     //强力标志-------》 强力开= 1，强力关 = 0
#define ADSF_STRONG     4               //强力标志位置
#define SYSFLAG_LIGHT   0x20    //00100000B     //灯光标志-------》 灯光开= 1，灯光关 = 0
#define ADSF_LIGHT      5               //灯光标志位置
#define SYSFLAG_AIRCLEAR    0x40    //01000000B     //空清/净化/健康标志-------》 空清/净化/健康开= 1，空清/净化/健康关 = 0
#define ADSF_AIRCLEAR   6               //空清/净化/健康标志位置
#define SYSFLAG_ECONOMIC    0x80    //10000000B     //经济标志-------》 经济开= 1，经济关 = 0
#define ADSF_ECONOMIC   7               //经济标志位置

/*ARC ADR_TMSTATE7定时状态常量（7态）*/
#define ARC_T7_NOTIME       0   //没有定时
#define ARC_T7_TIMEONP      1   //打开定开
#define ARC_T7_TIMEOFFP     2   //打开定关
#define ARC_T7_TIMEONW      3   //等待定开状态
#define ARC_T7_TIMEOFFW     4   //等待定关状态
#define ARC_T7_TIMEONCL     5   //取消定开
#define ARC_T7_TIMEOFFCL    6   //取消定关
/*ADR_TMSTATE3 定时状态常量（3态）*/
#define ARC_T3_NOTIME       0       //没有定时
#define ARC_T3_TIMEON       1       //有定开
#define ARC_T3_TIMEOFF      2       //有定关
#define ARC_T3_TIMEONOFF    3       //同时有定开定关
/**************************************************************************************************************
驱动数据取值定义
***************************************************************************************************************/
//载波周期*2  （1/频率）*2*10^6
#define IDX_Carry_39K   51  //51.2us
#define IDX_Carry_38K   52  //52.6us
#define IDX_Carry_33K   60  //60.6us
#define IDX_Carry_42K   47  //47.6us
#define IDX_Carry_37K   54  //54.05us
#define IDX_Carry_36K   55  //55.55us
#define IDX_Carry_35K   57  //57.14us
//发码类型
#define PMID0       0       //普通脉宽编码
#define PMID1       1       //相位编码
#define F_SEND_HS   8       //高位在前，默认低位在前，F_SEND_HS=1时发码高位在前
//DRV  FORMAT 驱动格式 命令字
#define CMD_Drv_Data    0x10
#define CMD_Drv_Pulse   0x20
#define CMD_Drv_Toggle  0x30
//数据取反发送
#define CMD_DATA_NOT    0x08
#define CMD_PULSE_NOH   0x08    //高电平为0 的PULSE
//IR电平时间单位
#define TUnit   160      //16us
//结束字
#define PULSE_TIME_END  0xffff
#define CMD_Drv_FEnd    0xff
/**************************************************************************************************************
电平时间数据存放位置（偏移量）
***************************************************************************************************************/
#define PMID0_D0_index  0
#define PMID0_D1_index  1
#define PMID0_P0_index  2
#define PMID0_P1_index  3
#define PMID0_P2_index  4
#define PMID0_P3_index  5
#define PMID0_P4_index  6
/**************************************************************************************************************
ADR_TYPE 定义
***************************************************************************************************************/
#define TYPE_FANDIR_LR      0x80    //0 默认自动风向与手机风向切换；1 左右风向和上下风向 同时可调
#define TYPE_DonotDisp      0x20    //没有显示
#define TYPE_DonotClrScreen 0x10    //关机不关显示
#define TYPE_SleepNHours    0x40    //睡眠可设多少个小时
#define TYPE_AM_NOTEMP      0X0100  //自动模式不显示温度
#define TYPE_DM_NOTEMP      0x0200  //除湿模式不显示温度
/*********************************/
#define HB1682   0x0
#define HB1688   0x1
#define HB1678   0x2
/**************************************************************************************************************
外部变量定义
***************************************************************************************************************/

extern unsigned char ArcIRdataBuff[240];    //空调红外输出脉冲数据（格式定义参考。。。）

/************************************************************************
    函 数 名 : void ArcLibInit()
    功能描述 : 空调码库全局变量初始化
    输入参数 : void
    输出参数 : void
    说明 :    空调码库全局变量初始化
**************************************************************************/
void ArcLibInit(void);

/*********************************************************************************
函数名：Read_ARC_IR_Data
输入参数      : int ARC_group                         空调码组号输入
输入/输出参数 : unsigned char ADR_VAR[14]             空调状态变量
输入参数      ：unsigned char ic_type                芯片类型：0x0:HB1682 0x1:HB1688
输出数据      ：ArcIRdataBuff[]                       此数组数据为208Byte，红外发码数据。请使用指令0x0D进行发送。
函数说明：   读取空调红外数据
          调用此函数时，先对ADR_VAR[]进行赋值。调用后，ADR_VAR[]返回最新的状态值。
     ArcIRdataBuff[]此数组数据为208Byte，红外发码数据。请使用指令0x0D进行发送。如ArcIRdataBuff[0]=0,表示没有不需要发红外码。
**********************************************************************************/
void Read_ARC_IR_Data(int ARC_group, unsigned char ADR_VAR[14], unsigned char ic_type); //按键处理，包括数据解析，红外数据分析输出

/********************************************************************************
函数名：Arc_Identification
输入参数 : unsigned const char * p_arc_len_buffer 空调数据输入
输入参数 ：unsigned char ic_type                 芯片类型：0x0:HB1682 0x1:HB1688
输出参数 : unsigned int * group_number    返回长度为10的数组 （码组号返回）
函数说明： 自动识别空调遥控器函数
           输入空调红外遥控器电源键学习数据，返回库码对应的码组号
*********************************************************************************/
void Arc_Identification(unsigned const char * p_arc_len_buffer, unsigned int * group_number, unsigned char ic_type);

void irarc_arc_lrc_learning(char first, char note);//空调开始学习
void irarc_urc_lrc_learning(char first, char note);//普通家电开始学习
void irarc_aurc_exit(char note);//退出学习或者发码
int irarc_open(void);//打开空调
void irarc_close(void);//关闭空调
int irarc_cold(void);//制冷模式
int irarc_heating(void);//制热模式
int irarc_dehumidification(void);//除湿模式
int irarc_energy_saving(void);//节能模式
int irarc_air_suplly(void);//送风模式
int irarc_sleep(void);//睡眠模式
int irarc_powerful(void);//强力模式
int irarc_temperature_inc(void);//升高温度
int irarc_temperature_dec(void);//降低温度
int irarc_max_wind(void);//最大风量
int irarc_min_wind(void);//最小风量
int irarc_temperature_set(int tmp);//设置温度 16 - 32
int irarc_swing_left_and_right(void);//左右摆动
int irarc_swing_auto(void);//自动风向
int irarc_swing_up_and_down(void);//上下摆动
int irarc_swing_manual(void);//手动风向
void irarc_timing_open(int times);//定时打开
void irarc_timing_close(int times);//定时关闭
void irarc_timing_exit();//关闭定时

#endif
