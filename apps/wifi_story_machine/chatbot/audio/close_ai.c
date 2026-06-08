#include "system/app_core.h"
#include "system/includes.h"
#include "asm/system_reset_reason.h"
#include "os/os_api.h"
#include "event/device_event.h"
#include "app_config.h"
#include "action.h"
#include "event/key_event.h"
#include "event/net_event.h"
#include "btstack/avctp_user.h"
#include "event/bt_event.h"
#include "lwip/netdb.h"
#include "server/ai_server.h"

#if defined CONFIG_ASR_ALGORITHM_ENABLE

#ifndef VOLUME_STEP
#define VOLUME_STEP 10
#endif

static struct ap_sp {
    unsigned short last_key;
} CLOSE_AI_SP = {0};
#define __this (&CLOSE_AI_SP)

static int CloseAIMode_key_click(struct key_event *key)
{
    int ret = true;
    int res;
    int play = 1;
    u8 volume = 0;

    printf("-->CloseAIMode_key_click  = %d \n", key->value);
    switch (key->value) {
    case KEY_MUTE:
    case KEY_OK://按键播放或者启动识别
#if (SXY_LL_YHH_BOARD)
        //ai_speaker_mode_stop();
        extern int audio_app_mode_switch(char *name);
        audio_app_mode_switch(NULL);
#else
//        if(net_music_play_start_status()){
//            net_music_play_puase();
//        }else{
//            aisp_wake(0);
//        }
#endif
        break;
    case KEY_DOWN:
        break;
    case KEY_UP:
        break;
    case KEY_MODE:
#if (SXY_LL_YHH_BOARD)
//        if(net_music_play_start_status()){
//            net_music_play_puase();
//        }else{
//            aisp_wake(0);
//        }
#else
//        ai_speaker_mode_stop();
        extern int audio_app_mode_switch(char *name);
        audio_app_mode_switch(NULL);
#endif
        break;
    case KEY_VOLUME_DEC:
        break;
    case KEY_VOLUME_INC:
        break;
    case KEY_POWER:;//唤醒
        aisp_wake(0);
        break;
    case KEY_SUPSPEND:
        if (!music_buf_play_supspend()) {
            if (!(music_buf_play_stop_staus() && music_play_stop_status())) {
                net_music_play_puase();
            }
        }
        break;
    case KEY_RESUM:
        if (music_buf_play_supspend()) {
            net_music_play_resum();
        }
    case KEY_CPU_RESET:
        ret = false;
        break;
    default:
        ret = false;
        break;
    }

    return ret;
}

static int CloseAIMode_key_long(struct key_event *key)
{
    int ret = true;
    int play = 1;
    u8 volume = 0;
    printf("-->CloseAIMode_key_long  = %d \n", key->value);
    switch (key->value) {
    case KEY_MODE:
        ;
        break;
    case KEY_VOLUME_DEC:
    case KEY_UP:
        if (music_buf_play_set_volume_step(-VOLUME_STEP)
            && music_play_set_volume_step(-VOLUME_STEP)
            && net_music_set_dec_volume_step(-VOLUME_STEP)) {
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
    case KEY_DOWN:
        if (music_buf_play_set_volume_step(VOLUME_STEP)
            && music_play_set_volume_step(VOLUME_STEP)
            && net_music_set_dec_volume_step(VOLUME_STEP)) {
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
        music_play_waite();
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
static int CloseAIMode_key_hand_up(struct key_event *key)
{
    int ret = true;
    int play = 1;
    u8 volume = 0;
    printf("-->CloseAIMode_key_hand_up  = %d \n", key->value);

    switch (key->value) {
    case KEY_MODE:
        ;
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

static int CloseAIMode_key_event_handler(struct key_event *key)
{
    int ret = 0;
    auto_sleep_check_clear();
    if (!alarm_music_play_stop() && (key->value != KEY_SUPSPEND && key->value != KEY_RESUM && key->value != KEY_POWER)) {
        return true;//闹钟在响，按键失效
    }
    printf("->key->action = %d ,key->value = %d \n", key->action, key->value);
    switch (key->action) {
    case KEY_EVENT_UP:
        CloseAIMode_key_hand_up(key);
        break;
    case KEY_EVENT_CLICK:
        CloseAIMode_key_click(key);
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
        CloseAIMode_key_long(key);
        break;
    case KEY_EVENT_DOUBLE_CLICK:
        if (key->value == KEY_OK) { //模式切换
#if (SXY_LL_YHH_BOARD)
            extern int tm_light_pwm_auto(void);
            tm_light_pwm_auto();
#elif (!defined TCFG_ADKEY_ENABLE || TCFG_ADKEY_ENABLE == 0)//不是AD按键支持双击模式切换
            //ai_speaker_mode_stop();
            extern int audio_app_mode_switch(char *name);
            audio_app_mode_switch(NULL);
#endif
        }
#ifdef  USED_TM1629_SHOWN
        else if (key->value == KEY_MODE) { //显示亮度切换
            int tm_1629_led_shown_bright_level_auto(void);
            tm_1629_led_shown_bright_level_auto();
        }
#endif
        break;
    case KEY_EVENT_TRIPLE_CLICK:
        break;
    case KEY_EVENT_FOURTH_CLICK:
        break;
    case KEY_EVENT_FIRTH_CLICK:;//五连击
#ifdef CONFIG_WFT_ENBALE
        extern int dev_iot_uart_task_init(void);
        dev_iot_uart_task_init();
#endif
        break;
    case KEY_EVENT_MORE_CLICK://六连击以上
        if (key->value == KEY_OK) {
            music_play_res_file("SysRstFaSet.mp3");
            system_restore_factory_settings();//恢复出厂设置
            music_play_waite();
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
static int CloseAIMode_device_event_handler(struct sys_event *sys_eve)
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
    }
    return ret;
}
static int CloseAIMode_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        return CloseAIMode_key_event_handler((struct key_event *)event->payload);
    case SYS_NET_EVENT:
    case SYS_DEVICE_EVENT:
    case SYS_BT_EVENT:
    default:
        return false;
    }
}

static int CloseAIMode_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;
    case APP_STA_START:
        ;
        ai_speaker_mode_exit();
        break;
    case APP_STA_PAUSE:
        break;
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
        break;
    case APP_STA_DESTROY:
        break;
    }
    return 0;
}

static const struct application_operation CloseAIMode_ops = {
    .state_machine  = CloseAIMode_state_machine,
    .event_handler  = CloseAIMode_event_handler,
};

REGISTER_APPLICATION(CloseAIMode) = {
    .name   = "CloseAIMode",
    .ops    = &CloseAIMode_ops,
    .state  = APP_STA_DESTROY,
};

#endif
