// #include "stdio.h"
// #include "string.h"
// #include "stdlib.h"
// #include "time.h"
// #include "stdbool.h"
// #include "gui_guider.h"
// #include "custom.h"


// // 播放/暂停按钮状态
// static bool g_play_pause_btn_state = false;

// /* 磁带转动核心变量 */
// static lv_timer_t *rotate_timer = NULL;
// static uint32_t start_time = 0;       // 旋转开始的时间戳(ms)
// static uint32_t pause_offset = 0;     // 暂停时已旋转的总时间(ms)
// static const uint32_t ROTATE_PERIOD = 4000;  // 旋转一圈的时间(ms)

// /**
//  * @brief 初始化磁带动画状态
//  * 将所有动画相关的状态变量重置为初始值
//  */
// static void init_tape_animation_state(void)
// {
//     start_time = 0;       // 重置开始时间
//     pause_offset = 0;     // 重置暂停偏移
//     rotate_timer = NULL;  // 确保定时器为空
//     g_play_pause_btn_state = true; // 设置默认状态为播放
//     lv_img_set_angle(guider_ui.screen_music_img_tape, 0); // 重置角度为0度
// }
// /**
//  * @brief 定时器回调：通过绝对时间计算角度（无累加误差）
//  */
// static void rotate_timer_cb(lv_timer_t *timer)
// {
//     // 计算当前已旋转的总时间（包含历史累计）
//     uint32_t current_time = lv_tick_get() - start_time + pause_offset;
    
//     // 计算角度：(总时间 / 周期) * 360度，取模360确保在0-359范围
//     int32_t angle = (current_time * 360 * 10) / ROTATE_PERIOD % (360 * 10);
    
//     // 设置角度
//     lv_img_set_angle(guider_ui.screen_music_img_tape, angle);
// }

// /**
//  * @brief 启动磁带动画函数
//  * 开始磁带旋转动画，并设置相关图标状态
//  */
// static void tape_rotate_start(void)
// {
//     // 设置磁带条的角度为-50度
//     lv_img_set_angle(guider_ui.screen_music_img_bar, -50);
    
//     // 启动定时器（从上次停止的角度继续）
//     if (!rotate_timer) {
//         start_time = lv_tick_get();  // 记录当前启动时间
//         rotate_timer = lv_timer_create(rotate_timer_cb, 10, NULL);  // 10ms刷新一次
//         lv_timer_set_repeat_count(rotate_timer, -1);  // 无限循环
//     }
// }

// /**
//  * @brief 停止磁带动画函数
//  * 停止磁带旋转动画，并保存当前状态
//  */
// static void tape_rotate_stop(void)
// {
//     // 停止定时器并记录已旋转的时间（关键：保存历史状态）
//     if (rotate_timer) {
//         pause_offset += lv_tick_get() - start_time;
//         lv_timer_del(rotate_timer);
//         rotate_timer = NULL;
//     }
    
//     // 220
//     lv_img_set_angle(guider_ui.screen_music_img_bar, 220);
// }
// // 播放/暂停按钮点击事件处理函数
// static void screen_music_btn_play_pause_event_handler (lv_event_t *e)
// {
//     lv_event_code_t code = lv_event_get_code(e);
//     lv_ui *ui = lv_event_get_user_data(e);
//     switch (code) {
//     case LV_EVENT_CLICKED:
//     {
//         // 切换按钮状态
//         g_play_pause_btn_state = !g_play_pause_btn_state;
//         if (g_play_pause_btn_state) {
//             // 设置为播放
//             lv_label_set_text(ui->screen_music_btn_play_pause_label, "I I");
//             lv_obj_set_style_text_font(ui->screen_music_btn_play_pause_label, &lv_font_Barlow__15, LV_PART_MAIN|LV_STATE_DEFAULT);
//             tape_rotate_start();
//             // 启动歌词滚动
//             lv_label_set_long_mode(ui->screen_music_lyric, LV_LABEL_LONG_SCROLL_CIRCULAR);
//         } else {
//             // 设置为暂停
//             lv_label_set_text(ui->screen_music_btn_play_pause_label, "   "LV_SYMBOL_PLAY " ");
//             lv_obj_set_style_text_font(ui->screen_music_btn_play_pause_label, &lv_font_Barlow__11, LV_PART_MAIN|LV_STATE_DEFAULT);
//             tape_rotate_stop();
//             // 停止歌词滚动
//             lv_label_set_long_mode(ui->screen_music_lyric, LV_LABEL_LONG_DOT);
//         }
//         break;
//     }
//     default:
//         break;
//     }
// }

// // 音量进度条状态
// static bool g_volume_bar_visible = false;
// // 音量进度条自动隐藏定时器
// static lv_timer_t *g_volume_bar_timer = NULL;
// // 音量进度条自动隐藏定时器回调函数
// static void volume_bar_auto_hide_cb(lv_timer_t *timer)
// {
//     lv_ui *ui = (lv_ui *)timer->user_data;
    
//     // 隐藏音量进度条
//     lv_obj_add_flag(ui->screen_music_bar_volume, LV_OBJ_FLAG_HIDDEN);
//     g_volume_bar_visible = false;
    
//     // 删除定时器
//     if (g_volume_bar_timer != NULL) {
//         lv_timer_del(g_volume_bar_timer);
//         g_volume_bar_timer = NULL;
//     }
// }
// /**
//  * 处理音量调节功能
//  * 区分网络音乐和蓝牙音乐
//  */
// void volume_icon_click_handler(void){
//     // 显示音量进度条
//     extern lv_ui guider_ui;

//     if(!guider_ui.screen_music_bar_volume){
//         printf("[错误] 音量进度条UI对象不存在\n");
//         return;
//     }
//     lv_obj_clear_flag(guider_ui.screen_music_bar_volume, LV_OBJ_FLAG_HIDDEN);

//     // 创建5秒后隐藏进度条的定时器
//     // 修复：传递有效的任务参数（使用系统默认任务ID 0）
//     //     // 取消现有定时器
//     // if(volume_hide_timer_id != 0){
//     //     sys_timeout_del(volume_hide_timer_id);
//     //     volume_hide_timer_id = 0;
//     // }
//     // volume_hide_timer_id = sys_timeout_add_to_task("sys_timer", NULL,volume_hide_timer_cb, 5000); // 5000ms = 5秒
//        // 删除定时器
//     if (g_volume_bar_timer != NULL) {
//         lv_timer_del(g_volume_bar_timer);
//         g_volume_bar_timer = NULL;
//     }
//    // 创建新的定时器，5秒后自动隐藏
//     g_volume_bar_timer = lv_timer_create(volume_bar_auto_hide_cb, 5000, &guider_ui);
    
//     int32_t current_volume = lv_bar_get_value(guider_ui.screen_music_bar_volume);

//     // sys_all_volume_auto_set(current_volume);

// }

// /**
//  * @brief 音量进度条事件处理函数（统一处理所有触控相关事件）
//  * @param e LVGL事件对象（包含事件类型、目标对象等信息）
//  */
// static void screen_music_bar_volume_event_handler(lv_event_t *e)
// {
//     lv_obj_t *volume_bar = lv_event_get_target(e);  // 获取音量进度条对象
//     lv_event_code_t event_code = lv_event_get_code(e);  // 获取当前事件类型

//     // 处理按下和拖拽事件
//     if(event_code == LV_EVENT_PRESSED || event_code == LV_EVENT_PRESSING) {
//         // 获取当前输入设备和触摸点
//         lv_indev_t *indev = lv_indev_get_act();
//         if(indev == NULL) return;

//         lv_point_t point;
//         lv_indev_get_point(indev, &point);

//         // 获取进度条的位置和尺寸
//         lv_area_t bar_area;
//         lv_obj_get_coords(volume_bar, &bar_area);
//         int32_t bar_height = lv_area_get_height(&bar_area); // 先声明变量
//         int32_t relative_y = point.y - bar_area.y1;
//         int32_t volume = 100 - (relative_y * 100 / bar_height);

//         static uint8_t last_volume_value = 0;
//         if(volume < 10) volume = 10;
//         if(volume > 100) volume = 100;
//         if (last_volume_value && (last_volume_value == volume)) {
//             printf("[音量事件] 音量未改变\n");
//             return ;
//         }
//         last_volume_value = volume;
//         lv_bar_set_value(volume_bar, volume, LV_ANIM_OFF);
//         printf("[音量事件] 点击调节至: %d%% (触摸Y: %d, 进度条高: %d)\n", volume, relative_y, bar_height);
//         volume_icon_click_handler();
//     }
//     else if(event_code == LV_EVENT_VALUE_CHANGED) {
//         // 值改变事件
//         int32_t current_volume = lv_bar_get_value(volume_bar);
//         printf("[音量事件] 音量值已更改为: %d%%\n", (int)current_volume);
//         volume_icon_click_handler();
        
//     }
// }
// // 音量按钮点击事件处理函数
// static void screen_music_img_volume_event_handler(lv_event_t *e)
// {
//     lv_event_code_t code = lv_event_get_code(e);
//     lv_ui *ui = lv_event_get_user_data(e);
//     switch (code) {
//     case LV_EVENT_CLICKED:
//     {
//         // 音量按钮点击事件处理逻辑
//         printf("音量按钮被点击\n");
        
//         // 调用音量图标点击处理函数
//         volume_icon_click_handler();
//         break;
//     }
//     default:
//         break;
//     }
// }

// // 上一首按钮点击事件处理函数
// static void screen_music_img_prev_event_handler(lv_event_t *e)
// {
//     lv_event_code_t code = lv_event_get_code(e);
//     lv_ui *ui = lv_event_get_user_data(e);
//     switch (code) {
//     case LV_EVENT_CLICKED:
//     {
//         // 上一首按钮点击事件处理逻辑
//         printf("上一首按钮被点击\n");
//         // 这里可以添加上一首歌曲的具体实现
//         break;
//     }
//     default:
//         break;
//     }
// }

// // 下一首按钮点击事件处理函数
// static void screen_music_img_next_event_handler(lv_event_t *e)
// {
//     lv_event_code_t code = lv_event_get_code(e);
//     lv_ui *ui = lv_event_get_user_data(e);
//     switch (code) {
//     case LV_EVENT_CLICKED:
//     {
//         // 下一首按钮点击事件处理逻辑
//         printf("下一首按钮被点击\n");
//         // 这里可以添加下一首歌曲的具体实现
//         break;
//     }
//     default:
//         break;
//     }
// }
// void music_event_init(lv_ui *ui) {
//     // 添加这行代码完全禁止页面滚动
//     lv_obj_set_scroll_dir(ui->screen_music, LV_DIR_NONE);
    
//     // 为播放/暂停按钮添加点击事件
//     lv_obj_remove_event_cb(ui->screen_music_btn_play_pause, NULL);
//     lv_obj_add_event_cb(ui->screen_music_btn_play_pause, screen_music_btn_play_pause_event_handler, LV_EVENT_ALL, ui);
//     // 初始化磁带动画状态
//     init_tape_animation_state();
//     if (g_play_pause_btn_state) {
//         // 设置为播放
//         lv_label_set_text(ui->screen_music_btn_play_pause_label, "I I");
//         lv_obj_set_style_text_font(ui->screen_music_btn_play_pause_label, &lv_font_Barlow__15, LV_PART_MAIN|LV_STATE_DEFAULT);
//         tape_rotate_start();
//         // 启动歌词滚动
//         lv_label_set_long_mode(ui->screen_music_lyric, LV_LABEL_LONG_SCROLL_CIRCULAR);
//     } 
//     // 为音量按钮添加点击事件
//     lv_obj_remove_event_cb(ui->screen_music_img_volume, NULL);
//     lv_obj_add_event_cb(ui->screen_music_img_volume, screen_music_img_volume_event_handler, LV_EVENT_CLICKED, ui);
    
//     // 为上一首按钮添加点击事件
//     lv_obj_remove_event_cb(ui->screen_music_img_prev, NULL);
//     lv_obj_add_event_cb(ui->screen_music_img_prev, screen_music_img_prev_event_handler, LV_EVENT_CLICKED, ui);
    
//     // 为下一首按钮添加点击事件
//     lv_obj_remove_event_cb(ui->screen_music_img_next, NULL);
//     lv_obj_add_event_cb(ui->screen_music_img_next, screen_music_img_next_event_handler, LV_EVENT_CLICKED, ui);
    
//     // 为音量进度条添加上下移动（滑动）事件
//     lv_obj_remove_event_cb(ui->screen_music_bar_volume, NULL);
//     lv_obj_add_event_cb(ui->screen_music_bar_volume, screen_music_bar_volume_event_handler, LV_EVENT_ALL, ui);

// }

// void music_event_setup(void) {
//     music_event_init(&guider_ui);
// }


  
