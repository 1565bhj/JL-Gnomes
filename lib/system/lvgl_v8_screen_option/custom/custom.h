/*
* Copyright 2023 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef __CUSTOM_H_
#define __CUSTOM_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"
#include "lvgl_develop.h"

#if CONFIG_LVGL_CONTENT_LIST_296_ENABLE
// 图标位置宏定义
#define WEATHER_ICON_X           67   /* 与参考工程 setup_scr_screen_main 初始太阳图标 X 一致；运行时由 screen_main_layout_weather_row_centered 居中 */
#define WEATHER_ICON_Y           195
#define ALARM_ICON_X             178  /* screen_main_img_no_alarm，与参考工程一致 */
#define ALARM_ICON_Y             19
#define WIFI_ICON_X              224  /* screen_main_img_no_wifi */
#define WIFI_ICON_Y              18
#define BLUETOOTH_ICON_X         202  /* screen_main_img_no_BT */
#define BLUETOOTH_ICON_Y         19
#define BATTERY_ICON_X           250  /* screen_main_img_normol_battery 等 */
#define BATTERY_ICON_Y           19

/* 主界面左上角：位置 pin（screen_main_img_location）与城市/地址（screen_main_span_direction） */
#define LOCATION_ICON_X          10
#define LOCATION_ICON_Y          11
#define CITY_ADDRESS_SPAN_X      29
#define CITY_ADDRESS_SPAN_Y      10

#else

// 图标位置宏定义
#define WEATHER_ICON_X           67   /* 与参考工程 setup_scr_screen_main 初始太阳图标 X 一致；运行时由 screen_main_layout_weather_row_centered 居中 */
#define WEATHER_ICON_Y           195

#define ALARM_ICON_X             224
#define ALARM_ICON_Y             10
#define WIFI_ICON_X              266
#define WIFI_ICON_Y              10
#define BLUETOOTH_ICON_X         246
#define BLUETOOTH_ICON_Y         10
#define BATTERY_ICON_X           292
#define BATTERY_ICON_Y           11

/* 主界面左上角：位置 pin（screen_main_img_location）与城市/地址（screen_main_span_direction） */
#define LOCATION_ICON_X          10
#define LOCATION_ICON_Y          5
#define CITY_ADDRESS_SPAN_X      29
#define CITY_ADDRESS_SPAN_Y      4

#endif

#define UI_SHOW_SCREEN_MAIN_NOMUTE_ICON   1

#if UI_SHOW_SCREEN_MAIN_NOMUTE_ICON
#define NOMUTE_ICON_X            204
#define NOMUTE_ICON_Y            14
#endif


#ifdef CONFIG_LVGL_UI_ENABLE
#define APPLAYER_ENABLE 1
#endif


/** 子页面「返回」四边等距扩大的触控区（像素）；实现为放大 obj 并设 pad，不依赖 ext_click（LV8 会搜不到父 coords 外扩区） */
#ifndef UI_SUBPAGE_RETURN_EXT_PAD
#define UI_SUBPAGE_RETURN_EXT_PAD  50
#endif


void custom_init(lv_ui *ui);

// 页面切换函数声明
void switch_to_alarm_page(void);
void switch_to_alarm_add_page(void);
void switch_to_alarm_set_page(void);
void switch_to_alarm_remove_page(void);
void switch_to_timer_page(void);
void switch_to_timer_add_page(void);
void switch_to_timer_remove_page(void);
// 音乐播放模式结构体
typedef struct {
    uint8_t mode_id;     // 模式ID
    char name[20];       // 模式名称
    char mode_status[20]; // 模式状态
    bool is_clicked;     // 是否点击
} music_play_mode_t;

// 定义播放模式常量（与 apps/wifi_story_machine/chatbot/audio/mode.c 中 audio_app_mode_check 返回值一致）
#define MODE_NET_ID 0    // 网络模式ID
#define MODE_BT_ID 1     // 蓝牙模式ID
#define MODE_SD_ID 2     // SD卡模式ID
#define MODE_USB_ID 3    // U盘模式ID
#define MODE_AUX_ID 4    // 线路输入(AUX)模式ID

// 外部声明音乐播放模式变量
extern music_play_mode_t g_current_play_mode;

void switch_to_music_page(const char *source_type);
void switch_to_conversation_page(void);
void switch_to_list_page(void);
void switch_to_content_page(void);
bool is_content_page(void);
#if CONFIG_LVGL_CONTENT_LIST_ENABLE
/** 阅读详情页左右滑翻页：dir>0 下一页，dir<0 上一页；已处理返回 1 */
int screen_content_detail_swipe_page(int dir);
#endif
#if CONFIG_LVGL_CONTENT_LIST_ENABLE && defined(APPLAYER_ENABLE) && APPLAYER_ENABLE
void page_switch_content_tts_stop(void);
/* 比 stop 更彻底：除了 http_tts_play_request_stop()，还会停 music_buf 消费、
 * 释放 websocket 接收 buffer、释放 music_buf 本地 buffer，把 SDK 内部 TTS
 * pipe (cycbuf) 真正清空。用于"重读"等需要立刻打断旧 TTS 并发起新段的场景。
 * 仅调 stop 不清缓冲会导致旧数据卡 cycbuf，新 request 写不进。 */
void page_switch_content_tts_force_clear(void);
void page_switch_content_tts_request(const char *utf8_text);
void page_switch_content_tts_request_heading_and_body(const char *heading, const char *body);
#endif

// 会话消息设置函数声明
void conversation_set_ai_message(lv_ui *ui, const char *message);
void conversation_set_user_message(lv_ui *ui, const char *message);

// 事件设置函数声明
void music_event_setup(void);
void alarm_event_setup(void);
void alarm_add_event_setup(void);
void alarm_set_event_setup(void);
void alarm_del_event_setup(void);

void timer_event_setup(void);
void timer_add_event_setup(void);
void timer_del_event_setup(void);

// 其他函数声明
void refresh_timer_del_page(lv_ui *ui);
void Dynamic_add_new_timer(lv_ui *ui, int hour, int minute, int second);
void page_move_init(void);

/** 同步下拉页/音乐页亮度与音量条；volume_value 为 0 时仅更新亮度（供背光调节等调用） */
void screen_set_sliders(int brightness_value, int volume_value);

/** 任一项为 CUSTOM_UI_BAR_OMIT 表示不更新该通道；0–100 为有效百分比（亮/音与下拉条一致：最小显示为 10） */
#define CUSTOM_UI_BAR_OMIT  (-1)
void custom_ui_bars_set(int brightness_pct, int volume_pct);
/** 只同步下拉+音乐两路音量条，volume_pct 0–100（条上最小 10，与现有一致） */
void custom_ui_volume_bars_set(int volume_pct);


void ui_subpage_return_set_large_touch_area(lv_obj_t *obj, lv_coord_t ext_pad);

/** 左上角返回图：点击回主界面 */
void sub_page_bind_return_img(lv_obj_t *img);
void sub_pages_bind_return_buttons(lv_ui *ui);

/** 首次切入智能体页时从 Flash 恢复选中（仅一次）；实现见 custom/agent_choose.c */
void agent_choose_restore_from_saved_id_first_enter_only(lv_ui *ui);

/** 睡眠音乐页 8 个音效 label：enable 非 0 时点击有效并播放对应 AVI；0 时忽略点击（如 AVI 播放中由应用层关闭） */
void screen_sleep_music_label_click_set_enabled(bool enable);
/** 取消睡眠音乐页 30/90 分钟定时关机（再次点击已选中的档位也会取消） */
void screen_sleep_music_cancel_duration_timer(void);

/** 长按音量键切到下拉页(音量/亮度条)流程的"原页面"快照接口：
 *  - 切到下拉页前调用 switch_save_origin_before_dropdown() 记录当前页；
 *  - 倒计时结束由 sys_timer 回调 switch_restore_origin_from_dropdown 切回原页面，
 *    替代旧实现里写死的 lv_demo_switch_to_main_page（导致非主页面卡在下拉页）。 */
void switch_save_origin_before_dropdown(void);
void switch_restore_origin_from_dropdown(void *priv);
void switch_save_origin_before_dialogue(void);
const char *switch_dialogue_return_page_get(void);
void switch_dialogue_return_page_set_main(void);
void switch_dialogue_return_page_set(const char *page_name);
void switch_to_page_by_name(const char *page_name);
void lv_demo_dialogue_end_return_page(void *priv);
void lv_demo_dialogue_cancel_resume_timer(void);

/** 网络 / SD / U 盘：语音或底层设置单曲循环时同步音乐页 mode 按钮显示 */
void lv_demo_music_sync_local_loop_ui(int enable);

/** 本地音乐（SD/U盘）曲名/歌词缓存，避免 music.c 与 screen_music.c 各有一份 music_info 不同步 */
void music_ui_cache_play_lyric(const char *lyric);
void music_ui_cache_play_lyric_clear(void);
int music_ui_cached_play_lyric_valid(void);
const char *music_ui_cached_play_lyric_get(void);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */
