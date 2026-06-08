#include "app_config.h"
#include "system/includes.h"
#include "device/device.h"
#include "uart.h"
#include "os/os_api.h"
#include "ai_uart_ctrol.h"
#include "event/key_event.h"

#ifdef AT_UART_CMD_ENABLE


// 定义命令结构体，用于存储AT命令表
typedef struct {
    const char *cmd_str;
} at_command_t;

// AT命令表，按照功能分类组织
static const at_command_t at_commands[] = {
    // AT使能命令
    {AT_CMD_ON},
    {AT_CMD_OFF},

    // AI对话相关命令
    {AT_CMD_DIALUOGE_START},
    {AT_CMD_REC_START},
    {AT_CMD_REC_END},
    {AT_CMD_AI_DIALUOGE_STOP},
    {AT_CMD_DIALUOGE_CLOSE},
    {AT_CMD_AI_MODE},

    // 数据类型相关命令
    {AT_CMD_STT},
    {AT_CMD_TTS},
    {AT_CMD_ALARM},
    {AT_CMD_ALARM_RING},
    {AT_CMD_TIME_COUNT_DOWN},
    {AT_CMD_TIME_COUNT_RING},
    {AT_CMD_ALARM_DEL},
    {AT_CMD_ALARM_DEL_ALL},
    {AT_CMD_NTP_TIME},
    {AT_CMD_WEATHER_FORECAST},
    {AT_CMD_AI_INTELL_AGENT},

    // 系统电源相关命令
    {AT_CMD_PWR_ON},
    {AT_CMD_PWR_OFF},

    // 音乐控制相关命令
    {AT_CMD_MUSIC_START},
    {AT_CMD_MUSIC_PUASE},
    {AT_CMD_MUSIC_CONTINUE},
    {AT_CMD_MUSIC_STOP},
    {AT_CMD_MUSIC_NEXT},
    {AT_CMD_MUSIC_LAST},
    {AT_CMD_MUSIC_LOOP},
    {AT_CMD_MUSIC_LOOP_OFF},
    {AT_CMD_MUSIC_LOOP_ALL},
    {AT_CMD_UDISK_MODE},
    {AT_CMD_SD_TF_MODE},
    {AT_CMD_BT_MODE},
    {AT_CMD_AUX_MODE},
    {AT_CMD_MODE_CHANGE},
    {AT_CMD_NET_MUSIC_PLAY},
    {AT_CMD_NET_MUSIC_STOP},

    // 音量控制相关命令
    {AT_CMD_VOLUME_DEC},
    {AT_CMD_VOLUME_INC},
    {AT_CMD_VOLUME_SET},

    // WiFi相关命令
    {AT_CMD_WIFI_CONFIG},
    {AT_CMD_GET_WIFI_SSID},
    {AT_CMD_SET_WIFI_SSID},
    {AT_CMD_GET_NET_CONNET},
    {AT_CMD_WIFI_DISCON},
    {AT_CMD_WIFI_CONNECTED},

    // 蓝牙相关命令
    {AT_CMD_GET_BT_SSID},
    {AT_CMD_SET_BT_SSID},
    {AT_CMD_GET_BT_CONNET},
    {AT_CMD_DISCON_BT},
    {AT_CMD_CONNECT_BT},
    {AT_CMD_BT_DISCON},
    {AT_CMD_BT_CONNECTED},

    // 电池相关命令
    {AT_CMD_GET_VBAT},

    // 错误相关命令
    {AT_CMD_ERROR},
    {AT_CMD_NO_SUPPORT},
    {AT_CMD_MIC_DISABLE},
    {AT_CMD_MIC_ENABLE},
    {AT_CMD_SPEAKER_DISABLE},
    {AT_CMD_SPEAKER_ENABLE},

#if defined TCFG_LED_PWM0_PORT && TCFG_LED_PWM0_PORT != -1
    // 灯光控制
    {AI_CMD_LED_OPEN},
    {AI_CMD_LED_CLOSE},
    {AI_CMD_LED_EARTH_OPEN},
    {AI_CMD_LED_EARTH_CLOSE},
    {AI_CMD_LED_BREATH_CLOSE},
    {AI_CMD_LED_BIRGHT_INC},
    {AI_CMD_LED_BIRGHT_DEC},
    {AI_CMD_LED_BIRGHT_MAX},
#endif

};

// 命令表大小
#define AT_COMMAND_COUNT (sizeof(at_commands) / sizeof(at_commands[0]))

static char at_cmd_on = 0;
static void *uart_hdl = NULL;
static unsigned char recv_buf[AT_UART_BUFFER_SIZE] ALIGNED(4);
static unsigned char uart_buf[1024]; // 串口接收缓存
static OS_SEM msg_sem;
static OS_MUTEX mtx;
static char g_asr_tts_result_buf[512] ALIGNED(4);

// 发送数据到串口
static void uart_send_data(char *data)
{
    if (uart_hdl && data) {
        os_mutex_pend(&mtx, 100);
        dev_write(uart_hdl, data, strlen(data) + 1);
        os_mutex_post(&mtx);
        printf("AT_CMD: %s\n", data);
    }
}
void at_uart_cmd_send(int index, char *data)
{
    if (!at_cmd_on) {
        return;
    }
    printf("send AT cmd %d : %s \n", at_cmd_on, data);
    os_mutex_pend(&mtx, 100);
    memset(g_asr_tts_result_buf, 0, sizeof(g_asr_tts_result_buf));
    switch (index) {
    // AI对话类型 (48-63)
    case AI_UART_CMD_DIALUOGE_START:        //48：启动AI对话
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "AI_START\r\n");
        break;
    case AI_UART_CMD_REC_START:             //49：正在录音
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "AI_REC_START\r\n");
        break;
    case AI_UART_CMD_REC_END:               //50：结束录音
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "AI_REC_END\r\n");
        break;
    case AI_UART_CMD_DIALUOGE_PLAY_START:   //51：AI对话音频开始播放
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "AT_DIALOGUE_PLAY_START\r\n");
        break;
    case AI_UART_CMD_DIALUOGE_PLAY_END:     //52：AI对话音频结束播放
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "AT_DIALOGUE_PLAY_END\r\n");
        break;

    // 数据类型 (64-79)
    case AI_UART_CMD_STT:                   //64：语音识别-stt
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "STT=%s\r\n", data);
        break;
    case AI_UART_CMD_TTS:                   //65：AI对话-tts
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "TTS=%s\r\n", data);
        break;
    case AI_UART_CMD_ALARM:                 //66：闹钟设置
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "ALARM_SET=%s\r\n", data);
        break;
    case AI_UART_CMD_ALARM_RING:            //67：闹钟时间到
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "ALARM_RING=%s\r\n", data);
        break;
    case AI_UART_CMD_TIME_COUNT_DOWN:       //68：计时器
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "TIMER_SET=%s\r\n", data);
        break;
    case AI_UART_CMD_TIME_COUNT_RING:       //69：计时器时间到
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "TIMER_RING=%s\r\n", data);
        break;
    case AI_UART_CMD_ALARM_DEL:             //70：删除闹钟
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "ALARM_DEL=%s\r\n", data);
        break;
    case AI_UART_CMD_ALARM_DEL_ALL:         //71：删除所有闹钟
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "ALARM_DEL_ALL\r\n");
        break;
    case AI_UART_CMD_AI_INTELL_AGENT_INFO:
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "AI_INTELL_AGENT=%s\r\n", data);
        break;
    // 系统命令 + 音乐命令类型 (79-100)
    case AI_UART_CMD_PWR_ON:                //79：开机
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "POWER_ON\r\n");
        break;
    case AI_UART_CMD_PWR_OFF:               //80：关机
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "POWER_OFF\r\n");
        break;
    case AI_UART_CMD_DIALUOGE_CLOSE:        //81：对话关闭
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "DIALOG_CLOSE\r\n");
        break;
    case AI_UART_CMD_MUSIC_START:           //82：播放音乐
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "MUSIC_PLAY\r\n");
        break;
    case AI_UART_CMD_MUSIC_PUASE:           //83：暂停播放
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "MUSIC_PAUSE\r\n");
        break;
    case AI_UART_CMD_MUSIC_CONTINUE:        //84：继续播放
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "MUSIC_RESUME\r\n");
        break;
    case AI_UART_CMD_MUSIC_STOP:            //85：停止播放
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "MUSIC_STOP\r\n");
        break;
    case AI_UART_CMD_MUSIC_NEXT:            //86：下一首
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "MUSIC_NEXT\r\n");
        break;
    case AI_UART_CMD_MUSIC_LAST:            //87：上一首
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "MUSIC_PREV\r\n");
        break;
    case AI_UART_CMD_MUSIC_LOOP:            //88：单曲循环
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "MUSIC_LOOP\r\n");
        break;
    case AI_UART_CMD_VOLUME_DEC:            //89：小声一点
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "VOLUME_DEC\r\n");
        break;
    case AI_UART_CMD_VOLUME_INC:            //90：大声一点
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "VOLUMEL_INC\r\n");
        break;
    case AI_UART_CMD_VOLUME_SET:            //91：音量设置
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "VOLUME_SET=%d\r\n", (int)data);
        break;
    case AI_UART_CMD_MUSIC_LOOP_EXIT:        //92：取消单曲循环
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "MUSIC_LOOP_OFF\r\n");
        break;
    case AI_UART_CMD_NET_MUSIC_PLAY:        //93：正在播放网络音乐
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "NET_MUSIC_PLAY\r\n");
        break;
    case AI_UART_CMD_NET_MUSIC_STOP:        //94：停止播放网络音乐
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "NET_MUSIC_STOP\r\n");
        break;

    // 设备控制命令类型 (100-121)
    case AI_UART_CMD_LED_OPEN:              //100：打开灯光
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "LED_OPEN\r\n");
        break;
    case AI_UART_CMD_LED_CLOSE:             //101：关闭灯光
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "LED_CLOSE\r\n");
        break;
    case AI_UART_CMD_LED_EARTH_OPEN:        //102：打开球体灯
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "LED_EARTH_OPEN\r\n");
        break;
    case AI_UART_CMD_LED_EARTH_CLOSE:       //103：关闭球体灯
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "LED_EARTH_CLOSE\r\n");
        break;
    case AI_UART_CMD_LED_BREATH_OPEN:       //104：打开呼吸灯
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "LED_BREATH_OPEN\r\n");
        break;
    case AI_UART_CMD_LED_BIRGHT_INC:        //105：灯光亮一点
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "LED_BRIGHT_INC\r\n");
        break;
    case AI_UART_CMD_LED_BIRGHT_DEC:        //106：灯光暗一点
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "LED_BRIGHT_DEC\r\n");
        break;
    case AI_UART_CMD_LED_BIRGHT_MAX:        //107：灯光最亮
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "LED_BRIGHT_MAX\r\n");
        break;

    case AI_UART_CMD_UDISK_MODE:            //108：播放U盘
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "UDISK_MODE\r\n");
        break;
    case AI_UART_CMD_SD_TF_MODE:            //109：播放SD/TF卡
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "SD_MODE\r\n");
        break;
    case AI_UART_CMD_BT_MODE:               //110：播放蓝牙
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "BT_MODE\r\n");
        break;
    case AI_UART_CMD_AUX_MODE:              //111：线路模式
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "AUX_MODE\r\n");
        break;
    case AI_UART_CMD_AI_MODE:               //112：AI模式
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "AI_MODE\r\n");
        break;
    case AI_UART_CMD_MODE_CHENG:            //113：模式切换
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "MODE_SWITCH\r\n");
        break;
    case AI_UART_CMD_WIFI_CONFIG:           //114：wifi配网模式
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "WIFI_CONFIG\r\n");
        break;
    case AI_UART_CMD_TIME_SHOWN_OPEN:       //116：打开时间显示
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "TIME_SHOWN_OPEN\r\n");
        break;
    case AI_UART_CMD_TIME_SHOWN_CLOSE:      //117：关闭时间显示
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "TIME_SHOWN_CLOSE\r\n");
        break;
    case AI_UART_CMD_SHOWN_BIRGHT_INC:      //118：显示亮一点
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "SHOWN_BRIGHT_INC\r\n");
        break;
    case AI_UART_CMD_SHOWN_BIRGHT_DEC:      //119：显示暗一点
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "SHOWN_BRIGHT_DEC\r\n");
        break;


    case AI_UART_CMD_BT_DISCON:             //145：蓝牙已断开
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "BT_DISCONNECTED\r\n");
        break;
    case AI_UART_CMD_BT_CONNECTED:          //146：已连接蓝牙
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "BT_CONNECTED\r\n");
        break;
    case AI_UART_CMD_WIFI_DISCON:           //147：WiFi已断开
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "WIFI_DISCONNECTED\r\n");
        break;
    case AI_UART_CMD_WIFI_CONNECTED:        //148：已连接WIFI
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "WIFI_CONNECTED\r\n");
        break;

    default:
        // 未知命令处理
        snprintf(g_asr_tts_result_buf, sizeof(g_asr_tts_result_buf), "UNKNOWN_CMD\r\n");
        break;
    }
    uart_send_data(g_asr_tts_result_buf);
    os_mutex_post(&mtx);
}

// 检查是否收到AT命令，并识别具体命令类型
static const char *check_at_command(unsigned char *buf, int len)
{
    // 转换为字符串以便比较
    char cmd_str[AT_UART_BUFFER_SIZE + 1];
    if (len > AT_UART_BUFFER_SIZE) {
        len = AT_UART_BUFFER_SIZE;
    }
    memcpy(cmd_str, buf, len);
    cmd_str[len] = '\0';

    // 快速检查是否包含"AT"字符串
    if (strstr(cmd_str, "AT") == NULL) {
        return NULL;
    }

    // 遍历命令表查找匹配的命令
    for (int i = 0; i < AT_COMMAND_COUNT; i++) {
        if (strstr(cmd_str, at_commands[i].cmd_str)) {
            return at_commands[i].cmd_str;
        }
    }

    // 如果是其他AT命令但不是具体的命令，返回通用AT标识
    return AT_CMD;
}

static int uart_send_respond_cmd(char err_status)
{
    // 发送OK响应
    if (err_status) {
        uart_send_data(AT_ERR);
    } else {
        uart_send_data(AT_OK);
    }
}
// 串口接收任务
static void uart_recv_task_main(void *priv)
{
    int ret;
    struct key_event key = {0};
    char err_status = 0;
    key.type = KEY_EVENT_USER;
    key.action = KEY_EVENT_CLICK;
    char sbuf[128] = {0};
    while (1) {
        /* 接收数据前先清空缓冲区，避免数据残留 */
        memset(recv_buf, 0, sizeof(recv_buf));
        int len = dev_read(uart_hdl, recv_buf, sizeof(recv_buf));
        if (len > 0) {
            printf("recv: %d, %s\n", len, recv_buf);

            // 检查是否收到AT命令，并识别具体命令
            const char *cmd = check_at_command(recv_buf, len);
            if (cmd) {
                // 根据不同命令类型打印不同信息
                printf("cmd: %s\n", cmd);
                music_play_res_file("KeyVol.mp3");
                if (strstr(cmd, AT_CMD_ON)) {
                    at_cmd_on = true;
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_OFF)) {
                    at_cmd_on = false;
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_DIALUOGE_START)) {
                    aisp_wake(0);
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_REC_START)) {
                    key_vad_pcm_send_set_status(1, 0);
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_REC_END)) {
                    key_vad_pcm_send_set_status(0, 1);
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_PWR_OFF)) {
                    aisp_all_pause(1);
                    music_play_waite();
                    music_play_res_file("PwrOff.mp3");
                    music_play_waite();
                    uart_send_data(AT_OK);
                    sys_power_poweroff();
                }
                if (strstr(cmd, AT_CMD_DIALUOGE_CLOSE) || strstr(cmd, AT_CMD_AI_DIALUOGE_STOP)) {
                    aisp_clear(0);
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_MIC_DISABLE)) {
                    // 禁用麦克风
                    aisp_all_pause(1);
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_MIC_ENABLE)) {
                    // 恢复麦克风
                    aisp_all_pause(0);
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_SPEAKER_DISABLE)) {
                    // 禁用喇叭
                    dac_mute_control(1, 1); // 1表示静音
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_SPEAKER_ENABLE)) {
                    // 恢复喇叭
                    dac_mute_control(0, 1); // 0表示取消静音
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_NET_MUSIC_PLAY) != NULL) {
                    net_music_play_next(net_music_play_type_get());
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_NET_MUSIC_STOP) != NULL) {
                    net_music_play_set_stop_notic();
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_MUSIC_START) != NULL) {
                    net_music_play_next(net_music_play_type_get());
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_MUSIC_PUASE) != NULL) {
                    if (ai_speaker_app()) {
                        net_music_play_puase();
                    } else {
                        key.type = KEY_EVENT_USER;
                        key.action = KEY_EVENT_CLICK;
                        key.value = KEY_SUPSPEND;
                        key_event_notify(KEY_EVENT_FROM_USER, &key);
                    }
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_MUSIC_CONTINUE) != NULL) {
                    key.value = KEY_RESUM;
                    key_event_notify(KEY_EVENT_FROM_USER, &key);
                    if (ai_speaker_app()) {
                        if (music_buf_play_supspend()) {
                            extern void net_music_play_resum(void);
                            sys_timeout_add_to_task("sys_timer", NULL, net_music_play_resum, 1000);//延时1s可能在播放会在播放前socke先关闭有再见的提示音
                        } else {
                            net_music_play_next(net_music_play_type_get());
                        }
                    } else {
                        key.type = KEY_EVENT_USER;
                        key.action = KEY_EVENT_CLICK;
                        key.value = KEY_RESUM;
                        key_event_notify(KEY_EVENT_FROM_USER, &key);
                    }
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_MUSIC_STOP) != NULL) {
                    if (ai_speaker_app()) {
                        net_music_play_set_stop_notic();
                    } else {
                        key.type = KEY_EVENT_USER;
                        key.action = KEY_EVENT_CLICK;
                        key.value = KEY_SUPSPEND;
                        key_event_notify(KEY_EVENT_FROM_USER, &key);
                    }
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_MUSIC_NEXT) != NULL) {
                    if (ai_speaker_app()) {
                        net_music_play_next(net_music_play_type_get());
                    } else {
                        key.type = KEY_EVENT_USER;
                        key.action = KEY_EVENT_CLICK;
                        key.value = KEY_DOWN;
                        key_event_notify(KEY_EVENT_FROM_USER, &key);
                    }
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_MUSIC_LAST) != NULL) {
                    if (ai_speaker_app()) {
                        if (net_music_play_last_check()) {
                            net_music_play_last_request();
                        } else {
                            net_music_play_next(net_music_play_type_get());
                        }
                    } else {
                        key.type = KEY_EVENT_USER;
                        key.action = KEY_EVENT_CLICK;
                        key.value = KEY_UP;
                        key_event_notify(KEY_EVENT_FROM_USER, &key);
                    }
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_MUSIC_LOOP) != NULL) {
                    net_music_play_loop();
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_MUSIC_LOOP_OFF) != NULL) {
                    net_music_play_loop_clear();
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_VOLUME_DEC) != NULL) {
                    key.value = KEY_VOLUME_DEC;
                    key_event_notify(KEY_EVENT_FROM_USER, &key);
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_VOLUME_INC) != NULL) {
                    key.value = KEY_VOLUME_INC;
                    key_event_notify(KEY_EVENT_FROM_USER, &key);
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_VOLUME_SET) != NULL) {  // 检查是否包含前缀
                    int volume = 0;
                    // 使用完整接收缓冲区解析，因为cmd可能只包含命令前缀
                    if (sscanf((char *)recv_buf, "AT+VOL_SET=%d", &volume) == 1) {
                        printf("VOL_SET: %d\n", volume);
                        // 使用音量值
                        sys_all_volume_auto_set(volume);
                    } else {
                        // 备用解析方法
                        char *eq_pos = strstr((char*)recv_buf, "=");
                        if (eq_pos) {
                            volume = atoi(eq_pos + 1);
                            printf("VOL_SET 1: %d\n", volume);
                        }
                    }
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_STT) != NULL) {
                    // 定义足够大的缓冲区存储STT结果
                    //char stt_result[256] = {0};
                }
                if (strstr(cmd, AT_CMD_TTS) != NULL) {
                    // 发送TTS响应文本

                    char *at_tts_val = NULL;
                    // 只提取双引号之间的内容作为TTS文本
                    static char tts_content[256];

                    // 直接从完整接收缓冲区解析，因为cmd可能只包含命令前缀
                    char *tts_quote_start = strchr((char*)recv_buf, '"');
                    if (tts_quote_start) {
                        // 找到结束的引号
                        char *tts_quote_end = strchr(tts_quote_start + 1, '"');
                        if (tts_quote_end) {
                            // 提取TTS内容（不包括引号）
                            int tts_len = tts_quote_end - tts_quote_start - 1;
                            if (tts_len > 0 && tts_len < sizeof(tts_content)) {
                                memcpy(tts_content, tts_quote_start + 1, tts_len);
                                tts_content[tts_len] = '\0'; // 确保字符串正确终止
                                at_tts_val = tts_content;
                                printf("解析到TTS参数: %s\n", at_tts_val);
                            }
                        }
                    }

                    // 如果没有成功解析，使用空字符串作为默认值
                    if (!at_tts_val) {
                        tts_content[0] = '\0';
                        at_tts_val = tts_content;
                        printf("未找到有效的双引号内容，使用空字符串\n");
                    }
                    http_tts_request(at_tts_val, strlen(at_tts_val));
                    http_tts_play_wait();
                    aisp_mic_gain_resum();
                    uart_send_respond_cmd(0);
                }
#if defined CONFIG_BT_ENABLE && defined CONFIG_BT_MUSIC_MODE_ENABLE
                if (strstr(cmd, AT_CMD_UDISK_MODE) != NULL) {
                    extern int audio_app_mode_switch(char *name);
                    audio_app_mode_switch("usbdisk_music");
                    uart_send_respond_cmd(0);
                }
#endif
#ifdef CONFIG_SD_MUSIC_MODE_ENABLE
                if (strstr(cmd, AT_CMD_SD_TF_MODE) != NULL) {
                    extern int audio_app_mode_switch(char *name);
                    audio_app_mode_switch("sd_music");
                    uart_send_respond_cmd(0);
                }
#endif
#if defined CONFIG_BT_ENABLE && defined CONFIG_BT_MUSIC_MODE_ENABLE
                if (strstr(cmd, AT_CMD_BT_MODE) != NULL) {
                    extern int audio_app_mode_switch(char *name);
                    audio_app_mode_switch("bt_music");
                    uart_send_respond_cmd(0);
                }
#endif
#ifdef CONFIG_AUX_MUSIC_MODE_ENABLE
                if (strstr(cmd, AT_CMD_AUX_MODE) != NULL) {
                    extern int audio_app_mode_switch(char *name);
                    audio_app_mode_switch("aux_music");
                    uart_send_respond_cmd(0);
                }
#endif
#ifdef CONFIG_ASR_ALGORITHM_ENABLE
                if (strstr(cmd, AT_CMD_AI_MODE) != NULL) {
                    extern int audio_app_mode_switch(char *name);
                    audio_app_mode_switch("ai_speaker");
                    uart_send_respond_cmd(0);
                }
#endif
                if (strstr(cmd, AT_CMD_MODE_CHANGE) != NULL) {
                    extern int audio_app_mode_switch(char *name);
                    audio_app_mode_switch(NULL);
                    uart_send_respond_cmd(0);
                }
#if defined CONFIG_BT_ENABLE && defined CONFIG_BT_MUSIC_MODE_ENABLE
                if (strstr(cmd, AT_CMD_CONNECT_BT) != NULL) {
                    bt_connection_enable();
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_DISCON_BT) != NULL) {
                    bt_connection_disable();
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_GET_BT_SSID) != NULL) {
                    extern const char *bt_get_local_name(void);
                    extern char *ble_get_name(void);
                    char bt_name[64] = {0};
                    snprintf(bt_name, sizeof(bt_name), "BT_NAME=%s,BLE_NAME=%s\r\n", bt_get_local_name(), ble_get_name());
                    uart_send_data(bt_name);
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_GET_BT_CONNET) != NULL) {
                    printf("AT_CMD_GET_BT_CONNET\n");
                    // 声明外部函数
                    extern char bt_connect_check(void);
                    char bt_status = bt_connect_check();
                    // 增大缓冲区以防止溢出
                    char bt_connect_status[64] = {0};
                    // 使用统一的AT命令格式响应
                    snprintf(bt_connect_status, sizeof(bt_connect_status), "BT_STATUS:%s\r\n", bt_status ? "CONNECTED" : "DISCON");
                    uart_send_data(bt_connect_status);
                    uart_send_respond_cmd(0);
                }
#endif
                if (strstr(cmd, AT_CMD_SET_WIFI_SSID) != NULL) {
                    printf("AT_CMD_SET_WIFI_SSID\n");

                    // 解析命令中的SSID和密码，格式为: AT+WIFI_SET_SSID=ssid:"xxx"pwd:xxx
                    char *cmd_start = strstr((char *)recv_buf, AT_CMD_SET_WIFI_SSID);
                    if (cmd_start) {
                        char ssid[64] = {0};
                        char pwd[64] = {0};
                        int parse_success = 0;

                        // 1. 查找SSID部分 - 改进版：先找到等号，再找ssid标签
                        char *eq_pos = strchr(cmd_start, '=');
                        if (eq_pos) {
                            char *ssid_label = strstr(eq_pos, "ssid:");
                            if (ssid_label) {
                                // 找到SSID开始的引号
                                char *ssid_quote_start = strchr(ssid_label + 5, '"');
                                if (ssid_quote_start) {
                                    // 找到SSID结束的引号
                                    char *ssid_quote_end = strchr(ssid_quote_start + 1, '"');
                                    if (ssid_quote_end) {
                                        // 提取SSID内容（不包括引号）
                                        int ssid_len = ssid_quote_end - ssid_quote_start - 1;
                                        if (ssid_len > 0 && ssid_len < sizeof(ssid)) {
                                            memcpy(ssid, ssid_quote_start + 1, ssid_len);
                                            ssid[ssid_len] = '\0'; // 确保字符串正确终止，特别是对于多字节字符
                                        }

                                        // 2. 查找密码部分 - 改进版：处理密码直到命令结束或遇到控制字符
                                        char *pwd_label = strstr(ssid_quote_end, "pwd:");
                                        if (pwd_label) {
                                            // 提取密码内容，直到遇到换行符、回车符或字符串结束
                                            char *pwd_start = pwd_label + 4;
                                            char *pwd_end = pwd_start;
                                            // 查找密码结束位置，排除控制字符
                                            while (*pwd_end && *pwd_end != '\n' && *pwd_end != '\r' && *pwd_end != '\0') {
                                                pwd_end++;
                                            }

                                            int pwd_len = pwd_end - pwd_start;
                                            if (pwd_len > 0 && pwd_len < sizeof(pwd)) {
                                                strncpy(pwd, pwd_start, pwd_len);
                                                pwd[pwd_len] = '\0'; // 确保字符串结束
                                            }

                                            printf("解析到的SSID: %s, 密码: %s\n", ssid, pwd);

                                            // 验证SSID和密码是否有效
                                            if (strlen(ssid) > 0) {
                                                extern volatile int keyworld_wifi_enter_congfig;
                                                keyworld_wifi_enter_congfig = 2;
                                                // wifi_sta_connect是无返回值函数
                                                wifi_sta_connect(ssid, pwd, 1);
                                                parse_success = 1;
                                            } else {
                                                printf("err：no SSID\n");
                                            }
                                        } else {
                                            printf("warning : no pwd\n");
                                            extern volatile int keyworld_wifi_enter_congfig;
                                            keyworld_wifi_enter_congfig = 2;
                                            // wifi_sta_connect是无返回值函数
                                            wifi_sta_connect(ssid, NULL, 1);
                                            parse_success = 1;

                                        }
                                    } else {
                                        printf("err: SSID\n");
                                        err_status = true;
                                    }
                                } else {
                                    printf("err: SSID\n");
                                    err_status = true;
                                }
                            } else {
                                printf("err: SSID\n");
                                err_status = true;
                            }
                        } else {
                            printf("err: SSID\n");
                            err_status = true;
                        }
                    }
                    uart_send_respond_cmd(err_status);
                    err_status = 0;
                }
                if (strstr(cmd, AT_CMD_WIFI_CONFIG) != NULL) {
                    aisp_wake(8);
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_GET_WIFI_SSID) != NULL) {
                    extern void wifi_sta_mode_info(char **ssid, char **pwd);
                    char *ssid = NULL;
                    char *pwd = NULL;
                    wifi_sta_mode_info(&ssid, &pwd);
                    // 添加响应缓冲区
                    if (ssid && ssid[0]) {
                        if (pwd && pwd[0]) {
                            sprintf(sbuf, "SSID=%s,PWD=%s\r\n", ssid, pwd);
                        } else {
                            sprintf(sbuf, "SSID=%s,PWD=NULL\r\n", ssid);
                        }
                        printf("-->GET SSID: %s, PWD: %s\n", ssid, pwd ? pwd : "NULL");
                        uart_send_data(sbuf);
                    }
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_GET_NET_CONNET) != NULL) {
                    // 获取网络连接状态
                    int net_status = sys_connect_net_success();
                    // 增大缓冲区以防止溢出
                    char net_connect_status[64] = {0};
                    // 使用统一的AT命令格式响应
                    snprintf(net_connect_status, sizeof(net_connect_status), "NET_STATUS:%s\r\n", net_status ? "CONNECTED" : "DISCON");
                    uart_send_data(net_connect_status);
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_GET_VBAT) != NULL) {
                    // 获取电池电量
#if (defined TCFG_VBAT_CHECK_EN && TCFG_VBAT_CHECK_EN)
                    int sys_get_vbat_percent(void);
                    int vbat = sys_get_vbat_percent();
#else
                    int vbat = 0;
#endif
                    // 增大缓冲区以防止溢出
                    char vbat_status[16] = {0};
                    snprintf(vbat_status, sizeof(vbat_status), "VBAT=%d%%\r\n", vbat);
                    uart_send_data(vbat_status);
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AT_CMD_NTP_TIME) != NULL) {
                    if (!sys_connect_net_success()) {
                        uart_send_data("NET CONNECT ERR\r\n");
                        uart_send_respond_cmd(1);
                    } else {
                        struct sys_time s_tm = {0};
                        void *rtc_hdl = dev_open("rtc", NULL);
                        if (rtc_hdl) {
                            dev_ioctl(rtc_hdl, IOCTL_GET_SYS_TIME, (u32)&s_tm);
                            dev_close(rtc_hdl);
                            sprintf(sbuf, "TIME=%4d-%2d-%2d %2d:%2d:%2d\r\n", s_tm.year, s_tm.month, s_tm.day, s_tm.hour, s_tm.min, s_tm.sec);
                            uart_send_data(sbuf);
                            uart_send_respond_cmd(0);
                        } else {
                            uart_send_respond_cmd(1);
                        }
                    }
                }
                if (strstr(cmd, AT_CMD_WEATHER_FORECAST) != NULL) {
                    if (!sys_connect_net_success()) {
                        uart_send_data("NET CONNECT ERR\r\n");
                        uart_send_respond_cmd(1);
                    } else {
                        char *pbuf = (char *)malloc(2048);
                        if (pbuf) {
                            memset(pbuf, 0, 2048);
                            ret = qyai_weather_forecast_get(NULL, 0, pbuf, 2048);
                            if (ret > 0) {
                                strcat(pbuf, "\r\n");
                                uart_send_data("WEATHER_FORECAST=");
                                uart_send_data(pbuf);
                                uart_send_respond_cmd(0);
                            } else {
                                uart_send_data(ret == -1 ? "No Bind Account" : "Server connect err");
                                uart_send_respond_cmd(1);
                            }
                            free(pbuf);
                        } else {
                            uart_send_respond_cmd(1);
                        }
                    }
                }
                if (strstr(cmd, AT_CMD_AI_INTELL_AGENT) != NULL) {
                    if (!sys_connect_net_success()) {
                        uart_send_data("NET CONNECT ERR\r\n");
                        uart_send_respond_cmd(1);
                    } else {
                        char *cmd_start = strstr(recv_buf, AT_CMD_AI_INTELL_AGENT);
                        if (cmd_start) {
                            char *start_pos = strchr(cmd_start, '=');
                            if (start_pos) {
                                start_pos++;
                                int id = atoi(start_pos);
                                if (id > 0) {
                                    ret = http_ai_intell_set_by_id(id);
                                    uart_send_respond_cmd(ret != 0 ? 1 : 0);
                                } else {
                                    char *end_pos = strstr(cmd_start, "\r\n");
                                    while (*start_pos == ' ') {
                                        start_pos++;
                                    }
                                    if (end_pos) {
                                        *end_pos = 0;
                                    }
                                    printf("=========================设置智能体: %s======================\n", start_pos);
                                    ret = http_ai_intell_set_by_name(start_pos);
                                    if (end_pos) {
                                        *end_pos = '\r';
                                    }
                                    uart_send_respond_cmd(ret != 0 ? 1 : 0);
                                }
                            } else {
                                uart_send_data("No = XXX\r\n");
                                uart_send_respond_cmd(1);
                            }
                        } else {
                            uart_send_respond_cmd(1);
                        }
                    }
                }

#if defined TCFG_LED_PWM0_PORT && TCFG_LED_PWM0_PORT != -1
                if (strstr(cmd, AI_CMD_LED_OPEN)) {
                    void tm_light_open(char notice);
                    tm_light_open(0);
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AI_CMD_LED_CLOSE)) {
                    void tm_light_close(char notice);
                    tm_light_close(0);
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AI_CMD_LED_EARTH_OPEN)) {
                    void tm_light_breath(char open, char notice);
                    tm_light_breath(1, 0);
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AI_CMD_LED_EARTH_CLOSE)) {
                    void tm_light_breath(char open, char notice);
                    tm_light_breath(0, 0);
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AI_CMD_LED_BIRGHT_INC)) {
                    int tm_light_pwm_decinc(int inc, int notice);
                    tm_light_pwm_decinc(1, 0);
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AI_CMD_LED_BIRGHT_DEC)) {
                    int tm_light_pwm_decinc(int inc, int notice);
                    tm_light_pwm_decinc(0, 0);
                    uart_send_respond_cmd(0);
                }
                if (strstr(cmd, AI_CMD_LED_BIRGHT_MAX)) {
                    int tm_light_pwm_set(char percent, char notice);
                    tm_light_pwm_set(100, 0);
                    uart_send_respond_cmd(0);
                }
#endif

            } else {
                //uart_send_respond_cmd(1);
            }
        } else if (len == UART_CIRCULAR_BUFFER_WRITE_OVERLAY) {
            dev_ioctl(uart_hdl, UART_FLUSH, 0); // 清空缓冲区
        }
    }
}


// 串口主任务
static void AT_uart_task_main(void *priv)
{
    char name[32];
    os_sem_create(&msg_sem, 0);
    os_mutex_create(&mtx);

    // 打开串口
#ifdef AT_UART_CMD_ENABLE
    sprintf(name, "uart%d", AT_UART_CMD_ENABLE);
    uart_hdl = dev_open(name, NULL);
#else
    uart_hdl = dev_open("uart0", NULL);
#endif

    if (!uart_hdl) {
        printf("open uart err !!!\n");
        return;
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

    uart_recv_task_main(NULL);

    dev_close(uart_hdl);
}

// 初始化串口任务
static int AT_uart_task_init(void)
{
    if (production_test_io_get()) {
        return 0;
    }
    os_task_create(AT_uart_task_main, NULL, 10, 1600, 0, "at_uart_cmd_task");
    return 0;
}

// 注册为late初始化，确保在系统启动后初始化
late_initcall(AT_uart_task_init);

#endif

