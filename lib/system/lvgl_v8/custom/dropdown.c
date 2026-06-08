#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdbool.h"
#include "gui_guider.h"
#include "custom.h"
#include "update/net_update.h"
/* 音量/亮度条较细，四向扩大可点区域（LVGL 在命中测试时会并入此边距） */
#ifndef DROPDOWN_BAR_EXT_CLICK_PAD
#define DROPDOWN_BAR_EXT_CLICK_PAD  5
#endif

void img_nomute_is_show(bool muted)
{
#if 0
#if UI_SHOW_SCREEN_MAIN_NOMUTE_ICON
    if (guider_ui.screen_main_img_nomute) {
        if (muted) {
            lv_obj_clear_flag(guider_ui.screen_main_img_nomute, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(guider_ui.screen_main_img_nomute, LV_OBJ_FLAG_HIDDEN);
        }
    }
#endif
#if UI_SHOW_SCREEN_DROPDOWN_NOMUTE_ICON
    if (guider_ui.screen_dropdown_img_nomute) {
        if (muted) {
            lv_obj_clear_flag(guider_ui.screen_dropdown_img_nomute, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(guider_ui.screen_dropdown_img_nomute, LV_OBJ_FLAG_HIDDEN);
        }
    }
#endif
#endif
}
// 麦克风按钮点击事件处理函数
static void dropdown_mic_event_handler(lv_event_t *e)
{
    static bool mic_state = false;
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        // 切换麦克风按钮的状态
        mic_state = !mic_state;
        if (!mic_state) {
#ifdef APPLAYER_ENABLE
            // 开启麦克风
            aisp_resume();
#endif
        } else {
#ifdef APPLAYER_ENABLE
            // 禁用麦克风
            aisp_suspend();
#endif
        }
        // 打印状态信息
        printf("-> 麦克风按钮状态: %s\n", mic_state ? "关闭" : "开启");
    }
}

// 扬声器按钮点击事件处理函数
static void dropdown_speaker_event_handler(lv_event_t *e)
{
    static bool speaker_state = false;
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        // 切换扬声器按钮的状态
        speaker_state = !speaker_state;
        if (!speaker_state) {
#ifdef APPLAYER_ENABLE
            // 开启扬声器
            dac_mute_control(0, 1); // 0表示取消静音
#endif
        } else {
#ifdef APPLAYER_ENABLE
            // 静音扬声器
            dac_mute_control(1, 1); // 1表示静音
#endif
        }
        img_nomute_is_show(speaker_state);
        // 打印状态信息
        printf("-> 扬声器按钮状态: %s\n", speaker_state ? "静音" : "开启");
    }
}

// 蓝牙按钮点击事件处理函数
static void dropdown_bt_event_handler(lv_event_t *e)
{
    static bool BT_state = false;
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        // 切换蓝牙按钮的状态
        int audio_app_mode_check(void);
        BT_state = audio_app_mode_check() == 1;
        if (!BT_state) {
#ifdef APPLAYER_ENABLE
            // 开启蓝牙
            extern int audio_app_mode_switch(char *name);
            audio_app_mode_switch("bt_music");
#endif
        } else {
#ifdef APPLAYER_ENABLE
            // 关闭蓝牙
            extern int audio_app_mode_switch(char *name);
            audio_app_mode_switch("ai_speaker");
#endif
        }
        // 打印状态信息
        printf("-> 蓝牙按钮状态: %s\n", BT_state ? "开启" : "关闭");
    }
}


// 初始化下拉框界面滑动条函数
void screen_dropdown_bars(uint8_t brightness_value, uint8_t volume_value)
{
    // 边界检查
    if (brightness_value < 10) {
        brightness_value = 10;
    }
    if (brightness_value > 100) {
        brightness_value = 100;
    }
    if (volume_value < 10) {
        volume_value = 10;
    }
    if (volume_value > 100) {
        volume_value = 100;
    }


    if (guider_ui.screen_dropdown_volumn_bar) {
        lv_bar_set_value(guider_ui.screen_dropdown_volumn_bar, volume_value, LV_ANIM_OFF);
        printf("[初始化] 音量条已设置为: %d%%\n", volume_value);
    }
    if (guider_ui.screen_music_bar_volume) {
        lv_bar_set_value(guider_ui.screen_music_bar_volume, volume_value, LV_ANIM_OFF);
        printf("[初始化] 音乐页音量条已设置为: %d%%\n", volume_value);
    }
    if (guider_ui.screen_dropdown_light_bar) {
        lv_bar_set_value(guider_ui.screen_dropdown_light_bar, brightness_value, LV_ANIM_OFF);
        printf("[初始化] 亮度条已设置为: %d%%\n", brightness_value);
    }

}

/**
 * 与硬件亮度/音量同步 LVGL 条。volume_value 为 0 时只更新亮度条（如语音/按键调背光）。
 */
void screen_set_sliders(int brightness_value, int volume_value)
{
    if (brightness_value < 10) {
        brightness_value = 10;
    }
    if (brightness_value > 100) {
        brightness_value = 100;
    }
    if (guider_ui.screen_dropdown_light_bar) {
        lv_bar_set_value(guider_ui.screen_dropdown_light_bar, brightness_value, LV_ANIM_OFF);
    }

    if (volume_value <= 0) {
        return;
    }
    if (volume_value < 10) {
        volume_value = 10;
    }
    if (volume_value > 100) {
        volume_value = 100;
    }
    if (guider_ui.screen_dropdown_volumn_bar) {
        lv_bar_set_value(guider_ui.screen_dropdown_volumn_bar, volume_value, LV_ANIM_OFF);
    }
    if (guider_ui.screen_music_bar_volume) {
        lv_bar_set_value(guider_ui.screen_music_bar_volume, volume_value, LV_ANIM_OFF);
    }
}

void init_screen_set_sliders(void)
{
    int light_value = 0;
    int volume_value = 0;
    if (user_lcd_light_read()) {
        light_value = user_lcd_light_read();
        printf("user_lcd_light_read() = %d\n", light_value);
    }
    if (sys_volume_read()) {
        volume_value = sys_volume_read(NULL);
        printf("sys_volume_read = %d\n", volume_value);
    }
    screen_dropdown_bars(light_value, volume_value);
}

void custom_ui_bars_set(int brightness_pct, int volume_pct, int music_playback_pct)
{
    if (brightness_pct != -1) {
        int b = brightness_pct;
        if (b < 10) {
            b = 10;
        }
        if (b > 100) {
            b = 100;
        }
        if (guider_ui.screen_dropdown_light_bar) {
            lv_bar_set_value(guider_ui.screen_dropdown_light_bar, b, LV_ANIM_OFF);
        }
    }
    if (volume_pct != -1) {
        int v = volume_pct;
        if (v < 10) {
            v = 10;
        }
        if (v > 100) {
            v = 100;
        }
        if (guider_ui.screen_dropdown_volumn_bar) {
            lv_bar_set_value(guider_ui.screen_dropdown_volumn_bar, v, LV_ANIM_OFF);
        }
        if (guider_ui.screen_music_bar_volume) {
            lv_bar_set_value(guider_ui.screen_music_bar_volume, v, LV_ANIM_OFF);
        }
    }
    if (music_playback_pct != -1) {
        int p = music_playback_pct;
        if (p < 0) {
            p = 0;
        }
        if (p > 100) {
            p = 100;
        }
        if (guider_ui.screen_music_slider) {
            lv_bar_set_value(guider_ui.screen_music_slider, p, LV_ANIM_OFF);
        }
    }
}

void custom_ui_volume_bars_set(int volume_pct)
{
    custom_ui_bars_set(-1, volume_pct, volume_pct);
}

// 设置亮度函数
static void set_brightness_value(uint8_t brightness)
{
    // 边界检查
    if (brightness < 10) {
        brightness = 10;
    }
    if (brightness > 100) {
        brightness = 100;
    }

    lcd_bl_pinstate(brightness);
    printf("[亮度设置] 亮度值已设置为: %d%%\n", brightness);

}

// 设置音量函数
static void set_volume_value(uint8_t volume)
{
    // 边界检查
    if (volume < 10) {
        volume = 10;
    }
    if (volume > 100) {
        volume = 100;
    }

    sys_all_volume_auto_set(volume);
    printf("[音量设置] 音量值已设置为: %d%%\n", volume);
}

/**
 * @brief 下拉菜单音量条事件处理函数（统一处理所有触控相关事件）
 * @param e LVGL事件对象（包含事件类型、目标对象等信息）
 */
static void dropdown_volumn_bar_event_handler(lv_event_t *e)
{
    lv_obj_t *volumn_bar = lv_event_get_target(e);  // 获取音量条对象
    lv_event_code_t event_code = lv_event_get_code(e);  // 获取当前事件类型

    if (event_code == LV_EVENT_VALUE_CHANGED) {
        int32_t current_value = lv_bar_get_value(volumn_bar);
        // 调用音量设置函数
        set_volume_value(current_value);
        // 打印音量值
        printf("[下拉菜单] 音量值已更改为: %d%%\n", (int)current_value);
    } else if (event_code == LV_EVENT_PRESSED || event_code == LV_EVENT_PRESSING) {
        // 获取当前输入设备和触摸点
        lv_indev_t *indev = lv_indev_get_act();
        if (indev == NULL) {
            return;
        }

        lv_point_t point;
        lv_indev_get_point(indev, &point);

        // 获取进度条的位置和尺寸
        lv_area_t bar_area;
        lv_obj_get_coords(volumn_bar, &bar_area);
        int32_t bar_width = lv_area_get_width(&bar_area); // 使用LVGL提供的函数获取宽度
        int32_t relative_x = point.x - bar_area.x1;
        int32_t volume = (relative_x * 100 / bar_width);

        static uint8_t last_volume_value = 0;
        // 限制音量范围
        if (volume < 10) {
            volume = 10;
        }
        if (volume > 100) {
            volume = 100;
        }
        // 避免重复处理相同的值
        if (last_volume_value && (last_volume_value == volume)) {
            printf("[下拉菜单] 音量未改变\n");
            return ;
        }
        last_volume_value = volume;
        // 更新音量条值
        lv_bar_set_value(volumn_bar, volume, LV_ANIM_OFF);
        lv_bar_set_value(guider_ui.screen_music_bar_volume, volume, LV_ANIM_OFF);
        // 调用音量设置函数
        set_volume_value(volume);
        printf("[下拉菜单] 音量调节至: %d%% (触摸X: %d, 进度条宽: %d)\n", volume, relative_x, bar_width);
    }
}

/**
 * @brief 下拉菜单亮度条事件处理函数（统一处理所有触控相关事件）
 * @param e LVGL事件对象（包含事件类型、目标对象等信息）
 */
static void dropdown_light_bar_event_handler(lv_event_t *e)
{
    lv_obj_t *light_bar = lv_event_get_target(e);  // 获取亮度条对象
    lv_event_code_t event_code = lv_event_get_code(e);  // 获取当前事件类型

    if (event_code == LV_EVENT_VALUE_CHANGED) {
        int32_t current_value = lv_bar_get_value(light_bar);
        // 调用亮度设置函数
        set_brightness_value(current_value);
        // 打印亮度值
        printf("[下拉菜单] 亮度值已更改为: %d%%\n", (int)current_value);
    } else if (event_code == LV_EVENT_PRESSED || event_code == LV_EVENT_PRESSING) {
        // 获取当前输入设备和触摸点
        lv_indev_t *indev = lv_indev_get_act();
        if (indev == NULL) {
            return;
        }

        lv_point_t point;
        lv_indev_get_point(indev, &point);

        // 获取进度条的位置和尺寸
        lv_area_t bar_area;
        lv_obj_get_coords(light_bar, &bar_area);
        int32_t bar_width = lv_area_get_width(&bar_area); // 使用LVGL提供的函数获取宽度
        int32_t relative_x = point.x - bar_area.x1;
        int32_t brightness = (relative_x * 100 / bar_width);

        static uint8_t last_brightness_value = 0;
        // 限制亮度范围
        if (brightness < 10) {
            brightness = 10;
        }
        if (brightness > 100) {
            brightness = 100;
        }
        // 避免重复处理相同的值
        if (last_brightness_value && (last_brightness_value == brightness)) {
            printf("[下拉菜单] 亮度未改变\n");
            return ;
        }
        last_brightness_value = brightness;

        // 更新亮度条值
        lv_bar_set_value(light_bar, brightness, LV_ANIM_OFF);
        // 调用亮度设置函数
        set_brightness_value(brightness);
        printf("[下拉菜单] 亮度调节至: %d%% (触摸X: %d, 进度条宽: %d)\n", brightness, relative_x, bar_width);
    }
}



#if 0
/** 将下拉页「设备版本」标签更新为当前 OTA 版本号 */
static void dropdown_refresh_device_version_label(lv_ui *ui)
{
    if (!ui || !ui->screen_dropdown_label_version) {
        return;
    }
    lv_label_set_text_fmt(ui->screen_dropdown_label_version, "设备版本：%s",
                          net_update_get_ota_version());
}

static void dropdown_widen_bar_touch_targets(lv_ui *ui)
{
    if (!ui) {
        return;
    }
    if (ui->screen_dropdown_volumn_bar) {
        lv_obj_set_ext_click_area(ui->screen_dropdown_volumn_bar, DROPDOWN_BAR_EXT_CLICK_PAD);
    }
    if (ui->screen_dropdown_light_bar) {
        lv_obj_set_ext_click_area(ui->screen_dropdown_light_bar, DROPDOWN_BAR_EXT_CLICK_PAD);
    }
}
#endif

// 初始化下拉框事件
void dropdown_event_init(lv_ui *ui)
{
#if 0
    lv_obj_set_scroll_dir(ui->screen_dropdown, LV_DIR_NONE);
    dropdown_refresh_device_version_label(ui);
#endif
    // 注册麦克风按钮点击事件
    lv_obj_remove_event_cb(ui->screen_dropdown_imgbtn_mic, dropdown_mic_event_handler);
    lv_obj_add_event_cb(ui->screen_dropdown_imgbtn_mic, dropdown_mic_event_handler, LV_EVENT_CLICKED, NULL);

    // 注册扬声器按钮点击事件
    lv_obj_remove_event_cb(ui->screen_dropdown_imgbtn_speaker, dropdown_speaker_event_handler);
    lv_obj_add_event_cb(ui->screen_dropdown_imgbtn_speaker, dropdown_speaker_event_handler, LV_EVENT_CLICKED, NULL);

    // 注册蓝牙按钮点击事件
    lv_obj_remove_event_cb(ui->screen_dropdown_imgbtn_BT, dropdown_bt_event_handler);
    lv_obj_add_event_cb(ui->screen_dropdown_imgbtn_BT, dropdown_bt_event_handler, LV_EVENT_CLICKED, NULL);

    // 注册音量条拖动事件
    lv_obj_remove_event_cb(ui->screen_dropdown_volumn_bar, NULL);
    lv_obj_add_event_cb(ui->screen_dropdown_volumn_bar, dropdown_volumn_bar_event_handler, LV_EVENT_ALL, NULL);

    // 注册亮度条拖动事件
    lv_obj_remove_event_cb(ui->screen_dropdown_light_bar, NULL);
    lv_obj_add_event_cb(ui->screen_dropdown_light_bar, dropdown_light_bar_event_handler, LV_EVENT_ALL, NULL);
#if 0
    dropdown_widen_bar_touch_targets(ui);
    img_nomute_is_show(false);
#endif
}

void dropdown_event_setup(void)
{
    dropdown_event_init(&guider_ui);
}

