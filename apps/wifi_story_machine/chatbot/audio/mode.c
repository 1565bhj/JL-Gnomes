#include "init.h"
#include "asm/system_reset_reason.h"
#include "server/audio_server.h"
#include "server/server_core.h"
#include "app_config.h"
#include "action.h"
#include "system/app_core.h"
#include "event/key_event.h"
#include "fs/fs.h"
#include "volume.h"
#include "asm/adc_api.h"
#include "system/timer.h"
#include "asm/efuse.h"
#include "asm/p33.h"
#include "asm/power_interface.h"
#include "asm/includes.h"
#include "ai_uart_ctrol.h"

#define MODE_PLAY_FILE_ENBLE    1   //播放模式提示音

struct audio_app_t {
    const char *tone_file_name;
    const char *app_name;
};

static const struct audio_app_t audio_app_table[] = {
#ifdef CONFIG_ASR_ALGORITHM_ENABLE
    {"AiMode.mp3", "ai_speaker"},
#endif
#if defined CONFIG_BT_ENABLE && defined CONFIG_BT_MUSIC_MODE_ENABLE
    {"BluteMode.mp3", "bt_music"},
#endif
#ifdef CONFIG_USB_DISK_MUSIC_MODE_ENABLE
    {"UsbDiskMode.mp3", "usbdisk_music"},
#endif
#ifdef CONFIG_SD_MUSIC_MODE_ENABLE
    {"SdMode.mp3", "sd_music"},
#endif
#ifdef CONFIG_AUX_MUSIC_MODE_ENABLE
    {"AuxMode.mp3", "aux_music"  },
#endif
};

static u8 mode_index = 0;

extern int music_play_res_file(const char *name);
extern int music_play_waite(void);

static void *mode_eq_hdl = NULL;

#if TCFG_EQ_ENABLE && !defined EQ_CORE_V1
void *get_eq_hdl(void)
{
    return mode_eq_hdl;
}
void app_eq_init(void)
{
    mode_eq_hdl = eq_open(2);
}
void app_eq_exit(void)
{
    if (mode_eq_hdl) {
        eq_close(mode_eq_hdl);
        mode_eq_hdl = NULL;
    }
}
#endif

int ai_speaker_app(void)
{
    struct intent it;
    struct application *app = get_current_app();
    if (app && !strcmp(app->name, audio_app_table[0].app_name)) {//AI模式
        return 1;
    }
    return 0;
}

int net_url_music_play(char *name, char *song_name, char *singer_name, void *type)
{
    struct intent it;
    struct application *app = get_current_app();
    if (app && strcmp(app->name, audio_app_table[0].app_name)) {//非AI模式
        init_intent(&it);
        it.name = app->name;
        it.action = ACTION_STOP;
        start_app(&it);
    }
    init_intent(&it);//切回AI智能体模式
    mode_index = 0;
    it.name = audio_app_table[mode_index++].app_name;
    it.action = ACTION_MUSIC_PLAY_MAIN;
    start_app(&it);

    net_music_name_save(name);

#ifdef  USED_TM1629_SHOWN
#if defined CONFIG_BT_ENABLE && defined CONFIG_BT_MUSIC_MODE_ENABLE
    void tm_1629_shown_bt(char on);
    if (!strcmp(it.name, "bt_music")) {
        tm_1629_shown_bt(1);
    } else {
        tm_1629_shown_bt(0);
    }
#endif
#ifdef CONFIG_SD_MUSIC_MODE_ENABLE
    void tm_1629_shown_tf(char on);
    if (!strcmp(it.name, "sd_music")) {
        tm_1629_shown_tf(1);
    } else {
        tm_1629_shown_tf(0);
    }
#endif
#ifdef CONFIG_ASR_ALGORITHM_ENABLE
    void tm_1629_shown_ai(char on);
    if (!strcmp(it.name, "ai_speaker")) {
        tm_1629_shown_ai(1);
    } else {
        tm_1629_shown_ai(0);
    }
#endif
#endif

#ifdef CONFIG_UI_PLAY_EMOJI
    extern int play_face_emoji(int index);
    play_face_emoji(AI_UART_CMD_MUSIC_START);
#endif
    return 0;
}

int audio_app_mode_check(void)
{
    char play = 0;
    struct intent it;
    struct application *app = get_current_app();
    if (app) {
        if (!strcmp(app->name, "ai_speaker")) {
            return 0;
        } else if (!strcmp(app->name, "bt_music")) {
            return 1;
        } else if (!strcmp(app->name, "sd_music")) {
            return 2;
        } else if (!strcmp(app->name, "usbdisk_music")) {
            return 3;
        } else if (!strcmp(app->name, "aux_music")) {
            return 4;
        }
    }
    return -1;
}

int audio_app_mode_num_switch(char index, char note)
{
    char play = 0;
    struct intent it;
    struct application *app = get_current_app();

    if (app) {
        if (!strcmp(app->name, audio_app_table[index].app_name)) {
            return 0;
        }
        init_intent(&it);
        it.name = app->name;
        it.action = ACTION_STOP;    //退出当前模式
        start_app(&it);
    }
    extern void net_music_play_set_stop(void);
    net_music_play_set_stop();//关闭网络播放
    mode_index = index;
    init_intent(&it);

    it.name = audio_app_table[mode_index].app_name;
    it.action = ACTION_MUSIC_PLAY_MAIN;
    start_app(&it);
    if (note) {
        music_play_stop(NULL);
        music_play_res_file(audio_app_table[mode_index].tone_file_name);
        play = true;
    }
    //等提示音播完了再切换模式
    init_intent(&it);
    it.name = audio_app_table[mode_index].app_name;
    it.action = ACTION_HOME_MAIN;
    printf("-> app : %s\n", audio_app_table[mode_index].app_name);


#ifdef  USED_TM1629_SHOWN
#if defined CONFIG_BT_ENABLE && defined CONFIG_BT_MUSIC_MODE_ENABLE
    void tm_1629_shown_bt(char on);
    if (!strcmp(it.name, "bt_music")) {
        tm_1629_shown_bt(1);
    } else {
        tm_1629_shown_bt(0);
    }
#endif
#ifdef CONFIG_SD_MUSIC_MODE_ENABLE
    void tm_1629_shown_tf(char on);
    if (!strcmp(it.name, "sd_music")) {
        tm_1629_shown_tf(1);
    } else {
        tm_1629_shown_tf(0);
    }
#endif
#ifdef CONFIG_ASR_ALGORITHM_ENABLE
    void tm_1629_shown_ai(char on);
    if (!strcmp(it.name, "ai_speaker")) {
        tm_1629_shown_ai(1);
    } else {
        tm_1629_shown_ai(0);
    }
#endif
#endif

#ifdef CONFIG_UI_PLAY_EMOJI
    extern int play_face_emoji(int index);
    if (!strcmp(it.name, "ai_speaker")) {
        play_face_emoji(AI_UART_CMD_AI_MODE);
    } else if (!strcmp(it.name, "bt_music")) {
        play_face_emoji(AI_UART_CMD_BT_MODE);
    } else if (!strcmp(it.name, "sd_music")) {
        play_face_emoji(AI_UART_CMD_SD_TF_MODE);
    } else if (!strcmp(it.name, "usbdisk_music")) {
        play_face_emoji(AI_UART_CMD_UDISK_MODE);
    } else if (!strcmp(it.name, "aux_music")) {
        play_face_emoji(AI_UART_CMD_AUX_MODE);
    }
#endif

    if (play) {
        music_play_waite();//等提示音播完了再切换模式
    }

    start_app(&it);
    mode_index++;
    return 0;
}
int audio_app_mode_switch_set(char *name, char note)
{
    if (name) {
        for (char i = 0; i < ARRAY_SIZE(audio_app_table); i++) {
            if (!strcmp(name, audio_app_table[i].app_name)) {
                return audio_app_mode_num_switch(i, note);
            }
        }

    }
    return 0;
}
int audio_app_mode_switch(char *name)
{
    char play = 0;
    struct intent it;
    if (mode_index >= ARRAY_SIZE(audio_app_table)) {
        mode_index = 0;
    }
    if (name) {
        for (char i = 0; i < ARRAY_SIZE(audio_app_table); i++) {
            if (!strcmp(name, audio_app_table[i].app_name)) {
                mode_index = i;
                goto __find;
            }
        }
        return -1;
    }
__find:
    if (get_current_app()) {
        init_intent(&it);
        it.name = audio_app_table[mode_index].app_name;
        it.action = ACTION_STOP;    //退出当前模式
        start_app(&it);
    }
    init_intent(&it);

    it.name = audio_app_table[mode_index].app_name;
    it.action = ACTION_MUSIC_PLAY_MAIN;
    start_app(&it);
    if (system_reset_reason_get() != SYS_RST_ALM_WKUP) {
        music_play_stop(NULL);
        music_play_res_file(audio_app_table[mode_index].tone_file_name);
        play = true;
    }
    //等提示音播完了再切换模式
    init_intent(&it);
    it.name = audio_app_table[mode_index].app_name;
    it.action = ACTION_HOME_MAIN;
    printf("-> app : %s\n", audio_app_table[mode_index].app_name);

#ifdef  USED_TM1629_SHOWN
#if defined CONFIG_BT_ENABLE && defined CONFIG_BT_MUSIC_MODE_ENABLE
    void tm_1629_shown_bt(char on);
    if (!strcmp(it.name, "bt_music")) {
        tm_1629_shown_bt(1);
    } else {
        tm_1629_shown_bt(0);
    }
#endif
#ifdef CONFIG_SD_MUSIC_MODE_ENABLE
    void tm_1629_shown_tf(char on);
    if (!strcmp(it.name, "sd_music")) {
        tm_1629_shown_tf(1);
    } else {
        tm_1629_shown_tf(0);
    }
#endif
#ifdef CONFIG_ASR_ALGORITHM_ENABLE
    void tm_1629_shown_ai(char on);
    if (!strcmp(it.name, "ai_speaker")) {
        tm_1629_shown_ai(1);
    } else {
        tm_1629_shown_ai(0);
    }
#endif
#endif

#ifdef CONFIG_UI_PLAY_EMOJI
    extern int play_face_emoji(int index);
    if (!strcmp(it.name, "ai_speaker")) {
        play_face_emoji(AI_UART_CMD_AI_MODE);
    } else if (!strcmp(it.name, "bt_music")) {
        play_face_emoji(AI_UART_CMD_BT_MODE);
    } else if (!strcmp(it.name, "sd_music")) {
        play_face_emoji(AI_UART_CMD_SD_TF_MODE);
    } else if (!strcmp(it.name, "usbdisk_music")) {
        play_face_emoji(AI_UART_CMD_UDISK_MODE);
    } else if (!strcmp(it.name, "aux_music")) {
        play_face_emoji(AI_UART_CMD_AUX_MODE);
    }
#endif

    if (play) {
        music_play_waite();//等提示音播完了再切换模式
    }

    start_app(&it);
    mode_index++;
    return 0;
}

int audio_app_all_play_stop_status(char mode)
{
    int stop_status = 1;
#if defined CONFIG_BT_ENABLE && defined CONFIG_BT_MUSIC_MODE_ENABLE
    extern int bt_music_decode_stop_status(void);
    stop_status = bt_music_decode_stop_status();
#endif
#ifdef CONFIG_AUX_MUSIC_MODE_ENABLE
    extern int aux_music_stop_status(void);
    if (stop_status) {
        stop_status = aux_music_stop_status();
    }
#endif
#ifdef CONFIG_SD_MUSIC_MODE_ENABLE
    extern int sd_music_stop_status(void);
    if (stop_status) {
        stop_status = sd_music_stop_status();
    }
#endif
#ifdef CONFIG_USB_DISK_MUSIC_MODE_ENABLE
    extern int usbdisk_music_stop_status(void);
    if (stop_status) {
        stop_status = usbdisk_music_stop_status();
    }
#endif
    if (mode) {//该模式下，只有检测各个APP是否处于播放状态，AI对话不在检测范围内
        if (stop_status) {
            stop_status = net_music_play_stop_status();
        }
        return stop_status;
    }
    if (stop_status && music_buf_play_stop_staus() && music_play_stop_status() && net_music_play_stop_status()) {
        return true;
    }
    return false;
}
