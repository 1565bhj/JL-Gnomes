#include "lvgl.h"
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"

#if TOMATO_TIMER_ENABLE
// 定义10个位置的坐标数组（x, y）
static const lv_point_t positions[] = {
    {136, 2},    // 位置1
    {79, 16},   // 位置2
    {47, 63},   // 位置3
    {42, 122},   // 位置4
    {78, 168},  // 位置5
    {132, 185},  // 位置6
    {189, 170},   // 位置7
    {225, 124},   // 位置8
    {224, 68},  // 位置9
    {191, 20}  // 位置10

};



// 定义最大生成的hide_part数量
#define MAX_HIDE_PART_COUNT 10

// hide_part对象数组
static lv_obj_t *hide_part_array[MAX_HIDE_PART_COUNT] = {NULL};

// 当前显示的hide_part索引
static uint8_t current_hide_part_index = 0;

// hide_part显示的时间间隔
static uint32_t hide_part_interval = 0;

// 倒计时相关变量
static uint32_t remaining_time = 0;
static uint32_t initial_time = 0;  // 初始时间，用于计算已过去的时间
static bool is_counting = false;
static lv_timer_t *countdown_timer = NULL;

// 定义番茄时间更新的参数结构体
typedef struct {
    lv_ui *ui;       // UI对象指针
    uint8_t hour;    // 小时
    uint8_t minute;  // 分钟
    uint8_t second;  // 秒
} tomato_time_params_t;

// 函数前向声明
static void update_time_display(void);
static void countdown_timer_cb(lv_timer_t *timer);
static void parse_time_from_display(void);
static void async_ui_update_tomato_time(void* param);
static void async_app_update_tomato_time(void* param);
static void Sync_ui_update_tomato_time(lv_ui *ui, uint8_t hour, uint8_t minute, uint8_t second);
static void Sync_app_update_tomato_time(lv_ui *ui, uint8_t hour, uint8_t minute, uint8_t second);

/**
 * @brief 从显示中解析时间
 */
static void parse_time_from_display(void)
{
    if (guider_ui.screen_tomato_time) {
        const char *time_str = lv_label_get_text(guider_ui.screen_tomato_time);
        uint32_t minutes, seconds;

        // 解析时间字符串 MM:SS
        if (sscanf(time_str, "%02d:%02d", &minutes, &seconds) == 2) {
            remaining_time = minutes * 60 + seconds;
            initial_time = remaining_time;  // 保存初始时间
            printf("Parsed time: %d seconds, initial_time set to %d\n", remaining_time, initial_time);
        }
    }
}

/**
 * @brief 初始化所有hide_part对象
 */
static void init_all_hide_parts(lv_ui *ui)
{
    uint8_t i;

    // 一次性生成所有hide_part对象
    for (i = 0; i < MAX_HIDE_PART_COUNT; i++) {

        // 创建新的hide_part对象
        lv_obj_t *new_hide_part = lv_img_create(ui->screen_tomato);
        lv_obj_add_flag(new_hide_part, LV_OBJ_FLAG_CLICKABLE);
        lv_img_set_src(new_hide_part, &_hide_part_alpha_52x51);
        lv_img_set_pivot(new_hide_part, 50, 50);
        lv_img_set_angle(new_hide_part, 0);

        // 设置位置
        lv_obj_set_pos(new_hide_part, positions[i].x, positions[i].y);
        lv_obj_set_size(new_hide_part, 52, 51);

        // 设置样式（与原来的对象相同）
        lv_obj_set_style_img_recolor_opa(new_hide_part, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_img_opa(new_hide_part, 230, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(new_hide_part, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_clip_corner(new_hide_part, true, LV_PART_MAIN | LV_STATE_DEFAULT);

        // 默认隐藏新创建的对象
        lv_obj_add_flag(new_hide_part, LV_OBJ_FLAG_HIDDEN);

        // 存储到数组中
        hide_part_array[i] = new_hide_part;

    }
}



/**
 * @brief screen_tomato_img_add点击事件处理函数
 */
static void screen_tomato_img_add_event_handler(lv_event_t *event)
{
    lv_event_code_t code = lv_event_get_code(event);
    if (code == LV_EVENT_CLICKED) {
        // 停止倒计时
        if (countdown_timer) {
            lv_timer_del(countdown_timer);
            countdown_timer = NULL;
        }
        is_counting = false;
        switch_to_timer_add_page();
    }
}

/**
 * @brief screen_tomato_img_reset点击事件处理函数
 */
static void screen_tomato_img_reset_event_handler(lv_event_t *event)
{
    lv_event_code_t code = lv_event_get_code(event);
    if (code == LV_EVENT_CLICKED) {
        // 停止倒计时
        if (countdown_timer) {
            lv_timer_del(countdown_timer);
            countdown_timer = NULL;
        }
        is_counting = false;

        // 隐藏所有hide_part
        uint8_t i;
        for (i = 0; i < MAX_HIDE_PART_COUNT; i++) {
            if (hide_part_array[i] != NULL) {
                lv_obj_add_flag(hide_part_array[i], LV_OBJ_FLAG_HIDDEN);
            }
        }

        // 重置当前显示的hide_part索引
        current_hide_part_index = 0;
        remaining_time = 0;
        // 更新显示
        update_time_display();
    }
}

/**
 * @brief 更新时间显示
 */
static void update_time_display_cb(void* p)
{
    char time_str[6];
    uint32_t minutes = remaining_time / 60;
    uint32_t seconds = remaining_time % 60;

    // 确保分钟数不超过99，秒数不超过59
    if (minutes > 99) {
        minutes = 99;
        seconds = 59;
    }

    // 格式化时间为MM:SS
    sprintf(time_str, "%02d:%02d", minutes, seconds);

    // 更新屏幕显示
    if (guider_ui.screen_tomato_time) {
        lv_label_set_text(guider_ui.screen_tomato_time, time_str);
    }
}
/**
 * @brief 更新时间显示
 */
static void update_time_display(void)
{
    lv_async_call(update_time_display_cb, NULL);
}
/**
 * @brief 倒计时定时器回调函数
 */
static void countdown_timer_cb(lv_timer_t *timer)
{
    if (remaining_time > 0) {
        remaining_time--;
        update_time_display();

        // 计算已经过去的时间
        uint32_t elapsed_time = initial_time - remaining_time;

        // 计算应该显示的hide_part索引
        if (hide_part_interval > 0) {
            uint8_t target_index = elapsed_time / hide_part_interval;

            // 如果目标索引大于当前索引且小于总数量，显示对应的hide_part
            if (target_index > current_hide_part_index && target_index < MAX_HIDE_PART_COUNT) {
                // 显示下一个hide_part
                if (hide_part_array[target_index] != NULL) {
                    lv_obj_clear_flag(hide_part_array[target_index], LV_OBJ_FLAG_HIDDEN);
                    printf("Displaying hide_part[%d] at elapsed time %d\n", target_index, elapsed_time);
                }
                current_hide_part_index = target_index;
            }
        }
    } else {
        // 时间到，停止倒计时
        is_counting = false;
        lv_timer_del(timer);
        countdown_timer = NULL;

        // 隐藏所有hide_part
        uint8_t i;
        for (i = 0; i < MAX_HIDE_PART_COUNT; i++) {
            if (hide_part_array[i] != NULL) {
                lv_obj_add_flag(hide_part_array[i], LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

/**
 * @brief 初始化番茄时间功能
 */
static void tomato_time_init(void)
{
    // 初始化所有hide_part对象
    init_all_hide_parts(&guider_ui);

    // 从显示中解析初始时间
    parse_time_from_display();

    // 计算hide_part显示的时间间隔
    if (MAX_HIDE_PART_COUNT > 0 && remaining_time > 0) {
        hide_part_interval = remaining_time / MAX_HIDE_PART_COUNT;
        printf("Hide_part interval calculated: %d seconds\n", hide_part_interval);
    }

    // 初始化时间显示
    update_time_display();

    // 重置当前显示的hide_part索引
    current_hide_part_index = 0;

    // 如果解析的时间不等于0，自动启动倒计时
    if (remaining_time > 0 && !is_counting) {
        // 创建并启动定时器（1秒间隔）
        countdown_timer = lv_timer_create(countdown_timer_cb, 1000, NULL);
        is_counting = true;
        printf("Automatic countdown started with %d seconds\n", remaining_time);
    }
}

void tomato_time_event_setup(void)
{

    lv_obj_set_scroll_dir(guider_ui.screen_tomato, LV_DIR_NONE);
    // 初始化番茄时间功能
    tomato_time_init();

    // 注册按键点击事件；扩大触控热区（与子页返回键同策略）
    if (guider_ui.screen_tomato_img_add) {
        ui_subpage_return_set_large_touch_area(guider_ui.screen_tomato_img_add, (lv_coord_t)UI_SUBPAGE_RETURN_EXT_PAD);
        lv_obj_add_event_cb(guider_ui.screen_tomato_img_add, screen_tomato_img_add_event_handler, LV_EVENT_CLICKED, NULL);
    }

    if (guider_ui.screen_tomato_img_reset) {
        ui_subpage_return_set_large_touch_area(guider_ui.screen_tomato_img_reset, (lv_coord_t)UI_SUBPAGE_RETURN_EXT_PAD);
        lv_obj_add_event_cb(guider_ui.screen_tomato_img_reset, screen_tomato_img_reset_event_handler, LV_EVENT_CLICKED, NULL);
    }
}

/**
 * @brief 番茄钟：仅 LVGL/UI（与 Sync_ui_dynamic_add_new_timer 对普通倒计时的拆分一致）
 */
static void async_ui_update_tomato_time(void* param)
{
    if (!param) {
        return;
    }

    tomato_time_params_t *tomato_params = (tomato_time_params_t *)param;

    char time_str[9];
    uint32_t total_seconds = tomato_params->hour * 3600 + tomato_params->minute * 60 + tomato_params->second;
    remaining_time = total_seconds;
    initial_time = total_seconds;

    uint32_t display_minutes = total_seconds / 60;
    uint32_t display_seconds = total_seconds % 60;
    if (display_minutes > 99) {
        display_minutes = 99;
        display_seconds = 59;
        printf("Warning: Time exceeds 99:59 limit, displaying 99:59\n");
    }
    sprintf(time_str, "%02d:%02d", display_minutes, display_seconds);

    if (tomato_params->ui->screen_tomato_time) {
        lv_label_set_text(tomato_params->ui->screen_tomato_time, time_str);
    }

    if (countdown_timer) {
        lv_timer_del(countdown_timer);
        countdown_timer = NULL;
    }
    is_counting = false;

    uint8_t i;
    for (i = 0; i < MAX_HIDE_PART_COUNT; i++) {
        if (hide_part_array[i] != NULL) {
            lv_obj_add_flag(hide_part_array[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
    current_hide_part_index = 0;

    if (MAX_HIDE_PART_COUNT > 0 && remaining_time > 0) {
        hide_part_interval = remaining_time / MAX_HIDE_PART_COUNT;
        printf("Hide_part interval recalculated: %d seconds\n", hide_part_interval);
    }

    if (remaining_time > 0 && !is_counting) {
        countdown_timer = lv_timer_create(countdown_timer_cb, 1000, NULL);
        is_counting = true;
        printf("Countdown started with %d seconds\n", remaining_time);
    }

    printf("Updated tomato time to %02d:%02d:%02d (displayed as %02d:%02d, total seconds: %u)\n",
           tomato_params->hour, tomato_params->minute, tomato_params->second, display_minutes, display_seconds, total_seconds);
    free(param);
}

/**
 * @brief 番茄钟：仅应用层（与 timer_event 中 async_app_dynamic_add_new_timer 一致：按时长决定是否启动/停止，勿用 is_counting；两根 lv_async_call(0ms) 执行顺序不确定，is_counting 可能晚于本回调更新）
 */
static void async_app_update_tomato_time(void* param)
{
    if (!param) {
        return;
    }
    tomato_time_params_t *tomato_params = (tomato_time_params_t *)param;
#ifdef APPLAYER_ENABLE
    start_timer_countdown(tomato_params->hour, tomato_params->minute, tomato_params->second, true);
    printf("开始应用层倒计时start_timer_countdown: %d:%d:%d\n", tomato_params->hour, tomato_params->minute, tomato_params->second);
#endif
    free(param);
}

static void Sync_ui_update_tomato_time(lv_ui *ui, uint8_t hour, uint8_t minute, uint8_t second)
{
    tomato_time_params_t *params = (tomato_time_params_t *)malloc(sizeof(tomato_time_params_t));
    if (params) {
        params->ui = ui;
        params->hour = hour;
        params->minute = minute;
        params->second = second;
        lv_async_call(async_ui_update_tomato_time, (void *)params);
    }
}



static void Sync_app_update_tomato_time(lv_ui *ui, uint8_t hour, uint8_t minute, uint8_t second)
{
    tomato_time_params_t *params = (tomato_time_params_t *)malloc(sizeof(tomato_time_params_t));
    if (params) {
        params->ui = ui;
        params->hour = hour;
        params->minute = minute;
        params->second = second;
        lv_async_call(async_app_update_tomato_time, (void *)params);
    }
    (void)ui;
}

void update_tomato_time(lv_ui *ui, uint8_t hour, uint8_t minute, uint8_t second)
{
    Sync_ui_update_tomato_time(ui, hour, minute, second);
    Sync_app_update_tomato_time(ui, hour, minute, second);
}
void Voice_dynamic_add_new_tomato_timer(int timeout)
{
    int hour = timeout / 3600;
    int minute = (timeout % 3600) / 60;
    int second = timeout % 60;
    Sync_ui_update_tomato_time(&guider_ui, hour, minute, second);
}

#else

void tomato_time_event_setup(void)
{
}
#endif


