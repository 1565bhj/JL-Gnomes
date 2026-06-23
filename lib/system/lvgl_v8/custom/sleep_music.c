#include "lvgl.h"
#include "stdio.h"
#include "gui_guider.h"
#include "custom.h"
#include "lvgl_develop.h"

extern unsigned int sys_timeout_add_to_task(const char *task_name, void *priv, void (*func)(void *priv), unsigned int msec);
extern void sys_timeout_del(unsigned int id);

#if CONFIG_LVGL_SLEEP_MUSIC_ENABLE

/* 与 apps/wifi_story_machine/ui/avi_face.c 中 sleep_music_avi_num[] 一致 */
static const int s_sleep_music_avi[8] = { 221, 222, 223, 224, 225, 226, 227, 228 };

static bool s_labels_click_enabled = true;

#ifdef CONFIG_UI_PLAY_EMOJI
extern int play_face_emoji(int avi_index);
extern void play_face_emoji_lock(int lock);
#endif

void screen_sleep_music_label_click_set_enabled(bool enable)
{
    s_labels_click_enabled = enable;
}

static void sleep_music_label_click_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    if (!s_labels_click_enabled) {
        return;
    }
    int idx = (int)(lv_uintptr_t)lv_event_get_user_data(e);
    if (idx < 0 || idx >= 8) {
        return;
    }
#ifdef CONFIG_UI_PLAY_EMOJI
    play_face_emoji_lock(0);
    play_face_emoji(s_sleep_music_avi[idx]);
#else
    (void)idx;
#endif
}

static void bind_label(lv_obj_t *lbl, int idx)
{
    if (lbl == NULL) {
        return;
    }
    lv_obj_add_flag(lbl, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_event_cb(lbl, sleep_music_label_click_cb);
    lv_obj_add_event_cb(lbl, sleep_music_label_click_cb, LV_EVENT_CLICKED, (void *)(lv_uintptr_t)idx);
}

#if GUIDER_UI_SLEEP_MUSIC_DURATION_LABELS

#ifdef CONFIG_UI_PLAY_EMOJI
extern int play_face_emoji_kill(void);
#endif

static unsigned int s_sleep_music_duration_timeout_id;
/** 0=两键均为白；30/90=仅对应键为蓝（互斥，不可两键同蓝） */
static int s_duration_blue_id;

static void sleep_music_duration_timeout_cb(void *priv);
static void sleep_music_duration_poweroff_async_cb(void *user_data);

void screen_sleep_music_cancel_duration_timer(void)
{
    if (s_sleep_music_duration_timeout_id) {
        sys_timeout_del(s_sleep_music_duration_timeout_id);
        s_sleep_music_duration_timeout_id = 0;
    }
}

/* 与 setup_scr_screen_sleep_music_create_duration_labels 中默认样式一致 */
static void sleep_music_duration_label_style_normal(lv_obj_t *lbl)
{
    if (lbl == NULL) {
        return;
    }
    lv_obj_set_style_bg_opa(lbl, 65, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(lbl, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void sleep_music_duration_label_style_selected(lv_obj_t *lbl)
{
    if (lbl == NULL) {
        return;
    }
    lv_obj_set_style_bg_opa(lbl, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(lbl, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void sleep_music_duration_labels_apply_from_state(void)
{
    lv_obj_t *l30 = guider_ui.screen_sleep_music_label_30min;
    lv_obj_t *l90 = guider_ui.screen_sleep_music_label_90min;

    if (l30 == NULL || l90 == NULL) {
        return;
    }
    if (s_duration_blue_id == SLEEP_MUSIC_DURATION_BLUE_SHORT) {
        sleep_music_duration_label_style_selected(l30);
        sleep_music_duration_label_style_normal(l90);
    } else if (s_duration_blue_id == SLEEP_MUSIC_DURATION_BLUE_LONG) {
        sleep_music_duration_label_style_normal(l30);
        sleep_music_duration_label_style_selected(l90);
    } else {
        sleep_music_duration_label_style_normal(l30);
        sleep_music_duration_label_style_normal(l90);
    }
}

/** 根据当前蓝选（30/90）启动单次系统定时器，到点关机（低功耗下节拍不丢） */
static void screen_sleep_music_duration_timer_arm(void)
{
    uint32_t period_ms;

    screen_sleep_music_cancel_duration_timer();
    if (s_duration_blue_id != SLEEP_MUSIC_DURATION_BLUE_SHORT &&
        s_duration_blue_id != SLEEP_MUSIC_DURATION_BLUE_LONG) {
        return;
    }
    period_ms = (s_duration_blue_id == SLEEP_MUSIC_DURATION_BLUE_SHORT) ? SLEEP_MUSIC_DURATION_MS_SHORT
                : SLEEP_MUSIC_DURATION_MS_LONG;
    s_sleep_music_duration_timeout_id = sys_timeout_add_to_task("sys_timer", NULL,
                                        sleep_music_duration_timeout_cb, period_ms);
    printf("sleep music poweroff armed: %u ms\n", (unsigned)period_ms);
}

static void sleep_music_duration_poweroff_async_cb(void *user_data)
{
    (void)user_data;
    s_duration_blue_id = 0;
    sleep_music_duration_labels_apply_from_state();
#ifdef CONFIG_UI_PLAY_EMOJI
    play_face_emoji_kill();
#endif
    extern void music_play_stop_all(void);
    extern int music_play_waite(void);
    extern void music_play_res_file(const char *name);
    extern void sys_power_poweroff(void);
    music_play_stop_all();
    music_play_res_file("PwrOff.mp3");
    music_play_waite();
    sys_power_poweroff();
}

static void sleep_music_duration_timeout_cb(void *priv)
{
    (void)priv;
    s_sleep_music_duration_timeout_id = 0;
    printf("sleep music timed poweroff\n");
    if (lv_async_call(sleep_music_duration_poweroff_async_cb, NULL) != LV_RES_OK) {
        sleep_music_duration_poweroff_async_cb(NULL);
    }
}

static void sleep_music_duration_label_click_cb(lv_event_t *e)
{
    lv_obj_t *tgt;
    lv_obj_t *l30;
    lv_obj_t *l90;

    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    if (!s_labels_click_enabled) {
        return;
    }
    tgt = lv_event_get_target(e);
    l30 = guider_ui.screen_sleep_music_label_30min;
    l90 = guider_ui.screen_sleep_music_label_90min;
    if (tgt == l30) {
        /* 已蓝再点 → 白；否则 → 仅短档蓝 */
        s_duration_blue_id = (s_duration_blue_id == SLEEP_MUSIC_DURATION_BLUE_SHORT) ? 0
                             : SLEEP_MUSIC_DURATION_BLUE_SHORT;
        sleep_music_duration_labels_apply_from_state();
        screen_sleep_music_duration_timer_arm();
    } else if (tgt == l90) {
        s_duration_blue_id = (s_duration_blue_id == SLEEP_MUSIC_DURATION_BLUE_LONG) ? 0
                             : SLEEP_MUSIC_DURATION_BLUE_LONG;
        sleep_music_duration_labels_apply_from_state();
        screen_sleep_music_duration_timer_arm();
    }
}

static void bind_duration_labels(void)
{
    lv_obj_t *l30 = guider_ui.screen_sleep_music_label_30min;
    lv_obj_t *l90 = guider_ui.screen_sleep_music_label_90min;

    sleep_music_duration_labels_apply_from_state();

    if (l30) {
        lv_obj_add_flag(l30, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_event_cb(l30, sleep_music_duration_label_click_cb);
        lv_obj_add_event_cb(l30, sleep_music_duration_label_click_cb, LV_EVENT_CLICKED, NULL);
    }
    if (l90) {
        lv_obj_add_flag(l90, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_event_cb(l90, sleep_music_duration_label_click_cb);
        lv_obj_add_event_cb(l90, sleep_music_duration_label_click_cb, LV_EVENT_CLICKED, NULL);
    }
}
#endif

void screen_sleep_music_event_setup(void)
{
    lv_obj_set_scroll_dir(guider_ui.screen_sleep_music, LV_DIR_NONE);
    bind_label(guider_ui.screen_sleep_music_label_1, 0);
    bind_label(guider_ui.screen_sleep_music_label_2, 1);
    bind_label(guider_ui.screen_sleep_music_label_3, 2);
    bind_label(guider_ui.screen_sleep_music_label_4, 3);
    bind_label(guider_ui.screen_sleep_music_label_5, 4);
    bind_label(guider_ui.screen_sleep_music_label_6, 5);
    bind_label(guider_ui.screen_sleep_music_label_7, 6);
    bind_label(guider_ui.screen_sleep_music_label_8, 7);
#if GUIDER_UI_SLEEP_MUSIC_DURATION_LABELS
    bind_duration_labels();
#endif
}

#else /* !CONFIG_LVGL_SLEEP_MUSIC_ENABLE */

void screen_sleep_music_event_setup(void)
{
}

void screen_sleep_music_label_click_set_enabled(bool enable)
{
    (void)enable;
}

#endif /* CONFIG_LVGL_SLEEP_MUSIC_ENABLE */
