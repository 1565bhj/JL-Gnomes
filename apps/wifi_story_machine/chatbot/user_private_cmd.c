#include "system/includes.h"
#include "server/audio_server.h"
#include "server/server_core.h"
#include "syscfg/syscfg_id.h"
#include "web_socket/websocket_api.h"
#include "wifi/wifi_connect.h"
#include "event/key_event.h"
#include "os/os_api.h"
#include "fs/fs.h"
#include "json_c/json.h"
#include "json_c/json_tokener.h"
#include "app_config.h"
#include "ai_uart_ctrol.h"

int tm_instruction_word_callback(char *word, char *asr);
int ai_uart_cmd_data_push(unsigned char data_type, char *buf, int len);
void alarm_time_read(void *buf, int size);
int alarm_time_write(long sec, char cyc);//sec：ntp时间戳，cyc：每天闹钟使能
unsigned int sys_timeout_stop_cnt_get(void);//获取第一个关闭播放和对话的时间倒计时
unsigned int sys_timeout_power_off_cnt_get(void);//获取最新的关机闹钟
unsigned int sys_timeout_countdown_cnt_get(void);//获取第一个倒计时
static bool lte_pai_code = false;
static char pai_code[32] = {0};

/*
    music_buf_play_stop_staus();//返回true则是没有播放对话和音乐
    music_play_stop_status();//返回true则是没有播放提示音
    net_music_play_start_status();//返回true则是正在播放音乐
    if(music_buf_play_stop_staus() && music_play_stop_status() && !net_music_play_start_status()){//完全没有播放判断条件

    }
*/
static u16 switch_to_main_time_id = 0;
static char user_close_status = 0;
static const char *user_sys_close[] = {
    "再见", "拜拜", "我要休息了", "我想休息了", "我要睡觉了",
    "我想睡觉了", "我知道了", "我知道啦", "我明白了", "我明白啦",
    "你退下", "你退下吧", "退出对话", "关闭对话", "关闭聊天", "闭嘴", "你闭嘴", "闭嘴吧", "停", "你停下", "你停一下",
    "停止聊天", "停止对话", "结束对话", "不想和你说了", "不想和你说话了", "暂停对话", "你先休息一会",
    "休息一会", "你先暂停", "休息", "休息一会吧", "你先休息一会吧",
    "好的", "明白", "OK", "ok", NULL,
};
static const char *user_sleep[] = {
    "休息了", "休息啦", "睡觉了", "睡觉啦", "去睡了", "去睡啦", "先睡了", "先睡啦", "晚安", "困了", "困啦", "好困", "先歇着了", "先歇着啦",
    "睡觉觉",
    NULL,
};
static const char *user_music_sleep[] = {
    "接下来30分钟助眠白噪音，祝你好梦",
    "接下来30分钟助眠白噪音，晚安",
    "接下来的30分钟，只有安静的声音陪你，晚安，好梦。",
    "今天辛苦了，接下来的30分钟，只有安静的声音陪你，晚安",
    "辛苦了吧，接下来的30分钟睡眠白噪音只属于你，祝你好梦。",
    "接下来的30分钟，在一片安静的森林、海上，听水流过，晚安",
    NULL,
};

static int user_sys_tts_play_end_check(void)
{
    int http_tts_is_playing(void);
    void sys_timeout_stop(void *p);
    if (!http_tts_is_playing()) {
        net_music_play_next("musicians_sleep");
        sys_timeout_add_to_task("sys_timer", 1, sys_timeout_stop, 30 * 60 * 1000);//30分钟白噪音播放时间
    } else {
        sys_timeout_add_to_task("sys_timer", NULL, user_sys_tts_play_end_check, 1 * 1000);
    }
    return 0;
}
static int user_sys_enter_sleep(void)
{
    static char tts[512];
    net_music_play_set_stop();
    strncpy(tts, user_music_sleep[rand() % 6], sizeof(tts));
    http_tts_request(tts, strlen(tts) + 1);
    sys_timeout_add_to_task("sys_timer", NULL, user_sys_tts_play_end_check, 1 * 1000);
    return 0;
}

//获取设备4G配对码
char *user_sys_get_lte_pai_code(void)
{
    return pai_code;
}

//清空4G配对码数据
void user_sys_clear_lte_pai_code(void)
{
    memset(pai_code, 0, sizeof(pai_code));
}

static int user_sys_close_check(char *word)
{
    int app_status = 0;
    int i;
    user_close_status = 0;
    for (i = 0; user_sleep[i] != NULL; i++) { //AI模式
        if (strstr(word, user_sleep[i]) && strlen(word) <= (strlen(user_sleep[i]) + 9)) {
            user_close_status = 1;//AI对话时候结束连续对话
            return 0;//立马结束对话
        }
    }
    for (i = 0; user_sys_close[i] != NULL; i++) { //AI模式
        if (strstr(word, user_sys_close[i]) && strlen(word) <= (strlen(user_sys_close[i]) + 6)) {
            user_close_status = 2;//AI对话时候结束连续对话
            return 0;
        }
    }
    return 0;
}

static int user_emoji_check(char *buf, int len)
{
    unsigned char emoji_head[2]  = {0xf0, 0x9f};
    unsigned short emoji_start =  0x0;//0x8c80;//{0x8c, 0x80};
    unsigned short emoji_end   = 0xFFFF;//0x998f;//{0x99, 0x8f};
    unsigned char *p = (unsigned char *)buf;

    while (*p != 0) {
        if (*p == emoji_head[0] && *(p + 1) == emoji_head[1]) {
            unsigned short id = (p[2] << 8) | p[3];
            printf("-> find emoji id = 0x%x\n", id);
            //put_buf(p, 4);
            if (emoji_start <= id && id <= emoji_end) {
                memmove(p, p + 4, len - ((int)p + 4 - (int)buf));
                p += 3;
            }
        }
        p++;
    }
    return strlen(buf) + 1;
}


//对话超时退出
void aisp_timeout_callback(void)
{
    printf("=====aisp_timeout_callback=====");
#ifdef AI_UART_CMD_CTROL_ENABLE
    ai_uart_cmd_data_push(AI_UART_CMD_DIALUOGE_CLOSE, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
    at_uart_cmd_send(AI_UART_CMD_DIALUOGE_CLOSE, NULL);
#endif
#if (defined CONFIG_LVGL_UI_ENABLE)
    int lv_demo_switch_to_main_page(void);
    int sec = audio_app_mode_check() == 1 ? 2 : 1;
    if (switch_to_main_time_id) {
        sys_timer_modify(switch_to_main_time_id, sec * 1000);
    } else if (!websockets_recv_net_music_valid() && ai_speaker_app()) {
        sys_timeout_add_to_task("sys_timer", NULL, lv_demo_switch_to_main_page, sec * 1000);
    }
#endif
}

//定制唤醒词更改，返回值：0唤醒词， 1配网模式
int aisp_wake_world_callback(int index)
{
    return 0;
}

//status = 0：小飞小飞唤醒，status = 8 ： 配网模式
int aisp_wake_callback(int status)
{
    switch (status) { //8配网模式，不需要处理
    case 0: //小飞小飞 ,返回0-继续对话，返回非0则需要重新唤醒
        break;
    case 1: //打开灯光
        break;
    case 2: //切换灯光
        break;
    case 3: //关闭灯光
        break;
    case 4: //打开香薰
        break;
    case 5: //关闭香薰
        break;
    case 6: //显示湿度
        break;
    case 7: //显示空气指数
        break;
    case 10: //调高音量
        break;
    case 11: //调低音量
        break;
    }
    return 0;//返回0-继续对话，返回非0则需要重新唤醒
}

static void net_ai_dialogue_txt_rm_spaces(char *str)
{
    int i = 0, j = 0;
    while (str[i]) {
        if (str[i] != ' ') {
            str[j++] = str[i];
        }
        i++;
    }
    str[j] = '\0';
}
#if (defined CONFIG_LVGL_UI_ENABLE)
static void net_ai_dialogue_resum_page(void *p)
{
    switch_to_main_time_id = 0;
    char id = audio_app_mode_check();
    id = id < 0 ? 0 : id;
    char *app[] = {"ai_speaker", "bt_music", "sd_music", "usbdisk_music", "aux_music"};
    if (id == 0) {
        lv_demo_switch_to_main_page();
    } else {
        lv_demo_switch_to_music_page(app[id]);
    }
}
#endif

//网络对话文本回调函数
int net_ai_dialogue_txt_recv_callback(char *buf, int len)
{
    //printf("===resp_txt : %s \n", buf);
    //4G配对码提取
    if (lte_pai_code) {
        memcpy(pai_code, buf, len);
        lte_pai_code = false;
        net_ai_dialogue_txt_rm_spaces(pai_code);
        printf("====device is not activation!!! pair_code====: %s", pai_code);
    }
    if (!lte_pai_code && strstr(buf, "配对码为")) {
        lte_pai_code = true;
    }

    //put_buf(buf, len);
#ifdef AI_UART_CMD_CTROL_ENABLE
    ai_uart_cmd_data_push(AI_UART_CMD_TTS, buf, len);
    int emoji_tts_uart_cmd_callback(char *emoji);
    emoji_tts_uart_cmd_callback(buf);
#endif
#ifdef AT_UART_CMD_ENABLE
    at_uart_cmd_send(AI_UART_CMD_TTS, buf);
#endif

    user_emoji_check(buf, len);//UI显示的在这行下面处理（已去掉表情图标）
#ifdef CONFIG_UI_ENABLE
#if (defined CONFIG_LVGL_UI_ENABLE)
    lv_demo_ai_dialogue_tts(buf, len);
    if (switch_to_main_time_id) {
        sys_timer_modify(switch_to_main_time_id, 30 * 1000);
    } else if (!websockets_recv_net_music_valid()) {
        switch_to_main_time_id = sys_timeout_add_to_task("sys_timer", NULL, net_ai_dialogue_resum_page, 30 * 1000);
    }
#endif
#if (defined CONFIG_UI_PLAY_EMOJI)
    extern int emoji_tts_callback(char *emoji);
    emoji_tts_callback(buf);
#endif
#endif
    return 0;
}

//网络语音识别文本回调函数
int net_ai_asr_txt_recv_callback(char *buf, int len)
{
    int ret = 0;
    while (*buf == ' ') {
        buf++;
    }
#ifdef AI_UART_CMD_CTROL_ENABLE
    ai_uart_cmd_data_push(AI_UART_CMD_STT, buf, len);
#endif
#ifdef AT_UART_CMD_ENABLE
    at_uart_cmd_send(AI_UART_CMD_STT, buf);
#endif
#if (defined CONFIG_UI_ENABLE && defined CONFIG_LVGL_UI_ENABLE)
    lv_demo_ai_dialogue_stt(buf, len);
    if (switch_to_main_time_id) {
        sys_timer_modify(switch_to_main_time_id, 30 * 1000);
    } else if (!websockets_recv_net_music_valid()) {
        switch_to_main_time_id = sys_timeout_add_to_task("sys_timer", NULL, net_ai_dialogue_resum_page, 30 * 1000);
    }
#endif

#ifdef CONFIG_QYAI_MUTILMODAL_ENABLE
    ret = qyai_text2img(buf);
    if (ret) {
        return ret;
    }
#endif

    ret = user_sys_close_check(buf);
    if (ret) {
        return ret;
    }
    ret = sys_instruction_word_callback(buf, NULL);
    if (ret) {
        return ret;
    }
    //定时关闭和定时停止播放：1小时后关闭，1小时后关闭音乐
    ret = sys_timeout_stop_off_callback(buf, len);
    if (ret) {
        return ret;
    }
#if (defined TCFG_LED_PWM0_PORT || defined TCFG_PWM1_PORT)
    ret = tm_instruction_word_callback(buf, NULL);
    if (ret) {
        return ret;
    }
#endif

#if (defined CONFIG_UI_ENABLE && defined CONFIG_LVGL_UI_ENABLE)
    ret = sys_alarm_look_callback(buf);
    if (ret) {
        return ret;
    }
#endif

#ifdef USED_TM1629_SHOWN
    int tm_1629_time_shown(char *word);
    ret = tm_1629_time_shown(buf);
    if (ret) {
        return ret;
    }
#endif
    printf("asr_txt : %s \n", buf);
    if (!strcmp("模式切换", buf)) { //"模式切换"用户自定义私有命令检测
        return 1;//返回1立刻结束对话，返回0则走大模型对话
    }
    return 0;
}

int ai_audio_play_start_callback(void)
{
#ifdef AI_UART_CMD_CTROL_ENABLE
    ai_uart_cmd_data_push(AI_UART_CMD_DIALUOGE_PLAY_START, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
    at_uart_cmd_send(AI_UART_CMD_DIALUOGE_PLAY_START, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
    play_face_emoji(AI_UART_CMD_DIALUOGE_PLAY_START);
#endif
    return 0;
}

int ai_audio_play_end_callback(void)
{
    //printf("=====ai_audio_play_end_callback=====");
    if (user_close_status) {
        extern int websockets_dialogue_timeout_init(int time_out, char use_voice_note);
        websockets_dialogue_timeout_init(0, 0);//立马退出对话
#ifdef CONFIG_LVGL_UI_ENABLE
        sys_timeout_add_to_task("sys_timer", NULL, net_ai_dialogue_resum_page, 1 * 1000);
#endif
        if (user_close_status == 1) {
            sys_timeout_add_to_task("sys_timer", NULL, user_sys_enter_sleep, 500);
        }
    }
#ifdef AI_UART_CMD_CTROL_ENABLE
    ai_uart_cmd_data_push(AI_UART_CMD_DIALUOGE_PLAY_END, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
    at_uart_cmd_send(AI_UART_CMD_DIALUOGE_PLAY_END, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
    play_face_emoji(AI_UART_CMD_EMOJI_DATA /*AI_UART_CMD_DIALUOGE_PLAY_END*/);
#endif
#ifdef CONFIG_LVGL_UI_ENABLE
    if (!user_close_status) {
        lv_demo_ai_dialogue_speak_listen(1);
        if (switch_to_main_time_id) {
            sys_timer_modify(switch_to_main_time_id, 30 * 1000);
        } else if (!websockets_recv_net_music_valid()) {
            switch_to_main_time_id = sys_timeout_add_to_task("sys_timer", NULL, net_ai_dialogue_resum_page, 30 * 1000);
        }
    }
#endif
    user_close_status = 0;
    return 0;
}

//start 1开始录音， 0结束录音
int aisp_record_callbak(int start)
{
#ifdef AI_UART_CMD_CTROL_ENABLE
    ai_uart_cmd_data_push(start ? AI_UART_CMD_REC_START : AI_UART_CMD_REC_END, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
    at_uart_cmd_send(start ? AI_UART_CMD_REC_START : AI_UART_CMD_REC_END, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
    play_face_emoji(start ? AI_UART_CMD_REC_START : AI_UART_CMD_REC_END);
#endif
    return 0;
}

//预留的AI对话的表情和系统命令
int net_ai_dialogue_emoji(char *buf, int size)
{
#ifdef CONFIG_UI_PLAY_EMOJI
    emoji_tts_callback(buf);
#endif
    return 0;
}
//预留的AI对话的表达回调
int net_ai_dialogue_expression(char *buf, int size)
{
    printf("dialogue_expression : %s \n", buf);
    return 0;
}
int net_ai_dialogue_cmd(char *buf, int size)
{
    int ret = sys_instruction_word_callback(buf, NULL);
    if (ret) {
        return ret;
    }
    return 0;
}
