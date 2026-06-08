#include "lvgl.h"
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"


extern lv_ui guider_ui;

/* 磁带转动核心变量 */
static lv_timer_t *rotate_timer = NULL;
static uint32_t start_time = 0;       // 旋转开始的时间戳(ms)
static uint32_t pause_offset = 0;     // 暂停时已旋转的总时间(ms)
static const uint32_t SWING_PERIOD = 3500;  // 摆动一个周期的时间(ms)
static const int32_t MAX_ANGLE = 200;       // 最大角度(20度，LVGL使用0.1度为单位)
static const int32_t MIN_ANGLE = -200;      // 最小角度(-20度)

/**
 * @brief 定时器回调：通过绝对时间计算角度（无累加误差）
 */
static void ring_rotate_timer_cb(lv_timer_t *timer)
{
    // 计算当前已摆动的总时间（包含历史累计）
    uint32_t current_time = lv_tick_get() - start_time + pause_offset;

    // 计算当前时间在周期中的位置（0到2π）
    double position = 2 * 3.1415926535 * (current_time % SWING_PERIOD) / SWING_PERIOD;

    // 使用正弦函数计算角度：-20度到20度之间平滑摆动
    int32_t angle = (int32_t)(sin(position) * ((MAX_ANGLE - MIN_ANGLE) / 2));

    // 设置角度
    lv_img_set_angle(guider_ui.screen_ring_light, angle);
}

/**
 * @brief 初始化摆动动画状态
 * 将所有动画相关的状态变量重置为初始值
 */
static void init_tape_animation_state(void)
{
    start_time = 0;       // 重置开始时间
    pause_offset = 0;     // 重置暂停偏移
    rotate_timer = NULL;  // 确保定时器为空
    lv_img_set_angle(guider_ui.screen_ring_light, 0); // 重置角度为0度
    // 停止定时器并记录已摆动的时间（关键：保存历史状态）
    if (rotate_timer) {
        pause_offset += lv_tick_get() - start_time;
        lv_timer_del(rotate_timer);
        rotate_timer = NULL;
    }

    // 启动定时器（从上次停止的角度继续）
    if (!rotate_timer) {
        start_time = lv_tick_get();  // 记录当前启动时间
        rotate_timer = lv_timer_create(ring_rotate_timer_cb, 10, NULL);  // 10ms刷新一次
        lv_timer_set_repeat_count(rotate_timer, -1);  // 无限循环
    }
}
/**
 * @brief 初始化音乐屏幕的事件处理
 */
void ring_event_init(lv_ui *ui)
{

    // 这里必须填写像素坐标，而非百分比！
    lv_img_set_pivot(ui->screen_ring_light, 100, 0);  // 请根据实际图像修改
    // 调用初始化函数
    init_tape_animation_state();
}

void ring_event_setup(void)
{
    ring_event_init(&guider_ui);
}
