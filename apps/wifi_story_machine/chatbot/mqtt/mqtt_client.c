#include "mqtt/MQTTClient.h"
#include "system/includes.h"
#include "wifi/wifi_connect.h"
#include "app_config.h"
#include <time.h>
#include <sys/time.h>
#include "net_update.h"
#include "cJSON.h"
#include "mbedtls/md.h"

#ifdef CONFIG_MQTT_IOT_ENABLE

#define COMMAND_TIMEOUT_MS      10000   //命令超时时间
#define MQTT_TIMEOUT_MS         10000   //接收阻塞时间
#define MQTT_KEEPALIVE_TIME     60000   //心跳时间
#define SEND_BUF_SIZE           1024    //发送buf大小
#define READ_BUF_SIZE           1024    //接收buf大小


//======================MQTT平台后台参数设置============================//
//static char mqtt_ssl        = 0;                        //服务器是是否需要加密，0：不需要，1：需要
//static int  ser_port        = 1883;                      //服务器端口号：默认不加密为1883，加密为8883
static char mqtt_ssl        = 1;                        //服务器是是否需要加密，0：不需要，1：需要
static int  ser_port        = 8883;                      //服务器端口号：默认不加密为1883，加密为8883
static char mqtt_version    = 4;                        //服务器版本：3 -> 3.1, 4 -> 3.1.1
static char clientID[84]    = "clientID";           //服务器clientID
static char address[48]     = "服务器的域名";       //服务器域名
static char username[64]    = "登录的用户名";       //服务器登录用户名
static char password[33]    = "登录的用户密码";     //服务器登录用户密码


static Client client;
static char send_buf[SEND_BUF_SIZE];    //发送buf
static char read_buf[READ_BUF_SIZE];    //接收buf

//消息发布临时buf
static char sendbuf[1024];
static char payload_json[256];
static char mqtt_task_init = 0;

int mqtt_dev_info_flash_read(int *server_port,
                             char *client_ID, int client_ID_len,
                             char *address, int address_len,
                             char *username, int username_len,
                             char *password, int password_len,
                             char *product_key, int product_key_len,
                             char *device_name, int device_name_len,
                             char *device_secret, int device_secret_len);

/* ---------- 平台接口声明（请替换为你的实际接口） ---------- */
extern int app_tts_play(const char *text);         // TTS 合成并播报
extern int app_play_url_audio(const char *url);    // 播放指定 URL 的音频
extern int app_music_play(void);                   // 开始播放音乐
extern int app_music_stop(void);                   // 停止播放音乐
/* ---------------------------------------------------------- */

/* 处理 reply 类型消息（设备请求的回复） */
static void handle_reply_message(cJSON *data, int err_code)
{
    if (err_code != 200) {
        printf("[MQTT] reply err_code=%d\n", err_code);
        // TODO: 根据业务需要上报错误 / 重试 / 提示用户
        return;
    }
    printf("[MQTT] reply ok\n");
    // TODO: 若 data 中有业务字段（如设备控制结果），在此解析处理
    if (data) {
        // 例如：cJSON *result = cJSON_GetObjectItem(data, "result");
        // ...
    }
}

/* 处理 request 类型消息（服务器主动请求） */
static void handle_request_message(cJSON *data, int err_code)
{
    if (err_code != 200) {
        printf("[MQTT] request err_code=%d\n", err_code);
        return;
    }
    if (!data) {
        printf("[MQTT] request data is null\n");
        return;
    }

    /* ---- 1. TTS 合成文本 ---- */
    cJSON *tts = cJSON_GetObjectItem(data, "tts");
    if (cJSON_IsString(tts) && tts->valuestring) {
        printf("[MQTT] request TTS: %s\n", tts->valuestring);
        http_tts_request(tts->valuestring, strlen(tts->valuestring) + 1);
        return;
    }

    /* ---- 2. 播放 URL 音频 ---- */
    cJSON *url = cJSON_GetObjectItem(data, "URL");
    if (cJSON_IsString(url) && url->valuestring) {
        printf("[MQTT] request URL audio: %s\n", url->valuestring);
        http_pipe_music_play_request_start(url->valuestring);
        return;
    }

    /* ---- 3. 播放音乐 start/stop ---- */
    cJSON *play_music = cJSON_GetObjectItem(data, "play_music");
    if (cJSON_IsString(play_music) && play_music->valuestring) {
        printf("[MQTT] request play_music: %s\n", play_music->valuestring);
        if (strcmp(play_music->valuestring, "start") == 0) {
            net_music_play_next(net_music_play_type_get());
        } else if (strcmp(play_music->valuestring, "stop") == 0) {
            net_music_play_set_stop();
            net_music_num_clear();
        } else {
            printf("[MQTT] unknown play_music cmd: %s\n", play_music->valuestring);
        }
        return;
    }

    printf("[MQTT] request with unknown data fields\n");
}

/* MQTT 消息到达回调 */
static void messageArrived(MessageData *data)
{
    if (!data || !data->message || data->message->payloadlen <= 0) {
        printf("[MQTT] err invalid message\n");
        return;
    }

    char *topic = malloc(data->topicName->lenstring.len + 1);
    if (topic) {
        strncpy(topic, data->topicName->lenstring.data, data->topicName->lenstring.len);
        topic[data->topicName->lenstring.len] = '\0';
        printf("[MQTT] recv topic : %s\n", topic);
        free(topic);
    }

    /* 1. 取出 payload，复制到以 '\0' 结尾的缓冲区（cJSON 需要字符串） */
    int len = data->message->payloadlen;
    char *payload = (char *)malloc(len + 1);
    if (!payload) {
        printf("[MQTT] malloc failed\n");
        return;
    }
    memcpy(payload, data->message->payload, len);
    payload[len] = '\0';

    printf("[MQTT] recv data : %s\n", payload);

    /* 2. 解析 JSON */
    cJSON *root = cJSON_Parse(payload);
    free(payload);
    if (!root) {
        printf("[MQTT] json parse failed\n");
        return;
    }

    /* 3. 取 err_code 和 type */
    cJSON *err_item  = cJSON_GetObjectItem(root, "err_code");
    cJSON *type_item = cJSON_GetObjectItem(root, "type");
    cJSON *data_item = cJSON_GetObjectItem(root, "data");

    int err_code = cJSON_IsNumber(err_item) ? err_item->valueint : 0;
    const char *type = cJSON_IsString(type_item) ? type_item->valuestring : NULL;

    if (!type) {
        printf("[MQTT] missing type field\n");
        cJSON_Delete(root);
        return;
    }

    /* 4. 按 type 分派 */
    if (strcmp(type, "reply") == 0) {
        handle_reply_message(data_item, err_code);
    } else if (strcmp(type, "request") == 0) {
        handle_request_message(data_item, err_code);
    } else {
        printf("[MQTT] unknown type: %s\n", type);
    }

    /* 5. 释放 JSON 树 */
    cJSON_Delete(root);
}

int MQTT_Subscribe_Topic(char *SubscribeTopic, int len)
{
    //订阅主题
    int err = MQTTSubscribe(&client, SubscribeTopic, QOS1, messageArrived);
    if (err != 0) {
        printf("MQTTSubscribe fail, err : 0x%x\n", err);
        return err;
    }
    return 0;
}
int MQTT_Publish_Message(char *PublishTopic, char *msg)
{
    MQTTMessage message = {0};

    message.qos = QOS1;
    message.retained = 0;
    message.payload = msg;
    message.payloadlen = strlen(msg) + 1;

    //发布消息
    int err = MQTTPublish(&client, PublishTopic, &message);
    if (err != 0) {
        printf("MQTTPublish fail, err : 0x%x\n", err);
        return err;
    }
    return 0;
}
/*
 * 创建上报请求的 JSON 对象
 * 返回：cJSON 根对象指针（使用后需调用 cJSON_Delete 释放）
 */
cJSON *create_report_request_json(void)
{
    // 1. 创建根对象
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    // 2. type 字段：主动上报请求
    cJSON_AddStringToObject(root, "type", "request");

    // 3. 创建 data 子对象
    cJSON *data = cJSON_CreateObject();
    if (!data) {
        cJSON_Delete(root);
        return NULL;
    }
    cJSON_AddItemToObject(root, "data", data);

    // 4. 电池电量（0~100）
    int battery = sys_get_vbat_percent();
    if (battery < 0) {
        battery = 0;
    }
    cJSON_AddNumberToObject(data, "battery", battery);

    // 5. 充电状态：1 正在充电，0 未充电
    cJSON_AddNumberToObject(data, "charge", sys_vbat_is_charging() ? 1 : 0);

    // 6. WiFi 名称（未连接时用空字符串）
    const char *wifi_name = wifi_sta_get_ssid();   // 需替换为你平台对应的获取接口
    cJSON_AddStringToObject(data, "wifi_name", wifi_name ? wifi_name : "");

    // 7. WiFi 信号强度（RSSI，负数，单位 dBm）
    cJSON_AddNumberToObject(data, "wifi_rssi", wifi_get_rssi());

    // 8. 音乐播放状态：1 正在播放，0 暂未播放
    cJSON_AddNumberToObject(data, "music_status", net_music_play_stop_status() ? 0 : 1);
    return root;
}

/*
 * 上报状态到服务器
 * 返回：0 成功，-1 失败
 */
int MQTT_report_status_to_server(char *PublishTopic)
{
    cJSON *json = create_report_request_json();
    if (!json) {
        return -1;
    }

    char *str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    if (!str) {
        return -1;
    }

    printf("report json: %s\n", str);
    MQTT_Publish_Message(PublishTopic, str);
    cJSON_free(str);
    return 0;
}

static int mqtt_task(void)
{
    Network network;
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
    MQTTMessage message = {0};
    int err;
    int loop_cnt = 0;
    char retry_cnt = 10;
    static char subscribeTopic[256] = "$sys/产品名称/%s/thing/property/post";   //订阅主题
    static char publishTopic[256] = "$sys/产品名称/%s/thing/property/post/reply";   //订阅主题
    unsigned char mac[6];// = {0x79, 0x2C, 0xA8, 0x20, 0x7C, 0x0B};

    bt_ble_get_mac(&mac);//使用蓝牙mac

    snprintf(clientID, sizeof(clientID), "%02X%02X%02X%02X%02X%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
    snprintf(subscribeTopic, sizeof(subscribeTopic), "$sys/hr-health/%02X%02X%02X%02X%02X%02X/thing/property/post", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
    snprintf(publishTopic, sizeof(publishTopic), "$sys/hr-health/%02X%02X%02X%02X%02X%02X/thing/property/post/reply", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);

    mqtt_task_init = 1;
    while (!sys_connect_net_success()) {
        printf("waitting net ...");
        os_time_dly(100);
    }
    mqtt_task_init = 2;

    if (mqtt_dev_info_flash_read(&ser_port,
                                 clientID, sizeof(clientID),
                                 address, sizeof(address),
                                 username, sizeof(username),
                                 password, sizeof(password),
                                 NULL, 0,
                                 NULL, 0,
                                 NULL, 0) == 0) {
        printf("---> read flash mqtt info : %d, %s , %s , %s , %s\n", ser_port, clientID, address, username, password);
    }

_reconnect:
    memset(&network, 0, sizeof(Network));
    //初始化网络接口
    NewNetwork(&network);

    //初始化网络加密
    if (mqtt_ssl) {
        NetWorkSetTLS(&network);
    }

    SetNetworkRecvTimeout(&network, 2000);

    //初始化客户端
    MQTTClient(&client, &network, COMMAND_TIMEOUT_MS, send_buf, sizeof(send_buf), read_buf, sizeof(read_buf));

    printf("---> MQTT connet ...\n");
    //tcp层连接服务器
    err = ConnectNetwork(&network, address, ser_port);
    if (err != 0) {
        printf("ConnectNetwork err : %d\n", err);
        goto exit;
    }

    connectData.willFlag = 0;
    connectData.MQTTVersion = mqtt_version;                        //mqtt版本号
    connectData.clientID.cstring = clientID;                       //客户端id
    connectData.username.cstring = username;                       //连接时的用户名
    connectData.password.cstring = password;                       //连接时的密码
    connectData.keepAliveInterval = MQTT_KEEPALIVE_TIME / 1000;    //心跳时间
    connectData.cleansession = 1;                                  //是否使能服务器的cleansession，0:禁止, 1:使能

#if 0 //调试信息
    printf("clientID : %s\n", clientID);
    printf("address  : %s\n", address);
    printf("username : %s\n", username);
    printf("password : %s\n", password);
#endif

    //mqtt层连接,向服务器发送连接请求
    err = MQTTConnect(&client, &connectData);
    if (err != 0) {
        network.disconnect(&network);
        printf("MQTTConnect fail, err : %d\n", err);
        goto exit;
    }
    printf("MQTTConnect:%d", err);

    //订阅主题
    err = MQTTSubscribe(&client, subscribeTopic, QOS1, messageArrived);
    if (err != 0) {
        MQTTDisconnect(&client);
        network.disconnect(&network);
        printf("MQTTSubscribe fail, err : 0x%x\n", err);
        goto exit;
    }
    printf("---> MQTT subscribeTopic : %s\n", subscribeTopic);

    //订阅主题
    err = MQTTSubscribe(&client, publishTopic, QOS1, messageArrived);
    if (err != 0) {
        MQTTDisconnect(&client);
        network.disconnect(&network);
        printf("MQTTSubscribe fail, err : 0x%x\n", err);
        goto exit;
    }
    printf("---> MQTT publishTopic : %s\n", publishTopic);

    /*
    //订阅设置回复主题
    err = MQTTSubscribe(&client, subscribeTopic_reply, QOS1, messageArrived);
    if (err != 0) {
        MQTTDisconnect(&client);
        network.disconnect(&network);
        printf("MQTTSubscribe fail, err : 0x%x\n", err);
        goto exit;
    }
    */
    //取消主题订阅
    //MQTTUnsubscribe(&client, subscribeTopic);

    while (sys_connect_net_success()) {
        if (0 == loop_cnt % 6) {
            MQTT_report_status_to_server(subscribeTopic);
        }

        loop_cnt++;

        err = MQTTYield(&client, MQTT_TIMEOUT_MS);
        if (err != 0) {
            if (client.isconnected) {
                //断开mqtt层连接
                err = MQTTDisconnect(&client);
                if (err != 0) {
                    printf("MQTTDisconnect fail\n");
                }

                //断开tcp层连接
                network.disconnect(&network);
            }

            printf("MQTT : Reconnecting\n");

            //重新连接
            if (--retry_cnt) {
                goto _reconnect;
            }
        }

    }
    if (client.isconnected) {
        MQTTDisconnect(&client);
    }

exit:
    mqtt_task_init = 0;
    return 0;
}

void mqtt_example(void)
{
    printf("mqtt_example ...\n");
    if (!mqtt_task_init) {
        if (thread_fork("mqtt_task", 10, 2 * 1024, 0, NULL, mqtt_task, NULL) != OS_NO_ERR) {
            printf("thread fork fail\n");
        }
    }
}
//late_initcall(mqtt_example);
#endif

