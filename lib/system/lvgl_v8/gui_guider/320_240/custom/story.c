#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdbool.h"
#include "gui_guider.h"
#include "custom.h"

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
    lv_img_set_angle(guider_ui.screen_story_img_tape, 0); // 重置角度为0度
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
    lv_img_set_angle(guider_ui.screen_story_img_tape, angle);
}

/**
 * @brief 启动磁带动画函数
 * 开始磁带旋转动画，并设置相关图标状态
 */
static void tape_rotate_start(void)
{
    // 设置磁带条的角度为-80度
    lv_img_set_angle(guider_ui.screen_story_img_bar, -80);
    
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
    lv_img_set_angle(guider_ui.screen_story_img_bar, 110);
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
    lv_anim_set_var(&a, guider_ui.screen_story_img_music);
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
    lv_anim_del(guider_ui.screen_story_img_music, (lv_anim_exec_xcb_t)lv_img_set_zoom);
    
    // 初始化动画
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, guider_ui.screen_story_img_music);
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
static void screen_story_btn_play_pause_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        // 切换按钮状态
        g_play_pause_btn_state = !g_play_pause_btn_state;
        if (g_play_pause_btn_state) {
            // 设置为播放
            lv_label_set_text(ui->screen_story_btn_play_pause_label, "I I");
            lv_obj_set_style_text_font(ui->screen_story_btn_play_pause_label, &lv_font_Barlow__18, LV_PART_MAIN|LV_STATE_DEFAULT);
            tape_rotate_start();
            // start_img_music_animation(); // 开始 img_music 动画
        } else {
            // 设置为暂停
            lv_label_set_text(ui->screen_story_btn_play_pause_label, "   "LV_SYMBOL_PLAY " ");
            lv_obj_set_style_text_font(ui->screen_story_btn_play_pause_label, &lv_font_Barlow__18, LV_PART_MAIN|LV_STATE_DEFAULT);
            tape_rotate_stop();
            // lv_anim_del(guider_ui.screen_story_img_music, (lv_anim_exec_xcb_t)lv_img_set_zoom); // 停止 img_music 动画
        }
        break;
    }
    default:
        break;
    }
}

// 上一个按钮点击事件处理函数
static void screen_story_last_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        // 上一个按钮点击事件处理逻辑
        printf("上一个故事按钮被点击\n");
        // 这里可以添加切换到上一个故事的具体实现
        break;
    }
    default:
        break;
    }
}

// 下一个按钮点击事件处理函数
static void screen_story_next_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        // 下一个按钮点击事件处理逻辑
        printf("下一个故事按钮被点击\n");
        // 这里可以添加切换到下一个故事的具体实现
        break;
    }
    default:
        break;
    }
}

// 进度条事件处理函数
static void screen_story_slider_event_handler(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    lv_event_code_t event_code = lv_event_get_code(e);
    
    if(event_code == LV_EVENT_VALUE_CHANGED) {
        // 进度条值改变事件
        int32_t value = lv_slider_get_value(slider);
        printf("[进度事件] 进度值已更改为: %d%%\n", (int)value);
        
        // 这里可以添加根据进度更新故事播放位置的实现
    }
}

/**
 * @brief 故事界面事件初始化函数
 * 为故事界面的各个组件添加事件处理
 */
void story_event_init(lv_ui *ui) {
    // 添加这行代码完全禁止页面滚动
    lv_obj_set_scroll_dir(ui->screen_story, LV_DIR_NONE);
    
    // 为播放/暂停按钮添加点击事件
    lv_obj_remove_event_cb(ui->screen_story_btn_play_pause, NULL);
    lv_obj_add_event_cb(ui->screen_story_btn_play_pause, screen_story_btn_play_pause_event_handler, LV_EVENT_ALL, ui);
    
    // 初始化磁带动画状态
    init_tape_animation_state();
    if (g_play_pause_btn_state) {
        // 设置为播放
        lv_label_set_text(ui->screen_story_btn_play_pause_label, "I I");
        lv_obj_set_style_text_font(ui->screen_story_btn_play_pause_label, &lv_font_Barlow__18, LV_PART_MAIN|LV_STATE_DEFAULT);
        tape_rotate_start();
        // start_img_music_animation(); // 开始 img_music 动画
    }
    
    // 为上一个按钮添加点击事件
    lv_obj_remove_event_cb(ui->screen_story_last, NULL);
    lv_obj_add_event_cb(ui->screen_story_last, screen_story_last_event_handler, LV_EVENT_CLICKED, ui);
    
    // 为下一个按钮添加点击事件
    lv_obj_remove_event_cb(ui->screen_story_next, NULL);
    lv_obj_add_event_cb(ui->screen_story_next, screen_story_next_event_handler, LV_EVENT_CLICKED, ui);
    
    // 为进度条添加事件
    lv_obj_remove_event_cb(ui->screen_story_slider, NULL);
    lv_obj_add_event_cb(ui->screen_story_slider, screen_story_slider_event_handler, LV_EVENT_ALL, ui);
}

/**
 * @brief 故事界面事件设置函数
 * 初始化故事界面的所有事件
 */
void story_event_setup(void) {
    story_event_init(&guider_ui);
}


  
