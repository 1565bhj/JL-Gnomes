#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lvgl.h"
#include "gui_guider.h"
#include "custom.h"
// // 外部函数声明 - 用于检测设备是否正在摇晃
extern bool gsensor_is_device_shaking(void);

// 音效播放函数声明
//extern int music_play_res_file(const char *name);
extern int mp3_buf_play_res_file(char *name);


// 游戏状态常量
enum GAME_STATE {
    GAME_STATE_READY,    // 准备状态
    GAME_STATE_WAITING_SHAKE,  // 等待摇晃状态
    GAME_STATE_ROLLING,  // 摇骰子中
    GAME_STATE_RESULT    // 显示结果
};

// 函数前向声明
static void dice_roll_timer_cb(lv_timer_t *timer);
static void reset_ui_elements(lv_ui *ui);
static void show_game_result(lv_ui *ui, bool is_success);
static void result_delay_timer_cb(lv_timer_t *timer);
static void game_die_start_btn_cb(lv_event_t *e);

static void reset_game_die(lv_ui *ui);
static void game_again_btn_cb(lv_event_t *e);
void init_game_die(lv_ui *ui);
// 游戏配置宏定义
#define RESULT_DELAY_TIME_MS 1000  // 结果显示延迟时间（毫秒）
#define MAX_ROLL_COUNT 15          // 最大滚动次数

// 游戏相关变量
static enum GAME_STATE game_state = GAME_STATE_READY; // 当前游戏状态
static int target_value = 7;  // 目标和值为7（将在初始化时重置为随机值）
static int current_attempts = 0; // 当前尝试次数
static int max_attempts = 1;   // 最大尝试次数
static lv_timer_t *dice_roll_timer = NULL; // 骰子滚动定时器
static lv_timer_t *result_delay_timer = NULL; // 结果显示延迟定时器
static int roll_count = 0; // 记录滚动次数
static int dice_values[2] = {0, 0}; // 两个骰子的值
static lv_ui *game_ui = NULL;  // 全局UI指针，与game_mouse.c保持一致

// 骰子图片资源数组
static const void *dice_images[] = {
    CONFIG_UI_RES_FILE_PATH"die1.bin", // 索引0不使用，从1开始对应骰子点数
    CONFIG_UI_RES_FILE_PATH"die1.bin",
    CONFIG_UI_RES_FILE_PATH"die2.bin",
    CONFIG_UI_RES_FILE_PATH"die3.bin",
    CONFIG_UI_RES_FILE_PATH"die4.bin",
    CONFIG_UI_RES_FILE_PATH"die5.bin",
    CONFIG_UI_RES_FILE_PATH"die6.bin"
};

// 预览骰子图片资源数组
static const void *preview_dice_images[] = {
    CONFIG_UI_RES_FILE_PATH"pdie1.bin", // 索引0不使用，从1开始对应骰子点数
    CONFIG_UI_RES_FILE_PATH"pdie1.bin",
    CONFIG_UI_RES_FILE_PATH"pdie2.bin",
    CONFIG_UI_RES_FILE_PATH"pdie3.bin",
    CONFIG_UI_RES_FILE_PATH"pdie4.bin",
    CONFIG_UI_RES_FILE_PATH"pdie5.bin",
    CONFIG_UI_RES_FILE_PATH"pdie6.bin"
};

// 重置游戏UI元素位置
static void reset_ui_elements(lv_ui *ui)
{
    // 安全检查：确保UI指针有效
    if (!ui) {
        return;
    }

    // 显示游戏背景
    if (ui->screen_dice_game_img_enter_game) {
        lv_obj_clear_flag(ui->screen_dice_game_img_enter_game, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(ui->screen_dice_game_img_enter_game, 0, 0);
    }

    // 显示游戏开始按钮
    if (ui->screen_dice_game_btn_enter_game) {
        lv_obj_clear_flag(ui->screen_dice_game_btn_enter_game, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(ui->screen_dice_game_btn_enter_game, 79, 162);
    }

    // 隐藏游戏结束相关元素
    if (ui->screen_dice_game_black_bg) {
        lv_obj_add_flag(ui->screen_dice_game_black_bg, LV_OBJ_FLAG_HIDDEN);
    }
    if (ui->screen_dice_game_btn_fail_again) {
        lv_obj_add_flag(ui->screen_dice_game_btn_fail_again, LV_OBJ_FLAG_HIDDEN);
    }
    if (ui->screen_dice_game_btn_vic_again) {
        lv_obj_add_flag(ui->screen_dice_game_btn_vic_again, LV_OBJ_FLAG_HIDDEN);
    }
    if (ui->screen_dice_game_fail) {
        lv_obj_add_flag(ui->screen_dice_game_fail, LV_OBJ_FLAG_HIDDEN);
    }
    if (ui->screen_dice_game_victory) {
        lv_obj_add_flag(ui->screen_dice_game_victory, LV_OBJ_FLAG_HIDDEN);
    }
    // 设置目标分数显示
    if (ui->screen_dice_game_score_span && ui->screen_dice_game_score) {
        char target_text[20];
        sprintf(target_text, "目标:%d点", target_value);
        lv_span_set_text(ui->screen_dice_game_score_span, target_text);
        lv_obj_clear_flag(ui->screen_dice_game_score, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(ui->screen_dice_game_score, 51, 38);
    }

    // 设置主骰子位置
    if (ui->screen_dice_game_die2) {
        lv_obj_clear_flag(ui->screen_dice_game_die2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(ui->screen_dice_game_die2, 40, 107);
    }
    if (ui->screen_dice_game_die3) {
        lv_obj_clear_flag(ui->screen_dice_game_die3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(ui->screen_dice_game_die3, 178, 107);
    }

    // 隐藏所有预览骰子
    if (ui->screen_dice_game_pdie1) {
        lv_obj_add_flag(ui->screen_dice_game_pdie1, LV_OBJ_FLAG_HIDDEN);
    }
    if (ui->screen_dice_game_pdie2) {
        lv_obj_add_flag(ui->screen_dice_game_pdie2, LV_OBJ_FLAG_HIDDEN);
    }
    if (ui->screen_dice_game_pdie3) {
        lv_obj_add_flag(ui->screen_dice_game_pdie3, LV_OBJ_FLAG_HIDDEN);
    }
    if (ui->screen_dice_game_pdie4) {
        lv_obj_add_flag(ui->screen_dice_game_pdie4, LV_OBJ_FLAG_HIDDEN);
    }
    if (ui->screen_dice_game_pdie5) {
        lv_obj_add_flag(ui->screen_dice_game_pdie5, LV_OBJ_FLAG_HIDDEN);
    }
    if (ui->screen_dice_game_pdie6) {
        lv_obj_add_flag(ui->screen_dice_game_pdie6, LV_OBJ_FLAG_HIDDEN);
    }
    // 确保img_die_name和game_die_start在最上层
    if (ui->screen_dice_game_img_enter_game) {
        lv_obj_move_foreground(ui->screen_dice_game_img_enter_game);
    }
    if (ui->screen_dice_game_btn_enter_game) {
        lv_obj_move_foreground(ui->screen_dice_game_btn_enter_game);
    }
}

// 显示游戏结果
static void show_game_result(lv_ui *ui, bool is_success)
{
    // 安全检查：确保UI指针有效
    if (!ui) {
        return;
    }

    // 显示结果背景
    if (ui->screen_dice_game_black_bg) {
        lv_obj_clear_flag(ui->screen_dice_game_black_bg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(ui->screen_dice_game_black_bg, 0, 0);
    }

    // 隐藏游戏开始相关元素
    if (ui->screen_dice_game_img_enter_game) {
        lv_obj_add_flag(ui->screen_dice_game_img_enter_game, LV_OBJ_FLAG_HIDDEN);
    }
    if (ui->screen_dice_game_btn_enter_game) {
        lv_obj_add_flag(ui->screen_dice_game_btn_enter_game, LV_OBJ_FLAG_HIDDEN);
    }

    // 隐藏所有骰子
    if (ui->screen_dice_game_die1) {
        lv_obj_add_flag(ui->screen_dice_game_die1, LV_OBJ_FLAG_HIDDEN);
    }
    if (ui->screen_dice_game_die2) {
        lv_obj_add_flag(ui->screen_dice_game_die2, LV_OBJ_FLAG_HIDDEN);
    }
    if (ui->screen_dice_game_die3) {
        lv_obj_add_flag(ui->screen_dice_game_die3, LV_OBJ_FLAG_HIDDEN);
    }
    if (ui->screen_dice_game_die4) {
        lv_obj_add_flag(ui->screen_dice_game_die4, LV_OBJ_FLAG_HIDDEN);
    }
    if (ui->screen_dice_game_die5) {
        lv_obj_add_flag(ui->screen_dice_game_die5, LV_OBJ_FLAG_HIDDEN);
    }
    if (ui->screen_dice_game_die6) {
        lv_obj_add_flag(ui->screen_dice_game_die6, LV_OBJ_FLAG_HIDDEN);
    }

    // 根据结果显示胜利或失败图片
    if (is_success) {
        if (ui->screen_dice_game_btn_vic_again) {
            lv_obj_set_pos(ui->screen_dice_game_btn_vic_again, 76, 138);
            lv_obj_clear_flag(ui->screen_dice_game_btn_vic_again, LV_OBJ_FLAG_HIDDEN);
        }

        if (ui->screen_dice_game_victory) {
            lv_obj_set_pos(ui->screen_dice_game_victory, 91, 33);
            lv_obj_clear_flag(ui->screen_dice_game_victory, LV_OBJ_FLAG_HIDDEN);
        }

    } else {
        if (ui->screen_dice_game_btn_fail_again) {
            lv_obj_set_pos(ui->screen_dice_game_btn_fail_again, 76, 138);
            lv_obj_clear_flag(ui->screen_dice_game_btn_fail_again, LV_OBJ_FLAG_HIDDEN);
        }

        if (ui->screen_dice_game_fail) {
            lv_obj_set_pos(ui->screen_dice_game_fail, 91, 33);
            lv_obj_clear_flag(ui->screen_dice_game_fail, LV_OBJ_FLAG_HIDDEN);
        }

    }

    if (is_success) {
        if (ui->screen_dice_game_black_bg) {
            lv_obj_move_foreground(ui->screen_dice_game_black_bg);
        }
        if (ui->screen_dice_game_victory) {
            lv_obj_move_foreground(ui->screen_dice_game_victory);
        }
        if (ui->screen_dice_game_btn_vic_again) {
            lv_obj_move_foreground(ui->screen_dice_game_btn_vic_again);
        }
    } else {
        // 设置图层顺序
        if (ui->screen_dice_game_black_bg) {
            lv_obj_move_foreground(ui->screen_dice_game_black_bg);
        }
        if (ui->screen_dice_game_fail) {
            lv_obj_move_foreground(ui->screen_dice_game_fail);
        }
        if (ui->screen_dice_game_btn_fail_again) {
            lv_obj_move_foreground(ui->screen_dice_game_btn_fail_again);
        }
    }
}

// 结果显示延迟回调函数
static void result_delay_timer_cb(lv_timer_t *timer)
{
    // 安全检查：确保game_ui指针有效
    if (!game_ui) {
        // 如果没有有效的UI指针，至少确保定时器被删除
        if (result_delay_timer != NULL) {
            lv_timer_del(result_delay_timer);
            result_delay_timer = NULL;
        }
        return;
    }

    // 删除定时器
    if (result_delay_timer != NULL) {
        lv_timer_del(result_delay_timer);
        result_delay_timer = NULL;
    }

    // 计算骰子和
    int sum = dice_values[0] + dice_values[1];

    // 判断是否成功
    if (sum == target_value) {
        // // 成功
#ifdef APPLAYER_ENABLE
        mp3_buf_play_res_file("GameWin.mp3");
#endif
        show_game_result(game_ui, true);
        game_state = GAME_STATE_RESULT;

    } else {
        // 检查是否还有尝试次数
        if (current_attempts >= max_attempts) {
            // // 失败
#ifdef APPLAYER_ENABLE
            mp3_buf_play_res_file("GameFail.mp3");
#endif
            show_game_result(game_ui, false);
            game_state = GAME_STATE_RESULT;
        } else {
            // 还有尝试次数，回到准备状态
            game_state = GAME_STATE_READY;
        }
    }
}

// 开始按钮点击回调函数
static void game_die_start_btn_cb(lv_event_t *e)
{
    if (e->code == LV_EVENT_CLICKED && game_state == GAME_STATE_READY) {
        // 安全检查：确保game_ui指针有效
        if (!game_ui) {
            return;
        }

        // 切换到等待摇晃状态，而不是直接开始摇晃
        game_state = GAME_STATE_WAITING_SHAKE;

        // 隐藏开始按钮和背景（添加安全检查）
        if (game_ui->screen_dice_game_img_enter_game) {
            lv_obj_add_flag(game_ui->screen_dice_game_img_enter_game, LV_OBJ_FLAG_HIDDEN);
        }
        if (game_ui->screen_dice_game_btn_enter_game) {
            lv_obj_add_flag(game_ui->screen_dice_game_btn_enter_game, LV_OBJ_FLAG_HIDDEN);
        }

        // 确保之前的定时器已经被删除
        if (dice_roll_timer != NULL) {
            lv_timer_del(dice_roll_timer);
            dice_roll_timer = NULL;
        }

        // 创建摇晃检测定时器，每100毫秒检查一次是否在摇晃
        dice_roll_timer = lv_timer_create(dice_roll_timer_cb, 100, NULL);
    }
}

// 骰子滚动动画回调函数
static void dice_roll_timer_cb(lv_timer_t *timer)
{
    // 安全检查：确保game_ui指针有效
    if (!game_ui) {
        // 如果UI指针无效，删除定时器以防止继续触发
        if (dice_roll_timer != NULL) {
            lv_timer_del(dice_roll_timer);
            dice_roll_timer = NULL;
            roll_count = 0;
        }
        return;
    }

    // 如果是等待摇晃状态，检查是否检测到摇晃
    if (game_state == GAME_STATE_WAITING_SHAKE) {
        // 调用外部函数检测是否正在摇晃
        if (gsensor_is_device_shaking()) {
            // // 检测到摇晃，开始摇骰子
#ifdef APPLAYER_ENABLE
            mp3_buf_play_res_file("GameShake.mp3");
#endif
            printf("检测到摇晃，开始摇骰子\n");
            game_state = GAME_STATE_ROLLING;
            roll_count = 0;
        }
        return; // 如果还在等待摇晃，不执行后续代码
    }

    // 每次定时器触发，随机显示骰子的面
    dice_values[0] = rand() % 6 + 1; // 第一个骰子1-6随机值
    dice_values[1] = rand() % 6 + 1; // 第二个骰子1-6随机值

    // 更新骰子图片（添加安全检查）
    if (game_ui->screen_dice_game_die2) {
        lv_img_set_src(game_ui->screen_dice_game_die2, dice_images[dice_values[0]]);
    }
    if (game_ui->screen_dice_game_die3) {
        lv_img_set_src(game_ui->screen_dice_game_die3, dice_images[dice_values[1]]);
    }

    // 滚动指定次数后停止
    roll_count++;
    if (roll_count >= MAX_ROLL_COUNT) {
        // 隐藏dice2和dice3（添加安全检查）
        if (game_ui->screen_dice_game_die2) {
            lv_obj_add_flag(game_ui->screen_dice_game_die2, LV_OBJ_FLAG_HIDDEN);
        }
        if (game_ui->screen_dice_game_die3) {
            lv_obj_add_flag(game_ui->screen_dice_game_die3, LV_OBJ_FLAG_HIDDEN);
        }

        // 添加30%的成功概率控制机制
        int random_chance = rand() % 100; // 生成0-99的随机数
        if (random_chance < 15) {
            // 30%的概率强制成功
            // 设置骰子值使它们的和等于目标值
            if (target_value <= 7) {
                dice_values[0] = 1;
                dice_values[1] = target_value - 1;
            } else {
                dice_values[0] = 6;
                dice_values[1] = target_value - 6;
            }
        }

        // 第一个预览骰子（添加安全检查）
        if (game_ui->screen_dice_game_pdie1) {
            lv_obj_add_flag(game_ui->screen_dice_game_pdie1, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(game_ui->screen_dice_game_pdie1, 40, 107);
        }

        // 第二个预览骰子（添加安全检查）
        if (game_ui->screen_dice_game_pdie2) {
            lv_obj_add_flag(game_ui->screen_dice_game_pdie2, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(game_ui->screen_dice_game_pdie2, 178, 107);
        }

        // 在最终确定骰子值后，统一设置预览骰子图片
        // 这确保了无论是否触发强制成功，图片只显示一次
        if (game_ui->screen_dice_game_pdie1) {
            lv_img_set_src(game_ui->screen_dice_game_pdie1, preview_dice_images[dice_values[0]]);
            lv_obj_clear_flag(game_ui->screen_dice_game_pdie1, LV_OBJ_FLAG_HIDDEN);
        }
        if (game_ui->screen_dice_game_pdie2) {
            lv_img_set_src(game_ui->screen_dice_game_pdie2, preview_dice_images[dice_values[1]]);
            lv_obj_clear_flag(game_ui->screen_dice_game_pdie2, LV_OBJ_FLAG_HIDDEN);
        }

        // 删除定时器
        if (dice_roll_timer != NULL) {
            lv_timer_del(dice_roll_timer);
            dice_roll_timer = NULL;
            roll_count = 0;
        }

        // 增加尝试次数
        current_attempts++;

        // 创建结果显示延迟定时器
        // 先检查是否已经存在定时器
        if (result_delay_timer != NULL) {
            lv_timer_del(result_delay_timer);
            result_delay_timer = NULL;
        }
        result_delay_timer = lv_timer_create(result_delay_timer_cb, RESULT_DELAY_TIME_MS, NULL);
    }
}

// 重置游戏
static void reset_game_die(lv_ui *ui)
{
    // 安全检查：确保UI指针有效
    if (!ui) {
        return;
    }
    // 1. 立即改变游戏状态，防止定时器回调中的状态判断问题
    game_state = GAME_STATE_READY;
    // 2. 删除所有定时器，防止它们在UI重置过程中触发回调
    if (dice_roll_timer != NULL) {
        // 确保定时器被删除前不会再次触发
        lv_timer_del(dice_roll_timer);
        dice_roll_timer = NULL;
        roll_count = 0;
    }
    if (result_delay_timer != NULL) {
        lv_timer_del(result_delay_timer);
        result_delay_timer = NULL;
    }
    // 3. 重置游戏变量
    current_attempts = 0;
    // 设置2-12之间的随机目标值
    target_value = rand() % 11 + 2; // rand()%11生成0-10，加2后变成2-12
    // 4. 最后重置UI元素
    reset_ui_elements(ui);
}

// 再来一局按钮回调函数
static void game_again_btn_cb(lv_event_t *e)
{
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    // 安全检查：确保UI指针有效
    if (!ui) {
        return;
    }
    // 重置游戏状态和UI
    reset_game_die(ui);


}

// 初始化骰子游戏功能
void init_game_die(lv_ui *ui)
{
    // 保存UI指针到全局变量
    game_ui = ui;

    // 设置随机数种子
    srand((unsigned int)time(NULL));

    // 初始化随机目标值（2-12之间）
    target_value = rand() % 11 + 2;

    lv_obj_set_scroll_dir(ui->screen_dice_game, LV_DIR_NONE);
    // 为开始按钮添加点击事件（添加安全检查，先移除旧绑定）
    if (ui->screen_dice_game_btn_enter_game) {
        // 移除之前的所有事件绑定，防止重复绑定导致冲突
        lv_obj_remove_event_cb(ui->screen_dice_game_btn_enter_game, game_die_start_btn_cb);
        lv_obj_add_event_cb(ui->screen_dice_game_btn_enter_game, game_die_start_btn_cb, LV_EVENT_CLICKED, NULL);
    }

    if (ui->screen_dice_game_btn_vic_again) {
        // 移除之前的所有事件绑定，防止重复绑定导致冲突
        lv_obj_remove_event_cb(ui->screen_dice_game_btn_vic_again, game_again_btn_cb);
        lv_obj_add_event_cb(ui->screen_dice_game_btn_vic_again, game_again_btn_cb, LV_EVENT_CLICKED, ui);
    }
    if (ui->screen_dice_game_btn_fail_again) {
        // 移除之前的所有事件绑定，防止重复绑定导致冲突
        lv_obj_remove_event_cb(ui->screen_dice_game_btn_fail_again, game_again_btn_cb);
        lv_obj_add_event_cb(ui->screen_dice_game_btn_fail_again, game_again_btn_cb, LV_EVENT_CLICKED, ui);
    }

    // 最后重置游戏状态和UI
    reset_game_die(ui);
}
void dice_game_event_setup(void)
{
    init_game_die(&guider_ui);
}

// 示例：简单的摇晃检测实现
// 注意：这只是一个示例，您需要根据实际硬件和传感器情况实现真实的摇晃检测
bool __attribute__((weak)) gsensor_is_device_shaking(void)
{
    // 在实际应用中，这里应该读取加速度传感器数据并判断是否发生了摇晃
    // 以下是一个模拟实现，随机返回true或false来模拟摇晃检测
    // 为了方便测试，这里设置一个较高的概率返回true
    return (rand() % 10 < 5);//0;//(rand() % 10 < 2); // 20%的概率检测到摇晃
}
