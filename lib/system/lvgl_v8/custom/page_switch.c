#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdbool.h"
#include "gui_guider.h"
#include "custom.h"
#include "ai_uart_ctrol.h"

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
bool is_whack_game_page(void);
bool is_dice_game_page(void);
// 修改 switch_to_page_async 函数中的对话页面判断条件
static void switch_to_page_async(void *page_name)
{
    const char *name = (const char *)page_name;
    if (!name) {
        return;
    }
    if (is_whack_game_page() || is_dice_game_page()) {
        music_play_res_file_unloop();
    }
    printf("page_name = %s", page_name);
    // 执行页面切换
    if (strcmp(page_name, "main") == 0) {
        if (guider_ui.screen_main) {
            lv_obj_update_layout(guider_ui.screen_main);
            lv_scr_load_anim(guider_ui.screen_main, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_main);
            lv_refr_now(NULL);
            printf("-> 切换到主页面\n");
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
    } else if (strcmp(page_name, "podcast") == 0) {
        if (guider_ui.screen_podcast) {
            lv_obj_update_layout(guider_ui.screen_podcast);
            lv_scr_load_anim(guider_ui.screen_podcast, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_podcast);
            lv_refr_now(NULL);
            printf("-> 切换到播客页面\n");
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
    } else if (strcmp(page_name, "draw_lots_game") == 0) {
        if (guider_ui.screen_draw_lots_game) {
            lv_obj_update_layout(guider_ui.screen_draw_lots_game);
            lv_scr_load_anim(guider_ui.screen_draw_lots_game, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_draw_lots_game);
            lv_refr_now(NULL);
            printf("-> 切换到抽签页面\n");
        }
    } else if (strcmp(page_name, "whack_game") == 0) {
        if (guider_ui.screen_whack_game) {
            lv_obj_update_layout(guider_ui.screen_whack_game);
            lv_scr_load_anim(guider_ui.screen_whack_game, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_whack_game);
            lv_refr_now(NULL);
            printf("-> 切换到打地鼠游戏页面\n");
        }
    } else if (strcmp(page_name, "dice_game") == 0) {
        if (guider_ui.screen_dice_game) {
            lv_obj_update_layout(guider_ui.screen_dice_game);
            lv_scr_load_anim(guider_ui.screen_dice_game, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_dice_game);
            lv_refr_now(NULL);
            printf("-> 切换到摇骰子游戏页面\n");
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
    } else if (strcmp(page_name, "podcast") == 0) {
        if (guider_ui.screen_podcast) {
            lv_obj_update_layout(guider_ui.screen_podcast);
            lv_scr_load_anim(guider_ui.screen_podcast, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_podcast);
            lv_refr_now(NULL);
            printf("-> 切换到播客页面\n");
        }
    }else if (strcmp(page_name, "tomato") == 0) {
        if (guider_ui.screen_tomato) {
            lv_obj_update_layout(guider_ui.screen_tomato);
            lv_scr_load_anim(guider_ui.screen_tomato, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_tomato);
            lv_refr_now(NULL);
            printf("-> 切换到番茄倒计时页面\n");
        }
    }else if (strcmp(page_name, "current") == 0) {
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

bool is_draw_lots_game_page(void)
{
    return is_current_page("draw_lots_game");
}

bool is_whack_game_page(void)
{
    return is_current_page("whack_game");
}

bool is_dice_game_page(void)
{
    return is_current_page("dice_game");
}

bool is_podcast_page(void)
{
    return is_current_page("podcast");
}

bool is_ring_page(void)
{
    return is_current_page("ring");
}

// 切换到番茄倒计时页面
bool is_tomato_page(void) {
    return is_current_page("tomato");
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
    switch_to_page("conversation");
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
    if (strcmp(source_type, "net_music") == 0 && get_current_play_mode_id() != 0) {
        printf("[网络音乐]\n");
        set_current_play_mode_id(0);
        update = true;
    } else if (strcmp(source_type, "bt_music") == 0 && get_current_play_mode_id() != 1) {
        printf("[蓝牙音乐]\n");
        set_current_play_mode_id(1);
        update = true;
    } else if (strcmp(source_type, "sd_music") == 0 && get_current_play_mode_id() != 2) {
        printf("[SD卡音乐]\n");
        set_current_play_mode_id(2);
        update = true;
    }
    if (update) {
        music_page_init();
        switch_to_page("music");
    }
}
// 切换到播客页面
void switch_to_podcast_page(void *p)
{
    void *lv_demo_net_music_play_type(void);
    int lv_demo_net_music_is_start(void);
    void lv_demo_music_play(char next, char *type);
    switch_to_page("podcast");
    char *type = lv_demo_net_music_play_type();
    char start = lv_demo_net_music_is_start();
    if (!type || strcmp(type, "podcast") || !start) {
        lv_demo_music_play(1, "podcast");
    }
}
// 切换到下拉框页面
void switch_to_dropdown_page(void)
{
    switch_to_page("dropdown");
}

// 切换到抽签页面
void switch_to_draw_lots_game_page(void)
{
    switch_to_page("draw_lots_game");
#ifdef CONFIG_UI_ONLY_EYE//只开UI的眼睛显示
    play_face_emoji(AI_UART_CMD_EMOJI_RAND0);
#endif
}
// 切换到打地鼠游戏页面
void switch_to_whack_game_page(void)
{
    music_play_res_file_loop("MouseGame.mp3");
    switch_to_page("whack_game");
}
// 切换到摇骰子游戏页面
void switch_to_dice_game_page(void)
{
    music_play_res_file_loop("DieGame.mp3");
    switch_to_page("dice_game");
}
// 切换到铃声页面
void switch_to_ring_page(void)
{
    if (is_whack_game_page() || is_dice_game_page()) {
        music_play_res_file_unloop();
    }
    switch_to_page("ring");
}
// 切换到番茄倒计时页面
void switch_to_tomato_page(void) {
    switch_to_page("tomato");
}
