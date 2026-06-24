#include "system/includes.h"
#include "server/audio_server.h"
#include "server/server_core.h"
#include "app_config.h"
#include "qyai_config.h"
#include "syscfg/syscfg_id.h"

//服务器地址选择：0-正式地址 1-测试地址 2-灰度地址
int qyai_chat_addr_get(void)
{
    return SADDR_CHAT;
}

int qyai_chat_addr_version(void)
{
#ifdef CONFIG_KWS_ENGLISH
    return 1;
#else
    return 0;
#endif
}

char *qyai_chat_audio_enc_format(void)//AI对话音频格式
{
    return "pcm";//音频编码
//    return "opus";//音频编码
}

char *qyai_mic_audio_enc_format(void)//麦输出音频格式，唤醒词需要PCM做算法
{
    return "pcm";//音频编码
}

char *qyai_chat_audio_dec_format(void)
{
    return "mp3";//音频解码
//    return "ogg";//音频解码
}

char *qyai_chat_language(void)
{
#ifdef CONFIG_KWS_ENGLISH
    return "en";
#else
    return "zh";
#endif
}

int qyai_url_debug(void)
{
    return 0;
}


//网络异常信息回调
extern int http_ai_dev_err_info_report_task(char *reason);
int qyai_net_socket_err_callback(char *msg, int err_code)
{
    printf("\n%s, err_code %d\n", msg, err_code);
    if (strstr(msg, "exception")) { //没有收到音频数据数据异常断开
        music_play_stop_all();
        music_play_res_file("ServErrExce.mp3");
    } else if (strstr(msg, "send")) { //发送数据阶段断开
        music_play_stop_all();
        music_play_res_file("ServErrSend.mp3");
    } else if (strstr(msg, "err no recv complete index")) { //没收到结束包
    } else {//连接阶段异常
    }
#ifndef CONFIG_NO_SDRAM_ENABLE
    http_ai_dev_err_info_report_task(msg);
#endif
    return 0;
}

//开启UI和开外挂WiFi，则降低音频缓存
#if (defined CONFIG_UI_ENABLE || defined CONFIG_EXT_WIFI_ENABLE)
int ai_audio_play_buf_size_set(void)
{
    return 640 * 1024;
}
#endif
