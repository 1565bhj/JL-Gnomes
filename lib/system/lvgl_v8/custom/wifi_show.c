#include "lvgl.h"
#include "lvgl_develop.h"
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"
#include "wifi_common.h"
#include <stdio.h>

#if CONFIG_LVGL_OPTION_WIFI_ENABLE
// 声明清除WiFi SSID的函数
extern void wifi_sta_info_clear_ssid(char *ssid);
char *lvgl_get_init_wifi_name(char *wifi_name);
// 前向声明
void generate_wifi_containers(void);

// 动态WiFi容器结构体
typedef struct {
    lv_obj_t *cont;
    lv_obj_t *span_name;
    lv_span_t *span_name_span;
    lv_obj_t *img_wifi;
    lv_obj_t *img_lock;
} wifi_container_t;

// 动态WiFi容器数组
static wifi_container_t wifi_containers[MAX_WIFI_COUNT];

// WiFi扫描结果数组
wifi_scan_result_t wifi_scan_results[MAX_WIFI_COUNT];
// 当前扫描到的WiFi数量
int current_wifi_count = 0;

// 当前点击的WiFi名称
char current_wifi_name[256] = "";

// 当前点击的WiFi索引
int current_wifi_index = -1;

// WiFi密码数组，所有密码统一为12345678
const char *wifi_passwords[] = {
    "12345678",
    "12345678",
    "12345678",
    "12345678",
    "12345678",
    "12345678",
    "12345678",
    "12345678",
    "12345678",
    "12345678"
};




/**
 * @brief 设置 WiFi 未连接容器的位置、大小和滚动条
 * @param container 容器对象
 * @param is_move true 表示移动到已连接容器位置，false 表示恢复到原来位置
 */
void set_wifi_disconnected_container(lv_obj_t *container, bool is_move)
{
    if (is_move) {
        // 移动到已连接容器位置
        lv_obj_set_pos(container, 0, 40);
        lv_obj_set_size(container, 320, 200);
    } else {
        // 恢复到原来位置
        lv_obj_set_pos(container, 0, 118);
        lv_obj_set_size(container, 320, 120);
    }
    // 将滚动条重置到顶部
    lv_obj_scroll_to_y(container, 0, LV_ANIM_OFF);
}

/**
 * @brief WiFi容器点击事件处理函数
 */
static void wifi_container_click_event_handler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    int index = (int)(uintptr_t)lv_event_get_user_data(e);
    printf("WiFi容器 %d 被点击了\n", index + 1);

    // 保存当前点击的WiFi名称和索引
    const char *wifi_name = get_wifi_scan_ssid_name(index);
    strcpy(current_wifi_name, wifi_name);
    current_wifi_index = index;

    // 切换到WiFi密码页面
    switch_to_wifi_psw_page();
}

/**
 * @brief 异步生成WiFi容器的回调函数
 */
static void generate_wifi_containers_async(void *param)
{
    lv_ui *ui = (lv_ui *)param;

    // 清除现有的WiFi容器
    for (int i = 0; i < MAX_WIFI_COUNT; i++) {
        if (wifi_containers[i].cont) {
            lv_obj_del(wifi_containers[i].cont);
            wifi_containers[i].cont = NULL;
            wifi_containers[i].span_name = NULL;
            wifi_containers[i].span_name_span = NULL;
            wifi_containers[i].img_wifi = NULL;
            wifi_containers[i].img_lock = NULL;
        }
    }

    // 隐藏原有的固定容器
    if (ui->screen_wifi_show_cont_wifi1) {
        lv_obj_add_flag(ui->screen_wifi_show_cont_wifi1, LV_OBJ_FLAG_HIDDEN);
    }
    if (ui->screen_wifi_show_cont_wifi2) {
        lv_obj_add_flag(ui->screen_wifi_show_cont_wifi2, LV_OBJ_FLAG_HIDDEN);
    }

    // 动态生成WiFi容器
    int wifi_count = get_wifi_scan_ssid_num();
    printf("wifi_count: %d\n", wifi_count);

    int actual_container_count = 0; // 实际生成的容器数量

    for (int i = 0; i < wifi_count && i < MAX_WIFI_COUNT; i++) {
        /* 仅当「已连接」区域真正显示时，才从列表里去掉当前 SSID，避免 span 里残留旧名称却未联网时误跳过 */

        const char *init_wifi_name = get_wifi_scan_ssid_name(i);
        const char *connected_wifi_name = NULL;
        bool ui_show_connected = guider_ui.screen_wifi_show_cont_wifi
                                 && !lv_obj_has_flag(guider_ui.screen_wifi_show_cont_wifi, LV_OBJ_FLAG_HIDDEN);
        if (guider_ui.screen_wifi_show_span_wifi_name_span) {
            connected_wifi_name = guider_ui.screen_wifi_show_span_wifi_name_span->txt;
        }
        if (ui_show_connected && connected_wifi_name && *connected_wifi_name != '\0'
            && init_wifi_name && strcmp(init_wifi_name, connected_wifi_name) == 0) {
            printf("跳过生成已连接的WiFi容器: %s\n", init_wifi_name);
            continue;
        }
        // 创建WiFi容器
        wifi_containers[i].cont = lv_obj_create(ui->screen_wifi_show_cont_wifi_disconnected);
        lv_obj_set_pos(wifi_containers[i].cont, 12, 39 + actual_container_count * 44);
        actual_container_count++; // 增加实际容器计数
        lv_obj_set_size(wifi_containers[i].cont, 295, 35);
        lv_obj_set_scrollbar_mode(wifi_containers[i].cont, LV_SCROLLBAR_MODE_OFF);

        // 为容器添加点击事件
        lv_obj_add_flag(wifi_containers[i].cont, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(wifi_containers[i].cont, wifi_container_click_event_handler, LV_EVENT_CLICKED, (void *)(uintptr_t)i);

        // 设置容器样式
        lv_obj_set_style_border_width(wifi_containers[i].cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(wifi_containers[i].cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(wifi_containers[i].cont, 39, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(wifi_containers[i].cont, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_dir(wifi_containers[i].cont, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(wifi_containers[i].cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_bottom(wifi_containers[i].cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_left(wifi_containers[i].cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_right(wifi_containers[i].cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(wifi_containers[i].cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        // 创建WiFi名称文本
        wifi_containers[i].span_name = lv_spangroup_create(wifi_containers[i].cont);
        lv_spangroup_set_align(wifi_containers[i].span_name, LV_TEXT_ALIGN_LEFT);
        lv_spangroup_set_overflow(wifi_containers[i].span_name, LV_SPAN_OVERFLOW_CLIP);
        lv_spangroup_set_mode(wifi_containers[i].span_name, LV_SPAN_MODE_FIXED);

        // 创建文本span
        wifi_containers[i].span_name_span = lv_spangroup_new_span(wifi_containers[i].span_name);
        // 使用扫描结果数组中的WiFi名称
        lv_span_set_text(wifi_containers[i].span_name_span, init_wifi_name);
        lv_style_set_text_color(&wifi_containers[i].span_name_span->style, lv_color_hex(0xffffff));
        lv_style_set_text_decor(&wifi_containers[i].span_name_span->style, LV_TEXT_DECOR_NONE);
        lv_style_set_text_font(&wifi_containers[i].span_name_span->style, &lv_font_MiSansDemibold_18);
        lv_obj_set_pos(wifi_containers[i].span_name, 12, 6);
        lv_obj_set_size(wifi_containers[i].span_name, 221, 20);

        // 为WiFi名称文本添加点击事件
        lv_obj_add_flag(wifi_containers[i].span_name, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(wifi_containers[i].span_name, wifi_container_click_event_handler, LV_EVENT_CLICKED, (void *)(uintptr_t)i);

        // 创建WiFi图标
        wifi_containers[i].img_wifi = lv_img_create(wifi_containers[i].cont);
        lv_obj_add_flag(wifi_containers[i].img_wifi, LV_OBJ_FLAG_CLICKABLE);
        lv_img_set_src(wifi_containers[i].img_wifi, &_set_wifi_alpha_16x14);
        lv_img_set_pivot(wifi_containers[i].img_wifi, 50, 50);
        lv_img_set_angle(wifi_containers[i].img_wifi, 0);
        lv_obj_set_pos(wifi_containers[i].img_wifi, 267, 12);
        lv_obj_set_size(wifi_containers[i].img_wifi, 16, 14);

        // 为WiFi图标添加点击事件
        lv_obj_add_event_cb(wifi_containers[i].img_wifi, wifi_container_click_event_handler, LV_EVENT_CLICKED, (void *)(uintptr_t)i);

        // 创建锁图标
        wifi_containers[i].img_lock = lv_img_create(wifi_containers[i].cont);
        lv_obj_add_flag(wifi_containers[i].img_lock, LV_OBJ_FLAG_CLICKABLE);
        lv_img_set_src(wifi_containers[i].img_lock, &_lock_alpha_18x16);
        lv_img_set_pivot(wifi_containers[i].img_lock, 50, 50);
        lv_img_set_angle(wifi_containers[i].img_lock, 0);
        lv_obj_set_pos(wifi_containers[i].img_lock, 240, 11);
        lv_obj_set_size(wifi_containers[i].img_lock, 18, 16);

        // 为锁图标添加点击事件
        lv_obj_add_event_cb(wifi_containers[i].img_lock, wifi_container_click_event_handler, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
    }
}

/**
 * @brief 动态生成WiFi容器（异步）
 */
/**
 * @brief 刷新WiFi容器
 * @param ui LVGL UI对象
 */
void refresh_wifi_containers(void)
{
    // 清除现有的WiFi容器
    for (int i = 0; i < MAX_WIFI_COUNT; i++) {
        if (wifi_containers[i].cont) {
            lv_obj_del(wifi_containers[i].cont);
            wifi_containers[i].cont = NULL;
            wifi_containers[i].span_name = NULL;
            wifi_containers[i].span_name_span = NULL;
            wifi_containers[i].img_wifi = NULL;
            wifi_containers[i].img_lock = NULL;
        }
    }

    // 重新生成WiFi容器
    generate_wifi_containers();
}

static void wifi_refresh_after_scan_async_cb(void *p)
{
    (void)p;
    refresh_wifi_containers();
}

void wifi_show_request_refresh_after_scan(void)
{
    lv_async_call(wifi_refresh_after_scan_async_cb, NULL);
}

void generate_wifi_containers(void)
{
    // 使用异步调用生成WiFi容器
    lv_async_call(generate_wifi_containers_async, &guider_ui);
}

/**
 * @brief 初始化wifi屏幕的事件处理
 */
static void screen_wifi_show_img_return_event_handler(lv_event_t *e)
{
    switch_to_main_page();
}

static void screen_wifi_show_cont_wifi_success_event_handler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_ui *ui = lv_event_get_user_data(e);

    // 确保 cont_wifi_choose 容器显示
    lv_obj_clear_flag(ui->screen_wifi_show_cont_wifi_choose, LV_OBJ_FLAG_HIDDEN);

    // 将 cont_wifi_choose 容器的位置设置为 (0, 0)
    lv_obj_set_pos(ui->screen_wifi_show_cont_wifi_choose, 0, 0);
    lv_obj_scroll_to_y(ui->screen_wifi_show_cont_wifi_disconnected, 0, LV_ANIM_OFF);
}

static void screen_wifi_show_btn_del_event_handler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_ui *ui = lv_event_get_user_data(e);

    // 获取已连接的WiFi名称
    const char *connected_ssid = NULL;
    lv_span_t *span = lv_spangroup_get_child(ui->screen_wifi_show_span_wifi_success, 0);
    if (span) {
        connected_ssid = span->txt;
    }
    if (connected_ssid) {
        // 调用函数清除WiFi SSID
        wifi_sta_info_clear_ssid((char *)connected_ssid);
        printf("-> clear wifi ssid: %s\n", connected_ssid);
        // 调用wifi_enter_sta_mode函数进行实际连接
        wifi_enter_sta_mode("123", "123");
    }

    // 清空已连接的WiFi名称span文本
    if (guider_ui.screen_wifi_show_span_wifi_name_span) {
        lv_span_set_text(guider_ui.screen_wifi_show_span_wifi_name_span, "");
    }
    // 隐藏已连接的 WiFi 容器
    lv_obj_add_flag(ui->screen_wifi_show_cont_wifi, LV_OBJ_FLAG_HIDDEN);

    // 设置 WiFi 未连接容器的位置、大小和滚动条
    set_wifi_disconnected_container(ui->screen_wifi_show_cont_wifi_disconnected, true);

    // 隐藏 cont_wifi_choose 容器
    lv_obj_add_flag(ui->screen_wifi_show_cont_wifi_choose, LV_OBJ_FLAG_HIDDEN);

    scan_and_print_wifi_networks();
    // 刷新WiFi容器
    refresh_wifi_containers();
}

static void screen_wifi_show_img_return_choose_event_handler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_ui *ui = lv_event_get_user_data(e);

    // 隐藏 cont_wifi_choose 容器
    lv_obj_add_flag(ui->screen_wifi_show_cont_wifi_choose, LV_OBJ_FLAG_HIDDEN);
    lv_obj_scroll_to_y(ui->screen_wifi_show_cont_wifi_disconnected, 0, LV_ANIM_OFF);
}

/**
 * @brief QR码容器点击事件处理函数
 */
static void screen_wifi_show_cont_QR_code_event_handler(lv_event_t *e)
{
    switch_to_net_page();
}

static lv_anim_t s_wifi_refresh_icon_anim;

static void wifi_refresh_icon_anim_exec(void *var, int32_t v)
{
    lv_img_set_angle((lv_obj_t *)var, (int16_t)v);
}

static void wifi_refresh_icon_anim_ready(lv_anim_t *a)
{
    lv_obj_t *img = (lv_obj_t *)a->var;
    if (img) {
        lv_img_set_angle(img, 0);
    }
}

/** 刷新箭头绕中心缓慢转一圈（与 setup 中 pivot 50,50 一致） */
static void wifi_show_refresh_icon_spin_once(lv_obj_t *img_refresh)
{
    if (!img_refresh) {
        return;
    }
    lv_anim_del(img_refresh, NULL);
    lv_anim_init(&s_wifi_refresh_icon_anim);
    lv_anim_set_var(&s_wifi_refresh_icon_anim, img_refresh);
    lv_anim_set_exec_cb(&s_wifi_refresh_icon_anim, wifi_refresh_icon_anim_exec);
    lv_anim_set_values(&s_wifi_refresh_icon_anim, 0, 3600);
    lv_anim_set_time(&s_wifi_refresh_icon_anim, 1000);
    lv_anim_set_path_cb(&s_wifi_refresh_icon_anim, lv_anim_path_ease_in_out);
    lv_anim_set_ready_cb(&s_wifi_refresh_icon_anim, wifi_refresh_icon_anim_ready);
    lv_anim_start(&s_wifi_refresh_icon_anim);
}

/**
 * @brief 刷新容器点击事件处理函数
 */
static void screen_wifi_show_cont_refresh_event_handler(lv_event_t *e)
{
    lv_ui *ui = lv_event_get_user_data(e);
    printf("刷新容器被点击了\n");
    if (ui) {
        wifi_show_refresh_icon_spin_once(ui->screen_wifi_show_img_refresh);
    }
    scan_and_print_wifi_networks();
    // 刷新WiFi容器
    refresh_wifi_containers();

}

/**
 * @brief 初始化密码验证成功后的异步处理回调函数
 * @param param 连接成功的WiFi名称
 */
static void init_password_success_async(void *param)
{

    char *init_wifi_name = (char *)param;
    // 修改span_wifi_success和span_wifi_name显示的名称
    lv_span_set_text(guider_ui.screen_wifi_show_span_wifi_success_span, init_wifi_name);
    lv_span_set_text(guider_ui.screen_wifi_show_span_wifi_name_span, init_wifi_name);
    // 恢复原来的情况：显示已连接的 WiFi 容器
    lv_obj_clear_flag(guider_ui.screen_wifi_show_cont_wifi, LV_OBJ_FLAG_HIDDEN);
    // 设置 WiFi 未连接容器的位置、大小和滚动条
    set_wifi_disconnected_container(guider_ui.screen_wifi_show_cont_wifi_disconnected, false);

}
/**
 * @brief 初始化密码验证成功后的操作
 * @param wifi_name 连接成功的WiFi名称
 */
void lvgl_init_password_success(char *init_wifi_name)
{

    // 使用异步调用处理密码验证成功后的操作
    lv_async_call(init_password_success_async, init_wifi_name);
}


void wifi_show_event_init(lv_ui *ui)
{
    lv_obj_set_scroll_dir(guider_ui.screen_wifi_show, LV_DIR_NONE);

    // 根据宏定义控制是否默认隐藏已连接的WiFi容器
#if DEFAULT_SHOW_CONNECTED_WIFI == 1
    // 不隐藏已连接的WiFi容器
    lv_obj_clear_flag(ui->screen_wifi_show_cont_wifi, LV_OBJ_FLAG_HIDDEN);
    // 设置WiFi未连接容器的位置、大小和滚动条为原始状态
    set_wifi_disconnected_container(ui->screen_wifi_show_cont_wifi_disconnected, false);
#else
    // 默认隐藏已连接的WiFi容器
    lv_obj_add_flag(ui->screen_wifi_show_cont_wifi, LV_OBJ_FLAG_HIDDEN);
    // 设置WiFi未连接容器的位置、大小和滚动条为移动状态
    set_wifi_disconnected_container(ui->screen_wifi_show_cont_wifi_disconnected, true);
#endif


    // 动态生成WiFi容器
    generate_wifi_containers();

    // 为返回按钮添加点击事件
    lv_obj_add_event_cb(ui->screen_wifi_show_img_return, screen_wifi_show_img_return_event_handler, LV_EVENT_CLICKED, ui);
    ui_subpage_return_set_large_touch_area(ui->screen_wifi_show_img_return, (lv_coord_t)UI_SUBPAGE_RETURN_EXT_PAD);

    // 为连接成功容器及其内部元素添加点击事件
    // 容器
    lv_obj_add_flag(ui->screen_wifi_show_cont_wifi_success, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(ui->screen_wifi_show_cont_wifi_success, screen_wifi_show_cont_wifi_success_event_handler, LV_EVENT_CLICKED, ui);
    // 文本
    lv_obj_add_flag(ui->screen_wifi_show_span_wifi_success, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(ui->screen_wifi_show_span_wifi_success, screen_wifi_show_cont_wifi_success_event_handler, LV_EVENT_CLICKED, ui);
    // 成功图标
    lv_obj_add_flag(ui->screen_wifi_show_img_suceess, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(ui->screen_wifi_show_img_suceess, screen_wifi_show_cont_wifi_success_event_handler, LV_EVENT_CLICKED, ui);
    // WiFi图标
    lv_obj_add_flag(ui->screen_wifi_show_img_wifi_success, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(ui->screen_wifi_show_img_wifi_success, screen_wifi_show_cont_wifi_success_event_handler, LV_EVENT_CLICKED, ui);

    // 为删除按钮添加点击事件
    lv_obj_add_event_cb(ui->screen_wifi_show_btn_del, screen_wifi_show_btn_del_event_handler, LV_EVENT_CLICKED, ui);

    // 为选择页面的返回按钮添加点击事件
    lv_obj_add_event_cb(ui->screen_wifi_show_img_return_choose, screen_wifi_show_img_return_choose_event_handler, LV_EVENT_CLICKED, ui);

    // 为QR码容器及其内部元素添加点击事件
    // 容器
    lv_obj_add_flag(ui->screen_wifi_show_cont_QR_code, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(ui->screen_wifi_show_cont_QR_code, screen_wifi_show_cont_QR_code_event_handler, LV_EVENT_CLICKED, ui);
    // 图片
    lv_obj_add_flag(ui->screen_wifi_show_img_QR_code, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(ui->screen_wifi_show_img_QR_code, screen_wifi_show_cont_QR_code_event_handler, LV_EVENT_CLICKED, ui);
    // 标签
    lv_obj_add_flag(ui->screen_wifi_show_label_QR_code, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(ui->screen_wifi_show_label_QR_code, screen_wifi_show_cont_QR_code_event_handler, LV_EVENT_CLICKED, ui);

    // 为刷新容器及其内部元素添加点击事件
    // 容器
    lv_obj_add_flag(ui->screen_wifi_show_cont_refresh, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(ui->screen_wifi_show_cont_refresh, screen_wifi_show_cont_refresh_event_handler, LV_EVENT_CLICKED, ui);
    // 图片
    lv_obj_add_flag(ui->screen_wifi_show_img_refresh, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(ui->screen_wifi_show_img_refresh, screen_wifi_show_cont_refresh_event_handler, LV_EVENT_CLICKED, ui);
    // 标签
    lv_obj_add_flag(ui->screen_wifi_show_label_refresh, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(ui->screen_wifi_show_label_refresh, screen_wifi_show_cont_refresh_event_handler, LV_EVENT_CLICKED, ui);

    // 设置滚动属性：只有 cont_wifi_disconnected 容器可以上下滑动
    // 启用 cont_wifi_disconnected 容器的滚动
    lv_obj_set_scrollbar_mode(ui->screen_wifi_show_cont_wifi_disconnected, LV_SCROLLBAR_MODE_ON);
    lv_obj_set_scroll_dir(ui->screen_wifi_show_cont_wifi_disconnected, LV_DIR_VER);

    // 禁用其他容器的滚动
    lv_obj_set_scrollbar_mode(ui->screen_wifi_show, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scrollbar_mode(ui->screen_wifi_show_cont_wifi, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scrollbar_mode(ui->screen_wifi_show_cont_title, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scrollbar_mode(ui->screen_wifi_show_cont_wifi_choose, LV_SCROLLBAR_MODE_OFF);
}

void wifi_show_event_setup(void)
{
    wifi_show_event_init(&guider_ui);
}
//void wifi_show_event_setup_once(void) {
//    wifi_show_event_init(&guider_ui);
//}
#else /* !CONFIG_LVGL_OPTION_WIFI_ENABLE — 仍供 apps 层 WiFi 任务链接，无 LVGL WiFi 页时不操作 UI */

void wifi_show_request_refresh_after_scan(void)
{
}

void lvgl_init_password_success(char *init_wifi_name)
{
    (void)init_wifi_name;
}

#endif /* CONFIG_LVGL_OPTION_WIFI_ENABLE */
