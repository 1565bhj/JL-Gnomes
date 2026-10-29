#ifndef CONFIG_BOARD_OPEN_DEV_CFG_H
#define CONFIG_BOARD_OPEN_DEV_CFG_H

/*
 * board_open_dev_cfg.h
 * 由图形编程配置器 v35 生成（IO 配置 + 功能模块）
 * 生成时间: 2026/10/29 11:58:23
 */

/* ================= 芯片 / 存储 ================= */
// CHIP        = QYAI-AC-A100模组
// SDRAM_SIZE  = 8MB
// FLASH_SIZE  = 8MB
#define CONFIG_BOARD_OPEN_DEV_ENABLE
#define CONFIG_BOARD_7916AB
#define __FLASH_SIZE__       8388608
#define __SDRAM_SIZE__       8388608

/* ================= 通用配置 ================= */
#define VOLUME_STEP                 10          //音量设置梯度
#define GAIN_STEP                   10          //录音增益梯度
#define MIN_VOLUME_VALUE            10          //最小音量
#define MAX_VOLUME_VALUE            100          //最大音量
#define INIT_VOLUME_VALUE           60          //默认初始化音量
#define HW_VOLUME_VALUE             70          //解码硬件最大音量
#define PRODUCTION_TEST_VOLUME      80          //厂测音量

/* ================= 音视频配置 ================= */
#define NET_MUSIC_CONFIG_DEC_ANALOG_VOLUME_ENABLE //网络音乐解码的淡入淡出
#define CONFIG_BT_MUSIC_MODE_ENABLE         //蓝牙播放模式使能
#define CONFIG_AISP_MIC_ADC_GAIN            80      //本地空闲下mic增益，远场拾音加大80以上，进场拾音40以下
/* 麦克风/回采增益参数（线性表达式 y = ax + b） */
#define AEC_SLOPE          -0.5
#define AEC_INTERCEPT      80
#define MIC1_SLOPE          -0.6
#define MIC1_INTERCEPT      85
#define MIC2_SLOPE          -0.6
#define MIC2_INTERCEPT      85


/* ================= 网络配置 ================= */
#define CONFIG_MORE_NOTIC_REPLY_ENABLE      //使能聊天多次提示回复语
#define CONFIG_QYAI_MQTT_IOT_ENABLE             //开启QYAI-MQTT

/* ================= 系统配置 ================= */
#define TCFG_VBAT_CHECK_EN                  0           //使能电量检测
#define BT_DEV_NAME                         "QYAI"      //经典蓝牙名称(UTF-8, <=16字节)
#define TCFG_AUTO_SLEEP_CHECK_EN            1           //使能特定时间待机检测空闲后进入关机
#define TCFG_AUTO_DEV_LOW_PWER_TIME_SEC     30          //使能30秒进入外设低功耗(未关机，只是把耗电外设降低功耗)

/* ================= 量产配置 ================= */

/* ================= 调试日志打印 ================= */
#define CONFIG_DEBUG_ENABLE      1
#define TCFG_DEBUG_PORT          IO_PORTA_10
#define TCFG_DEBUG_BAUD          1000000

/* ================= 电源使能 ================= */
#define TCFG_VCC33_CTRL_PORT    IO_PORTB_03
#define TCFG_IOVDD_CTRL_PORT    IO_PORTB_00

/* ================= 按键 ADKEY ================= */
#define TCFG_ADKEY_ENABLE       1
#define TCFG_ADKEY_PORT         IO_PORTB_01
#define ADKEY_KEY_COUNT         3
#define ADKEY_PULLUP_RESISTOR   22
/* 按键1: 电阻 0KΩ, 功能值 0x01011111 */
#define ADKEY_KEY1_PULLDOWN_RESISTOR   0
#define ADKEY_KEY1_FUNC                0x01011111
/* 按键2: 电阻 33KΩ, 功能值 0x00000033 */
#define ADKEY_KEY2_PULLDOWN_RESISTOR   33
#define ADKEY_KEY2_FUNC                0x00000033
/* 按键3: 电阻 51KΩ, 功能值 0x00000022 */
#define ADKEY_KEY3_PULLDOWN_RESISTOR   51
#define ADKEY_KEY3_FUNC                0x00000022
#define CONFIG_PRESS_LONG_KEY_POWERON                   //长按开关

/* ================= 麦克风 / 回采 ================= */
#define MIC1                    1
#define MIC2                    2
#define MIC_AEC                 0
#define AUDIO_RECORD_MIC_COUNT  2   //1单麦,2双麦

/* ================= DAC 输出 ================= */
#define TCFG_DAC_MUTE_PORT           IO_PORTH_01
#define TCFG_DAC_MUTE_VALUE          0
#define TCFG_DAC_AUTO_MUTE_ENABLE    0
#define TCFG_DAC_DIFF_OUT_ENABLE     1
#define TCFG_DAC_SOFT_CHANNEL_NUM    2
#define TCFG_DAC_HW_CHANNEL_BIT      (BIT(0)|BIT(1)|BIT(2)|BIT(3))   //BIT(0):DACFL、 BIT(1):DACFR、 BIT(2):DACRL、 BIT(3):DACRR

/* ================= 充电检查 ================= */

/* ================= 软件 IIC 接口 ================= */

/* ================= 屏幕 SPI 接口 ================= */

/* ================= SDIO 配置（SD卡） ================= */
#define TCFG_SD0_ENABLE         0
#define TCFG_SD1_ENABLE         0

/* ================= SD卡播放模式 ================= */

/* ================= U盘播放 ================= */

/* ================= 线路输入(AUX/linein)模式 ================= */
/* ================= 4G/LTE 蜂窝网络 ================= */
/* ================= WS2812 灯带(UART2)================= */
/* ================= LED 呼吸灯 ================= */
#endif
