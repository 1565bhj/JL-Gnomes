#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdbool.h"
#include "lvgl_develop.h"
#include "gui_guider.h"
#include "custom.h"
#include "ai_uart_ctrol.h"
#include "app_config.h"
#include "wifi_common.h"

#if LV_USE_KEYBOARD != 0 || LV_USE_ZH_KEYBOARD != 0
#include "lvgl.h"
/** 顶层键盘仅用于 WiFi 密码页：离开该页时解除绑定并隐藏，避免叠在新页面上 */
static void lvgl_hide_top_layer_keyboard_when_leaving_wifi_psw(const char *target_page)
{
    if (!target_page || strcmp(target_page, "wifi_psw") == 0) {
        return;
    }
    lv_obj_t *kb = guider_ui.g_kb_top_layer;
    if (kb == NULL) {
        return;
    }
#if LV_USE_ZH_KEYBOARD != 0
    lv_zh_keyboard_set_textarea(kb, NULL);
#endif
#if LV_USE_KEYBOARD != 0
    lv_keyboard_set_textarea(kb, NULL);
#endif
    lv_obj_move_background(kb);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
}
#endif

extern int lv_demo_ai_dialogue_speak_listen(char mode);

// 全局UI对象
lv_ui guider_ui;
// 页面状态管理结构体
typedef struct {
    char current_page[32];       // 当前页面名称
    char previous_page[32];      // 上一个页面名称
} page_state_t;

// 页面状态实例
static page_state_t page_state = {
    .current_page = "main",
    .previous_page = "main"
};

/* 长按音量键时会临时切到下拉页(screen_dropdown)显示音量条，
 * 倒计时结束后需要"切回原来的页面"而不是硬切 main，这里存进入下拉页之前的页面名 */
static char s_dropdown_origin_page[32] = "main";
/* 进入对话页前快照，对话/语音播控结束后切回（如音乐页播歌时语音切歌） */
static char s_dialogue_return_page[32] = "main";

bool is_current_page(const char *page_name);
static void switch_to_page(const char *page_name);

// 修改 switch_to_page_async 函数中的对话页面判断条件
static void switch_to_page_async(void *page_name)
{
    const char *name = (const char *)page_name;
    if (!name) {
        return;
    }
#if LV_USE_KEYBOARD != 0 || LV_USE_ZH_KEYBOARD != 0
    lvgl_hide_top_layer_keyboard_when_leaving_wifi_psw(name);
#endif
#if CONFIG_LVGL_CONTENT_LIST_ENABLE && defined(APPLAYER_ENABLE) && APPLAYER_ENABLE
    if (is_current_page("content") && strcmp(name, "content") != 0) {
        page_switch_content_tts_stop();
    }
#endif
    printf("page_name = %s", page_name);
    // 执行页面切换
    if (strcmp(page_name, "main") == 0) {
//// #ifdef CONFIG_UI_PLAY_EMOJI
////         /* 睡眠音乐页全屏 AVI 已关 LVGL 推屏；与上滑返回一致，先停 AVI 再切主界面，否则 lv_refr 与解码任务易死锁 */
////         if (is_current_page("sleep_music")) {
////             extern void play_face_emoji_lock(int lock);
////             extern int play_face_emoji_kill(void);
////             play_face_emoji_lock(0);
////             play_face_emoji_kill();
////         }
//// #endif
        if (guider_ui.screen_main) {
            lv_obj_update_layout(guider_ui.screen_main);
            lv_scr_load_anim(guider_ui.screen_main, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_main);
            lv_refr_now(NULL);
            printf("-> 切换到主页面\n");
        }
    } else if (strcmp(page_name, "option") == 0) {
        if (guider_ui.screen_option) {
            lv_obj_update_layout(guider_ui.screen_option);
            lv_scr_load_anim(guider_ui.screen_option, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_option);
            lv_refr_now(NULL);
            printf("-> 切换到选择页面\n");
        }
    } else if (strcmp(page_name, "conversation") == 0) {
        if (guider_ui.screen_conversation) {
            lv_obj_update_layout(guider_ui.screen_conversation);
            lv_scr_load_anim(guider_ui.screen_conversation, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_conversation);
            lv_refr_now(NULL);
            printf("-> 切换到对话页面\n");
        }
    } else if (strcmp(page_name, "music") == 0) {
        if (guider_ui.screen_music) {
            lv_obj_update_layout(guider_ui.screen_music);
            lv_scr_load_anim(guider_ui.screen_music, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_music);
            lv_refr_now(NULL);
            printf("-> 切换到音乐页面\n");
        }
    } else if (strcmp(page_name, "alarm") == 0) {
        if (guider_ui.screen_alarm) {
            lv_obj_update_layout(guider_ui.screen_alarm);
            lv_scr_load_anim(guider_ui.screen_alarm, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_alarm);
            lv_refr_now(NULL);
            printf("-> 切换到闹钟页面\n");
        }
    } else if (strcmp(page_name, "alarm_add") == 0) {
        if (guider_ui.screen_alarm_add) {
            lv_obj_update_layout(guider_ui.screen_alarm_add);
            lv_scr_load_anim(guider_ui.screen_alarm_add, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_alarm_add);
            lv_refr_now(NULL);
            printf("-> 切换到新建闹钟页面\n");
        }
    } else if (strcmp(page_name, "alarm_remove") == 0) {
        if (guider_ui.screen_alarm_remove) {
            lv_obj_update_layout(guider_ui.screen_alarm_remove);
            lv_scr_load_anim(guider_ui.screen_alarm_remove, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_alarm_remove);
            lv_refr_now(NULL);
            printf("-> 切换到删除闹钟页面\n");
        }
    } else if (strcmp(page_name, "alarm_set") == 0) {
        if (guider_ui.screen_alarm_set) {
            lv_obj_update_layout(guider_ui.screen_alarm_set);
            lv_scr_load_anim(guider_ui.screen_alarm_set, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_alarm_set);
            lv_refr_now(NULL);
            printf("-> 切换到闹钟设置页面\n");
        }
    } else if (strcmp(page_name, "timer") == 0) {
        if (guider_ui.screen_timer) {
            lv_obj_update_layout(guider_ui.screen_timer);
            lv_scr_load_anim(guider_ui.screen_timer, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_timer);
            lv_refr_now(NULL);
            printf("-> 切换到倒计时页面\n");
        }
    } else if (strcmp(page_name, "timer_add") == 0) {
        if (guider_ui.screen_timer_add) {
            lv_obj_update_layout(guider_ui.screen_timer_add);
            lv_scr_load_anim(guider_ui.screen_timer_add, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_timer_add);
            lv_refr_now(NULL);
            printf("-> 切换到新建倒计时页面\n");
        }
    } else if (strcmp(page_name, "timer_remove") == 0) {
        if (guider_ui.screen_timer_remove) {
            lv_obj_update_layout(guider_ui.screen_timer_remove);
            lv_scr_load_anim(guider_ui.screen_timer_remove, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_timer_remove);
            lv_refr_now(NULL);
            printf("-> 切换到删除倒计时页面\n");
        }
    } else if (strcmp(page_name, "dropdown") == 0) {
        if (guider_ui.screen_dropdown) {
            lv_obj_update_layout(guider_ui.screen_dropdown);
            lv_scr_load_anim(guider_ui.screen_dropdown, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_dropdown);
            lv_refr_now(NULL);
            lv_bar_set_value(guider_ui.screen_dropdown_volumn_bar, sys_volume_read(NULL), LV_ANIM_OFF);//重新设置音量进度条
            printf("-> 切换到下拉框页面\n");
        }
    } else if (strcmp(page_name, "ring") == 0) {
        if (guider_ui.screen_ring) {
            lv_obj_update_layout(guider_ui.screen_ring);
            lv_scr_load_anim(guider_ui.screen_ring, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_ring);
            lv_refr_now(NULL);
            printf("-> 切换到铃声页面\n");
        }
    } else if (strcmp(page_name, "tomato") == 0) {
        if (guider_ui.screen_tomato) {
            lv_obj_update_layout(guider_ui.screen_tomato);
            lv_scr_load_anim(guider_ui.screen_tomato, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_tomato);
            lv_refr_now(NULL);
            printf("-> 切换到番茄倒计时页面\n");
        }
    } else if (strcmp(page_name, "wifi_show") == 0) {
#if CONFIG_LVGL_OPTION_WIFI_ENABLE
        if (guider_ui.screen_wifi_show) {
            lv_obj_update_layout(guider_ui.screen_wifi_show);
            lv_scr_load_anim(guider_ui.screen_wifi_show, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_wifi_show);
            lv_refr_now(NULL);
            printf("-> 切换到WiFi显示页面\n");
        }
#endif
    } else if (strcmp(page_name, "wifi_psw") == 0) {
#if CONFIG_LVGL_OPTION_WIFI_ENABLE
        if (guider_ui.screen_wifi_psw) {
            lv_obj_update_layout(guider_ui.screen_wifi_psw);
            lv_scr_load_anim(guider_ui.screen_wifi_psw, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_wifi_psw);
            lv_refr_now(NULL);
            printf("-> 切换到WiFi密码页面\n");
        }
#endif
    } else if (strcmp(page_name, "net") == 0) {
        if (guider_ui.screen_net) {
            lv_obj_update_layout(guider_ui.screen_net);
            lv_scr_load_anim(guider_ui.screen_net, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_net);
            lv_refr_now(NULL);
            printf("-> 切换到配网页面\n");
        }
    } else if (strcmp(page_name, "agent") == 0) {
#if CONFIG_LVGL_AGENT_ENABLE
        if (guider_ui.screen_agent) {
            lv_obj_update_layout(guider_ui.screen_agent);
            lv_scr_load_anim(guider_ui.screen_agent, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_agent);
            lv_refr_now(NULL);
            printf("-> 切换到智能体选择页面\n");
            agent_choose_restore_from_saved_id_first_enter_only(&guider_ui);
        }
#endif
    } else if (strcmp(page_name, "calculator") == 0) {
#if CONFIG_LVGL_CALCULATOR_ENABLE
        if (guider_ui.screen_calculator) {
            lv_obj_update_layout(guider_ui.screen_calculator);
            lv_scr_load_anim(guider_ui.screen_calculator, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_calculator);
            lv_refr_now(NULL);
            printf("-> 切换到计算机页面\n");
        }
#endif
    } else if (strcmp(page_name, "list") == 0) {
#if CONFIG_LVGL_CONTENT_LIST_ENABLE
        if (guider_ui.screen_list) {
            lv_obj_update_layout(guider_ui.screen_list);
            lv_scr_load_anim(guider_ui.screen_list, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            lv_obj_invalidate(guider_ui.screen_list);
            lv_refr_now(NULL);
            printf("-> 切换到内容列表页面\n");
        }
#endif
    } else if (strcmp(page_name, "content") == 0) {
#if CONFIG_LVGL_CONTENT_LIST_ENABLE
        if (guider_ui.screen_content) {
            lv_obj_update_layout(guider_ui.screen_content);
            lv_scr_load_anim(guider_ui.screen_content, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            lv_obj_invalidate(guider_ui.screen_content);
            lv_refr_now(NULL);
            printf("-> 切换到内容详情页面\n");
        }
#endif
    } else if (strcmp(page_name, "draw_lots_game") == 0) {
#if defined(CONFIG_LVGL_DRAW_LOTS_GAME_ENABLE) && CONFIG_LVGL_DRAW_LOTS_GAME_ENABLE
        if (guider_ui.screen_draw_lots_game) {
            lv_obj_update_layout(guider_ui.screen_draw_lots_game);
            lv_scr_load_anim(guider_ui.screen_draw_lots_game, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            lv_obj_invalidate(guider_ui.screen_draw_lots_game);
            lv_refr_now(NULL);
            printf("-> 切换到抽签页面\n");
        }
#endif
    } else if (strcmp(page_name, "sleep_music") == 0) {
#if CONFIG_LVGL_SLEEP_MUSIC_ENABLE
        if (guider_ui.screen_sleep_music) {
            lv_obj_update_layout(guider_ui.screen_sleep_music);
            lv_scr_load_anim(guider_ui.screen_sleep_music, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_sleep_music);
            lv_refr_now(NULL);
            printf("-> 切换到睡眠音乐页面\n");
        }
#endif
    } else if (strcmp(page_name, "current") == 0) {
        lv_obj_invalidate(lv_scr_act());  // 使当前屏幕失效
        lv_refr_now(NULL);
        return;
    }
    // 统一更新当前页面状态
    strncpy(page_state.current_page, page_name, sizeof(page_state.current_page) - 1);
    page_state.current_page[sizeof(page_state.current_page) - 1] = '\0';
}


static void switch_to_page(const char *page_name)
{
    lv_async_call(switch_to_page_async, (void *)page_name);
}



// 判断当前页面类型的函数
bool is_current_page(const char *page_name)
{
    if (!page_name) {
        return false;
    }
    return strcmp(page_state.current_page, page_name) == 0;
}

/** 切到下拉页(显示音量条/亮度条)之前调用：把当前页快照到 s_dropdown_origin_page，
 *  超时倒计时结束时由 switch_restore_origin_from_dropdown 切回这一页。
 *  当前若已在 dropdown，则不覆盖快照——重复长按音量不会把 origin 退化成 "dropdown"。 */
void switch_save_origin_before_dropdown(void)
{
    if (is_current_page("dropdown")) {
        return;
    }
    strncpy(s_dropdown_origin_page, page_state.current_page,
            sizeof(s_dropdown_origin_page) - 1);
    s_dropdown_origin_page[sizeof(s_dropdown_origin_page) - 1] = '\0';
    printf("[下拉返回] 记录原页面: %s\n", s_dropdown_origin_page);
}

/** sys_timer 回调（签名: void(*)(void *)）：长按音量键释放后约 2s 由 timer 触发，
 *  把下拉页切回 switch_save_origin_before_dropdown 时记下的原页面。
 *  ① 当前已经不是 dropdown：说明用户在倒计时内手动切走了，直接 return，不强切；
 *  ② 快照为空 / 非法 / 仍是 dropdown：兜底切回 main，避免死循环卡 dropdown。 */
void switch_restore_origin_from_dropdown(void *priv)
{
    (void)priv;
    if (!is_current_page("dropdown")) {
        return;
    }
    const char *target = s_dropdown_origin_page;
    if (!target || !target[0] || strcmp(target, "dropdown") == 0) {
        target = "main";
    }
    printf("[下拉返回] 切回原页面: %s\n", target);
    switch_to_page(target);
}

void switch_save_origin_before_dialogue(void)
{
    if (is_current_page("conversation")) {
        printf("[对话返回] 已在对话页，跳过\n");
        return;
    }
    printf("[对话返回] 调用save_origin, current_page=%s, 已有return_page=%s\n",
           page_state.current_page, s_dialogue_return_page);
    /* 已经记录了有效的返回页面（如 "music"），不覆盖 */
    if (s_dialogue_return_page[0] && strcmp(s_dialogue_return_page, "main") != 0) {
        printf("[对话返回] 保留已记录原页面: %s\n", s_dialogue_return_page);
        return;
    }
    strncpy(s_dialogue_return_page, page_state.current_page,
            sizeof(s_dialogue_return_page) - 1);
    s_dialogue_return_page[sizeof(s_dialogue_return_page) - 1] = '\0';
    printf("[对话返回] 记录原页面: %s\n", s_dialogue_return_page);
}

const char *switch_dialogue_return_page_get(void)
{
    return s_dialogue_return_page;
}

void switch_dialogue_return_page_set_main(void)
{
    strncpy(s_dialogue_return_page, "main", sizeof(s_dialogue_return_page) - 1);
    s_dialogue_return_page[sizeof(s_dialogue_return_page) - 1] = '\0';
}

void switch_dialogue_return_page_set(const char *page_name)
{
    if (!page_name) {
        return;
    }
    strncpy(s_dialogue_return_page, page_name, sizeof(s_dialogue_return_page) - 1);
    s_dialogue_return_page[sizeof(s_dialogue_return_page) - 1] = '\0';
}

void switch_to_page_by_name(const char *page_name)
{
    if (!page_name || !page_name[0]) {
        return;
    }
    switch_to_page(page_name);
}

// 快捷页面判断函数
bool is_main_page(void)
{
    return is_current_page("main");
}

bool is_alarm_page(void)
{
    return is_current_page("alarm");
}

bool is_alarm_add_page(void)
{
    return is_current_page("alarm_add");
}

bool is_alarm_remove_page(void)
{
    return is_current_page("alarm_remove");
}

bool is_alarm_set_page(void)
{
    return is_current_page("alarm_set");
}

bool is_timer_page(void)
{
    return is_current_page("timer");
}

bool is_timer_add_page(void)
{
    return is_current_page("timer_add");
}

bool is_timer_remove_page(void)
{
    return is_current_page("timer_remove");
}

bool is_conversation_page(void)
{
    return is_current_page("conversation");
}

bool is_music_page(void)
{
    return is_current_page("music");
}

bool is_dropdown_page(void)
{
    return is_current_page("dropdown");
}

bool is_ring_page(void)
{
    return is_current_page("ring");
}

bool is_tomato_page(void)
{
    return is_current_page("tomato");
}

bool is_wifi_show_page(void)
{
    return is_current_page("wifi_show");
}

bool is_wifi_psw_page(void)
{
    return is_current_page("wifi_psw");
}

bool is_net_page(void)
{
    return is_current_page("net");
}

bool is_agent_page(void)
{
    return is_current_page("agent");
}

bool is_calculator_page(void)
{
    return is_current_page("calculator");
}

bool is_sleep_music_page(void)
{
    return is_current_page("sleep_music");
}

bool is_list_page(void)
{
    return is_current_page("list");
}

bool is_content_page(void)
{
    return is_current_page("content");
}

/**
 * 页面跳转函数集合
 * 以下函数用于在不同页面之间进行切换，并处理相关的页面进入逻辑
 */

// 切换到当前页面
void switch_to_current_page(void)
{
    switch_to_page("current");
}

// 切换到主页面
void switch_to_main_page(void)
{
    switch_to_page("main");
}

// 切换到选择页面
void switch_to_option_page(void)
{
    switch_to_page("option");
}
// 切换到闹钟页面
void switch_to_alarm_page(void)
{
    switch_to_page("alarm");
}

// 切换到新建闹钟页面
void switch_to_alarm_add_page(void)
{
    switch_to_page("alarm_add");
}

// 切换到删除闹钟页面
void switch_to_alarm_remove_page(void)
{
    switch_to_page("alarm_remove");
}

// 切换到闹钟设置页面
void switch_to_alarm_set_page(void)
{
    extern_reset_alarm_set_options();
    switch_to_page("alarm_set");
}

// 切换到倒计时页面
void switch_to_timer_page(void)
{
#if TOMATO_TIMER_ENABLE
    switch_to_page("tomato");
#else
    switch_to_page("timer");
#endif // TOMATO_TIMER_ENABLE
}

// 切换到新建倒计时页面
void switch_to_timer_add_page(void)
{
    switch_to_page("timer_add");
}

// 切换到删除倒计时页面
void switch_to_timer_remove_page(void)
{
    switch_to_page("timer_remove");
}

// 切换到对话页面
void switch_to_conversation_page(void)
{
    switch_save_origin_before_dialogue();
    switch_to_page("conversation");
    lv_demo_ai_dialogue_speak_listen(1);
}

// 切换到音乐页面
void switch_to_music_page(const char *source_type)
{
    if (!source_type) {
        return;
    }
    char update = 0;
    if (!is_music_page()) {
        update = true;
    }
    printf("-> Music play start from: %s\n", source_type);
    // 更新播放状态
    if (strcmp(source_type, "net_music") == 0) {
        printf("[网络音乐]\n");
        set_current_play_mode_id(MODE_NET_ID);
        update = 1;
    } else if (strcmp(source_type, "bt_music") == 0) {
        printf("[蓝牙音乐]\n");
        set_current_play_mode_id(MODE_BT_ID);
        update = 1;
    } else if (strcmp(source_type, "sd_music") == 0) {
        printf("[SD卡音乐]\n");
        set_current_play_mode_id(MODE_SD_ID);
        update = 1;
    } else if (strcmp(source_type, "usbdisk_music") == 0) {
        printf("[U盘音乐]\n");
        set_current_play_mode_id(MODE_USB_ID);
        update = 1;
    } else if (strcmp(source_type, "aux_music") == 0) {
        printf("[线路输入AUX]\n");
        set_current_play_mode_id(MODE_AUX_ID);
        update = 1;
    }
    if (update) {
        music_page_init();
        switch_to_page("music");
    }
}
// 切换到下拉框页面
void switch_to_dropdown_page(void)
{
    switch_to_page("dropdown");
}

// 切换到铃声页面
void switch_to_ring_page(void)
{
    switch_to_page("ring");
}
// 切换到番茄倒计时页面
void switch_to_tomato_page(void)
{
    switch_to_page("tomato");
}
// 切换到WiFi显示页面
void switch_to_wifi_show_page(void)
{
#if CONFIG_LVGL_OPTION_WIFI_ENABLE
    /*
     * 原逻辑仅看 actual_wifi_num：未做过扫描时为 0，即使用户已连 WiFi 也会误进配网页。
     * 已关联/已 DHCP 时仍进 WiFi 列表页并触发扫描刷新。
     */
    enum wifi_sta_connect_state st = wifi_get_sta_connect_state();
    bool sta_linked = (st == WIFI_STA_CONNECT_SUCC || st == WIFI_STA_NETWORK_STACK_DHCP_SUCC);
    if (get_wifi_scan_ssid_num() > 0 || sta_linked || lv_demo_system_net_connect_success()) {
        switch_to_page("wifi_show");
        if (get_wifi_scan_ssid_num() == 0) {
            scan_and_print_wifi_networks(); /* 本工程未链接，扫描由其它模块或后续接入 */
        }
        refresh_wifi_containers();
    } else {
        switch_to_page("net");
        aisp_wake(0);
    }
#else
    (void)0;
#endif
}
// 切换到WiFi密码页面
void switch_to_wifi_psw_page(void)
{
#if CONFIG_LVGL_OPTION_WIFI_ENABLE
    // 更新WiFi名称显示
    update_wifi_name_display_async();
    switch_to_page("wifi_psw");
#else
    (void)0;
#endif
}
// 切换到配网页面
void switch_to_net_page(void)
{
    if (is_ring_page()) {
        return;
    }
    switch_to_page("net");
}
// 切换到智能体选择页面
void switch_to_agent_page(void)
{
#if CONFIG_LVGL_AGENT_ENABLE
    /*
     * lv_demo_system_net_connect_success() 在 multiple_ethernetif 里往往要求 NET_CONNECT_SERVER_OK
     *（连上业务服务器/外网探测），WiFi 已 DHCP 成功但探测未完成时仍为 0，会误进配网页。
     */
    int net_ok = lv_demo_system_net_connect_success();
    enum wifi_sta_connect_state st = wifi_get_sta_connect_state();
    bool wifi_ip_ok = (st == WIFI_STA_NETWORK_STACK_DHCP_SUCC);
    if (net_ok || wifi_ip_ok) {
        switch_to_page("agent");
    } else {
        switch_to_page("net");
        aisp_wake(0);
    }
#else
    (void)0;
#endif
}
// 切换到计算机页面
void switch_to_calculator_page(void)
{
#if CONFIG_LVGL_CALCULATOR_ENABLE
    switch_to_page("calculator");
#else
    (void)0;
#endif
}
// 切换到睡眠音乐页面
void switch_to_sleep_music_page(void)
{
#if CONFIG_LVGL_SLEEP_MUSIC_ENABLE
    switch_to_page("sleep_music");
    screen_sleep_music_label_click_set_enabled(true);
//#ifdef CONFIG_UI_PLAY_EMOJI
//    play_face_emoji(221);
//#endif
#else
    (void)0;
#endif
}

void switch_to_list_page(void)
{
    switch_to_page("list");
}

void switch_to_content_page(void)
{
    switch_to_page("content");
}

void ui_subpage_return_set_large_touch_area(lv_obj_t *obj, lv_coord_t ext_pad)
{
    if (obj == NULL) {
        return;
    }
    if (ext_pad < 0) {
        return;
    }
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    if (ext_pad == 0) {
        lv_obj_set_ext_click_area(obj, 0);
        return;
    }
    /* LVGL8: lv_indev_search_obj 仅在触点落在每层的 obj->coords 内时才向子级递归；
     * ext_click_area 只参与命中测试，无法把子对象的「外扩」点带入父级 coords 外的搜索链，
     * 外扩到父/屏幕外时等效不生效。改为：扩大本体坐标 + 四边等宽 padding，把 22x22 留在原视觉位。 */
    lv_coord_t w = lv_obj_get_width(obj);
    lv_coord_t h = lv_obj_get_height(obj);
    lv_coord_t x = lv_obj_get_x(obj);
    lv_coord_t y = lv_obj_get_y(obj);
    lv_obj_set_x(obj, x - ext_pad);
    lv_obj_set_y(obj, y - ext_pad);
    lv_obj_set_size(obj, w + 2 * ext_pad, h + 2 * ext_pad);
    lv_obj_set_style_pad_all(obj, ext_pad, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_ext_click_area(obj, 0);
    lv_obj_invalidate(obj);
}

static void sub_page_return_img_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    switch_to_main_page();
}

void sub_page_bind_return_img(lv_obj_t *img)
{
    if (!img) {
        return;
    }
    ui_subpage_return_set_large_touch_area(img, (lv_coord_t)UI_SUBPAGE_RETURN_EXT_PAD);
    lv_obj_remove_event_cb(img, sub_page_return_img_event_cb);
    lv_obj_add_event_cb(img, sub_page_return_img_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_move_foreground(img);
}

void sub_pages_bind_return_buttons(lv_ui *ui)
{
    if (!ui) {
        return;
    }
    sub_page_bind_return_img(ui->screen_music_img_return);
    sub_page_bind_return_img(ui->screen_conversation_img_return);
    /* 下拉页返回键暂不显示（先隐藏） */
    if (ui->screen_dropdown_img_return) {
        lv_obj_add_flag(ui->screen_dropdown_img_return, LV_OBJ_FLAG_HIDDEN);
    }
    sub_page_bind_return_img(ui->screen_alarm_img_return);
    sub_page_bind_return_img(ui->screen_timer_img_return);
    sub_page_bind_return_img(ui->screen_ring_img_return);
    sub_page_bind_return_img(ui->screen_tomato_img_return);
    sub_page_bind_return_img(ui->screen_net_img_return);
#if CONFIG_LVGL_SLEEP_MUSIC_ENABLE
    if (ui->screen_sleep_music_img_return) {
//        sub_page_bind_return_img(ui->screen_sleep_music_img_return);
        ui_subpage_return_set_large_touch_area(ui->screen_sleep_music_img_return, (lv_coord_t)UI_SUBPAGE_RETURN_EXT_PAD - 20);
        lv_obj_remove_event_cb(ui->screen_sleep_music_img_return, sub_page_return_img_event_cb);
        lv_obj_add_event_cb(ui->screen_sleep_music_img_return, sub_page_return_img_event_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_move_foreground(ui->screen_sleep_music_img_return);
    }
#endif

}
