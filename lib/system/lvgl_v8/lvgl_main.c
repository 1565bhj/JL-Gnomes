//#include "app_config.h"

#if LV_USE_PERF_MONITOR || LV_USE_MEM_MONITOR
#include "widgets/lv_label.h"
#endif

#define QYAI_USE_LVGL_UI_DEMO

#include "asm/gpio.h"
#include "lv_conf.h"
#include "system/includes.h"
#include "lvgl.h"
#include "sys_time.h"
#include "gui_guider.h"
#include "cJSON_common\cJSON.h"
#include "app_config.h"

// 全局UI对象
lv_ui guider_ui;


// 电池相关函数声明
void battery_update_timer_cb(lv_timer_t *timer);
void charge_status_check(void);

// 系统状态检测函数声明
extern void ai_speaker_check_network_status(void);
extern void BT_check_bluetooth_status(void);
extern void alarm_status_check(void);
void charge_status_init(void);

// UI更新函数声明
extern void time_ui_update(void);
extern void chat_process_pending_messages(void);
void init_screen_set_sliders(void);
void time_ui_update(void);
void alarm_status_check(void);

/**
 * @brief 统一的系统状态检测函数
 * 用于在一个定时器中检测电池、网络、闹钟和蓝牙状态
 */
static void system_status_check(void)
{
    // 检查电池状态
    // 由于charge_status_check可能未定义，这里提供简单实现
    // charge_status_check();

    // 空实现防止编译错误

    // 检查网络状态
    ai_speaker_check_network_status();

    // 检查闹钟状态
    alarm_status_check();

    // 检查蓝牙状态
    BT_check_bluetooth_status();

}

/**
 * LVGL主任务
 * 初始化系统并处理UI事件和时间更新
 */
static void lvgl_main_task(void *priv)
{
    LV_UNUSED(priv);
    printf("[LVGL] 初始化系统...\n");

    // 初始化LVGL核心组件
    lv_init();
    lv_port_disp_init();    // 初始化显示接口
    lv_port_indev_init();   // 初始化输入设备
    lv_port_fs_init();
    lv_png_init();

#ifdef QYAI_USE_LVGL_UI_DEMO
    // 初始化UI
    setup_ui(&guider_ui);
    events_init(&guider_ui);

    // 启动时间更新流程
    start_time_update_with_net_wait();

    alarm_ui_init();

    ai_speaker_check_network_status();

//    //定时检测系统状态
    lv_timer_create(system_status_check, 2 * 1000, NULL);

    //初始化进度条
    init_screen_set_sliders();
    printf("[LVGL] 进入主循环...\n");

#else //LVGL官方DEMO
    lv_demo_widgets();
    lv_example_rlottie_1();//FIXME:有死机问题 void renderer::Layer::render(VPainter *painter, const VRle &inheritMask
    lv_example_rlottie_2();
    lv_example_btn_1();
    lv_demo_keypad_encoder();
    lv_demo_music();
    lv_demo_stress();
    lvgl_fs_test();
#endif
    // 主事件循环
    while (1) {
        time_ui_update();    // 立即更新UI
        u32 time_till_next = lv_timer_handler();//跑lv_timer_handler刷新UI
        if (LV_DISP_DEF_REFR_PERIOD > 1 && time_till_next >= 1000 / OS_TICKS_PER_SEC) {
            msleep(time_till_next);
        }
    }
}
int lvgl_main_task_update(void)
{
    puts("lvgl_main_task_update - 更新时间显示服务\n");
    if (guider_ui.screen_main) {
        lv_obj_invalidate(lv_scr_act());  // 使当前屏幕失效
        lv_refr_now(NULL);
    }
    // 立即触发刷新（NULL 表示默认显示器）
}
/**
 * 更新包装函数
 * 用于异步调用屏幕更新
 */
void lvgl_main_task_update_async_cb(void *data)
{
    LV_UNUSED(data);
    lvgl_main_task_update();
}
/**
 * 初始化LVGL主任务
 * 创建线程运行UI主循环
 */
int lvgl_main_task_init(void)
{
    return thread_fork("lvgl_main_task", 1, 16 * 1024, 0, 0, lvgl_main_task, NULL);
}


