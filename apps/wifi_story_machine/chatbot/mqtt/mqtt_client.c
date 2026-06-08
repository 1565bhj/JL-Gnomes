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
static char mqtt_ssl        = 0;                    //服务器是是否需要加密，0：不需要，1：需要
static char mqtt_version    = 3;                    //服务器版本：3 -> 3.1, 4 -> 3.1.1
static int ser_port         = 1883;                 //服务器端口号：默认不加密为1883，加密为8883
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
//接收回调，当订阅的主题有信息下发时，在这里接收
static void messageArrived(MessageData *data)
{
    printf("Message arrived on topic (len : %d, topic : %s)\n", data->topicName->lenstring.len, data->topicName->lenstring.data);
//    char temp[256] = {0};
//    strncpy(temp, data->topicName->lenstring.data, data->topicName->lenstring.len);
//    temp[data->topicName->lenstring.len] = '\0';
//    printf("Message arrived on topic (len : %d, topic : %s)\n", data->topicName->lenstring.len, temp);
//
//    memset(temp, 0, sizeof(temp));
//    strncpy(temp, data->message->payload, data->message->payloadlen);
//    temp[data->message->payloadlen] = '\0';
//    printf("message (len : %d, payload : %s)\n", data->message->payloadlen, temp);

    /*
        cJSON *root = cJSON_Parse(temp);
        if (root != NULL) {
            cJSON *params_item = cJSON_GetObjectItem(root, "params");
            if (params_item) {
                cJSON *reset_item = cJSON_GetObjectItem(params_item, "reset");//恢复出厂
                if (reset_item) {
                    if ((cJSON_IsBool(reset_item) && cJSON_IsTrue(reset_item)) ||
                        (cJSON_IsNumber(reset_item) && reset_item->valueint)) {
                        //todo
                    }
                }
            }
            cJSON_Delete(root);
        }
    */
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

static int mqtt_task(void)
{
    Network network;
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
    MQTTMessage message = {0};
    int err;
    int loop_cnt = 0;
    char *subscribeTopic = "aiGateway/read/";   //订阅主题
    char *publishTopic = "aiGateway/send/";     //发布消息的主题

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
//    err = MQTTSubscribe(&client, publishTopic, QOS1, messageArrived);
    if (err != 0) {
        MQTTDisconnect(&client);
        network.disconnect(&network);
        printf("MQTTSubscribe fail, err : 0x%x\n", err);
        goto exit;
    }
    printf("---> MQTT subscribeTopic : %s\n", subscribeTopic);
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

    //填充要发送的json数据
    const char *json_str = "{\"massageCode\":2205,\"mac\":\"1962934412\",\"data\":{\"header\":{\"namespace\":\"ai.ConnectedHome.Discovery\",\"name\":\"DiscoverAppliancesRequest\",\"messageId\":\"1\",\"payloadVersion\":\"1\"},\"payload\":{\"accessToken\":\"efwefw\"}}}";
    strncpy(payload_json, json_str, sizeof(payload_json) - 1);
    payload_json[sizeof(payload_json) - 1] = '\0';

    while (sys_connect_net_success()) {
        if (0 == loop_cnt % 2) {
            message.qos = QOS1;
            message.retained = 0;
            message.payload = sendbuf;
            message.payloadlen = strlen(sendbuf) + 1;
            //发布消息
            err = MQTTPublish(&client, publishTopic, &message);
            if (err != 0) {
                printf("MQTTPublish fail, err : 0x%x\n", err);
            }

            printf("MQTTPublish payload:(%s)\n", sendbuf);
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
            goto _reconnect;
        }

    }
    MQTTDisconnect(&client);

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

