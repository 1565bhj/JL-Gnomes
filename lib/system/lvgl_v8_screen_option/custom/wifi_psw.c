#include "lvgl.h"
#include "lvgl_develop.h"
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"
#include "wifi_common.h"
#include <stdio.h>

#ifdef APPLAYER_ENABLE
extern int music_play_res_file(const char *name);
#endif

#if CONFIG_LVGL_OPTION_WIFI_ENABLE
/**
 * @brief 异步更新WiFi名称显示的回调函数
 */
static void update_wifi_name_async(void *param)
{
    lv_ui *ui = (lv_ui *)param;

    // 更新WiFi名称显示
    if (current_wifi_name[0] != '\0') {
        lv_span_set_text(ui->screen_wifi_psw_span_wifi_name_span, current_wifi_name);
    } else {
        // 如果current_wifi_name为空，使用默认名称
        lv_span_set_text(ui->screen_wifi_psw_span_wifi_name_span, "手动输入WIFI");
    }
}

/**
 * @brief 异步更新WiFi名称显示
 */
void update_wifi_name_display_async(void)
{
    lv_async_call(update_wifi_name_async, &guider_ui);
}

/**
 * @brief 返回按钮点击事件处理函数
 */
static void screen_wifi_psw_img_return_event_handler(lv_event_t *e)
{
    // 切换到wifi_show页面
    switch_to_wifi_show_page();
}

#if CONFIG_LVGL_WIFI_PASSWORD_MASK_ENABLE && CONFIG_LVGL_WIFI_PASSWORD_IMGBTN_ENABLE
/**
 * @brief 密码显隐 imgbtn：close（未勾选）= 替换符；open（勾选）= 明文
 */
static void screen_wifi_psw_imgbtn_psw_event_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) {
        return;
    }
    lv_obj_t *btn = lv_event_get_target(e);
    lv_ui *ui = lv_event_get_user_data(e);
    bool open_show_plain = lv_obj_has_state(btn, LV_STATE_CHECKED);
    lv_textarea_set_password_mode(ui->screen_wifi_psw_ta_input, !open_show_plain);
    if (!open_show_plain) {
        lv_textarea_set_password_bullet(ui->screen_wifi_psw_ta_input, "*");
    }
}
#endif

/**
 * @brief 输入框事件处理函数
 */
static void screen_wifi_psw_ta_input_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_ui *ui = lv_event_get_user_data(e);

    if (code == LV_EVENT_FOCUSED) {
        // 输入框获得焦点，隐藏提示文本，显示删除和连接按钮
        lv_obj_add_flag(ui->screen_wifi_psw_span_tip, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui->screen_wifi_psw_btn_del, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui->screen_wifi_psw_btn_connect, LV_OBJ_FLAG_HIDDEN);
    } else if (code == LV_EVENT_DEFOCUSED) {
        // 输入框失去焦点，显示提示文本，隐藏删除和连接按钮
        lv_obj_clear_flag(ui->screen_wifi_psw_span_tip, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui->screen_wifi_psw_btn_del, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui->screen_wifi_psw_btn_connect, LV_OBJ_FLAG_HIDDEN);
    } else if (code == LV_EVENT_VALUE_CHANGED) {
        // 输入框值变化，检查密码长度
        const char *password = lv_textarea_get_text(ui->screen_wifi_psw_ta_input);
        int password_len = strlen(password);

        if (password_len >= 8) {
            // 密码长度大于等于8位，设置按钮为可点击状态，字体颜色改为#2195f6
            lv_obj_clear_state(ui->screen_wifi_psw_btn_connect, LV_STATE_DISABLED);
            lv_obj_set_style_text_color(ui->screen_wifi_psw_btn_connect, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
        } else {
            // 密码长度小于8位，设置按钮为不可点击状态，恢复默认字体颜色
            lv_obj_add_state(ui->screen_wifi_psw_btn_connect, LV_STATE_DISABLED);
            lv_obj_set_style_text_color(ui->screen_wifi_psw_btn_connect, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
}

/**
 * @brief 取消按钮点击事件处理函数
 */
static void screen_wifi_psw_btn_del_event_handler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_ui *ui = lv_event_get_user_data(e);

    // 清空密码输入框
    lv_textarea_set_text(ui->screen_wifi_psw_ta_input, "");
}

/**
 * @brief 错误容器确认按钮点击事件处理函数
 */
static void screen_wifi_psw_btn_know_event_handler(lv_event_t *e)
{
    lv_ui *ui = lv_event_get_user_data(e);
    lv_obj_add_flag(ui->screen_wifi_psw_cont_err, LV_OBJ_FLAG_HIDDEN); // 隐藏错误容器
}


/**
 * @brief 密码验证成功后的异步处理回调函数
 */
static void handle_password_success_async(void *param)
{
    lv_ui *ui = (lv_ui *)param;

    // 密码正确，修改span_wifi_success和span_wifi_name显示的名称
    lv_span_set_text(ui->screen_wifi_show_span_wifi_success_span, current_wifi_name);
    lv_span_set_text(ui->screen_wifi_show_span_wifi_name_span, current_wifi_name);
    // 恢复原来的情况：显示已连接的 WiFi 容器
    lv_obj_clear_flag(ui->screen_wifi_show_cont_wifi, LV_OBJ_FLAG_HIDDEN);
    // 设置 WiFi 未连接容器的位置、大小和滚动条
    set_wifi_disconnected_container(ui->screen_wifi_show_cont_wifi_disconnected, false);

    // 切换到wifi_show页面
    switch_to_wifi_show_page();
#ifdef APPLAYER_ENABLE
    /* 不再用 keyworld=2 触发 NET_EVENT 里的 WifiConSucc，在此补播与 ai_speaker 一致 */
    music_play_res_file("WifiConSucc.mp3");
    extern int http_wifi_device_bind_app_task(char *phone);
    if (user_data_account_bound_read() != 1) {
        int ret = http_wifi_device_bind_app_task("12345678910");
        printf("http_wifi_device_bind_app_task = %d\n", ret);
    }

#endif
    // 隐藏错误容器和连接中容器
    lv_obj_add_flag(ui->screen_wifi_psw_cont_err, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->screen_wifi_psw_cont_connecting, LV_OBJ_FLAG_HIDDEN);
    // 清空密码输入框
    lv_textarea_set_text(ui->screen_wifi_psw_ta_input, "");

}

/**
 * @brief 连接中容器点击事件处理函数
 */
static void screen_wifi_psw_cont_connecting_event_handler(lv_event_t *e)
{
    lv_ui *ui = lv_event_get_user_data(e);

}

/**
 * @brief 连接状态检查的回调函数
 */
static void check_connect_status_cb(lv_timer_t *timer)
{
    lv_ui *ui = (lv_ui *)timer->user_data;
    static int check_count = 0;
    const int MAX_CHECK_COUNT = 30; // 最多检查30次，约30秒

    // 验证连接是否成功
    enum wifi_sta_connect_state connect_state = wifi_get_sta_connect_state();

    if (connect_state == WIFI_STA_CONNECT_SUCC || connect_state == WIFI_STA_NETWORK_STACK_DHCP_SUCC) {
        // 连接成功，停止定时器
        lv_timer_del(timer);
        check_count = 0;
        /* 连接成功时勿设为 2：唤醒逻辑里 keyworld_wifi_enter_congfig 非 0 且非 3
         * 会走「配网」提示音分支，导致已联网仍像配网、无法进对话。
         * 置 0 清除配网态；NET_EVENT_CONNECTED 仍会完成后续清理。 */
        set_keyworld_wifi_enter_config(0);
        // 使用异步调用处理密码验证成功后的操作
        lv_async_call(handle_password_success_async, ui);
    } else if (connect_state == WIFI_STA_CONNECT_ASSOCIAT_FAIL) {
        // 密码错误，停止定时器
        lv_timer_del(timer);
        check_count = 0;
        // 显示错误容器
        lv_obj_clear_flag(ui->screen_wifi_psw_cont_err, LV_OBJ_FLAG_HIDDEN);
        lv_span_set_text(ui->screen_wifi_psw_span_tip_err_span, "密码错误，连接失败。");
        lv_obj_set_pos(ui->screen_wifi_psw_cont_err, 0, 0);
        // 隐藏连接中容器
        lv_obj_add_flag(ui->screen_wifi_psw_cont_connecting, LV_OBJ_FLAG_HIDDEN);
        // 清空密码输入框
        lv_textarea_set_text(ui->screen_wifi_psw_ta_input, "");
    } else if (connect_state == WIFI_STA_CONNECT_ASSOCIAT_TIMEOUT ||
               connect_state == WIFI_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID ||
               connect_state == WIFI_STA_NETWORK_STACK_DHCP_TIMEOUT) {
        // 连接超时或未找到SSID，停止定时器
        lv_timer_del(timer);
        check_count = 0;
        // 显示错误容器
        lv_obj_clear_flag(ui->screen_wifi_psw_cont_err, LV_OBJ_FLAG_HIDDEN);
        if (connect_state == WIFI_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID) {
            lv_span_set_text(ui->screen_wifi_psw_span_tip_err_span, "未找到WiFi网络，连接失败。");
        } else if (connect_state == WIFI_STA_NETWORK_STACK_DHCP_TIMEOUT) {
            lv_span_set_text(ui->screen_wifi_psw_span_tip_err_span, "获取IP地址超时，连接失败。");
        } else {
            lv_span_set_text(ui->screen_wifi_psw_span_tip_err_span, "连接超时，连接失败。");
        }
        lv_obj_set_pos(ui->screen_wifi_psw_cont_err, 0, 0);
        // 隐藏连接中容器
        lv_obj_add_flag(ui->screen_wifi_psw_cont_connecting, LV_OBJ_FLAG_HIDDEN);
        // 清空密码输入框
        lv_textarea_set_text(ui->screen_wifi_psw_ta_input, "");
    } else if (check_count >= MAX_CHECK_COUNT) {
        // 超过最大检查次数，停止定时器
        lv_timer_del(timer);
        check_count = 0;
        // 显示错误容器
        lv_obj_clear_flag(ui->screen_wifi_psw_cont_err, LV_OBJ_FLAG_HIDDEN);
        lv_span_set_text(ui->screen_wifi_psw_span_tip_err_span, "连接超时，连接失败。");
        lv_obj_set_pos(ui->screen_wifi_psw_cont_err, 0, 0);
        // 隐藏连接中容器
        lv_obj_add_flag(ui->screen_wifi_psw_cont_connecting, LV_OBJ_FLAG_HIDDEN);
        // 清空密码输入框
        lv_textarea_set_text(ui->screen_wifi_psw_ta_input, "");
    }

    check_count++;
}

/**
 * @brief 连接按钮点击事件处理函数
 */
static void screen_wifi_psw_btn_connect_event_handler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_ui *ui = lv_event_get_user_data(e);
    // 输入框获得焦点，隐藏提示文本，显示删除和连接按钮
    lv_obj_add_flag(ui->screen_wifi_psw_span_tip, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui->screen_wifi_psw_btn_del, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui->screen_wifi_psw_btn_connect, LV_OBJ_FLAG_HIDDEN);
    // 显示连接中容器
    lv_obj_clear_flag(ui->screen_wifi_psw_cont_connecting, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(ui->screen_wifi_psw_cont_connecting, 0, 0);

    // 获取输入的密码
    const char *input_password = lv_textarea_get_text(ui->screen_wifi_psw_ta_input);
    // // 调用wifi_enter_sta_mode函数进行实际连接
    // wifi_enter_sta_mode((char *)current_wifi_name, (char *)input_password);
    /* 须走 wifi_sta_connect(...,1)：在 DHCP 成功后由 wifi_app_task 调用 wifi_store_mode_info 落盘 */
    wifi_sta_connect((char *)current_wifi_name, (char *)input_password, 1);
    // 创建定时器，每秒检查一次连接状态
    lv_timer_create(check_connect_status_cb, 1000, ui);
}

/**
 * @brief 初始化wifi密码屏幕的事件处理
 */
void wifi_psw_event_init(lv_ui *ui)
{
    /* 密码输入：白字（与 Guider 一致；重新生成界面后仍生效） */
    lv_obj_set_style_text_color(ui->screen_wifi_psw_ta_input, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_wifi_psw_ta_input, lv_color_hex(0x2d2d2d), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_wifi_psw_ta_input, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 更新WiFi名称显示
    update_wifi_name_display_async();
    // 隐藏错误容器和连接中容器
    lv_obj_add_flag(ui->screen_wifi_psw_cont_err, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->screen_wifi_psw_cont_connecting, LV_OBJ_FLAG_HIDDEN);
    // 清空密码输入框
    lv_textarea_set_text(ui->screen_wifi_psw_ta_input, "");
    // 初始化按钮状态：隐藏删除和连接按钮
    lv_obj_add_flag(ui->screen_wifi_psw_btn_del, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->screen_wifi_psw_btn_connect, LV_OBJ_FLAG_HIDDEN);
    // 初始化连接按钮为不可点击状态，字体颜色为灰色
    lv_obj_add_state(ui->screen_wifi_psw_btn_connect, LV_STATE_DISABLED);
    lv_obj_set_style_text_color(ui->screen_wifi_psw_btn_connect, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);

    // 为返回按钮添加点击事件
    lv_obj_add_event_cb(ui->screen_wifi_psw_img_return, screen_wifi_psw_img_return_event_handler, LV_EVENT_CLICKED, ui);
    ui_subpage_return_set_large_touch_area(ui->screen_wifi_psw_img_return, (lv_coord_t)UI_SUBPAGE_RETURN_EXT_PAD);

    // 为取消按钮添加点击事件
    lv_obj_add_event_cb(ui->screen_wifi_psw_btn_del, screen_wifi_psw_btn_del_event_handler, LV_EVENT_CLICKED, ui);

    // 为连接按钮添加点击事件
    lv_obj_add_event_cb(ui->screen_wifi_psw_btn_connect, screen_wifi_psw_btn_connect_event_handler, LV_EVENT_CLICKED, ui);

    // 为输入框添加焦点事件处理函数
    lv_obj_add_event_cb(ui->screen_wifi_psw_ta_input, screen_wifi_psw_ta_input_event_handler, LV_EVENT_ALL, ui);

    // 为错误容器的确认按钮添加点击事件
    lv_obj_add_event_cb(ui->screen_wifi_psw_btn_know, screen_wifi_psw_btn_know_event_handler, LV_EVENT_CLICKED, ui);

    // 为连接中容器添加点击事件
    lv_obj_add_event_cb(ui->screen_wifi_psw_cont_connecting, screen_wifi_psw_cont_connecting_event_handler, LV_EVENT_CLICKED, ui);

#if CONFIG_LVGL_WIFI_PASSWORD_MASK_ENABLE && CONFIG_LVGL_WIFI_PASSWORD_IMGBTN_ENABLE
    lv_obj_add_event_cb(ui->screen_wifi_psw_imgbtn_psw, screen_wifi_psw_imgbtn_psw_event_handler, LV_EVENT_VALUE_CHANGED, ui);
    lv_obj_clear_state(ui->screen_wifi_psw_imgbtn_psw, LV_STATE_CHECKED);
    lv_textarea_set_password_mode(ui->screen_wifi_psw_ta_input, true);
    lv_textarea_set_password_bullet(ui->screen_wifi_psw_ta_input, "*");
#endif
}

void wifi_psw_event_setup(void)
{
    wifi_psw_event_init(&guider_ui);
}
#endif
