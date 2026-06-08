#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdbool.h"
#include "gui_guider.h"
#include "custom.h"


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

// 修改 switch_to_page_async 函数中的对话页面判断条件
static void switch_to_page_async(void *page_name) {
    const char *name = (const char *)page_name;
    if (!name) return;

    printf("page_name =%s",page_name);
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
    }else if (strcmp(page_name, "conversation") == 0) {
        if (guider_ui.screen_conversation) {
            lv_obj_update_layout(guider_ui.screen_conversation);
            lv_scr_load_anim(guider_ui.screen_conversation, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_conversation);
            lv_refr_now(NULL);
            printf("-> 切换到对话页面\n");
        }
    }else if (strcmp(page_name, "music") == 0) {
        if (guider_ui.screen_music) {
            lv_obj_update_layout(guider_ui.screen_music);
            lv_scr_load_anim(guider_ui.screen_music, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_music);
            lv_refr_now(NULL);
            printf("-> 切换到音乐页面\n");
        }
    }else if (strcmp(page_name, "alarm") == 0) {
        if (guider_ui.screen_alarm) {
            lv_obj_update_layout(guider_ui.screen_alarm);
            lv_scr_load_anim(guider_ui.screen_alarm, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_alarm);
            lv_refr_now(NULL);
            printf("-> 切换到闹钟页面\n");
        }
    }else if (strcmp(page_name, "alarm_add") == 0) {
        if (guider_ui.screen_alarm_add) {
            lv_obj_update_layout(guider_ui.screen_alarm_add);
            lv_scr_load_anim(guider_ui.screen_alarm_add, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_alarm_add);
            lv_refr_now(NULL);
            printf("-> 切换到新建闹钟页面\n");
        }
    }else if (strcmp(page_name, "alarm_remove") == 0) {
        if (guider_ui.screen_alarm_remove) {
            lv_obj_update_layout(guider_ui.screen_alarm_remove);
            lv_scr_load_anim(guider_ui.screen_alarm_remove, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_alarm_remove);
            lv_refr_now(NULL);
            printf("-> 切换到删除闹钟页面\n");
        }
    }else if (strcmp(page_name, "alarm_set") == 0) {
        if (guider_ui.screen_alarm_set) {
            lv_obj_update_layout(guider_ui.screen_alarm_set);
            lv_scr_load_anim(guider_ui.screen_alarm_set, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_alarm_set);
            lv_refr_now(NULL);
            printf("-> 切换到闹钟设置页面\n");
        }
    }else if (strcmp(page_name, "timer") == 0) {
        if (guider_ui.screen_timer) {
            lv_obj_update_layout(guider_ui.screen_timer);
            lv_scr_load_anim(guider_ui.screen_timer, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_timer);
            lv_refr_now(NULL);
            printf("-> 切换到倒计时页面\n");
        }
    }else if (strcmp(page_name, "timer_add") == 0) {
        if (guider_ui.screen_timer_add) {
            lv_obj_update_layout(guider_ui.screen_timer_add);
            lv_scr_load_anim(guider_ui.screen_timer_add, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_timer_add);
            lv_refr_now(NULL);
            printf("-> 切换到新建倒计时页面\n");
        }
    }else if (strcmp(page_name, "timer_remove") == 0) {
        if (guider_ui.screen_timer_remove) {
            lv_obj_update_layout(guider_ui.screen_timer_remove);
            lv_scr_load_anim(guider_ui.screen_timer_remove, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_timer_remove);
            lv_refr_now(NULL);
            printf("-> 切换到删除倒计时页面\n");
        }
    }else if (strcmp(page_name, "dropdown") == 0) {
        if (guider_ui.screen_dropdown) {
            lv_obj_update_layout(guider_ui.screen_dropdown);
            lv_scr_load_anim(guider_ui.screen_dropdown, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
            // 对新页面进行更新和刷新
            lv_obj_invalidate(guider_ui.screen_dropdown);
            lv_refr_now(NULL);
            printf("-> 切换到下拉框页面\n");
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


static void switch_to_page(const char *page_name) {
    lv_async_call(switch_to_page_async, (void *)page_name);
}



// 判断当前页面类型的函数
bool is_current_page(const char *page_name) {
    if (!page_name) return false;
    return strcmp(page_state.current_page, page_name) == 0;
}

// 快捷页面判断函数
bool is_main_page(void) {
    return is_current_page("main");
}

bool is_alarm_page(void) {
    return is_current_page("alarm");
}

bool is_alarm_add_page(void) {
    return is_current_page("alarm_add");
}

bool is_alarm_remove_page(void) {
    return is_current_page("alarm_remove");
}

bool is_alarm_set_page(void) {
    return is_current_page("alarm_set");
}

bool is_timer_page(void) {
    return is_current_page("timer");
}

bool is_timer_add_page(void) {
    return is_current_page("timer_add");
}

bool is_timer_remove_page(void) {
    return is_current_page("timer_remove");
}

bool is_conversation_page(void) {
    return is_current_page("conversation");
}

bool is_music_page(void) {
    return is_current_page("music");
}

bool is_dropdown_page(void) {
    return is_current_page("dropdown");
}




/**
 * 页面跳转函数集合
 * 以下函数用于在不同页面之间进行切换，并处理相关的页面进入逻辑
 */

// 切换到当前页面
void switch_to_current_page(void) {
    switch_to_page("current");
}

// 切换到主页面
void switch_to_main_page(void) {
    switch_to_page("main");
}

// 切换到闹钟页面
void switch_to_alarm_page(void) {
    switch_to_page("alarm");
}

// 切换到新建闹钟页面
void switch_to_alarm_add_page(void) {
    switch_to_page("alarm_add");
}

// 切换到删除闹钟页面
void switch_to_alarm_remove_page(void) {
    switch_to_page("alarm_remove");
}

// 切换到闹钟设置页面
void switch_to_alarm_set_page(void) {
    extern_reset_alarm_set_options();
    switch_to_page("alarm_set");
}

// 切换到倒计时页面
void switch_to_timer_page(void) {
    switch_to_page("timer");
}

// 切换到新建倒计时页面
void switch_to_timer_add_page(void) {
    switch_to_page("timer_add");
}

// 切换到删除倒计时页面
void switch_to_timer_remove_page(void) {
    switch_to_page("timer_remove");
}

// 切换到对话页面
void switch_to_conversation_page(void) {
    switch_to_page("conversation");
}

// 切换到音乐页面
void switch_to_music_page(const char *source_type) {
    if (!source_type) {
        return;
    }
    printf("-> Music play start from: %s\n", source_type);
    // 更新播放状态
    if (strcmp(source_type, "net_music") == 0) {
        printf("[网络音乐]\n");
        set_current_play_mode_id(0);     

    } else if (strcmp(source_type, "bt_music") == 0) {
        printf("[蓝牙音乐]\n");
        set_current_play_mode_id(1);    

    } else if (strcmp(source_type, "sd_music") == 0) {
        printf("[SD卡音乐]\n");
        set_current_play_mode_id(2);
    
    }
    music_page_init();
    switch_to_page("music");
}

// 切换到下拉框页面
void switch_to_dropdown_page(void) {
    switch_to_page("dropdown");
}
