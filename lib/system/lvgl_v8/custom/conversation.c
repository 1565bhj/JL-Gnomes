/**
 * @file ui_action_ai_chat_scr.c
 * @brief AI聊天界面相关功能实现
 * @details 包含聊天界面创建、聊天气泡显示、头像设置等功能
 */


#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdbool.h"
#include "gui_guider.h"
#include "custom.h"


/**
 * @brief 聊天气泡类型枚举
 * @details 定义了聊天界面中两种不同类型的消息气泡
 */
typedef enum {
    CHAT_BUBBLE_INCOMING = 0,  // AI回复的消息（左侧显示）
    CHAT_BUBBLE_OUTGOING       // 用户发送的消息（右侧显示）
} chat_bubble_type_t;

/**
 * @brief 消息显示相关宏定义
 * @details 定义了聊天界面中各种UI元素的尺寸和间距
 */
#define LABLE_MAX_WIDTH 188     // 消息文本的最大宽度（根据屏幕尺寸调整）
#define LABLE_MAX_HIGHT 182     // 消息文本的最大高度（根据屏幕尺寸调整）
#define AVATAR_SIZE 30          // 头像大小（宽度和高度）
#define AVATAR_BUBBLE_GAP 10     // 头像和气泡之间的间距

/**
 * @brief 控制说话图标显示状态
 * @param icon_flag 图标状态标志：0表示显示说话中图标，1表示显示静音图标
 */
void speak_icon_show(uint8_t icon_flag);

/**
 * @brief 创建聊天气泡对象
 * @param parent 父容器对象
 * @param text 气泡中显示的文本内容
 * @param type 气泡类型（接收或发送）
 * @return 创建的气泡对象指针
 */
extern lv_obj_t *chat_bubble_create(lv_obj_t *parent, const char *text, chat_bubble_type_t type);

/**
 * @brief 统一的聊天对话函数
 * @details 根据发送者类型添加聊天气泡和头像
 *
 * @param chat_container 聊天容器
 * @param message 消息文本内容
 * @param type 发送者类型（CHAT_BUBBLE_INCOMING 表示AI，CHAT_BUBBLE_OUTGOING 表示用户）
 */
void add_chat_message(lv_obj_t *chat_container, const char *message, chat_bubble_type_t type);

/**
 * @brief 外部异步调用对话函数
 * @details 用于外部异步添加聊天消息，自动使用全局UI对象的聊天容器
 *
 * @param message 消息文本内容
 * @param type 发送者类型（true 表示AI，false 表示用户）
 */
void async_user_add_chat_message(const char *message);
void async_ai_add_chat_message(const char *message);
void async_speak_listen_icon_show(bool is_listen);

void chat_clear_btn_event_cb(void);
void load_chat_ai_scr(void);


// 屏幕加载标志
static uint8_t load_scr_flag = 0;

// 静态变量存储AI和用户的消息容器
static lv_obj_t *ai_message_row = NULL;
static lv_obj_t *user_message_row = NULL;
static lv_obj_t *ai_bubble = NULL;
static lv_obj_t *user_bubble = NULL;
static lv_obj_t *ai_label = NULL;
static lv_obj_t *user_label = NULL;

/**
 * @brief 初始化聊天气泡样式
 * @param style 样式结构体指针
 * @param bg_color 气泡背景颜色
 * @param text_color 气泡文本颜色
 * @param align 文本对齐方式
 */
static void chat_bubble_style_init(lv_style_t *style, lv_color_t bg_color, lv_color_t text_color,
                                   lv_align_t align)
{
    lv_style_init(style);

    // 基础样式设置：圆角和背景
    lv_style_set_radius(style, 12);      // 设置圆角半径为12像素
    lv_style_set_bg_color(style, bg_color);  // 设置背景颜色
    lv_style_set_bg_opa(style, LV_OPA_20);  // 设置背景不透明度为100%

    // 内边距设置：气泡内文本与边框的距离
    lv_style_set_pad_top(style, 5);      // 顶部内边距
    lv_style_set_pad_bottom(style, 5);   // 底部内边距
    lv_style_set_pad_left(style, 10);    // 左侧内边距
    lv_style_set_pad_right(style, 10);   // 右侧内边距

    // 文本样式设置
    lv_style_set_text_color(style, text_color);  // 设置文本颜色
    lv_style_set_text_font(style, &lv_font_MiSansDemibold_18);  // 设置默认字体
    lv_style_set_text_align(style, align);  // 设置文本对齐方式

    // 最大宽度限制：限制气泡最大宽度为屏幕宽度的3/4
    lv_style_set_max_width(style, lv_disp_get_hor_res(NULL) * 3 / 4);
    // 最大高度限制：限制气泡最大高度为固定值
    lv_style_set_max_height(style, LABLE_MAX_HIGHT);
}

/**
 * @brief 创建聊天气泡对象
 * @param parent 父容器对象
 * @param text 气泡中显示的文本内容
 * @param type 气泡类型（接收或发送）
 * @return 创建的气泡对象指针
 */
lv_obj_t *chat_bubble_create(lv_obj_t *parent, const char *text, chat_bubble_type_t type)
{
    // 参数有效性检查
    if (!text) {
        return NULL;  // 如果文本为空，直接返回
    }

    // 创建气泡容器对象
    lv_obj_t *bubble = lv_obj_create(parent);
    lv_obj_set_width(bubble, LV_SIZE_CONTENT);  // 宽度自适应内容
    lv_obj_set_height(bubble, LV_SIZE_CONTENT); // 高度自适应内容

    // lv_obj_remove_style_all(bubble); // 移除默认样式，避免继承父容器样式
    // lv_obj_set_style_bg_opa(bubble, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(bubble, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    // 样式定义与应用：根据气泡类型应用不同样式
    static lv_style_t incoming_style, outgoing_style;  // 静态样式，避免重复创建
    if (type == CHAT_BUBBLE_INCOMING) { // 接收消息气泡（AI消息）
        chat_bubble_style_init(&incoming_style, lv_color_hex(0xFFFFFF),
                               lv_color_hex(0xFFFFFF), LV_TEXT_ALIGN_LEFT);
        lv_obj_add_style(bubble, &incoming_style, LV_PART_MAIN);
    } else {  // 发送消息气泡（用户消息）
        chat_bubble_style_init(&outgoing_style, lv_color_hex(0xFFFFFF),
                               lv_color_hex(0xFFFFFF), LV_TEXT_ALIGN_LEFT);
        lv_obj_add_style(bubble, &outgoing_style, LV_PART_MAIN);
    }

    // 添加文本标签到气泡容器
    lv_obj_t *label = lv_label_create(bubble);
    lv_label_set_text(label, text);  // 设置标签文本

    // 设置字体为自定义25号字体
    lv_obj_set_style_text_font(label, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 文本自动换行+高度自适应设置
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);  // 启用长文本自动换行

    // 限制标签宽度，确保文本在超过最大宽度时换行
    lv_obj_set_width(label, LABLE_MAX_WIDTH);

    // 高度自适应内容
    lv_obj_set_height(label, LV_SIZE_CONTENT);

    // 确保气泡容器大小适应标签大小
    lv_obj_set_width(bubble, LV_SIZE_CONTENT);
    lv_obj_set_height(bubble, LV_SIZE_CONTENT);
    // 启用滚动功能
    lv_obj_set_scrollbar_mode(bubble, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_scroll_dir(bubble, LV_DIR_VER);
    return bubble;
}


/**
 * @brief 创建聊天界面
 * @details 初始化聊天滚动容器、设置布局和样式，并添加示例消息
 */
void chat_interface_create(void)
{
    // 使用现有滚动容器：从UI对象中获取聊天框架容器
    lv_obj_t *chat_scroll = guider_ui.screen_conversation_cont_frame;

    // 设置滚动容器样式
    static lv_style_t scroll_style;  // 静态样式，避免重复创建
    lv_style_init(&scroll_style);    // 初始化样式

    // 设置布局为列布局，让消息垂直排列
    lv_style_set_layout(&scroll_style, LV_LAYOUT_FLEX);  // 设置为弹性布局
    lv_style_set_flex_flow(&scroll_style, LV_FLEX_FLOW_COLUMN);  // 设置为列布局
    lv_style_set_flex_main_place(&scroll_style, LV_FLEX_ALIGN_START);  // 设置主轴对齐方式为顶部
    // lv_style_set_flex_align(&scroll_style, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);  // 设置对齐方式

    // 设置内边距：容器与边界的距离
    lv_obj_set_style_pad_top(chat_scroll, 20, 0);     // 顶部内边距
    lv_obj_set_style_pad_bottom(chat_scroll, 20, 0);  // 底部内边距
    // lv_obj_set_style_pad_left(chat_scroll, 20, 0);    // 左侧内边距
    // lv_obj_set_style_pad_right(chat_scroll, 20, 0);   // 右侧内边距

    // 设置间距：消息之间的间距
    lv_obj_set_style_pad_row(chat_scroll, -10, 0);      // 行间距（垂直方向）
    // lv_obj_set_style_pad_column(chat_scroll, 20, 0);   // 列间距（水平方向）


    // 应用样式到滚动容器
    lv_obj_add_style(chat_scroll, &scroll_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(chat_scroll, LV_OBJ_FLAG_EVENT_BUBBLE);

    // 禁用滚动功能
    lv_obj_set_scrollbar_mode(chat_scroll, LV_SCROLLBAR_MODE_OFF);


//
//    // 添加示例消息
//    add_chat_message(chat_scroll, "你好，我是AI助手", CHAT_BUBBLE_INCOMING);  // 添加AI助手消息
//    add_chat_message(chat_scroll, "你好，我是用户", CHAT_BUBBLE_OUTGOING);   // 添加用户消息
//    add_chat_message(chat_scroll, "我想要你去带我出去玩，出去玩，给我介绍一下哪里比较好玩，好玩好玩我想要你去带我出去玩，出去玩，给我介绍一下哪里比较好玩，好玩好玩", CHAT_BUBBLE_OUTGOING);   // 添加用户消息
//    add_chat_message(chat_scroll, "你好，我是AI助手", CHAT_BUBBLE_INCOMING);  // 添加AI助手消息
}


/**
 * @brief 统一聊天对话函数，用于添加一条聊天消息
 * @param chat_container 聊天容器对象
 * @param message 消息文本内容
 * @param type 消息类型（接收或发送）
 */
void add_chat_message(lv_obj_t *chat_container, const char *message, chat_bubble_type_t type)
{
    // 参数有效性检查
    if (!chat_container || !message) {
        return;  // 如果容器或消息为空，直接返回
    }

    bool needs_layout_update = true;

    if (type == CHAT_BUBBLE_INCOMING) { // AI 消息（左侧显示）
        if (user_message_row) {
            // 清空用户消息文本
            if (user_label) {
                lv_label_set_text(user_label, "");
            }
            lv_obj_add_flag(user_message_row, LV_OBJ_FLAG_HIDDEN);
        }

        // 如果AI消息容器不存在，创建它
        if (!ai_message_row) {
            // 创建消息行容器：用于容纳头像和气泡
            ai_message_row = lv_obj_create(chat_container);
            lv_obj_set_width(ai_message_row, lv_obj_get_width(chat_container));  // 宽度占满父容器
            lv_obj_set_height(ai_message_row, LV_SIZE_CONTENT);  // 高度自适应内容

            // 设置样式
            lv_obj_set_style_bg_opa(ai_message_row, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(ai_message_row, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

            // 设置为行布局
            lv_obj_set_flex_flow(ai_message_row, LV_FLEX_FLOW_ROW);  // 设置为行布局
            lv_obj_set_style_pad_left(ai_message_row, 21, 0);    // 左侧内边距
            lv_obj_set_style_pad_right(ai_message_row, 21, 0);   // 右侧内边距
            lv_obj_set_flex_align(ai_message_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

            // AI 头像设置
            lv_obj_t *ai_avatar = lv_img_create(ai_message_row);
            lv_img_set_src(ai_avatar, &_ai_alpha_30x30);  // 设置AI头像图片
            lv_obj_set_size(ai_avatar, AVATAR_SIZE, AVATAR_SIZE);  // 设置头像大小

            // AI 气泡设置
            ai_bubble = chat_bubble_create(ai_message_row, message, type);
            lv_obj_set_style_pad_left(ai_bubble, AVATAR_BUBBLE_GAP, 0);  // 设置头像与气泡的间距

            // 获取气泡中的标签对象
            ai_label = lv_obj_get_child(ai_bubble, 0);

            lv_obj_add_flag(ai_message_row, LV_OBJ_FLAG_EVENT_BUBBLE);
            // 确保容器不受父容器布局影响
            lv_obj_clear_flag(ai_message_row, LV_OBJ_FLAG_LAYOUT_1);
            lv_obj_add_flag(ai_message_row, LV_OBJ_FLAG_IGNORE_LAYOUT);
        } else {
            // 如果AI消息容器已存在，更新文本
            if (ai_label) {
                // 追加文本到末尾
                lv_label_ins_text(ai_label, LV_LABEL_POS_LAST, message);
                // lv_label_set_text(ai_label, message);
                // 重新计算容器大小
                lv_obj_set_height(ai_bubble, LV_SIZE_CONTENT);
                lv_obj_set_height(ai_message_row, LV_SIZE_CONTENT);
            }
        }

        // 设置消息容器位置
        lv_obj_set_pos(ai_message_row, 0, -30);

        // 显示AI消息容器
        lv_obj_clear_flag(ai_message_row, LV_OBJ_FLAG_HIDDEN);
        // 确保消息容器显示在最前面
        lv_obj_move_foreground(ai_message_row);

    } else { // 用户消息（右侧显示）
        // 隐藏AI消息容器
        if (ai_message_row) {
            // 清空AI消息文本
            if (ai_label) {
                lv_label_set_text(ai_label, "");
            }
            lv_obj_add_flag(ai_message_row, LV_OBJ_FLAG_HIDDEN);
        }

        // 如果用户消息容器不存在，创建它
        if (!user_message_row) {
            // 创建消息行容器：用于容纳头像和气泡
            user_message_row = lv_obj_create(chat_container);
            lv_obj_set_width(user_message_row, lv_obj_get_width(chat_container));  // 宽度占满父容器
            lv_obj_set_height(user_message_row, LV_SIZE_CONTENT);  // 高度自适应内容

            // 设置样式
            lv_obj_set_style_bg_opa(user_message_row, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(user_message_row, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

            // 设置为行布局
            lv_obj_set_flex_flow(user_message_row, LV_FLEX_FLOW_ROW);  // 设置为行布局
            lv_obj_set_style_pad_left(user_message_row, 21, 0);    // 左侧内边距
            lv_obj_set_style_pad_right(user_message_row, 21, 0);   // 右侧内边距
            lv_obj_set_flex_align(user_message_row, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

            // 用户气泡设置
            user_bubble = chat_bubble_create(user_message_row, message, type);
            lv_obj_set_style_pad_right(user_bubble, AVATAR_BUBBLE_GAP, 0);  // 设置气泡与头像的间距

            // 用户头像设置
            lv_obj_t *user_avatar = lv_img_create(user_message_row);
            lv_img_set_src(user_avatar, &_user_alpha_30x30);  // 设置用户头像图片
            lv_obj_set_size(user_avatar, AVATAR_SIZE, AVATAR_SIZE);  // 设置头像大小

            // 获取气泡中的标签对象
            user_label = lv_obj_get_child(user_bubble, 0);

            lv_obj_add_flag(user_message_row, LV_OBJ_FLAG_EVENT_BUBBLE);
            // 确保容器不受父容器布局影响
            lv_obj_clear_flag(user_message_row, LV_OBJ_FLAG_LAYOUT_1);
            lv_obj_add_flag(user_message_row, LV_OBJ_FLAG_IGNORE_LAYOUT);
        } else {
            // 如果用户消息容器已存在，更新文本
            if (user_label) {
                // 追加文本到末尾
                lv_label_ins_text(user_label, LV_LABEL_POS_LAST, message);
                // lv_label_set_text(user_label, message);
                // 重新计算容器大小
                lv_obj_set_height(user_bubble, LV_SIZE_CONTENT);
                lv_obj_set_height(user_message_row, LV_SIZE_CONTENT);
            }
        }

        // 设置消息容器位置
        lv_obj_set_pos(user_message_row, 0, -30);

        // 显示用户消息容器
        lv_obj_clear_flag(user_message_row, LV_OBJ_FLAG_HIDDEN);
        // 确保消息容器显示在最前面
        lv_obj_move_foreground(user_message_row);

    }

    // 如果需要更新布局和滚动
    if (needs_layout_update && chat_container) {
        if (type == CHAT_BUBBLE_INCOMING && ai_label && ai_bubble) {
            lv_obj_update_layout(ai_label); // 更新布局以确保计算了正确的滚动尺寸
            lv_obj_update_layout(ai_bubble); // 更新布局以确保计算了正确的滚动尺寸

            // 滚动到底部，保留5像素底部边距
            lv_coord_t scroll_bottom = lv_obj_get_scroll_bottom(ai_bubble);
            lv_coord_t scroll_offset = scroll_bottom + 5;
            if (scroll_offset > 0) {
                lv_obj_scroll_by(ai_bubble, 0, -scroll_offset, LV_ANIM_ON);
            }
        } else if (type == CHAT_BUBBLE_OUTGOING && user_label && user_bubble) {
            lv_obj_update_layout(user_label); // 更新布局以确保计算了正确的滚动尺寸
            lv_obj_update_layout(user_bubble); // 更新布局以确保计算了正确的滚动尺寸

            // 滚动到底部，保留5像素底部边距
            lv_coord_t scroll_bottom = lv_obj_get_scroll_bottom(user_bubble);
            lv_coord_t scroll_offset = scroll_bottom + 5;
            if (scroll_offset > 0) {
                lv_obj_scroll_by(user_bubble, 0, -scroll_offset, LV_ANIM_ON);
            }
        }
    }
}

/**
 * @brief 外部异步调用对话函数实现
 * @details 用于外部异步添加聊天消息，自动使用全局UI对象的聊天容器

 */
void user_add_chat_message_cb(void *message)
{
    const char *message_str = (const char *)message;
    if (!message_str || !guider_ui.screen_conversation_cont_frame) {
        return;
    }

    add_chat_message(guider_ui.screen_conversation_cont_frame, message_str, CHAT_BUBBLE_OUTGOING);

    // 释放复制的字符串
    free((void*)message_str);
}

void async_user_add_chat_message(const char *message)
{
    if (!message) {
        return;
    }

    // 复制字符串以确保异步调用时仍然有效
    char *message_copy = strdup(message);
    if (message_copy) {
        lv_async_call(user_add_chat_message_cb, (void*)message_copy);
    }
}

/**
 * @brief 外部异步调用对话函数实现
 * @details 用于外部异步添加聊天消息，自动使用全局UI对象的聊天容器

 */
void ai_add_chat_message_cb(void *message)
{
    const char *message_str = (const char *)message;
    if (!message_str || !guider_ui.screen_conversation_cont_frame) {
        return;
    }

    add_chat_message(guider_ui.screen_conversation_cont_frame, message_str, CHAT_BUBBLE_INCOMING);

    // 释放复制的字符串
    free((void*)message_str);
}

void async_ai_add_chat_message(const char *message)
{
    if (!message) {
        return;
    }

    // 复制字符串以确保异步调用时仍然有效
    char *message_copy = strdup(message);
    if (message_copy) {
        lv_async_call(ai_add_chat_message_cb, (void*)message_copy);
    }
}
/**
 * @brief 控制说话图标显示状态
 * @param icon_flag 图标状态标志：0表示显示说话中图标，1表示显示聆听图标
 */
static void speak_listen_icon_show(void *is_listen)
{
    bool icon_flag = (bool)(intptr_t)is_listen;
    if (icon_flag == 0) {
        lv_span_set_text(guider_ui.screen_conversation_span_listen_speak_span, "正在对话……");

    } else if (icon_flag == 1) {
        lv_span_set_text(guider_ui.screen_conversation_span_listen_speak_span, "正在聆听……");

    }

}
void async_speak_listen_icon_show(bool is_listen)
{
    lv_async_call(speak_listen_icon_show, (void*)is_listen);
}

void unload_chat_ai_scr(void)
{
    lv_scr_load(guider_ui.screen_option);
    lv_obj_del(guider_ui.screen_conversation);

}
void load_chat_ai_scr(void)
{
    chat_interface_create();
    lv_obj_add_flag(guider_ui.screen_conversation_cont_frame, LV_OBJ_FLAG_EVENT_BUBBLE);
}



void conversation_event_init(lv_ui *ui)
{
    if (ui == NULL) {
        return;
    }
    load_chat_ai_scr();
    // 隐藏静态创建的容器
    lv_obj_add_flag(ui->screen_conversation_cont_ai, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->screen_conversation_cont_user, LV_OBJ_FLAG_HIDDEN);


}

void conversation_event_setup(void)
{
    conversation_event_init(&guider_ui);
}



