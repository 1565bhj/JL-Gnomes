#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdbool.h"
#include "gui_guider.h"
#include "custom.h"


// 用于跟踪 `麦克风按钮`, `扬声器按钮`, `蓝牙按钮` 的样式状态//默认关闭
static bool mic_state = false;
static bool speaker_state = false;
static bool BT_state = false;

// 麦克风按钮点击事件处理函数
static void dropdown_mic_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        // 切换麦克风按钮的状态
        mic_state = !mic_state;
        if(mic_state){
#ifdef APPLAYER_ENABLE
            // 开启麦克风
            aisp_all_pause(0);
#endif
        }else{
#ifdef APPLAYER_ENABLE
            // 禁用麦克风
            aisp_all_pause(1);
#endif
        }
        // 打印状态信息
        printf("-> 麦克风按钮状态: %s\n", mic_state ? "关闭" : "开启");
    }
}

// 扬声器按钮点击事件处理函数
static void dropdown_speaker_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        // 切换扬声器按钮的状态
        speaker_state = !speaker_state;
        if(!speaker_state){
#ifdef APPLAYER_ENABLE
            // 开启扬声器
            dac_mute_control(0); // 0表示取消静音
#endif
        }else{
#ifdef APPLAYER_ENABLE
            // 静音扬声器
            dac_mute_control(1); // 1表示静音
#endif
        }
        // 打印状态信息
        printf("-> 扬声器按钮状态: %s\n", speaker_state ? "静音" : "开启");
    }
}

// 蓝牙按钮点击事件处理函数
static void dropdown_bt_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        // 切换蓝牙按钮的状态
        BT_state = !BT_state;
        if(BT_state){
#ifdef APPLAYER_ENABLE
            // 开启蓝牙
            extern int audio_app_mode_switch(char *name);
            audio_app_mode_switch("bt_music");
#endif
        }else{
#ifdef APPLAYER_ENABLE
            // 关闭蓝牙
            extern int audio_app_mode_switch(char *name);
            audio_app_mode_switch(NULL);
#endif
        }
        // 打印状态信息
        printf("-> 蓝牙按钮状态: %s\n", BT_state ? "开启" : "关闭");
    }
}


// 初始化下拉框界面滑动条函数
void screen_dropdown_bars(uint8_t brightness_value,uint8_t volume_value)
{
    // 边界检查
    if(brightness_value < 10) brightness_value = 10;
    if(brightness_value > 100) brightness_value = 100;
    if(volume_value < 10) volume_value = 10;
    if(volume_value > 100) volume_value = 100;


    // if(guider_ui.screen_set_slider_volumn){
    //     lv_slider_set_value(guider_ui.screen_set_slider_volumn, volume_value, LV_ANIM_OFF);
    //     printf("[初始化] 音量滑动条已设置为: %d%%\n", volume_value);
    // }
    // if(guider_ui.screen_music_bar_volume){
    //     lv_bar_set_value(guider_ui.screen_music_bar_volume, volume_value, LV_ANIM_OFF);
    //     printf("[初始化] 音量滑动条已设置为: %d%%\n", volume_value);
    // }
    // if(guider_ui.screen_set_slider_light){
    //     lv_slider_set_value(guider_ui.screen_set_slider_light, brightness_value, LV_ANIM_OFF);
    //     printf("[初始化] 亮度滑动条已设置为: %d%%\n", brightness_value);
    // }

}
void init_screen_set_sliders(void){
    int light_value = 0;
    int volume_value = 0;
    // if(user_lcd_light_read()){
    //     light_value = user_lcd_light_read();
    //     printf("user_lcd_light_read() = %d\n",light_value);
    // }
    // if(sys_volume_read()){
    //     volume_value = sys_volume_read(NULL);
    //     printf("sys_volume_read = %d\n",volume_value);
    // }
    // screen_set_sliders(light_value,volume_value);
}



// 设置亮度函数
static void set_brightness_value(uint8_t brightness)
{
    // 边界检查
    if(brightness < 10) brightness = 10;
    if(brightness > 100) brightness = 100;

    // lcd_bl_pwm_pinstate(brightness);
    printf("[亮度设置] 亮度值已设置为: %d%%\n", brightness);

}

// 设置音量函数
static void set_volume_value(uint8_t volume)
{
    // 边界检查
    if(volume < 10) volume = 10;
    if(volume > 100) volume = 100;

    // sys_all_volume_auto_set(volume);
    printf("[音量设置] 音量值已设置为: %d%%\n", volume);

}

/**
 * @brief 下拉菜单音量条事件处理函数（统一处理所有触控相关事件）
 * @param e LVGL事件对象（包含事件类型、目标对象等信息）
 */
static void dropdown_volumn_bar_event_handler(lv_event_t *e) {
    lv_obj_t *volumn_bar = lv_event_get_target(e);  // 获取音量条对象
    lv_event_code_t event_code = lv_event_get_code(e);  // 获取当前事件类型
    
    if(event_code == LV_EVENT_VALUE_CHANGED) {
        int32_t current_value = lv_bar_get_value(volumn_bar);
         // 调用音量设置函数
        set_volume_value(current_value);
        // 打印音量值
        printf("[下拉菜单] 音量值已更改为: %d%%\n", (int)current_value);
    }
    else if(event_code == LV_EVENT_PRESSED || event_code == LV_EVENT_PRESSING) {
        // 获取当前输入设备和触摸点
        lv_indev_t *indev = lv_indev_get_act();
        if(indev == NULL) return;
        
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
        if(volume < 0) volume = 0;
        if(volume > 100) volume = 100;
        // 避免重复处理相同的值
        if (last_volume_value && (last_volume_value == volume)) {
            printf("[下拉菜单] 音量未改变\n");
            return ;
        }
        last_volume_value = volume;
        // 更新音量条值
        lv_bar_set_value(volumn_bar, volume, LV_ANIM_OFF);
        // 调用音量设置函数
        set_volume_value(volume);
        printf("[下拉菜单] 音量调节至: %d%% (触摸X: %d, 进度条宽: %d)\n", volume, relative_x, bar_width);
    }
}

/**
 * @brief 下拉菜单亮度条事件处理函数（统一处理所有触控相关事件）
 * @param e LVGL事件对象（包含事件类型、目标对象等信息）
 */
static void dropdown_light_bar_event_handler(lv_event_t *e) {
    lv_obj_t *light_bar = lv_event_get_target(e);  // 获取亮度条对象
    lv_event_code_t event_code = lv_event_get_code(e);  // 获取当前事件类型
    
    if(event_code == LV_EVENT_VALUE_CHANGED) {
        int32_t current_value = lv_bar_get_value(light_bar);
        // 调用亮度设置函数
        set_brightness_value(current_value);
        // 打印亮度值
        printf("[下拉菜单] 亮度值已更改为: %d%%\n", (int)current_value);
    }
    else if(event_code == LV_EVENT_PRESSED || event_code == LV_EVENT_PRESSING) {
        // 获取当前输入设备和触摸点
        lv_indev_t *indev = lv_indev_get_act();
        if(indev == NULL) return;
        
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
        if(brightness < 0) brightness = 0;
        if(brightness > 100) brightness = 100;
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

// 初始化下拉框事件
void dropdown_event_init(lv_ui *ui) {
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
}

void dropdown_event_setup(void){
    dropdown_event_init(&guider_ui);
}
 
