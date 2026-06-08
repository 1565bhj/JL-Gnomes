#include "web_socket/websocket_api.h"
#include "wifi/wifi_connect.h"
#include "system/includes.h"
#include "os/os_api.h"
#include "wav_head.h"
#include "fs/fs.h"
#include "json_c/json.h"
#include "json_c/json_tokener.h"
#include "app_config.h"

#ifdef IRARC_UART_ENABLE
//=================空调指令=====================//
static const char *air_conditioner_openclose_str[] = {
    "打开空调",//0
    "关闭空调",//1
    NULL,
};
static const char *air_conditioner_mode_str[] = {
    "制冷模式",//0
    "制热模式",//1
    "除湿模式",//2
    "送风模式",//3
    "节能模式",//4
    "睡眠模式",//5
    "强力模式",//6
    NULL,
};
static const char *air_conditioner_maxmin_wind_str[] = {
    "最大风量",
    "最小风量",
};
static const char *air_conditioner_incdec_temp_str[] = {
    "升高温度",
    "降低温度",
    "设置温度",
    NULL,
};
static const char *air_conditioner_lr_swing_str[] = {
    "左右摆动",
    "上下摆动",
    NULL,
};
static const char *air_conditioner_wdir_str[] = {
    "自动风向",
    "手动风向",
    NULL,
};
static const char *air_conditioner_timing_str[] = {
    "定时打开",
    "定时关闭",
    "关闭定时",
    NULL,
};
#define AIR_CONDIT_CFG   "空调学习模式"
#define UIR_CONDIT_CFG   "家电学习模式"
#define EXIT_CONDIT_CFG  "退出学习模式"

int air_conditioner_instruction_word_callback(char *word, char *asr)
{
    char status = 0;
    char play = 0;
    char src_str[64];
    int i, j;

    printf("-> air_conditioner_word_callback \n");
    if (!strcmp(AIR_CONDIT_CFG, word)) { //配置空调
        irarc_arc_lrc_learning(1, 1);
        status = 1;
        goto exit;
    }
    if (!strcmp(UIR_CONDIT_CFG, word)) { //配置家电
        irarc_urc_lrc_learning(1, 1);
        status = 1;
        goto exit;
    }
    if (!strcmp(EXIT_CONDIT_CFG, word)) { //退出学习模式
        irarc_aurc_exit(1);
        status = 1;
        goto exit;
    }
    for (i = 0; air_conditioner_openclose_str[i] != NULL; i++) { //
        if (!strcmp(air_conditioner_openclose_str[i], word)) {
            printf("-> word %s , %s \n", word, air_conditioner_openclose_str[i]);
            play = status = 1;
            switch (i) {
            case 0:
                play = irarc_open();
                break;//"打开空调",//0
            case 1:
                irarc_close();
                break;//"关闭空调",//1
            }
            goto exit;
        }
    }
    for (i = 0; air_conditioner_mode_str[i] != NULL; i++) { //
        if (strstr(air_conditioner_mode_str[i], word)) {
            status = 1;
            switch (i) {
            case 0:
                play = irarc_cold();
                break;//"制冷模式",//0
            case 1:
                play = irarc_heating();
                break;//"制热模式",//1
            case 2:
                play = irarc_dehumidification();
                break;//"除湿模式",//2
            case 3:
                play = irarc_air_suplly();
                break;//"送风模式",//3
            case 4:
                play = irarc_energy_saving();
                break;//"节能模式",//4
            case 5:
                play = irarc_sleep();
                break;//"睡眠模式",//5
            case 6:
                play = irarc_powerful();
                break;//"强力模式",//6
            }
            goto exit;
        }
    }
    for (i = 0; air_conditioner_maxmin_wind_str[i] != NULL; i++) { //
        if (!strcmp(air_conditioner_maxmin_wind_str[i], word)) {
            status = 1;
            switch (i) {
            case 0:
                play = irarc_max_wind();
                break;//"最大风量"
            case 1:
                play = irarc_min_wind();
                break;//"最小风量"
            }
            goto exit;
        }
    }
    for (i = 0; air_conditioner_incdec_temp_str[i] != NULL; i++) { //
        if (!strcmp(air_conditioner_incdec_temp_str[i], word)) {
            status = 1;
            switch (i) {
            case 0:
                play = irarc_max_wind();
                break;//"升高温度",
            case 1:
                play = irarc_min_wind();
                break;//"降低温度",
            case 2:
                play = irarc_temperature_set(25);
                break;//"设置温度",
            }
            goto exit;
        }
    }
    for (i = 0; air_conditioner_lr_swing_str[i] != NULL; i++) { //
        if (!strcmp(air_conditioner_lr_swing_str[i], word)) {
            status = 1;
            switch (i) {
            case 0:
                play = irarc_swing_left_and_right();
                break;//"左右摆动",
            case 1:
                play = irarc_swing_up_and_down();
                break;//"上下摆动",
            }
            goto exit;
        }
    }
    for (i = 0; air_conditioner_wdir_str[i] != NULL; i++) { //
        if (!strcmp(air_conditioner_wdir_str[i], word)) {
            status = 1;
            switch (i) {
            case 0:
                play = irarc_swing_auto();
                break;//"自动风向",
            case 1:
                play = irarc_swing_manual();
                break;//"手动风向",
            }
            goto exit;
        }
    }
    for (i = 0; air_conditioner_timing_str[i] != NULL; i++) { //
        if (!strcmp(air_conditioner_timing_str[i], word)) {
            play = status = 1;
            switch (i) {
            case 0:
                irarc_timing_open(2);
                break;//"定时打开",
            case 1:
                irarc_timing_close(2);
                break;//"定时关闭",
            case 2:
                irarc_timing_exit();
                break;//"关闭定时",
            }
            goto exit;
        }
    }
exit:
    if (status && play) {
#ifdef SYS_COMMAND_ANSER_PLAY_ENABLE
        music_play_anser_OK();//好的
#endif
    }
    return status;
}

#endif
