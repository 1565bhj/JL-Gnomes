#ifndef CONFIG_BOARD_7916A_DEVELOP_CFG_H
#define CONFIG_BOARD_7916A_DEVELOP_CFG_H

#ifdef CONFIG_BOARD_DEVELOP

//#define CONFIG_FLASH_FOUR_WIDTH_ENABLE                 //FLASH四线模式

#define CONFIG_STORE_VOLUME     1           //保存音量大小
#define VOLUME_STEP             10          //音量设置梯度
#define GAIN_STEP               10          //录音增益梯度
#define MIN_VOLUME_VALUE        10          //最小音量
#define MAX_VOLUME_VALUE        100         //最大音量
#define INIT_VOLUME_VALUE       60          //默认初始化音量
#define HW_VOLUME_VALUE         70          //解码硬件最大音量
#define PRODUCTION_TEST_VOLUME  80          //厂测音量
#define FORCE_DAC_SAMPLE_TRATE  0           //强制解码采样率（暂无用）
#define ASR_CONTINIU_ENABLE     0           //语音识别连续对话（暂无用）
//#define CONFIG_USE_TTS_REPLY_ENABLE         //使能回复语对话使用在线TTS
#define NET_MUSIC_CONFIG_DEC_ANALOG_VOLUME_ENABLE //网络音乐解码的淡入淡出
#define CONFIG_MORE_NOTIC_REPLY_ENABLE      //使能多次提示回复语
#define CONFIG_KWS_XIAOYUXIAOYU             //唤醒词：小语小语
//#define CONFIG_KWS_ENGLISH                  //英语版唤醒词：hello you you，海外版本（在本文件最后的海外语言配置项进行语言选择）

#define SYS_COMMAND_ANSER_PLAY_ENABLE         //系统命令采用本地“好的”提示，注释则使用服务器下发
//#define BT_DISCON_MUSIC_NOTICE_ENABLE       //播报蓝牙断开开关
//#define TCFG_EQ_ONLINE_ENABLE     1           //1:支持在线EQ调试
//#define PRODUCTION_ALL_TEST_ENABLE            //所有功能产测模式使能
#define CONFIG_QYAI_MQTT_IOT_ENABLE             //开启QYAI-MQTT

/***********************4G模组芯片厂商*****************************************/
#define VENDOR_ML307R       0 //中移,307R/C系列
#define VENDOR_YUGEYM310    1 //域格
#define VENDOR_GT108        2 //全网通
#define VENDOR_ML120H       3 //骐俊
#define VENDOR_AIR780E      4 //合宙780E
#define VENDOR_NT26K        5 //利尔达NT26K
#define VENDOR_CE105        6 //雁飞CE105

//根据不同的芯片去分配不同的USB口
//USB口使能：0x1-->USB0； 0x2-->USB1； 0x3--->USB0和USB1
#define CONFIG_USB_OTG_EN_VAL   0x03 //High-Speed端口，7916两组端口都有
//#define CONFIG_USB_OTG_EN_VAL 0x01 //Full-Speed端口，7911只有FUSB
#define DEFAULT_NET_CHANNEL     0 //默认出厂设置的网络模式：0-wifi模式，1-4G模式
/*****************************************************************************/

#ifdef CONFIG_NET_ENABLE
#define CONFIG_ASR_ALGORITHM_ENABLE         //mode:打断唤醒模式使能
#define CONFIG_NET_MUSIC_MODE_ENABLE        //mode:网络播放模式使能
#define CONFIG_BT_MUSIC_MODE_ENABLE         //蓝牙播放模式使能
//#define CONFIG_USB_DISK_MUSIC_MODE_ENABLE   //U盘播放模式使能
//#define CONFIG_SD_MUSIC_MODE_ENABLE         //SD卡播放模式使能
//#define CONFIG_AUX_MUSIC_MODE_ENABLE        //line in线路输入模式使能
#endif // CONFIG_NET_ENABLE




//*********************************************************************************//
//                            所有样机板子                                         //
//*********************************************************************************//
#define SXY_QYAI_DEVELEP_BOARD          1   //QYAI-开发板
#define SXY_QYAI_4G_DEMO_BOARD          0   //QYAI-4G-demo板
#define SXY_QYAI_UI_BOARD               0   //QYAI-UI-开发板


//*********************************************************************************//
//                           QYAI-开发板                                           //
//*********************************************************************************//
#if SXY_QYAI_DEVELEP_BOARD

//#define CONFIG_QYAI_MUTILMODAL_ENABLE     //开启QYAI多模态，热敏打印机则开启
//#define CONFIG_MQTT_IOT_ENABLE  //开启MQTT IOT

//#define RF_FCC_DATA_TRIM_ENABLE //使用RF_FCC校准频偏，校准PA-gian
//#define RF_FCC_TEST_ENABLE  //使能RF_FCC测试，详细配置见"apps/common/rf_fcc_tool/include/rf_fcc_main.h"
//#define PRODUCTION_YIERD_UART_CMD_ENABLE  //易尔达USB串口射频AT测试

#ifdef PRODUCTION_YIERD_UART_CMD_ENABLE
#define YIERD_WIFI_MODULE //易尔达7916模组
#endif

//#define AI_UART_CMD_CTROL_ENABLE            0       //AI模组串口0-串口协议使能
//#define AI_UART_CMD_TX_PORT       IO_PORTC_05         //AI模组串口0-TX

//#define AT_UART_CMD_ENABLE                    0   //使用串口0作为AT命令串口
//#define UART0_BUAD                            115200 //串口0波特率

//#define TCFG_EQ_ONLINE_ENABLE     1            //1:支持在线EQ调试

//#define PRODUCTION_TEST_ENABLE                          //模组产测模式使能
//#define PRODUCTION_ALL_TEST_ENABLE                          //开启量产测试所有功能
////#define PRODUCTION_TEST_PORT                IO_PORTC_05     //触发模组量产测试功能的IO口=
////#define PRODUCTION_ALL_TEST_PORT            IO_PORTA_10//IO_PORTB_01     //触发量产测试所有功能的IO口
#if (defined PRODUCTION_ALL_TEST_ENABLE || defined PRODUCTION_TEST_ENABLE)
#define PRODUCTION_ALL_TEST_WIFI_SSID       "WIFI-PD-TEST"    //量产测试WiFi名称
#define PRODUCTION_ALL_TEST_WIFI_PWD        "12345678"        //量产测试WiFi密码
#define PRODUCTION_ALL_TEST_BT_NAME         "PD_TEST"           //量产测试蓝牙名称

#define PRODUCTION_WIFI_TEST_ENABLE         //开启WiFi测试
#ifdef CONFIG_LTE_PHY_ENABLE
#define PRODUCTION_NET_TEST_ENABLE          //开启4G网络测试
#endif
#define PRODUCTION_MIC_TEST_ENABLE          //开启麦克风测试
#define PRODUCTION_DAC_TEST_ENABLE          //开启喇叭测试
//可以在这里添加其他的厂测宏定义
#ifdef CONFIG_UI_ENABLE
#define PRODUCTION_LCD_TEST_ENABLE          //开启LCD屏幕测试
#endif
#endif

#define CONFIG_BT_MUSIC_MODE_ENABLE         //蓝牙播放模式使能

/* SD */
#define CONFIG_SD_MUSIC_MODE_ENABLE         //SD卡播放模式使能
#define CONFIG_SD_AUTO_ENABLE               //自动检测SD卡插入

/* UDISK */
//#define CONFIG_USB_DISK_MUSIC_MODE_ENABLE   //U盘播歌模式使能
//#define CONFIG_USB_DISK_AUTO_ENABLE         //自动检测U盘插入

/* LINE-IN */
//#define TCFG_AUX_DET_PORT               IO_PORTA_07 //AUX检测引脚
//#define CONFIG_AUX_MUSIC_MODE_ENABLE    //LINE-IN线路输入模式使能
//#define CONFIG_AUX_AUTO_ENABLE          //自动检测LINE-IN线插入

//#define CONFIG_USE_TTS_REPLY_ENABLE        //使能回复语对话使用在线TTS
#define CONFIG_MORE_NOTIC_REPLY_ENABLE      //使能多次提示回复语

//语音检测的灵敏度相关参数
#define CONFIG_AISP_MIC_ADC_GAIN            80      //本地空闲下mic增益，远场拾音加大80以上，进场拾音40以下
#define SPEECH_ENERGY_MIN                   (128)   //语音检测最小值能量值
#define DIGITAL_VOL_AGC                     0      //AEC后的音频数字增益

#define TCFG_DEBUG_PORT                     IO_PORTA_10 //打印
#define TCFG_VCC33_CTRL_PORT                IO_PORTB_03 //3.3V电原控制IO
#define TCFG_IOVDD_CTRL_PORT                IO_PORTB_00 //VDDIO电原控制IO
//#define TCFG_VCC33_CTRL_PORT              IO_PORTA_09 //3.3V电原控制IO
//#define TCFG_IOVDD_CTRL_PORT              IO_PORTA_08 //VDDIO电原控制IO

//#define TCFG_FIRST_POWER_OFF_EN                 //第一次上电开机需要关机

//#define USED_WS2812B_SHOWN                              //WS2812-rgb灯带
#ifdef USED_WS2812B_SHOWN
//#define CONFIG_SPECTRUM_FFT_EFFECT_ENABLE               //开启FFT频谱显示
#define WS2812B_SHOWN_LED_NUM               12          //WS2812-rgb灯带-12个灯珠
//#define WS2812B_FFT_EFFECT_ENABLE                       //WS2812B开启FFT频谱显示

#define WS2812B_HW_UART_NUM                 2           //WS2812使用串口硬件（0 1 2），硬件需要加反相器输出到WS2812的DIN引脚
#define UART2_BUAD                          3333333     //WS2812的串口的波特率（WS2812的0和1电平务必是2倍关系才能使用，0码300ns则是3.3M，是400s则是2.5M）
#define WS2812B_HW_UART_TX_PORT             IO_PORTC_09 //串口TX控制WS2812任意一个引脚（建议使用任意引脚，注：board_791x.x的串口设备port = PORT_REMAP,）

//#define WS2812B_HW_SPI_NUM                  2           //WS2812使用SPI2硬件（WS2812的0和1电平务必是3倍关系才能使用，但是79系列IC有BUG，颜色可能会出错）
//#define SPI2_CLOCK_HZ   2500000                         //WS2812的SPI的波特率（WS2812的0和1电平务必是3倍关系才能使用，0码300ns则是2.5M）
//#define SPI2_ATTR_SET   SPI_SCLK_L_UPL_SMPH | SPI_UNIDIR_MODE //上升沿更新数据，下降沿采样数据，单向单线
#endif

//#define USED_TM1629_SHOWN                               //使用TM1629显示
//#define TCFG_TM1629_SPI_CS                  IO_PORTH_02 //SPI2A_CS
//#define TCFG_TM1629_POWER_PORT              IO_PORTC_01 //TM1629 POWER
//#define CONFIG_SPECTRUM_FFT_EFFECT_ENABLE               //开启FFT频谱显示
//#define USED_TM1629_EAY                                 //开启使用TM1629的LED显示

//#define TCFG_VBAT_CHECK_EN                  1           //使能电量检测
//#define TCFG_VBAT_CHECK_AD_PORT             IO_PORTA_10 //AD IO检测VBAT电池电量IO
//#define TCFG_VBAT_CHECK_AD_PORT_PPS         0.6         //AD IO检测VBAT电池的分压占比

#define TCFG_AUTO_SLEEP_CHECK_EN            1           //使能特定时间待机检测空闲后进入关机
//#define TCFG_AUTO_SLEEP_CHECK_ONLY_SPEECH   1           //使能只检测语音对话作为是否空闲
#define TCFG_AUTO_DEV_LOW_PWER_TIME_SEC     30          //使能30秒进入外设低功耗(未关机，只是把耗电外设降低功
//#define AUTO_DEEP_SLEEP_TIME_MIN            5           //使能进入（5分钟）: wifi关闭、蓝牙关闭、LVGL任务挂起
//#define AUTO_PWEROFF_SLEEP_TIME_MIN         5           //5分钟自动关机

#ifdef AUTO_DEEP_SLEEP_TIME_MIN
#define TCFG_LCD_SLEEP_BACKLIGHT_PWM_DUTY   10           //0-100，睡眠时：LCD屏幕背光PWM值
#endif // AUTO_DEEP_SLEEP_TIME_MIN

//#define UART0_BUAD                          9600        //串口0的波特率
//#define TIMER_UART_CALIBRATION_ENABLE       0           //使用串口0作为时钟校准

//#define TCFG_LED_PWM0_PORT_SHORT_PRESS_AUTO 1           //PWM0的IO短按自动：开灯-70-30-关灯

//#define TCFG_LED_STATUES_PORT               IO_PORTH_00 //LED的IO:慢闪状态灯
#if (TCFG_VBAT_CHECK_EN && defined TCFG_LED_STATUES_PORT)
#define TCFG_LED_STATUES_VBAT_NET_EN        1           //LED的IO:显示网络状态、电池状态、正常状态
#define TCFG_LED_STATUES_NORMAL_BLINK       1           //LED的IO:正常状态闪烁
#endif

//#define LED_EYA_OPEN                        1           //高电平睁眼-LED
//#define TCFG_LED_EYA_R_PORT                 IO_PORTH_04 //LED眼睛：对话眨眼
//#define TCFG_LED_EYA_L_PORT                 IO_PORTH_04 //LED眼睛：对话眨眼

#define TCFG_LED_PWM0_PORT                  IO_PORTH_00 //PWM0的IO-LED背景灯
#define TCFG_LED_PWM0_EYE_EN                1           //开启LED对话眨眼（可以开灯也可以唤醒闪烁）
//#define TCFG_LED_PWMCH_PORT                 PWMCH1_L    //LED背景灯的PWMCH通道

//#define TCFG_PWM1_PORT                      IO_PORTA_08 //地球仪灯光的PWM1的IO
//#define TCFG_PWM1_FREQ                      165000 //地球仪灯光PWM1的频率
//#define TCFG_PWM1_FREQ_DUTY                 50 //地球仪灯光PWM1的频率占空比

//#ifdef CONFIG_LTE_PHY_ENABLE
//#define LTE_POWER_ONOFF_PORT                IO_PORTA_10 //LTE-4G开关IO
//#endif

#define TCFG_ADKEY_ENABLE                   1         //AD按键
#define TCFG_ADKEY_PORT                     IO_PORTB_01 //ADC按键IO
//#define TCFG_IOKEY_PORT                     IO_PORTB_01 //IO按键的IO
//#define TCFG_IOKEY_ENABLE                   1           //IO按键

//#define TCFG_POWER_KEY_LONG_PRESS_VAD_EN    1           //使用按键长按触发VAD

//#define TCFG_KEY_VAD_PORT                   IO_PORTH_04 //独立按键触发VAD

//#define TCFG_TOUCH_KEY_PORT                 IO_PORTH_07 //触摸的IO

//#define CONFIG_PRESS_LONG_KEY_POWERON                   //长按开关

#ifdef CONFIG_LTE_PHY_ENABLE
/* POWER引脚不同的4G模组不一样(根据实际情况来定义) */
#define LTE_POWER_ONOFF_PORT                IO_PORTC_06 //LTE-4G电源开关IO
//#define LTE_USB_SWITCH_PORT                 IO_PORTA_03 //LTE-4G模组USB接到主控的USB通过模拟开关控制（默认1高电平选择到4G模块）

/* 4G模组厂商选择 */
//#define CONFIG_LTE_VENDOR                   (VENDOR_GT108) //fixme: 是否还需要适配GT108
#endif

#define TCFG_DAC_MUTE_PORT                  IO_PORTH_01 //功放静音IO
#define TCFG_DAC_MUTE_VALUE                 0           //功放静音IO电平值
#define TCFG_DAC_AUTO_MUTE_ENABLE           0

#define TCFG_DAC_DIFF_OUT_ENABLE            1
#define TCFG_DAC_HW_CHANNEL_BIT             BIT(0)|BIT(1)|BIT(2)|BIT(3)   //BIT(0):DACFL、 BIT(1):DACFR、 BIT(2):DACRL、 BIT(3):DACRR
#define TCFG_DAC_SOFT_CHANNEL_NUM           2                               //差分两个通道


#define TCFG_SD0_ENABLE                     1           //使能SD卡
#define TCFG_SD_PORTS                       'B'         //SD0/SD1的ABCD组(默认为开发板SD0-D),注意:IO占用问题
#define TCFG_SD_DAT_WIDTH                   1           //1:单线模式, 4:四线模式
#define TCFG_SD_DET_MODE                    SD_CMD_DECT //检测模式:命令检测，时钟检测，IO检测
#define TCFG_SD_DET_IO                      -1          //SD_DET_MODE为SD_IO_DECT时有效
#define TCFG_SD_DET_IO_LEVEL                0           //IO检卡上线的电平(0/1),SD_DET_MODE为SD_IO_DECT时有效
#define TCFG_SD_CLK                         24000000    //SDIO时钟

#if TCFG_VBAT_CHECK_EN
#define CONFIG_ASR_POWER_OFF_ENABLE         //语音在线关机功能
#endif
#endif


//*********************************************************************************//
//                            数芯语-UI-开发板                                     //
//*********************************************************************************//
#if SXY_QYAI_UI_BOARD //数芯语-UI-开发板

//#define CONFIG_QYAI_MUTILMODAL_ENABLE //开启多模态
//#define CONFIG_MQTT_IOT_ENABLE  //开启MQTT IOT

#define CONFIG_UI_ENABLE                            //开启UI显示
#define CONFIG_UI_FONT_RES_SAVE_IN_FLASH            //UI字库存放在flash

//#define USE_LCD_TE_ROTATE_ENABLE            1 //开启TE使用时播放视频翻转视频数据

#define CONFIG_LVGL_UI_ENABLE
//#define CONFIG_UI_PLAY_EMOJI                        //开启眼睛表情显示

//#define TCFG_LVGL_FRAME_BUFFER_ROTATE_ANGLE 270       //LVGL帧直推旋转角度
#if TCFG_LCD_JD9853_ENABLE
#define TCFG_LVGL_FRAME_BUFFER_ROTATE_ANGLE 270       //LVGL帧直推旋转角度
#endif // TCFG_LCD_JD9853_ENABLE

//是否开启(需要重编LVGL库)
#ifdef CONFIG_LVGL_UI_ENABLE
#define CONFIG_LVGL_AGENT_ENABLE            1  //开启智能体页面
#define CONFIG_LVGL_TIMER_SLEEP_ENABLE      1  //开启定时休眠页面
#define CONFIG_LVGL_SLEEP_MUSIC_ENABLE      1  //开启睡眠音乐页面
#define CONFIG_LVGL_OPTION_WIFI_ENABLE      1  //开启WiFi页面
#define CONFIG_LVGL_CONTENT_LIST_ENABLE     1  //开启内容页面
#define CONFIG_LVGL_CALCULATOR_ENABLE       1  //开启计算器页面

// 宏定义是否使能LVGL游戏
#define CONFIG_LVGL_GAMES_ENABLE            0
#if CONFIG_LVGL_GAMES_ENABLE
#define CONFIG_LVGL_DRAW_LOTS_GAME_ENABLE   0//使能抽签游戏
#define CONFIG_LVGL_DICE_GAME_ENABLE        0//使能骰子游戏
#define CONFIG_LVGL_MOUSE_GAME_ENABLE       1//使能打地鼠游戏
#endif
#define CONFIG_LVGL_RANDOM_GAME_ENABLE      0//使能随机游戏
#define TOMATO_TIMER_ENABLE                 1//番茄倒计时
#endif



#ifdef CONFIG_UI_PLAY_EMOJI
//#define CONFIG_UI_TOW_EYE                           //开2个眼睛显示
//#define CONFIG_UI_GIF_EYE                           //开启GIF眼睛显示
#define CONFIG_UI_AVI_EYE                           //开启AVI眼睛显示
#ifdef CONFIG_UI_TOW_EYE
#define CONFIG_UI_MIRROR_EYE                        //开启另外眼睛镜像显示
#endif
#endif

//#define TCFG_TOUCH_GST820_ENABLE            1 //触摸
//#define TCFG_TOUCH_GST328_ENABLE            1 //触摸

//#define CONFIG_GSENSOR_ENABLE               1 //加速度传感器
//#define CONFIG_SC7A20H_GSENSOR_ENABLE       1   //SC7A20H加速度传感器

// 宏定义是否使能LVGL游戏
//#define CONFIG_LVGL_GAMES_ENABLE            1
//#ifdef CONFIG_LVGL_GAMES_ENABLE
//#define CONFIG_LVGL_DRAW_LOTS_GAME_ENABLE   1//使能抽签游戏
//#define CONFIG_LVGL_DICE_GAME_ENABLE        1//使能骰子游戏
//#define CONFIG_LVGL_SHAKE_GAME_ENABLE       1//使能摇一摇游戏
//#endif

//#define TCFG_EQ_ONLINE_ENABLE     1            //1:支持在线EQ调试

//#define PRODUCTION_TEST_ENABLE                          //模组产测模式使能
//#define PRODUCTION_ALL_TEST_ENABLE                          //开启量产测试所有功能
////#define PRODUCTION_TEST_PORT                IO_PORTC_05     //触发模组量产测试功能的IO口=
////#define PRODUCTION_ALL_TEST_PORT            IO_PORTA_10//IO_PORTB_01     //触发量产测试所有功能的IO口
#if (defined PRODUCTION_ALL_TEST_ENABLE || defined PRODUCTION_TEST_ENABLE)
#define PRODUCTION_ALL_TEST_WIFI_SSID       "WIFI-PD-TEST"    //量产测试WiFi名称
#define PRODUCTION_ALL_TEST_WIFI_PWD        "12345678"        //量产测试WiFi密码
#define PRODUCTION_ALL_TEST_BT_NAME         "PD_TEST"           //量产测试蓝牙名称

#define PRODUCTION_WIFI_TEST_ENABLE         //开启WiFi测试
#ifdef CONFIG_LTE_PHY_ENABLE
#define PRODUCTION_NET_TEST_ENABLE          //开启4G网络测试
#endif
#define PRODUCTION_MIC_TEST_ENABLE          //开启麦克风测试
#define PRODUCTION_DAC_TEST_ENABLE          //开启喇叭测试
//可以在这里添加其他的厂测宏定义
#ifdef CONFIG_UI_ENABLE
#define PRODUCTION_LCD_TEST_ENABLE          //开启LCD屏幕测试
#endif
#endif


#define CONFIG_BT_MUSIC_MODE_ENABLE         //蓝牙播放模式使能
#define CONFIG_SD_MUSIC_MODE_ENABLE         //SD卡播放模式使能
#define CONFIG_SD_AUTO_ENABLE

//#define CONFIG_USE_TTS_REPLY_ENABLE        //使能回复语对话使用在线TTS
#define CONFIG_MORE_NOTIC_REPLY_ENABLE     //使能多次提示回复语

//语音检测的灵敏度相关参数
#define CONFIG_AISP_MIC_ADC_GAIN            80      //本地空闲下mic增益，远场拾音加大80以上，进场拾音40以下
#define SPEECH_ENERGY_MIN                   (128)   //语音检测最小值能量值
#define DIGITAL_VOL_AGC                     0      //AEC后的音频数字增益

#define TCFG_DEBUG_PORT                     IO_PORTA_10 //打印调试IO
#define TCFG_VCC33_CTRL_PORT                IO_PORTB_03 //3.3V电原控制IO
#define TCFG_IOVDD_CTRL_PORT                IO_PORTB_00 //VDDIO电原控制IO

//#define TCFG_FIRST_POWER_OFF_EN                 //第一次上电开机需要关机

//#define USED_WS2812B_SHOWN                              //WS2812-rgb灯带
#ifdef USED_WS2812B_SHOWN
#define WS2812B_SHOWN_LED_NUM               12          //WS2812-rgb灯带-12个灯珠
//#define WS2812B_FFT_EFFECT_ENABLE                       //WS2812B开启FFT频谱显示

#define WS2812B_HW_UART_NUM                 2           //WS2812使用串口硬件（0 1 2），硬件需要加反相器输出到WS2812的DIN引脚
#define UART2_BUAD                          3333333     //WS2812的串口的波特率（WS2812的0和1电平务必是2倍关系才能使用，0码300ns则是3.3M，是400s则是2.5M）
#define WS2812B_HW_UART_TX_PORT             IO_PORTC_09 //串口TX控制WS2812任意一个引脚（建议使用任意引脚，注：board_791x.x的串口设备port = PORT_REMAP,）

//#define WS2812B_HW_SPI_NUM                  2           //WS2812使用SPI2硬件（WS2812的0和1电平务必是3倍关系才能使用，但是79系列IC有BUG，颜色可能会出错）
//#define SPI2_CLOCK_HZ   2500000                         //WS2812的SPI的波特率（WS2812的0和1电平务必是3倍关系才能使用，0码300ns则是2.5M）
//#define SPI2_ATTR_SET   SPI_SCLK_L_UPL_SMPH | SPI_UNIDIR_MODE //上升沿更新数据，下降沿采样数据，单向单线
#endif


#define TCFG_CHARING_STATUS_PORT            IO_PORTC_06     //TYPE-C插入电池充电检测IO
#define TCFG_CHARGFULL_STATUS_PORT          IO_PORT_PR_00   //充满检测IO
#define TCFG_LCD_BL_PWM_PORT                IO_PORTH_03     //LCD背光灯的PWM IO
#define TCFG_LCD_BL_PWM_DEV_NUM             "pwm1"          //LCD背光灯的PWM设备名称

//#define TCFG_VBAT_CHECK_EN                  1           //使能电量检测
//#define TCFG_VBAT_CHECK_AD_PORT             IO_PORTA_10 //AD IO检测VBAT电池电量IO
//#define TCFG_VBAT_CHECK_AD_PORT_PPS         0.6         //AD IO检测VBAT电池的分压占比

#define TCFG_AUTO_SLEEP_CHECK_EN            1           //使能特定时间待机检测空闲后进入关机
//#define TCFG_AUTO_SLEEP_CHECK_ONLY_SPEECH   1           //使能只检测语音对话作为是否空闲
#define TCFG_AUTO_DEV_LOW_PWER_TIME_SEC     30          //使能30秒进入外设低功耗(未关机，只是把耗电外设降低功
//#define AUTO_DEEP_SLEEP_TIME_MIN            5           //使能进入（5分钟）: wifi关闭、蓝牙关闭、LVGL任务挂起
//#define AUTO_PWEROFF_SLEEP_TIME_MIN         5           //5分钟自动关机

#ifdef AUTO_DEEP_SLEEP_TIME_MIN
#define TCFG_LCD_SLEEP_BACKLIGHT_PWM_DUTY   10           //0-100，睡眠时：LCD屏幕背光PWM值
#endif // AUTO_DEEP_SLEEP_TIME_MIN

//#define UART0_BUAD                          9600        //串口0的波特率
//#define TIMER_UART_CALIBRATION_ENABLE       0           //使用串口0作为时钟校准

//#define TCFG_LED_PWM0_PORT_SHORT_PRESS_AUTO 1           //PWM0的IO短按自动：开灯-70-30-关灯

#define TCFG_LED_STATUES_PORT               IO_PORTH_00 //LED的IO:慢闪状态灯
#if (TCFG_VBAT_CHECK_EN && defined TCFG_LED_STATUES_PORT)
#define TCFG_LED_STATUES_VBAT_NET_EN        1           //LED的IO:显示网络状态、电池状态、正常状态
#define TCFG_LED_STATUES_NORMAL_BLINK       1           //LED的IO:正常状态闪烁
#endif

//#define LED_EYA_OPEN                        1           //高电平睁眼-LED
//#define TCFG_LED_EYA_R_PORT                 IO_PORTH_04 //LED眼睛：对话眨眼
//#define TCFG_LED_EYA_L_PORT                 IO_PORTH_04 //LED眼睛：对话眨眼

//#define TCFG_LED_PWM0_PORT                  IO_PORTH_00 //PWM0的IO-LED背景灯
//#define TCFG_LED_PWM0_EYE_EN                1           //开启LED对话眨眼（可以开灯也可以唤醒闪烁）
//#define TCFG_LED_PWMCH_PORT                 PWMCH1_L    //LED背景灯的PWMCH通道

#define TCFG_ADKEY_ENABLE                   1         //AD按键
#define TCFG_ADKEY_PORT                       IO_PORTB_01 //ADC按键IO
//#define TCFG_IOKEY_PORT                     IO_PORTB_01 //IO按键的IO
//#define TCFG_IOKEY_ENABLE                   1           //IO按键

//#define TCFG_POWER_KEY_LONG_PRESS_VAD_EN    1           //使用按键长按触发VAD

//#define TCFG_KEY_VAD_PORT                   IO_PORTH_04 //独立按键触发VAD

//#define TCFG_TOUCH_KEY_PORT                 IO_PORTH_07 //触摸的IO

#define CONFIG_PRESS_LONG_KEY_POWERON                   //长按开关

#ifdef CONFIG_LTE_PHY_ENABLE
/* POWER引脚不同的4G模组不一样(根据实际情况来定义) */
#define LTE_POWER_ONOFF_PORT                IO_PORTC_06 //LTE-4G电源开关IO
//#define LTE_USB_SWITCH_PORT                 IO_PORTA_03 //LTE-4G模组USB接到主控的USB通过模拟开关控制（默认1高电平选择到4G模块）

/* 4G模组厂商选择 */
#define CONFIG_LTE_VENDOR                   (VENDOR_ML307R) //默认中移全网通cat1-ML307R模组
#endif

#define TCFG_DAC_MUTE_PORT                  IO_PORTH_01 //功放静音IO
#define TCFG_DAC_MUTE_VALUE                 0           //功放静音IO电平值
#define TCFG_DAC_AUTO_MUTE_ENABLE           0

#define TCFG_DAC_DIFF_OUT_ENABLE            1
#define TCFG_DAC_HW_CHANNEL_BIT             BIT(0)|BIT(1)|BIT(2)|BIT(3)   //BIT(0):DACFL、 BIT(1):DACFR、 BIT(2):DACRL、 BIT(3):DACRR
#define TCFG_DAC_SOFT_CHANNEL_NUM           2                               //差分两个通道

#define TCFG_SD0_ENABLE                     1           //使能SD卡
#define TCFG_SD_PORTS                       'B'         //SD0/SD1的ABCD组(默认为开发板SD0-D),注意:IO占用问题
#define TCFG_SD_DAT_WIDTH                   1           //1:单线模式, 4:四线模式
#define TCFG_SD_DET_MODE                    SD_CMD_DECT //检测模式:命令检测，时钟检测，IO检测
#define TCFG_SD_DET_IO                      -1          //SD_DET_MODE为SD_IO_DECT时有效
#define TCFG_SD_DET_IO_LEVEL                0           //IO检卡上线的电平(0/1),SD_DET_MODE为SD_IO_DECT时有效
#define TCFG_SD_CLK                         24000000    //SDIO时钟

#if TCFG_VBAT_CHECK_EN
#define CONFIG_ASR_POWER_OFF_ENABLE         //语音在线关机功能
#endif
#endif


//*********************************************************************************//
//                            QYAI-4G-DEMO                                         //
//*********************************************************************************//
#if SXY_QYAI_4G_DEMO_BOARD   //SXY_QYAI_4G_DEMO_BOARD

//#define CONFIG_MQTT_IOT_ENABLE  //开启MQTT IOT

//#define BLE_BROADCAST_ENABLE_AT_ALL //蓝牙广播常开(WIFI/4G网络模式下)
//#define TCFG_EQ_ONLINE_ENABLE     1            //1:支持在线EQ调试

//#define PRODUCTION_TEST_ENABLE                          //模组产测模式使能
//#define PRODUCTION_ALL_TEST_ENABLE                          //开启量产测试所有功能

//#define PRODUCTION_TEST_ENABLE                          //模组产测模式使能
//#define PRODUCTION_ALL_TEST_ENABLE                          //开启量产测试所有功能
////#define PRODUCTION_TEST_PORT                IO_PORTC_05     //触发模组量产测试功能的IO口=
////#define PRODUCTION_ALL_TEST_PORT            IO_PORTA_10//IO_PORTB_01     //触发量产测试所有功能的IO口
#if (defined PRODUCTION_ALL_TEST_ENABLE || defined PRODUCTION_TEST_ENABLE)
#define PRODUCTION_ALL_TEST_WIFI_SSID       "WIFI-PD-TEST"    //量产测试WiFi名称
#define PRODUCTION_ALL_TEST_WIFI_PWD        "12345678"        //量产测试WiFi密码
#define PRODUCTION_ALL_TEST_BT_NAME         "PD_TEST"           //量产测试蓝牙名称

#define PRODUCTION_WIFI_TEST_ENABLE         //开启WiFi测试
#ifdef CONFIG_LTE_PHY_ENABLE
#define PRODUCTION_NET_TEST_ENABLE          //开启4G网络测试
#endif
#define PRODUCTION_MIC_TEST_ENABLE          //开启麦克风测试
#define PRODUCTION_DAC_TEST_ENABLE          //开启喇叭测试
//可以在这里添加其他的厂测宏定义
#ifdef CONFIG_UI_ENABLE
#define PRODUCTION_LCD_TEST_ENABLE          //开启LCD屏幕测试
#endif
#endif



#define CONFIG_BT_MUSIC_MODE_ENABLE         //蓝牙播放模式使能

#define TCFG_DEBUG_PORT                     IO_PORTA_10 //打印调试IO
#define TCFG_VCC33_CTRL_PORT                IO_PORTB_03 //3.3V电原控制IO
#define TCFG_IOVDD_CTRL_PORT                IO_PORTB_00 //VDDIO电原控制IO

//#define TCFG_FIRST_POWER_OFF_EN                 //第一次上电开机需要关机


//语音检测的灵敏度相关参数
#define CONFIG_AISP_MIC_ADC_GAIN            80      //本地空闲下mic增益，远场拾音加大80以上，进场拾音40以下
#define SPEECH_ENERGY_MIN                   (128)   //语音检测最小值能量值
#define DIGITAL_VOL_AGC                     0      //AEC后的音频数字增益

#ifdef CONFIG_LTE_PHY_ENABLE
/* POWER引脚不同的4G模组不一样(根据实际情况来定义) */
#define LTE_POWER_ONOFF_PORT                IO_PORTC_06 //LTE-4G电源开关IO
//#define LTE_USB_SWITCH_PORT                 IO_PORTA_03 //LTE-4G模组USB接到主控的USB通过模拟开关控制（默认1高电平选择到4G模块）

/* 4G模组厂商选择 */
#define CONFIG_LTE_VENDOR                   (VENDOR_ML307R) //默认中移全网通cat1-ML307R模组
#endif

//#define TCFG_VBAT_CHECK_EN                  1           //使能电量检测
//#define TCFG_VBAT_CHECK_AD_PORT             IO_PORTA_10 //AD IO检测VBAT电池电量IO
//#define TCFG_VBAT_CHECK_AD_PORT_PPS         0.6         //AD IO检测VBAT电池的分压占比

//#define TCFG_AUTO_SLEEP_CHECK_EN            1           //使能特定时间待机检测空闲后进入关机
//#define TCFG_AUTO_SLEEP_CHECK_ONLY_SPEECH   1           //使能只检测语音对话作为是否空闲
//#define TCFG_AUTO_DEV_LOW_PWER_TIME_SEC     30          //使能30秒进入外设低功耗(未关机，只是把耗电外设降低功
//#define AUTO_DEEP_SLEEP_TIME_MIN            5           //使能进入（5分钟）: wifi关闭、蓝牙关闭、LVGL任务挂起
//#define AUTO_SLEEP_TIME_MIN                 5           //5分钟自动关机

#ifdef AUTO_DEEP_SLEEP_TIME_MIN
#define TCFG_LCD_SLEEP_BACKLIGHT_PWM_DUTY   10           //0-100，睡眠时：LCD屏幕背光PWM值
#endif // AUTO_DEEP_SLEEP_TIME_MIN

//#define TCFG_LED_STATUES_PORT               IO_PORTH_04 //LED的IO:慢闪状态灯
#if (TCFG_VBAT_CHECK_EN && defined TCFG_LED_STATUES_PORT)
#define TCFG_LED_STATUES_VBAT_NET_EN        1           //LED的IO:显示网络状态、电池状态、正常状态
#define TCFG_LED_STATUES_NORMAL_BLINK       1           //LED的IO:正常状态闪烁
#endif

#define TCFG_LED_PWM0_PORT                  IO_PORTH_04 //PWM0的IO-LED背景灯
#define TCFG_LED_PWM0_EYE_EN                1           //开启LED对话眨眼（可以开灯也可以唤醒闪烁）
//#define TCFG_LED_PWMCH_PORT                 PWMCH1_L    //LED背景灯的PWMCH通道

//#define TCFG_ADKEY_ENABLE                   1           //AD按键
//#define TCFG_ADKEY_PORT                     IO_PORTB_01 //ADC按键IO
#define TCFG_IOKEY_ENABLE                   1           //IO按键
#define TCFG_IOKEY_PORT                     IO_PORTB_01 //按键的IO
//#define CONFIG_PRESS_LONG_KEY_POWERON                   //长按开关

#define TCFG_DAC_MUTE_PORT                  IO_PORTH_01 //功放静音IO
#define TCFG_DAC_MUTE_VALUE                 0           //功放静音IO电平值
#define TCFG_DAC_AUTO_MUTE_ENABLE           0

#define TCFG_DAC_DIFF_OUT_ENABLE            1
#define TCFG_DAC_HW_CHANNEL_BIT             (BIT(0)|BIT(1)|BIT(2)|BIT(3))   //BIT(0):DACFL、 BIT(1):DACFR、 BIT(2):DACRL、 BIT(3):DACRR
#define TCFG_DAC_SOFT_CHANNEL_NUM           2                               //差分两个通道
#endif


//*********************************************************************************//
//                            摄像头引脚-应用的通道配置                            //
//*********************************************************************************//
#define TCFG_CAMERA_XCLK_PORT               IO_PORTC_00
#define TCFG_CAMERA_RESET_PORT              -1

#define TCFG_SW_IIC_CLK_PORT                IO_PORTA_00
#define TCFG_SW_IIC_DAT_PORT                IO_PORTH_02

#define TCFG_SW_IIC1_CLK_PORT                IO_PORTC_00
#define TCFG_SW_IIC1_DAT_PORT                IO_PORTC_02

//*********************************************************************************//
//                            AUDIO_ADC应用的通道配置                              //
//*********************************************************************************//
//PA回采方式：使用PA0/AUX0 PH4/AUX3 linein单端回采
#define CONFIG_AISP_AEC_USE_LINEIN          0       //1:使用linein回采，0不使用

#define AEC_MIC_P_N         (CONFIG_AISP_AEC_USE_LINEIN ? 0 : LADC_CH_MIC0_P_N)

#define CONFIG_AUDIO_ENC_SAMPLE_SOURCE      AUDIO_ENC_SAMPLE_SOURCE_MIC
#define TCFG_MIC_CHANNEL_MAP                (LADC_CH_MIC1_P_N | LADC_CH_MIC2_P_N | AEC_MIC_P_N)

/**********************************
//   LADC_CH_AUX0  AUX通道0 PA0
//   LADC_CH_AUX1  AUX通道1 PH7
//   LADC_CH_AUX2  AUX通道2 PA5
//   LADC_CH_AUX3  AUX通道3 PH4
**********************************/
#ifdef CONFIG_AUX_MUSIC_MODE_ENABLE
#define TCFG_LINEIN_CHANNEL_MAP             LADC_CH_AUX0// | LADC_CH_AUX3
#define TCFG_LINEIN_CHANNEL_NUM             1
#else
#if CONFIG_AISP_AEC_USE_LINEIN
#define TCFG_LINEIN_CHANNEL_MAP             LADC_CH_AUX3//LADC_CH_AUX0
#define TCFG_LINEIN_CHANNEL_NUM             1
#else
#define TCFG_LINEIN_CHANNEL_MAP             -1//LADC_CH_AUX0 | LADC_CH_AUX3
#define TCFG_LINEIN_CHANNEL_NUM             0
#endif
#endif

#define MIC0                                1   //mic0通道：对应LADC_CH_MIC1_P_N
#define MIC1                                2   //mic1通道：对应LADC_CH_MIC2_P_N
#define MIC2_AEC                            0   //回采AEC回声消除通道：对应LADC_CH_MIC0_P_N

#define TCFG_MIC_CHANNEL_NUM                (2 | (AEC_MIC_P_N ? 1 : 0))   //MIC通道数量

#define CONFIG_ASR_CLOUD_ADC_CHANNEL        MIC0        //云端识别mic通道
#define CONFIG_VOICE_NET_CFG_ADC_CHANNEL    MIC0        //声波配网mic通道
#define CONFIG_AISP_MIC0_ADC_CHANNEL        MIC0        //本地唤醒左mic通道
#define CONFIG_AISP_MIC1_ADC_CHANNEL        MIC1        //本地唤醒右mic通道
#define CONFIG_REVERB_ADC_CHANNEL           MIC0        //混响mic通道
#define CONFIG_PHONE_CALL_ADC_CHANNEL       MIC0        //通话mic通道
#define CONFIG_UAC_MIC_ADC_CHANNEL          MIC0        //UAC mic通道

#define AUDIO_RECORD_MIC_COUNT              2 //1单麦,2双麦

#ifndef CONFIG_AISP_MIC_ADC_GAIN
#define CONFIG_AISP_MIC_ADC_GAIN            90      //本地唤醒mic增益
#endif
#define CONFIG_AISP_AEC_ADC_GAIN            (CONFIG_AISP_MIC_ADC_GAIN - 10)     //本地AEC回采增益

#define CONFIG_AISP_MIC2_AEC_ADC_CHANNEL    MIC2_AEC        //本地使用硬件回采的回声消除mic通道

#define CONFIG_AISP_LINEIN_ADC_GAIN         CONFIG_AISP_AEC_ADC_GAIN
#define CONFIG_AISP_LINEIN_ADC_CHANNEL      (AEC_MIC_P_N ? 3 : 0)           //本地唤醒LINEIN回采DAC通道，值为:0/1/2/3，不能和MIC冲突使用，和TCFG_LINEIN_CHANNEL_MAP有关
#define CONFIG_AUDIO_LINEIN_CHANNEL         TCFG_LINEIN_CHANNEL_NUM         //LIENIN通道数，使用LINEIN回采DAC通道需要使能通道数>0
#define CONFIG_AUDIO_LINEIN_CHANNEL_MAP     TCFG_LINEIN_CHANNEL_MAP

#ifdef CONFIG_USB_DISK_MUSIC_MODE_ENABLE
#define TCFG_UDISK_ENABLE                   1     //U盘主机功能
#endif

#ifdef CONFIG_PROJECT_MUTEX
#error "project config can not enable at the same time, just select one!!!"
#else
#define CONFIG_PROJECT_MUTEX
#endif

#endif  //CONFIG_BOARD_DEVELOP

//*********************************************************************************//
//               海外离线语言配置（暂只支持打开一种语言配置）                      //
//*********************************************************************************//
#define CONFIG_INSTRUCTION_CN                 //中文指令（非海外版默认）

////东亚语言
//#define CONFIG_INSTRUCTION_YUE                //粤语指令
//#define CONFIG_INSTRUCTION_JA                 //日语指令
//#define CONFIG_INSTRUCTION_KO                 //韩语指令
////东南亚语言
//#define CONFIG_INSTRUCTION_TH                 //泰语指令
//#define CONFIG_INSTRUCTION_VI                 //越南语指令
//#define CONFIG_INSTRUCTION_ID                 //印尼语指令
//#define CONFIG_INSTRUCTION_MS                 //马来语指令
//#define CONFIG_INSTRUCTION_MY                 //缅甸语指令
//#define CONFIG_INSTRUCTION_KM                 //高棉语指令
//#define CONFIG_INSTRUCTION_LO                 //老挝语指令
////南亚语言
//#define CONFIG_INSTRUCTION_HI                 //印地语指令
//#define CONFIG_INSTRUCTION_BN                 //孟加拉语指令
//#define CONFIG_INSTRUCTION_TA                 //泰米尔语指令
//#define CONFIG_INSTRUCTION_TE                 //泰卢固语指令
//#define CONFIG_INSTRUCTION_GU                 //古吉拉特语指令
//#define CONFIG_INSTRUCTION_KN                 //卡纳达语指令
//#define CONFIG_INSTRUCTION_ML                 //马拉雅拉姆语指令
//#define CONFIG_INSTRUCTION_UR                 //乌尔都语指令
//#define CONFIG_INSTRUCTION_PA                 //旁遮普语指令
////欧洲语言
#define CONFIG_INSTRUCTION_EN                 //英语指令
//#define CONFIG_INSTRUCTION_DE                 //德语指令
//#define CONFIG_INSTRUCTION_FR                 //法语指令
//#define CONFIG_INSTRUCTION_ES                 //西班牙语指令
//#define CONFIG_INSTRUCTION_RU                 //俄语指令
//#define CONFIG_INSTRUCTION_PT                 //葡萄牙语指令
//#define CONFIG_INSTRUCTION_NL                 //荷兰语指令
//#define CONFIG_INSTRUCTION_PL                 //波兰语指令
//#define CONFIG_INSTRUCTION_UK                 //乌克兰语指令
//#define CONFIG_INSTRUCTION_LT                 //立陶宛语指令
////中东语言
//#define CONFIG_INSTRUCTION_AR                 //阿拉伯语指令
//#define CONFIG_INSTRUCTION_HE                 //希伯来语指令
//#define CONFIG_INSTRUCTION_FA                 //波斯语指令
//#define CONFIG_INSTRUCTION_TR                 //土耳其语指令



//*********************************************************************************//
//                        SD 配置（暂只支持打开一个SD外设）                        //
//*********************************************************************************//
//SD0   cmd,  clk,  data0, data1, data2, data3
//A     PB6   PB7   PB5    PB5    PB3    PB2
//B     PA7   PA8   PA9    PA10   PA5    PA6
//C     PH1   PH2   PH0    PH3    PH4    PH5
//D     PC9   PC10  PC8    PC7    PC6    PC5

//SD1   cmd,  clk,  data0, data1, data2, data3
//A     PH6   PH7   PH5    PH4    PH3    PH2
//B     PC0   PC1   PC2    PC3    PC4    PC5

#endif

