/*
* Copyright 2023 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lvgl.h"
#include "gui_guider.h"
#include "custom.h"
// 宏定义
#define CAVE_COUNT 6              // 洞穴数量
#define MOUSE_MIN_MOVE_TIME 1200    // 老鼠最小移动时间（毫秒）- 0.4秒
#define MOUSE_MAX_MOVE_TIME 1600    // 老鼠最大移动时间（毫秒）- 0.6秒
#define MOUSE_CLICK_MIN_WAIT_TIME 800  // 点击后最小等待时间（毫秒）- 1秒
#define MOUSE_CLICK_MAX_WAIT_TIME 1000  // 点击后最大等待时间（毫秒）- 1.5秒
#define GAME_DURATION 30           // 游戏持续时间（秒）
#define MOUSE2_DISPLAY_TIME 2     // mouse2 图片显示时间（毫秒）
#define CAVE_Y_OFFSET -15
#define CAVE_X_OFFSET 8
// 在宏定义部分添加
#define MOUSE_CLICK_DISABLE_TIME 100  // 地鼠移动前100ms点击失效时间（毫秒）
// 在游戏状态变量部分添加
static uint32_t last_move_timestamp = 0;  // 记录地鼠上次移动的时间戳
// 游戏状态定义
typedef enum {
    GAME_READY,      // 准备开始
    GAME_PLAYING,    // 游戏进行中
    GAME_OVER        // 游戏结束
} game_state_t;

// 定义洞穴位置结构
typedef struct {
    int32_t x;
    int32_t y;
} cave_position_t;

// 所有洞穴位置 - 从setup_scr_screen_game_mouse.c中提取的实际坐标
static cave_position_t cave_positions[CAVE_COUNT] = {
    {24, 79},   // cave1
    {13, 168},  // cave2
    {95, 124},  // cave3
    {169, 69},  // cave4
    {166, 174}, // cave5
    {241, 124}, // cave6
};

// 游戏状态变量
static game_state_t game_state = GAME_READY;
static int game_countdown_seconds = GAME_DURATION;
static lv_timer_t *game_countdown_timer = NULL;
static int game_counter = 0;
static int current_cave = -1;  // 记录当前地鼠所在洞穴
static lv_timer_t *mouse_timer = NULL;  // 合并后的地鼠定时器
static lv_ui *game_ui = NULL;  // 保存UI指针
static bool is_hit_state = false; // 新增：标记当前是否为被击中状态

// 函数前向声明
void reset_mouse_game(lv_ui *ui);
static void mouse_timer_cb(lv_timer_t *timer);
static void game_countdown_timer_cb(lv_timer_t *timer);
static void mouse_click_cb(lv_event_t *e);
static void game_over_btn_cb(lv_event_t *e);
static void game_again_btn_cb(lv_event_t *e);

// 游戏开始按钮回调
static void game_start_btn_cb(lv_event_t *e)
{
    if (e->code == LV_EVENT_CLICKED && game_state == GAME_READY) {
        // 开始游戏
        game_state = GAME_PLAYING;

        // 隐藏游戏开始画面元素
        lv_obj_add_flag(game_ui->screen_whack_game_img_enter_bg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(game_ui->screen_whack_game_btn_enter_game, LV_OBJ_FLAG_HIDDEN);

        // 游戏开始时隐藏img_bg2
        lv_obj_add_flag(game_ui->screen_whack_game_black_bg, LV_OBJ_FLAG_HIDDEN);

        // // 显示游戏进行中的元素 - 注意：不再显示span_score，它只在游戏结束时显示
        lv_obj_clear_flag(game_ui->screen_whack_game_time, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(game_ui->screen_whack_game_img_countdown, LV_OBJ_FLAG_HIDDEN);

        // 重置倒计时
        game_countdown_seconds = GAME_DURATION;
        char time_text[10];
        sprintf(time_text, "%d", game_countdown_seconds);
        lv_span_set_text(game_ui->screen_whack_game_time_span, time_text);

        // 确保地鼠移动定时器存在并启动
        if (!mouse_timer) {
            mouse_timer = lv_timer_create(mouse_timer_cb, MOUSE_MIN_MOVE_TIME, NULL);
        }
        lv_timer_reset(mouse_timer);
        lv_timer_resume(mouse_timer);

        // 确保倒计时定时器存在并启动
        if (!game_countdown_timer) {
            game_countdown_timer = lv_timer_create(game_countdown_timer_cb, 1000, NULL);
        }
        lv_timer_reset(game_countdown_timer);
        lv_timer_resume(game_countdown_timer);

        // 强制更新UI
        lv_obj_refresh_ext_draw_size(game_ui->screen_whack_game_time);
        lv_obj_invalidate(game_ui->screen_whack_game_time);
    }
}

// 游戏结束按钮回调
static void game_over_btn_cb(lv_event_t *e)
{
    if (e->code == LV_EVENT_CLICKED) {
        reset_mouse_game(game_ui);
    }
}

// 再来一次按钮回调
static void game_again_btn_cb(lv_event_t *e)
{
    if (e->code == LV_EVENT_CLICKED) {
        reset_mouse_game(game_ui);
    }
}
// 修改mouse_timer_cb函数，在每次地鼠移动时更新时间戳
static void mouse_timer_cb(lv_timer_t *timer)
{
    if (game_state != GAME_PLAYING || !game_ui) {
        return;
    }
    lv_obj_add_flag(game_ui->screen_whack_game_mouse2, LV_OBJ_FLAG_HIDDEN);

    if (is_hit_state) {
        // 如果是被击中状态，隐藏mouse2，切换回普通状态
        lv_obj_add_flag(game_ui->screen_whack_game_mouse2, LV_OBJ_FLAG_HIDDEN);
        is_hit_state = false;

        // 生成新的随机洞穴位置
        int new_cave;
        do {
            new_cave = rand() % CAVE_COUNT;
        } while (CAVE_COUNT > 1 && new_cave == current_cave);
        current_cave = new_cave;

        // 移动并显示普通地鼠
        lv_obj_set_pos(
            game_ui->screen_whack_game_mouse,
            cave_positions[new_cave].x + CAVE_X_OFFSET,
            cave_positions[new_cave].y + CAVE_Y_OFFSET
        );
        lv_obj_add_flag(game_ui->screen_whack_game_mouse2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(game_ui->screen_whack_game_mouse, LV_OBJ_FLAG_HIDDEN);

        // 更新移动时间戳
        last_move_timestamp = lv_tick_get();

        // 设置下一次普通移动时间
        uint32_t random_time = MOUSE_MIN_MOVE_TIME +
                               (rand() % (MOUSE_MAX_MOVE_TIME - MOUSE_MIN_MOVE_TIME + 1));
        lv_timer_set_period(timer, random_time);
    } else {
        // 普通状态下，随机移动地鼠位置
        int new_cave;
        do {
            new_cave = rand() % CAVE_COUNT;
        } while (CAVE_COUNT > 1 && new_cave == current_cave);
        current_cave = new_cave;

        // 移动并显示普通地鼠
        lv_obj_set_pos(
            game_ui->screen_whack_game_mouse,
            cave_positions[new_cave].x + CAVE_X_OFFSET,
            cave_positions[new_cave].y + CAVE_Y_OFFSET
        );
        lv_obj_clear_flag(game_ui->screen_whack_game_mouse, LV_OBJ_FLAG_HIDDEN);

        // 更新移动时间戳
        last_move_timestamp = lv_tick_get();

        // 设置下一次普通移动时间
        uint32_t random_time = MOUSE_MIN_MOVE_TIME +
                               (rand() % (MOUSE_MAX_MOVE_TIME - MOUSE_MIN_MOVE_TIME + 1));
        lv_timer_set_period(timer, random_time);
    }
}
// 修改mouse_click_cb函数，增加点击有效性检查
static void mouse_click_cb(lv_event_t *e)
{
    // 检查游戏状态、UI指针、是否被击中状态，以及点击是否在有效时间内
    uint32_t current_time = lv_tick_get();
    uint32_t time_since_last_move = current_time - last_move_timestamp;

    if (e->code == LV_EVENT_CLICKED &&
        game_state == GAME_PLAYING &&
        game_ui &&
        !is_hit_state &&
        time_since_last_move > MOUSE_CLICK_DISABLE_TIME) {
#ifdef APPLAYER_ENABLE
        mp3_buf_play_res_file("GameHam.mp3");
#endif
        // 1. 立即保存当前被点击的洞穴位置
        int clicked_cave = current_cave;
        if (clicked_cave < 0 || clicked_cave >= CAVE_COUNT) {  // 异常防护
            return;
        }

        // 2. 显示mouse2在被点击的洞穴位置
        lv_obj_add_flag(game_ui->screen_whack_game_mouse, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(
            game_ui->screen_whack_game_mouse2,
            cave_positions[clicked_cave].x + CAVE_X_OFFSET,
            cave_positions[clicked_cave].y + CAVE_Y_OFFSET
        );
        lv_obj_clear_flag(game_ui->screen_whack_game_mouse2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(game_ui->screen_whack_game_mouse2);

        // 3. 设置为被击中状态，并设置定时器周期为mouse2显示时间
        is_hit_state = true;
        if (mouse_timer) {
            lv_timer_set_period(mouse_timer, MOUSE2_DISPLAY_TIME);
            lv_timer_reset(mouse_timer);
        }

        // 4. 更新分数
        if (game_counter < 99) {
            game_counter++;
        }

        // 更新分数显示
        char counter_text[10];
        if (game_counter >= 99) {
            sprintf(counter_text, "99+");
        } else {
            sprintf(counter_text, "得分：%02d", game_counter);
        }
        lv_span_set_text(game_ui->screen_whack_game_score_num_span, counter_text);
    }
}

// 倒计时定时器回调
static void game_countdown_timer_cb(lv_timer_t *timer)
{
    if (game_state != GAME_PLAYING || !game_ui) {
        return;
    }

    // 减少倒计时并更新显示
    game_countdown_seconds--;
    char time_text[10];
    sprintf(time_text, "%d", game_countdown_seconds);
    lv_span_set_text(game_ui->screen_whack_game_time_span, time_text);

    // 强制刷新UI以确保显示更新
    lv_obj_refresh_ext_draw_size(game_ui->screen_whack_game_time);
    lv_obj_invalidate(game_ui->screen_whack_game_time);

    // 游戏结束逻辑
    if (game_countdown_seconds <= 0) {
        game_state = GAME_OVER;

        // 停止地鼠移动和倒计时 - 修复：将 mouse_timer 改为 mouse_timer
        if (mouse_timer) {
            lv_timer_pause(mouse_timer);
        }
        if (game_countdown_timer) {
            lv_timer_pause(game_countdown_timer);
        }

        // 隐藏地鼠
        lv_obj_add_flag(game_ui->screen_whack_game_mouse, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(game_ui->screen_whack_game_mouse2, LV_OBJ_FLAG_HIDDEN);

        // 隐藏游戏进行中的元素
        lv_obj_add_flag(game_ui->screen_whack_game_time, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(game_ui->screen_whack_game_img_countdown, LV_OBJ_FLAG_HIDDEN);

        // 设置游戏结束页面元素位置（参考setup_scr_screen_game_mouse.c中的位置）
        lv_obj_set_pos(game_ui->screen_whack_game_score_num, 127, 59);
        lv_obj_set_pos(game_ui->screen_whack_game_img_score, 58, 26);
        lv_obj_set_pos(game_ui->screen_whack_game_btn_again, 80, 142);
        lv_obj_set_pos(game_ui->screen_whack_game_black_bg, 0, 0);

        // // 显示游戏结束画面元素
        lv_obj_clear_flag(game_ui->screen_whack_game_black_bg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(game_ui->screen_whack_game_img_score, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(game_ui->screen_whack_game_btn_again, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(game_ui->screen_whack_game_score_num, LV_OBJ_FLAG_HIDDEN);

    }
}

// 初始化游戏
void init_mouse_game(lv_ui *ui)
{
    game_ui = ui;

    // 初始化随机数
    srand((unsigned int)time(NULL));

    // 创建并暂停合并后的定时器
    mouse_timer = lv_timer_create(mouse_timer_cb, MOUSE_MIN_MOVE_TIME, NULL);
    lv_timer_pause(mouse_timer);

    game_countdown_timer = lv_timer_create(game_countdown_timer_cb, 1000, NULL);
    lv_timer_pause(game_countdown_timer); // 初始暂停

    // 初始化倒计时显示
    char time_text[10];
    sprintf(time_text, "%d", game_countdown_seconds);
    lv_span_set_text(ui->screen_whack_game_time_span, time_text);

    // 初始化分数显示
    char counter_text[10];
    sprintf(counter_text, "得分:%02d", game_counter);
    lv_span_set_text(ui->screen_whack_game_score_num_span, counter_text);

    lv_obj_set_scroll_dir(ui->screen_whack_game, LV_DIR_NONE);

    lv_obj_set_pos(ui->screen_whack_game_img_enter_bg, 0, 0);
    lv_obj_set_pos(ui->screen_whack_game_btn_enter_game, 78, 153);
    lv_obj_move_foreground(ui->screen_whack_game_img_enter_bg);      // 背景图层置于底层
    lv_obj_move_foreground(ui->screen_whack_game_btn_enter_game);     // 标题置于上方


    // 添加地鼠点击事件
    lv_obj_add_event_cb(ui->screen_whack_game_mouse, mouse_click_cb, LV_EVENT_CLICKED, NULL);

    // 添加游戏开始按钮事件
    lv_obj_add_event_cb(ui->screen_whack_game_btn_enter_game, game_start_btn_cb, LV_EVENT_CLICKED, NULL);

    // 添加再来一次按钮事件
    lv_obj_add_event_cb(ui->screen_whack_game_btn_again, game_again_btn_cb, LV_EVENT_CLICKED, NULL);

    // 初始状态设置
    reset_mouse_game(ui);
}

// 重置游戏
// 修改reset_mouse_game函数，修复编译错误
void reset_mouse_game(lv_ui *ui)
{
    // 停止并重置所有定时器
    if (mouse_timer) {
        lv_timer_pause(mouse_timer);
        lv_timer_set_period(mouse_timer, MOUSE_MIN_MOVE_TIME);
    }

    if (game_countdown_timer) {
        lv_timer_pause(game_countdown_timer);
    }

    // 重置游戏状态
    game_state = GAME_READY;
    game_countdown_seconds = GAME_DURATION;
    game_counter = 0;
    current_cave = -1;
    is_hit_state = false; // 重置被击中状态

    // 更新显示
    char counter_text[10];
    sprintf(counter_text, "得分:%02d", game_counter);
    lv_span_set_text(ui->screen_whack_game_score_num_span, counter_text);

    char time_text[10];
    sprintf(time_text, "%d", game_countdown_seconds);
    lv_span_set_text(ui->screen_whack_game_time_span, time_text);

    // // 隐藏所有游戏进行中和结束的元素
    lv_obj_add_flag(game_ui->screen_whack_game_black_bg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(game_ui->screen_whack_game_img_score, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(game_ui->screen_whack_game_btn_again, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(game_ui->screen_whack_game_score_num, LV_OBJ_FLAG_HIDDEN);


    // 显示游戏开始界面元素并设置正确位置
    lv_obj_clear_flag(ui->screen_whack_game_img_enter_bg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui->screen_whack_game_btn_enter_game, LV_OBJ_FLAG_HIDDEN);


    // // 确保位置固定 - 设置为UI文件中定义的原始位置
    lv_obj_set_pos(ui->screen_whack_game_img_enter_bg, 0, 0);    // 游戏标题位置
    lv_obj_set_pos(ui->screen_whack_game_btn_enter_game, 78, 153);  // 开始按钮位置


}

// 删除这个重复定义的函数
void mouse_game_event_setup(void)
{
    init_mouse_game(&guider_ui);
}


