#ifndef CONFIG_BOARD_OPEN_DEV_CFG_H
#define CONFIG_BOARD_OPEN_DEV_CFG_H

/*
 * board_open_dev_cfg.h
 * 由图形编程配置器 v35 生成（IO 配置 + 功能模块）
 * 生成时间: 2026/11/3 10:22:06
 */

/* ================= 芯片 / 存储 ================= */
// CHIP        = AC7916AB
// SDRAM_SIZE  = 8MB
// FLASH_SIZE  = 8MB
#define CONFIG_BOARD_OPEN_DEV_ENABLE
#define CONFIG_BOARD_7916AB
#define __FLASH_SIZE__              8388608
#define __SDRAM_SIZE__              8388608

/* ================= 通用配置 ================= */
#define VOLUME_STEP                 10          //音量设置梯度
#define GAIN_STEP                   10          //录音增益梯度
#define MIN_VOLUME_VALUE            10          //最小音量
#define MAX_VOLUME_VALUE            100          //最大音量
#define INIT_VOLUME_VALUE           60          //默认初始化音量
#define HW_VOLUME_VALUE             70          //解码硬件最大音量
#define PRODUCTION_TEST_VOLUME      80          //厂测音量
#define IFLY_KWS_MLP_CONST_ARRAY
#define IFLY_KWS_NAME               "小语小语"
#define CONFIG_XY_KEYWORDS
#define CONFIG_PRESS_LONG_KEY_POWERON                   //长按开关

/* ================= 音视频配置 ================= */
#define NET_MUSIC_CONFIG_DEC_ANALOG_VOLUME_ENABLE //网络音乐解码的淡入淡出
#define CONFIG_AISP_MIC_ADC_GAIN            80      //本地空闲下mic增益，远场拾音加大80以上，进场拾音40以下
/* 麦克风/回采增益参数（线性表达式 y = ax + b） */
#define AEC_SLOPE           -0.43
#define AEC_INTERCEPT       87.86
#define MIC1_SLOPE          -0.57
#define MIC1_INTERCEPT      108.63
#define MIC2_SLOPE          -0.52
#define MIC2_INTERCEPT      96.55


/* ================= 网络配置 ================= */
#define CONFIG_MORE_NOTIC_REPLY_ENABLE      //使能聊天多次提示回复语
#define CONFIG_QYAI_MQTT_IOT_ENABLE             //开启QYAI-MQTT

/* ================= 系统配置 ================= */
#define TCFG_VBAT_CHECK_EN                  1           //使能电量检测
#define BT_DEV_NAME                         "QYAI"      //经典蓝牙名称(UTF-8, <=16字节)
#define TCFG_AUTO_SLEEP_CHECK_EN            1           //使能特定时间待机检测空闲后进入关机
#define TCFG_AUTO_DEV_LOW_PWER_TIME_SEC     30          //使能30秒进入外设低功耗(未关机，只是把耗电外设降低功耗)
#define AUTO_PWEROFF_SLEEP_TIME_MIN         10           //10分钟自动关机

/* ================= 量产配置 ================= */

/* ================= 调试日志打印 ================= */
#define CONFIG_DEBUG_ENABLE             1
#define TCFG_DEBUG_PORT                 IO_PORTH_04
#define TCFG_DEBUG_BAUD                 1000000

/* ================= 串口命令 ================= */
// #define AT_UART_CMD_ENABLE      2

/* ================= 电源使能 ================= */
#define TCFG_VCC33_CTRL_PORT    IO_PORTH_07

/* ================= 按键 ================= */
#define TCFG_IOKEY_ENABLE               1           //IO按键
#define TCFG_IOKEY_PORT                 IO_PORTC_06 //按键的IO

/* ================= 麦克风 / 回采 ================= */
#define MIC1                            1
#define MIC2                            2
#define MIC_AEC                         0
#define AUDIO_RECORD_MIC_COUNT          2   //1单麦,2双麦

/* ================= DAC 输出 ================= */
#define TCFG_DAC_MUTE_PORT              IO_PORTC_07
#define TCFG_DAC_MUTE_VALUE             0
#define TCFG_DAC_AUTO_MUTE_ENABLE       0
#define TCFG_DAC_DIFF_OUT_ENABLE        1
#define TCFG_DAC_SOFT_CHANNEL_NUM       2
#define TCFG_DAC_HW_CHANNEL_BIT         (BIT(0)|BIT(1)|BIT(2)|BIT(3))   //BIT(0):DACFL、 BIT(1):DACFR、 BIT(2):DACRL、 BIT(3):DACRR

/* ================= 充电检查 ================= */
#define TCFG_CHARING_STATUS_PORT        IO_PORTC_05
#define TCFG_CHARGFULL_STATUS_PORT      -1

/* ================= 软件 IIC 接口 (三轴传感器)================= */
#define CONFIG_GSENSOR_ENABLE
#define CONFIG_SC7A20H_GSENSOR_ENABLE
#define TCFG_SW_IIC1_CLK_PORT           IO_PORTA_09
#define TCFG_SW_IIC1_SDA_PORT           IO_PORTA_07
#define TCFG_SW_IIC1_DELAY              50

/* ================= 屏幕 SPI 接口 ================= */
#define CONFIG_UI_ENABLE
#define CONFIG_UI_PLAY_EMOJI

#ifdef CONFIG_UI_PLAY_EMOJI
#define CONFIG_UI_TOW_EYE                           //开2个眼睛显示
//#define CONFIG_UI_GIF_EYE                           //开启GIF眼睛显示
#define CONFIG_UI_AVI_EYE                           //开启AVI眼睛显示
#ifdef CONFIG_UI_TOW_EYE
#define CONFIG_UI_MIRROR_EYE                        //开启另外眼睛镜像显示
#endif
#endif

#define LCD_SPI_NAME                    "spi1"
#define LCD_SPI_RS_PORT                 IO_PORTH_02
#define LCD_SPI_TE_PORT                 IO_PORTA_00
#define LCD_SPI_RST_PORT                -1
#define LCD_SPI_CS_PORT                 IO_PORTA_06
#define LCD_SPI_CS1_PORT                IO_PORTC_01
#define LCD_SPI_BL_PORT                 IO_PORTH_00
#define TCFG_LCD_GC9D01_ENABLE          1

/* ================= SDIO 配置（SD卡） ================= */
#define TCFG_SD0_ENABLE         0
#define TCFG_SD1_ENABLE         0

/* ================= 舵机配置 ================= */
#define TCFG_SERVO_ENABLE
#define TCFG_SERVOEN_PORT                   IO_PORTC_04
#define SERVO_COUNT                         3

#ifdef TCFG_SERVO_ENABLE
#define TCFG_SERVOEN_PORT                   IO_PORTC_04
#define TCFG_SERVO1_PORT                    IO_PORTH_01
#define TCFG_SERVO1_PWMCH                   PWMCH3_L
#define TCFG_SERVO2_PORT                    IO_PORTC_02
#define TCFG_SERVO2_PWMCH                   PWMCH4_L
#define TCFG_SERVO3_PORT                    IO_PORTA_08
#define TCFG_SERVO3_PWMCH                   PWMCH1_L
#endif

/* ================= 触摸 ================= */
#define CONFIG_TOUCH_ADC_ENABLE
#define TOUCH_ADC_PORT              IO_PORTB_01
#define TOUCH_ADC_CHANNEL           AD_CH_PB01

/* ================= SD卡播放模式 ================= */

/* ================= U盘播放 ================= */

/* ================= 线路输入(AUX/linein)模式 ================= */
/* ================= 4G/LTE 蜂窝网络 ================= */
/* ================= WS2812 灯带(UART2)================= */
/* ================= LED 呼吸灯 ================= */


#endif