#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdbool.h"
#include "gui_guider.h"
#include "custom.h"
#include "lvgl_develop.h"

/* 与 jieli-chatbot custom/screen_option.c 一致：枚举 + CONFIG 条件片段拼出 ICON_DEFINITIONS，索引与 open_icon_screen 共用；游戏见 lvgl_develop.h CONFIG_LVGL_GAMES_ENABLE 及子项 */
typedef enum {
    ICON_ID_COUNTDOWN = 0,
    ICON_ID_ALARM,
    ICON_ID_MUSIC,
    ICON_ID_AI_CONVERSATION,
#if CONFIG_LVGL_OPTION_WIFI_ENABLE
    ICON_ID_WIFI,
#endif
#if CONFIG_LVGL_SLEEP_MUSIC_ENABLE
    ICON_ID_SLEEP_MUSIC,
#endif
#if CONFIG_LVGL_AGENT_ENABLE
    ICON_ID_AGENT,
#endif
#if CONFIG_LVGL_CALCULATOR_ENABLE
    ICON_ID_CALCULATOR,
#endif
#if defined(CONFIG_LVGL_GAMES_ENABLE) && CONFIG_LVGL_GAMES_ENABLE
#if defined(CONFIG_LVGL_DRAW_LOTS_GAME_ENABLE) && CONFIG_LVGL_DRAW_LOTS_GAME_ENABLE
    ICON_ID_DRAW_LOTS,
#endif
#if defined(CONFIG_LVGL_DICE_GAME_ENABLE) && CONFIG_LVGL_DICE_GAME_ENABLE
    ICON_ID_DICE,
#endif
#if defined(CONFIG_LVGL_MOUSE_GAME_ENABLE) && CONFIG_LVGL_MOUSE_GAME_ENABLE
    ICON_ID_WHACK,
#endif
#endif
#if CONFIG_LVGL_CONTENT_LIST_ENABLE
    ICON_ID_CONTENT,
#endif
    ICON_ID_MAX
} IconID_t;

#define OPTION_ICON_COUNT      ICON_ID_MAX
// 图标之间的距离
#define OPTION_ICON_DISTANCE       78
/* 行首与左缘间距，与 scrollicon 中 10+i*OPTION_ICON_DISTANCE 一致；松手吸附必须相对该值取模，勿用 0%78 否则会整排误吸附到下一格 */
#define OPTION_ICON_LEFT_MARGIN    10
/* 左右键整排平移 */
#define OPTION_ICON_SCROLL_ANIM_MS       2
/* 松手后吸附网格回弹：与上项独立，便于只调回弹、不改键控平移；1=动画对齐网格（默认开，渐停、不生硬） */
#ifndef OPTION_ICON_RELEASE_SNAP_ENABLE
#define OPTION_ICON_RELEASE_SNAP_ENABLE  1
#endif
#ifndef OPTION_ICON_RELEASE_SNAP_ANIM_MS
#define OPTION_ICON_RELEASE_SNAP_ANIM_MS 500
#endif
/* ease_out 末段更柔和地「落」到格点；要对称加减速可改 lv_anim_path_ease_in_out */
#define OPTION_ICON_RELEASE_SNAP_PATH    lv_anim_path_ease_out
/* 松手惯性：在末帧滑动方向再滑行一段，再进网格吸附（可关） */
#ifndef OPTION_MAIN_INERTIA_ENABLE
#define OPTION_MAIN_INERTIA_ENABLE  1
#endif
#ifndef OPTION_MAIN_INERTIA_COEFF
#define OPTION_MAIN_INERTIA_COEFF   3   /* 末帧位移 × 本系数 = 初算惯性，再与 lv_indev vect 混合 */
#endif
#ifndef OPTION_MAIN_INERTIA_VECT_W
#define OPTION_MAIN_INERTIA_VECT_W  2   /* lv_indev_get_vect 水平分量权（/2 为与末帧等权混合） */
#endif
#ifndef OPTION_MAIN_INERTIA_MAX_PX
#define OPTION_MAIN_INERTIA_MAX_PX  (OPTION_ICON_DISTANCE * 8)
#endif
#ifndef OPTION_MAIN_INERTIA_MIN_DX
#define OPTION_MAIN_INERTIA_MIN_DX  1   /* 末帧|dx|低于此、且合速度过低时不加惯性，避免点按抖动 */
#endif
/* 松手后惯性不一次性走完：用 ease_out 动画 0→inert，再进网格；0=恢复瞬时惯性 */
#ifndef OPTION_MAIN_INERTIA_COAST_ENABLE
#define OPTION_MAIN_INERTIA_COAST_ENABLE 1
#endif
#ifndef OPTION_MAIN_INERTIA_COAST_TIME_MIN_MS
#define OPTION_MAIN_INERTIA_COAST_TIME_MIN_MS  150
#endif
#ifndef OPTION_MAIN_INERTIA_COAST_TIME_PER_PX
#define OPTION_MAIN_INERTIA_COAST_TIME_PER_PX 1
#endif
#ifndef OPTION_MAIN_INERTIA_COAST_TIME_MAX_MS
#define OPTION_MAIN_INERTIA_COAST_TIME_MAX_MS  700
#endif
/* 余摆段插值：linear=匀速多滑一截、末段不减速；ease_out 为渐停 */
#ifndef OPTION_MAIN_INERTIA_COAST_PATH
#define OPTION_MAIN_INERTIA_COAST_PATH  lv_anim_path_ease_out
#endif

// 定义图标结构体，包含图标对象指针和x坐标
typedef struct {
    lv_obj_t *obj;
    lv_obj_t *border_obj;  /* 66x66 正方形边框 overlay */
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
/* 上一帧 PRESSING 的水平位移，用于松手时估算惯性 */
static int32_t s_last_drag_dx;
// 屏幕对象指针
static lv_obj_t *screen;
// 新增：当前最大的图标索引
static int32_t current_max_icon_idx = 0;
// 当前被点击选中的图标索引（-1 表示无选中，不显示白框）
static int32_t selected_icon_idx = -1;
// 异步更新白框时待应用的选中索引（-1 表示全部隐藏）
static int32_t s_pending_border_idx = 0;
// 触摸拖动图标条后白框已全部隐藏；首轮按键仅恢复「屏上最左可见」图标白框，不切换项、不平移
static bool s_border_hidden_after_touch_drag = false;
/* 松手惯性动画占位（在 main_move_white_border / pressing 中 lv_anim_del） */
static int32_t s_coast_var_dummy;
static int32_t s_coast_prev;
static int32_t s_deferred_inert;

// 按下事件回调函数声明
static void  pressing_cb(lv_event_t * e);
// 释放事件回调函数声明
static void  released_cb(lv_event_t * e);
// 设置x坐标回调函数声明
static void set_x_cb(void * var, int32_t v);
static void icon_set_x_cb(void * var, int32_t v);
// 自定义动画创建函数声明
static void lv_myanim_creat(void * var, lv_anim_exec_xcb_t exec_cb, uint32_t time, uint32_t delay, lv_anim_path_cb_t path_cb,
                            int32_t start, int32_t end, lv_anim_ready_cb_t completed_cb, lv_anim_deleted_cb_t deleted_cb);
// 图标点击事件回调函数声明
static void icon_click_cb(lv_event_t * e);
// 新增：打开对应屏幕函数声明
static void open_icon_screen(int32_t icon_idx);
void lv_screen_open_icon_screen_set(short x, short y);
// 根据选中索引更新图标白色边框（仅该索引对应图标显示 66x66 白框）
static void update_selected_icon_border(int32_t selected_idx);
// 统一更新白框：有选中则显示在选中图标，否则显示在最左侧图标
static void update_icon_border_display(void);
static void main_icon_release_apply_snap(int32_t inert_px);
static void main_icon_coast_exec(void * var, int32_t v);
static void main_icon_coast_ready(lv_anim_t *a);

// 获取当前带白框的图标索引（供外部调用）
int get_white_border_icon_index(void);
// 白框左右移动：is_next为true表示下一个图标，false表示上一个图标，循环
void main_move_white_border(bool is_next);
// 进入当前带白框图标对应的页面
void main_open_white_border_icon_screen(void);

#if CONFIG_LVGL_OPTION_WIFI_ENABLE
#define ICON_OPT_WIFI        ICON_DEF(ICON_ID_WIFI, _op_wifi_alpha_66x85, "WiFi设置")
#else
#define ICON_OPT_WIFI
#endif
#if CONFIG_LVGL_SLEEP_MUSIC_ENABLE
#define ICON_OPT_SLEEP_MUSIC ICON_DEF(ICON_ID_SLEEP_MUSIC, _op_sleep_music_alpha_66x85, "睡眠音乐")
#else
#define ICON_OPT_SLEEP_MUSIC
#endif
#if CONFIG_LVGL_AGENT_ENABLE
#define ICON_OPT_AGENT       ICON_DEF(ICON_ID_AGENT, _op_agent_alpha_66x85, "智能体选择")
#else
#define ICON_OPT_AGENT
#endif
#if CONFIG_LVGL_CALCULATOR_ENABLE
#define ICON_OPT_CALCULATOR  ICON_DEF(ICON_ID_CALCULATOR, _op_calculator_alpha_66x85, "计算器")
#else
#define ICON_OPT_CALCULATOR
#endif
#if defined(CONFIG_LVGL_GAMES_ENABLE) && CONFIG_LVGL_GAMES_ENABLE
#if defined(CONFIG_LVGL_DRAW_LOTS_GAME_ENABLE) && CONFIG_LVGL_DRAW_LOTS_GAME_ENABLE
#define ICON_OPT_DRAW_LOTS   ICON_DEF(ICON_ID_DRAW_LOTS, _op_draw_lots_game_66x85, "抽签")
#else
#define ICON_OPT_DRAW_LOTS
#endif
#if defined(CONFIG_LVGL_DICE_GAME_ENABLE) && CONFIG_LVGL_DICE_GAME_ENABLE
#define ICON_OPT_DICE        ICON_DEF(ICON_ID_DICE, _op_dice_game_66x85, "摇骰子")
#else
#define ICON_OPT_DICE
#endif
#if defined(CONFIG_LVGL_MOUSE_GAME_ENABLE) && CONFIG_LVGL_MOUSE_GAME_ENABLE
#define ICON_OPT_WHACK       ICON_DEF(ICON_ID_WHACK, _op_whack_game_66x85, "打地鼠")
#else
#define ICON_OPT_WHACK
#endif
#else
#define ICON_OPT_DRAW_LOTS
#define ICON_OPT_DICE
#define ICON_OPT_WHACK
#endif
#if CONFIG_LVGL_CONTENT_LIST_ENABLE
#define ICON_OPT_CONTENT     ICON_DEF(ICON_ID_CONTENT, _op_content_alpha_66x85, "内容")
#else
#define ICON_OPT_CONTENT
#endif

#define ICON_DEFINITIONS \
    ICON_DEF(ICON_ID_COUNTDOWN, _op_timer_alpha_66x85, "倒计时") \
    ICON_DEF(ICON_ID_ALARM, _op_alarm_alpha_66x85, "闹钟") \
    ICON_DEF(ICON_ID_MUSIC, _op_music_alpha_66x85, "音乐") \
    ICON_DEF(ICON_ID_AI_CONVERSATION, _op_conversation_alpha_66x85, "AI对话") \
    ICON_OPT_WIFI \
    ICON_OPT_SLEEP_MUSIC \
    ICON_OPT_AGENT \
    ICON_OPT_CALCULATOR \
    ICON_OPT_DRAW_LOTS \
    ICON_OPT_DICE \
    ICON_OPT_WHACK \
    ICON_OPT_CONTENT



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
        icon[i].x = OPTION_ICON_LEFT_MARGIN + i * OPTION_ICON_DISTANCE;
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

        /* 创建 66x66 正方形边框 overlay，默认隐藏，由 update_selected_icon_border 控制显示 */
        icon[i].border_obj = lv_obj_create(icon[i].obj);
        lv_obj_set_size(icon[i].border_obj, 66, 66);
        lv_obj_set_pos(icon[i].border_obj, 0, 0);
        lv_obj_set_style_bg_opa(icon[i].border_obj, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(icon[i].border_obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(icon[i].border_obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_opa(icon[i].border_obj, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_side(icon[i].border_obj, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(icon[i].border_obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_clear_flag(icon[i].border_obj, LV_OBJ_FLAG_CLICKABLE);  /* 不拦截点击 */
        lv_obj_add_flag(icon[i].border_obj, LV_OBJ_FLAG_HIDDEN);
    }
    selected_icon_idx = -1;
    s_border_hidden_after_touch_drag = false;
    update_icon_border_display();   /* 未点击时最左侧显示白框 */
}



// 异步回调：在 LVGL 主循环中执行白框显示更新，避免在非 UI 线程改控件
static void update_selected_icon_border_async_cb(void *data)
{
    LV_UNUSED(data);
    int32_t selected_idx = s_pending_border_idx;
    for (int32_t i = 0; i < OPTION_ICON_COUNT; i++) {
        lv_obj_add_flag(icon[i].border_obj, LV_OBJ_FLAG_HIDDEN);
    }
    if (selected_idx >= 0 && selected_idx < OPTION_ICON_COUNT) {
        lv_obj_clear_flag(icon[selected_idx].border_obj, LV_OBJ_FLAG_HIDDEN);
    }
}

// 根据选中索引更新白色边框（异步）：保证页面上只有一个图标有白框
static void update_selected_icon_border(int32_t selected_idx)
{
    s_pending_border_idx = selected_idx;
    lv_async_call(update_selected_icon_border_async_cb, NULL);
}

// 统一更新白框：有点击选中则显示在选中图标，否则默认显示在倒计时图标（索引0）
static void update_icon_border_display(void)
{
    int32_t show_idx;
    if (selected_icon_idx >= 0 && selected_icon_idx < OPTION_ICON_COUNT) {
        show_idx = selected_icon_idx;
    } else {
        show_idx = 0;  // 默认倒计时图标
    }
    update_selected_icon_border(show_idx);
}

// 根据图标索引返回中文名称，用于打印（与 ICON_ID_* 一致）
static const char *get_icon_name_cn(int32_t idx)
{
    if (idx == ICON_ID_COUNTDOWN) {
        return "倒计时";
    }
    if (idx == ICON_ID_ALARM) {
        return "闹钟";
    }
    if (idx == ICON_ID_MUSIC) {
        return "音乐";
    }
    if (idx == ICON_ID_AI_CONVERSATION) {
        return "AI对话";
    }
#if CONFIG_LVGL_OPTION_WIFI_ENABLE
    if (idx == ICON_ID_WIFI) {
        return "WiFi设置";
    }
#endif
#if CONFIG_LVGL_SLEEP_MUSIC_ENABLE
    if (idx == ICON_ID_SLEEP_MUSIC) {
        return "睡眠音乐";
    }
#endif
#if CONFIG_LVGL_AGENT_ENABLE
    if (idx == ICON_ID_AGENT) {
        return "智能体选择";
    }
#endif
#if CONFIG_LVGL_CALCULATOR_ENABLE
    if (idx == ICON_ID_CALCULATOR) {
        return "计算器";
    }
#endif
#if defined(CONFIG_LVGL_GAMES_ENABLE) && CONFIG_LVGL_GAMES_ENABLE
#if defined(CONFIG_LVGL_DRAW_LOTS_GAME_ENABLE) && CONFIG_LVGL_DRAW_LOTS_GAME_ENABLE
    if (idx == ICON_ID_DRAW_LOTS) {
        return "抽签";
    }
#endif
#if defined(CONFIG_LVGL_DICE_GAME_ENABLE) && CONFIG_LVGL_DICE_GAME_ENABLE
    if (idx == ICON_ID_DICE) {
        return "摇骰子";
    }
#endif
#if defined(CONFIG_LVGL_MOUSE_GAME_ENABLE) && CONFIG_LVGL_MOUSE_GAME_ENABLE
    if (idx == ICON_ID_WHACK) {
        return "打地鼠";
    }
#endif
#endif
#if CONFIG_LVGL_CONTENT_LIST_ENABLE
    if (idx == ICON_ID_CONTENT) {
        return "内容";
    }
#endif
    return "未知";
}

/* 与屏幕水平 [0, scr_w] 相交的图标里，x 最小者（屏上最左可见） */
static int32_t main_leftmost_on_screen_icon_index(void)
{
    int32_t best = -1;
    int32_t best_x = 0;
    int32_t i;
    for (i = 0; i < OPTION_ICON_COUNT; i++) {
        int32_t x = lv_obj_get_x(icon[i].obj);
        int32_t w = lv_obj_get_width(icon[i].obj);
        if (x + w <= 0 || x >= scr_w) {
            continue;
        }
        if (best < 0 || x < best_x) {
            best = i;
            best_x = x;
        }
    }
    return (best >= 0) ? best : 0;
}

// 获取当前带白框的图标索引：有选中返回选中索引，否则默认返回倒计时图标（索引0）
int get_white_border_icon_index(void)
{
    if (s_border_hidden_after_touch_drag) {
        int32_t lm = main_leftmost_on_screen_icon_index();
        printf("[白框] 当前带白框图标: %s (索引%d, 触摸拖动后白框已隐藏，屏上最左可见)\n", get_icon_name_cn(lm), lm);
        return lm;
    }
    if (selected_icon_idx >= 0) {
        printf("[白框] 当前带白框图标: %s (索引%d, 来自选中)\n", get_icon_name_cn(selected_icon_idx), selected_icon_idx);
        return selected_icon_idx;
    }
    printf("[白框] 当前带白框图标: %s (索引0, 默认)\n", get_icon_name_cn(0));
    return 0;
}

// 用形参控制左右移动：move_left 为 true 整条向左平移一格（x 减小），false 向右（x 增大）
void lv_icon_move_dir(bool move_left)
{
    int32_t offset_x = move_left ? -OPTION_ICON_DISTANCE : OPTION_ICON_DISTANCE;
    for (int32_t i = 0; i < OPTION_ICON_COUNT; i++) {
        // 计算新坐标（先算后更，避免动画参数错误）
        int32_t new_x = icon[i].x + offset_x;

        // 创建动画（使用补偿后的正确坐标）
        lv_myanim_creat(icon[i].obj, icon_set_x_cb, OPTION_ICON_SCROLL_ANIM_MS, 0, lv_anim_path_ease_in_out,
                        icon[i].x, new_x, NULL, NULL);

        // 更新图标最终坐标
        icon[i].x = new_x;
    }


    // （可选）如果需要更新最大图标状态，补充调用
    // update_max_icon_status();
}

/* 首、尾图标均完全在水平显示区内时，白框切换不必整条平移 */
static bool main_icons_first_and_last_both_on_screen(void)
{
    int32_t w0 = lv_obj_get_width(icon[0].obj);
    int32_t x0 = icon[0].x;
    int32_t li = OPTION_ICON_COUNT - 1;
    int32_t wL = lv_obj_get_width(icon[li].obj);
    int32_t xL = icon[li].x;
    return (x0 >= 0 && (x0 + w0) <= scr_w && xL >= 0 && (xL + wL) <= scr_w);
}

// 白框左右移动：按屏幕上的实际白框位置，is_next 为 true 表示移到右侧下一个，false 表示左侧上一个，循环
// 若白框所在图标在屏外，则整体平移使该图标进入可见区域
void main_move_white_border(bool is_next)
{
    int32_t i;

    /* 取消未完成的平移/对齐动画，并用控件实际 x 写回 icon[].x。
     * 否则 lv_icon_move_dir 会把 icon[].x 先写成动画终点，环绕后又可能为负，
     * 在“下一个”且实际应处理右侧裁切时仍命中 sel_x<0，误触发反向平移。 */
    for (i = 0; i < OPTION_ICON_COUNT; i++) {
        lv_anim_del(icon[i].obj, set_x_cb);
        lv_anim_del(icon[i].obj, icon_set_x_cb);
        icon[i].x = lv_obj_get_x(icon[i].obj);
    }
    (void)lv_anim_del((void *)&s_coast_var_dummy, main_icon_coast_exec);

    /* 触摸拖动后白框已隐藏：首轮按键只恢复屏上最左可见图标白框，不切换上/下一项、不平移 */
    if (s_border_hidden_after_touch_drag) {
        s_border_hidden_after_touch_drag = false;
        int32_t left = main_leftmost_on_screen_icon_index();
        selected_icon_idx = left;
        LV_UNUSED(is_next);
        printf("[白框] 触摸拖动后首轮按键: 恢复 %s (索引%d，屏上最左)，未切换/未平移\n", get_icon_name_cn(left), left);
        update_selected_icon_border(left);
        return;
    }

    int32_t cur = get_white_border_icon_index();
    int32_t next_idx;
    if (is_next) {
        next_idx = (cur + 1) % OPTION_ICON_COUNT;
    } else {
        next_idx = cur - 1;
        if (next_idx < 0) {
            next_idx = OPTION_ICON_COUNT - 1;
        }
    }
    selected_icon_idx = next_idx;
    printf("[白框] 移动后带白框图标: %s (索引%d, %s)\n", get_icon_name_cn(selected_icon_idx), selected_icon_idx, is_next ? "下一个" : "上一个");

    /* 首尾循环：与 pressing_cb 一致，整条平移一格并依赖 x 回调做 total_width 环绕 */
    bool wrap_from_last = (cur == OPTION_ICON_COUNT - 1 && is_next);
    bool wrap_from_first = (cur == 0 && !is_next);

    int32_t icon_width = lv_obj_get_width(icon[selected_icon_idx].obj);
    int32_t sel_x = icon[selected_icon_idx].x;
    bool off_left = sel_x < 0;
    bool off_right = (sel_x + icon_width > scr_w);


    if (wrap_from_last) {
        if (!main_icons_first_and_last_both_on_screen()) {
            lv_icon_move_dir(true);  /* 最后一个按“下一个”：x 增大，首项从左侧绕入 */
        }

    } else if (wrap_from_first) {
        if (!main_icons_first_and_last_both_on_screen()) {
            lv_icon_move_dir(false);   /* 第一个按“上一个”：x 减小，末项从右侧绕入 */
        }
    } else if (is_next) {
        /* 下一个：优先右侧裁切；左侧外则 x 增大拉回，右侧外则 x 减小拉回 */
        if (off_right) {
            lv_icon_move_dir(true);
        } else if (off_left) {
            // printf("[白框] 平移：按下一个键，选中图标左侧超出屏幕，整条向右平移一格\n");
            lv_icon_move_dir(true);
        } else {
        }
    } else {
        if (off_left) {
            lv_icon_move_dir(false);
        } else if (off_right) {
            lv_icon_move_dir(false);
        } else {
        }
    }

    /* 更新白框显示：确保移动后白框仍在目标图标上 */
    update_selected_icon_border(selected_icon_idx);
}


// 进入当前带白框图标对应的页面
void main_open_white_border_icon_screen(void)
{
    int32_t idx = get_white_border_icon_index();
    open_icon_screen(idx);
}

/* 主界面选项条整排平移 dx（px），含循环与 lv_obj 同步，与 pressing_cb 中逻辑一致 */
static void main_icon_row_shift_dx(int32_t dx)
{
    for (int32_t i = 0; i < OPTION_ICON_COUNT; i++) {
        icon[i].x += dx;
        int32_t icon_width = lv_obj_get_width(icon[i].obj);
        int32_t total_width = OPTION_ICON_COUNT * OPTION_ICON_DISTANCE;
        while (icon[i].x < -icon_width) {
            icon[i].x += total_width;
        }
        while (icon[i].x > (scr_w)) {
            icon[i].x -= total_width;
        }
        lv_obj_set_x(icon[i].obj, icon[i].x);
    }
}

/* 松手惯性分帧：lv_anim 插值 0→inert 用 ease_out，像「多滑一截再减速停」 */
static void main_icon_coast_exec(void * var, int32_t v)
{
    (void)var;
    {
        int32_t d = v - s_coast_prev;
        s_coast_prev = v;
        if (d) {
            main_icon_row_shift_dx(d);
        }
    }
}

/* 松手后网格吸附 + 白框（inert 用于拉长吸附时间） */
static void main_icon_release_apply_snap(int32_t inert_px)
{
    int32_t offset_x = 0;
    uint32_t snap_dur_ms;
    {
        uint32_t extra;
        snap_dur_ms = (uint32_t)OPTION_ICON_RELEASE_SNAP_ANIM_MS;
        if (inert_px != 0) {
            extra = (uint32_t)LV_ABS(inert_px) / 2U;
            if (extra > 450) {
                extra = 450;
            }
            snap_dur_ms += extra;
        }
    }
    {
        int32_t x0 = icon[0].x;
        int32_t r = (int32_t)((x0 - (int32_t)OPTION_ICON_LEFT_MARGIN) % (int32_t)OPTION_ICON_DISTANCE);
        if (r < 0) {
            r += (int32_t)OPTION_ICON_DISTANCE;
        }
        if (r > (int32_t)OPTION_ICON_DISTANCE / 2) {
            offset_x = (int32_t)OPTION_ICON_DISTANCE - r;
        } else {
            offset_x = -r;
        }
    }
    for (int32_t i = 0; i < OPTION_ICON_COUNT; i++) {
        int32_t new_x = icon[i].x + offset_x;
        int32_t icon_width = lv_obj_get_width(icon[i].obj);
        int32_t total_width = OPTION_ICON_COUNT * OPTION_ICON_DISTANCE;
        while (new_x < -icon_width * OPTION_ICON_COUNT) {
            new_x += total_width;
        }
        while (new_x > icon_width * OPTION_ICON_COUNT * 2) {
            new_x -= total_width;
        }
#if OPTION_ICON_RELEASE_SNAP_ENABLE
        lv_myanim_creat(icon[i].obj, set_x_cb, snap_dur_ms, 0,
                        OPTION_ICON_RELEASE_SNAP_PATH,
                        icon[i].x, new_x, 0, 0);
        icon[i].x = new_x;
#else
        set_x_cb(icon[i].obj, new_x);
#endif
    }
    if (!s_border_hidden_after_touch_drag) {
        update_icon_border_display();
    }
}

static void main_icon_coast_ready(lv_anim_t *a)
{
    (void)a;
    main_icon_release_apply_snap(s_deferred_inert);
}

// 按下事件回调函数
static void  pressing_cb(lv_event_t * e)
{
    static  lv_point_t click_point1, click_point2;
    int32_t i;

    // 如果当前未处于触摸状态
    if (touched == false) {
        for (i = 0; i < OPTION_ICON_COUNT; i++) {
            /* 按键平移与松手对齐可能分别使用 icon_set_x_cb / set_x_cb，需一并取消 */
            lv_anim_del(icon[i].obj, set_x_cb);
            lv_anim_del(icon[i].obj, icon_set_x_cb);
        }
        (void)lv_anim_del((void *)&s_coast_var_dummy, main_icon_coast_exec);

        // 获取当前输入设备的点击点坐标
        lv_indev_get_point(lv_indev_get_act(), &click_point1);
        s_last_drag_dx = 0;
        // 设置触摸状态为已触摸
        touched = true;
        return;
    } else {
        // 如果已经处于触摸状态，获取当前点击点坐标
        lv_indev_get_point(lv_indev_get_act(), &click_point2);
    }

    // 计算触摸偏移量x
    t_offset_x = click_point2.x - click_point1.x;
    s_last_drag_dx = t_offset_x;
    /* 一旦发生明显水平拖动，隐藏全部白框，直到下次按键 main_move_white_border */
    if (t_offset_x > 2 || t_offset_x < -2) {
        if (!s_border_hidden_after_touch_drag) {
            s_border_hidden_after_touch_drag = true;
            selected_icon_idx = -1;
            update_selected_icon_border(-1);
        }
    }
    // 更新上一次点击点坐标
    click_point1.x = click_point2.x;

    main_icon_row_shift_dx(t_offset_x);
}



// 释放事件回调函数
static void  released_cb(lv_event_t * e)
{
    lv_point_t release_pos;
    // 获取释放时的坐标
    lv_indev_get_point(lv_indev_get_act(), &release_pos);

    int32_t inert;
    uint32_t coast_ms;
    (void)release_pos;
    (void)e;

    touched = false;
    inert = 0;

#if OPTION_MAIN_INERTIA_ENABLE
    {
        int32_t vx = 0;
        lv_indev_t *d = lv_indev_get_act();
        if (d) {
            lv_point_t v;
            lv_indev_get_vect(d, &v);
            vx = (lv_coord_t)v.x;
        }
        inert = s_last_drag_dx * (int32_t)OPTION_MAIN_INERTIA_COEFF
                + (vx * (int32_t)OPTION_MAIN_INERTIA_VECT_W) / 2;
        inert /= 2;
        if (LV_ABS(s_last_drag_dx) < (int32_t)OPTION_MAIN_INERTIA_MIN_DX
            && LV_ABS(vx) < (int32_t)OPTION_MAIN_INERTIA_MIN_DX) {
            inert = 0;
        } else {
            if (inert > (int32_t)OPTION_MAIN_INERTIA_MAX_PX) {
                inert = (int32_t)OPTION_MAIN_INERTIA_MAX_PX;
            } else if (inert < -(int32_t)OPTION_MAIN_INERTIA_MAX_PX) {
                inert = -(int32_t)OPTION_MAIN_INERTIA_MAX_PX;
            }
        }
    }
#endif
    s_last_drag_dx = 0;

#if OPTION_MAIN_INERTIA_ENABLE && OPTION_MAIN_INERTIA_COAST_ENABLE
    if (inert != 0) {
        s_deferred_inert = inert;
        s_coast_prev = 0;
        coast_ms = (uint32_t)OPTION_MAIN_INERTIA_COAST_TIME_MIN_MS
                   + (uint32_t)LV_ABS(inert) * (uint32_t)OPTION_MAIN_INERTIA_COAST_TIME_PER_PX;
        if (coast_ms < (uint32_t)OPTION_MAIN_INERTIA_COAST_TIME_MIN_MS) {
            coast_ms = (uint32_t)OPTION_MAIN_INERTIA_COAST_TIME_MIN_MS;
        }
        if (coast_ms > (uint32_t)OPTION_MAIN_INERTIA_COAST_TIME_MAX_MS) {
            coast_ms = (uint32_t)OPTION_MAIN_INERTIA_COAST_TIME_MAX_MS;
        }
        lv_myanim_creat((void *)&s_coast_var_dummy, main_icon_coast_exec, coast_ms, 0,
                        OPTION_MAIN_INERTIA_COAST_PATH, 0, inert, main_icon_coast_ready, NULL);
        return;
    }
#endif
    if (inert != 0) {
        main_icon_row_shift_dx(inert);
    }
    main_icon_release_apply_snap(inert);
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
            s_border_hidden_after_touch_drag = false;
            selected_icon_idx = i;
            update_selected_icon_border(i);   /* 点击哪个图标就给哪个加白框 */
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
// 设置x坐标回调函数
static void icon_set_x_cb(void * var, int32_t v)
{
    lv_obj_t *obj = (lv_obj_t *)var;
    int32_t icon_width = lv_obj_get_width(obj);
    int32_t total_width = OPTION_ICON_COUNT * OPTION_ICON_DISTANCE;
    /* 与 set_x_cb / pressing_cb 相同：按键平移时图标条循环滚动 */
    if (v < -icon_width) {
        v += total_width;
    } else if (v > (scr_w)) {
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
    case ICON_ID_COUNTDOWN:
        printf("点击进入倒计时页面");
        switch_to_timer_page();
        break;
    case ICON_ID_ALARM:
        printf("点击进入闹钟页面");
        switch_to_alarm_page();
        break;
    case ICON_ID_MUSIC:
        printf("点击进入音乐播放页面");
        switch_to_music_page("net_music");
        break;
    case ICON_ID_AI_CONVERSATION:
        printf("点击进入对话页面");
        switch_to_conversation_page();
        lv_demo_ai_dialogue_speak_listen(1);
        aisp_wake(0);
        break;
#if CONFIG_LVGL_OPTION_WIFI_ENABLE
    case ICON_ID_WIFI:
        printf("点击进入WiFi设置页面");
        switch_to_wifi_show_page();
        break;
#endif
#if CONFIG_LVGL_SLEEP_MUSIC_ENABLE
    case ICON_ID_SLEEP_MUSIC:
        printf("点击进入睡眠音乐页面");
        switch_to_sleep_music_page();
        break;
#endif
#if CONFIG_LVGL_AGENT_ENABLE
    case ICON_ID_AGENT:
        printf("点击进入智能体选择页面");
        switch_to_agent_page();
        break;
#endif
#if CONFIG_LVGL_CALCULATOR_ENABLE
    case ICON_ID_CALCULATOR:
        printf("点击进入计算机页面");
        switch_to_calculator_page();
        break;
#endif
#if defined(CONFIG_LVGL_GAMES_ENABLE) && CONFIG_LVGL_GAMES_ENABLE
#if defined(CONFIG_LVGL_DRAW_LOTS_GAME_ENABLE) && CONFIG_LVGL_DRAW_LOTS_GAME_ENABLE
    case ICON_ID_DRAW_LOTS:
        printf("选项条进入抽签\n");
        switch_to_draw_lots_game_page();
        break;
#endif
#if defined(CONFIG_LVGL_DICE_GAME_ENABLE) && CONFIG_LVGL_DICE_GAME_ENABLE
    case ICON_ID_DICE:
        printf("选项条进入摇骰子\n");
        switch_to_dice_game_page();
        break;
#endif
#if defined(CONFIG_LVGL_MOUSE_GAME_ENABLE) && CONFIG_LVGL_MOUSE_GAME_ENABLE
    case ICON_ID_WHACK:
        printf("选项条进入打地鼠\n");
        switch_to_whack_game_page();
        break;
#endif
#endif
#if CONFIG_LVGL_CONTENT_LIST_ENABLE
    case ICON_ID_CONTENT:
        printf("点击进入内容列表页面");
        switch_to_list_page();
        break;
#endif
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
#include "lvgl_develop.h"
#include "gui_guider.h"
#include "cJSON_common\cJSON.h"


// 闹钟检测相关声明
int get_alarm_count(void); // 获取闹钟数量
void alarm_status_check(void);
int audio_app_mode_check(void);
// 时间更新配置参数
#define TIME_UPDATE_INTERVAL_MS     1000    // 时间轮询间隔(毫秒)
#define TIME_RETRY_INTERVAL_MS      1000    // 时间重试间隔(毫秒)
#define TIME_MAX_RETRY_COUNT        60      // 最大重试次数

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
    lv_obj_set_pos(guider_ui.screen_main_img_location, LOCATION_ICON_X, LOCATION_ICON_Y);
    lv_obj_set_pos(guider_ui.screen_main_span_direction, CITY_ADDRESS_SPAN_X, CITY_ADDRESS_SPAN_Y);

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
                // 异步更新时间
                lv_async_call(time_update_async_cb, NULL);
                if (is_net_page()) {
                    switch_to_main_page();
                }
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
    extern int lv_demo_system_bt_connect(void);
    int bt_status = lv_demo_system_bt_connect() ? 1 : 0;

    /* 主界面与下拉栏状态栏：已连接显示「点亮」图(img_set_BT)，未连接显示灰态(img_no_BT) */
    if (guider_ui.screen_main_img_no_BT && guider_ui.screen_main_img_set_BT) {
        lv_obj_set_pos(guider_ui.screen_main_img_set_BT, BLUETOOTH_ICON_X, BLUETOOTH_ICON_Y);
        lv_obj_set_pos(guider_ui.screen_main_img_no_BT, BLUETOOTH_ICON_X, BLUETOOTH_ICON_Y);
    }
    if (guider_ui.screen_dropdown_img_set_BT && guider_ui.screen_dropdown_img_no_BT) {
        lv_obj_set_pos(guider_ui.screen_dropdown_img_set_BT, BLUETOOTH_ICON_X, BLUETOOTH_ICON_Y);
        lv_obj_set_pos(guider_ui.screen_dropdown_img_no_BT, BLUETOOTH_ICON_X, BLUETOOTH_ICON_Y);
    }

    if (bt_status) {
        if (guider_ui.screen_main_img_no_BT && guider_ui.screen_main_img_set_BT) {
            lv_obj_add_flag(guider_ui.screen_main_img_no_BT, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(guider_ui.screen_main_img_set_BT, LV_OBJ_FLAG_HIDDEN);
        }
        if (guider_ui.screen_dropdown_img_no_BT && guider_ui.screen_dropdown_img_set_BT) {
            lv_obj_add_flag(guider_ui.screen_dropdown_img_no_BT, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(guider_ui.screen_dropdown_img_set_BT, LV_OBJ_FLAG_HIDDEN);
        }
    } else {
        if (guider_ui.screen_main_img_no_BT && guider_ui.screen_main_img_set_BT) {
            lv_obj_add_flag(guider_ui.screen_main_img_set_BT, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(guider_ui.screen_main_img_no_BT, LV_OBJ_FLAG_HIDDEN);
        }
        if (guider_ui.screen_dropdown_img_no_BT && guider_ui.screen_dropdown_img_set_BT) {
            lv_obj_add_flag(guider_ui.screen_dropdown_img_set_BT, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(guider_ui.screen_dropdown_img_no_BT, LV_OBJ_FLAG_HIDDEN);
        }
    }

    /* 下拉控制区大蓝牙键：与连接态一致切换资源（原先两态均为灰图，连接后仍显灰） */
    if (guider_ui.screen_dropdown_imgbtn_BT) {
        if (bt_status) {
            lv_imgbtn_set_src(guider_ui.screen_dropdown_imgbtn_BT, LV_IMGBTN_STATE_RELEASED, NULL, &_BT2_alpha_40x30, NULL);
            lv_imgbtn_set_src(guider_ui.screen_dropdown_imgbtn_BT, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &_BT2_alpha_40x30, NULL);
        } else {
            lv_imgbtn_set_src(guider_ui.screen_dropdown_imgbtn_BT, LV_IMGBTN_STATE_RELEASED, NULL, &_no_BT2_alpha_40x30, NULL);
            lv_imgbtn_set_src(guider_ui.screen_dropdown_imgbtn_BT, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &_no_BT2_alpha_40x30, NULL);
        }
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
    if (guider_ui.screen_main_img_normal_fully_battery) {
        lv_obj_add_flag(guider_ui.screen_main_img_normal_fully_battery, LV_OBJ_FLAG_HIDDEN);
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
    if (guider_ui.screen_dropdown_img_normal_fully_battery) {
        lv_obj_add_flag(guider_ui.screen_dropdown_img_normal_fully_battery, LV_OBJ_FLAG_HIDDEN);
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
    if (guider_ui.screen_main_img_normal_fully_battery) {
        lv_obj_set_pos(guider_ui.screen_main_img_normal_fully_battery, BATTERY_ICON_X, BATTERY_ICON_Y);
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
    if (guider_ui.screen_dropdown_img_normal_fully_battery) {
        lv_obj_set_pos(guider_ui.screen_dropdown_img_normal_fully_battery, BATTERY_ICON_X, BATTERY_ICON_Y);
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
                //printf("电池电量显示: 低电量图标 (%d%%)\n", percent);
            }
        } else if (percent >= 75) {  // 满电未插电
            if (guider_ui.screen_main_img_normal_fully_battery) {
                lv_obj_clear_flag(guider_ui.screen_main_img_normal_fully_battery, LV_OBJ_FLAG_HIDDEN);
            }
            if (guider_ui.screen_dropdown_img_normal_fully_battery) {
                lv_obj_clear_flag(guider_ui.screen_dropdown_img_normal_fully_battery, LV_OBJ_FLAG_HIDDEN);
            }
        } else {  // 正常电量（未充满）
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

static void btn_content_click_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if (!ui || !ui->screen_main_btn_content) {
        return;
    }
    if (code == LV_EVENT_CLICKED) {
        printf("内容按钮被点击，进入内容列表\n");
        switch_to_list_page();
    }
}

// 初始化主页面事件
void screen_main_event_init(lv_ui *ui)
{
    if (!ui) {
        return;
    }

    lv_obj_set_scroll_dir(guider_ui.screen_main, LV_DIR_NONE);
    lv_obj_add_flag(guider_ui.screen_main_label_1, LV_OBJ_FLAG_HIDDEN);
    main_scrollicon();
    if (ui->screen_main_btn_content) {
        lv_obj_remove_event_cb(ui->screen_main_btn_content, btn_content_click_handler);
        lv_obj_add_event_cb(ui->screen_main_btn_content, btn_content_click_handler, LV_EVENT_CLICKED, ui);
    }

}
void screen_main_event_setup(void)
{
    screen_main_event_init(&guider_ui);
}
