#include "web_socket/websocket_api.h"
#include "wifi/wifi_connect.h"
#include "system/includes.h"
#include "os/os_api.h"
#include "wav_head.h"
#include "fs/fs.h"
#include "json_c/json.h"
#include "json_c/json_tokener.h"
#include "app_config.h"

#if (defined TCFG_LED_PWM0_PORT || defined TCFG_PWM1_PORT)
//=================底座灯指令=====================//
static const char *tm_light_open_str[] = {//开
    "开", "打开", //"切换","切换回","切换到","切换至","切换成","开启",
    NULL,
};
static const char *tm_light_close_str[] = {//关
    "关", "关闭", //"关掉","熄灭","灭掉","结束","取消",
    NULL,
};
static const char *tm_base_light_str[] = {//底座灯
    "灯", "灯光", //"底座灯光","系统灯光","夜灯","小夜灯","氛围灯","底座灯",
    NULL,
};
static const char *tm_earth_light_str[] = {//球体灯
    "球体灯", "球体灯光", //"球灯","球内灯光","球内灯","地球仪灯光","地球仪","地球仪灯","地球仪内部灯光",
    NULL,
};
static const char *tm_all_light_str[] = {//所有灯光
    "全部灯光", "所有灯光", //"全亮模式",
    NULL,
};
static const char *tm_breathing_light_str[] = {//呼吸灯
    "呼吸灯", "呼吸灯模式",
    NULL,
};
static const char *tm_base_light_pwm_decinc[] = {
    "暗一点", "调暗一点",
    "亮一点", "调亮一点",
    NULL,
};

enum {
    TM_LIGHT_INIT = 0,
    TM_BASE_LIGHT_OPEN,
    TM_BASE_LIGHT_CLOSE,

    TM_EARTH_LIGHT_OPEN,
    TM_EARTH_LIGHT_CLOSE,

    TM_ALL_LIGHT_OPEN,
    TM_ALL_LIGHT_CLOSE,

    TM_BREATHING_LIGHT_OPEN,
    TM_BREATHING_LIGHT_CLOSE,
};
struct tm_light {
    char base_light;
    char earth_light;
    char breathing;
} TM_LIGHT;

extern void tm_light_open(char notice);
extern void tm_light_close(char notice);
extern void tm_light_breath(char open, char notice);
extern void tm_light_earth_ctrol(char open, char notice);
extern int tm_light_pwm_decinc(int inc, int notice);

int tm_instruction_word_callback(char *word, char *asr)
{
    char status = 0;
    char src_str[64];
    char base_light = 0xFF;
    char earth_light = 0xFF;
    char breathing = 0xFF;
    int i, j;

    printf("-> tm_instruction_word_callback\n");
    for (i = 0; tm_base_light_pwm_decinc[i] != NULL; i++) { //暗一点、亮一点
        if (!strcmp(tm_base_light_pwm_decinc[i], word)) {
#ifdef TCFG_LED_PWM0_PORT
            tm_light_pwm_decinc(i >= 2 ? 1 : 0, 1);
#endif
            printf("-> tm_light_pwm_decinc: %s\n", word);
            return 1;
        }
    }

    for (i = 0; tm_light_open_str[i] != NULL; i++) { //开
        for (j = 0; tm_base_light_str[j] != NULL; j++) { //底座灯
            sprintf(src_str, "%s%s", tm_light_open_str[i], tm_base_light_str[j]);
            //printf("%s\n",src_str);
            if (!strcmp(src_str, word)) {
                status = TM_BASE_LIGHT_OPEN;
                printf("-> TM_BASE_LIGHT_OPEN: %s , %s\n", src_str, word);
                goto exit;
            }
        }
    }
    for (i = 0; tm_light_open_str[i] != NULL; i++) { //开
        for (j = 0; tm_earth_light_str[j] != NULL; j++) { //球体灯
            sprintf(src_str, "%s%s", tm_light_open_str[i], tm_earth_light_str[j]);
            //printf("%s\n",src_str);
            if (!strcmp(src_str, word)) {
                status = TM_EARTH_LIGHT_OPEN;
                printf("-> TM_EARTH_LIGHT_OPEN: %s , %s\n", src_str, word);
                goto exit;
            }
        }
    }
    for (i = 0; tm_light_close_str[i] != NULL; i++) { //关
        for (j = 0; tm_base_light_str[j] != NULL; j++) { //底座灯
            sprintf(src_str, "%s%s", tm_light_close_str[i], tm_base_light_str[j]);
            //printf("%s\n",src_str);
            if (!strcmp(src_str, word)) {
                status = TM_BASE_LIGHT_CLOSE;
                printf("-> TM_BASE_LIGHT_CLOSE: %s , %s\n", src_str, word);
                goto exit;
            }
        }
    }
    for (i = 0; tm_light_close_str[i] != NULL; i++) { //关
        for (j = 0; tm_earth_light_str[j] != NULL; j++) { //球体灯
            sprintf(src_str, "%s%s", tm_light_close_str[i], tm_earth_light_str[j]);
            //printf("%s\n",src_str);
            if (!strcmp(src_str, word)) {
                status = TM_EARTH_LIGHT_CLOSE;
                printf("-> TM_EARTH_LIGHT_CLOSE: %s , %s\n", src_str, word);
                goto exit;
            }
        }
    }
    for (i = 0; tm_light_open_str[i] != NULL; i++) { //开
        for (j = 0; tm_all_light_str[j] != NULL; j++) { //所有灯光
            sprintf(src_str, "%s%s", tm_light_open_str[i], tm_all_light_str[j]);
            //printf("%s\n",src_str);
            if (!strcmp(src_str, word)) {
                status = TM_ALL_LIGHT_OPEN;
                printf("-> TM_ALL_LIGHT_OPEN : %s , %s\n", src_str, word);
                goto exit;
            }
        }
    }
    for (i = 0; tm_light_close_str[i] != NULL; i++) { //关
        for (j = 0; tm_all_light_str[j] != NULL; j++) { //所有灯光
            sprintf(src_str, "%s%s", tm_light_close_str[i], tm_all_light_str[j]);
            //printf("%s\n",src_str);
            if (!strcmp(src_str, word)) {
                status = TM_ALL_LIGHT_CLOSE;
                printf("-> TM_ALL_LIGHT_CLOSE: %s , %s\n", src_str, word);
                goto exit;
            }
        }
    }
    for (i = 0; tm_light_open_str[i] != NULL; i++) { //开
        for (j = 0; tm_breathing_light_str[j] != NULL; j++) { //呼吸灯
            sprintf(src_str, "%s%s", tm_light_open_str[i], tm_breathing_light_str[j]);
            //printf("%s\n",src_str);
            if (!strcmp(src_str, word)) {
                status = TM_BREATHING_LIGHT_OPEN;
                printf("-> TM_BREATHING_LIGHT_OPEN: %s , %s\n", src_str, word);
                goto exit;
            }
        }
    }
    for (i = 0; tm_light_close_str[i] != NULL; i++) { //关
        for (j = 0; tm_breathing_light_str[j] != NULL; j++) { //呼吸灯
            sprintf(src_str, "%s%s", tm_light_close_str[i], tm_breathing_light_str[j]);
            //printf("%s\n",src_str);
            if (!strcmp(src_str, word)) {
                status = TM_BREATHING_LIGHT_CLOSE;
                printf("-> TM_BREATHING_LIGHT_CLOSE: %s , %s\n", src_str, word);
                goto exit;
            }
        }
    }

exit:
    switch (status) {
    case TM_BASE_LIGHT_OPEN:
        base_light = 1;
        break;
    case TM_EARTH_LIGHT_OPEN:
        earth_light = 1;
        break;
    case TM_ALL_LIGHT_OPEN:
        base_light = 1;
        earth_light = 1;
        break;
    case TM_BASE_LIGHT_CLOSE:
        base_light = 0;
        break;
    case TM_EARTH_LIGHT_CLOSE:
        earth_light = 0;
        break;
    case TM_ALL_LIGHT_CLOSE:
        base_light = 0;
        earth_light = 0;
        breathing = 0;
        break;
    case TM_BREATHING_LIGHT_OPEN:
        breathing = 1;
        break;
    case TM_BREATHING_LIGHT_CLOSE:
        breathing = 0;
        break;
    default:
        break;
    }
//    printf("base_light = %d , %d , %d \n",base_light,earth_light,breathing);
#if (defined TCFG_PWM1_PORT && TCFG_PWM1_PORT != -1)
    if (earth_light != (char)0xFF) { //球体灯光
        TM_LIGHT.earth_light = earth_light;
        tm_light_earth_ctrol(TM_LIGHT.earth_light, base_light != (char)0xFF ? 0 : 1);
    }
#endif
    if (base_light != (char)0xFF) { //底座灯光
        TM_LIGHT.base_light = base_light;
#ifdef TCFG_LED_PWM0_PORT
        if (TM_LIGHT.base_light) {
            tm_light_open(1);
        } else {
            tm_light_close(1);
        }
#endif
    } else {
        if (breathing != (char)0xFF) { //呼吸灯模式
            TM_LIGHT.breathing = breathing;
#ifdef TCFG_LED_PWM0_PORT
            tm_light_breath(TM_LIGHT.breathing, 1);
#endif
        }
    }
    return status ? 1 : 0;
}

#endif
