#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"

#ifndef OPTION_ICON_ZOOM_GRADIENT
#define OPTION_ICON_ZOOM_GRADIENT  0
#endif

/* 图标枚举与 CONFIG 条件编译：与主工程底部栏顺序一致（抽签 → 摇骰子 → 打地鼠 → 播客） */
typedef enum {
    ICON_ID_COUNTDOWN = 0,
    ICON_ID_ALARM,
    ICON_ID_MUSIC,
    ICON_ID_AI_CONVERSATION,
#if CONFIG_LVGL_MOUSE_GAME_ENABLE
    ICON_ID_WHACK,
#endif
#if CONFIG_LVGL_TIMER_SLEEP_ENABLE
    ICON_ID_SLEEP,
#endif
#if CONFIG_LVGL_CONTENT_LIST_ENABLE
    ICON_ID_CONTENT,
#endif
    ICON_ID_MAX
} IconID_t;

#define OPTION_ICON_COUNT      ICON_ID_MAX


#define OPTION_ICON_MOVE           ((OPTION_ICON_DISTANCE * 3) / 5)
#define OPTION_ICON_ZOOM_BIG       256
#define OPTION_MAX_ICON_THRESHOLD  256
#define OPTION_SWIPE_THRESHOLD     40

// 图标之间的距离（中心间距，越大左右排得越开）
#define OPTION_ICON_DISTANCE       150
#define OPTION_ICON_ZOOM_SMALL     160


#if CONFIG_LVGL_MOUSE_GAME_ENABLE
#define ICON_OPT_WHACK       ICON_DEF(ICON_ID_WHACK, _new_whack_alpha_120x120, "打地鼠")
#else
#define ICON_OPT_WHACK
#endif
#if CONFIG_LVGL_TIMER_SLEEP_ENABLE
#define ICON_OPT_SLEEP       ICON_DEF(ICON_ID_SLEEP, _new_sleep_alpha_120x120, "定时休眠")
#else
#define ICON_OPT_SLEEP
#endif
#if CONFIG_LVGL_CONTENT_LIST_ENABLE
#define ICON_OPT_CONTENT     ICON_DEF(ICON_ID_CONTENT, _new_content_alpha_120x120, "内容")
#else
#define ICON_OPT_CONTENT
#endif


#define ICON_DEFINITIONS \
    ICON_DEF(ICON_ID_COUNTDOWN, _new_timer_alpha_120x120, "倒计时") \
    ICON_DEF(ICON_ID_ALARM, _new_alarm_alpha_120x120, "闹钟") \
    ICON_DEF(ICON_ID_MUSIC, _new_music_alpha_120x120, "音乐") \
    ICON_DEF(ICON_ID_AI_CONVERSATION, _new_conversation_alpha_120x120, "AI对话") \
    ICON_OPT_WHACK \
    ICON_OPT_SLEEP \
    ICON_OPT_CONTENT
typedef struct {
    lv_obj_t *obj;
    int32_t x;
    char name_lab[20];
    bool is_max;
    int32_t icon_idx;
} option_icon_typedef;

static option_icon_typedef icon[OPTION_ICON_COUNT];
static bool touched = false;
static int32_t scr_w;
static lv_obj_t *icon_name_lab;
static int32_t t_offset_x;
static lv_obj_t *screen;
static int32_t current_max_icon_idx = 0;
static bool s_scrollicon_done;

// 按下事件回调函数声明
static void  pressing_cb(lv_event_t * e);
// 释放事件回调函数声明
static void  released_cb(lv_event_t * e);
// 设置x坐标回调函数声明
static void set_x_cb(void * var, int32_t v);
// 自定义动画创建函数声明
static void lv_myanim_creat(void * var, lv_anim_exec_xcb_t exec_cb, uint32_t time, uint32_t delay, lv_anim_path_cb_t path_cb,
                            int32_t start, int32_t end, lv_anim_ready_cb_t completed_cb, lv_anim_deleted_cb_t deleted_cb);
static void update_max_icon_status(void);
static void open_icon_screen(int32_t icon_idx);

void lv_screen_open_icon_screen_set(short x, short y);

void scrollicon(void)
{
    int32_t i;
    lv_coord_t opt_w;
    lv_coord_t opt_h;

    if (s_scrollicon_done || !guider_ui.screen_option) {
        return;
    }

    scr_w = lv_disp_get_hor_res(lv_disp_get_default());
    lv_memset(icon, 0, sizeof(icon));

    /* 必须挂在 screen_option 上；setup 阶段 lv_scr_act() 往往还是上一页，用它会建错父对象导致看不到图标 */
    screen = lv_obj_create(guider_ui.screen_option);
    lv_obj_remove_style_all(screen);
    lv_obj_set_pos(screen, 0, 20);
    opt_w = lv_obj_get_width(guider_ui.screen_option);
    opt_h = lv_obj_get_height(guider_ui.screen_option);
    if (opt_w <= 0) {
        opt_w = scr_w;
    }
    if (opt_h <= 30) {
        opt_h = lv_disp_get_ver_res(lv_disp_get_default());
    }
    lv_obj_set_size(screen, opt_w, opt_h - 30);

    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(screen, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(screen, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(screen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(screen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_move_foreground(screen);

    lv_obj_add_event_cb(screen, pressing_cb, LV_EVENT_PRESSING, 0);
    lv_obj_add_event_cb(screen, released_cb, LV_EVENT_RELEASED, 0);

    icon_name_lab = guider_ui.screen_option_op_name;
    if (icon_name_lab) {
        lv_obj_set_style_text_font(icon_name_lab, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    for (i = 0; i < OPTION_ICON_COUNT; i++) {
        // 创建图标对象并添加到屏幕上
        icon[i].obj = lv_img_create(screen);
        // 设置图标对象的用户数据为对应的图标结构体指针
        icon[i].obj->user_data = &icon[i];
        // 初始化是否为最大图标标志
        icon[i].is_max = false;
        // 设置图标索引
        icon[i].icon_idx = i;


        // 使用宏定义设置图标源和名称
#define ICON_DEF(idx, src, name) \
            if (i == idx) { \
                lv_img_set_src(icon[idx].obj, &src); \
                strcpy(icon[idx].name_lab, name); \
            }
        ICON_DEFINITIONS
#undef ICON_DEF
        // 计算图标x坐标
        icon[i].x = (i - OPTION_ICON_COUNT / 2) * OPTION_ICON_DISTANCE;
        lv_obj_center(icon[i].obj);
        lv_img_set_pivot(icon[i].obj, 60, 60);
        if (i == OPTION_ICON_COUNT / 2) {
            lv_img_set_zoom(icon[i].obj, OPTION_ICON_ZOOM_BIG);
            if (icon_name_lab) {
                lv_label_set_text(icon_name_lab, icon[i].name_lab);
            }
            icon[i].is_max = true;
            current_max_icon_idx = i;
        } else {
            // 否则设置为小尺寸
            lv_img_set_zoom(icon[i].obj, OPTION_ICON_ZOOM_SMALL);

            // 修改后
            // 移除小图标的可点击标志和事件回调
            lv_obj_clear_flag(icon[i].obj, LV_OBJ_FLAG_CLICKABLE);
        }
        lv_obj_set_x(icon[i].obj, icon[i].x);
    }
    s_scrollicon_done = true;
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

    for (i = 0; i < OPTION_ICON_COUNT; i++) {
        // 更新图标x坐标
        icon[i].x += t_offset_x;
        // 处理图标x坐标超出范围的情况（循环滚动）
        while (icon[i].x < (-OPTION_ICON_COUNT / 2) * OPTION_ICON_DISTANCE) {
            icon[i].x += (OPTION_ICON_COUNT) * OPTION_ICON_DISTANCE;
        }
        while (icon[i].x >  (OPTION_ICON_COUNT / 2) * OPTION_ICON_DISTANCE) {
            icon[i].x -= (OPTION_ICON_COUNT) * OPTION_ICON_DISTANCE;
        }
        // 设置图标对象的x坐标
        lv_obj_set_x(icon[i].obj, icon[i].x);

        // 如果图标x坐标超出一定范围，设置为小尺寸
        if (icon[i].x >= OPTION_ICON_DISTANCE || icon[i].x <= -OPTION_ICON_DISTANCE) {
            if (icon[i].is_max) {
                lv_img_set_zoom(icon[i].obj, OPTION_ICON_ZOOM_SMALL);
                icon[i].is_max = false;
            }
            continue;
        }

        // 根据x坐标计算图标尺寸
        v = (icon[i].x >= 0) ? icon[i].x : -icon[i].x;

        // 更新图标名称标签
        if (icon_name_lab && v <= OPTION_ICON_MOVE) {
            lv_label_set_text(icon_name_lab, icon[i].name_lab);
        }

        // 计算缩放值（用整数运算代替浮点运算，减少开销）
        int32_t zoom = OPTION_ICON_ZOOM_SMALL + (OPTION_ICON_DISTANCE - v) * (OPTION_ICON_ZOOM_BIG - OPTION_ICON_ZOOM_SMALL) / OPTION_ICON_DISTANCE;
        lv_img_set_zoom(icon[i].obj, zoom);

        // 更新最大图标状态
        icon[i].is_max = (zoom >= OPTION_MAX_ICON_THRESHOLD);
    }

    // 更新当前最大图标索引
    update_max_icon_status();
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

void lv_open_icon_screen_set(short x, short y)
{
    lv_coord_t icon_x = lv_obj_get_x(icon[current_max_icon_idx].obj);
    lv_coord_t icon_y = lv_obj_get_y(icon[current_max_icon_idx].obj);
    lv_coord_t icon_w = lv_obj_get_width(icon[current_max_icon_idx].obj);
    lv_coord_t icon_h = lv_obj_get_height(icon[current_max_icon_idx].obj);
    printf("--> x %d , y %d\n", x, y);
    printf("--> screen_set :%d-%d , %d-%d \n", icon_x, icon_x + icon_w, icon_y, icon_y + icon_h);
    if (x >= icon_x && x <= (icon_x + icon_w) && y >= icon_y && y <= (icon_y + icon_h)) {
        open_icon_screen(current_max_icon_idx);
    }
}

// 释放事件回调函数
static void  released_cb(lv_event_t * e)
{
    lv_point_t release_pos;
    // 获取释放时的坐标
    lv_indev_get_point(lv_indev_get_act(), &release_pos);

    int32_t offset_x;
    offset_x = 0;
    // 设置触摸状态为未触摸
    touched = false;

    for (int32_t i = 0; i < OPTION_ICON_COUNT; i++) {
        // 如果图标x坐标大于0
        if (icon[i].x > 0) {
            // 根据x坐标与图标距离的关系计算偏移量
            if (icon[i].x % OPTION_ICON_DISTANCE > OPTION_ICON_DISTANCE / 2) {
                offset_x = OPTION_ICON_DISTANCE - icon[i].x % OPTION_ICON_DISTANCE;
            } else {
                offset_x = -icon[i].x % OPTION_ICON_DISTANCE;
            }
            break;
        }
    }

    for (int32_t i = 0; i < OPTION_ICON_COUNT; i++) {
        // 创建动画，使图标回到合适位置，固定100ms快速回弹
        lv_myanim_creat(icon[i].obj, set_x_cb, 100, 0, lv_anim_path_ease_out,
                        icon[i].x, icon[i].x + offset_x + t_offset_x / 20 * OPTION_ICON_DISTANCE, 0, 0);
        // 更新图标x坐标
        icon[i].x += offset_x + t_offset_x / 20 * OPTION_ICON_DISTANCE;
        // 处理图标x坐标超出范围的情况（循环滚动）
        while (icon[i].x < (-OPTION_ICON_COUNT / 2) * OPTION_ICON_DISTANCE) {
            icon[i].x += (OPTION_ICON_COUNT) * OPTION_ICON_DISTANCE;
        }
        while (icon[i].x >  (OPTION_ICON_COUNT / 2) * OPTION_ICON_DISTANCE) {
            icon[i].x -= (OPTION_ICON_COUNT) * OPTION_ICON_DISTANCE;
        }
    }

    // 更新当前最大图标索引
    update_max_icon_status();
    if (current_max_icon_idx != -1) {
        lv_open_icon_screen_set(release_pos.x, release_pos.y);
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
// 设置x坐标回调函数
static void set_x_cb(void * var, int32_t v)
{
    // 处理x坐标超出范围的情况（循环滚动）
    while (v < (-OPTION_ICON_COUNT / 2) * OPTION_ICON_DISTANCE) {
        v += (OPTION_ICON_COUNT) * OPTION_ICON_DISTANCE;
    }
    while (v > (OPTION_ICON_COUNT / 2) * OPTION_ICON_DISTANCE) {
        v -= (OPTION_ICON_COUNT) * OPTION_ICON_DISTANCE;
    }

    // 设置对象的x坐标
    lv_obj_set_x(var, v);

    // 获取图标结构体指针
    option_icon_typedef * xxx = (option_icon_typedef *)(((lv_obj_t *)var)->user_data);
    // 更新图标结构体中的x坐标
    xxx->x = v;

    int32_t v_abs = v >= 0 ? v : -v;
    int32_t zoom;
    bool was_max = xxx->is_max; // 保存之前的is_max状态

    // 如果x坐标为0，设置图标为大尺寸
    if (v == 0) {
        zoom = OPTION_ICON_ZOOM_BIG;
        xxx->is_max = true;
    }
    // 如果x坐标超出一定范围，设置图标为小尺寸
    else if (v >= OPTION_ICON_DISTANCE || v <= -OPTION_ICON_DISTANCE) {
        zoom = OPTION_ICON_ZOOM_SMALL;
        xxx->is_max = false;
        lv_obj_set_style_shadow_width(var, 0, LV_PART_MAIN);
    }
    // 其他情况，根据x坐标计算图标尺寸
    else {
        zoom = OPTION_ICON_ZOOM_SMALL + (float)(OPTION_ICON_DISTANCE - v_abs) / OPTION_ICON_DISTANCE * (OPTION_ICON_ZOOM_BIG - OPTION_ICON_ZOOM_SMALL);
        xxx->is_max = (zoom >= OPTION_MAX_ICON_THRESHOLD);
    }

    lv_img_set_zoom(var, zoom);

    if (icon_name_lab && v >= -OPTION_ICON_MOVE && v <= OPTION_ICON_MOVE) {
        lv_label_set_text(icon_name_lab, xxx->name_lab);
    }
    // 更新当前最大图标索引
    update_max_icon_status();
}

// 新增：更新最大图标状态
static void update_max_icon_status(void)
{
    for (int32_t i = 0; i < OPTION_ICON_COUNT; i++) {
        if (icon[i].is_max) {
            current_max_icon_idx = i;
            // 可以在这里添加额外的逻辑，比如打印日志输出当前最大图标
            // printf("当前最大图标: %s (索引: %d)\n", icon[i].name_lab, i);
            return;
        }
    }
}

// 新增：打开对应屏幕函数
static void open_icon_screen(int32_t icon_idx)
{
    switch (icon_idx) {
    case ICON_ID_COUNTDOWN:
        switch_to_timer_page();
        break;
    case ICON_ID_ALARM:
        switch_to_alarm_page();
        break;
    case ICON_ID_MUSIC:
        switch_to_music_page("net_music");
        break;
    case ICON_ID_AI_CONVERSATION:
        switch_to_conversation_page();
        lv_demo_ai_dialogue_speak_listen(1);
        aisp_wake(0);
        break;
#if CONFIG_LVGL_MOUSE_GAME_ENABLE
    case ICON_ID_WHACK:
        switch_to_whack_game_page();
        break;
#endif
#if CONFIG_LVGL_TIMER_SLEEP_ENABLE
    case ICON_ID_SLEEP:
        switch_to_sleep_page();
        break;
#endif
#if CONFIG_LVGL_CONTENT_LIST_ENABLE
    case ICON_ID_CONTENT:
        switch_to_list_page();
        break;
#endif
    default:
        break;
    }
}

static void option_return_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);

    if (!ui || !ui->screen_option_img_return) {
        return;
    }
    if (code == LV_EVENT_CLICKED) {
        switch_to_main_page();
    }
}

void screen_option_event_init(lv_ui *ui)
{
    if (!ui || !guider_ui.screen_option) {
        return;
    }

    lv_obj_set_scroll_dir(guider_ui.screen_option, LV_DIR_NONE);
    if (guider_ui.screen_option_img_return) {
//        ui_return_btn_enlarge_touch(guider_ui.screen_option_img_return);
        lv_obj_add_event_cb(guider_ui.screen_option_img_return, option_return_event_handler, LV_EVENT_CLICKED, ui);
    }
    scrollicon();
    if (guider_ui.screen_option_img_return) {
        lv_obj_move_foreground(guider_ui.screen_option_img_return);
    }
    if (guider_ui.screen_option_op_name) {
        lv_obj_move_foreground(guider_ui.screen_option_op_name);
    }
}

void screen_option_event_setup(void)
{
    screen_option_event_init(&guider_ui);
}
