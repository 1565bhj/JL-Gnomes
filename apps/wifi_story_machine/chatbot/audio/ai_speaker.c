#include "system/app_core.h"
#include "system/includes.h"
#include "asm/system_reset_reason.h"
#include "os/os_api.h"
#include "event/device_event.h"
#include "app_config.h"
#include "action.h"
#include "event/key_event.h"
#include "event/net_event.h"
#include "wifi/wifi_connect.h"
#include "btstack/avctp_user.h"
#include "event/bt_event.h"
#include "lwip/netdb.h"
#include "server/ai_server.h"
#include "ai_uart_ctrol.h"
#include "qyai_config.h"

#if defined CONFIG_ASR_ALGORITHM_ENABLE

#ifndef VOLUME_STEP
#define VOLUME_STEP 10
#endif

//0:小飞小飞，8：配网模式
extern void aisp_wake(char index);
extern void key_vad_pcm_send_set_status(char start, char noice_not);
extern void sys_power_poweroff(void);
extern int music_play_waite(void);
extern int net_music_dec_stop(void);
extern int is_production_test_enter(char wake);
extern int sys_net_channel_read(void);
extern int aisp_open(u16 sample_rate);
extern void aisp_suspend(void);
extern void aisp_resume(void);
extern int aisp_close(void);

extern void wifi_sta_mode_info(char **ssid, char **pwd);
extern int music_buf_play_set_volume(int volume);
extern int music_buf_play_set_volume_step(int step);
extern int music_buf_play_supspend(void);
extern int music_buf_play_set_supspend_get(void);
extern int music_play_waite(void);

extern void net_music_play_resum(void);
extern int net_music_play_last(void);
extern int net_music_play_last_check(void);
extern int net_music_play_next(char *priv);

extern int music_play_set_volume(int volume);
extern int music_play_set_volume_step(int step);

extern int net_music_set_dec_volume_step(int step);
extern int net_music_set_dec_volume(int volume);
extern void wifi_sta_reconnect(void);
extern void pcm_send_set_status(char start);
extern int music_buf_play_stop_status(void);
extern int music_play_stop_status(void);
extern int music_play_hello(char index);
extern int music_play_res_file(const char *name);
extern void ble_cfg_net_result_notify(int event);
extern int aisp_mic_gain_set(unsigned char volume);
extern void aisp_clear(char stop_net_music);
extern int tm_1629_time_shown_level_low_power(char enter_low_pwr);
extern void dac_mute_control(char enable, char force);
extern int sys_net_channel_read(void);

extern int wifi_is_init(void);
extern int wifi_is_online(void);
extern void wifi_and_network_on(void);
extern void wifi_and_network_off(void);

#ifdef CONFIG_LTE_PHY_ENABLE
extern int lte_power_control(void *on);
extern int app_usb_at_cmd_init(u8 usb_id_num, void (*lte_online_callback)(void));
extern int app_usb_at_cmd_deinit(void);
extern int app_usb_at_cmd_send(u8 trig_type, u8 opt_code, u8 param);
extern int app_usb_at_cmd_send_reset(void);
#if (CONFIG_LTE_VENDOR==VENDOR_GT108)
extern int lte_dev_info_opt(u8 opt_code, char *iccid_buf_0, char *iccid_buf_1, char *wifi_mac_buf, char *imei_buf, char *card_id);
static u8 card_id = 0;
#endif

extern u8 IPV4_ADDR_CONFLICT_DETECT;
static u8 net_work_on = NET_WORK_ON_NONE;
#endif

//产测模式
void enter_product_func_test(void *priv);

extern char *user_sys_get_lte_pai_code(void);
extern void user_sys_clear_lte_pai_code(void);
extern int http_wifi_device_bind_app(char *phone);
extern int http_lte_device_bind_app(char *code, char *phone, char *imei);
extern int http_device_unbind_app(void);

//语音检测最小能量
const int speech_energy_min = SPEECH_ENERGY_MIN;
//数字增益是否使能
char speech_digital_vol_agc = DIGITAL_VOL_AGC;

volatile int aisp_aec_gain = 0;
volatile int aisp_mic0_gain = 0;
volatile int aisp_mic1_gain = 0;
volatile int keyword_start = 0;
volatile int keyword_wifi_enter_congfig = 0;

//usb hotplug state
int usb_hp_state = DEVICE_EVENT_OFFLINE;
u8 usb_id = 0;
#define WIFI_NET_CHECK_CNT (15)

//WIFI联网
extern int bt_net_config_set_ssid_pwd_get(char **ssid, char **pwd, char clean);
extern void wifi_sta_connect(char *ssid, char *pwd, char save);

static struct ap_sp {
    unsigned short net_dicon_cnt;
    unsigned short associat_timeout_cnt;
    unsigned short auto_sleep_cnt;
    unsigned short last_key;
} AI_SP = {0};

#define __this (&AI_SP)

/**
 * @brief 读取系统唤醒词状态
 * @return value 状态值：0-未唤醒，1-唤醒状态
 */
int system_keyword_start(void)
{
    return keyword_start;
}

/**
 * @brief 设置配网模式状态
 * @param enable 状态值：0-未唤醒，1-唤醒状态
 */
void system_keyword_set(char enable)
{
    keyword_start = enable;
}

/**
 * @brief 设置配网模式状态
 * @param value 状态值：0-退出配网模式，1-进入配网模式，2-正在连接wifi，3-wifi连接成功状态
 */
void system_set_keyword_wifi_enter_config(int value)
{
    keyword_wifi_enter_congfig = value;
}

/**
 * @brief 获取配网模式状态
 * @return value 状态值：0-退出配网模式，1-进入配网模式，2-正在连接wifi，3-wifi连接成功状态
 */
int system_get_keyword_wifi_enter_config(void)
{
    return keyword_wifi_enter_congfig;
}

//恢复出厂设置
void system_restore_factory_settings(void)
{
    void wifi_sta_info_clear(void);
    int user_data_info_clear(void);
    int sys_volume_clear(void);
    int sys_net_channel_info_clear(void);
    void alarm_time_clear(void);
    //debug
    void token_flag_erase(void);

#ifdef CONFIG_LTE_PHY_ENABLE
    int ch = sys_net_channel_read();
    user_sys_clear_lte_pai_code();
    http_device_unbind_app();//设备解绑
    token_flag_erase();

    //4G网络模式下
    if (ch == 1) {
        app_usb_at_cmd_send_reset();//复位模块
        usb_net_set_module_reset_status(1);//阻断模块数据发送
        lte_power_control(0);//掉电
    }
#endif

#if TCFG_USER_BT_CLASSIC_ENABLE
    extern void bt_music_del_all_info(void);
    bt_music_del_all_info();
#endif
    alarm_time_clear();
#ifdef CONFIG_WIFI_ENABLE
    wifi_sta_info_clear();
#endif
#ifdef CONFIG_EXT_WIFI_ENABLE
    ext_wifi_sta_info_clear(NULL);
#endif
    websockets_ip_addr_clean();
    sys_net_channel_info_clear();
    user_data_info_clear();
    sys_volume_clear();
}

int auto_sleep_is_enter(void)
{
#ifdef TCFG_AUTO_DEV_LOW_PWER_TIME_SEC
    if (__this->auto_sleep_cnt > TCFG_AUTO_DEV_LOW_PWER_TIME_SEC / 10) {
        return true;
    }
#endif
    return 0;
}

int auto_sleep_check_clear(void)
{
    __this->auto_sleep_cnt = 0;
    dac_mute_control(0, 0);
#ifdef  USED_TM1629_SHOWN
    tm_1629_time_shown_level_low_power(0);
#endif
}

#if (defined TCFG_AUTO_SLEEP_CHECK_EN && TCFG_AUTO_SLEEP_CHECK_EN == 1)
//#ifndef AUTO_SLEEP_TIME_MIN
//#define AUTO_SLEEP_TIME_MIN  50000
//#endif
#define RETURN_CHECK(x) if(!(x)){__this->auto_sleep_cnt = 0;return 0;}
static int auto_sleep_check(void)
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

#if (defined TCFG_AUTO_SLEEP_CHECK_ONLY_SPEECH && TCFG_AUTO_SLEEP_CHECK_ONLY_SPEECH != 0)
    if (stop_status && music_buf_play_stop_status() && music_play_stop_status() && net_music_play_stop_status() && !keyword_start) {
        __this->auto_sleep_cnt++;
    } else {
        __this->auto_sleep_cnt = 0;
    }
#else //不播放音乐&不对话：空闲
    RETURN_CHECK(stop_status);
    if (stop_status && music_buf_play_stop_status() && music_play_stop_status() && net_music_play_stop_status() && !keyword_start) {
        __this->auto_sleep_cnt++;
    } else {
        __this->auto_sleep_cnt = 0;
    }
#endif

#ifdef TCFG_AUTO_DEV_LOW_PWER_TIME_SEC
    if (__this->auto_sleep_cnt > TCFG_AUTO_DEV_LOW_PWER_TIME_SEC / 10) { //秒钟后低功耗
        if (stop_status) {
            dac_mute_control(1, 0);
        }
#ifdef  USED_TM1629_SHOWN
        tm_1629_time_shown_level_low_power(1);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
        play_face_emoji(AI_UART_CMD_EMOJI_REST);
#endif
    } else if (__this->auto_sleep_cnt < TCFG_AUTO_DEV_LOW_PWER_TIME_SEC / 10) {
#ifdef  USED_TM1629_SHOWN
        tm_1629_time_shown_level_low_power(0);
#endif
        dac_mute_control(0, 0);
    }
#endif
#ifdef AUTO_SLEEP_TIME_MIN
    if (__this->auto_sleep_cnt >= AUTO_SLEEP_TIME_MIN * 6) { //5分钟后低功耗
#if 1
        sys_power_poweroff();
#else
        if (__this->auto_sleep_cnt == AUTO_SLEEP_TIME_MIN * 6) {
            music_play_res_file("autoPwrOff.mp3");
            sys_timer_add_to_task("sys_timer", NULL, power_off_check, 9000);//9秒后启动,autoPwrOff.mp3=4s
        }
    }
#endif
    }
#endif
}
static int auto_sleep_init(void)
{
    sys_timer_add_to_task("sys_timer", NULL, auto_sleep_check, 10000);//10秒1次检测
    return 0;
}
late_initcall(auto_sleep_init);
#endif

//回声消除回采增益（播放音频下）
int AEC_GAIN(int volume)
{
    //y = -0.75x + 128.21
    return (int)(-0.75 * (double)(volume) + 128.21);
    //return (CONFIG_AISP_AEC_ADC_GAIN);
}

//麦克风0增益（播放音频下）
int MIC0_GAIN(int volume)
{
    //y = -0.6429x + 102.857
    return (int)(-0.6429 * (double)(volume) + 102.857);
    //return (CONFIG_AISP_MIC_ADC_GAIN);
}

//麦克风1增益（播放音频下）
int MIC1_GAIN(int volume)
{
    return MIC0_GAIN(volume);
    //return (CONFIG_AISP_MIC_ADC_GAIN);
}

int ai_speaker_mode_init(void)
{
    log_i("ai_speaker_play_main\n");
#ifdef ASR_SAMPLE_RATE
    return aisp_open(ASR_SAMPLE_RATE);
#else
    return aisp_open(16000);
#endif
}

void ai_speaker_mode_exit(void)
{
    websockets_client_dialogue_exit();
    websockets_close_request(1);
    websockets_dialogue_timeout_del();
    aisp_close();
}

//支持文本对话
void ai_speaker_text_dialogue(char *text)//text为UTF8格式
{
    music_buf_play_accept(0);
    websockets_client_dialogue_exit();
    websockets_close_request(1);
    websockets_dialogue_timeout_del();

    music_buf_play_stop_all();
    music_buf_play_set_stop();
    net_music_play_set_stop();
    websockets_free_lbuf_buf();
    music_buf_play_free_lbuf();

    music_buf_play_accept(1);
    websocket_client_text_dialogue_create(text);
}

static void ai_speaker_mode_stop(void)
{
    aisp_clear(1);
}

int ai_speaker_volume_set(int volume, char update_gain)
{
    int ret = music_buf_play_set_volume(volume) && music_play_set_volume(volume) && net_music_set_dec_volume(volume);
#if defined CONFIG_BT_ENABLE && defined CONFIG_BT_MUSIC_MODE_ENABLE
    extern int bt_music_set_volume(u8 volume);
    ret |= bt_music_set_volume((u8)volume);
#endif
#ifdef CONFIG_USB_DISK_MUSIC_MODE_ENABLE
    extern int usbdisk_music_set_dec_volume(int volume);
    ret |= usbdisk_music_set_dec_volume(volume);
#endif
#ifdef CONFIG_SD_MUSIC_MODE_ENABLE
    extern int sd_music_set_dec_volume(int volume);
    ret |= sd_music_set_dec_volume(volume);
#endif
#ifdef CONFIG_AUX_MUSIC_MODE_ENABLE
    extern int aux_dec_volume_set_volume(int volume);
    ret |= aux_dec_volume_set_volume(volume);
#endif
    if (ret) {
        sys_volume_write(&volume);
    }
    if (update_gain) {
//        aisp_mic_gain_set(volume);
    }
    return 0;
}

int sys_all_volume_auto_set(int volume)
{
    return ai_speaker_volume_set(volume, 0);
}

static int ai_speaker_key_click(struct key_event *key)
{
    int ret = true;
    int res;
    int play = 1;
    u8 volume = 0;

    printf("-->ai_speaker_key_click  = %d \n", key->value);
    switch (key->value) {
    case KEY_MUTE:
    case KEY_OK://按键播放或者启动识别

        /*
        if(music_buf_play_supspend()){
            net_music_play_resum();
        }*/

        if (net_music_play_start_status()) {
#ifdef CONFIG_NO_SDRAM_ENABLE
            net_music_dec_stop();
#else
            net_music_play_puase();
#endif
        } else {
#ifdef CONFIG_LVGL_UI_ENABLE
            lv_demo_ai_dialogue_start_mode(1);
#endif
            aisp_wake(0);
        }
        break;
    case KEY_DOWN:
        net_music_play_next(net_music_play_type_get());
        break;
    case KEY_UP:
        if (net_music_play_last_check()) {
            net_music_play_last_request();
        } else {
            net_music_play_next(net_music_play_type_get());
        }
        break;
    case KEY_MODE:
        ai_speaker_mode_stop();
        extern int audio_app_mode_switch(char *name);
        audio_app_mode_switch(NULL);
        break;
    case KEY_VOLUME_DEC:
        if (net_music_set_dec_volume_step(-VOLUME_STEP) && music_buf_play_set_volume_step(-VOLUME_STEP)
            && music_play_set_volume_step(-VOLUME_STEP)) {
            sys_volume_write_step(-VOLUME_STEP);
            play = 0;//没有播放音频
        }
        sys_volume_read(&volume);
        if (volume > 0 && volume <= 100 && play) {
            aisp_mic_gain_set(volume);
        }
        break;
    case KEY_VOLUME_INC:
        if (net_music_set_dec_volume_step(VOLUME_STEP) && music_buf_play_set_volume_step(VOLUME_STEP)
            && music_play_set_volume_step(VOLUME_STEP)) {
            sys_volume_write_step(VOLUME_STEP);
            play = 0;//没有播放音频
        }
        sys_volume_read(&volume);
        if (volume > 0 && volume <= 100 && play) {
            aisp_mic_gain_set(volume);
        }
        break;
    case KEY_POWER:;//唤醒
#ifdef CONFIG_LVGL_UI_ENABLE
        lv_demo_ai_dialogue_start_mode(1);
#endif
        aisp_wake(0);
        break;
    case KEY_SUPSPEND:
        if (!music_buf_play_supspend()) {
            if (!(music_buf_play_stop_status() && music_play_stop_status())) {
                net_music_play_puase();
            }
        }
        break;
    case KEY_RESUM:
        if (music_buf_play_supspend()) {
            net_music_play_resum();
        }
        break;
    case KEY_CPU_RESET:
        ai_speaker_mode_exit();
        ret = false;
        break;
    default:
        ret = false;
        break;
    }

    return ret;
}

static int ai_speaker_key_long(struct key_event *key)
{
    int ret = true;
    int play = 1;
    u8 volume = 0;
    printf("-->ai_speaker_key_long  = %d \n", key->value);
    switch (key->value) {
    case KEY_MODE:
        ;
#if (defined TCFG_POWER_KEY_LONG_PRESS_VAD_EN)
        if (((__this->last_key >> 8) & 0xFF) != KEY_EVENT_LONG) {
            key_vad_pcm_send_set_status(1, 1);
        }
#else
        aisp_wake(8); //按键进入配网
#endif
        break;
    case KEY_VOLUME_DEC:
    case KEY_DOWN:
        if (net_music_set_dec_volume_step(-VOLUME_STEP) && music_buf_play_set_volume_step(-VOLUME_STEP)
            && music_play_set_volume_step(-VOLUME_STEP)) {
            sys_volume_write_step(-VOLUME_STEP);
            play = 0;//没有播放音频
        }
        sys_volume_read(&volume);
#if (SXY_LL_YHH_BOARD && defined USED_TM1629_SHOWN)
        extern void tm_1629_shown_set_volume(int vol);
        tm_1629_shown_set_volume(volume);
#endif
        if (volume > 0 && volume <= 100 && play) {
            aisp_mic_gain_set(volume);
        }
        break;
    case KEY_VOLUME_INC:
    case KEY_UP:
        if (net_music_set_dec_volume_step(VOLUME_STEP) && music_buf_play_set_volume_step(VOLUME_STEP)
            && music_play_set_volume_step(VOLUME_STEP)) {
            sys_volume_write_step(VOLUME_STEP);
            play = 0;//没有播放音频
        }
        sys_volume_read(&volume);
#if (SXY_LL_YHH_BOARD && defined USED_TM1629_SHOWN)
        extern void tm_1629_shown_set_volume(int vol);
        tm_1629_shown_set_volume(volume);
#endif
        if (volume > 0 && volume <= 100 && play) {
            aisp_mic_gain_set(volume);
        }
        break;
    case KEY_SUPSPEND:
        aisp_all_pause(1);
        break;
    case KEY_RESUM:
        aisp_all_pause(0);
        break;
    case KEY_OK:
    case KEY_POWER:
        ;
        aisp_all_pause(1);
        ai_speaker_mode_exit();
        music_play_stop_all();
        music_play_waite();
#ifdef CONFIG_UI_PLAY_EMOJI
        play_face_emoji(AI_UART_CMD_PWR_OFF);
#endif
        music_play_res_file("PwrOff.mp3");
        music_play_waite();
        sys_power_poweroff();
        break;
    default:
        ret = false;
        break;
    }

    return ret;
}

static int ai_speaker_key_hand_up(struct key_event *key)
{
    int ret = true;
    int play = 1;
    u8 volume = 0;
    printf("-->ai_speaker_key_hand_up  = %d \n", key->value);

    switch (key->value) {
    case KEY_MODE:
        ;
#if (defined TCFG_POWER_KEY_LONG_PRESS_VAD_EN)
        if (((__this->last_key >> 8) & 0xFF) == KEY_EVENT_LONG) {
            printf("-->KEY_EVENT_LONG hand_up\n");
            key_vad_pcm_send_set_status(0, 1);
        }
#endif
        break;
    case KEY_SUPSPEND:
        aisp_all_pause(1);
        break;
    case KEY_RESUM:
        aisp_all_pause(0);
        break;
    }
    return true;
}

int aisp_sys_power_off(void)
{
    struct key_event key = {0};
    key.type = KEY_EVENT_USER;
    key.action = KEY_EVENT_LONG;
    key.value = KEY_POWER;
    return key_event_notify(KEY_EVENT_FROM_USER, &key);
}

int sys_net_channel_change_reset(void)
{
    struct key_event key = {0};
    key.type = KEY_EVENT_USER;
    key.action = KEY_EVENT_FOURTH_CLICK;
    key.value = KEY_OK;
    return key_event_notify(KEY_EVENT_FROM_USER, &key);
}

int app_usb_is_plugin_state(void)
{
    return (usb_hp_state == DEVICE_EVENT_IN);
}

static int ai_speaker_key_event_handler(struct key_event *key)
{
    int ret = 0;
    auto_sleep_check_clear();
    if (!alarm_music_play_stop() && (key->value != KEY_SUPSPEND && key->value != KEY_RESUM && key->value != KEY_POWER)) {
        return true;//闹钟在响，按键失效
    }

    int cur_net_mode = sys_net_mode_read();
    int ch = sys_net_channel_read();

    printf("->key->action = %d ,key->value = %d \n", key->action, key->value);
    switch (key->action) {
    case KEY_EVENT_UP:
        ai_speaker_key_hand_up(key);
        break;
    case KEY_EVENT_CLICK:
        ai_speaker_key_click(key);
        break;
    case KEY_EVENT_HOLD:
        if (key->value == KEY_CANCLE) { //暂停播放
            music_buf_play_set_stop();
            websockets_free_lbuf_buf();
            music_buf_play_free_lbuf();
            break;
        } else {//闹钟唤醒事件
            //music_play_res_file("AlarmRing1.mp3");
            music_play_alarm(key->value);
        }
        break;
    case KEY_EVENT_LONG:
        ai_speaker_key_long(key);
        break;
    case KEY_EVENT_DOUBLE_CLICK:
        //模式切换
        if (key->value == KEY_OK) {
#ifdef CONFIG_QYAI_MUTILMODAL_ENABLE
            qyai_text2img_cyc_mode_auto(1);
            break;
#endif
#if (SXY_LL_YHH_BOARD)
            extern int tm_light_pwm_auto(void);
            tm_light_pwm_auto();
            //不是AD按键支持双击模式切换
#else //if (!defined TCFG_ADKEY_ENABLE || TCFG_ADKEY_ENABLE == 0)
            ai_speaker_mode_stop();
            extern int audio_app_mode_switch(char *name);
            audio_app_mode_switch(NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
            extern int play_face_emoji(int avi_index);
            if (audio_app_mode_check() == 1) {
                play_face_emoji(AI_UART_CMD_BT_MODE);
            }
#endif
        }
#ifdef  USED_TM1629_SHOWN
        else if (key->value == KEY_MODE) { //显示亮度切换
            int tm_1629_led_shown_bright_level_auto(void);
            tm_1629_led_shown_bright_level_auto();
        }
#endif
        //是否开启自动切换：0->NOT AUTO MODE、1->AUTO MODE
        else if (key->value == KEY_UP) {
//            int net_mode = sys_net_mode_read();
//            net_mode = (net_mode + 1) % 2;
//            printf("write net_mode:%d", net_mode);
//            sys_net_mode_write(net_mode);
//            system_reset();
        } else if (key->value == KEY_DOWN) {
            ;
        }
        break;
    case KEY_EVENT_TRIPLE_CLICK:
#if IP_NAPT
        if (wl_wifi_cur_mode_get() == WIFI_MODE_AP) {
            printf("=====invalid action=====");
            break;
        } else if (wl_wifi_cur_mode_get() == WIFI_MODE_NONE) {
            //fixme: start station mode
            printf("=====invalid action=====");
            break;
        }
#endif
        //配网按键
        if (key->value == KEY_OK) {
#if (defined CONFIG_WIFI_ENABLE && defined CONFIG_LTE_PHY_ENABLE)
            if (ch == 1) {
                break;
            }
#endif
            //WIFI模式下按键进入配网
            aisp_wake(8);
        }

        break;
    case KEY_EVENT_FOURTH_CLICK:
#if (defined CONFIG_WIFI_ENABLE && defined CONFIG_LTE_PHY_ENABLE)
#if IP_NAPT
        printf("=====invalid action=====");
        break;
#endif
        if (key->value == KEY_OK) {
            if (cur_net_mode == NET_MODE_MANUAL_SET) { //手动模式: 4G和WiFi切换
                int net_ch = sys_net_channel_read();
                if (net_ch < 0) {
                    sys_net_channel_write(NET_CH_SELECT_WIFI);
                    net_ch = sys_net_channel_read();
                } else {
                    net_ch = net_ch ? 0 : 1;
                    sys_net_channel_write(net_ch);
                    if (sys_net_channel_read() == net_ch) {
                        aisp_all_pause(1);
                        usb_net_set_module_reset_status(1);
                        music_play_res_file("SysReset.mp3");
                        music_play_waite();
                        //4G模式下切走，先发复位指令再断电
                        if (sys_net_channel_read() == 0 && app_usb_get_driver_status()) {
                            app_usb_at_cmd_send_reset();
                            lte_power_control(0);
                        }
                        system_reset();
                    }
                }
            } else {
                //UI界面/按键触发选择网络模式
                if (cur_net_mode == NET_MODE_AUTO) {
                    goto done;
                }

                u8 key_down_cnt = sys_net_channel_read();
                if (key_down_cnt == 0) {
                    //切到4G模式[wifi->lte]
                    printf("=====switch to lte mode=====");
                    if (wifi_connect_net_success()) {
                        printf("=====wifi state: connect!!!=====");
                        wifi_and_network_off();
                    } else {
                        printf("=====wifi state: not connect!!!=====");
                        if (wifi_is_online()) { //处于扫描的状态
                            music_play_res_file("NetConting.mp3");
                            wifi_and_network_off();
                        }
                    }
                    //music_play_res_file("NetConting.mp3");
                    net_work_on = NET_WORK_ON_LTE;
                    sys_net_channel_write(NET_CH_SELECT_LTE);
                    //设置lte为网卡
                    qyai_net_interface_set(1);
                    lte_power_control(1);
                } else if (key_down_cnt == 1) {
                    lte_power_control(0);
                    //切到WIFI模式[lte->wifi]
                    printf("=====switch to wifi mode=====");
                    if (lte_connect_net_success()) {
                        printf("=====lte state: connect!!!=====");
                    } else {
                        printf("=====lte state: not connect!!!=====");
                    }
                    net_work_on = NET_WORK_ON_WIFI;
                    sys_net_channel_write(NET_CH_SELECT_WIFI);
                    //设置wifi为网卡
                    qyai_net_interface_set(0);
                }

                if (++key_down_cnt >= 2) {
                    key_down_cnt = 0;
                }
            }
        } else
#endif
            //四连按进产测模式
            if (key->value == KEY_MODE || key->value == KEY_OK || key->value == KEY_UP) {
#ifdef PRODUCTION_ALL_TEST_ENABLE
                if (is_in_config_network_state()) {
                    config_network_stop();
                }
                aisp_all_pause(1);
                int wait = 100;
                while (--wait) {
                    if (music_buf_play_stop_status() && music_play_stop_status() && !net_music_play_start_status()) {
                        break;
                    }
                    os_time_dly(1);
                }
                aisp_all_pause(0);
                enter_product_func_test(1);
#endif
            }
done:
        break;
    //五连击
    case KEY_EVENT_FIRTH_CLICK:
        if (key->value == KEY_OK) {
#ifdef CONFIG_LTE_PHY_ENABLE
            //切卡操作
            app_usb_at_change_sim_card(key);
#endif
            //qyai_net_socket_err_callback("err no recv complete index", 0, websockets_client_trace_id_get());
        }
        break;

    //六连击以上，恢复出厂设置
    case KEY_EVENT_MORE_CLICK:
        if (key->value == KEY_OK || key->value == KEY_MODE) {
            extern int http_wifi_device_unbind_app_task(void);
            http_wifi_device_unbind_app_task();
            aisp_all_pause(1);
            music_play_res_file("SysRstFaSet.mp3");
            music_play_waite();
            //恢复出厂设置
            system_restore_factory_settings();
            music_play_res_file("SysReset.mp3");
            music_play_waite();
            system_reset();
        }
        break;
    default:
        break;
    }
    __this->last_key = ((key->action & 0xFF) << 8) | (key->value & 0xFF);
    return true;
}

#if IP_NAPT
static void net_http_server_thread(void)
{
    printf("=====net_http_server_thread start=====");
    webhttp_serv_main_thread();
    while (1) {
        printf("=====net_http_server_thread run=====");
        os_time_dly(2 * 100);
    }
}
#endif

#ifdef CONFIG_LTE_PHY_ENABLE
//设备sim卡绑定
extern int http_ai_sim_card_bind_request_task(char *iccid, void (*set_callback)(int code, char *msg, char *data));
extern int http_ai_sim_card_bind_request(char *_iccid, void (*set_callbak)(int code, char *msg, char *data));
extern char *app_usb_module_get_iccid(void);
#define AI_SPEAKER_POLL_TASK "poll_task"
static void ai_speaker_poll_task(void *priv)
{
    //接收消息队列buf
    int msg[8] = {0};
    int err = 0;
    printf("=====ai_speaker_poll_task=====");
    while (1) {
        //阻塞等待消息
        err = os_taskq_pend("taskq", msg, ARRAY_SIZE(msg));
        if (err != OS_TASKQ || msg[0] != Q_USER) {
            continue;
        }
        printf("-->recv msg kid[%d]\n", msg[1]);
        switch (msg[1]) {
        case 0:
            for (u8 i = 0; i < WIFI_NET_CHECK_CNT; i++) {
                printf("=====usb: wifi_net_check_cnt=%d", i);
                os_time_dly(200);
                //插入4G,优先检查wifi是否能联网
                if (wifi_connect_net_success()) {
                    printf("=====wifi connect success, break=====\r\n");
                    lte_power_control(0);
                    goto done;
                }
            }

            //WIFI初始化，但连不上AP
            if (wifi_is_online()) {
                printf("=====wifi has been enable=====");
                //notes:
                //WIFI已连接成功: NET DISCONNECT事件
                //WIFI未连接成功: NET STOP事件
                wifi_and_network_off();
            } else {
                printf("=====wifi is not enable=====");
                //music_play_res_file("NetConting.mp3");
                //4G关闭静态ip冲突检测
                IPV4_ADDR_CONFLICT_DETECT = 0;
                lte_net_init();
                lte_net_restart();
                app_usb_at_cmd_init(usb_id, NULL);
                sys_net_channel_write(NET_CH_SELECT_LTE);
            }
done:
            break;
        case 1:
#if IP_NAPT
            if (thread_fork("net_http_server_thread", 10, 1024, 0, NULL, net_http_server_thread, NULL) != OS_NO_ERR) {
                puts("http_server_thread create err\n");
            }
#endif
            break;
        default:
            break;
        }
    }
}

int lte_event_handler(struct device_event *device_eve)
{
    int msg[4] = {0};
    int net_ch = sys_net_channel_read();
    int net_mode = sys_net_mode_read();
    usb_id = ((const char *)device_eve->value)[8] - '0';
    usb_hp_state = device_eve->event;
    //兼容没有IO控制电源的USB LTE模块设计,否则wifi联网过程中，USB直接就枚举，走后面的流程
    if ((net_ch == NET_CH_SELECT_WIFI) && (net_mode == NET_MODE_MANUAL_SET)) {
        return 0;
    }
    switch (device_eve->event) {
    case DEVICE_EVENT_IN:
        printf("usb wireless%d device IN,net_mode:%d", usb_id, net_mode);
        if (net_mode == NET_MODE_AUTO) {
            msg[0] = 0;
            os_taskq_post_type(AI_SPEAKER_POLL_TASK, Q_USER, 1, msg);
        } else {
            //4G关闭静态ip冲突检测
            IPV4_ADDR_CONFLICT_DETECT = 0;
            app_usb_at_cmd_init(usb_id, lte_net_init);
        }
done:
        break;
    case DEVICE_EVENT_OUT:
        printf("usb wireless%d device OUT", usb_id);
        if (net_mode == NET_MODE_AUTO) {
            //sys_net_channel_write(NET_CH_SELECT_WIFI);
            qyai_net_interface_set(0);
            app_usb_at_cmd_deinit();
        } else {
            net_dhcp_status_set(NET_DISCONNECT);
            app_usb_at_cmd_deinit();
            sys_timeout_add(1, lte_power_control, 1000);//断电1秒后重供电
        }
        break;
    }
    return 0;
}

static void ai_speaker_card_bind_callbak(int code, char *msg, char *data)
{
    if (msg) {
        printf("sim card recv msg : %s \n");
    }
}

static void http_sim_card_bind_request(void)
{
    char *iccid_ptr = app_usb_module_get_iccid();
    http_ai_sim_card_bind_request_task(iccid_ptr, ai_speaker_card_bind_callbak);
}
#endif // CONFIG_LTE_PHY_ENABLE

int ai_net_event_handler(struct net_event *event)
{
    int net_ch = 0;
    int net_mode = NET_MODE_MANUAL_SET;
#if (defined CONFIG_WIFI_ENABLE && defined CONFIG_LTE_PHY_ENABLE)
    net_ch = sys_net_channel_read();
    if (net_ch < 0) {
        net_ch = 0;
    }
#elif (defined CONFIG_LTE_PHY_ENABLE)
    net_ch = 1;
#endif
    if (!ASCII_StrCmp(event->arg, "net", 4)) {
        net_mode = sys_net_mode_read();
        printf("net_event=%d, mode_type=%d, net_ch=%d",  event->event, net_mode, net_ch);
        switch (event->event) {
        case NET_CONNECT_TIMEOUT_NOT_FOUND_SSID:
        case NET_CONNECT_ASSOCIAT_TIMEOUT:
        case NET_CONNECT_ASSOCIAT_FAIL:
            if (net_ch != 0) {
                break;
            }
#if BT_NET_CFG_EN
            if (is_in_config_network_state()) {
                ble_cfg_net_result_notify(event->event);
            }
#endif
            if (++__this->associat_timeout_cnt < 3) {
                if (app_usb_is_plugin_state()  && net_mode == NET_MODE_AUTO) {
                    printf("=====wifi do not reconnect=====");
                    break;
                }
#ifdef CONFIG_WIFI_ENABLE
                wifi_off();
                wifi_on();
                printf("=====reconnect=====:%d", __this->associat_timeout_cnt);
                wifi_sta_reconnect();
#endif
#ifdef CONFIG_EXT_WIFI_ENABLE
                //ext_wifi_off();
                //ext_wifi_on();
                ext_wifi_sta_reconnect();
#endif
#if (defined CONFIG_WIFI_ENABLE && defined CONFIG_BT_ENABLE)
                //卸载wifi驱动后需要重新设置一下共存参数才能生效
                void switch_rf_coexistence_config_table(u8 index);
                switch_rf_coexistence_config_table(get_rf_coexistence_config_index());
#endif
            } else {
                if ((__this->associat_timeout_cnt == 3 || __this->associat_timeout_cnt == 4) && !is_production_test_enter(0)) {
                    music_play_res_file("WifiReCfg.mp3");
#ifdef CONFIG_UI_PLAY_EMOJI
                    extern int play_face_emoji(int index);
                    play_face_emoji(AI_UART_CMD_WIFI_CONFIG);
#endif
                }
                if (!is_in_config_network_state()) {
                    config_network_start();
                    system_set_keyword_wifi_enter_config(1);
                }
                keyword_wifi_enter_congfig = 2;
            }
            break;
        case NET_EVENT_SMP_CFG_FIRST:
        case NET_CONNECT_DHCP_TIMEOUT:
            ;
#ifdef CONFIG_UI_PLAY_EMOJI
            extern int play_face_emoji(int index);
            play_face_emoji(AI_UART_CMD_WIFI_CONFIG);
#endif
            if (net_ch != 0) {
                music_play_res_file("NetNote.mp3");
                break;
            }
#ifdef CONFIG_BT_ENABLE
            //修复问题：第一次开机连接路由器，DHCP超时连不上，设备发起重连
            char *ssid, *pwd;
            if (bt_net_config_set_ssid_pwd_get(&ssid, &pwd, 0)) {
                printf("===reconnet, ssid: %s, pwd: %s\n", ssid, pwd);
#ifdef CONFIG_WIFI_ENABLE
                wifi_sta_connect(ssid, pwd, 1);
#endif
#ifdef CONFIG_EXT_WIFI_ENABLE
                ext_wifi_sta_connect(ssid, pwd, 1);
#endif
                bt_net_config_set_ssid_pwd_get(NULL, NULL, 1);
                break;
            }
#endif
#ifdef PRODUCTION_ALL_TEST_ENABLE
            if (is_production_test_enter(0)) {
                wifi_sta_connect(PRODUCTION_ALL_TEST_WIFI_SSID, PRODUCTION_ALL_TEST_WIFI_PWD, 0);
                break;
            }
#endif

#if BT_NET_CFG_EN
            if (is_in_config_network_state()) {
                ble_cfg_net_result_notify(NET_CONNECT_ASSOCIAT_FAIL);
            }
#endif
            if (!is_in_config_network_state() && !is_production_test_enter(0)) {
                if (net_ch == 0) {
                    music_play_res_file("WifiNote.mp3");
                    config_network_start();
                    system_set_keyword_wifi_enter_config(1);
                }
            }
#if (defined TCFG_LED_STATUES_VBAT_NET_EN && TCFG_LED_STATUES_VBAT_NET_EN == 1)
            //0初始化 1网络异常-快闪  2电池<20%-快闪  3网络正常和电池>=20%-常亮
            extern void led_status_set(int status);
            led_status_set(1);
#endif
            break;
        case NET_EVENT_SMP_CFG_FINISH:
            keyword_wifi_enter_congfig = 2;
            __this->net_dicon_cnt = 0;//配网清空连接失败次数
            if (is_in_config_network_state()) {
                if (net_ch == 0) {
                    music_play_res_file("WifiCfgRecv.mp3");
                    config_network_connect();
                } else {
                    music_play_res_file("NetCfgRecv.mp3");
                }
#ifdef CONFIG_ASR_ALGORITHM
                aisp_resume();
#endif
            }
            break;
        case NET_EVENT_CONNECTED:
            __this->associat_timeout_cnt = 0;
            printf("NET_EVENT_CONNECTED : %d, %d \n", keyword_wifi_enter_congfig, __this->net_dicon_cnt);
            if (keyword_wifi_enter_congfig >= 2 && system_reset_reason_get() != SYS_RST_ALM_WKUP && !is_production_test_enter(0) && __this->net_dicon_cnt == 0) {
                if (net_ch == 0) {
                    music_play_res_file("WifiConSucc.mp3");
#ifdef CONFIG_UI_PLAY_EMOJI
                    extern int play_face_emoji(int avi_index);
                    play_face_emoji(AI_UART_CMD_WIFI_CONNECTED);
#endif
                } else {
                    music_play_res_file("NetConSucc.mp3");
                }
            }

            if (keyword_wifi_enter_congfig != 1) {
                config_network_stop();
                keyword_wifi_enter_congfig = 0;
            }

#ifdef CONFIG_UI_PLAY_EMOJI
            extern int play_face_emoji(int index);
            play_face_emoji(AI_UART_CMD_WIFI_CONNECTED);
#endif


#if BT_NET_CFG_EN
            ble_cfg_net_result_notify(event->event);
#endif

#ifndef CONFIG_NO_SDRAM_ENABLE
            extern void wifi_pcm_stream_task_init(void);
            wifi_pcm_stream_task_init();
#endif
            extern void ntptime_update(void);
            ntptime_update();//NTP时间校准
#ifndef CONFIG_NO_SDRAM_ENABLE
            if (net_ch == 1 || net_ch == 0) {
#if (CONFIG_ASR_ALGORITHM == IFLYTEK_ALGORITHM)
                void user_aiui_connected_init(void);
                user_aiui_connected_init();
#endif
            }
#endif
#if (defined CONFIG_WIFI_ENABLE && defined CONFIG_LTE_PHY_ENABLE)
            sys_net_channel_ssid_reset();
#endif
            int is_production_test_enter(char wake);
            is_production_test_enter(3);
#ifdef CONFIG_ALIYUN_IOT_ENABLE
            void aliyun_mqtt_init(void);
            aliyun_mqtt_init();
#endif
#if (defined TCFG_LED_STATUES_VBAT_NET_EN && TCFG_LED_STATUES_VBAT_NET_EN == 1)
            //0初始化 1网络异常-快闪  2电池<20%-快闪  3网络正常和电池>=20%-常亮
            extern void led_status_set(int status);
            led_status_set(3);
#endif
#ifndef CONFIG_NO_SDRAM_ENABLE
            extern void net_dns_ip_check_first_fast(char *task);
            sys_timeout_add_to_task("app_core", "app_core", net_dns_ip_check_first_fast, 1 * 60 * 1000);//1分钟检测一次IP
            sys_timer_add_to_task("app_core", "app_core", net_dns_ip_check_first_fast, 20 * 60 * 1000);//20分钟检测一次IP
#endif
#if IP_NAPT
            int msg[4] = {0};
            msg[0] = 1;
            os_taskq_post_type(AI_SPEAKER_POLL_TASK, Q_USER, 1, msg);
#endif

#ifdef CONFIG_LTE_PHY_ENABLE
            if (net_ch == 1) {
                sys_timeout_add_to_task("app_core", NULL, http_sim_card_bind_request, 3000);//注意：低内存版本，联网同时开启多个http引起内存不足死机问题
            }
#endif
#if 0
            http_ai_get_photo_grap(http_get_photo_grap_cb);

            //参数：返回请求结果的回调函数+多模态拍照类型，上报消耗一次拍照次数
            char photo_type[32];
            snprintf(photo_type, sizeof(photo_type), "\"photo_type\":%d", OPT_MODE_TEXT_IMG2_IMG);
            http_ai_consume_photo_grap(2, http_consume_photo_grap_cb, photo_type);

#endif
            break;
        case NET_EVENT_DISCONNECTED:
            canceladdrinfo();
            __this->net_dicon_cnt = (keyword_wifi_enter_congfig == 1 || keyword_wifi_enter_congfig == 2) ? 0 : (__this->net_dicon_cnt + 1);//配网清空连接失败次数
            printf("NET_EVENT_DISCONNECTED : %d, %d \n", keyword_wifi_enter_congfig, __this->net_dicon_cnt);
#ifdef CONFIG_LTE_PHY_ENABLE
            if (net_work_on == NET_WORK_ON_LTE) {
                printf("=====key switch to lte mode=====");
                break;
            }
#endif
            if (!keyword_wifi_enter_congfig && system_reset_reason_get() != SYS_RST_ALM_WKUP && !is_production_test_enter(0)) {
                if (net_ch == 0) {
                    if (!is_in_config_network_state()) {
#ifdef CONFIG_LTE_PHY_ENABLE
                        if (net_mode == NET_MODE_AUTO) {
                            music_play_res_file("NetDiscon.mp3");
                            //WIFI的DISCONNECTED事件，在WIFI已经连接成功的情况下：pc主动断开WIFI
                            if (app_usb_is_plugin_state()) {
                                //4G关闭静态ip冲突检测
                                IPV4_ADDR_CONFLICT_DETECT = 0;
                                lte_net_init();
                                lte_net_restart();
                                app_usb_at_cmd_init(usb_id, NULL);
                                sys_net_channel_write(NET_CH_SELECT_LTE);
                            } else { //WiFi断开[pc主动]，plug out等价于掉电
                                if (wifi_is_online()) {
                                    printf("=====usb is offline, do reconn=====");
                                    sys_net_channel_write(NET_CH_SELECT_LTE);
                                    //wifi停止扫描
                                    wifi_and_network_off();
                                    //开启LTE
                                    lte_power_control(1);
                                }
                            }

                        } else { //manual模式: 按键切，lte->wifi
                            printf("=====manual mode: wifi do reconnect!!!=====");
                            wifi_and_network_off();
                            wifi_and_network_on();
                            wifi_sta_reconnect();
                        }
#else
                        if (app_usb_is_plugin_state()  && net_mode == NET_MODE_AUTO) {
                            printf("=====wifi do not reconnect=====");
                            break;
                        }
                        printf("=====wifi do reconnect!!!!=====");
#ifdef CONFIG_WIFI_ENABLE
                        wifi_and_network_off();
                        wifi_and_network_on();
                        wifi_sta_reconnect();
#endif
#ifdef CONFIG_EXT_WIFI_ENABLE
                        //ext_wifi_off();
                        //ext_wifi_on();
                        ext_wifi_sta_reconnect();
#endif
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
                        extern int play_face_emoji(int index);
                        play_face_emoji(AI_UART_CMD_WIFI_DISCON);
#endif

#if (defined CONFIG_WIFI_ENABLE && defined CONFIG_BT_ENABLE)
                        //卸载wifi驱动后需要重新设置一下共存参数才能生效
                        void switch_rf_coexistence_config_table(u8 index);
                        switch_rf_coexistence_config_table(get_rf_coexistence_config_index());
#endif
                    }
                } else {
                    //fixme: 修改提示音为“4G网络已断开”
                    //music_play_res_file("NetDiscon.mp3");
                    if (net_mode == NET_MODE_AUTO) {
                        sys_net_channel_write(NET_CH_SELECT_WIFI);
                        //music_play_res_file("WifiConting.mp3");
                        wifi_and_network_on();
                    }
                }
            }
#ifndef CONFIG_NO_SDRAM_ENABLE
            if (net_ch == 1 || net_ch == 0) {
#if (CONFIG_ASR_ALGORITHM == IFLYTEK_ALGORITHM)
                void user_aiui_discon_uninit(void);
                user_aiui_discon_uninit();
#endif
            }
#endif
#ifdef CONFIG_ALIYUN_IOT_ENABLE
            void aliyun_mqtt_uninit(void);
            aliyun_mqtt_uninit();
#endif
#if (defined TCFG_LED_STATUES_VBAT_NET_EN && TCFG_LED_STATUES_VBAT_NET_EN == 1)
            extern void led_status_set(int status);//0初始化 1网络异常-快闪  2电池<20%-快闪  3网络正常和电池>=20%-常亮
            led_status_set(1);
#endif
            return false;
        case NET_EVENT_SMP_CFG_TIMEOUT:
            if (!is_production_test_enter(0)) {
                if (net_ch == 0) {
                    if (is_in_config_network_state()) {
                        music_play_res_file("WifiConErr.mp3");
                    }
                } else {
                    //todo:处理超时场景
                }
            }
            break;

        case NET_SMP_CFG_COMPLETED:
#ifdef CONFIG_AIRKISS_NET_CFG
            wifi_smp_set_ssid_pwd();
#endif
            break;
        case NET_EVENT_DISCONNECTED_AND_REQ_CONNECT:
            puts("\n NET_EVENT_DISCONNECTED_AND_REQ_CONNECT \n");
            if (net_ch != 0 || keyword_wifi_enter_congfig) {
                break;
            }
            if (app_usb_is_plugin_state()  && net_mode == NET_MODE_AUTO) {
                printf("=====wifi do not reconnect=====");
                break;
            }
#ifdef CONFIG_WIFI_ENABLE
            wifi_and_network_off();
            wifi_and_network_on();
            wifi_sta_reconnect();
#endif
#ifdef CONFIG_EXT_WIFI_ENABLE
            //ext_wifi_off();
            //ext_wifi_on();
            ext_wifi_sta_reconnect();
#endif
#if (defined CONFIG_WIFI_ENABLE && defined CONFIG_BT_ENABLE)
            //卸载wifi驱动后需要重新设置一下共存参数才能生效
            void switch_rf_coexistence_config_table(u8 index);
            switch_rf_coexistence_config_table(get_rf_coexistence_config_index());
#endif

//            if(!is_in_config_network_state()){
//                config_network_start();
//                music_play_res_file("WifiReCfg.mp3");
//                system_set_keyword_wifi_enter_config(1);
//            }
            break;

        //WIFI未连接成功情况下，4G接入
        case NET_EVENT_STOP:
#ifdef CONFIG_LTE_PHY_ENABLE
            if (net_work_on == NET_WORK_ON_LTE) {
                printf("=====key switch to lte mode=====");
                break;
            }
            if (net_ch == 0) {
                if (net_mode == NET_MODE_AUTO) {
                    if (app_usb_is_plugin_state()) {
                        //正在连接4G网络
                        music_play_res_file("NetConting.mp3");
                        //4G关闭静态ip冲突检测
                        IPV4_ADDR_CONFLICT_DETECT = 0;
                        lte_net_init();
                        lte_net_restart();
                        app_usb_at_cmd_init(usb_id, NULL);
                        sys_net_channel_write(NET_CH_SELECT_LTE);
                    }
                }
            }
#endif
            break;
        case NET_NTP_GET_TIME_SUCC: //NTP获取成功事件返回
            break;
        case NET_NTP_GET_TIME_SUCC + 32://初始化连接网络
            if (!keyword_wifi_enter_congfig && system_reset_reason_get() != SYS_RST_ALM_WKUP  && !is_production_test_enter(0)) {
                if (!__this->net_dicon_cnt) {
                    if (net_ch == 0) {
#ifdef CONFIG_UI_PLAY_EMOJI
                        extern int play_face_emoji(int avi_index);
                        play_face_emoji(AI_UART_CMD_WIFI_CONNECTING);
#endif
                        music_play_res_file("WifiConting.mp3");
                    } else {
                        music_play_res_file("NetConting.mp3");
                    }
                }
                keyword_wifi_enter_congfig = 3;
            }
            break;
        case NET_NTP_GET_TIME_SUCC + 33://网络连接异常
            if (!keyword_wifi_enter_congfig) {
                music_play_res_file("ServErrCon.mp3");
            }
            break;
        case NET_NTP_GET_TIME_SUCC + 34://账号绑定成功
            music_play_res_file("BindSucc.mp3");
            keyword_wifi_enter_congfig = 0;
            break;
        default:
            break;
        }
    }

    return false;
}

int ai_speaker_device_event_handler(struct sys_event *sys_eve)
{
    int ret = false;
    extern int audio_app_mode_switch(char *name);
    struct device_event *device_eve = (struct device_event *)sys_eve->payload;
    /* SD卡插拔处理 */
    if (sys_eve->from == DEVICE_EVENT_FROM_SD) {
        switch (device_eve->event) {
        case DEVICE_EVENT_IN:
            //等待SD卡挂载完成才开始搜索文件
            //printf("->device_eve->arg : %s\n",device_eve->arg);
#if (defined CONFIG_SD_MUSIC_MODE_ENABLE && defined CONFIG_SD_AUTO_ENABLE)
            if (system_reset_reason_get() != SYS_RST_ALM_WKUP) {
                audio_app_mode_switch("sd_music");
            }
            ret = true;
#endif
            break;
        case DEVICE_EVENT_OUT:
            break;
        }
#if TCFG_UDISK_ENABLE
        /* U盘插拔处理 */
    } else if (sys_eve->from == DEVICE_EVENT_FROM_USB_HOST && !strncmp((const char *)device_eve->value, "udisk", 5)) {
        switch (device_eve->event) {
        case DEVICE_EVENT_IN:
            //printf("->device_eve->arg : %s\n",device_eve->arg);
#if (defined CONFIG_USB_DISK_MUSIC_MODE_ENABLE && defined CONFIG_USB_DISK_AUTO_ENABLE)
            if (system_reset_reason_get() != SYS_RST_ALM_WKUP) {
                audio_app_mode_switch("usbdisk_music");
            }
            ret = true;
#endif
            break;
        case DEVICE_EVENT_OUT:
            break;
        }
#endif

#ifdef CONFIG_AUX_MUSIC_MODE_ENABLE
        /* AUX插拔处理 */
    } else if (sys_eve->from == DEVICE_EVENT_FROM_LINEIN) {
        switch (device_eve->event) {
        case DEVICE_EVENT_IN:
            //printf("->device_eve->arg : %s\n",device_eve->arg);
#if (defined CONFIG_AUX_AUTO_ENABLE)
            if (system_reset_reason_get() != SYS_RST_ALM_WKUP) {
                audio_app_mode_switch("aux_music");
            }
            ret = true;
#endif
            break;
        case DEVICE_EVENT_OUT:
            break;
        }
#endif
    } else if (sys_eve->from == DEVICE_EVENT_FROM_ALM) {
        switch (device_eve->event) {
        case DEVICE_EVENT_IN://闹钟事件
            music_play_alarm(device_eve->value);
#ifdef CONFIG_LVGL_UI_ENABLE
            int lv_demo_switch_to_ring_page(void);
            lv_demo_switch_to_ring_page();
#endif
            break;
        case DEVICE_EVENT_OUT:
            break;
        }
    } else if (sys_eve->from == DEVICE_EVENT_FROM_USB_HOST && !strncmp((const char *)device_eve->value, "wireless", 8)) {
#ifdef CONFIG_LTE_PHY_ENABLE
        return lte_event_handler(device_eve);
#endif
    }

    return ret;
}

static int ai_speaker_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        return ai_speaker_key_event_handler((struct key_event *)event->payload);
    case SYS_NET_EVENT:
        return ai_net_event_handler((struct net_event *)event->payload);
    case SYS_DEVICE_EVENT:
        return ai_speaker_device_event_handler(event);
#ifdef CONFIG_BT_ENABLE
    case SYS_BT_EVENT:
        ;
        return app_music_bt_event_handler(event);
#endif
    default:
        return false;
    }
}

static int ai_speaker_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    int net_ch = sys_net_channel_read();
    int net_mode = sys_net_mode_read();
#if (defined CONFIG_WIFI_ENABLE && defined CONFIG_LTE_PHY_ENABLE)
    if (net_ch < 0) {
        net_ch = 0;
    }
#elif (defined CONFIG_LTE_PHY_ENABLE)
    net_ch = 1;
#endif
    switch (state) {
    case APP_STA_CREATE:
        break;
    case APP_STA_START:
        switch (it->action) {
        case ACTION_HOME_MAIN:
            ;
#ifdef CONFIG_WIFI_ENABLE
            if (net_ch == 0) {
                if (!wifi_is_init()) {
                    wifi_and_network_on();
                }
            }
#endif
            //初始化操作
            ai_speaker_mode_init();
#ifdef CONFIG_LTE_PHY_ENABLE
            //创建一个检测线程
            if (net_mode == NET_MODE_AUTO) {
                if (thread_fork(AI_SPEAKER_POLL_TASK, 27,   1024, 256, NULL, ai_speaker_poll_task, NULL) != OS_NO_ERR) {
                    printf("net_poll_check thread fork fail\n");
                }
            }
#endif
            break;
        case ACTION_MUSIC_PLAY_MAIN:
            break;
        case ACTION_MUSIC_PLAY_VOICE_PROMPT:
            if (!strcmp((const char *)it->data, "BtSucceed.mp3")) {
                music_play_res_file("BtSucceed.mp3");
            } else if (!strcmp((const char *)it->data, "BtDisc.mp3")) {
#ifdef BT_DISCON_MUSIC_NOTICE_ENABLE
                music_play_res_file("BtDisc.mp3");
#endif
            } else if (!strcmp((const char *)it->data, "ring.mp3")) {
                music_play_res_file("ring.mp3");
            }
            break;
        default:
            break;
        }
        break;
    case APP_STA_PAUSE:
        break;
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
        ai_speaker_mode_stop();
        //ai_speaker_mode_exit();
        break;
    case APP_STA_DESTROY:
        break;
    }

    return 0;
}

static const struct application_operation ai_speaker_ops = {
    .state_machine  = ai_speaker_state_machine,
    .event_handler  = ai_speaker_event_handler,
};

REGISTER_APPLICATION(ai_speaker) = {
    .name   = "ai_speaker",
    .ops    = &ai_speaker_ops,
    .state  = APP_STA_DESTROY,
};
#endif

