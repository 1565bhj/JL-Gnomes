#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdbool.h"
#include "gui_guider.h"
#include "custom.h"

// 图标数量
#define OPTION_ICON_COUNT          8
// 图标之间的距离
#define OPTION_ICON_DISTANCE       78


// 定义图标结构体，包含图标对象指针和x坐标
typedef struct {
    lv_obj_t *obj;
    int32_t x;
    int32_t icon_idx; // 图标索引，用于标识不同的功能
} option_icon_typedef;

// 声明图标结构体数组
static option_icon_typedef icon[OPTION_ICON_COUNT];
// 触摸状态标志
static bool touched = false;
// 屏幕宽度
static int32_t scr_w;

// 触摸偏移量x
static int32_t t_offset_x;
// 屏幕对象指针
static lv_obj_t *screen;
// 新增：当前最大的图标索引
static int32_t current_max_icon_idx = 0;

// 按下事件回调函数声明
static void  pressing_cb(lv_event_t * e);
// 释放事件回调函数声明
static void  released_cb(lv_event_t * e);
// 设置x坐标回调函数声明
static void set_x_cb(void * var, int32_t v);
// 自定义动画创建函数声明
static void lv_myanim_creat(void * var, lv_anim_exec_xcb_t exec_cb, uint32_t time, uint32_t delay, lv_anim_path_cb_t path_cb,
                            int32_t start, int32_t end, lv_anim_ready_cb_t completed_cb, lv_anim_deleted_cb_t deleted_cb);
// 图标点击事件回调函数声明
static void icon_click_cb(lv_event_t * e);
// 新增：打开对应屏幕函数声明
static void open_icon_screen(int32_t icon_idx);
void lv_screen_open_icon_screen_set(short x, short y);
// 图标定义宏 - 修复：使用正确的图标资源名称
#define ICON_DEFINITIONS \
    ICON_DEF(0, _op_timer_66x85, "倒计时") \
    ICON_DEF(1, _op_alarm_66x85, "闹钟") \
    ICON_DEF(2, _op_music_66x85, "音乐") \
    ICON_DEF(3, _op_conversation_66x85, "AI对话") \
    ICON_DEF(4, _op_draw_lots_game_66x85, "抽签") \
    ICON_DEF(5, _op_whack_game_66x85, "打地鼠") \
    ICON_DEF(6, _op_dice_game_66x85, "摇骰子") \
    ICON_DEF(7, _op_podcast_66x85, "播客")


// 滚动图标功能函数
void main_scrollicon(void)
{
    int32_t i;
    // 获取默认显示器的水平分辨率（屏幕宽度）
    scr_w = lv_disp_get_hor_res(lv_disp_get_default());
    // 初始化图标结构体数组为0
    lv_memset(icon, 0, sizeof(icon));

    // 禁用生成主屏幕的滚动
    if (guider_ui.screen_main) {
        lv_obj_set_scroll_dir(guider_ui.screen_main, LV_DIR_NONE);
    }

    // 确保主容器不会裁剪其子对象，允许图标在容器外显示
    // 在LVGL v8中，通过设置clip_corner为0来禁用裁剪
    lv_obj_set_style_clip_corner(guider_ui.screen_main_cont, 0, LV_PART_MAIN);

    // // 记录实际使用的容器
    screen = guider_ui.screen_main_cont;

    // 清除屏幕的可滚动标志
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    // 为屏幕添加按下事件回调函数
    lv_obj_add_event_cb(screen, pressing_cb, LV_EVENT_PRESSING, 0);
    // 为屏幕添加释放事件回调函数
    lv_obj_add_event_cb(screen, released_cb, LV_EVENT_RELEASED, 0);

    for (i = 0; i < OPTION_ICON_COUNT; i++) {
        // // 创建图标对象并添加到屏幕上
        icon[i].obj = lv_img_create(screen);
        // 设置图标对象的用户数据为对应的图标结构体指针
        icon[i].obj->user_data = &icon[i];
        // 设置图标索引
        icon[i].icon_idx = i;

//        // 确保图标可点击
//        lv_obj_add_flag(icon[i].obj, LV_OBJ_FLAG_CLICKABLE);
////        // 为图标添加点击事件回调
//        lv_obj_add_event_cb(icon[i].obj, icon_click_cb, LV_EVENT_CLICKED, NULL);

        // 设置图标大小
        lv_obj_set_size(icon[i].obj, 66, 85);

        // 计算并设置图标初始X坐标 - 左侧留出空间，右侧额外添加空间以便循环
        icon[i].x = 10 + i * OPTION_ICON_DISTANCE;
        lv_obj_set_x(icon[i].obj, icon[i].x);

        // 设置图标Y坐标，使其垂直居中
        lv_obj_set_y(icon[i].obj, 10);

        // 使用宏定义设置图标源和名称
#define ICON_DEF(idx, src, name) \
            if (i == idx) { \
                lv_img_set_src(icon[idx].obj, &src); \
            }
        ICON_DEFINITIONS
#undef ICON_DEF

    }
}



// 按下事件回调函数
static void  pressing_cb(lv_event_t * e)
{
    static  lv_point_t click_point1, click_point2;
    int32_t v, i;

    // 如果当前未处于触摸状态
    if (touched == false) {
        for (i = 0; i < OPTION_ICON_COUNT; i++) {
            // 删除图标对象上的动画（如果有）
            lv_anim_del(icon[i].obj, set_x_cb);
        }

        // 获取当前输入设备的点击点坐标
        lv_indev_get_point(lv_indev_get_act(), &click_point1);
        // 设置触摸状态为已触摸
        touched = true;
        return;
    } else {
        // 如果已经处于触摸状态，获取当前点击点坐标
        lv_indev_get_point(lv_indev_get_act(), &click_point2);
    }

    // 计算触摸偏移量x
    t_offset_x = click_point2.x - click_point1.x;
    // 更新上一次点击点坐标
    click_point1.x = click_point2.x;

    for (int32_t i = 0; i < OPTION_ICON_COUNT; i++) {
        // 更新图标x坐标
        icon[i].x += t_offset_x;

        // 获取图标宽度
        int32_t icon_width = lv_obj_get_width(icon[i].obj);
        int32_t total_width = OPTION_ICON_COUNT * OPTION_ICON_DISTANCE;

        // 处理x坐标超出范围的情况（循环滚动）
        // 左移逻辑：当图标开始移出屏幕左侧时，将其移到右侧
        if (icon[i].x < -icon_width) {
            icon[i].x += total_width;
        }
        // 右移逻辑：当图标完全移出屏幕右侧时，将其移到左侧
        else if (icon[i].x > (scr_w)) { // 当图标完全移出屏幕右侧时才循环
            icon[i].x -= total_width;
        }

        // 设置图标对象的x坐标
        lv_obj_set_x(icon[i].obj, icon[i].x);
    }
}



// 释放事件回调函数
static void  released_cb(lv_event_t * e)
{
    lv_point_t release_pos;
    // 获取释放时的坐标
    lv_indev_get_point(lv_indev_get_act(), &release_pos);

    int32_t offset_x = 0;
    // 设置触摸状态为未触摸
    touched = false;

    // 计算偏移量，使图标对齐到网格
    // 找到第一个可见图标来计算对齐偏移
    for (int32_t i = 0; i < OPTION_ICON_COUNT; i++) {
        // 计算当前图标的对齐偏移
        int32_t current_offset = icon[i].x % OPTION_ICON_DISTANCE;
        if (current_offset > OPTION_ICON_DISTANCE / 2) {
            offset_x = OPTION_ICON_DISTANCE - current_offset;
        } else {
            offset_x = -current_offset;
        }
        break;
    }

    for (int32_t i = 0; i < OPTION_ICON_COUNT; i++) {
        // 更新图标x坐标
        int32_t new_x = icon[i].x + offset_x;
        int32_t icon_width = lv_obj_get_width(icon[i].obj);
        int32_t total_width = OPTION_ICON_COUNT * OPTION_ICON_DISTANCE;

        // 处理x坐标超出范围的情况（循环滚动）
        // 左移逻辑：当图标开始移出屏幕左侧时，将其移到右侧
        if (new_x < -icon_width * 5) {
            new_x += total_width;
        }
        // 右移逻辑：当图标开始移出屏幕右侧时，将其移到左侧
        else if (new_x > icon_width * OPTION_ICON_COUNT * 2) { // 320是屏幕宽度，当图标开始进入右侧边界时就循环
            new_x -= total_width;
        }

        // 创建动画，使图标回到合适位置
        lv_myanim_creat(icon[i].obj, set_x_cb, 300, 0, lv_anim_path_ease_out,
                        icon[i].x, new_x + 10, 0, 0);

        // 更新图标x坐标
        icon[i].x = new_x;
    }


}
void lv_screen_open_icon_screen_set(short x, short y)
{
    // 获取容器的坐标
    lv_coord_t cont_x = lv_obj_get_x(screen);
    lv_coord_t cont_y = lv_obj_get_y(screen);
    printf("--> x %d , y %d\n", x, y);
    // 遍历所有图标，检查点击位置是否在任何图标上
    for (int32_t i = 0; i < OPTION_ICON_COUNT; i++) {
        // 获取当前图标的位置和大小
        lv_coord_t icon_x = lv_obj_get_x(icon[i].obj);
        lv_coord_t icon_y = lv_obj_get_y(icon[i].obj);
        lv_coord_t icon_w = lv_obj_get_width(icon[i].obj);
        lv_coord_t icon_h = lv_obj_get_height(icon[i].obj);

        // 计算图标相对于屏幕的绝对坐标边界
        lv_coord_t icon_left = icon_x + cont_x;
        lv_coord_t icon_top = icon_y + cont_y;
        lv_coord_t icon_right = icon_left + icon_w;
        lv_coord_t icon_bottom = icon_top + icon_h;

        // 判断点击是否在当前图标范围内
        if (x >= icon_left && x <= icon_right && y >= icon_top && y <= icon_bottom) {
            printf("--> 点击图标 %d\n", i);
            open_icon_screen(i);
        }
    }
}

// 自定义动画创建函数
static void lv_myanim_creat(void * var, lv_anim_exec_xcb_t exec_cb, uint32_t time, uint32_t delay, lv_anim_path_cb_t path_cb,
                            int32_t start, int32_t end, lv_anim_ready_cb_t completed_cb, lv_anim_deleted_cb_t deleted_cb)
{
    lv_anim_t xxx;
    // 初始化动画对象
    lv_anim_init(&xxx);
    // 设置动画对象的变量
    lv_anim_set_var(&xxx, var);
    // 设置动画执行回调函数
    lv_anim_set_exec_cb(&xxx, exec_cb);
    // 设置动画时间
    lv_anim_set_time(&xxx, time);
    // 设置动画延迟
    lv_anim_set_delay(&xxx, delay);
    // 设置动画的起始值和结束值
    lv_anim_set_values(&xxx, start, end);
    // 如果有路径回调函数，设置路径回调函数
    if (path_cb) {
        lv_anim_set_path_cb(&xxx, path_cb);
    }
    // 如果有动画完成回调函数，设置动画完成回调函数
    if (completed_cb) {
        lv_anim_set_ready_cb(&xxx, completed_cb);
    }
    // 如果有动画删除回调函数，设置动画删除回调函数
    if (deleted_cb) {
        lv_anim_set_deleted_cb(&xxx, deleted_cb);
    }
    // 启动动画
    lv_anim_start(&xxx);
}

// 设置x坐标回调函数
static void set_x_cb(void * var, int32_t v)
{
    lv_obj_t *obj = (lv_obj_t *)var;
    int32_t icon_width = lv_obj_get_width(obj);
    int32_t total_width = OPTION_ICON_COUNT * OPTION_ICON_DISTANCE;

    // 处理x坐标超出范围的情况（循环滚动）
    // 左移逻辑：当图标开始移出屏幕左侧时，将其移到右侧
    if (v < -icon_width) {
        v += total_width;
    }
    // 右移逻辑：当图标完全移出屏幕右侧时，将其移到左侧
    else if (v > (scr_w)) { // 当图标完全移出屏幕右侧时才循环
        v -= total_width;
    }

    // 设置对象的x坐标
    lv_obj_set_x(obj, v);

    // 获取图标结构体指针并更新x坐标
    option_icon_typedef * icon_ptr = (option_icon_typedef *)obj->user_data;
    if (icon_ptr) {
        icon_ptr->x = v;
    }
}


// 图标点击事件回调函数实现
static void icon_click_cb(lv_event_t * e)
{
    // 获取事件目标对象
    lv_obj_t *obj = lv_event_get_target(e);
    // 获取图标结构体指针
    option_icon_typedef * icon_ptr = (option_icon_typedef *)obj->user_data;
    // 调用打开对应屏幕函数
    open_icon_screen(icon_ptr->icon_idx);
}

// 新增：打开对应屏幕函数
static void open_icon_screen(int32_t icon_idx)
{
    //  根据不同的图标索引打开不同的屏幕
    switch (icon_idx) {
    case 0: //
        printf("点击进入倒计时页面");
        switch_to_timer_page();
        break;
    case 1: //
        printf("点击进入闹钟页面");
        switch_to_alarm_page();
        break;
    case 2: //
        printf("点击进入音乐播放页面");
        switch_to_music_page("net_music");
        break;
    case 3: //
        printf("点击进入对话页面");
        switch_to_conversation_page();
        break;
    case 4: //
        printf("点击进入抽签页面");
        switch_to_draw_lots_game_page();
        break;
    case 5: //
        printf("点击进入打地鼠页面");
        switch_to_whack_game_page();
        break;
    case 6: //
        printf("点击进入摇骰子页面");
        switch_to_dice_game_page();
        break;
    case 7: //
        printf("点击进入播客页面");
        switch_to_podcast_page();
        break;
    default:
        break;
    }

}


#ifdef APPLAYER_ENABLE
#include "asm/gpio.h"
#include "lv_conf.h"
#include "system/includes.h"
#include "lvgl.h"
#include "sys_time.h"
#include "gui_guider.h"
#include "cJSON_common\cJSON.h"


// 闹钟检测相关声明
int get_alarm_count(void); // 获取闹钟数量
void alarm_status_check(void);
int audio_app_mode_check(void);
// 时间更新配置参数
#define TIME_UPDATE_INTERVAL_MS     1000    // 时间轮询间隔(毫秒)
#define TIME_RETRY_INTERVAL_MS      1000    // 时间重试间隔(毫秒)
#define TIME_MAX_RETRY_COUNT        (600000 * 60)      // 最大重试次数

// 外部函数声明 - 来自alarm.c
extern uint8_t sys_time_to_weekday(struct sys_time *t);
// 外部函数声明
extern int get_sys_time(struct sys_time *t);  // 系统时间获取函数

// 函数声明
void start_time_update_with_net_wait(void);
void start_weather_update_with_net_wait(void);
// 声明用于页面跳转的函数
void switch_to_main_page(void);
void switch_to_image_page(void);

extern void alarm_ui_init(void);
// 网络状态控制变量
static bool g_network_failed = false;         // 网络连接失败标志
static int  g_time_retry_count = 0;           // 时间获取重试计数器
static int high_temp = 0;
static int low_temp = 0;

// 闹钟警告定时器ID
static uint16_t alarm_warning_hide_timer_id = 0;

// 全局UI对象
lv_ui guider_ui;

// 获取星期几的中文名称
static const char *get_weekday_name(int weekday)
{
    static const char *weekday_names[] = {
        "", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期日",
    };

    if (weekday >= 1 && weekday <= 7) {
        return weekday_names[weekday];
    }
    return "星期未更新";
}

typedef struct {
    char week_str[16];
    volatile bool time_update_pending;
} time_update_data_t;

static time_update_data_t g_weekday_data = {0};


/**
 * 时间更新数据结构
 * 用于线程安全的时间数据传递
 */
typedef struct {
    char        time_str[16];                 // 格式化时间 "HH:MM"
    char        date_str[16];                 // 格式化日期 "YYYY-MM-DD"
    char        week_str[16];                 // 格式化星期
    volatile bool time_updated;               // 时间更新标志位
} time_data_t;

static time_data_t g_time_data = {0};         // 全局时间数据

/**
 * 线程安全的时间数据更新
 * 仅获取和格式化时间，不操作UI
 */
static void time_data_update(void)
{
    struct sys_time current_time = {0};  // 显式初始化为0
    int ret = get_sys_time(&current_time);
    uint8_t weekday = sys_time_to_weekday(&current_time);

    // 获取系统时间失败检查
    if (ret != 0) {
        printf("err get time = %d\n", ret);
        return;
    }

    // 检查关键字段是否有效（如年份合理）
    if (current_time.year < 2024 || current_time.year > 2100) {
        printf("err year: %d\n", current_time.year);
        return;
    }

    // 每分钟打印一次详细时间信息
    static uint8_t last_minute = 0xFF;
    if (last_minute != current_time.min) {
        last_minute = current_time.min;
//        printf("[时间同步] 当前时间: %04d-%02d-%02d %02d:%02d:%02d\n",
//               current_time.year, current_time.month, current_time.day,
//               current_time.hour, current_time.min, current_time.sec);
    }
    // 校验时间字段合法性，防止异常值导致字符串过长
    if (current_time.hour < 0 || current_time.hour >= 24 ||
        current_time.min < 0 || current_time.min >= 60 ||
        current_time.sec < 0 || current_time.sec >= 60
       ) {
//        printf("[时间错误] 无效时间值: %d:%d\n", current_time.hour, current_time.min, current_time.sec);
        return;  // 放弃此次更新，避免缓冲区溢出
    }

    // 格式化时间字符串
    snprintf(g_time_data.time_str, sizeof(g_time_data.time_str),
             "%02d:%02d", current_time.hour, current_time.min);

    // 格式化日期字符串
    snprintf(g_time_data.date_str, sizeof(g_time_data.date_str),
             "%04d/%02d/%02d", current_time.year, current_time.month, current_time.day);

    snprintf(g_time_data.week_str, sizeof(g_time_data.week_str), "%s", get_weekday_name(weekday));

    // 设置更新标志
    g_time_data.time_updated = true;
}

/**
 * 处理时间UI更新
 * 在主线程中执行，确保UI操作的线程安全性
 */
void time_ui_update(void)
{
    if (g_time_data.time_updated) {
        g_time_data.time_updated = false;

        // 更新时间显示
        if (guider_ui.screen_main_span_time_span) {
            lv_span_set_text(guider_ui.screen_main_span_time_span, g_time_data.time_str);
            lv_obj_invalidate(guider_ui.screen_main_span_time_span);
//            printf("[UI更新] 时间显示已更新: %s\n", g_time_data.time_str);
        } else {
            printf("[UI错误] screen_main_span_time 对象无效\n");
        }
        // 更新下拉框时间显示
        if (guider_ui.screen_dropdown_time_span) {
            lv_span_set_text(guider_ui.screen_dropdown_time_span, g_time_data.time_str);
            lv_obj_invalidate(guider_ui.screen_dropdown_time_span);
            //            printf("[UI更新] 时间显示已更新: %s\n", g_time_data.time_str);
        } else {
            printf("[UI错误] screen_dropdown_time_span 对象无效\n");
        }

        // 注释：日期显示功能可根据需要启用
        if (guider_ui.screen_main_span_date_span) {
            lv_span_set_text(guider_ui.screen_main_span_date_span, g_time_data.date_str);
            lv_obj_invalidate(guider_ui.screen_main_span_date_span);
//             printf("[UI更新] 日期显示已更新: %s\n", g_time_data.time_str);
        } else {
            printf("[UI错误] screen_main_span_date 对象无效\n");
        }
        // 注释：日期显示功能可根据需要启用
        if (guider_ui.screen_main_span_week_span) {
            lv_span_set_text(guider_ui.screen_main_span_week_span, g_time_data.week_str);
//             printf("===============================\n");
//             printf("==week_str = %s=============\n",g_time_data.week_str);
//             printf("===============================\n");
            lv_obj_invalidate(guider_ui.screen_main_span_week_span);
//             printf("[UI更新] 星期显示已更新: %s\n", g_time_data.week_str);
        } else {
            printf("[UI错误] screen_main_span_week_span 对象无效\n");
        }

    }
}

/**
 * 时间更新定时器回调函数
 * 周期性触发时间数据更新
 */
static void time_update_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    time_data_update();  // 仅更新数据，不直接操作UI
}


static void network_connected_init_async(void *data);
/**
 * 网络连接检查与时间服务初始化
 * 网络未连接时进行重试
 */
static void network_check_and_init(void *priv)
{
    LV_UNUSED(priv);

    if (lv_demo_system_net_connect_success()) {  // 检查网络连接状态
        //printf("[网络状态] 已连接，初始化时间服务...\n");
        lv_async_call(network_connected_init_async, NULL);
    } else {
        g_time_retry_count++;
        //printf("[网络状态] 时间更新尝试 %d: 网络未连接\n", g_time_retry_count);

        if (g_time_retry_count < TIME_MAX_RETRY_COUNT) {
            // 继续重试
            sys_timeout_add_to_task("sys_timer", NULL, network_check_and_init, TIME_RETRY_INTERVAL_MS);
        } else {
            printf("[网络错误] 超过最大重试次数(%d)，时间服务初始化失败\n",
                   TIME_MAX_RETRY_COUNT);
            g_network_failed = true;
        }
    }
}

/**
 * 启动时间更新流程
 * 开始网络等待和时间同步
 */
void start_time_update_with_net_wait(void)
{
    g_time_retry_count = 0;
    //printf("\n[时间服务] 启动时间更新流程，等待网络连接...\n");
    sys_timeout_add_to_task("sys_timer", NULL, network_check_and_init, 0); // 立即开始第一次检查
}


/**
 * 时间更新包装函数
 * 用于异步调用时间更新
 */
static void time_update_async_cb(void *data)
{
    LV_UNUSED(data);
    time_data_update();
    time_ui_update();    // 立即更新UI
}

/**
 * 网络连接后的初始化操作
 * 启动时间更新定时器
 */
static void network_connected_init(void)
{
    // 异步更新时间
    lv_async_call(time_update_async_cb, NULL);

    // 创建时间更新定时器
    static lv_timer_t *time_update_timer = NULL;
    if (!time_update_timer) {
        time_update_timer = lv_timer_create(time_update_timer_cb, TIME_UPDATE_INTERVAL_MS, NULL);
    }

    int fetch_weather_data(void);  // 调用天气函数
    if (fetch_weather_data() && ++g_time_retry_count <= TIME_MAX_RETRY_COUNT) {  // 调用天气函数
        sys_timeout_add_to_task("sys_timer", NULL, network_check_and_init, TIME_RETRY_INTERVAL_MS);
    }
}

/**
 * 网络连接初始化异步包装函数
 */
static void network_connected_init_async(void *data)
{
    LV_UNUSED(data);
    network_connected_init();
}

/******************************************************************************************
**************************************获取天气值*******************************************
*******************************************************************************************/


/**
 * 根据天气文本显示相应的图标
 * @param weather 天气文本
 */
static void update_weather_icon(int city_cnt, const char *weather)
{
    // 隐藏所有天气图标

//    lv_obj_add_flag(guider_ui.screen_main_img_sun, LV_OBJ_FLAG_HIDDEN);
//    lv_obj_add_flag(guider_ui.screen_main_img_rain, LV_OBJ_FLAG_HIDDEN);
//    lv_obj_add_flag(guider_ui.screen_main_img_snow, LV_OBJ_FLAG_HIDDEN);
//    lv_obj_add_flag(guider_ui.screen_main_img_wind, LV_OBJ_FLAG_HIDDEN);
//    lv_obj_add_flag(guider_ui.screen_main_img_windy, LV_OBJ_FLAG_HIDDEN);
//    lv_obj_add_flag(guider_ui.screen_main_img_overcast, LV_OBJ_FLAG_HIDDEN);

//    // 设置所有图标到太阳图标的位置 (85, 206)
//    lv_obj_set_pos(guider_ui.screen_main_img_rain, WEATHER_ICON_X, WEATHER_ICON_Y);
//    lv_obj_set_pos(guider_ui.screen_main_img_snow, WEATHER_ICON_X, WEATHER_ICON_Y);
//    lv_obj_set_pos(guider_ui.screen_main_img_wind, WEATHER_ICON_X, WEATHER_ICON_Y);
//    lv_obj_set_pos(guider_ui.screen_main_img_windy, WEATHER_ICON_X, WEATHER_ICON_Y);
//    lv_obj_set_pos(guider_ui.screen_main_img_overcast, WEATHER_ICON_X, WEATHER_ICON_Y);

    // 根据天气关键字显示相应的图标
    if (weather == NULL) {
        // 如果天气文本为空，显示默认的windy图标
        lv_img_set_src(guider_ui.screen_main_img_sun, &_windy_alpha_25x20);
        return;
    } else {
        lv_obj_set_pos(guider_ui.screen_main_img_sun, WEATHER_ICON_X - city_cnt * 16, WEATHER_ICON_Y); //使用18汉字 - 2 = 16
        // 晴天/太阳图标
        if (strstr(weather, "晴") != NULL) {
            lv_img_set_src(guider_ui.screen_main_img_sun, &_sun_alpha_25x20);
        }
        // 雨天图标
        else if (strstr(weather, "雨") != NULL) {
            lv_img_set_src(guider_ui.screen_main_img_sun, &_rain_alpha_25x20);
        }
        // 雪天图标
        else if (strstr(weather, "雪") != NULL) {
            lv_img_set_src(guider_ui.screen_main_img_sun, &_snow_alpha_25x20);
        }
        // 风图标
        else if (strstr(weather, "风") != NULL) {
            lv_img_set_src(guider_ui.screen_main_img_sun, &_windy_alpha_25x20);
        }
        // 阴天图标
        else if (strstr(weather, "阴") != NULL || strstr(weather, "多云") != NULL) {
            lv_img_set_src(guider_ui.screen_main_img_sun, &_overcast_alpha_25x20);
        }
        // 默认显示windy图标
        else {
            lv_img_set_src(guider_ui.screen_main_img_sun, &_windy_alpha_25x20);
        }
    }
    lv_obj_clear_flag(guider_ui.screen_main_img_sun, LV_OBJ_FLAG_HIDDEN);
}

int update_weather_ui_from_json(const char *json_data)
{
    int ret = -1;
    if (!json_data) {
        //printf("[错误] 无效的JSON数据\n");
        return ret;
    }

    // 解析JSON数据
    cJSON *root = cJSON_Parse(json_data);
    if (!root) {
        //printf("[错误] JSON解析失败: %s\n", cJSON_GetErrorPtr());
        return ret;
    }

    // 获取结果数组
    cJSON *results = cJSON_GetObjectItem(root, "results");
    if (!results || !cJSON_IsArray(results) || cJSON_GetArraySize(results) == 0) {
        //printf("[错误] 未找到有效的结果数组\n");
        cJSON_Delete(root);
        return ret;
    }

    // 获取第一个结果
    cJSON *first_result = cJSON_GetArrayItem(results, 0);
    if (!first_result) {
        //printf("[错误] 未找到天气结果\n");
        cJSON_Delete(root);
        return ret;
    }

    // 获取每日天气数组
    cJSON *daily = cJSON_GetObjectItem(first_result, "daily");
    if (!daily || !cJSON_IsArray(daily) || cJSON_GetArraySize(daily) == 0) {
        //printf("[错误] 未找到每日天气数据\n");
        cJSON_Delete(root);
        return ret;
    }

    // 获取今天的天气数据（第一条数据）
    cJSON *today = cJSON_GetArrayItem(daily, 0);
    if (!today) {
        //printf("[错误] 未找到今天的天气数据\n");
        cJSON_Delete(root);
        return ret;
    }

    cJSON *location = cJSON_GetObjectItem(first_result, "location");

    if (!location || !today) {
        //printf("[错误] location 或 daily 信息缺失\n");
        cJSON_Delete(root);
        return ret;
    }

    const char *city = cJSON_GetStringValue(cJSON_GetObjectItem(location, "name"));
    const char *weather = cJSON_GetStringValue(cJSON_GetObjectItem(today, "text_day"));
    const char *temp_day = cJSON_GetStringValue(cJSON_GetObjectItem(today, "high"));
    const char *temp_night = cJSON_GetStringValue(cJSON_GetObjectItem(today, "low"));
    const char *humidity = cJSON_GetStringValue(cJSON_GetObjectItem(today, "humidity"));
    const char *wind = cJSON_GetStringValue(cJSON_GetObjectItem(today, "wind_direction"));
    const char *wind_speed = cJSON_GetStringValue(cJSON_GetObjectItem(today, "wind_scale"));

    lv_img_set_src(guider_ui.screen_main_img_location, &_location_set_alpha_15x18);
    lv_obj_clear_flag(guider_ui.screen_main_img_location, LV_OBJ_FLAG_HIDDEN);

    lv_span_set_text(guider_ui.screen_main_span_direction_span, city);
    lv_style_set_text_color(&guider_ui.screen_main_span_direction_span->style, lv_color_hex(0xffffff));

//    // 动态更新location图标位置，使其始终显示在城市文字左侧
//    lv_obj_add_flag(guider_ui.screen_main_img_location, LV_OBJ_FLAG_HIDDEN);
//    lv_obj_align_to(guider_ui.screen_main_img_location_set, guider_ui.screen_main_span_direction, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
//    lv_obj_clear_flag(guider_ui.screen_main_img_location_set, LV_OBJ_FLAG_HIDDEN);

    printf("[城市] city = %s\n", city);
    // 更新天气和温度
    if (weather && temp_day && temp_night) {
        char weather_str[64];
        char weather_temp_str[64];
        int empty_city_len = 4 - strlen(city) / 3;//UTF8一个汉字站3字节，国内最长市名称4个汉字
        empty_city_len = empty_city_len < 0 ? 0 : empty_city_len;

        snprintf(weather_str, sizeof(weather_str), "%s", weather);
        snprintf(weather_temp_str, sizeof(weather_temp_str), "%s~%s℃", temp_night, temp_day);
        lv_obj_set_pos(guider_ui.screen_main_span_weather, WEATHER_ICON_X - empty_city_len * 16 + 30, WEATHER_ICON_Y); //使用18汉字 - 2 = 16
        lv_span_set_text(guider_ui.screen_main_span_weather_span, weather_temp_str);

        // 添加天气图标更新
        printf("[tq] weather_str = %s\n", weather);
        update_weather_icon(empty_city_len, weather);
        //printf("[成功] 已更新温度数据\n");
        ret = 0;
    }

    // 释放JSON对象
    cJSON_Delete(root);
    return ret;
}

int fetch_weather_data(void)
{
    int ret = -1;
    // 1. 检查网络连接状态
    if (!lv_demo_system_net_connect_success()) {
        return ret;
    }

    char *buf = (char *)malloc(2048);
    if (buf == NULL) {
        printf("mem alloc err\n");
        return ret;
    }
    memset(buf, 0, 2048);

    // 2. 获取天气数据
    int result = qyai_weather_forecast_get(NULL, 0, buf, 2048);

    // 3. 处理结果
    if (result <= 0) {
        if (result == -1) {
            printf("[天气错误] 未绑定账号\n");
        } else {
            printf("[天气错误] 获取天气数据失败。\n");
        }
    } else {
        printf("\n%s\n", buf);
        // 4. 解析JSON
        ret = update_weather_ui_from_json(buf);
        if (ret) {
            printf("[天气错误] 获取天气数据异常\n");
        }
    }
    //printf("=========== 天气数据获取结束 ===========\n\n");
    free(buf);
    return ret;
}

/**
 * 闹钟状态检测函数
 * 检查当前是否有设置的闹钟，并相应地更新图标显示
 */
void alarm_status_check(void)
{
    // 检查UI对象是否有效
    if (!guider_ui.screen_main_img_no_alarm) {
        return;
    }
    if (!guider_ui.screen_main_img_set_alarm) {
        return;
    }
    // 检查UI对象是否有效
    if (!guider_ui.screen_dropdown_img_no_alarm) {
        return;
    }
    if (!guider_ui.screen_dropdown_img_set_alarm) {
        return;
    }
    lv_obj_set_pos(guider_ui.screen_main_img_set_alarm, ALARM_ICON_X, ALARM_ICON_Y);
    lv_obj_set_pos(guider_ui.screen_main_img_no_alarm, ALARM_ICON_X, ALARM_ICON_Y);
    lv_obj_set_pos(guider_ui.screen_dropdown_img_set_alarm, ALARM_ICON_X, ALARM_ICON_Y);
    lv_obj_set_pos(guider_ui.screen_dropdown_img_no_alarm, ALARM_ICON_X, ALARM_ICON_Y);
    // 调用get_alarm_count函数获取当前闹钟数量
    int alarm_count = get_alarm_count();
    // 根据是否有闹钟来显示或隐藏无闹钟图标
    if (alarm_count > 0) {
        // 有闹钟
        lv_obj_add_flag(guider_ui.screen_main_img_no_alarm, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(guider_ui.screen_main_img_set_alarm, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(guider_ui.screen_dropdown_img_no_alarm, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(guider_ui.screen_dropdown_img_set_alarm, LV_OBJ_FLAG_HIDDEN);
        //printf("[闹钟图标] 有闹钟\n");
    } else {
        // 无闹钟
        lv_obj_add_flag(guider_ui.screen_main_img_set_alarm, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(guider_ui.screen_main_img_no_alarm, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(guider_ui.screen_dropdown_img_set_alarm, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(guider_ui.screen_dropdown_img_no_alarm, LV_OBJ_FLAG_HIDDEN);
        //printf("[闹钟图标] 没有闹钟\n");
    }
}

/**
 * @brief 检查网络连接状态的外部调用函数
 * @return 1表示网络连接成功，0表示网络连接失败
 */
int ai_speaker_check_network_status(void)
{
    extern int lv_demo_system_net_interface(void);
    int net_interface = lv_demo_system_net_interface();
    int net_status = lv_demo_system_net_connect_success();
    //printf("[网络状态检查] 当前网络连接状态：%s（状态码：%d）\n", net_status ? "已连接" : "未连接", net_status);

    // 添加WiFi图标显示逻辑
    if (guider_ui.screen_main_img_no_wifi) {
        if (net_interface) {//4G
            if (net_status) {
                lv_img_set_src(guider_ui.screen_main_img_no_wifi, &_set_4g_alpha_18x16);
            } else {
                lv_img_set_src(guider_ui.screen_main_img_no_wifi, &_no_4g_alpha_18x16);
            }
        } else {//wifi
            if (net_status) {
                lv_img_set_src(guider_ui.screen_main_img_no_wifi, &_set_wifi_alpha_18x16);
            } else {
                lv_img_set_src(guider_ui.screen_main_img_no_wifi, &_no_wifi_alpha_18x16);
            }
        }
        lv_obj_clear_flag(guider_ui.screen_main_img_no_wifi, LV_OBJ_FLAG_HIDDEN);
//        lv_obj_set_pos(guider_ui.screen_main_img_set_wifi, WIFI_ICON_X, WIFI_ICON_Y);
//        lv_obj_set_pos(guider_ui.screen_main_img_no_wifi, WIFI_ICON_X, WIFI_ICON_Y);
//        if (net_status) {
//            // 网络已连接
//            lv_obj_add_flag(guider_ui.screen_main_img_no_wifi, LV_OBJ_FLAG_HIDDEN);
//            lv_obj_clear_flag(guider_ui.screen_main_img_set_wifi, LV_OBJ_FLAG_HIDDEN);
//            //printf("[WiFi图标] 网络已连接\n");
//        } else {
//            // 网络未连接
//            lv_obj_add_flag(guider_ui.screen_main_img_set_wifi, LV_OBJ_FLAG_HIDDEN);
//            lv_obj_clear_flag(guider_ui.screen_main_img_no_wifi, LV_OBJ_FLAG_HIDDEN);
//            //printf("[WiFi图标] 网络未连接\n");
//        }
    }
    // 添加WiFi图标显示逻辑
    if (guider_ui.screen_dropdown_img_no_wifi) {
        if (net_interface) {//4G
            if (net_status) {
                lv_img_set_src(guider_ui.screen_dropdown_img_no_wifi, &_set_4g_alpha_18x16);
            } else {
                lv_img_set_src(guider_ui.screen_dropdown_img_no_wifi, &_no_4g_alpha_18x16);
            }
        } else {//wifi
            if (net_status) {
                lv_img_set_src(guider_ui.screen_dropdown_img_no_wifi, &_set_wifi_alpha_18x16);
            } else {
                lv_img_set_src(guider_ui.screen_dropdown_img_no_wifi, &_no_wifi_alpha_18x16);
            }
        }
        lv_obj_clear_flag(guider_ui.screen_dropdown_img_no_wifi, LV_OBJ_FLAG_HIDDEN);
    }
    return net_status;
}

/**
 * @brief 检查蓝牙连接状态的外部调用函数
 * @return 1表示蓝牙连接成功，0表示蓝牙连接失败
 */
int BT_check_bluetooth_status(void)
{
    // 使用实际的蓝牙连接检查函数
    extern char lv_demo_system_bt_connect(void);
    int bt_status = lv_demo_system_bt_connect();
    //printf("[蓝牙状态检查] 当前蓝牙连接状态：%s（状态码：%d）\n", bt_status ? "已连接" : "未连接", bt_status);
    if (bt_status) {
        // 蓝牙已连接
        lv_img_set_src(guider_ui.screen_dropdown_img_no_BT, &_BT_alpha_14x16);
        lv_obj_clear_flag(guider_ui.screen_dropdown_img_no_BT, LV_OBJ_FLAG_HIDDEN);
    } else {
        // 蓝牙未连接
        lv_img_set_src(guider_ui.screen_dropdown_img_no_BT, &_no_BT_alpha_14x16);
        lv_obj_clear_flag(guider_ui.screen_dropdown_img_no_BT, LV_OBJ_FLAG_HIDDEN);
    }
    return bt_status;
}

// 更新电池电量显示 - 作为唯一控制电池图标显示的函数
static void update_battery_display(int percent, int charging, int chargfull)
{
    // 先隐藏所有图标
    if (guider_ui.screen_main_img_fully_battery) {
        lv_obj_add_flag(guider_ui.screen_main_img_fully_battery, LV_OBJ_FLAG_HIDDEN);
    }
    if (guider_ui.screen_main_img_charging_battery) {
        lv_obj_add_flag(guider_ui.screen_main_img_charging_battery, LV_OBJ_FLAG_HIDDEN);
    }
    if (guider_ui.screen_main_img_low_battery) {
        lv_obj_add_flag(guider_ui.screen_main_img_low_battery, LV_OBJ_FLAG_HIDDEN);
    }
    if (guider_ui.screen_main_img_normol_battery) {
        lv_obj_add_flag(guider_ui.screen_main_img_normol_battery, LV_OBJ_FLAG_HIDDEN);
    }
    // 先隐藏所有图标
    if (guider_ui.screen_dropdown_img_fully_battery) {
        lv_obj_add_flag(guider_ui.screen_dropdown_img_fully_battery, LV_OBJ_FLAG_HIDDEN);
    }
    if (guider_ui.screen_dropdown_img_charging_battery) {
        lv_obj_add_flag(guider_ui.screen_dropdown_img_charging_battery, LV_OBJ_FLAG_HIDDEN);
    }
    if (guider_ui.screen_dropdown_img_low_battery) {
        lv_obj_add_flag(guider_ui.screen_dropdown_img_low_battery, LV_OBJ_FLAG_HIDDEN);
    }
    if (guider_ui.screen_dropdown_img_normal_battery) {
        lv_obj_add_flag(guider_ui.screen_dropdown_img_normal_battery, LV_OBJ_FLAG_HIDDEN);
    }


    // 统一设置图标位置
    if (guider_ui.screen_main_img_fully_battery) {
        lv_obj_set_pos(guider_ui.screen_main_img_fully_battery, BATTERY_ICON_X, BATTERY_ICON_Y);
    }
    if (guider_ui.screen_main_img_charging_battery) {
        lv_obj_set_pos(guider_ui.screen_main_img_charging_battery, BATTERY_ICON_X, BATTERY_ICON_Y);
    }
    if (guider_ui.screen_main_img_low_battery) {
        lv_obj_set_pos(guider_ui.screen_main_img_low_battery, BATTERY_ICON_X, BATTERY_ICON_Y);
    }
    if (guider_ui.screen_main_img_normol_battery) {
        lv_obj_set_pos(guider_ui.screen_main_img_normol_battery, BATTERY_ICON_X, BATTERY_ICON_Y);
    }
    // 统一设置图标位置
    if (guider_ui.screen_dropdown_img_fully_battery) {
        lv_obj_set_pos(guider_ui.screen_dropdown_img_fully_battery, BATTERY_ICON_X, BATTERY_ICON_Y);
    }
    if (guider_ui.screen_dropdown_img_charging_battery) {
        lv_obj_set_pos(guider_ui.screen_dropdown_img_charging_battery, BATTERY_ICON_X, BATTERY_ICON_Y);
    }
    if (guider_ui.screen_dropdown_img_low_battery) {
        lv_obj_set_pos(guider_ui.screen_dropdown_img_low_battery, BATTERY_ICON_X, BATTERY_ICON_Y);
    }
    if (guider_ui.screen_dropdown_img_normal_battery) {
        lv_obj_set_pos(guider_ui.screen_dropdown_img_normal_battery, BATTERY_ICON_X, BATTERY_ICON_Y);
    }


    // 根据充电状态和电量百分比显示正确的图标
    if (charging) {  // 正在充电
        if (chargfull) {  // 已充满电
            if (guider_ui.screen_main_img_fully_battery) {
                lv_obj_clear_flag(guider_ui.screen_main_img_fully_battery, LV_OBJ_FLAG_HIDDEN);
                //printf("电池电量显示: 充电且满电图标\n");
            }
            if (guider_ui.screen_dropdown_img_fully_battery) {
                lv_obj_clear_flag(guider_ui.screen_dropdown_img_fully_battery, LV_OBJ_FLAG_HIDDEN);
                //printf("电池电量显示: 充电且满电图标\n");
            }
        } else {  // 充电中但未充满
            if (guider_ui.screen_main_img_charging_battery) {
                lv_obj_clear_flag(guider_ui.screen_main_img_charging_battery, LV_OBJ_FLAG_HIDDEN);
                //printf("电池电量显示: 充电图标\n");
            }
            if (guider_ui.screen_dropdown_img_charging_battery) {
                lv_obj_clear_flag(guider_ui.screen_dropdown_img_charging_battery, LV_OBJ_FLAG_HIDDEN);
                //printf("电池电量显示: 充电图标\n");
            }
        }
    } else {  // 未充电
        if (percent <= 22) {  // 低电量
            if (guider_ui.screen_main_img_low_battery) {
                lv_obj_clear_flag(guider_ui.screen_main_img_low_battery, LV_OBJ_FLAG_HIDDEN);
                //printf("电池电量显示: 低电量图标 (%d%%)\n", percent);
            }
            if (guider_ui.screen_dropdown_img_low_battery) {
                lv_obj_clear_flag(guider_ui.screen_dropdown_img_low_battery, LV_OBJ_FLAG_HIDDEN);
                printf("电池电量显示: 低电量图标 (%d%%)\n", percent);
            }
        } else {  // 正常电量
            if (guider_ui.screen_main_img_normol_battery) {
                lv_obj_clear_flag(guider_ui.screen_main_img_normol_battery, LV_OBJ_FLAG_HIDDEN);
                //printf("电池电量显示: 正常图标 (%d%%)\n", percent);
            }
            if (guider_ui.screen_dropdown_img_normal_battery) {
                lv_obj_clear_flag(guider_ui.screen_dropdown_img_normal_battery, LV_OBJ_FLAG_HIDDEN);
                //printf("电池电量显示: 正常图标 (%d%%)\n", percent);
            }
        }
    }
}

// 电池电量定时器回调函数 - 每分钟更新一次
static void battery_update_timer_cb(void *p)
{
    int percent = ((int)p >> 2);
    int charging = ((int)p >> 1) & 0x1;
    int chargfull = ((int)p & 0x1);
    update_battery_display(percent, charging, chargfull);
}

// 电池电量定时器回调函数 - 每分钟更新一次
void battery_update_sync(void *p)
{
    lv_async_call(battery_update_timer_cb, p);
}

#endif



// 音乐按钮点击事件处理函数
static void btn_music_click_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if (!ui || !ui->screen_main_btn_music) {
        return;
    }
    if (code == LV_EVENT_CLICKED) {
        printf("音乐按钮被点击，跳转到音乐页面\n");
#ifdef APPLAYER_ENABLE
        if (audio_app_mode_check() == -1) {
            return;
        } else if (audio_app_mode_check() == 0) {
            switch_to_music_page("net_music");
        } else if (audio_app_mode_check() == 1) {
            switch_to_music_page("bt_music");
        } else if (audio_app_mode_check() == 2) {
            switch_to_music_page("sd_music");
        } else if (audio_app_mode_check() == 3) {
            switch_to_music_page("usbdisk_music");
        } else if (audio_app_mode_check() == 4) {
            switch_to_music_page("aux_music");
        }
#else
        switch_to_music_page("net_music");
#endif
    }
}

// 对话按钮点击事件处理函数
static void btn_conversation_click_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if (!ui || !ui->screen_main_btn_conversation) {
        return;
    }
    if (code == LV_EVENT_CLICKED) {
        printf("对话按钮被点击，跳转到对话页面\n");
        switch_to_conversation_page();
    }
}

// 闹钟按钮点击事件处理函数
static void btn_alarm_click_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if (!ui || !ui->screen_main_btn_alarm) {
        return;
    }
    if (code == LV_EVENT_CLICKED) {
        printf("闹钟按钮被点击，跳转到闹钟页面\n");
        switch_to_alarm_page();
    }
}

// 倒计时按钮点击事件处理函数
static void btn_timer_click_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if (!ui || !ui->screen_main_btn_timer) {
        return;
    }
    if (code == LV_EVENT_CLICKED) {
        printf("倒计时按钮被点击，跳转到倒计时页面\n");
        switch_to_timer_page();

    }
}

// 初始化主页面事件
void screen_main_event_init(lv_ui *ui)
{
    if (!ui) {
        return;
    }

    lv_obj_set_scroll_dir(guider_ui.screen_main, LV_DIR_NONE);
    main_scrollicon();

}
void screen_main_event_setup(void)
{
    screen_main_event_init(&guider_ui);
}
