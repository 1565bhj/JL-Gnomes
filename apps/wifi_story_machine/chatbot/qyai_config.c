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
#ifdef CONFIG_INSTRUCTION_CN
    return "zh";    // 中文
#elif defined(CONFIG_INSTRUCTION_YUE)
    return "yue";   // 粤语
#elif defined(CONFIG_INSTRUCTION_JA)
    return "ja";    // 日语
#elif defined(CONFIG_INSTRUCTION_KO)
    return "ko";    // 韩语
#elif defined(CONFIG_INSTRUCTION_TH)
    return "th";    // 泰语
#elif defined(CONFIG_INSTRUCTION_VI)
    return "vi";    // 越南语
#elif defined(CONFIG_INSTRUCTION_ID)
    return "id";    // 印尼语
#elif defined(CONFIG_INSTRUCTION_MS)
    return "ms";    // 马来语
#elif defined(CONFIG_INSTRUCTION_MY)
    return "my";    // 缅甸语
#elif defined(CONFIG_INSTRUCTION_KM)
    return "km";    // 高棉语（柬埔寨）
#elif defined(CONFIG_INSTRUCTION_LO)
    return "lo";    // 老挝语
#elif defined(CONFIG_INSTRUCTION_HI)
    return "hi";    // 印地语
#elif defined(CONFIG_INSTRUCTION_BN)
    return "bn";    // 孟加拉语
#elif defined(CONFIG_INSTRUCTION_TA)
    return "ta";    // 泰米尔语
#elif defined(CONFIG_INSTRUCTION_TE)
    return "te";    // 泰卢固语
#elif defined(CONFIG_INSTRUCTION_GU)
    return "gu";    // 古吉拉特语
#elif defined(CONFIG_INSTRUCTION_KN)
    return "kn";    // 卡纳达语
#elif defined(CONFIG_INSTRUCTION_ML)
    return "ml";    // 马拉雅拉姆语
#elif defined(CONFIG_INSTRUCTION_UR)
    return "ur";    // 乌尔都语
#elif defined(CONFIG_INSTRUCTION_PA)
    return "pa";    // 旁遮普语
#elif defined(CONFIG_INSTRUCTION_EN)
    return "en";    // 英语
#elif defined(CONFIG_INSTRUCTION_DE)
    return "de";    // 德语
#elif defined(CONFIG_INSTRUCTION_FR)
    return "fr";    // 法语
#elif defined(CONFIG_INSTRUCTION_ES)
    return "es";    // 西班牙语
#elif defined(CONFIG_INSTRUCTION_RU)
    return "ru";    // 俄语
#elif defined(CONFIG_INSTRUCTION_PT)
    return "pt";    // 葡萄牙语
#elif defined(CONFIG_INSTRUCTION_NL)
    return "nl";    // 荷兰语
#elif defined(CONFIG_INSTRUCTION_PL)
    return "pl";    // 波兰语
#elif defined(CONFIG_INSTRUCTION_UK)
    return "uk";    // 乌克兰语
#elif defined(CONFIG_INSTRUCTION_LT)
    return "lt";    // 意大利语
#elif defined(CONFIG_INSTRUCTION_AR)
    return "ar";    // 阿拉伯语
#elif defined(CONFIG_INSTRUCTION_HE)
    return "he";    // 希伯来语
#elif defined(CONFIG_INSTRUCTION_FA)
    return "fa";    // 波斯语
#elif defined(CONFIG_INSTRUCTION_TR)
    return "tr";    // 土耳其语
#else
    return "zh";    // 默认返回中文
#endif
}

int qyai_url_debug(void)
{
    return 0;
}


//网络异常信息回调
int http_ai_dev_err_info_report_task(const char *reason, const char *cur_trace_id);
int qyai_net_socket_err_callback(char *msg, int err_code, const char *trace_id)
{
    //printf("msg:%s, err:%d, trace_id:%s\n", msg ? msg : "(null)", err_code, trace_id ? trace_id : "(null)");
    if (trace_id == NULL) {
        return -1;
    }
    /*
        if (strstr(msg, "exception")) { //没有收到音频数据数据异常断开
            music_play_stop_all();
            music_play_res_file("ServErrExce.mp3");
        } else if (strstr(msg, "send")) { //发送数据阶段断开
            music_play_stop_all();
            music_play_res_file("ServErrSend.mp3");
        } else if (strstr(msg, "err no recv complete index")) { //没收到结束包
            ;
        } else {//连接阶段异常
            ;
        }
    */

#ifndef CONFIG_NO_SDRAM_ENABLE
    size_t len = strlen(trace_id);
    //局部兜住，全局变，局部不变
    char *trace_id_bak = malloc(len + 1);
    if (trace_id_bak == NULL) {
        return -1;
    }
    memcpy(trace_id_bak, trace_id, len + 1);
    http_ai_dev_err_info_report_task(msg, trace_id_bak);
    free(trace_id_bak);
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
