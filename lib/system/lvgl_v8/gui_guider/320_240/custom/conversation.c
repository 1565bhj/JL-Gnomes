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
extern lv_obj_t * chat_bubble_create(lv_obj_t *parent, const char *text, chat_bubble_type_t type);

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


/**
 * @brief 初始化聊天气泡样式
 * @param style 样式结构体指针
 * @param bg_color 气泡背景颜色
 * @param text_color 气泡文本颜色
 * @param align 文本对齐方式
 */
static void chat_bubble_style_init(lv_style_t *style, lv_color_t bg_color, lv_color_t text_color,
                                 lv_align_t align) {
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
}

/**
 * @brief 创建聊天气泡对象
 * @param parent 父容器对象
 * @param text 气泡中显示的文本内容
 * @param type 气泡类型（接收或发送）
 * @return 创建的气泡对象指针
 */
lv_obj_t * chat_bubble_create(lv_obj_t *parent, const char *text, chat_bubble_type_t type) {
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
    lv_obj_set_style_border_opa(bubble, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
    // 样式定义与应用：根据气泡类型应用不同样式
    static lv_style_t incoming_style, outgoing_style;  // 静态样式，避免重复创建
    if(type == CHAT_BUBBLE_INCOMING) {  // 接收消息气泡（AI消息）
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
    lv_obj_set_style_text_font(label, &lv_font_MiSansDemibold_18, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 文本自动换行+高度自适应设置
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);  // 启用长文本自动换行

    // 计算文本宽度：用于确定是否需要限制标签宽度
    uint32_t text_len = strlen(text);       // 文本长度
    lv_coord_t letter_space = 0;           // 字符间距
    lv_text_flag_t flag = LV_TEXT_FLAG_NONE;  // 文本标志
    lv_coord_t text_width = lv_txt_get_width(
        text,
        text_len,
        &lv_font_MiSansDemibold_18,
        letter_space,
        flag
    );  // 获取文本显示宽度

    // 根据文本长度设置标签宽度：超过最大宽度时限制宽度
    if(text_width > LABLE_MAX_WIDTH) {
        lv_obj_set_width(label, LABLE_MAX_WIDTH);
    }

    // 高度自适应内容
    lv_obj_set_height(label, LV_SIZE_CONTENT);

    return bubble;
}


/**
 * @brief 创建聊天界面
 * @details 初始化聊天滚动容器、设置布局和样式，并添加示例消息
 */
void chat_interface_create(void) {
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

    // 启用滚动（当前注释掉）
    // lv_obj_set_scrollbar_mode(chat_scroll, LV_SCROLLBAR_MODE_AUTO);



    // 添加示例消息
    add_chat_message(chat_scroll, "你好，我是AI助手", CHAT_BUBBLE_INCOMING);  // 添加AI助手消息
    add_chat_message(chat_scroll, "你好，我是用户", CHAT_BUBBLE_OUTGOING);   // 添加用户消息
    add_chat_message(chat_scroll, "我想要你去带我出去玩，出去玩，给我介绍一下哪里比较好玩，好玩好玩我想要你去带我出去玩，出去玩，给我介绍一下哪里比较好玩，好玩好玩", CHAT_BUBBLE_OUTGOING);   // 添加用户消息
    add_chat_message(chat_scroll, "你好，我是AI助手", CHAT_BUBBLE_INCOMING);  // 添加AI助手消息
}


/**
 * @brief 统一聊天对话函数，用于添加一条聊天消息
 * @param chat_container 聊天容器对象
 * @param message 消息文本内容
 * @param type 消息类型（接收或发送）
 */
void add_chat_message(lv_obj_t *chat_container, const char *message, chat_bubble_type_t type) {
    // 参数有效性检查
    if (!chat_container || !message) {
        return;  // 如果容器或消息为空，直接返回
    }

    // 创建消息行容器：用于容纳头像和气泡
    lv_obj_t *message_row = lv_obj_create(chat_container);
    lv_obj_set_width(message_row, lv_obj_get_width(chat_container));  // 宽度占满父容器
    lv_obj_set_height(message_row, LV_SIZE_CONTENT);  // 高度自适应内容

     //Write style for photo_scr_photo_page_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(message_row, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(message_row, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // lv_obj_remove_style_all(message_row);  // 移除默认样式
    lv_obj_set_flex_flow(message_row, LV_FLEX_FLOW_ROW);  // 设置为行布局
    lv_obj_set_style_pad_left(message_row, 21, 0);    // 左侧内边距
    lv_obj_set_style_pad_right(message_row, 21, 0);   // 右侧内边距


    if (type == CHAT_BUBBLE_INCOMING) { // AI 消息（左侧显示）
        lv_obj_set_flex_align(message_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        // lv_obj_set_flex_align(message_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);  // 设置对齐方式
        // AI 头像设置
        lv_obj_t *ai_avatar = lv_img_create(message_row);
        lv_img_set_src(ai_avatar, &_ai_alpha_30x30);  // 设置AI头像图片
        lv_obj_set_size(ai_avatar, AVATAR_SIZE, AVATAR_SIZE);  // 设置头像大小

        // AI 气泡设置
        lv_obj_t *ai_bubble = chat_bubble_create(message_row, message, type);
        lv_obj_set_style_pad_left(ai_bubble, AVATAR_BUBBLE_GAP, 0);  // 设置头像与气泡的间距


    } else { // 用户消息（右侧显示）
        // 用户消息右对齐布局设置
        lv_obj_set_flex_align(message_row, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        // 用户气泡设置
        lv_obj_t *user_bubble = chat_bubble_create(message_row, message, type);
        lv_obj_set_style_pad_right(user_bubble, AVATAR_BUBBLE_GAP, 0);  // 设置气泡与头像的间距

        // 用户头像设置
        lv_obj_t *user_avatar = lv_img_create(message_row);
        lv_img_set_src(user_avatar, &_user_alpha_30x30);  // 设置用户头像图片
        lv_obj_set_size(user_avatar, AVATAR_SIZE, AVATAR_SIZE);  // 设置头像大小

    }
    lv_obj_add_flag(message_row, LV_OBJ_FLAG_EVENT_BUBBLE);
    // 消息行间距设置：设置当前消息行与上一条消息的垂直间距
    lv_obj_set_style_pad_top(message_row, 5, 0);

    // 滚动到底部：确保最新消息可见，启用动画效果
    lv_obj_scroll_to_view(message_row, LV_ANIM_ON);
}

/**
 * @brief 外部异步调用对话函数实现
 * @details 用于外部异步添加聊天消息，自动使用全局UI对象的聊天容器

 */
void user_add_chat_message_cb(void *message) {
    const char *message_str = (const char *)message;
    if (!message_str || !guider_ui.screen_conversation_cont_frame) {
        return;
    }

    add_chat_message(guider_ui.screen_conversation_cont_frame, message_str, CHAT_BUBBLE_OUTGOING);

    // 释放复制的字符串
    free((void*)message_str);
}

void async_user_add_chat_message(const char *message) {
    if (!message) return;

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
void ai_add_chat_message_cb(void *message) {
    const char *message_str = (const char *)message;
    if (!message_str || !guider_ui.screen_conversation_cont_frame) {
        return;
    }

    add_chat_message(guider_ui.screen_conversation_cont_frame, message_str, CHAT_BUBBLE_INCOMING);

    // 释放复制的字符串
    free((void*)message_str);
}

void async_ai_add_chat_message(const char *message) {
    if (!message) return;

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
    if(icon_flag == 0)
    {
        lv_span_set_text(guider_ui.screen_conversation_span_listen_speak_span, "正在对话……");

    } else if(icon_flag == 1) {
        lv_span_set_text(guider_ui.screen_conversation_span_listen_speak_span, "正在聆听……");

    }

}
void async_speak_listen_icon_show(bool is_listen) {
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



void conversation_event_init(lv_ui *ui) {
    if (ui == NULL) {
        return;
    }
    load_chat_ai_scr();
    // 隐藏静态创建的容器
    lv_obj_add_flag(ui->screen_conversation_cont_ai, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->screen_conversation_cont_user, LV_OBJ_FLAG_HIDDEN);


}

void conversation_event_setup(void) {
    conversation_event_init(&guider_ui);
}




