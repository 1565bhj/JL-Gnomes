#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdbool.h"
#include "gui_guider.h"
#include "custom.h"
#include "timer.h"

// 用于跟踪预设时间按钮的选中状态
static bool g_min5_selected = false;
static bool g_min10_selected = false;
static bool g_min25_selected = false;
static void clear_timer_add_events_and_selection(lv_ui *ui);
// 重置所有预设时间按钮的状态
static void reset_preset_time_buttons(lv_ui *ui) {
    // 重置按钮状态变量
    g_min5_selected = false;
    g_min10_selected = false;
    g_min25_selected = false;
    
    // 重置所有按钮的样式为未选中状态
    // 5分钟按钮
    lv_obj_set_style_bg_opa(ui->screen_timer_add_min5, 40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer_add_min5, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_timer_add_min5, lv_color_hex(0x9e9e9e), LV_PART_MAIN|LV_STATE_DEFAULT);
    
    // 10分钟按钮
    lv_obj_set_style_bg_opa(ui->screen_timer_add_min10, 40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer_add_min10, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_timer_add_min10, lv_color_hex(0x9e9e9e), LV_PART_MAIN|LV_STATE_DEFAULT);
    
    // 25分钟按钮
    lv_obj_set_style_bg_opa(ui->screen_timer_add_min25, 40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_timer_add_min25, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_timer_add_min25, lv_color_hex(0x9e9e9e), LV_PART_MAIN|LV_STATE_DEFAULT);
}


// 动态生成滚轮选项的辅助函数
static void generate_roller_options(lv_obj_t *roller, int start, int end, bool infinite) {
    char options[2000] = {0};  // 足够容纳所有选项
    char buffer[10] = {0};
    
    if (infinite) {
        // 无限循环模式：直接生成所有选项
        for (int i = start; i <= end; i++) {
            sprintf(buffer, "%d\n", i);
            strcat(options, buffer);
        }
    } else {
        // 正常模式但需要视觉上的无缝循环：添加额外的边界选项
        // 在开始之前添加3个结束的数字
        for (int i = end - 2; i <= end; i++) {
            sprintf(buffer, "%d\n", i);
            strcat(options, buffer);
        }
        
        // 生成主要选项
        for (int i = start; i <= end; i++) {
            sprintf(buffer, "%d\n", i);
            strcat(options, buffer);
        }
        
        // 在结束之后添加3个开始的数字
        for (int i = start; i <= start + 2; i++) {
            sprintf(buffer, "%d\n", i);
            strcat(options, buffer);
        }
    }
    
    // 移除最后一个换行符
    options[strlen(options) - 1] = '\0';
    
    // 根据参数设置滚轮模式
    lv_roller_set_options(roller, options, infinite ? LV_ROLLER_MODE_INFINITE : LV_ROLLER_MODE_NORMAL);
    
    if (!infinite) {
        // 在正常模式下，初始位置设置为第一个实际选项（索引3）
        lv_roller_set_selected(roller, 3, LV_ANIM_OFF);
    }
}

// 获取分钟/秒滚轮的实际选中值（0-59）
uint8_t get_min_sec_roller_value(lv_obj_t *roller) {
    uint16_t selected = lv_roller_get_selected(roller);
    
    // 选项结构：[57,58,59, 0-59, 0,1,2]
    if (selected == 0) {
        return 57;
    } else if (selected == 1) {
        return 58;
    } else if (selected == 2) {
        return 59;
    } else if (selected >= 3 && selected <= 62) {
        return selected - 3;
    } else if (selected == 63) {
        return 0;
    } else if (selected == 64) {
        return 1;
    } else { // selected == 65
        return 2;
    }
}

// 分钟/秒滚轮自定义无缝循环事件处理函数（正常模式下的视觉无缝循环）
static void min_sec_roller_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *roller = lv_event_get_target(e);
    
    if(code == LV_EVENT_VALUE_CHANGED) {
        uint16_t selected = lv_roller_get_selected(roller);
        
        // 实现视觉上的无缝循环效果：0-59
        if(selected == 0) {
            // 当滚动到最上端的57（我们添加的额外选项）时，无缝跳转到实际的57（索引60）
            lv_roller_set_selected(roller, 60, LV_ANIM_OFF);
        } else if(selected == 1) {
            // 当滚动到最上端的58（我们添加的额外选项）时，无缝跳转到实际的58（索引61）
            lv_roller_set_selected(roller, 61, LV_ANIM_OFF);
        } else if(selected == 2) {
            // 当滚动到最上端的59（我们添加的额外选项）时，无缝跳转到实际的59（索引62）
            lv_roller_set_selected(roller, 62, LV_ANIM_OFF);
        } else if(selected == 63) {
            // 当滚动到最下端的0（我们添加的额外选项）时，无缝跳转到实际的0（索引3）
            lv_roller_set_selected(roller, 3, LV_ANIM_OFF);
        } else if(selected == 64) {
            // 当滚动到最下端的1（我们添加的额外选项）时，无缝跳转到实际的1（索引4）
            lv_roller_set_selected(roller, 4, LV_ANIM_OFF);
        } else if(selected == 65) {
            // 当滚动到最下端的2（我们添加的额外选项）时，无缝跳转到实际的2（索引5）
            lv_roller_set_selected(roller, 5, LV_ANIM_OFF);
        }
    }
}

// 倒计时添加页面确认按钮点击事件处理函数
static void timer_add_yes_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if(!ui || !ui->screen_timer_add_img_yes){
        return;
    }
    if(code == LV_EVENT_CLICKED) {
        uint8_t hour = 0;
        uint8_t min = 0;
        uint8_t sec = 0;
        
        // 检查是否有预设时间被选中
        if (g_min5_selected) {
            // 使用5分钟预设时间
            hour = 0;
            min = 5;
            sec = 0;
        } else if (g_min10_selected) {
            // 使用10分钟预设时间
            hour = 0;
            min = 10;
            sec = 0;
        } else if (g_min25_selected) {
            // 使用25分钟预设时间
            hour = 0;
            min = 25;
            sec = 0;
        } else {
            // 获取用户通过滚轮选择的时间
            hour = lv_roller_get_selected(ui->screen_timer_add_roller_hour);
            min = get_min_sec_roller_value(ui->screen_timer_add_roller_min);
            sec = get_min_sec_roller_value(ui->screen_timer_add_roller_sec);
            if(hour == 0 && min == 0 && sec == 0) {
               return;
            }
        }
        // 根据获取的时间值创建新倒计时
        Sync_ui_dynamic_add_new_timer(ui, hour, min, sec);
        Sync_app_dynamic_add_new_timer(ui, hour, min, sec);

        // 清除所有新增页面的选择状态和事件
        clear_timer_add_events_and_selection(ui);
        // 切换回倒计时主页面
        switch_to_timer_page();
    }
}

#ifdef APPLAYER_ENABLE

// 定义TIME_CNT常量
#define TIME_CNT 10 // 与sys_instruct_word.c中保持一致

// 引入sys_instruct_word.c中定义的倒计时相关变量和函数
extern unsigned int sys_timeout_countdown_dec_cnt;
extern unsigned int sys_timeout_countdown_time[];
extern unsigned int sys_timeout_countdown_dec_cnt_id;

// 声明必要的外部函数
extern unsigned int sys_timer_add_to_task(const char *task_name, void *arg, void (*func)(void *), unsigned int period);
extern unsigned int sys_timeout_add_to_task(const char *task_name, void *arg, void (*func)(void *), unsigned int timeout);
extern void sys_timeout_countdown(void *p);
extern void sys_timer_del(unsigned int timer_id);

// 设置特定时间的倒计时启用状态
// hour: 小时 (0-23)
// minute: 分钟 (0-59)
// second: 秒 (0-59)
// enable: 是否启用倒计时
// 返回值: 0表示成功，非0表示失败
int start_timer_countdown(uint8_t hour, uint8_t min, uint8_t sec, bool enable) {
    // 参数合法性检查
    if (hour > 23 || min > 59 || sec > 59) {
        printf("[错误] 参数无效: hour=%d, minute=%d, second=%d\n", hour, min, sec);
        return -1; // 参数无效
    }

    // 计算总秒数
    uint32_t total_seconds = hour * 3600 + min * 60 + sec;
    uint32_t timeout = total_seconds * 1000; // 转换为毫秒

    // 检查总秒数是否超过合理范围（24小时）
    if (total_seconds > 86400) {
        printf("[错误] 总秒数(%d)超过24小时最大限制\n", total_seconds);
        return -2; // 超出24小时最大限制
    }

    // 根据enable参数执行不同操作
    if (!enable) {
        // 禁用倒计时
        if (sys_timeout_countdown_dec_cnt > 0) {
            sys_timeout_countdown_dec_cnt = 0;
            sys_timer_del(sys_timeout_countdown_dec_cnt_id);
            printf("[倒计时同步] 倒计时已停止\n");
        }
        return 0;
    } else {
        // 使用与sys_instruct_word.c相同的倒计时机制
        // 第一种方式：使用递减计数（每秒更新一次）
        if (sys_timeout_countdown_dec_cnt == 0) {
            sys_timeout_countdown_dec_cnt = total_seconds;
            sys_timeout_countdown_time[0] = total_seconds;
            sys_timeout_countdown_dec_cnt_id = sys_timer_add_to_task("sys_timer", &sys_timeout_countdown_dec_cnt, sys_timeout_countdown, 1000);
            printf("[倒计时同步] 倒计时启动成功: %02d:%02d:%02d, 总计%d秒\n", hour, min, sec, total_seconds);
        } else {
            // 第二种方式：使用单次定时器（适用于多个倒计时）
            for (int i = 1; i < TIME_CNT; i++) {
                if (sys_timeout_countdown_time[i] == 0) {
                    sys_timeout_countdown_time[i] = total_seconds;
                    sys_timeout_add_to_task("sys_timer", (void*)i, sys_timeout_countdown, timeout);
                    printf("[倒计时同步] 倒计时启动成功: %02d:%02d:%02d, 总计%d秒\n", hour, min, sec, total_seconds);
                    break;
                }
            }
        }
        return 0;
    }
}
#endif

// 倒计时添加页面返回按钮点击事件处理函数
static void timer_add_return_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if(!ui || !ui->screen_timer_add_img_return){
        return;
    }
    if(code == LV_EVENT_CLICKED) {
        // 清除所有新增页面的选择状态和事件
        clear_timer_add_events_and_selection(ui);
        // 切换回倒计时主页面
        switch_to_timer_page();
    }
}

// 5分钟按钮点击事件处理函数
static void timer_add_min5_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if(!ui || !ui->screen_timer_add_min5){
        return;
    }
    if(code == LV_EVENT_CLICKED) {
        // 如果已经选中，则取消选中
        if(g_min5_selected) {
            // 重置所有预设时间按钮
            reset_preset_time_buttons(ui);
            return;
        }
        
        // 重置所有预设时间按钮
        reset_preset_time_buttons(ui);
        
        // 设置当前按钮为选中状态
        g_min5_selected = true;
        
        // 应用btn_2样式（选中状态）
        lv_obj_set_style_bg_opa(ui->screen_timer_add_min5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(ui->screen_timer_add_min5, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(ui->screen_timer_add_min5, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    }
}

// 10分钟按钮点击事件处理函数
static void timer_add_min10_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if(!ui || !ui->screen_timer_add_min10){
        return;
    }
    if(code == LV_EVENT_CLICKED) {
        // 如果已经选中，则取消选中
        if(g_min10_selected) {
            // 重置所有预设时间按钮
            reset_preset_time_buttons(ui);
            return;
        }
        
        // 重置所有预设时间按钮
        reset_preset_time_buttons(ui);
        
        // 设置当前按钮为选中状态
        g_min10_selected = true;
        
        // 应用btn_2样式（选中状态）
        lv_obj_set_style_bg_opa(ui->screen_timer_add_min10, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(ui->screen_timer_add_min10, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(ui->screen_timer_add_min10, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    }
}

// 25分钟按钮点击事件处理函数
static void timer_add_min25_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if(!ui || !ui->screen_timer_add_min25){
        return;
    }
    if(code == LV_EVENT_CLICKED) {
        // 如果已经选中，则取消选中
        if(g_min25_selected) {
            // 重置所有预设时间按钮
            reset_preset_time_buttons(ui);
            return;
        }
        
        // 重置所有预设时间按钮
        reset_preset_time_buttons(ui);
        
        // 设置当前按钮为选中状态
        g_min25_selected = true;
        
        // 应用btn_2样式（选中状态）
        lv_obj_set_style_bg_opa(ui->screen_timer_add_min25, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(ui->screen_timer_add_min25, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(ui->screen_timer_add_min25, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    }
}
// 清除所有倒计时新增页面的选择状态和事件
static void clear_timer_add_events_and_selection(lv_ui *ui) {
    if(!ui) {
        return;
    }

    // 重置预设时间按钮的点击状态
    reset_preset_time_buttons(ui);

    // 重置小时滚轮状态（无限循环模式，直接设置为0）
    if(ui->screen_timer_add_roller_hour) {
        lv_roller_set_selected(ui->screen_timer_add_roller_hour, 0, LV_ANIM_OFF);
    }

    // 重置分钟滚轮状态（正常模式，设置到实际的0，索引为3）
    if(ui->screen_timer_add_roller_min) {
        // 临时移除事件处理程序，避免设置索引时被触发
        lv_obj_remove_event_cb(ui->screen_timer_add_roller_min, min_sec_roller_event_handler);
        lv_roller_set_selected(ui->screen_timer_add_roller_min, 3, LV_ANIM_OFF);
        // 重新添加事件处理程序
        lv_obj_add_event_cb(ui->screen_timer_add_roller_min, min_sec_roller_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    }

    // 重置秒滚轮状态（正常模式，设置到实际的0，索引为3）
    if(ui->screen_timer_add_roller_sec) {
        // 临时移除事件处理程序，避免设置索引时被触发
        lv_obj_remove_event_cb(ui->screen_timer_add_roller_sec, min_sec_roller_event_handler);
        lv_roller_set_selected(ui->screen_timer_add_roller_sec, 3, LV_ANIM_OFF);
        // 重新添加事件处理程序
        lv_obj_add_event_cb(ui->screen_timer_add_roller_sec, min_sec_roller_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    }
}
// 初始化新建倒计时事件
void timer_add_event_init(lv_ui *ui) {

    // 动态生成小时、分钟和秒滚轮选项
    generate_roller_options(ui->screen_timer_add_roller_hour, 0, 23, true);  // 小时: 0-23 (无限循环模式)
    generate_roller_options(ui->screen_timer_add_roller_min, 0, 59, false);  // 分钟: 0-59 (正常模式)
    generate_roller_options(ui->screen_timer_add_roller_sec, 0, 59, false);  // 秒: 0-59 (正常模式)
        
    // 注册返回按钮点击事件
    lv_obj_remove_event_cb(ui->screen_timer_add_img_return, timer_add_return_event_handler);
    lv_obj_add_event_cb(ui->screen_timer_add_img_return, timer_add_return_event_handler, LV_EVENT_CLICKED, ui);
    // 注册确认按钮点击事件
    lv_obj_remove_event_cb(ui->screen_timer_add_img_yes, timer_add_yes_event_handler);
    lv_obj_add_event_cb(ui->screen_timer_add_img_yes, timer_add_yes_event_handler, LV_EVENT_CLICKED, ui);

    // 注册分钟和秒滚轮事件处理函数（自定义无限循环）
    lv_obj_remove_event_cb(ui->screen_timer_add_roller_min, min_sec_roller_event_handler);
    lv_obj_remove_event_cb(ui->screen_timer_add_roller_sec, min_sec_roller_event_handler);
    
    lv_obj_add_event_cb(ui->screen_timer_add_roller_min, min_sec_roller_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui->screen_timer_add_roller_sec, min_sec_roller_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    
    // 先移除可能已存在的事件处理函数，防止多次注册
    lv_obj_remove_event_cb(ui->screen_timer_add_min5, timer_add_min5_event_handler);
    lv_obj_remove_event_cb(ui->screen_timer_add_min10, timer_add_min10_event_handler);
    lv_obj_remove_event_cb(ui->screen_timer_add_min25, timer_add_min25_event_handler);
    
    // 注册预设时间按钮点击事件
    lv_obj_add_event_cb(ui->screen_timer_add_min5, timer_add_min5_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_timer_add_min10, timer_add_min10_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_timer_add_min25, timer_add_min25_event_handler, LV_EVENT_CLICKED, ui);

    // 初始化预设时间按钮状态
    reset_preset_time_buttons(ui);
    clear_timer_add_events_and_selection(ui);

}

void timer_add_event_setup(void){
    timer_add_event_init(&guider_ui);
}
 