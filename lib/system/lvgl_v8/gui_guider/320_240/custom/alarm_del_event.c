
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdbool.h"
#include "gui_guider.h"
#include "custom.h"
#include "alarm.h"

// 函数原型声明
static void alarm_del_button_event_handler(lv_event_t *e);
static void alarm_del_return_event_handler(lv_event_t *e);
static void alarm_del_all_choice_event_handler(lv_event_t *e);
static void alarm_del_remove_event_handler(lv_event_t *e);
static void clear_alarm_del_events_and_selection(lv_ui *ui);
static void alarm_remove_img_del_event_handler(lv_event_t *e);
static bool all_selected = false;

/**
 * 刷新闹钟删除页面的闹钟列表
 * @param ui: GUI全局对象
 */
void refresh_alarm_del_page(lv_ui *ui) {
    // 1. 清除当前所有动态创建的闹钟条目
    lv_obj_t *cont = ui->screen_alarm_remove_cont;
    int i = 0;
    lv_obj_t *child;
    while((child = lv_obj_get_child(cont, i)) != NULL) {
        // 检查当前子对象是否需要删除（通过user_data标记）
        if(lv_obj_get_user_data(child) == (void*)1) {
            lv_obj_del(child);
            // 删除后索引不变，因为下一个子对象会移动到当前位置
        } else {
            i++;
        }
    }
    
    // 2. 遍历全局闹钟数组，动态创建闹钟条目
    for(int i = 0; i < g_alarm_count; i++) {
        // 检查闹钟是否有效
        if(g_alarms[i].time_text[0] == '\0') {
            continue;
        }
        
        // 计算新闹钟条目的Y轴位置（从1开始计数，Y轴间隔51px）
        int y_pos = 0 + i * 61; // Y轴位置，与闹钟主页面保持一致
        
        // 创建闹钟条目容器
        lv_obj_t *alarm_cont = lv_obj_create(ui->screen_alarm_remove_cont);
        lv_obj_set_pos(alarm_cont, 2, y_pos);
        lv_obj_set_size(alarm_cont, 258, 51);
        // 使用user_data标记这是动态创建的闹钟条目
        lv_obj_set_user_data(alarm_cont, (void*)1);
        
        // 设置容器样式（与静态创建的容器样式一致）
        static lv_style_t cont_style;
        lv_style_init(&cont_style);
        lv_style_set_border_width(&cont_style, 2);
        lv_style_set_border_opa(&cont_style, 0);
        lv_style_set_border_color(&cont_style, lv_color_hex(0xffffff));
        lv_style_set_border_side(&cont_style, LV_BORDER_SIDE_FULL);
        lv_style_set_radius(&cont_style, 0);
        lv_style_set_bg_opa(&cont_style, 53);
        lv_style_set_bg_color(&cont_style, lv_color_hex(0xffffff));
        lv_style_set_bg_grad_dir(&cont_style, LV_GRAD_DIR_NONE);
        lv_style_set_pad_top(&cont_style, 0);
        lv_style_set_pad_bottom(&cont_style, 0);
        lv_style_set_pad_left(&cont_style, 0);
        lv_style_set_pad_right(&cont_style, 0);
        lv_style_set_shadow_width(&cont_style, 0);
        lv_obj_add_style(alarm_cont, &cont_style, 0);
        lv_obj_set_style_radius(alarm_cont, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
        
        // 创建时间标签组（参考Dynamic_add_new_alarm的布局）
        lv_obj_t *time_group = lv_spangroup_create(alarm_cont);
        lv_spangroup_set_align((lv_obj_t *)time_group, LV_TEXT_ALIGN_LEFT);
        lv_spangroup_set_overflow((lv_obj_t *)time_group, LV_SPAN_OVERFLOW_CLIP);
        lv_spangroup_set_mode((lv_obj_t *)time_group, LV_SPAN_MODE_EXPAND);
        
        // 设置时间标签组位置和大小（与Dynamic_add_new_alarm保持一致）
        lv_obj_set_pos((lv_obj_t *)time_group, 11, 2);
        lv_obj_set_size((lv_obj_t *)time_group, 86, 25);
        
        // 创建时间文本span
        lv_span_t *time_span = lv_spangroup_new_span((lv_obj_t *)time_group);
        
        // 使用独立存储的时间文本，避免依赖可能失效的time_span指针
        lv_span_set_text(time_span, g_alarms[i].time_text);
        
        // 设置时间文本样式
        lv_style_set_text_font(&time_span->style, &lv_font_Barlow__28);
        lv_style_set_text_color(&time_span->style, lv_color_hex(0xffffff));
        lv_style_set_text_decor(&time_span->style, LV_TEXT_DECOR_NONE);
        
        // 设置span组主样式
        static lv_style_t span_group_style;
        lv_style_init(&span_group_style);
        lv_style_set_border_width(&span_group_style, 0);
        lv_style_set_radius(&span_group_style, 0);
        lv_style_set_bg_opa(&span_group_style, 0);
        lv_style_set_pad_top(&span_group_style, 0);
        lv_style_set_pad_right(&span_group_style, 0);
        lv_style_set_pad_bottom(&span_group_style, 0);
        lv_style_set_pad_left(&span_group_style, 0);
        lv_style_set_shadow_width(&span_group_style, 0);
        lv_obj_add_style((lv_obj_t *)time_group, &span_group_style, LV_PART_MAIN|LV_STATE_DEFAULT);
        lv_spangroup_refr_mode((lv_obj_t *)time_group);
        
        // 创建重复文本标签组（参考Dynamic_add_new_alarm的布局）
        lv_obj_t *repeat_group = lv_spangroup_create(alarm_cont);
        lv_spangroup_set_align((lv_obj_t *)repeat_group, LV_TEXT_ALIGN_LEFT);
        lv_spangroup_set_overflow((lv_obj_t *)repeat_group, LV_SPAN_OVERFLOW_CLIP);
        lv_spangroup_set_mode((lv_obj_t *)repeat_group, LV_SPAN_MODE_BREAK);
        
        // 创建重复文本span
        lv_span_t *repeat_span = lv_spangroup_new_span(repeat_group);
        
        // 直接使用独立存储的重复设置文本，避免重新计算导致的不一致
        // 如果重复文本为空，设置默认值
        if (g_alarms[i].repeat_text[0] == '\0') {
            lv_span_set_text(repeat_span, "只响一次");
        } else {
            lv_span_set_text(repeat_span, g_alarms[i].repeat_text);
        }
        
        // 设置重复文本样式
        lv_style_set_text_color(&repeat_span->style, lv_color_hex(0xc4c4c4));
        lv_style_set_text_decor(&repeat_span->style, LV_TEXT_DECOR_NONE);
        lv_style_set_text_font(&repeat_span->style, &lv_font_MiSansDemibold_12);
        
        // 设置重复文本组位置和大小（与Dynamic_add_new_alarm保持一致）
        lv_obj_set_pos((lv_obj_t *)repeat_group, 13, 33);
        lv_obj_set_size((lv_obj_t *)repeat_group, 239, 12);
        
        // 设置重复文本组主样式
        static lv_style_t repeat_group_style;
        lv_style_init(&repeat_group_style);
        lv_style_set_border_width(&repeat_group_style, 0);
        lv_style_set_radius(&repeat_group_style, 0);
        lv_style_set_bg_opa(&repeat_group_style, 0);
        lv_style_set_pad_top(&repeat_group_style, 0);
        lv_style_set_pad_right(&repeat_group_style, 0);
        lv_style_set_pad_bottom(&repeat_group_style, 0);
        lv_style_set_pad_left(&repeat_group_style, 0);
        lv_style_set_shadow_width(&repeat_group_style, 0);
        lv_obj_add_style((lv_obj_t *)repeat_group, &repeat_group_style, LV_PART_MAIN|LV_STATE_DEFAULT);
        lv_spangroup_refr_mode((lv_obj_t *)repeat_group);
        
        // 创建删除按钮（替换开关位置，使用imgbtn_del）
        lv_obj_t *imgbtn_del = lv_imgbtn_create(alarm_cont);
        lv_obj_add_flag(imgbtn_del, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(imgbtn_del, LV_OBJ_FLAG_CHECKABLE);
        lv_imgbtn_set_src(imgbtn_del, LV_IMGBTN_STATE_RELEASED, NULL, &_un_press_alpha_20x20, NULL);
        lv_imgbtn_set_src(imgbtn_del, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &_alarm_del_alpha_20x20, NULL);
        
        // 创建按钮标签
        lv_obj_t *imgbtn_del_label = lv_label_create(imgbtn_del);
        lv_label_set_text(imgbtn_del_label, "");
        lv_label_set_long_mode(imgbtn_del_label, LV_LABEL_LONG_WRAP);
        lv_obj_align(imgbtn_del_label, LV_ALIGN_CENTER, 0, 0);
        
        // 设置按钮内边距
        lv_obj_set_style_pad_all(imgbtn_del, 0, LV_STATE_DEFAULT);
        
        // 设置删除按钮位置和大小
        lv_obj_set_pos(imgbtn_del, 221, 14);
        lv_obj_set_size(imgbtn_del, 20, 20);
        
        // 设置按钮默认样式
        lv_obj_set_style_text_color(imgbtn_del, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(imgbtn_del, &lv_font_MiSansDemibold_18, LV_PART_MAIN|LV_STATE_DEFAULT);
        lv_obj_set_style_text_opa(imgbtn_del, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
        lv_obj_set_style_text_align(imgbtn_del, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
        lv_obj_set_style_radius(imgbtn_del, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
        lv_obj_set_style_clip_corner(imgbtn_del, true, LV_PART_MAIN|LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(imgbtn_del, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
        
        // 设置按钮按下状态样式
        lv_obj_set_style_img_recolor_opa(imgbtn_del, 0, LV_PART_MAIN|LV_STATE_PRESSED);
        lv_obj_set_style_img_opa(imgbtn_del, 255, LV_PART_MAIN|LV_STATE_PRESSED);
        lv_obj_set_style_text_color(imgbtn_del, lv_color_hex(0xFF33FF), LV_PART_MAIN|LV_STATE_PRESSED);
        lv_obj_set_style_text_font(imgbtn_del, &lv_font_MiSansDemibold_18, LV_PART_MAIN|LV_STATE_PRESSED);
        lv_obj_set_style_text_opa(imgbtn_del, 255, LV_PART_MAIN|LV_STATE_PRESSED);
        lv_obj_set_style_shadow_width(imgbtn_del, 0, LV_PART_MAIN|LV_STATE_PRESSED);
        
        // 设置按钮选中状态样式
        lv_obj_set_style_img_recolor_opa(imgbtn_del, 0, LV_PART_MAIN|LV_STATE_CHECKED);
        lv_obj_set_style_img_opa(imgbtn_del, 255, LV_PART_MAIN|LV_STATE_CHECKED);
        lv_obj_set_style_text_color(imgbtn_del, lv_color_hex(0xFF33FF), LV_PART_MAIN|LV_STATE_CHECKED);
        lv_obj_set_style_text_font(imgbtn_del, &lv_font_MiSansDemibold_18, LV_PART_MAIN|LV_STATE_CHECKED);
        lv_obj_set_style_text_opa(imgbtn_del, 255, LV_PART_MAIN|LV_STATE_CHECKED);
        lv_obj_set_style_shadow_width(imgbtn_del, 0, LV_PART_MAIN|LV_STATE_CHECKED);
        
        // 设置按钮释放状态样式
        lv_obj_set_style_img_recolor_opa(imgbtn_del, 0, LV_PART_MAIN|LV_IMGBTN_STATE_RELEASED);
        lv_obj_set_style_img_opa(imgbtn_del, 255, LV_PART_MAIN|LV_IMGBTN_STATE_RELEASED);
        
        // 根据闹钟的is_selected状态设置按钮的初始状态
        if(g_alarms[i].is_selected) {
            lv_imgbtn_set_state(imgbtn_del, LV_IMGBTN_STATE_CHECKED_RELEASED);
        } else {
            lv_imgbtn_set_state(imgbtn_del, LV_IMGBTN_STATE_RELEASED);
        }
        
        // 设置删除按钮事件
        lv_obj_add_event_cb(imgbtn_del, alarm_del_button_event_handler, LV_EVENT_CLICKED, &g_alarms[i]);
        
        // 标记为动态创建的对象，方便后续删除
        lv_obj_set_user_data(imgbtn_del, (void *)0x1);
        


    }
    
    // 检查是否有闹钟，没有则禁用全选按钮
    if (g_alarm_count == 0) {
        // 同时设置禁用状态和移除可点击标志
        lv_obj_add_state(ui->screen_alarm_remove_imgbtn_choice, LV_STATE_DISABLED);
        lv_obj_clear_flag(ui->screen_alarm_remove_imgbtn_choice, LV_OBJ_FLAG_CLICKABLE);
        all_selected = false;
    } else {
        // 同时清除禁用状态和添加可点击标志
        lv_obj_clear_state(ui->screen_alarm_remove_imgbtn_choice, LV_STATE_DISABLED);
        lv_obj_add_flag(ui->screen_alarm_remove_imgbtn_choice, LV_OBJ_FLAG_CLICKABLE);
    }
}

// 闹钟删除页面返回按钮点击事件处理函数
static void alarm_del_return_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if(!ui || !ui->screen_alarm_remove_return){
        return;
    }
    if(code == LV_EVENT_CLICKED) {
        // 切换回闹钟主页面
        clear_alarm_del_events_and_selection(ui);
        switch_to_alarm_page();
    }
}


// 单个闹钟删除按钮点击事件处理函数
static void alarm_del_button_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    
    if(code == LV_EVENT_CLICKED) {
        // 当手动选择/取消选择单个闹钟时，全选按钮恢复未按下状态
        lv_imgbtn_set_state(guider_ui.screen_alarm_remove_imgbtn_choice, LV_IMGBTN_STATE_RELEASED);
        all_selected = false;
        // 获取当前点击的闹钟对象
        AlarmItem *current_alarm = (AlarmItem *)lv_event_get_user_data(e);
        
        // 切换选择状态
        current_alarm->is_selected = !current_alarm->is_selected;
        
        // 更新按钮图片状态
        if(current_alarm->is_selected) {
            lv_imgbtn_set_state(btn, LV_IMGBTN_STATE_CHECKED_RELEASED);
        } else {
            lv_imgbtn_set_state(btn, LV_IMGBTN_STATE_RELEASED);
        }
        
        // 统计已选择的项数和实际存在的闹钟数量
        int selected_count = 0;
        bool all_alarms_selected = true;
        
        // 使用g_alarm_count而不是MAX_ALARMS来遍历实际存在的闹钟
        for(int i = 0; i < g_alarm_count; i++) {
            if(g_alarms[i].is_selected) {
                selected_count++;
            } else {
                all_alarms_selected = false;
            }
        }
        
        // 如果手动选中了所有实际存在的闹钟，设置全选按钮为选中状态
        if(all_alarms_selected && selected_count > 0 && selected_count == g_alarm_count) {
            lv_imgbtn_set_state(guider_ui.screen_alarm_remove_imgbtn_choice, LV_IMGBTN_STATE_CHECKED_RELEASED);
            all_selected = true;
        }
        
        // 更新选择文本显示
        char choice_text[50];
        if(selected_count > 0) {
            sprintf(choice_text, "已选择%d项", selected_count);
        } else {
            sprintf(choice_text, "未选择");
        }
        
        // 设置选择文本
        lv_span_set_text(guider_ui.screen_alarm_remove_choice_span, choice_text);
        
    }
}

// 全选按钮点击事件处理函数
static void alarm_del_all_choice_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    
    if(code == LV_EVENT_CLICKED) {
        // 检查当前是否所有闹钟都已被选中
        all_selected = true;
        for(int i = 0; i < g_alarm_count; i++) {
            if(!g_alarms[i].is_selected) {
                all_selected = false;
                break;
            }
        }
        
        // 全选或取消全选所有闹钟
        bool new_state = !all_selected;
        int enabled_count = 0;
        
        for(int i = 0; i < g_alarm_count; i++) {
            g_alarms[i].is_selected = new_state;
            if(g_alarms[i].is_selected) {
                enabled_count++;
            }
        }
        
        // 只有在不是全选的情况下才刷新页面
        // 刷新页面，更新按钮状态
        refresh_alarm_del_page(&guider_ui);
        
        // 更新选择文本显示
        if(new_state && enabled_count > 0) {
            char choice_text[50];
            sprintf(choice_text, "已选择%d项", enabled_count);
            lv_span_set_text(guider_ui.screen_alarm_remove_choice_span, choice_text);
        } else {
            lv_span_set_text(guider_ui.screen_alarm_remove_choice_span, "未选择");
        }
    }
}
// 删除图片按下放大、松手恢复的事件处理函数
static void alarm_remove_img_del_event_handler(lv_event_t *e) {
    printf("-> 进入删除图片事件处理函数\n");
    
    // 确保事件和目标对象有效
    if (!e) {
        printf("-> 事件对象无效，直接返回\n");
        return;
    }
    
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *img_del = lv_event_get_target(e);
    
    // 添加空指针检查
    if(!img_del) {
        printf("-> 目标对象无效，直接返回\n");
        return;
    }
    
    if(code == LV_EVENT_PRESSED) {
        // 按下时放大图片（1.1倍）
        printf("-> 按下删除按钮，放大图片\n");
        lv_img_set_zoom(img_del, 282); // 256 * 1.1 ≈ 282
    } else if(code == LV_EVENT_RELEASED) {
        // 松开时恢复原始大小
        printf("-> 松开删除按钮，恢复原始大小\n");
        lv_img_set_zoom(img_del, 256); // LV_IMG_ZOOM_NONE = 256

        // 遍历闹钟数组，从后往前删除选中的闹钟
        int removed_count = 0;
        
        // 首先记录所有需要删除的索引，避免在遍历时修改数组导致问题
        int *indices_to_delete = NULL;
        int selected_count = 0;
        
        // 第一次遍历：统计选中的闹钟数量
        for(int i = 0; i < g_alarm_count; i++) {
            if(g_alarms[i].is_selected) {
                selected_count++;
            }
        }
        
        if(selected_count == 0) {
            printf("-> 没有选中任何闹钟\n");
            return;
        }
        
        // 分配内存存储需要删除的索引
        indices_to_delete = (int *)malloc(selected_count * sizeof(int));
        if(!indices_to_delete) {
            printf("-> 内存分配失败\n");
            return;
        }
        
        // 第二次遍历：收集需要删除的索引（从后往前收集）
        int idx = 0;
        for(int i = g_alarm_count - 1; i >= 0; i--) {
            if(g_alarms[i].is_selected) {
                indices_to_delete[idx++] = i;
            }
        }
        
        // 执行删除操作
        for(int i = 0; i < selected_count; i++) {
            int delete_idx = indices_to_delete[i];
            printf("-> 删除闹钟索引: %d\n", delete_idx);
#ifdef APPLAYER_ENABLE
            alarm_delete_by_index(delete_idx);
#endif 
            // 注意：不要直接删除time_span，因为它是time_group的子元素
            // 只需要删除父容器，子元素会自动被删除
            if(g_alarms[delete_idx].time_group) {
                lv_obj_del(g_alarms[delete_idx].time_group);
                g_alarms[delete_idx].time_group = NULL;
                // time_span是time_group的子元素，删除父容器后子元素也会被删除
                g_alarms[delete_idx].time_span = NULL;
            }
            
            if(g_alarms[delete_idx].switch_obj) {
                lv_obj_del(g_alarms[delete_idx].switch_obj);
                g_alarms[delete_idx].switch_obj = NULL;
            }
            
            // 只需要删除父容器，子元素会自动被删除
            if(g_alarms[delete_idx].repeat_group) {
                lv_obj_del(g_alarms[delete_idx].repeat_group);
                g_alarms[delete_idx].repeat_group = NULL;
                // repeat_span是repeat_group的子元素，删除父容器后子元素也会被删除
                g_alarms[delete_idx].repeat_span = NULL;
            }
            
            // 从数组中移除并重新排列
            for(int j = delete_idx; j < g_alarm_count - 1; j++) {
                g_alarms[j] = g_alarms[j + 1];
            }
            
            // 更新重新排列后所有闹钟的索引值
            for(int j = 0; j < g_alarm_count - 1; j++) {
                g_alarms[j].index = j + 1; // 索引从1开始编号
            }
            
            removed_count++;
            g_alarm_count--;
        }
        
        // 释放内存
        free(indices_to_delete);
        
        printf("-> 成功删除 %d 个闹钟，剩余 %d 个\n", removed_count, g_alarm_count);
        
        //先刷新删除页面，再刷新主页面
        refresh_alarm_del_page(&guider_ui);
        refresh_alarm_page(&guider_ui);
        
        // 重置全选状态
        all_selected = false;
        if (guider_ui.screen_alarm_remove_imgbtn_choice) {
            lv_imgbtn_set_state(guider_ui.screen_alarm_remove_imgbtn_choice, LV_IMGBTN_STATE_RELEASED);
        }
        
        // 更新选择文本显示 - 添加安全检查
        if (guider_ui.screen_alarm_remove_choice_span) {
            lv_span_set_text(guider_ui.screen_alarm_remove_choice_span, "未选择");
            printf("-> 更新选择文本为: 未选择\n");
        }
        
        // 将全选按钮设置为未按下状态 - 添加安全检查
        if (guider_ui.screen_alarm_remove_imgbtn_choice) {
            lv_imgbtn_set_state(guider_ui.screen_alarm_remove_imgbtn_choice, LV_IMGBTN_STATE_RELEASED);
            printf("-> 重置全选按钮状态\n");
            
            // 如果没有闹钟了，禁用全选按钮
            if (g_alarm_count == 0) {
                lv_obj_add_state(guider_ui.screen_alarm_remove_imgbtn_choice, LV_STATE_DISABLED);
                lv_obj_clear_flag(guider_ui.screen_alarm_remove_imgbtn_choice, LV_OBJ_FLAG_CLICKABLE);
            }
        }
        
        // 重置全选标志
        all_selected = false;
        printf("-> 重置全选标志\n");
    } else if(code == LV_EVENT_PRESS_LOST) {
        // 仅在失去焦点时恢复原始大小，不执行删除操作
        printf("-> 失去焦点，恢复原始大小\n");
        lv_img_set_zoom(img_del, 256); // LV_IMG_ZOOM_NONE = 256
    }
}


static void clear_alarm_del_events_and_selection(lv_ui *ui) {
    if (!ui) {
        return;
    }
    
    // 1. 将全选按钮设置为未按下状态并清除事件处理函数
    lv_imgbtn_set_state(ui->screen_alarm_remove_imgbtn_choice, LV_IMGBTN_STATE_RELEASED);
        
    // 3. 清除所有动态创建的闹钟的选择状态
    // 遍历全局闹钟数组，重置选择状态
    for (int i = 0; i < g_alarm_count; i++) {
        g_alarms[i].is_selected = false;
    }
    
    // 4. 清除动态创建的删除按钮的选择状态
    // 获取闹钟容器
    lv_obj_t *cont = ui->screen_alarm_remove_cont;
    if (!cont) {
        return;
    }
    
    // 遍历容器中的所有子对象
    uint32_t child_count = lv_obj_get_child_cnt(cont);
    for (uint32_t i = 0; i < child_count; i++) {
        lv_obj_t *child = lv_obj_get_child(cont, i);
        if (!child) {
            continue;
        }
        
        // 检查是否为动态创建的闹钟条目容器
        if (lv_obj_get_user_data(child) == (void*)1) {
            // 遍历容器中的子对象，查找删除按钮
            uint32_t grandchild_count = lv_obj_get_child_cnt(child);
            for (uint32_t j = 0; j < grandchild_count; j++) {
                lv_obj_t *grandchild = lv_obj_get_child(child, j);
                if (!grandchild) {
                    continue;
                }
                
                // 检查是否为动态创建的删除按钮
                if (lv_obj_get_user_data(grandchild) == (void*)0x1) {
                    // 重置按钮状态为未选中
                    lv_imgbtn_set_state(grandchild, LV_IMGBTN_STATE_RELEASED);
                }
            }
        }
    }
    
    // 5. 更新选择文本显示
    lv_span_set_text(ui->screen_alarm_remove_choice_span, "未选择");
    

}

/**
 * 语音删除闹钟函数
 * @param hour: 小时（0-23）
 * @param minute: 分钟（0-59）
 * @param second: 秒（0-59，可选）
 * @return: true表示删除成功，false表示删除失败
 */
bool Voice_delete_alarm(int hour, int minute, int second) {
    printf("-> 进入语音删除闹钟函数，删除时间: %02d:%02d:%02d\n", hour, minute, second);

    // 参数有效性检查
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) {
        printf("-> 时间参数无效，无法删除\n");
        return false;
    }

    // 根据时分查找匹配的闹钟索引（忽略秒参数）
    int alarm_index = -1;

    // 遍历所有闹钟，只比较小时和分钟部分
    for (int i = 0; i < g_alarm_count; i++) {
        if (g_alarms[i].time_text[0] != '\0') {
            // 解析闹钟时间的小时和分钟
            int alarm_hour, alarm_minute;
            if (sscanf(g_alarms[i].time_text, "%d:%d", &alarm_hour, &alarm_minute) == 2) {
                // 只比较小时和分钟，忽略秒
                if (alarm_hour == hour && alarm_minute == minute) {
                    alarm_index = i;
                    break;
                }
            }
        }
    }

    if (alarm_index == -1) {
        printf("-> 未找到匹配的闹钟时间:\n");
        return false;
    }

    printf("-> 找到匹配的闹钟索引: %d, 时间: %s\n", alarm_index, g_alarms[alarm_index].time_text);

    // 删除UI对象
    // 注意：不要直接删除time_span，因为它是time_group的子元素
    // 只需要删除父容器，子元素会自动被删除
    if (g_alarms[alarm_index].time_group) {
        lv_obj_del(g_alarms[alarm_index].time_group);
        g_alarms[alarm_index].time_group = NULL;
        g_alarms[alarm_index].time_span = NULL;
    }

    if (g_alarms[alarm_index].switch_obj) {
        lv_obj_del(g_alarms[alarm_index].switch_obj);
        g_alarms[alarm_index].switch_obj = NULL;
    }

    // 只需要删除父容器，子元素会自动被删除
    if (g_alarms[alarm_index].repeat_group) {
        lv_obj_del(g_alarms[alarm_index].repeat_group);
        g_alarms[alarm_index].repeat_group = NULL;
        g_alarms[alarm_index].repeat_span = NULL;
    }

    // 从数组中移除并重新排列
    for (int j = alarm_index; j < g_alarm_count - 1; j++) {
        g_alarms[j] = g_alarms[j + 1];
    }

    // 更新重新排列后所有闹钟的索引值
    for (int j = 0; j < g_alarm_count - 1; j++) {
        g_alarms[j].index = j + 1; // 索引从1开始编号
    }

    g_alarm_count--;
    printf("-> 成功删除闹钟，剩余 %d 个\n", g_alarm_count);

    // 刷新页面
    refresh_alarm_del_page(&guider_ui);
    refresh_alarm_page(&guider_ui);

    // 重置全选状态
    all_selected = false;
    if (guider_ui.screen_alarm_remove_imgbtn_choice) {
        lv_imgbtn_set_state(guider_ui.screen_alarm_remove_imgbtn_choice, LV_IMGBTN_STATE_RELEASED);

        // 如果没有闹钟了，禁用全选按钮
        if (g_alarm_count == 0) {
            lv_obj_add_state(guider_ui.screen_alarm_remove_imgbtn_choice, LV_STATE_DISABLED);
            lv_obj_clear_flag(guider_ui.screen_alarm_remove_imgbtn_choice, LV_OBJ_FLAG_CLICKABLE);
        }
    }

    // 更新选择文本显示 - 添加安全检查
    if (guider_ui.screen_alarm_remove_choice_span) {
        lv_span_set_text(guider_ui.screen_alarm_remove_choice_span, "未选择");
    }

    return true;
}

// 初始化闹钟删除页面事件
void alarm_del_event_init(lv_ui *ui) {
    // 注册返回按钮点击事件
    lv_obj_add_event_cb(ui->screen_alarm_remove_return, alarm_del_return_event_handler, LV_EVENT_CLICKED, ui);
    
    // 注册全选按钮点击事件
    lv_obj_remove_event_cb(ui->screen_alarm_remove_imgbtn_choice, alarm_del_all_choice_event_handler);
    lv_obj_add_event_cb(ui->screen_alarm_remove_imgbtn_choice, alarm_del_all_choice_event_handler, LV_EVENT_CLICKED, ui);
    
    // 注册删除所选按钮点击事件
    // lv_obj_remove_event_cb(ui->screen_alarm_remove_imgbtn_del, alarm_del_remove_event_handler);
    // lv_obj_add_event_cb(ui->screen_alarm_remove_imgbtn_del, alarm_del_remove_event_handler, LV_EVENT_CLICKED, ui);
    
    // 注册删除图片的按下放大、松手恢复事件
    lv_obj_remove_event_cb(ui->screen_alarm_remove_img_del, alarm_remove_img_del_event_handler);
    lv_obj_add_event_cb(ui->screen_alarm_remove_img_del, alarm_remove_img_del_event_handler, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui->screen_alarm_remove_img_del, alarm_remove_img_del_event_handler, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(ui->screen_alarm_remove_img_del, alarm_remove_img_del_event_handler, LV_EVENT_PRESS_LOST, NULL);
    
    // 检查是否有闹钟，没有则禁用全选按钮
    if (g_alarm_count == 0) {
        // 同时设置禁用状态和移除可点击标志
        lv_obj_add_state(ui->screen_alarm_remove_imgbtn_choice, LV_STATE_DISABLED);
        lv_obj_clear_flag(ui->screen_alarm_remove_imgbtn_choice, LV_OBJ_FLAG_CLICKABLE);
        all_selected = false;
    } else {
        // 同时清除禁用状态和添加可点击标志
        lv_obj_clear_state(ui->screen_alarm_remove_imgbtn_choice, LV_STATE_DISABLED);
        lv_obj_add_flag(ui->screen_alarm_remove_imgbtn_choice, LV_OBJ_FLAG_CLICKABLE);
    }

    // 隐藏静态创建的闹钟容器（cont1、cont2、cont3）
    lv_obj_add_flag(ui->screen_alarm_remove_cont_1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->screen_alarm_remove_cont_2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->screen_alarm_remove_cont_3, LV_OBJ_FLAG_HIDDEN);
    
    // 设置选择文本的初始状态
    lv_span_set_text(ui->screen_alarm_remove_choice_span, "未选择");
    
    // 初始化时刷新闹钟列表，创建动态条目和按钮
    refresh_alarm_del_page(ui);
    clear_alarm_del_events_and_selection(ui);


}


void alarm_del_event_setup(void){
    alarm_del_event_init(&guider_ui);
}
