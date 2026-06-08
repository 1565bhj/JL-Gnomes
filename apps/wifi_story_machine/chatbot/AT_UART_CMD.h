#ifndef __AT_UART_CMD_H__
#define __AT_UART_CMD_H__

// 基础AT命令定义
#define AT_OK       "OK\r\n"
#define AT_ERR      "ERROR\r\n"
#define AT_CMD      "AT"

// 串口缓冲区大小
#define AT_UART_BUFFER_SIZE  256


// AT使能命令
#define AT_CMD_ON                 "AT+ON"       // AT使能命令（开启则AI模组对话等都会发数据到串口）
#define AT_CMD_OFF                "AT+OFF"      // AT禁用命令（关闭则只有请求命令的结果发在串口）

// AI对话相关AT命令
#define AT_CMD_DIALUOGE_START     "AT+AI_START"      // 启动AI对话功能，VAD实现自动对话功能
#define AT_CMD_REC_START          "AT+AI_REC_START"  // 开始录音，不适用VAD，直接录音
#define AT_CMD_REC_END            "AT+AI_REC_END"    // 结束录音
#define AT_CMD_AI_DIALUOGE_STOP   "AT+AI_STOP"       // 停止AI对话

// 数据类型相关AT命令
#define AT_CMD_STT                "AT+STT"           // 语音识别(Speech To Text)，暂时不支持请求
#define AT_CMD_TTS                "AT+TTS"           // 文字转语音(Text To Speech)
#define AT_CMD_ALARM              "AT+ALARM_SET"     // 设置闹钟
#define AT_CMD_ALARM_RING         "AT+ALARM_RING"    // 闹钟响铃
#define AT_CMD_TIME_COUNT_DOWN    "AT+TIMER_SET"     // 设置定时器
#define AT_CMD_TIME_COUNT_RING    "AT+TIMER_RING"    // 定时器响铃
#define AT_CMD_ALARM_DEL          "AT+ALARM_DEL"     // 删除特定闹钟
#define AT_CMD_ALARM_DEL_ALL      "AT+ALARM_DEL_ALL" // 删除所有闹钟
#define AT_CMD_NTP_TIME           "AT+NTP_TIME"      // 获取NTP时间
#define AT_CMD_WEATHER_FORECAST   "AT+WEATHER_FORECAST"// 获取天气预报
#define AT_CMD_AI_INTELL_AGENT    "AT+AI_INTELL_AGENT" // 设置智能体

// 系统和音乐命令相关AT命令
#define AT_CMD_PWR_ON             "AT+POWER_ON"      // 开机命令
#define AT_CMD_PWR_OFF            "AT+POWER_OFF"     // 关机命令
#define AT_CMD_DIALUOGE_CLOSE     "AT+DIALOG_CLOSE"  // 关闭对话功能
#define AT_CMD_MUSIC_START        "AT+MUSIC_PLAY"    // 播放音乐
#define AT_CMD_MUSIC_PUASE        "AT+MUSIC_PAUSE"   // 暂停音乐
#define AT_CMD_MUSIC_CONTINUE     "AT+MUSIC_RESUME"  // 继续播放音乐
#define AT_CMD_MUSIC_STOP         "AT+MUSIC_STOP"    // 停止播放音乐
#define AT_CMD_MUSIC_NEXT         "AT+MUSIC_NEXT"    // 播放下一首
#define AT_CMD_MUSIC_LAST         "AT+MUSIC_PREV"    // 播放上一首
#define AT_CMD_MUSIC_LOOP         "AT+MUSIC_LOOP"    // 单曲循环
#define AT_CMD_MUSIC_LOOP_OFF     "AT+MUSIC_LOOP_OFF" // 退出单曲循环
#define AT_CMD_VOLUME_DEC         "AT+VOLUME_DEC"      // 降低音量
#define AT_CMD_VOLUME_INC         "AT+VOLUMEL_INC"     // 增加音量
#define AT_CMD_VOLUME_SET         "AT+VOLUME_SET"  // 只保留固定前缀，AT+VOL_SET=90
#define AT_CMD_MUSIC_LOOP_ALL     "AT+MUSIC_LOOP_ALL" // 全部循环
#define AT_CMD_NET_MUSIC_PLAY     "AT+NET_MUSIC_PLAY"  // 播放网络音乐
#define AT_CMD_NET_MUSIC_STOP     "AT+NET_MUSIC_STOP"  // 停止播放网络音乐


#define AT_CMD_UDISK_MODE         "AT+UDISK_MODE"    // 播放U盘音乐
#define AT_CMD_SD_TF_MODE         "AT+SD_MODE"       // 播放SD卡音乐
#define AT_CMD_BT_MODE            "AT+BT_MODE"       // 播放蓝牙音乐
#define AT_CMD_AUX_MODE           "AT+AUX_MODE"      // 播放AUX音频输入
#define AT_CMD_AI_MODE            "AT+AI_MODE"       // 播放AI音频
#define AT_CMD_MODE_CHANGE        "AT+MODE_SWITCH"   // 切换模式

#define AT_CMD_WIFI_CONFIG        "AT+WIFI_CONFIG"   // 配置WIFI// 设备设置和信息查询命令相关AT命令
#define AT_CMD_GET_WIFI_SSID      "AT+WIFI_GET_SSID" // 获取当前连接的WIFI名称
#define AT_CMD_GET_BT_SSID        "AT+BT_GET_NAME"   // 获取蓝牙名称
#define AT_CMD_SET_WIFI_SSID      "AT+WIFI_SET_SSID" // 设置WIFI名称
#define AT_CMD_SET_BT_SSID        "AT+BT_SET_NAME"   // 设置蓝牙名称

#define AT_CMD_GET_VBAT           "AT+BATTERY_GET"   // 获取电池电量

#define AT_CMD_GET_NET_CONNET     "AT+WIFI_STATUS"   // 获取网络连接状态
#define AT_CMD_GET_BT_CONNET      "AT+BT_STATUS"     // 获取蓝牙连接状态
#define AT_CMD_DISCON_BT          "AT+BT_DISCONNECT" // 断开蓝牙连接
#define AT_CMD_CONNECT_BT         "AT+BT_CONNECT"    // 连接蓝牙
#define AT_CMD_BT_DISCON          "AT+BT_DISCONNECTED" // 蓝牙断开状态通知
#define AT_CMD_BT_CONNECTED       "AT+BT_CONNECTED"  // 蓝牙连接状态通知
#define AT_CMD_WIFI_DISCON        "AT+WIFI_DISCONNECTED" // WIFI断开状态通知
#define AT_CMD_WIFI_CONNECTED     "AT+WIFI_CONNECTED" // WIFI连接状态通知

// 错误相关AT命令
#define AT_CMD_ERROR              "AT+ERROR"         // 错误命令响应
#define AT_CMD_NO_SUPPORT         "AT+NOT_SUPPORT"   // 不支持的命令响应

// 音频设备控制命令
#define AT_CMD_MIC_DISABLE        "AT+MIC_DISABLE"   // 禁用麦克风
#define AT_CMD_MIC_ENABLE         "AT+MIC_ENABLE"    // 恢复麦克风
#define AT_CMD_SPEAKER_DISABLE    "AT+SPK_DISABLE"   // 禁用喇叭
#define AT_CMD_SPEAKER_ENABLE     "AT+SPK_ENABLE"    // 恢复喇叭

// 灯光控制
#define AI_CMD_LED_OPEN           "AT+LED_OPEN"       // 打开灯光
#define AI_CMD_LED_CLOSE          "AT+LED_CLOSE"      // 关闭灯光
#define AI_CMD_LED_EARTH_OPEN     "AT+LED_EARTH_OPEN" // 打开球体光
#define AI_CMD_LED_EARTH_CLOSE    "AT+LED_EARTH_CLOSE"// 关闭球体光
#define AI_CMD_LED_BREATH_CLOSE   "AT+LED_BREATH_CLOSE"// 关闭呼吸灯
#define AI_CMD_LED_BIRGHT_INC     "AT+LED_BIRGHT_INC" // 亮一点
#define AI_CMD_LED_BIRGHT_DEC     "AT+LED_BIRGHT_DEC" // 暗一点
#define AI_CMD_LED_BIRGHT_MAX     "AT+LED_BIRGHT_MAX" // 灯光最亮


#endif
