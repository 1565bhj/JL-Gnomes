#include "app_config.h"
#include "system/includes.h"
#include "device/device.h"
#include <time.h>
#include <sys/time.h>
#include "uart.h"
#include "syscfg/syscfg_id.h"
#include "event/key_event.h"
#include "event/net_event.h"
#include "os/os_api.h"
#include "device/gpio.h"
#include "ai_uart_ctrol.h"
#include "cJSON_common/cJSON.h"

#ifdef CONFIG_WIFI_ENABLE
#include "wifi/wifi_connect.h"
#endif

#ifdef AI_UART_CMD_CTROL_ENABLE
extern int utc_timer_update_get(struct sys_time *time);
static void *uart_hdl = NULL;
static char time_update_check = 0;
static u8 uart_buf[1024] __attribute__((aligned(32))); //用于串口接收缓存数据的循环buf
static void *lbuf_hdl = NULL;
static void *lbuffer = NULL;
static OS_SEM msg_sem;
unsigned char recv_buf[128] ALIGNED(32);

#define LBUF_MEM_BUF_SIZE   4096

extern int ai_uart_cmd_data_push(unsigned char data_type, char *buf, int len);//外文件使用的函数
extern int emoji_face_tts_callback(char *emoji, int len);
//发送校验和
static unsigned short data_check_sum(unsigned char *data, int len)
{
    int i;
    unsigned short sum = 0;
    for (i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}
static void uart_send_data(char *buf, int len)
{
    if (uart_hdl) {
        dev_write(uart_hdl, buf, len);
    }
}
static int ai_uart_data_send(void)
{
    struct ai_tx_uart_info *tx_info = NULL;
    if (!lbuf_hdl) {
        return -1;
    }
    tx_info = lbuf_pop(lbuf_hdl, BIT(0));
    if (tx_info) {
        if (tx_info->head == SEND_HEAD) {
            uart_send_data(tx_info, sizeof(struct ai_tx_uart_info) + tx_info->len);
        }
        lbuf_free(tx_info);
    }
    return 0;
}
static int ai_uart_data_recv(char *buf, int len)
{
    struct ai_rx_uart_head *rx_info = (struct ai_rx_uart_head *)buf;
    struct ai_tx_uart_info *tx_info = NULL;
    enum datatype type = 0;
    cJSON *root = NULL;
    char sbuf[128];
    if (!rx_info) {
        return -1;
    }
    if (rx_info->head == RECV_HEAD) {
        type = rx_info->err_type;
        printf("rx_info->err_type = %d\n", type);
    } else if (rx_info->head == SEND_HEAD) {
        tx_info = (struct ai_tx_uart_info *)buf;
        type = tx_info->datatype;
        struct key_event key = {0};
        key.type = KEY_EVENT_USER;
        key.action = KEY_EVENT_CLICK;
        printf("buf = %s\n", buf);
        switch (type) {
        // AI对话相关命令
        case AI_UART_CMD_DIALUOGE_START:    // 48：启动AI对话
            // 启动AI对话的具体实现
            printf("-->启动AI对话\n");
            key_vad_pcm_send_set_status(1, 1);
            break;
        case AI_UART_CMD_REC_START:    // 48：启动AI对话
            // 启动AI对话的具体实现
            printf("-->开始录音\n");
            key_vad_pcm_send_set_status(1, 0);
            break;
        case AI_UART_CMD_REC_END:           // 50：结束录音
            // 结束录音的具体实现
            printf("-->结束录音\n");
            key_vad_pcm_send_set_status(0, 1);
            break;
        case AI_UART_CMD_AI_DIALUOGE_STOP:  // 53：终止AI对话
            printf("-->终止AI对话\n");
            aisp_clear(0);
            break;

        // 系统命令 + 音乐命令
        case AI_UART_CMD_PWR_OFF:           // 80：关机
            // 关机的具体实现
            printf("-->执行关机操作\n");
            aisp_all_pause(1);
            music_play_waite();
            music_play_res_file("PwrOff.mp3");
            music_play_waite();
            sys_power_poweroff();
            break;
        case AI_UART_CMD_DIALUOGE_CLOSE:    // 81：对话关闭
            printf("-->关闭当前对话\n");
            aisp_clear(0);
            break;
        case AI_UART_CMD_MUSIC_START:       // 82：播放音乐
            printf("-->启动音乐播放\n");
            key.value = KEY_RESUM;
            key_event_notify(KEY_EVENT_FROM_USER, &key);
            break;
        case AI_UART_CMD_MUSIC_PUASE:       // 83：暂停播放
            printf("-->暂停音乐播放\n");
            key.value = KEY_SUPSPEND;
            key_event_notify(KEY_EVENT_FROM_USER, &key);
            break;
        case AI_UART_CMD_MUSIC_CONTINUE:    // 84：继续播放
            printf("-->继续音乐播放\n");
            key.value = KEY_RESUM;
            key_event_notify(KEY_EVENT_FROM_USER, &key);
            break;
        case AI_UART_CMD_MUSIC_STOP:        // 85：停止播放
            printf("-->停止音乐播放\n");
            key.value = KEY_SUPSPEND;
            key_event_notify(KEY_EVENT_FROM_USER, &key);
            break;
        case AI_UART_CMD_MUSIC_NEXT:        // 86：下一首
            printf("-->切换到下一首音乐\n");
            key.value = KEY_DOWN;
            key_event_notify(KEY_EVENT_FROM_USER, &key);
            break;
        case AI_UART_CMD_MUSIC_LAST:        // 87：上一首
            printf("-->切换到上一首音乐\n");
            key.value = KEY_UP;
            key_event_notify(KEY_EVENT_FROM_USER, &key);
            break;
        case AI_UART_CMD_MUSIC_LOOP:        // 88：单曲循环
            printf("-->开启单曲循环模式\n");
            net_music_play_loop();
            break;
        case AI_UART_CMD_VOLUME_DEC:        // 89：小声一点
            printf("-->降低音量\n");
            key.value = KEY_VOLUME_DEC;
            key_event_notify(KEY_EVENT_FROM_USER, &key);
            break;
        case AI_UART_CMD_VOLUME_INC:        // 90：大声一点
            printf("-->增大音量\n");
            key.value = KEY_VOLUME_INC;
            key_event_notify(KEY_EVENT_FROM_USER, &key);
            break;
        case AI_UART_CMD_VOLUME_SET:        // 91：设置音量
            printf("-->开始解析并设置音量\n");
            printf("buf = %s\n", tx_info->data);
            root = cJSON_Parse(tx_info->data);     // 解析JSON字符串
            if (root != NULL) {
                // 从根对象获取"volume"字段
                cJSON *volume_item = cJSON_GetObjectItem(root, "volume");

                if (volume_item && cJSON_IsNumber(volume_item)) {
                    int volume = volume_item->valueint; // 获取音量数值
                    printf("-->设置目标音量为：%d\n", volume);
                    // 使用音量值

                    struct application *app = get_current_app();

#if defined CONFIG_BT_ENABLE && defined CONFIG_BT_MUSIC_MODE_ENABLE
                    if (app && !strcmp(app->name, "bt_music")) {
                        bt_music_set_volume(volume);
                        break;
                    }
#endif
#ifdef CONFIG_SD_MUSIC_MODE_ENABLE
                    if (app && !strcmp(app->name, "sd_music")) {
                        sd_music_set_dec_volume(volume);
                        break;
                    }
#endif
#ifdef CONFIG_USB_DISK_MUSIC_MODE_ENABLE
                    if (app && !strcmp(app->name, "usbdisk_music")) {
                        usbdisk_music_set_dec_volume(volume);
                        break;
                    }
#endif
#ifdef CONFIG_ASR_ALGORITHM_ENABLE
                    if (app && !strcmp(app->name, "ai_speaker")) {
                        ai_speaker_volume_set(volume, 0);
                        break;
                    }
#endif
                } else {
                    // 处理字段不存在或类型错误的情况
                    printf("-->未找到有效的音量字段（voluem）\n");
                }
                cJSON_Delete(root); // 释放cJSON对象，避免内存泄漏
//            } else {
//                printf("-->音量设置JSON解析失败\n");
//                ai_uart_cmd_data_push(AI_UART_CMD_ERROR, NULL, 0);
            }
            break;

        // 播放源相关命令
        case AI_UART_CMD_UDISK_MODE:        // 108：播放U盘
            printf("-->切换到U盘播放模式\n");
            extern int audio_app_mode_switch(char *name);
            audio_app_mode_switch("usbdisk_music");
            break;
        case AI_UART_CMD_SD_TF_MODE:        // 109：播放SD卡/TF卡
            printf("-->切换到SD/TF卡播放模式\n");
            extern int audio_app_mode_switch(char *name);
            audio_app_mode_switch("sd_music");
            break;
        case AI_UART_CMD_BT_MODE:           // 110：播放蓝牙
            printf("-->切换到蓝牙播放模式\n");
            extern int audio_app_mode_switch(char *name);
            audio_app_mode_switch("bt_music");
            break;
        case AI_UART_CMD_AUX_MODE:          // 111：线路模式
            printf("-->切换到线路输入（AUX）模式\n");
            extern int audio_app_mode_switch(char *name);
            audio_app_mode_switch("aux_music");
            break;
        case AI_UART_CMD_AI_MODE:           // 112：AI模式
            printf("-->切换到AI音箱模式\n");
            extern int audio_app_mode_switch(char *name);
            audio_app_mode_switch("ai_speaker");
            break;

        // 蓝牙连接相关命令
        case AI_UART_CMD_DISCON_BT:         // 143：断开蓝牙
            printf("-->断开当前蓝牙连接\n");
            bt_connection_disable();
            break;
        case AI_UART_CMD_CONNECT_BT:        // 144：连接蓝牙
            printf("-->启动蓝牙连接\n");
            bt_connection_enable();
            break;

        case AI_UART_CMD_SET_WIFI_SSID:     //138：wifi连接路由器SSID信息
            printf("-->开始解析并连接WiFi\n");
            root = cJSON_Parse(tx_info->data);     // 解析JSON字符串
            printf("buf = %s\n", tx_info->data);
            if (root != NULL) {
                // 从根对象获取"ssid"和"pwd"字段
                cJSON *ssid_item = cJSON_GetObjectItem(root, "ssid");
                cJSON *pwd_item = cJSON_GetObjectItem(root, "pwd");
                if (ssid_item && cJSON_IsString(ssid_item) && pwd_item && cJSON_IsString(pwd_item)) {
                    printf("-->连接WiFi：SSID=%s，密码=******\n", ssid_item->valuestring);
                    wifi_sta_connect(ssid_item->valuestring, pwd_item->valuestring, 1);
                } else {
                    printf("-->WiFi连接参数不完整或格式错误\n");
                }
                cJSON_Delete(root); // 释放内存
            } else {
                printf("-->WiFi配置JSON解析失败\n");
            }
            break;
        case AI_UART_CMD_SET_BT_SSID:       //139：蓝牙设备名称信息
            printf("-->暂不支持设置蓝牙设备名称\n");
//            ai_uart_cmd_data_push(AI_UART_CMD_ERROR, NULL, 0);
            break;

        case AI_UART_CMD_GET_WIFI_SSID:;     //136：获取wifi已连接路由器SSID信息
            extern void wifi_sta_mode_info(char **ssid, char **pwd);
            char *ssid = NULL;
            char *pwd = NULL;
            wifi_sta_mode_info(&ssid, &pwd);
            if (ssid && ssid[0]) {
                if (pwd && pwd[0]) {
                    sprintf(sbuf, "{\"ssid\":\"%s\",\"pwd\":\"%s\"}", ssid, pwd);
                } else {
                    sprintf(sbuf, "{\"ssid\":\"%s\",\"pwd\":\"\"}", ssid);
                }
                printf("-->获取wifi已连接路由器SSID信息：\n");
                ai_uart_cmd_data_push(AI_UART_CMD_GET_NET_CONNET, sbuf, strlen(sbuf) + 1);//外文件使用的函数
            }
            printf("-->查询当前连接的WiFi SSID\n");
            break;
        case AI_UART_CMD_GET_BT_SSID:       //137：获取蓝牙设备名称信息
            printf("-->查询当前蓝牙设备名称\n");
            extern const char *bt_get_local_name(void);
            extern char *ble_get_name(void);
            sprintf(sbuf, "{\"bt_name\":\"%s\",\"ble_name\":\"\"}", bt_get_local_name(), ble_get_name());
            ai_uart_cmd_data_push(AI_UART_CMD_GET_BT_SSID, sbuf, strlen(sbuf) + 1);//外文件使用的函数
            break;
        case AI_UART_CMD_GET_VBAT:          //140：电量查询，带有数据则为电量值
            printf("-->查询当前设备电量\n");
#if TCFG_VBAT_CHECK_EN
            int sys_get_vbat_percent(void);
            int vbat_status = sys_get_vbat_percent();
            sprintf(sbuf, "{\"battery\":%d}", vbat_status);
            printf("-->电量查询结果：%d%%\n", vbat_status);
            ai_uart_cmd_data_push(AI_UART_CMD_GET_NET_CONNET, sbuf, strlen(sbuf) + 1);//外文件使用的函数
#endif
            break;
        case AI_UART_CMD_GET_NET_CONNET:    //141：查看wifi联网状态
            printf("-->查询WiFi联网状态\n");
            // 获取网络连接状态
            int net_status = sys_connect_net_success();
            sprintf(sbuf, "{\"net_status\":%d}", net_status);
            printf("-->WiFi联网状态：%s（状态码：%d）\n", net_status ? "已连接" : "未连接", net_status);
            ai_uart_cmd_data_push(AI_UART_CMD_GET_NET_CONNET, sbuf, strlen(sbuf) + 1);//外文件使用的函数
            break;
        case AI_UART_CMD_GET_BT_CONNET:     //142：查看蓝牙连接状态
            printf("-->查询蓝牙连接状态\n");
            char bt_connect_check(void);
            char bt_status = bt_connect_check();
            sprintf(sbuf, "{\"bt_status\":%d}", bt_status);
            printf("-->BT连接状态：%s（状态码：%d）\n", bt_status ? "已连接" : "未连接", bt_status);
            ai_uart_cmd_data_push(AI_UART_CMD_GET_NET_CONNET, sbuf, strlen(sbuf) + 1);//外文件使用的函数
            break;
        default:
//            ai_uart_cmd_data_push(AI_UART_CMD_NO_SUPPORT, NULL, 0);
            break;
        }

    }
    return 0;
}
static int ai_uart_buff_init(void)
{
    if (!lbuffer) {
        lbuffer = malloc(LBUF_MEM_BUF_SIZE);
    }
    if (!lbuf_hdl) {
        lbuf_hdl = lbuf_init(lbuffer, LBUF_MEM_BUF_SIZE, 4, sizeof(struct ai_tx_uart_info));
    }
}
int ai_uart_cmd_data_push(unsigned char data_type, char *buf, int len)//外文件使用的函数
{
    struct ai_tx_uart_info *tx_info = NULL;
    if (!lbuf_hdl) {
        return -1;
    }
    int size = lbuf_free_space(lbuf_hdl);
    if ((sizeof(struct ai_tx_uart_info) + len) < size) {
        tx_info = lbuf_alloc(lbuf_hdl, sizeof(struct ai_tx_uart_info) + len + 1);//多加1，在len = 0情况下也可以申请
        if (tx_info) {
            memset(tx_info, 0, sizeof(struct ai_tx_uart_info) + len + 1);
            tx_info->head = SEND_HEAD;
            tx_info->len = len;
            tx_info->datatype = data_type;
            if (buf && len) {
                memcpy(tx_info->data, buf, len);
            }
            tx_info->checksum = data_check_sum(&tx_info->datatype, tx_info->len + sizeof(struct ai_tx_uart_info) - (sizeof(tx_info->head) - sizeof(tx_info->checksum)));
            lbuf_push((void *)tx_info, BIT(0));
            os_sem_post(&msg_sem);
            //printf("-> txt : %s \n",buf);
        } else {
            printf("-> lbuf_alloc err\n");
        }
    } else {
        printf("-> ai_uart_cmd_data_push err no mem, %d %d\n", len, size);
    }
    return len;
}
static void uart_recv_task_main(void *priv)
{
    while (1) {
        /* 5 . 接收数据 */
        int len = dev_read(uart_hdl, recv_buf, sizeof(recv_buf));
        if (len <= 0) {
            //printf("\n  uart recv err len = %d\n", len);
            if (len == UART_CIRCULAR_BUFFER_WRITE_OVERLAY) {
                printf("\n UART_CIRCULAR_BUFFER_WRITE_OVERLAY err\n");
                dev_ioctl(uart_hdl, UART_FLUSH, 0); //如果由于用户长期不取走接收的数据导致循环buf接收回卷覆盖,因此直接冲掉循环buf所有数据重新接收
            } else if (len == UART_RECV_TIMEOUT) {
                //puts("UART_RECV_TIMEOUT...\r\n");
            }
            continue;
        }
        printf("\n uart recv len = %d\n", len);
        put_buf(recv_buf, len);
        ai_uart_data_recv(recv_buf, len);//接收到指令
    }
}
static void uart_task_main(void *priv)
{
    unsigned int cnt = 0;
    char name[32];
    os_sem_create(&msg_sem, 0);
    ai_uart_buff_init();

#ifdef AI_UART_CMD_CTROL_ENABLE
    sprintf(name, "uart%d", AI_UART_CMD_CTROL_ENABLE);
    uart_hdl = dev_open(name, NULL);
#else
    uart_hdl = dev_open("uart0", NULL);
#endif
    if (!uart_hdl) {
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

#ifdef AI_UART_CMD_TX_PORT
    gpio_set_pull_down(AI_UART_CMD_TX_PORT, 1);
    gpio_set_pull_up(AI_UART_CMD_TX_PORT, 1);
    gpio_direction_output(AI_UART_CMD_TX_PORT, 0);
    gpio_set_die(AI_UART_CMD_TX_PORT, 1);
#endif

    os_task_create(uart_recv_task_main, NULL, 10, 1000, 0, "at_uart_recv_task");
    while (1) {
        int err = os_sem_pend(&msg_sem, 500);
        if (!err) {
            ai_uart_data_send();
        }
    }
    dev_close(uart_hdl);
}
static int uart_task_init(void)
{
    if (production_test_io_get()) {
        return 0;
    }
    os_task_create(uart_task_main, NULL, 10, 1000, 0, "at_uart_cmd_task");
    return 0;
}
late_initcall(uart_task_init);

static const char *happy_str[] = {
    "😀", "😃", "😄", "😁", "😆", "🤣", "😂",
    "快乐", "欢喜", "开心", "欢乐", "喜", "笑哭",
    NULL,
};
static const char *smile_str[] = {
    "🙂", "🙃", "🫠", "🤗", "🤭",
    "微笑", "友好", "冷笑", "呵呵", "放松",
    NULL,
};
static const char *excited_str[] = {
    "🤩", "😁", "😉",
//    "兴奋","期待",
    NULL,
};
static const char *quite_str[] = {
    "🤫", "😶", "😯", "🙂", "😌", "😐", "😑", "😐", "🤐",
//    "静静","安静",
    NULL,
};
static const char *think_str[] = {
    "🤔", "😔", "🤔", "🧐", "🫤", "😕", "🤓", "😖",
//    "思考","专注",
    NULL,
};
static const char *sad_str[] = {
    "😢", "😭", "😣", "😢", "🥹", "😧", "🥺", "😵", "🥴", "😫", "😩",
    "悲伤", "难过", "伤心",
    NULL,
};
static const char *grievance_str[] = {
    "😢", "🥹", "😂", "😢", "🫥", "🥹", "😖", "☹️", "🙁", "😟", "😕", "😔", "😞",
    "委屈", "哭", "泪", "呜呜",
    NULL,
};
static const char *angry_str[] = {
    "😡", "🤬", "😤", "😡", "👿", "😈", "😒",
//    "生气","暴躁","发火","不爽","诅咒",
    NULL,
};
static const char *whiny_str[] = {
    "😑", "😖", "🙄", "😶‍🌫️", "😬",
//    "懊恼","烦躁",
    NULL,
};
static const char *surprise_str[] = {
    "😠", "😲", "😯", "😮", "😧",
//    "惊讶","诧异",
    NULL,
};
static const char *confused_str[] = {
    "😕", "🫤", "😟", "🤓", "🧐",
//    "疑惑","不解",
    NULL,
};
static const char *shy_str[] = {
    "😊",
//    "害羞","腼腆",
    NULL,
};
static const char *sleep_str[] = {
    "😪", "😴", "😫", "🥱", "😫", "😩",
//    "休息","睡觉","疲倦","困了","困倦",
    NULL,
};
static const char *enjoy_str[] = {
    "🤩", "🥰", "😍", "😎", "😘",
//    "陶醉","享受","可爱",
    NULL,
};
static const char *naughty_str[] = {
    "😜", "🤪", "😝", "😋", "😛", "😝", "🤨", "🤓", "🥵", "🤑",
//    "调皮","捣蛋",
    NULL,
};
static const char *fear_str[] = {
    "😅", "🫢", "🫣", "😧", "😦", "😯", "😧", "😱", "😨", "😰", "😥", "😖",
//    "恐惧","害怕","担心","紧张",
    NULL,
};
static const char *proud_str[] = {
    "🥲", "🥹", "😵",
//    "骄傲","自豪",
    NULL,
};
static const char *depressed_str[] = {
    "😞", "😦", "😥", "😣", "😟", "😧", "😨", "🙁",
//    "沮丧","失落",
    NULL,
};
static const char *desire_str[] = {
    "😍", "🥺",
//    "急切","渴望",
    NULL,
};
static const char *gentle_str[] = {
    "🥰", "😘", "😚", "😙", "😗", "😉", "😌", "🥳", "🤩",
//    "温柔","亲切",
    NULL,
};
static const char *curiosity_str[] = {
    "😵",
//    "好奇","探索",
    NULL,
};
static const char *cute_str[] = {
    "☺️",
//    "可爱",
    NULL,
};

int emoji_tts_uart_cmd_callback(char *emoji)
{
    // 定义表情符号映射表
    struct EmojiMap {
        const char **strings;
        char index;
    };

    static const struct EmojiMap emoji_maps[] = {
        {happy_str, AI_UART_CMD_EMOJI_HAPPY},
        {smile_str, AI_UART_CMD_EMOJI_SMILE},
        {excited_str, AI_UART_CMD_EMOJI_EXCITE},
        {quite_str, AI_UART_CMD_EMOJI_QUIET},
        {think_str, AI_UART_CMD_EMOJI_THINK},
        {sad_str, AI_UART_CMD_EMOJI_SAD},
        {grievance_str, AI_UART_CMD_EMOJI_GIEVAN},
        {angry_str, AI_UART_CMD_EMOJI_ANGRY},
        {whiny_str, AI_UART_CMD_EMOJI_FRET},
        {surprise_str, AI_UART_CMD_EMOJI_AMAZE},
        {confused_str, AI_UART_CMD_EMOJI_DOUBT},
        {shy_str, AI_UART_CMD_EMOJI_SHY},
        {sleep_str, AI_UART_CMD_EMOJI_SLEEP},
        {enjoy_str, AI_UART_CMD_EMOJI_REVEL},
        {naughty_str, AI_UART_CMD_EMOJI_NAUGHTY},
        {fear_str, AI_UART_CMD_EMOJI_FEAR},
        {proud_str, AI_UART_CMD_EMOJI_PROUD},
        {depressed_str, AI_UART_CMD_EMOJI_DEPPRESS,},
        {desire_str, AI_UART_CMD_EMOJI_DESIRE},
        {gentle_str, AI_UART_CMD_EMOJI_GENTLE},
        {curiosity_str, AI_UART_CMD_EMOJI_INQUIST},
        {cute_str, AI_UART_CMD_EMOJI_CUTE,},
        {NULL, 0} // 结束标记
    };

    int i = 0;
    int j = 0;
    char emoji_index = 0;

    // 遍历表情符号映射表
    for (i = 0; emoji_maps[i].strings != NULL; i++) {
        const char **str_list = emoji_maps[i].strings;
        // 遍历当前表情符号列表
        for (j = 0; str_list[j] != NULL; j++) {
            if (strstr(emoji, str_list[j])) {
                emoji_index = emoji_maps[i].index;
                goto exit;
            }
        }
    }

exit:
    if (emoji_index) {
        printf("-->emoji_index = %d\n", emoji_index);
        ai_uart_cmd_data_push(emoji_index, emoji, strlen(emoji) + 1);
    }
    return 0;
}
#endif
