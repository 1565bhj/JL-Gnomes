#include "web_socket/websocket_api.h"
#include "wifi/wifi_connect.h"
#include "system/includes.h"
#include "event/key_event.h"
#include "os/os_api.h"
#include "wav_head.h"
#include "fs/fs.h"
#include "json_c/json.h"
#include "json_c/json_tokener.h"
#include "app_config.h"
#include "event/device_event.h"
#include "ai_uart_ctrol.h"

extern int ai_speaker_volume_set(int volume, char update_gain);
extern int music_play_anser_OK(void);
extern int websockets_client_dialogue_timeout_exit(char *use_voice_note);
extern int music_buf_play_supspend(void);
extern int music_play_waite(void);
extern int net_music_play_last(void);
extern int net_music_play_last_check(void);
extern void net_music_play_set_stop(void);
extern void net_music_play_puase(void);
extern void net_music_play_resum(void);
extern int net_music_play_loop(void);
extern void net_music_play_set_stop_notic(void);
extern int net_music_play_next(char *priv);
extern void net_music_play_last_request(void);
extern int audio_app_mode_num_switch(char index, char note);
extern void alarm_clean(void);

//=================系统指令=====================//
//////////////////////////////////////////
static const char *sys_close[] = {
    "关了", "关闭", "关掉", "停了", "退下", "对话关了", "对话关闭", "关掉对话", "对话关掉", "对话停了",
    "结束对话", "关闭对话", "滚", "不想和你说了", "不想和你说话了",
#ifdef CONFIG_KWS_ENGLISH
    "turn off", "close it", "close", "shut it down", "shut down", "shutdown", "stop it", "stop", "byebye", "goodbye",
    "Turn Off", "Close it", "Close", "Shut it down", "Shut down", "Shutdown", "Stop it", "Stop", "Byebye", "Goodbye",
    "system_exit", "system exit",
    "System_exit", "System exit",
#endif
    NULL,
};

static const char *sys_str_power_off[] = {
    "关机",
#ifdef CONFIG_KWS_ENGLISH
    "power off", "Power off",
    "turn off power", "Turn off power",
#endif
    NULL,
};

/////////////////////////////////////////
static const char *sys_30min[] = {//先长后短
    "一个半小时过后", "一个半小时以后", "一个半小时后",
    "半个小时过后", "半个小时以后", "半个小时后",
    "半小时过后", "半小时以后", "半小时后",
#ifdef CONFIG_KWS_ENGLISH
    "After half an hour",
    "after half an hour",
#endif
    NULL,
};

static const char *sys_90min[] = {
    "一个小时半过后", "一个小时半以后", "一个小时半后",
#ifdef CONFIG_KWS_ENGLISH
    "After one and a half hours",
    "after one and a half hours",
#endif
    NULL,
};

/////////////////////////////////////////
static const char *sys_hour_timeout[] = {//先长后短
    "个小时过后", "个小时以后", "个小时后", "小时过后", "小时以后", "小时后",
#ifdef CONFIG_KWS_ENGLISH
    "hours after",
#endif
    NULL,
};

static const char *sys_min_timeout[] = {//先长后短
    "分钟过后", "分钟以后", "分钟后",
#ifdef CONFIG_KWS_ENGLISH
    "minutes later",
#endif
    NULL,
};

/////////////////////////////////////////
static const char *sys_countown_timeout[] = {//先长后短
    "倒计时", "计时",
#ifdef CONFIG_KWS_ENGLISH
    "Countdown",
    "countdown",
#endif
    NULL,
};

static const char *sys_countown_timeout_close[] = {//先长后短
    "取消计时", "删除计时", "取消倒计时", "删除倒计时",
#ifdef CONFIG_KWS_ENGLISH
    "Cancel timer", "Delete timer", "Cancel countdown", "Delete countdown",
    "cancel timer", "delete timer", "cancel countdown", "delete countdown",
#endif
    NULL,
};

//static const char *sys_sec_timeout[] = {//先长后短
//    "秒过后","秒以后","秒后",
//    NULL,
//};
/////////////////////////////////////////
//=================音乐指令=====================//
static const char *sys_music_play[] = {
    "播放", "播放歌曲", "播放音乐",
#ifdef CONFIG_KWS_ENGLISH
    "Play", "Play song", "Play music",
    "play", "play song", "play music",
#endif
    NULL,
};

static const char *sys_music_continue[] = {
    "继续播放", "继续播歌", "继续播放音乐",
#ifdef CONFIG_KWS_ENGLISH
    "Continue playing", "Continue playing song", "Continue playing music",
    "continue playing", "continue playing song", "continue playing music",
    "music_resume", "music resume",
    "Music_resume", "Music resume",
#endif
    NULL,
};

static const char *sys_music_stop[] = {
    "暂停", "暂停播放", "暂停播放音乐",
#ifdef CONFIG_KWS_ENGLISH
    "Pause", "Pause playback", "Pause music",
    "pause", "pause playback", "pause music",
    "music_pause", "music pause",
    "Music_pause", "Music pause",
#endif
    NULL,
};

static const char *sys_music_close[] = {
    "停止播歌", "停止播放", "停止播放音乐", "关闭音乐", "音乐关闭", "关闭歌曲", "歌曲关闭", "音乐停止", "停止音乐",
#ifdef CONFIG_KWS_ENGLISH
    "Stop song", "Stop playback", "Stop music", "Turn off music", "Music off", "Turn off song", "Song off", "Music stop", "Stop music",
    "stop song", "stop playback", "stop music", "turn off music", "music off", "turn off song", "song off", "music stop", "stop music",
    "music_stop", "music stop",
    "Music_stop", "Music stop",
#endif
    NULL,
};

static const char *sys_music_next[] = {
    "下一首", "播放下一首", "换一首", "换一首歌", "换一首音乐", "切歌", "换歌",
#ifdef CONFIG_KWS_ENGLISH
    "Next song", "Play next", "Change one", "Change song", "Change music", "Skip song", "Change track",
    "next song", "play next", "change one", "change song", "change music", "skip song", "change track",
    "music_next_track", "music next track",
    "Music_next_track", "Music next track",
#endif
    NULL,
};

static const char *sys_music_last[] = {
    "上一首", "播放上一首", "播放上一首歌", "播放上一首音乐",
#ifdef CONFIG_KWS_ENGLISH
    "Previous song", "Play previous", "Play previous song", "Play previous music",
    "previous song", "play previous", "play previous song", "play previous music",
    "music_previous_track", "music previous track",
    "Music_previous_track", "Music previous track",
#endif
    NULL,
};

static const char *sys_music_loop[] = {
    "单曲循环",
#ifdef CONFIG_KWS_ENGLISH
    "Single track loop", "Single song loop", "Single song cycle",
    "single track loop", "single song loop", "single song cycle",
    "music_loop_single", "music loop single",
    "Music_loop_single", "Music loop single",
#endif
    NULL,
};

static const char *sys_music_loop_close[] = {
    "关闭循环", "退出循环", "取消循环",
    "关闭单曲循环", "退出单曲循环", "取消单曲循环",
#ifdef CONFIG_KWS_ENGLISH
    "Turn off loop", "Exit loop", "Cancel loop",
    "Turn off single track loop", "Exit single track loop", "Cancel single track loop",
    "Turn off single song loop", "Exit single song loop", "Cancel single song loop",

    "turn off loop", "exit loop", "cancel loop",
    "turn off single track loop", "exit single track loop", "cancel single track loop",
    "turn off single song loop", "exit single song loop", "cancel single song loop",
#endif
    NULL,
};

//=================设备控制指令=====================//
static const char *sys_music_app_ai[] = {
    "AI模式", "Ai模式", "AI智能体模式", "智能体模式",
#ifdef CONFIG_KWS_ENGLISH
    "AI mode", "AI agent mode", "Agent mode",
    "Ai mode", "Ai agent mode", "agent mode",
    "ai mode", "ai agent mode", "agent mode",
#endif
    NULL,
};

static const char *sys_music_app_udisk[] = {
    "播放U盘", "U盘播放模式", "U盘模式",
#ifdef CONFIG_KWS_ENGLISH
    "Play USB disk", "USB disk play mode", "USB disk mode",
    "play USB disk", "usb disk play mode", "usb disk mode",
    "source_usb", "source usb",
    "Source_usb", "Source usb",
#endif
    NULL,
};

static const char *sys_music_app_sd[] = {
    "播放SD卡", "播放TF卡", "SD卡播放模式", "TF卡播放模式", "SD卡模式", "TF卡模式",
    "Sd卡播放模式", "Tf卡播放模式", "Sd卡模式", "Tf卡模式",
#ifdef CONFIG_KWS_ENGLISH
    "Play SD card", "Play TF card", "SD card play mode", "TF card play mode", "SD card mode", "TF card mode",
    "SD card play mode", "TF card play mode", "SD card mode", "TF card mode", "SD mode", "TF mode",

    "play SD card", "play TF card", "sd card play mode", "tf card play mode", "sd card mode", "tf card mode",
    "sd card play mode", "tf card play mode", "SD card mode", "TF card mode", "sd mode", "tf mode",
    "source_sdcard", "source sdcard",
    "Source_sdcard", "Source sdcard",
#endif
    NULL,
};

static const char *sys_music_app_bt[] = {
    "播放蓝牙", "蓝牙模式", "蓝牙播放模式",
#ifdef CONFIG_KWS_ENGLISH
    "Play Bluetooth", "Bluetooth mode", "Bluetooth play mode",
    "play Bluetooth", "bluetooth mode", "bluetooth play mode",
#endif
    NULL,
};

static const char *sys_music_app_aux[] = {
    "线路模式", "线路输入模式",
#ifdef CONFIG_KWS_ENGLISH
    "Line mode", "Line input mode",
    "line mode", "line input mode",
#endif
    NULL,
};

//=================闹钟指令=====================//
static const char *sys_alarm_del[] = {
    "删除闹钟", "取消闹钟", "取消提醒", "关闭闹钟",
#ifdef CONFIG_KWS_ENGLISH
    "Delete alarm", "Cancel alarm", "Cancel reminder", "Turn off alarm",
    "delete alarm", "cancel alarm", "cancel reminder", "turn off alarm",
#endif
    NULL,
};

static const char *sys_alarm_del_all[] = {
    "删除所有闹钟", "取消所有闹钟", "取消所有提醒", "关闭所有闹钟",
#ifdef CONFIG_KWS_ENGLISH
    "Delete all alarms", "Cancel all alarms", "Cancel all reminders", "Turn off all alarms",
    "delete all alarms", "cancel all alarms", "cancel all reminders", "turn off all alarms",
#endif
    NULL,
};

static const char *sys_alarm_look[] = {
    "查看闹钟", "有哪些闹钟", "闹钟页面", "进入闹钟页面", "查看闹钟页面",
#ifdef CONFIG_KWS_ENGLISH
    "View alarms", "What alarms are there", "Alarm page", "Enter alarm page", "View alarm page",
    "view alarms", "what alarms are there", "alarm page", "enter alarm page", "view alarm page",
#endif
    NULL,
};
void respon_ok(void *waite);
int sys_alarm_look_callback(char *word)
{
    int i;

    for (i = 0; sys_alarm_look[i] != NULL; i++) { //AI模式
        if (!strcmp(word, sys_alarm_look[i])) {
            respon_ok(1);//好的
#ifdef CONFIG_LVGL_UI_ENABLE
            lv_demo_switch_to_alarm_page();

#endif
            puts("-> sys_alarm_look\n");
            return 1;
        }
    }
    return 0;
}
//=================音量调节指令=====================//
#define SYS_VOLUME_SET      "音量调到"
#define SYS_VOLUME_SET_EN  "volume_set_"
static const char *sys_music_volume[] = {
    "小声一点", "音乐小声一点", "音量调低", "音量调小",
    "大声一点", "音乐大声一点", "音量调高", "音量调大",
#ifdef CONFIG_KWS_ENGLISH
    "A little quieter", "Music a little quieter", "Turn volume down", "Lower volume",
    "A little louder", "Music a little louder", "Turn volume up", "Increase volume",

    "a little quieter", "music a little quieter", "turn volume down", "lower volume",
    "a little louder", "music a little louder", "turn volume up", "increase volume",

    "volume_down", "volume down", "set volume_down", "set volume down",
    "volume_up", "volume up", "set volume_up", "set volume up",

    "Volume_down", "Volume down", "Set volume_down", "Set volume down",
    "Volume_up", "Volume up", "Set volume_up", "Set volume up",
#endif
    NULL,
};
static const char *sys_music_max_volume[] = {
    "最大音量", "最高音量", "音量最大", "音量最高", "最大声音", "最高声音", "声音最大", "声音最高",
#ifdef CONFIG_KWS_ENGLISH
    "max volume", "maximum volume", "Max volume", "Maximum volume",
#endif
    NULL,
};
static const char *sys_music_min_volume[] = {
    "最小音量", "最低音量", "音量最小", "音量最低", "最小声音", "最低声音", "声音最小", "声音最低",
#ifdef CONFIG_KWS_ENGLISH
    "min volume", "minimum volume", "Min volume", "Minimum volume",
#endif
    NULL,
};

enum {
    MUSIC_INIT = 0,
    MUSIC_STOP,
    MUSIC_CLOSE,
    MUSIC_CONTTINUE,
    MUSIC_PLAY,
    MUSIC_NEXT,
    MUSIC_LAST,
    MUSIC_LOOP,
    MUSIC_LOOP_CLOSE,
    MUSIC_SET_VOLUME,
};

enum {
    MUSIC_APP_AI = 0,//下面顺序务必和audio_app_table[]一样
#if defined CONFIG_BT_ENABLE && defined CONFIG_BT_MUSIC_MODE_ENABLE
    MUSIC_APP_BT,
#endif
#ifdef CONFIG_USB_DISK_MUSIC_MODE_ENABLE
    MUSIC_APP_USB_DISK,
#endif
#ifdef CONFIG_SD_MUSIC_MODE_ENABLE
    MUSIC_APP_SD,
#endif
#ifdef CONFIG_AUX_MUSIC_MODE_ENABLE
    MUSIC_APP_AUX,
#endif
};

static void respon_ok(void *waite)
{
#ifdef SYS_COMMAND_ANSER_PLAY_ENABLE
    music_play_anser_OK();//好的
    if (waite) {
        music_play_waite();//等提示音播完了关闭设备
    }
#endif
}

static void respon_volume_max_min(int max, void *wait)
{
#ifdef SYS_COMMAND_ANSER_PLAY_ENABLE
    if (max) {
        music_play_res_file("MaxVolume.mp3");
    } else {
        music_play_res_file("MinVolume.mp3");
    }
    if (wait) {
        music_play_waite();//等提示音播完了关闭设备
    }
#endif
}

static void volume_respon(void *waite, int dec)
{
#ifdef SYS_COMMAND_ANSER_PLAY_ENABLE
    music_play_anser_volume(dec);//音量
    if (waite) {
        music_play_waite();//等提示音播完了关闭设备
    }
#endif
}

static int sys_music_app_switch_check(char *word)
{
    int app_status = 0;
    int i;

    for (i = 0; sys_music_app_ai[i] != NULL; i++) { //AI模式
        if (strstr(word, sys_music_app_ai[i]) && strlen(word) <= strlen(sys_music_app_ai[i]) + 3) {
            respon_ok(1);//好的
            app_status = MUSIC_APP_AI;
            puts("-> close_dialuge\n");
            goto exit;
        }
    }
#if defined CONFIG_BT_ENABLE && defined CONFIG_BT_MUSIC_MODE_ENABLE
    for (i = 0; sys_music_app_bt[i] != NULL; i++) { //蓝牙盘模式
        if (strstr(word, sys_music_app_bt[i]) && strlen(word) <= strlen(sys_music_app_bt[i]) + 3) {
            respon_ok(1);//好的
            app_status = MUSIC_APP_BT;
#ifdef CONFIG_LVGL_UI_ENABLE
            lv_demo_music_clean();//清除音乐接口
            lv_demo_switch_to_music_page("bt_music");
#endif
            puts("-> close_dialuge\n");
            goto exit;
        }
    }
#endif
#ifdef CONFIG_USB_DISK_MUSIC_MODE_ENABLE
    for (i = 0; sys_music_app_udisk[i] != NULL; i++) { //U盘模式
        if (strstr(word, sys_music_app_udisk[i]) && strlen(word) <= strlen(sys_music_app_udisk[i]) + 3) {
            respon_ok(1);//好的
            app_status = MUSIC_APP_USB_DISK;
#ifdef CONFIG_LVGL_UI_ENABLE
            lv_demo_music_clean();//清除音乐接口
            lv_demo_switch_to_music_page("usbdisk_music");
#endif
            puts("-> close_dialuge\n");
            goto exit;
        }
    }
#endif
#ifdef CONFIG_SD_MUSIC_MODE_ENABLE
    for (i = 0; sys_music_app_sd[i] != NULL; i++) { //SD卡模式
        if (strstr(word, sys_music_app_sd[i]) && strlen(word) <= strlen(sys_music_app_sd[i]) + 3) {
            respon_ok(1);//好的
            app_status = MUSIC_APP_SD;
#ifdef CONFIG_LVGL_UI_ENABLE
            lv_demo_music_clean();//清除音乐接口
            lv_demo_switch_to_music_page("sd_music");
#endif
            puts("-> close_dialuge\n");
            goto exit;
        }
    }
#endif
#ifdef CONFIG_AUX_MUSIC_MODE_ENABLE
    for (i = 0; sys_music_app_aux[i] != NULL; i++) { //AUX模式
        if (strstr(word, sys_music_app_aux[i]) && strlen(word) <= strlen(sys_music_app_aux[i]) + 3) {
            respon_ok(1);//好的
            app_status = MUSIC_APP_AUX;
#ifdef CONFIG_LVGL_UI_ENABLE
            lv_demo_music_clean();//清除音乐接口
            lv_demo_switch_to_music_page("aux_music");
#endif
            puts("-> close_dialuge\n");
            goto exit;
        }
    }
#endif
    return 0;
exit:
    audio_app_mode_num_switch(app_status, 1);
    return 1;//1立马关闭对话
}

extern int keyworld_start;
static void sys_close_timeout_check(void)
{
    if (keyworld_start) {
        websockets_free_lbuf_buf();
        websockets_close_request(1);
    }
}

int sys_instruction_word_callback(char *word, char *asr)
{
    struct key_event key = {0};
    int music_status = 0;
    int close_type = 0;
    int i;
#if (defined AI_UART_CMD_CTROL_ENABLE || defined AT_UART_CMD_ENABLE)
    char sbuf[64];
#endif

    word = word ? word : (asr ? asr : NULL);
    asr = asr ? asr : (word ? word : NULL);
    if (!word && !asr) {
        return 0;
    }
#ifdef CONFIG_ASR_POWER_OFF_ENABLE
    for (i = 0; sys_str_power_off[i] != NULL; i++) { //关机
        if (strstr(word, sys_str_power_off[i]) && strlen(word) <= strlen(sys_str_power_off[i]) + 3) {
            int aisp_sys_power_off(void);
            sys_timeout_add_to_task("sys_timer", NULL, aisp_sys_power_off, 1000);
#ifdef AI_UART_CMD_CTROL_ENABLE
            ai_uart_cmd_data_push(AI_UART_CMD_PWR_OFF, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
            at_uart_cmd_send(AI_UART_CMD_PWR_OFF, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
            play_face_emoji(AI_UART_CMD_PWR_OFF);
#endif
#ifndef CONFIG_USE_TTS_REPLY_ENABLE
            respon_ok(1);//好的
#endif
            close_type = 1;//1立马关闭对话
            goto exit;
        }
    }
#endif
    for (i = 0; sys_close[i] != NULL; i++) { //关闭设备对话
        if (strstr(word, sys_close[i]) && strlen(word) <= strlen(sys_close[i]) + 3) {
            printf("-> close_dialuge : %s %s\n", word, sys_close[i]);
            if (ai_speaker_app()) {
                music_status = MUSIC_CLOSE;
            }
            respon_ok(1);//好的
            //audio_app_mode_num_switch(MUSIC_APP_AI, 0);//关闭系统默认切回AI智能体模式
            close_type = 1;//1立马关闭对话
            net_music_play_loop_clear();
#ifdef CONFIG_ASR_POWER_OFF_ENABLE
            if (asr && strstr(asr, "关机") && strlen(asr) <= 9) {
                int aisp_sys_power_off(void);
                sys_timeout_add_to_task("sys_timer", NULL, aisp_sys_power_off, 1000);
#ifdef AI_UART_CMD_CTROL_ENABLE
                ai_uart_cmd_data_push(AI_UART_CMD_PWR_OFF, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
                at_uart_cmd_send(AI_UART_CMD_PWR_OFF, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
                play_face_emoji(AI_UART_CMD_PWR_OFF);
#endif
            } else {
                sys_timeout_add_to_task("sys_timer", NULL, sys_close_timeout_check, 1000);//1秒后检查时钟主动退出，没有则断开连接
            }
#else
            sys_timeout_add_to_task("sys_timer", NULL, sys_close_timeout_check, 1000);//1秒后检查时钟主动退出，没有则断开连接
#endif
#ifdef CONFIG_LVGL_UI_ENABLE
            lv_demo_music_clean();//清除音乐接口
            lv_demo_switch_to_main_page();
#endif
            goto exit;
        }
    }
    for (i = 0; sys_alarm_del_all[i] != NULL; i++) { //删除所有闹钟
        if (strstr(word, sys_alarm_del_all[i]) && strlen(word) <= strlen(sys_alarm_del_all[i]) + 3) {
            puts("-> larm_del_all\n");
            alarm_clean();
            if (music_play_res_file("AlarmDelAll.mp3")) { //闹钟已取消
                respon_ok(1);//好的
            }
#ifdef AI_UART_CMD_CTROL_ENABLE
            ai_uart_cmd_data_push(AI_UART_CMD_ALARM_DEL_ALL, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
            at_uart_cmd_send(AI_UART_CMD_ALARM_DEL_ALL, NULL);
#endif
#ifdef CONFIG_LVGL_UI_ENABLE
            lv_demo_switch_to_alarm_page();
            lv_demo_all_alarms_del_flush_from_server();
#endif
            close_type = 1;//1立马关闭对话
            goto exit;
        }
    }
#if 0 //播放音乐由服务器主动下发
    for (i = 0; sys_music_play[i] != NULL; i++) { //播放歌曲
        if (strstr(word, sys_music_play[i]) && strlen(word) <= strlen(sys_music_play[i]) + 3) {
            music_status = MUSIC_PLAY;
            close_type = 1;//1立马关闭对话
            puts("-> MUSIC_PLAY\n");
            goto exit;
        }
    }
#endif
    for (i = 0; sys_music_stop[i] != NULL; i++) { //暂停播放
        if (strstr(word, sys_music_stop[i]) && strlen(word) <= strlen(sys_music_stop[i]) + 3) {
            music_status = MUSIC_STOP;
            close_type = 2;//2立马关闭对话不恢复音乐播放
            puts("-> MUSIC_STOP\n");
            goto exit;
        }
    }
    for (i = 0; sys_music_close[i] != NULL; i++) { //停止播放
        if (strstr(word, sys_music_close[i]) && strlen(word) <= strlen(sys_music_close[i]) + 3) {
            music_status = MUSIC_CLOSE;
            respon_ok(1);//好的
            close_type = 1;//2立马关闭对话不恢复音乐播放
            puts("-> MUSIC_CLOSE\n");
            goto exit;
        }
    }
    for (i = 0; sys_music_continue[i] != NULL; i++) { //继续播放
        if (strstr(word, sys_music_continue[i]) && strlen(word) <= strlen(sys_music_continue[i]) + 3) {
            music_status = MUSIC_CONTTINUE;
            close_type = 1;//1立马关闭对话
            puts("-> MUSIC_CONTTINUE\n");
            goto exit;
        }
    }
    for (i = 0; sys_music_next[i] != NULL; i++) { //下一首播放
        if (strstr(word, sys_music_next[i]) && strlen(word) <= strlen(sys_music_next[i]) + 3) {
            music_status = MUSIC_NEXT;
            close_type = 1;//1立马关闭对话
            puts("-> MUSIC_NEXT\n");
            goto exit;
        }
    }
    for (i = 0; sys_music_last[i] != NULL; i++) { //上一首播放
        if (strstr(word, sys_music_last[i]) && strlen(word) <= strlen(sys_music_last[i]) + 3) {
            music_status = MUSIC_LAST;
            close_type = 1;//1立马关闭对话
            puts("-> MUSIC_LAST\n");
            goto exit;
        }
    }
    for (i = 0; sys_music_loop_close[i] != NULL; i++) { //退出单曲循环
        if (strstr(word, sys_music_loop_close[i]) && strlen(word) <= strlen(sys_music_loop_close[i]) + 3) {
            respon_ok(1);//好的
            music_status = MUSIC_LOOP_CLOSE;
            close_type = 1;//1立马关闭对话
            puts("-> MUSIC_LOOP_CLOSE\n");
            goto exit;
        }
    }
    for (i = 0; sys_music_loop[i] != NULL; i++) { //单曲循环
        if (strstr(word, sys_music_loop[i]) && strlen(word) <= strlen(sys_music_loop[i]) + 3) {
            respon_ok(1);//好的
            music_status = MUSIC_LOOP;
            close_type = 1;//1立马关闭对话
            puts("-> MUSIC_LOOP\n");
            goto exit;
        }
    }
    for (i = 0; sys_music_max_volume[i] != NULL; i++) { //最大音量
        if (strstr(word, sys_music_max_volume[i]) && strlen(word) <= strlen(sys_music_max_volume[i]) + 3) {
            ai_speaker_volume_set(MAX_VOLUME_VALUE, 0);
            music_status = MUSIC_SET_VOLUME;
            close_type = 1;//1立马关闭对话
            puts("-> MUSIC_SET_MAX_VOLUME\n");
            goto exit;
        }
    }
    for (i = 0; sys_music_min_volume[i] != NULL; i++) { //最小音量
        if (strstr(word, sys_music_min_volume[i]) && strlen(word) <= strlen(sys_music_min_volume[i]) + 3) {
            ai_speaker_volume_set(MIN_VOLUME_VALUE, 0);
            music_status = MUSIC_SET_VOLUME;
            close_type = 1;//1立马关闭对话
            puts("-> MUSIC_SET_MIN_VOLUME\n");
            goto exit;
        }
    }
    for (i = 0; sys_music_volume[i] != NULL; i++) { //音量条件
        if (strstr(word, sys_music_volume[i]) && strlen(word) <= strlen(sys_music_volume[i]) + 3) {
            puts("-> MUSIC_VOLUME\n");
            music_status = MUSIC_SET_VOLUME;
            char dec = (i % 8) < 4 ? 1 : 0;
            struct key_event key = {0};
            key.action = KEY_EVENT_CLICK;
            key.type = KEY_EVENT_USER;
            key.value = dec ? KEY_VOLUME_DEC : KEY_VOLUME_INC;
            key_event_notify(KEY_EVENT_FROM_USER, &key);
#ifdef AI_UART_CMD_CTROL_ENABLE
            ai_uart_cmd_data_push(dec ? AI_UART_CMD_VOLUME_DEC : AI_UART_CMD_VOLUME_INC, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
            at_uart_cmd_send(dec ? AI_UART_CMD_VOLUME_DEC : AI_UART_CMD_VOLUME_INC, NULL);
#endif
            if (dec && sys_volume_read(NULL) >= MIN_VOLUME_VALUE + VOLUME_STEP) {
                volume_respon(1, dec);
            } else if (!dec && sys_volume_read(NULL) <= MAX_VOLUME_VALUE - VOLUME_STEP) {
                volume_respon(1, dec);
            }
            close_type = 1;//1立马关闭对话
            goto exit;
        } else if (strstr(word, SYS_VOLUME_SET) || strstr(word, SYS_VOLUME_SET_EN)) {
            u8 vol_old = 0;
            sys_volume_read(&vol_old);
            music_status = MUSIC_SET_VOLUME;
            char *new_word = strstr(word, SYS_VOLUME_SET);
            if (!new_word) {
                new_word = strstr(word, SYS_VOLUME_SET_EN);
                new_word += strlen(SYS_VOLUME_SET_EN);
            } else {
                new_word += strlen(SYS_VOLUME_SET);
            }
            word = new_word;
            int volume = atoi(word);
            if (volume > 0 && volume <= 100) {
                volume = volume < MIN_VOLUME_VALUE ? MIN_VOLUME_VALUE : volume;
                printf("-> music volume set:%d\n", volume);
                if ((vol_old == MAX_VOLUME_VALUE || vol_old == MIN_VOLUME_VALUE) && volume == vol_old) {
                    respon_volume_max_min(vol_old == MAX_VOLUME_VALUE, 0);
                } else {
                    ai_speaker_volume_set(volume, 0);
                    respon_ok(1);//好的
                }
                close_type = 1;//1立马关闭对话
#ifdef AI_UART_CMD_CTROL_ENABLE
                sprintf(sbuf, "{\"volume\":%d}", volume);
                ai_uart_cmd_data_push(AI_UART_CMD_VOLUME_SET, sbuf, strlen(sbuf) + 1);
#endif
#ifdef AT_UART_CMD_ENABLE
                at_uart_cmd_send(AI_UART_CMD_VOLUME_SET, sbuf);
#endif
                goto exit;
            } else {
                char *n = word;
                volume = 0;
                printf("-> n:%s\n", n);
                if ((strstr(n, "最小") && strlen(n) <= strlen("最小") + 3) || (strstr(n, "最低") && strlen(n) <= strlen("最低") + 3) ||
                    strstr(n, "min volume") || strstr(n, "minimum volume") ||
                    strstr(n, "Min volume") || strstr(n, "Minimum volume")) {
                    volume = MIN_VOLUME_VALUE;
                } else if ((strstr(n, "最大") && strlen(n) <= strlen("最大") + 3) || (strstr(n, "最高") && strlen(n) <= strlen("最高") + 3) ||
                           strstr(n, "max volume") || strstr(n, "maximum volume") ||
                           strstr(n, "Max volume") || strstr(n, "Maximum volume")) {
                    volume = MAX_VOLUME_VALUE;
                } else {
                    char *pnum[] = {"", "一", "二", "三", "四", "五", "六", "七", "八", "九", "十", "百"};
                    for (int i = 0; i < 12; i++) {
                        char str[16];
                        char *f = NULL;
                        if (i < 10) {
                            for (int j = 0; j < 10; j++) {
                                sprintf(str, "%s%s%s", pnum[9 - i], pnum[10], pnum[9 - j]);
                                f = strstr(n, str);
                                if (f) {
                                    volume = (i == 9 && j == 9) ? 10 : (i == 9 && j != 9) ? (10 + (9 - j)) : (i != 9 && j == 9) ? ((9 - i) * 10) : ((9 - i) * 10 + (9 - j));
                                    break;
                                }
                            }
                        } else if (i >= 10) {
                            if (strstr(n, "一百")) {
                                volume = 100;
                                break;
                            } else {
                                for (int j = 1; j <= 10; j++) {
                                    sprintf(str, "%s", pnum[j]);
                                    f = strstr(n, str);
                                    if (f) {
                                        volume = j;
                                        break;
                                    }
                                }
                            }
                        }
                        if (f) {
                            break;
                        }
                    }
                }
                printf("-> music volume set:%d\n", volume);
                if ((vol_old == MAX_VOLUME_VALUE || vol_old == MIN_VOLUME_VALUE) && volume == vol_old) {
                    respon_volume_max_min(vol_old == MAX_VOLUME_VALUE, 0);
                } else if (volume) {
                    ai_speaker_volume_set(volume, 0);
                    respon_ok(1);//好的
                }
                close_type = 1;//1立马关闭对话
#ifdef AI_UART_CMD_CTROL_ENABLE
                sprintf(sbuf, "{\"volume\":%d}", volume);
                ai_uart_cmd_data_push(AI_UART_CMD_VOLUME_SET, sbuf, strlen(sbuf) + 1);
#endif
#ifdef AT_UART_CMD_ENABLE
                at_uart_cmd_send(AI_UART_CMD_PWR_OFF, volume);
#endif
                goto exit;
            }
        }
    }
    close_type = sys_music_app_switch_check(word);
    return close_type;
exit:
    if (music_status) {
        switch (music_status) {
        case MUSIC_STOP:
#ifdef AI_UART_CMD_CTROL_ENABLE
            ai_uart_cmd_data_push(AI_UART_CMD_MUSIC_PUASE, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
            at_uart_cmd_send(AI_UART_CMD_MUSIC_PUASE, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
            play_face_emoji(AI_UART_CMD_MUSIC_PUASE);
#endif
            if (ai_speaker_app()) {
                if (net_music_play_pause_status()) {
                    respon_ok(0);//好的
                }
                net_music_play_puase();
            } else {
                respon_ok(1);//好的
                key.type = KEY_EVENT_USER;
                key.action = KEY_EVENT_CLICK;
                key.value = KEY_SUPSPEND;
                key_event_notify(KEY_EVENT_FROM_USER, &key);
            }
            break;
        case MUSIC_CLOSE:
#ifdef AI_UART_CMD_CTROL_ENABLE
            ai_uart_cmd_data_push(AI_UART_CMD_MUSIC_STOP, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
            at_uart_cmd_send(AI_UART_CMD_MUSIC_STOP, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
            play_face_emoji(AI_UART_CMD_MUSIC_STOP);
#endif
            if (ai_speaker_app()) {
                net_music_play_set_stop_notic();
            } else {
                key.type = KEY_EVENT_USER;
                key.action = KEY_EVENT_CLICK;
                key.value = KEY_SUPSPEND;
                key_event_notify(KEY_EVENT_FROM_USER, &key);
            }
            break;
        case MUSIC_CONTTINUE:
#ifdef AI_UART_CMD_CTROL_ENABLE
            ai_uart_cmd_data_push(AI_UART_CMD_MUSIC_CONTINUE, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
            at_uart_cmd_send(AI_UART_CMD_MUSIC_CONTINUE, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
            play_face_emoji(AI_UART_CMD_MUSIC_CONTINUE);
#endif
            if (ai_speaker_app()) {
                if (music_buf_play_supspend()) {
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
            break;
        case MUSIC_PLAY:
#ifdef AI_UART_CMD_CTROL_ENABLE
            ai_uart_cmd_data_push(AI_UART_CMD_MUSIC_START, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
            at_uart_cmd_send(AI_UART_CMD_MUSIC_START, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
            play_face_emoji(AI_UART_CMD_MUSIC_START);
#endif
            net_music_play_next(net_music_play_type_get());
            break;
        case MUSIC_NEXT:
#ifdef AI_UART_CMD_CTROL_ENABLE
            ai_uart_cmd_data_push(AI_UART_CMD_MUSIC_NEXT, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
            at_uart_cmd_send(AI_UART_CMD_MUSIC_NEXT, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
            play_face_emoji(AI_UART_CMD_MUSIC_NEXT);
#endif
            if (ai_speaker_app()) {
                net_music_play_next(net_music_play_type_get());
            } else {
                key.type = KEY_EVENT_USER;
                key.action = KEY_EVENT_CLICK;
                key.value = KEY_DOWN;
                key_event_notify(KEY_EVENT_FROM_USER, &key);
            }
            break;
        case MUSIC_LAST:
#ifdef AI_UART_CMD_CTROL_ENABLE
            ai_uart_cmd_data_push(AI_UART_CMD_MUSIC_LAST, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
            at_uart_cmd_send(AI_UART_CMD_MUSIC_LAST, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
            play_face_emoji(AI_UART_CMD_MUSIC_LAST);
#endif
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
            break;
        case MUSIC_LOOP:
#ifdef AI_UART_CMD_CTROL_ENABLE
            ai_uart_cmd_data_push(AI_UART_CMD_MUSIC_LOOP, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
            at_uart_cmd_send(AI_UART_CMD_MUSIC_LOOP, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
            play_face_emoji(AI_UART_CMD_MUSIC_LOOP);
#endif
            if (ai_speaker_app()) {
                net_music_play_loop();
            } else {
                key.type = KEY_EVENT_USER;
                key.action = KEY_EVENT_CLICK;
                key.value = KEY_LOOP;
                key_event_notify(KEY_EVENT_FROM_USER, &key);
            }
            break;
        case MUSIC_LOOP_CLOSE:
#ifdef AI_UART_CMD_CTROL_ENABLE
            ai_uart_cmd_data_push(AI_UART_CMD_MUSIC_LOOP_EXIT, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
            at_uart_cmd_send(AI_UART_CMD_MUSIC_LOOP_EXIT, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
            play_face_emoji(AI_UART_CMD_MUSIC_LOOP_EXIT);
#endif
            if (ai_speaker_app()) {
                int net_music_play_loop_fore_set(char loop);
                net_music_play_loop_fore_set(0);
            } else {
                key.type = KEY_EVENT_USER;
                key.action = KEY_EVENT_CLICK;
                key.value = KEY_EXIT_LOOP;
                key_event_notify(KEY_EVENT_FROM_USER, &key);
            }
            break;
        case MUSIC_SET_VOLUME:
            break;
        default:
            sys_timeout_add_to_task("sys_timer", 0, websockets_client_dialogue_timeout_exit, 1000);
            break;
        }
#ifdef CONFIG_LVGL_UI_ENABLE
        extern int music_buf_play_set_supspend_get(void);//语音设置的暂停模式
        if (music_buf_play_set_supspend_get() && (music_status == MUSIC_CLOSE || music_status == MUSIC_STOP)) {
            lv_demo_music_play_pause_button(1);//暂停图标显示
            //lv_demo_switch_to_music_page("net_music");
        } else if (!net_music_play_pause_status() && ai_speaker_app()) { //不是暂停就是停止，则回到主页面
            lv_demo_switch_to_main_page();
        }
#endif
    }

    return close_type;
}

static unsigned int sys_timeout_stop_dec_cnt = 0;
static unsigned int sys_timeout_power_off_dec_cnt = 0;
unsigned int sys_timeout_countdown_dec_cnt = 0;

#define TIME_CNT    10
unsigned int sys_timeout_countdown_time[TIME_CNT] = {0};
unsigned char sys_timeout_countdown_time_content[32] = {0};
unsigned int sys_timeout_stop_dec_cnt_id = 0;
unsigned int sys_timeout_countdown_dec_cnt_id = 0;

unsigned int sys_timeout_stop_cnt_get(void)
{
    return sys_timeout_stop_dec_cnt;
}

unsigned int sys_timeout_power_off_cnt_get(void)
{
    return sys_timeout_power_off_dec_cnt;
}

unsigned int sys_timeout_countdown_cnt_get(void)
{
    return sys_timeout_countdown_dec_cnt;
}

void sys_timeout_stop(void *p)
{
    if ((unsigned int)p == (unsigned int)&sys_timeout_stop_dec_cnt) {
        if (sys_timeout_stop_dec_cnt) {
            --sys_timeout_stop_dec_cnt;
            if (sys_timeout_stop_dec_cnt == 0) {
                sys_timer_del(sys_timeout_stop_dec_cnt_id);
#ifdef AI_UART_CMD_CTROL_ENABLE
                ai_uart_cmd_data_push(AI_UART_CMD_DIALUOGE_CLOSE, NULL, 0);
                ai_uart_cmd_data_push(AI_UART_CMD_MUSIC_STOP, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
                at_uart_cmd_send(AI_UART_CMD_DIALUOGE_CLOSE, NULL);
                at_uart_cmd_send(AI_UART_CMD_MUSIC_STOP, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
                play_face_emoji(AI_UART_CMD_DIALUOGE_CLOSE);
#endif
                if (ai_speaker_app()) {
                    net_music_play_loop_clear();
                    aisp_clear(1);
                } else {
                    aisp_app_suspend();
                }
            }
        }
    } else if (p) {
#ifdef AI_UART_CMD_CTROL_ENABLE
        ai_uart_cmd_data_push(AI_UART_CMD_DIALUOGE_CLOSE, NULL, 0);
        ai_uart_cmd_data_push(AI_UART_CMD_MUSIC_STOP, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
        at_uart_cmd_send(AI_UART_CMD_DIALUOGE_CLOSE, NULL);
        at_uart_cmd_send(AI_UART_CMD_MUSIC_STOP, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
        play_face_emoji(AI_UART_CMD_DIALUOGE_CLOSE);
#endif
        if (ai_speaker_app()) {
            net_music_play_loop_clear();
            aisp_clear(1);
        } else {
            aisp_app_suspend();
        }
    }
}

static void sys_timeout_power_off(void *p)
{
    if ((unsigned int)p == (unsigned int)&sys_timeout_power_off_dec_cnt) {
        if (sys_timeout_power_off_dec_cnt) {
            --sys_timeout_power_off_dec_cnt;
            if (sys_timeout_power_off_dec_cnt == 0) {
                struct intent it;
                struct application *app = get_current_app();
                if (app) {
                    init_intent(&it);
                    it.name = app->name;
                    it.action = ACTION_STOP;    //退出当前模式
                    start_app(&it);
                }
#ifdef AI_UART_CMD_CTROL_ENABLE
                ai_uart_cmd_data_push(AI_UART_CMD_PWR_OFF, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
                at_uart_cmd_send(AI_UART_CMD_PWR_OFF, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
                play_face_emoji(AI_UART_CMD_PWR_OFF);
#endif
                sys_power_poweroff();//关闭时候不提示
            }
        }
    } else if (p) {
        struct intent it;
        struct application *app = get_current_app();
        if (app) {
            init_intent(&it);
            it.name = app->name;
            it.action = ACTION_STOP;    //退出当前模式
            start_app(&it);
        }
#ifdef AI_UART_CMD_CTROL_ENABLE
        ai_uart_cmd_data_push(AI_UART_CMD_PWR_OFF, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
        at_uart_cmd_send(AI_UART_CMD_PWR_OFF, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
        play_face_emoji(AI_UART_CMD_PWR_OFF);
#endif
        sys_power_poweroff();//关闭时候不提示
    }
}

int __attribute__((weak)) timeout_countdown_callback(int remain_sec)
{
    return 0;
}

static char *timeout_countdown_content_sprintf(int sec)
{
    int h = sec / (60 * 60);
    int m = (sec % (60 * 60)) / 60;
    int s = sec % 60;
    char dat[16];
    memset(sys_timeout_countdown_time_content, 0, sizeof(sys_timeout_countdown_time_content));
    if (h) {
        snprintf(dat, sizeof(dat), "%d小时", h);
        strcat(sys_timeout_countdown_time_content, dat);
    }
    if (m) {
        snprintf(dat, sizeof(dat), "%d分钟", m);
        strcat(sys_timeout_countdown_time_content, dat);
    }
    if (s) {
        snprintf(dat, sizeof(dat), "%d秒", s);
        strcat(sys_timeout_countdown_time_content, dat);
    }
    strcat(sys_timeout_countdown_time_content, "计时时间到");
    printf("timeout : %s \n", sys_timeout_countdown_time_content);
    return sys_timeout_countdown_time_content;
}
void sys_timeout_countdown(void *p)//倒计时
{
    char sbuf[64];
    struct device_event event = {0};

    if ((unsigned int)p == (unsigned int)&sys_timeout_countdown_dec_cnt) {
        if (sys_timeout_countdown_dec_cnt) {
            --sys_timeout_countdown_dec_cnt;
            timeout_countdown_callback(sys_timeout_countdown_dec_cnt);
            if (sys_timeout_countdown_dec_cnt == 0) {
                sys_timer_del(sys_timeout_countdown_dec_cnt_id);
                struct key_event key = {0};
                aisp_app_suspend();
                event.event = DEVICE_EVENT_IN;
                event.value = timeout_countdown_content_sprintf(sys_timeout_countdown_time[0]);
                device_event_notify(DEVICE_EVENT_FROM_ALM, &event);
#ifdef AI_UART_CMD_CTROL_ENABLE
                sprintf(sbuf, "{\"time_ring\":%d}", sys_timeout_countdown_time[0]);
                ai_uart_cmd_data_push(AI_UART_CMD_TIME_COUNT_RING, sbuf, strlen(sbuf) + 1);
#endif
#ifdef AT_UART_CMD_ENABLE
                at_uart_cmd_send(AI_UART_CMD_TIME_COUNT_RING, sbuf);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
                play_face_emoji(AI_UART_CMD_TIME_COUNT_RING);
#endif
                sys_timeout_countdown_time[0] = 0;
            }
        }
    } else if (p && *(int *)p) {
        printf("time countdown end : %d sec\n", *(int*)p);
        struct key_event key = {0};
        timeout_countdown_callback(0);
        aisp_app_suspend();
        event.event = DEVICE_EVENT_IN;
        event.value = timeout_countdown_content_sprintf(*(int*)p);
        device_event_notify(DEVICE_EVENT_FROM_ALM, &event);
#ifdef AI_UART_CMD_CTROL_ENABLE
        sprintf(sbuf, "{\"time_ring\":%d}", *(int*)p);
        ai_uart_cmd_data_push(AI_UART_CMD_TIME_COUNT_RING, sbuf, strlen(sbuf) + 1);
#endif
#ifdef AT_UART_CMD_ENABLE
        at_uart_cmd_send(AI_UART_CMD_TIME_COUNT_RING, sbuf);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
        play_face_emoji(AI_UART_CMD_TIME_COUNT_RING);
#endif
        *(int*)p = 0;
    }
}

static int asr_time_min_get(char *start_str, int slen, float *hour)
{
    int i, j, k, n;
    int min_timeout = 0;
    int min_hltimeout = 0;
    int min_10 = 0;
    char fstr = 0;
    float h = 0;
    char *asr = start_str;
    char *find_1 = NULL;
    short *ch_utf8 = NULL;

    char utft2[3] = {0xE4, 0xB8, 0xA4}; //两
    char utft1to10[11][3] = {
        0x00, 0x00, 0x00, //
        0xE4, 0xB8, 0x80, //一
        0xE4, 0xBA, 0x8C, //二
        0xE4, 0xB8, 0x89, //三
        0xE5, 0x9B, 0x9B, //四
        0xE4, 0xBA, 0x94, //五
        0xE5, 0x85, 0xAD, //六
        0xE4, 0xB8, 0x83, //七
        0xE5, 0x85, 0xAB, //八
        0xE4, 0xB9, 0x9D, //九
        0xE5, 0x8D, 0x81, //十
    };
    if (asr) {
        //put_buf(asr, slen);
        j = slen;//你帮我在二十五分钟后关闭，你帮我在25分钟后关闭
        if (j == 1) { //1-9
            j--;
            if (asr[j] >= '0' && asr[j] <= '9') {
                min_timeout = asr[j] - '0';
                printf("单数字timeout : %d \n", min_timeout);
            }
        } else {
            k = j - 1;
            while (j > 0) {
                if (asr[k] >= '0' && asr[k] <= '9') {
                    while (--k) {
                        if ((asr[k] >= '0' && asr[k] <= '9')) {
                            continue;
                        }
                        if (asr[k] == '.') {
                            fstr = 1;
                            continue;
                        } else {
                            break;
                        }
                    }
                    find_1 = &asr[k];
                    if (fstr) {
                        h = atof(find_1);
                        if (h <= 0) {
                            h = 0;
                        } else if (hour) {
                            *hour = h;
                        }
                        min_timeout = (int)h;
                    } else {
                        min_timeout = atoi(find_1);
                        if (min_timeout < 0) {
                            min_timeout = 0;
                        }
                    }
                    printf("多数字timeout : %d \n", min_timeout);
                    break;
                } else {
                    int f = 0;
                    for (int n = 0; n < 11; n++) {
                        if (memcmp(&utft1to10[n][0], &asr[k - 2], 3) == 0 || memcmp(&utft2, &asr[k - 2], 3) == 0) {
                            if (memcmp(&utft2, &asr[k - 2], 3) == 0) {
                                f = 2;
                            } else {
                                f = n;
                            }
                            if ((min_hltimeout & 0xFF) == 0 && f < 10 && min_10 == 0) { //低八位=0
                                min_hltimeout |= f;
                            } else {
                                if (f == 10 && !min_10) { //低八位=0
                                    min_10 = 10;
                                } else if (f < 10) {
                                    if (((min_hltimeout >> 8) & 0xFF) == 0 && f < 10) { //高八位=0
                                        min_hltimeout |= (f << 8);
                                    }
                                }
                            }
                            break;
                        }
                    }
                    if (!f) { //不是一到十汉字
                        printf("->no found 一到十\n");
                        break;
                    }
                    j -= 3;
                    k -= 3;
                }
            }
            if (!min_timeout && (min_10 || min_hltimeout)) {
                min_timeout = min_hltimeout ? ((min_hltimeout >> 8) * 10 + (min_hltimeout & 0xFF)) : min_10;
                printf("汉字timeout : %d \n", min_timeout);
            }
        }
    }
    return min_timeout;
}

int sys_timeout_stop_off_callback(char *asr, int len)
{
    struct key_event key = {0};
    int music_status = 0;
    int close_type = 0;
    float hour = 0;
    int i, j, k;
    int min_timeout = 0;
    int timeout = 0;
    char is_time_countdown = 0;
    char *find = NULL;
    char src_str[128];

#ifdef CONFIG_ASR_POWER_OFF_ENABLE
    if (asr && strstr(asr, "关机") && strlen(asr) <= 9) {
        close_type = 1;//1立马关闭对话
        respon_ok(1);//好的
        int aisp_sys_power_off(void);
        sys_timeout_add_to_task("sys_timer", NULL, aisp_sys_power_off, 1000);
        return close_type;
    }
#endif
    for (i = 0; sys_countown_timeout_close[i] != NULL; i++) { //取消计时
        if (strstr(asr, sys_countown_timeout_close[i]) && strlen(asr) < (strlen(sys_countown_timeout_close[i]) + 3)) {
            sys_timeout_countdown_dec_cnt = 0;
            if (sys_timeout_countdown_dec_cnt_id) {
                sys_timer_del(sys_timeout_countdown_dec_cnt_id);
                sys_timeout_countdown_dec_cnt_id = 0;
            }
            for (int n = 0; n < TIME_CNT; n++) {
                sys_timeout_countdown_time[n] = 0;
            }
            memset(sys_timeout_countdown_time_content, 0, sizeof(sys_timeout_countdown_time_content));
            close_type = 1;//1立马关闭对话
            if (music_play_res_file("TimeCntDel.mp3")) {
                respon_ok(1);//好的
            }
            printf("del time countdown\n");
            return close_type;
        }
    }
    //printf("->asr = %s \n",asr);
    for (i = 0; sys_30min[i] != NULL; i++) { //关闭
        for (j = 0; sys_close[j] != NULL; j++) { //关闭
            sprintf(src_str, "%s%s", sys_30min[i], sys_close[j]);
            if (strstr(asr, src_str)) {
                min_timeout = 30;
                timeout = min_timeout * 60 * 1000;
                close_type = 1;//1立马关闭对话
                printf("->30min_timeout close \n");
                if (sys_timeout_stop_dec_cnt == 0) {
                    sys_timeout_stop_dec_cnt = timeout / 1000;
                    sys_timeout_stop_dec_cnt_id = sys_timer_add_to_task("sys_timer", &sys_timeout_stop_dec_cnt, sys_timeout_stop, 1000);
                } else {
                    sys_timeout_add_to_task("sys_timer", timeout, sys_timeout_stop, timeout);
                }
                goto exit;
            }
        }
    }
    for (i = 0; sys_30min[i] != NULL; i++) { //暂停音乐
        for (j = 0; sys_music_stop[j] != NULL; j++) { //暂停音乐
            sprintf(src_str, "%s%s", sys_30min[i], sys_music_stop[j]);
            if (strstr(asr, src_str)) {
                min_timeout = 30;
                timeout = min_timeout * 60 * 1000;
                close_type = 1;//1立马关闭对话
                printf("->30min_timeout close \n");
                if (sys_timeout_stop_dec_cnt == 0) {
                    sys_timeout_stop_dec_cnt = timeout / 1000;
                    sys_timeout_stop_dec_cnt_id = sys_timer_add_to_task("sys_timer", &sys_timeout_stop_dec_cnt, sys_timeout_stop, 1000);
                } else {
                    sys_timeout_add_to_task("sys_timer", timeout, sys_timeout_stop, timeout);
                }
                goto exit;
            }
        }
    }
    for (i = 0; sys_30min[i] != NULL; i++) { //关闭音乐
        for (j = 0; sys_music_close[j] != NULL; j++) { //关闭音乐
            sprintf(src_str, "%s%s", sys_30min[i], sys_music_close[j]);
            if (strstr(asr, src_str)) {
                min_timeout = 30;
                timeout = min_timeout * 60 * 1000;
                close_type = 1;//1立马关闭对话
                printf("->30min_timeout close \n");
                if (sys_timeout_stop_dec_cnt == 0) {
                    sys_timeout_stop_dec_cnt = timeout / 1000;
                    sys_timeout_stop_dec_cnt_id = sys_timer_add_to_task("sys_timer", &sys_timeout_stop_dec_cnt, sys_timeout_stop, 1000);
                } else {
                    sys_timeout_add_to_task("sys_timer", timeout, sys_timeout_stop, timeout);
                }
                goto exit;
            }
        }
    }
    for (i = 0; sys_30min[i] != NULL; i++) {
        for (j = 0; sys_str_power_off[j] != NULL; j++) { //关机
            sprintf(src_str, "%s%s", sys_30min[i], sys_str_power_off[j]);
            if (strstr(asr, src_str)) {
                min_timeout = 30;
                timeout = min_timeout * 60 * 1000;
                close_type = 1;//1立马关闭对话
                printf("->30min_timeout off \n");
                if (sys_timeout_power_off_dec_cnt == 0) {
                    sys_timeout_power_off_dec_cnt = timeout / 1000;
                    sys_timer_add_to_task("sys_timer", &sys_timeout_power_off_dec_cnt, sys_timeout_power_off, 1000);
                } else {
                    sys_timeout_add_to_task("sys_timer", timeout, sys_timeout_power_off, timeout);
                }
                goto exit;
            }
        }
    }
    for (i = 0; sys_90min[i] != NULL; i++) {
        for (j = 0; sys_close[j] != NULL; j++) { //关闭
            sprintf(src_str, "%s%s", sys_90min[i], sys_close[j]);
            if (strstr(asr, src_str)) {
                min_timeout = 90;
                timeout = min_timeout * 60 * 1000;
                close_type = 1;//1立马关闭对话
                printf("->90min_timeout close \n");
                if (sys_timeout_stop_dec_cnt == 0) {
                    sys_timeout_stop_dec_cnt = timeout / 1000;
                    sys_timeout_stop_dec_cnt_id = sys_timer_add_to_task("sys_timer", &sys_timeout_stop_dec_cnt, sys_timeout_stop, 1000);
                } else {
                    sys_timeout_add_to_task("sys_timer", timeout, sys_timeout_stop, timeout);
                }
                goto exit;
            }
        }
    }
    for (i = 0; sys_90min[i] != NULL; i++) {
        for (j = 0; sys_music_stop[j] != NULL; j++) { //关闭音乐
            sprintf(src_str, "%s%s", sys_90min[i], sys_music_stop[j]);
            if (strstr(asr, src_str)) {
                min_timeout = 90;
                timeout = min_timeout * 60 * 1000;
                close_type = 1;//1立马关闭对话
                printf("->90min_timeout close \n");
                if (sys_timeout_stop_dec_cnt == 0) {
                    sys_timeout_stop_dec_cnt = timeout / 1000;
                    sys_timeout_stop_dec_cnt_id = sys_timer_add_to_task("sys_timer", &sys_timeout_stop_dec_cnt, sys_timeout_stop, 1000);
                } else {
                    sys_timeout_add_to_task("sys_timer", timeout, sys_timeout_stop, timeout);
                }
                goto exit;
            }
        }
    }
    for (i = 0; sys_90min[i] != NULL; i++) {
        for (j = 0; sys_music_close[j] != NULL; j++) { //关闭音乐
            sprintf(src_str, "%s%s", sys_90min[i], sys_music_close[j]);
            if (strstr(asr, src_str)) {
                min_timeout = 90;
                timeout = min_timeout * 60 * 1000;
                close_type = 1;//1立马关闭对话
                printf("->90min_timeout close \n");
                if (sys_timeout_stop_dec_cnt == 0) {
                    sys_timeout_stop_dec_cnt = timeout / 1000;
                    sys_timeout_stop_dec_cnt_id = sys_timer_add_to_task("sys_timer", &sys_timeout_stop_dec_cnt, sys_timeout_stop, 1000);
                } else {
                    sys_timeout_add_to_task("sys_timer", timeout, sys_timeout_stop, timeout);
                }
                goto exit;
            }
        }
    }
    for (i = 0; sys_90min[i] != NULL; i++) {
        for (j = 0; sys_str_power_off[j] != NULL; j++) { //关机
            sprintf(src_str, "%s%s", sys_90min[i], sys_str_power_off[j]);
            if (strstr(asr, src_str)) {
                min_timeout = 90;
                timeout = min_timeout * 60 * 1000;
                close_type = 1;//1立马关闭对话
                printf("->90min_timeout off \n");
                if (sys_timeout_power_off_dec_cnt == 0) {
                    sys_timeout_power_off_dec_cnt = timeout / 1000;
                    sys_timer_add_to_task("sys_timer", &sys_timeout_power_off_dec_cnt, sys_timeout_power_off, 1000);
                } else {
                    sys_timeout_add_to_task("sys_timer", timeout, sys_timeout_power_off, timeout);
                }
                goto exit;
            }
        }
    }
    for (i = 0; sys_min_timeout[i] != NULL; i++) { //n分钟
        for (j = 0; sys_close[j] != NULL; j++) { //关闭
            sprintf(src_str, "%s%s", sys_min_timeout[i], sys_close[j]);
            //printf("-> min asr : %s, src_str: %s\n",asr,src_str);
            find = strstr(asr, src_str);
            if (find) {
                k = find - asr;
                min_timeout = asr_time_min_get(asr, k, &hour);
                if (min_timeout > 0 || hour > 0) {
                    if (hour > 0) {
                        if ((hour / 60) > 72) {
                            return 0;
                        }
                        timeout = hour * 60 * 1000;
                    } else {
                        if ((min_timeout / 60) > 72) {
                            return 0;
                        }
                        timeout = min_timeout * 60 * 1000;
                    }
                    close_type = 1;//1立马关闭对话
                    printf("->min_timeout close : %d %0.2f %d\n", min_timeout, hour, timeout);
                    if (sys_timeout_stop_dec_cnt == 0) {
                        sys_timeout_stop_dec_cnt = timeout / 1000;
                        sys_timeout_stop_dec_cnt_id = sys_timer_add_to_task("sys_timer", &sys_timeout_stop_dec_cnt, sys_timeout_stop, 1000);
                    } else {
                        sys_timeout_add_to_task("sys_timer", timeout, sys_timeout_stop, timeout);
                    }
                    goto exit;
                }
            }
        }
    }
    for (i = 0; sys_min_timeout[i] != NULL; i++) { //n分钟
        for (j = 0; sys_music_stop[j] != NULL; j++) { //关闭音乐
            sprintf(src_str, "%s%s", sys_min_timeout[i], sys_music_stop[j]);
            //printf("-> min asr : %s, src_str: %s\n",asr,src_str);
            find = strstr(asr, src_str);
            if (find) {
                k = find - asr;
                min_timeout = asr_time_min_get(asr, k, &hour);
                if (min_timeout > 0 || hour > 0) {
                    if (hour > 0) {
                        if ((hour / 60) > 72) {
                            return 0;
                        }
                        timeout = hour * 60 * 1000;
                    } else {
                        if ((min_timeout / 60) > 72) {
                            return 0;
                        }
                        timeout = min_timeout * 60 * 1000;
                    }
                    close_type = 1;//1立马关闭对话
                    printf("->min_timeout close : %d %0.2f %d\n", min_timeout, hour, timeout);
                    if (sys_timeout_stop_dec_cnt == 0) {
                        sys_timeout_stop_dec_cnt = timeout / 1000;
                        sys_timeout_stop_dec_cnt_id = sys_timer_add_to_task("sys_timer", &sys_timeout_stop_dec_cnt, sys_timeout_stop, 1000);
                    } else {
                        sys_timeout_add_to_task("sys_timer", timeout, sys_timeout_stop, timeout);
                    }
                    goto exit;
                }
            }
        }
    }
    for (i = 0; sys_min_timeout[i] != NULL; i++) { //n分钟
        for (j = 0; sys_music_close[j] != NULL; j++) { //关闭音乐
            sprintf(src_str, "%s%s", sys_min_timeout[i], sys_music_close[j]);
            //printf("-> min asr : %s, src_str: %s\n",asr,src_str);
            find = strstr(asr, src_str);
            if (find) {
                k = find - asr;
                min_timeout = asr_time_min_get(asr, k, &hour);
                if (min_timeout > 0 || hour > 0) {
                    if (hour > 0) {
                        if ((hour / 60) > 72) {
                            return 0;
                        }
                        timeout = hour * 60 * 1000;
                    } else {
                        if ((min_timeout / 60) > 72) {
                            return 0;
                        }
                        timeout = min_timeout * 60 * 1000;
                    }
                    close_type = 1;//1立马关闭对话
                    printf("->min_timeout close : %d %0.2f %d\n", min_timeout, hour, timeout);
                    if (sys_timeout_stop_dec_cnt == 0) {
                        sys_timeout_stop_dec_cnt = timeout / 1000;
                        sys_timeout_stop_dec_cnt_id = sys_timer_add_to_task("sys_timer", &sys_timeout_stop_dec_cnt, sys_timeout_stop, 1000);
                    } else {
                        sys_timeout_add_to_task("sys_timer", timeout, sys_timeout_stop, timeout);
                    }
                    goto exit;
                }
            }
        }
    }
    for (i = 0; sys_min_timeout[i] != NULL; i++) { //n分钟
        for (j = 0; sys_str_power_off[j] != NULL; j++) { //关机
            sprintf(src_str, "%s%s", sys_min_timeout[i], sys_str_power_off[j]);
            //printf("-> hour asr : %s, src_str: %s\n",asr,src_str);
            find = strstr(asr, src_str);
            if (find) {
                k = find - asr;
                min_timeout = asr_time_min_get(asr, k, &hour);
                if (min_timeout > 0 || hour > 0) {
                    if (hour > 0) {
                        if ((hour / 60) > 72) {
                            return 0;
                        }
                        timeout = hour * 60 * 1000;
                    } else {
                        if ((min_timeout / 60) > 72) {
                            return 0;
                        }
                        timeout = min_timeout * 60 * 1000;
                    }
                    close_type = 1;//1立马关闭对话
                    printf("->min_timeout off : %d %0.2f %d\n", min_timeout, hour, timeout);
                    if (sys_timeout_power_off_dec_cnt == 0) {
                        sys_timeout_power_off_dec_cnt = timeout / 1000;
                        sys_timer_add_to_task("sys_timer", &sys_timeout_power_off_dec_cnt, sys_timeout_power_off, 1000);
                    } else {
                        sys_timeout_add_to_task("sys_timer", timeout, sys_timeout_power_off, timeout);
                    }
                    goto exit;
                }
            }
        }
    }
    for (i = 0; sys_hour_timeout[i] != NULL; i++) { //n小时
        for (j = 0; sys_close[j] != NULL; j++) { //关闭
            sprintf(src_str, "%s%s", sys_hour_timeout[i], sys_close[j]);
            //printf("-> hour asr : %s, src_str: %s\n",asr,src_str);
            find = strstr(asr, src_str);
            if (find) {
                k = find - asr;
                min_timeout = asr_time_min_get(asr, k, &hour);
                if (min_timeout > 0 || hour > 0) {
                    if (hour > 0) {
                        if (hour > 72) {
                            return 0;
                        }
                        timeout = hour * 60 * 60 * 1000;
                    } else {
                        if (min_timeout > 72) {
                            return 0;
                        }
                        timeout = min_timeout * 60 * 60 * 1000;
                    }
                    close_type = 1;//1立马关闭对话
                    printf("->hour_timeout close : %d %0.2f %d\n", min_timeout, hour, timeout);
                    if (sys_timeout_stop_dec_cnt == 0) {
                        sys_timeout_stop_dec_cnt = timeout / 1000;
                        sys_timeout_stop_dec_cnt_id = sys_timer_add_to_task("sys_timer", &sys_timeout_stop_dec_cnt, sys_timeout_stop, 1000);
                    } else {
                        sys_timeout_add_to_task("sys_timer", timeout, sys_timeout_stop, timeout);
                    }
                    goto exit;
                }
            }
        }
    }
    for (i = 0; sys_hour_timeout[i] != NULL; i++) { //n小时
        for (j = 0; sys_music_stop[j] != NULL; j++) { //关闭
            sprintf(src_str, "%s%s", sys_hour_timeout[i], sys_music_stop[j]);
            //printf("-> hour asr : %s, src_str: %s\n",asr,src_str);
            find = strstr(asr, src_str);
            if (find) {
                k = find - asr;
                min_timeout = asr_time_min_get(asr, k, &hour);
                if (min_timeout > 0 || hour > 0) {
                    if (hour > 0) {
                        if (hour > 72) {
                            return 0;
                        }
                        timeout = hour * 60 * 60 * 1000;
                    } else {
                        if (min_timeout > 72) {
                            return 0;
                        }
                        timeout = min_timeout * 60 * 60 * 1000;
                    }
                    close_type = 1;//1立马关闭对话
                    printf("->hour_timeout close : %d %0.2f %d\n", min_timeout, hour, timeout);
                    if (sys_timeout_stop_dec_cnt == 0) {
                        sys_timeout_stop_dec_cnt = timeout / 1000;
                        sys_timeout_stop_dec_cnt_id = sys_timer_add_to_task("sys_timer", &sys_timeout_stop_dec_cnt, sys_timeout_stop, 1000);
                    } else {
                        sys_timeout_add_to_task("sys_timer", timeout, sys_timeout_stop, timeout);
                    }
                    goto exit;
                }
            }
        }
    }
    for (i = 0; sys_hour_timeout[i] != NULL; i++) { //n小时
        for (j = 0; sys_music_close[j] != NULL; j++) { //关闭
            sprintf(src_str, "%s%s", sys_hour_timeout[i], sys_music_close[j]);
            //printf("-> hour asr : %s, src_str: %s\n",asr,src_str);
            find = strstr(asr, src_str);
            if (find) {
                k = find - asr;
                min_timeout = asr_time_min_get(asr, k, &hour);
                if (min_timeout > 0 || hour > 0) {
                    if (hour > 0) {
                        if (hour > 72) {
                            return 0;
                        }
                        timeout = hour * 60 * 60 * 1000;
                    } else {
                        if (min_timeout > 72) {
                            return 0;
                        }
                        timeout = min_timeout * 60 * 60 * 1000;
                    }
                    close_type = 1;//1立马关闭对话
                    printf("->hour_timeout close : %d %0.2f %d\n", min_timeout, hour, timeout);
                    if (sys_timeout_stop_dec_cnt == 0) {
                        sys_timeout_stop_dec_cnt = timeout / 1000;
                        sys_timeout_stop_dec_cnt_id = sys_timer_add_to_task("sys_timer", &sys_timeout_stop_dec_cnt, sys_timeout_stop, 1000);
                    } else {
                        sys_timeout_add_to_task("sys_timer", timeout, sys_timeout_stop, timeout);
                    }
                    goto exit;
                }
            }
        }
    }
    for (i = 0; sys_hour_timeout[i] != NULL; i++) { //n小时
        for (j = 0; sys_str_power_off[j] != NULL; j++) { //关机
            sprintf(src_str, "%s%s", sys_hour_timeout[i], sys_str_power_off[j]);
            //printf("-> hour asr : %s, src_str: %s\n",asr,src_str);
            find = strstr(asr, src_str);
            if (find) {
                k = find - asr;
                min_timeout = asr_time_min_get(asr, k, &hour);
                if (min_timeout > 0 || hour > 0) {
                    if (hour > 0) {
                        timeout = hour * 60 * 60 * 1000;
                        if (hour > 72) {
                            return 0;
                        }
                    } else {
                        if (min_timeout > 72) {
                            return 0;
                        }
                        timeout = min_timeout * 60 * 60 * 1000;
                    }
                    close_type = 1;//1立马关闭对话
                    printf("->hour_timeout off : %d %0.2f %d\n", min_timeout, hour, timeout);
                    if (sys_timeout_power_off_dec_cnt == 0) {
                        sys_timeout_power_off_dec_cnt = timeout / 1000;
                        sys_timer_add_to_task("sys_timer", &sys_timeout_power_off_dec_cnt, sys_timeout_power_off, 1000);
                    } else {
                        sys_timeout_add_to_task("sys_timer", timeout, sys_timeout_power_off, timeout);
                    }
                    goto exit;
                }
            }
        }
    }

    for (i = 0; sys_countown_timeout[i] != NULL; i++) { //计时三十五分钟，计时三十五秒，计时三十五小时，
        find = strstr(asr, sys_countown_timeout[i]);
        if (find) {
            find += strlen(sys_countown_timeout[i]);
            k = 0;
            char sec_mode = 0;
            char *end = strstr(find, "小时");
            if (end) {
                k = end - find;
                sec_mode = 0;
            } else {
                end = strstr(find, "分钟");
                if (end) {
                    k = end - find;
                    sec_mode = 1;
                } else {
                    end = strstr(find, "秒");
                    if (end) {
                        k = end - find;
                        sec_mode = 2;
                    }
                }
            }
            if (k > 0 && k < 10) {
                min_timeout = asr_time_min_get(find, k, &hour);
                if (min_timeout > 0 || hour > 0) {
                    if (hour > 0 && sec_mode == 0) {
                        timeout = hour * 60 * 60 * 1000;
                        if (hour > 72) {
                            return 0;
                        }
                    } else {
                        if (sec_mode == 2) {
                            timeout = min_timeout * 1000;
                        } else if (sec_mode == 1) {
                            timeout = min_timeout * 60 * 1000;
                        } else if (sec_mode == 0) {
                            if (min_timeout > 72) {
                                return 0;
                            }
                            timeout = min_timeout * 60 * 60 * 1000;
                        }
                    }
                    close_type = 1;//1立马关闭对话
                    printf("->timeout countdown : %d sec\n", timeout / 1000);
#ifdef AI_UART_CMD_CTROL_ENABLE
                    char sbuf[64];
                    sprintf(sbuf, "{\"time_count_down\":%d}", timeout / 1000);
                    ai_uart_cmd_data_push(AI_UART_CMD_TIME_COUNT_DOWN, sbuf, strlen(sbuf) + 1);
#endif
#ifdef AT_UART_CMD_ENABLE
                    char sbuf[64];
                    sprintf(sbuf, "{\"time_count_down\":%d}", timeout / 1000);
                    at_uart_cmd_send(AI_UART_CMD_TIME_COUNT_RING, sbuf);
#endif
                    if (sys_timeout_countdown_dec_cnt == 0) {
                        sys_timeout_countdown_dec_cnt = timeout / 1000;//
                        sys_timeout_countdown_time[0] = sys_timeout_countdown_dec_cnt;
                        sys_timeout_countdown_dec_cnt_id = sys_timer_add_to_task("sys_timer", &sys_timeout_countdown_dec_cnt, sys_timeout_countdown, 1000);
                    } else {
                        int i;
                        for (i = 1; i < TIME_CNT; i++) {
                            if (sys_timeout_countdown_time[i] == 0) {
                                break;
                            }
                        }
                        if (i >= TIME_CNT) {
                            i = TIME_CNT - 1;
                        }
                        sys_timeout_countdown_time[i] = timeout / 1000;
                        sys_timeout_add_to_task("sys_timer", &sys_timeout_countdown_time[i], sys_timeout_countdown, timeout);
                    }
#ifdef CONFIG_LVGL_UI_ENABLE
                    lv_demo_timer_add_sec(timeout / 1000);
                    lv_demo_switch_to_timer_page();
#endif
                    is_time_countdown = true;
                    goto exit;
                }
            }
        }
    }

    return 0;
exit:
    if (is_time_countdown) {
        if (!sys_connect_net_success()) {
            if (music_play_res_file("TimeCntStar.mp3")) {
                respon_ok(1);//好的
            }
        } else {
#ifdef CONFIG_USE_TTS_REPLY_ENABLE
            char *content = "好的，计时开始";
            http_tts_request(content, strlen(content) + 1);
#else
            if (music_play_res_file("TimeCntStar.mp3")) {
                respon_ok(1);//好的
            }
#endif
        }
    } else {
        respon_ok(1);//好的
    }

#ifdef CONFIG_LVGL_UI_ENABLE
    if (!is_time_countdown) {
        lv_demo_switch_to_main_page();
    }
#endif
    return close_type;
}

int user_instruction_word_callback(char *word, char *asr)
{
    int ret;
#ifdef IRARC_UART_ENABLE
    int air_conditioner_instruction_word_callback(char *word, char *asr);
    ret = air_conditioner_instruction_word_callback(word, asr);
    if (ret) {
        return ret;
    }
#endif
    return 0;
}

int net_music_time_off(int sec)
{
    if (sec) {
        sys_timeout_add_to_task("sys_timer", 0, net_music_play_set_stop, sec * 1000);
    }
    return 0;
}
