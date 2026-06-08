#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdbool.h"
#include "gui_guider.h"
#include "custom.h"

// 当前播放模式ID
extern int get_current_play_mode_id(void);
extern void set_current_play_mode_id(int mode_id);

// 播客信息结构体
typedef struct {
    char title[256];
    char artist[256];
    char genre[256];
    uint32_t current_time;
    uint32_t last_time;
    uint32_t total_time;
    bool title_changed;
    bool artist_changed;
    bool genre_changed;
    bool current_time_changed;
    bool total_time_changed;
} podcast_info_t;


// 使用静态变量避免频繁的内存分配
static podcast_info_t podcast_info = {
    .title = "播客名未更新",
    .artist = "主播名未更新",
    .genre = "节目简介未更新",
    .current_time = 0,
    .total_time = 4 * 60 + 14, // 初始时间值 (4分14秒)
    .title_changed = false,
    .artist_changed = false,
    .genre_changed = false,
    .current_time_changed = false,
    .total_time_changed = false
};



// 定义固定缓冲区
#define LYRIC_BUFFER_SIZE 256
static char lyric_buffer[LYRIC_BUFFER_SIZE];
// 更新标志，用于批量更新
static volatile bool no_need_ui_update = false;

void lv_demo_handle_music_pause(void);
void lv_demo_handle_music_play(void);
void lv_demo_handle_music_prev(void);
void lv_demo_handle_music_next(void);
void lv_demo_handle_music_mode_switch(void);

// 简单的UI更新函数，直接在适当的线程中调用
static void direct_ui_update(void)
{
    // 检查播客页面根对象是否有效
    if (!guider_ui.screen_podcast) {
        return;
    }
    //printf(" direct_ui_update简单的UI更新函数，直接在适当的线程中调用\n");
    // 更新标题
    if (podcast_info.title_changed && guider_ui.screen_podcast_podcast_title) {
        lv_label_set_text(guider_ui.screen_podcast_podcast_title, podcast_info.title);
        podcast_info.title_changed = false;
//         // 关键设置：设置为水平循环滚动模式
//        lv_label_set_long_mode(guider_ui.screen_podcast_podcast_title, LV_LABEL_LONG_SCROLL_CIRCULAR);
//
//        // 确保文本居中对齐，但需要配合标签高度限制以避免垂直滚动
//        lv_obj_set_style_text_align(guider_ui.screen_podcast_podcast_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
//
//        // 确保标签足够高以显示一行文本，避免垂直滚动
//        lv_obj_set_height(guider_ui.screen_podcast_podcast_title, lv_font_get_line_height(lv_obj_get_style_text_font(guider_ui.screen_podcast_podcast_title, LV_PART_MAIN)) + 4);
//
//        // 设置滚动动画时间（控制滚动速度）
//        lv_obj_set_style_anim_time(guider_ui.screen_podcast_podcast_title, 5000, LV_PART_MAIN); // 5秒完成一次循环
//
//        // 刷新对象以确保设置生效
//        lv_obj_update_layout(guider_ui.screen_podcast_podcast_title);
    }
    // 更新总播放时间
    if (podcast_info.total_time_changed && guider_ui.screen_podcast_time_end_span) {
        char time_str[16];
        uint32_t min = podcast_info.total_time / 60;
        uint32_t sec = podcast_info.total_time % 60;
        lv_snprintf(time_str, sizeof(time_str), "%d:%02d", min, sec);
        lv_span_set_text(guider_ui.screen_podcast_time_end_span, time_str);

        // 更新进度条范围
        lv_bar_set_range(guider_ui.screen_podcast_slider, 0, 100);
        podcast_info.total_time_changed = false;
    }
    // 更新当前播放时间
    if (podcast_info.current_time_changed && guider_ui.screen_podcast_time_start_span) {
        char time_str[16];
        uint32_t min = podcast_info.current_time / 60;
        uint32_t sec = podcast_info.current_time % 60;
        lv_snprintf(time_str, sizeof(time_str), "%d:%02d", min, sec);
        lv_span_set_text(guider_ui.screen_podcast_time_start_span, time_str);

        // 更新进度条位置
        if (podcast_info.total_time > 0) {
            uint32_t progress = (podcast_info.current_time * 100) / podcast_info.total_time;
            if (progress > 100) {
                progress = 100;
            }
            lv_bar_set_value(guider_ui.screen_podcast_slider, progress, LV_ANIM_OFF);
        }
        podcast_info.current_time_changed = false;
    }
    void async_screen_music_btn_play_pause_show(void* param);
    async_screen_music_btn_play_pause_show(0);
}

static void lock_ui_to_update(void *lock)
{
    no_need_ui_update = (bool)lock;
}

// 定时器回调，用于批量更新UI
static void ui_update_timer_cb(void *param)
{
    if (!no_need_ui_update) {
        direct_ui_update();
    }
}

// 请求UI更新（防抖处理）
static void request_ui_update(void)
{
    if (!is_podcast_page()) {
        return ;
    }
    lv_async_call(ui_update_timer_cb, NULL);
}
static char original_title[128] = {0};
static bool original_title_initialized = false; // 添加初始化标志

#ifdef APPLAYER_ENABLE
#else
int put_buf(const char *str, int len)
{

}
#endif
// 更新标题，播客名
void podcast_update_title(char *title)
{
    if (!title) {
        return;
    }
    // 检查内容是否有变化
    if (strcmp(podcast_info.title, title) != 0) {
        lv_snprintf(podcast_info.title, sizeof(podcast_info.title), "%s", title);
        podcast_info.title_changed = true;
        request_ui_update();
    }

}


// 更新总时长（注意：需在UI结构体中补充时间显示控件才能完全生效）
void podcast_update_time(uint32_t total_sec)
{
    if (total_sec == 0) {
        return;
    }

    // 检查时长是否有变化
    if (podcast_info.total_time != total_sec) {
        podcast_info.total_time = total_sec;
        podcast_info.total_time_changed = true;
        request_ui_update();
    }
}

// 更新进度条范围（保留兼容性）
void podcast_update_slider_range(uint32_t total_time)
{
    // 此函数现在通过 lv_demo_podcast_update_time 处理
    podcast_update_time(total_time);
}



// 更新当前播放时间（注意：需在UI结构体中补充时间显示控件才能完全生效）
void podcast_update_current_time(uint32_t curr_sec)
{
    // 检查当前播放时间是否有变化
    if (podcast_info.current_time != curr_sec) {
        podcast_info.current_time = curr_sec;
        podcast_info.current_time_changed = true;
        request_ui_update();
    }

}


// 获取当前播放时间
uint32_t podcast_get_current_time(void)
{
    return podcast_info.current_time;
}

// 播放/暂停按钮状态
static bool g_play_pause_btn_state = false;

/* 磁带转动核心变量 */
static lv_timer_t *rotate_timer = NULL;
static uint32_t start_time = 0;       // 旋转开始的时间戳(ms)
static uint32_t pause_offset = 0;     // 暂停时已旋转的总时间(ms)
static const uint32_t ROTATE_PERIOD = 4000;  // 旋转一圈的时间(ms)

/* img_music 动画相关变量 */
static lv_anim_t img_music_anim;
static bool img_music_anim_reverse = false;
static const int32_t IMG_MUSIC_ORIGINAL_WIDTH = 228;  // 原始宽度
static const int32_t IMG_MUSIC_MIN_WIDTH = 180;       // 最小宽度
static const uint32_t IMG_MUSIC_ANIM_TIME = 800;      // 动画持续时间(ms)
static const uint16_t IMG_MUSIC_ZOOM_ORIGINAL = 256;  // LVGL图片原始缩放值(100%)
static const uint16_t IMG_MUSIC_ZOOM_MIN = 205;       // 缩放80% (256 * 0.8 = 204.8 取整)

/**
 * @brief 初始化磁带动画状态
 * 将所有动画相关的状态变量重置为初始值
 */
static void init_tape_animation_state(void)
{
    start_time = 0;       // 重置开始时间
    pause_offset = 0;     // 重置暂停偏移
    rotate_timer = NULL;  // 确保定时器为空
    g_play_pause_btn_state = true; // 设置默认状态为播放
    lv_img_set_angle(guider_ui.screen_podcast_img_tape, 0); // 重置角度为0度
}

/**
 * @brief 定时器回调：通过绝对时间计算角度（无累加误差）
 */
static void rotate_timer_cb(lv_timer_t *timer)
{
    // 计算当前已旋转的总时间（包含历史累计）
    uint32_t current_time = lv_tick_get() - start_time + pause_offset;

    // 计算角度：(总时间 / 周期) * 360度，取模360确保在0-359范围
    int32_t angle = (current_time * 360 * 10) / ROTATE_PERIOD % (360 * 10);

    // 设置角度
    lv_img_set_angle(guider_ui.screen_podcast_img_tape, angle);
}

/**
 * @brief 启动磁带动画函数
 * 开始磁带旋转动画，并设置相关图标状态
 */
static void tape_rotate_start(void)
{
    // 设置磁带条的角度为-80度
    lv_img_set_angle(guider_ui.screen_podcast_img_bar, -80);

    // 启动定时器（从上次停止的角度继续）
    if (!rotate_timer) {
        start_time = lv_tick_get();  // 记录当前启动时间
        rotate_timer = lv_timer_create(rotate_timer_cb, 10, NULL);  // 10ms刷新一次
        lv_timer_set_repeat_count(rotate_timer, -1);  // 无限循环
    }
}

/**
 * @brief 停止磁带动画函数
 * 停止磁带旋转动画，并保存当前状态
 */
static void tape_rotate_stop(void)
{
    // 停止定时器并记录已旋转的时间（关键：保存历史状态）
    if (rotate_timer) {
        pause_offset += lv_tick_get() - start_time;
        lv_timer_del(rotate_timer);
        rotate_timer = NULL;
    }

    // 设置磁带条的角度为110度
    lv_img_set_angle(guider_ui.screen_podcast_img_bar, 110);
}

/**
 * @brief img_music 动画结束回调函数
 * 实现动画的反向播放，形成循环效果
 */
static void img_music_anim_end_cb(lv_anim_t *anim)
{
    // 切换动画方向
    img_music_anim_reverse = !img_music_anim_reverse;

    // 重新配置并启动动画
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, guider_ui.screen_podcast_img_music);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_img_set_zoom);
    lv_anim_set_time(&a, IMG_MUSIC_ANIM_TIME);
    lv_anim_set_playback_delay(&a, 200); // 延迟200ms后开始反向动画
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_ready_cb(&a, img_music_anim_end_cb);

    if (img_music_anim_reverse) {
        // 从最小宽度恢复到原始宽度
        lv_anim_set_values(&a, IMG_MUSIC_ZOOM_MIN, IMG_MUSIC_ZOOM_ORIGINAL);
    } else {
        // 从原始宽度缩短到最小宽度
        lv_anim_set_values(&a, IMG_MUSIC_ZOOM_ORIGINAL, IMG_MUSIC_ZOOM_MIN);
    }

    lv_anim_start(&a);
}

/**
 * @brief 初始化并启动 img_music 的缩放动画
 */
static void start_img_music_animation(void)
{
    // 确保不会同时启动多个动画
    lv_anim_del(guider_ui.screen_podcast_img_music, (lv_anim_exec_xcb_t)lv_img_set_zoom);

    // 初始化动画
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, guider_ui.screen_podcast_img_music);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_img_set_zoom);
    lv_anim_set_time(&a, IMG_MUSIC_ANIM_TIME);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_ready_cb(&a, img_music_anim_end_cb);

    // 设置动画从原始大小到缩小比例
    lv_anim_set_values(&a, IMG_MUSIC_ZOOM_ORIGINAL, IMG_MUSIC_ZOOM_MIN);

    // 启动动画
    lv_anim_start(&a);
}

// 播放/暂停按钮点击事件处理函数
static void screen_podcast_btn_play_pause_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    switch (code) {
    case LV_EVENT_CLICKED: {
            // 切换按钮状态
            g_play_pause_btn_state = !g_play_pause_btn_state;
            if (g_play_pause_btn_state) {
                // 设置为播放
                lv_label_set_text(ui->screen_podcast_btn_play_pause_label, "I I");
                lv_obj_set_style_text_font(ui->screen_podcast_btn_play_pause_label, &lv_font_Barlow__18, LV_PART_MAIN | LV_STATE_DEFAULT);
                tape_rotate_start();
                lv_label_set_long_mode(ui->screen_podcast_podcast_title, LV_LABEL_LONG_SCROLL_CIRCULAR);
                lv_demo_handle_music_play();
                lock_ui_to_update(0);
                // start_img_music_animation(); // 开始 img_music 动画
            } else {
                // 设置为暂停
                lv_label_set_text(ui->screen_podcast_btn_play_pause_label, "   "LV_SYMBOL_PLAY " ");
                lv_obj_set_style_text_font(ui->screen_podcast_btn_play_pause_label, &lv_font_Barlow__18, LV_PART_MAIN | LV_STATE_DEFAULT);
                tape_rotate_stop();
                lv_demo_handle_music_pause();
                lv_label_set_long_mode(ui->screen_podcast_podcast_title, LV_LABEL_LONG_DOT);
                lv_anim_del(guider_ui.screen_podcast_img_music, (lv_anim_exec_xcb_t)lv_img_set_zoom); // 停止 img_music 动画
                lock_ui_to_update(1);
                sys_timeout_add_to_task("sys_timer", 0, lock_ui_to_update, 800);
            }
            break;
        }
    default:
        break;
    }
}
/**
 * 处理上一曲播放功能
 * 根据当前播放模式执行相应操作
 */
void lv_podcast_handle_music_prev(void)
{
#ifdef APPLAYER_ENABLE
    lv_demo_music_play(0, "podcast");
#endif
}
/**
 * 处理下一曲播放功能
 * 根据当前播放模式执行相应操作
 */
void lv_podcast_handle_music_next(void)
{
#ifdef APPLAYER_ENABLE
    lv_demo_music_play(1, "podcast");
#endif
}
// 上一个按钮点击事件处理函数
static void screen_podcast_last_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    switch (code) {
    case LV_EVENT_CLICKED: {
            // 上一个按钮点击事件处理逻辑
            printf("上一个故事按钮被点击\n");
            lv_podcast_handle_music_prev();
            // 这里可以添加切换到上一个故事的具体实现
            break;
        }
    default:
        break;
    }
}

// 下一个按钮点击事件处理函数
static void screen_podcast_next_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    switch (code) {
    case LV_EVENT_CLICKED: {
            // 下一个按钮点击事件处理逻辑
            printf("下一个故事按钮被点击\n");
            lv_podcast_handle_music_next();
            // 这里可以添加切换到下一个故事的具体实现
            break;
        }
    default:
        break;
    }
}

// 进度条事件处理函数
static void screen_podcast_slider_event_handler(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    lv_event_code_t event_code = lv_event_get_code(e);

    if (event_code == LV_EVENT_VALUE_CHANGED) {
        // 进度条值改变事件
        int32_t value = lv_slider_get_value(slider);
        printf("[进度事件] 进度值已更改为: %d%%\n", (int)value);

        // 这里可以添加根据进度更新故事播放位置的实现
    }
}
// 播放/暂停按钮显示处理函数
void async_screen_podcast_btn_play_pause_show(void* param)
{
    bool is_pause = (bool)(intptr_t)param;
    static bool pause_status = -1;
    if (!is_podcast_page() || pause_status == is_pause) {
        return;
    }
    pause_status = is_pause;
    if (!is_pause) {
        // 设置为播放
        if (guider_ui.screen_podcast_btn_play_pause_label) {
            lv_label_set_text(guider_ui.screen_podcast_btn_play_pause_label, "I I");
            lv_obj_set_style_text_font(guider_ui.screen_podcast_btn_play_pause_label, &lv_font_Barlow__18, LV_PART_MAIN | LV_STATE_DEFAULT);
            tape_rotate_start();
            lv_label_set_long_mode(guider_ui.screen_podcast_podcast_title, LV_LABEL_LONG_SCROLL_CIRCULAR);
        }
        // start_img_music_animation(); // 开始 img_music 动画
    } else {
        // 设置为暂停
        if (guider_ui.screen_podcast_btn_play_pause_label) {
            lv_label_set_text(guider_ui.screen_podcast_btn_play_pause_label, "   "LV_SYMBOL_PLAY " ");
            lv_obj_set_style_text_font(guider_ui.screen_podcast_btn_play_pause_label, &lv_font_Barlow__18, LV_PART_MAIN | LV_STATE_DEFAULT);
            tape_rotate_stop();

            lv_label_set_long_mode(guider_ui.screen_podcast_podcast_title, LV_LABEL_LONG_DOT);
            lv_anim_del(guider_ui.screen_podcast_img_music, (lv_anim_exec_xcb_t)lv_img_set_zoom); // 停止 img_music 动画
        }
    }
}
// 异步调用播放/暂停按钮显示处理函数
void screen_podcast_btn_play_pause_show(bool is_pause)
{
//    printf("异步调用播放/暂停按钮显示处理函数: %s\n", is_pause ? "暂停" : "播放");
    lv_async_call(async_screen_podcast_btn_play_pause_show, (void *)is_pause);
}
/**
 * @brief 故事界面事件初始化函数
 * 为故事界面的各个组件添加事件处理
 */
void podcast_event_init(lv_ui *ui)
{
    // 添加这行代码完全禁止页面滚动
    lv_obj_set_scroll_dir(ui->screen_podcast, LV_DIR_NONE);

    // 为播放/暂停按钮添加点击事件
    lv_obj_remove_event_cb(ui->screen_podcast_btn_play_pause, NULL);
    lv_obj_add_event_cb(ui->screen_podcast_btn_play_pause, screen_podcast_btn_play_pause_event_handler, LV_EVENT_ALL, ui);

    // 初始化磁带动画状态
    init_tape_animation_state();
    if (g_play_pause_btn_state) {
        // 设置为播放
        lv_label_set_text(ui->screen_podcast_btn_play_pause_label, "I I");
        lv_obj_set_style_text_font(ui->screen_podcast_btn_play_pause_label, &lv_font_Barlow__18, LV_PART_MAIN | LV_STATE_DEFAULT);
        tape_rotate_start();
        // start_img_music_animation(); // 开始 img_music 动画
    }

    // 为上一个按钮添加点击事件
    lv_obj_remove_event_cb(ui->screen_podcast_last, NULL);
    lv_obj_add_event_cb(ui->screen_podcast_last, screen_podcast_last_event_handler, LV_EVENT_CLICKED, ui);

    // 为下一个按钮添加点击事件
    lv_obj_remove_event_cb(ui->screen_podcast_next, NULL);
    lv_obj_add_event_cb(ui->screen_podcast_next, screen_podcast_next_event_handler, LV_EVENT_CLICKED, ui);

    // 为进度条添加事件
    lv_obj_remove_event_cb(ui->screen_podcast_slider, NULL);
    lv_obj_add_event_cb(ui->screen_podcast_slider, screen_podcast_slider_event_handler, LV_EVENT_ALL, ui);
}

/**
 * @brief 故事界面事件设置函数
 * 初始化故事界面的所有事件
 */
void podcast_event_setup(void)
{
    podcast_event_init(&guider_ui);
}
