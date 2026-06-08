#include "system/includes.h"
#include "server/audio_server.h"
#include "server/server_core.h"
#include "app_config.h"
#include "qyai_config.h"
#include "syscfg/syscfg_id.h"

//服务器地址选择：0-正式地址 1-测试地址 2-灰度地址
int qyai_chat_addr_get(void)
{
    return SADDR_TEST;
}

char *qyai_chat_audio_enc_format(void)
{
    return "pcm";//音频编码
//    return "opus";//音频编码
}

char *qyai_chat_audio_dec_format(void)
{
    return "mp3";//音频解码
//    return "ogg";//音频解码
}

//网络异常信息回调
int qyai_net_socket_err_callback(char *msg, int err_code)
{
    printf("%s, err_code %d\n", msg, err_code);
    if (strstr(msg, "exception")) { //没有收到音频数据数据异常断开
        ;
    } else if (strstr(msg, "send")) { //发送数据阶段断开
        ;
    } else {//连接阶段异常
        ;
    }
    return 0;
}

