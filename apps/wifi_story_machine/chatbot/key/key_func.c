#include "system/includes.h"
#include "asm/gpio.h"
#include "app_config.h"
#include "storage_device.h"
#include "generic/log.h"
#include "os/os_api.h"
#include "event/key_event.h"
#include "event/device_event.h"
#include "event/net_event.h"
#include "fs/fs.h"
#include "asm/pwm.h"
#include "device/device.h"
#include "ai_uart_ctrol.h"

/* ===================== 按键功能宏（示例） =====================
 * 每个 KEYx_FUNC 为 32 位整型，位域分配：
 *   bit0-3   : 短按
 *   bit4-7   : 长按
 *   bit8-11  : 双击
 *   bit12-15 : 三击
 *   bit16-19 : 四击
 *   bit20-23 : 五击
 *   bit24-27 : 六击以上
 * ============================================================ */
#ifndef ADKEY_KEY1_FUNC
#define ADKEY_KEY1_RFUNC    0x01011111
#else
#define ADKEY_KEY1_RFUNC    0x01011111
#endif

#ifndef ADKEY_KEY2_FUNC
#define ADKEY_KEY2_FUNC    0x00000000
#endif

#ifndef ADKEY_KEY3_FUNC
#define ADKEY_KEY3_FUNC    0x00000000
#endif

#ifndef ADKEY_KEY4_FUNC
#define ADKEY_KEY4_FUNC    0x00000000
#endif

#ifndef ADKEY_KEY5_FUNC
#define ADKEY_KEY5_FUNC    0x00000000
#endif

#ifndef ADKEY_KEY6_FUNC
#define ADKEY_KEY6_FUNC    0x00000000
#endif

/* 根据 key_val 取对应功能宏 */
static uint32_t get_key_func(int key_val)
{
    switch (key_val) {
    case KEY_1:
        return ADKEY_KEY1_RFUNC;
    case KEY_2:
        return ADKEY_KEY2_FUNC;
    case KEY_3:
        return ADKEY_KEY3_FUNC;
    case KEY_4:
        return ADKEY_KEY4_FUNC;
    case KEY_5:
        return ADKEY_KEY5_FUNC;
    case KEY_6:
        return ADKEY_KEY6_FUNC;
    default:
        return 0;
    }
}

/* 从 32 位功能值中取对应位段 */
static int get_field(uint32_t func, int lo, int hi)
{
    uint32_t mask = ((1u << (hi - lo + 1)) - 1u);
    return (int)((func >> lo) & mask);
}

/* ===================== 功能执行函数（示例） ===================== */
int key_do_short_action(int act)
{
    switch (act) {
    case KEY_SHORT_WAKEUP:      /* 短按唤醒 */
        if (system_keyword_start()) {
            aisp_clear(0);
        }
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
    case KEY_SHORT_NEXT_SONG:   /* 短按下一首 */
        net_music_play_next(net_music_play_type_get());
        break;
    case KEY_SHORT_PREV_SONG:   /* 短按上一首 */
        if (net_music_play_last_check()) {
            net_music_play_last_request();
        } else {
            net_music_play_next(net_music_play_type_get());
        }
        break;
    case KEY_SHORT_LED_MODE:    /* 短按灯模式切换 */
#ifdef  USED_WS2812B_SHOWN
        ws2812_shown_all_auto_mode();
#endif
#if (defined TCFG_LED_PWM0_PORT || defined TCFG_LED_PWM0_CH)
        tm_light_pwm_auto_decinc_openclose();
#endif
        break;
    case KEY_SHORT_MODE_SWITCH:   /* 短按模式切换 */
        audio_app_mode_switch(NULL);
#ifdef CONFIG_UI_PLAY_EMOJI
        extern int play_face_emoji(int avi_index);
        if (audio_app_mode_check() == 1) {
            play_face_emoji(AI_UART_CMD_BT_MODE);
        }
#endif
        break;
    case KEY_SHORT_MUSIC_SWITCH:   /* 短按暂停/播放音乐 */
        if (music_buf_play_supspend()) {
            net_music_play_resum();
        } else if (net_music_play_start_status()) {
            net_music_play_puase();
        } else {
            net_music_play_next(net_music_play_type_get());
        }
        break;
    default:
        return false;
    }
    return true;
}

int key_do_long_action(int last_key, int act)
{
    char play = 1;
    u8 volume = 0;
    switch (act) {
    case KEY_LONG_POWER:        /* 长按开关机 */
        aisp_all_pause(1);
        ai_speaker_mode_exit();
        music_play_stop_all();
        music_play_waite();
#ifdef CONFIG_UI_PLAY_EMOJI
        play_face_emoji(AI_UART_CMD_PWR_OFF);
#endif
        music_play_res_file("PwrOff.mp3");
        music_play_waite();
        sys_power_poweroff(0);
        break;
    case KEY_LONG_VOL_UP:       /* 长按音量加 */
        if (net_music_set_dec_volume_step(VOLUME_STEP) && music_buf_play_set_volume_step(VOLUME_STEP)
            && music_play_set_volume_step(VOLUME_STEP) && http_pipe_music_play_set_volume_step(VOLUME_STEP)) {
            sys_volume_write_step(VOLUME_STEP);
            play = 0;//没有播放音频
        }
        sys_volume_read(&volume);
        if (volume > 0 && volume <= 100 && play) {
            aisp_mic_gain_set(volume);
        }
        break;
    case KEY_LONG_VOL_DOWN:     /* 长按音量减 */
        if (net_music_set_dec_volume_step(-VOLUME_STEP) && music_buf_play_set_volume_step(-VOLUME_STEP)
            && music_play_set_volume_step(-VOLUME_STEP) && http_pipe_music_play_set_volume_step(-VOLUME_STEP)) {
            sys_volume_write_step(-VOLUME_STEP);
            play = 0;//没有播放音频
        }
        sys_volume_read(&volume);
        if (volume > 0 && volume <= 100 && play) {
            aisp_mic_gain_set(volume);
        }
        break;
    case KEY_LONG_4G_WIFI:      /* 长按4G/WIFI模式 */
        int cur_net_mode = sys_net_mode_read();
        int ch = sys_net_channel_read();
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
        }
        break;
    case KEY_LONG_NET_CFG:      /* 长按配网模式 */
        if (((last_key >> 8) & 0xFF) != KEY_EVENT_LONG) {
            aisp_wake(8);//WIFI模式下按键进入配网
        }
        break;
    case KEY_LONG_KEY_VAD:      /* 长按对话 */
        if (((last_key >> 8) & 0xFF) != KEY_EVENT_LONG) {
            printf("-->KEY_EVENT_LONG key vad start\n");
            key_vad_pcm_send_set_status(1, 1);
        }
        break;
    case KEY_LONG_LED_ONOFF:    /* 长按开关灯 */
        if (((last_key >> 8) & 0xFF) != KEY_EVENT_LONG) {
#ifdef  USED_WS2812B_SHOWN
            ws2812_shown_onoff_auto();
#endif
        }
        break;
    default:
        return false;
    }
    return true;
}

/* ===================== 抬手（UP）处理 ===================== */
int key_do_handup(int last_key, int act)
{
    switch (act) {
    case KEY_LONG_NET_CFG:      /* 长按配网模式抬手 */
        break;
    case KEY_LONG_KEY_VAD:      /* 长按对话抬手 */
        if (((last_key >> 8) & 0xFF) == KEY_EVENT_LONG) {
            printf("-->KEY_EVENT_LONG key vad stop\n");
            key_vad_pcm_send_set_status(0, 1);
        }
        break;
    default:
        return false;
    }
    return true;
}

int key_do_double_action(int act)
{
    switch (act) {
    case KEY_DOUBLE_MODE_SWITCH: /* 双击模式切换 */
        audio_app_mode_switch(NULL);
#ifdef CONFIG_UI_PLAY_EMOJI
        extern int play_face_emoji(int avi_index);
        if (audio_app_mode_check() == 1) {
            play_face_emoji(AI_UART_CMD_BT_MODE);
        }
#endif
        break;
    case KEY_DOUBLE_NET_CFG:
        aisp_wake(8);//WIFI模式下按键进入配网
        break;
    default:
        return false;
    }
    return true;
}

int key_do_triple_action(int act)
{
    switch (act) {
    case KEY_TRIPLE_NET_CFG:     /* 三击配网模式 */
        aisp_wake(8);//WIFI模式下按键进入配网
        break;
    default:
        return false;
    }
    return true;
}

int key_do_fourth_action(int act)
{
    switch (act) {
    case KEY_FOURTH_FACTORY_TEST:  /* 四击厂测模式 */
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
        break;
    case KEY_FOURTH_4G_WIFI:;       /* 四击4G/WIFI模式 */
#ifdef CONFIG_LTE_PHY_ENABLE
        int cur_net_mode = sys_net_mode_read();
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
        }
#endif
        break;
    default:
        return false;
    }
    return true;
}

int key_do_five_action(int act)
{
    switch (act) {
    case KEY_FIVE_FACTORY_TEST:  /* 五击厂测模式 */
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
        break;
    default:
        return false;
    }
    return true;
}

int key_do_many_action(int act)
{
    switch (act) {
    case KEY_MANY_FACTORY_RESET: /* 六击以上恢复出厂设置 */
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
        break;
    default:
        return false;
    }
    return true;
}

/* ===================== 主函数 ===================== */
int key_func_set(int last_key, int key_action, int key_val)
{
    uint32_t func = get_key_func(key_val);
    int act = 0;

    switch (key_action) {
    case KEY_EVENT_UP:             /* 释放检测 */
        act = get_field(func, 4, 7);//只拿长按事件
        return key_do_handup(last_key, act);
    case KEY_EVENT_CLICK:          /* 短按 */
        act = get_field(func, 0, 3);
        return key_do_short_action(act);
    case KEY_EVENT_LONG:           /* 长按 */
        act = get_field(func, 4, 7);
        return key_do_long_action(last_key, act);
    case KEY_EVENT_DOUBLE_CLICK:   /* 双击 */
        act = get_field(func, 8, 11);
        return key_do_double_action(act);
    case KEY_EVENT_TRIPLE_CLICK:   /* 三击 */
        act = get_field(func, 12, 15);
        return key_do_triple_action(act);
    case KEY_EVENT_FOURTH_CLICK:   /* 四击 */
        act = get_field(func, 16, 19);
        return key_do_fourth_action(act);
    case KEY_EVENT_FIRTH_CLICK:    /* 五击 */
        act = get_field(func, 20, 23);
        return key_do_five_action(act);
    case KEY_EVENT_SIX_CLICK:      /* 六击 */
    case KEY_EVENT_MORE_CLICK:     /* 六击以上 */
        act = get_field(func, 24, 27);
        return key_do_many_action(act);
    default:
        return false;
    }
}


#ifdef KEY_DATA_PLAN_REST_ENABLE
int key_press_123(int action_value)
{
    static int start_ms = 0;
    static char start_cnt = 0;
    unsigned char action = action_value & 0xFF;
    unsigned char value = (action_value >> 8) & 0xFF;
#define KEY_PRESS_123_VAL KEY_OK
    switch (action) {
    case KEY_EVENT_CLICK:
        if (value == KEY_PRESS_123_VAL) {
            start_ms = timer_get_ms();
            start_cnt = 1;
        }
        break;
    case KEY_EVENT_DOUBLE_CLICK:
        if (value == KEY_PRESS_123_VAL && start_cnt == 1 && (timer_get_ms() - start_ms) < 5000) {
            start_cnt = 2;
            start_ms = timer_get_ms();
        }
        break;
    case KEY_EVENT_TRIPLE_CLICK:
        if (value == KEY_PRESS_123_VAL && start_cnt == 2 && (timer_get_ms() - start_ms) < 5000) {
            user_data_plan_reset_config(0, 1);
            music_play_stop_all();
            websockets_client_dialogue_exit();
            websockets_close_request(1);
            websockets_dialogue_timeout_del();
            music_buf_play_stop_all();
            music_buf_play_set_stop();
            websockets_free_lbuf_buf();
            music_buf_play_free_lbuf();
            music_play_res_file("KeyVol.mp3");
            return true;
        }
        break;
    default :
        start_ms = 0;
        start_cnt = 0;
        break;
    }
    return false;
}
#endif // KEY_DATA_PLAN_REST
