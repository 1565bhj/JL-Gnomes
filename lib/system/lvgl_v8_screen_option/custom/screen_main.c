#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdbool.h"
#include "gui_guider.h"
#include "custom.h"


#ifdef APPLAYER_ENABLE
#include "asm/gpio.h"
#include "lv_conf.h"
#include "system/includes.h"
#include "lvgl.h"
#include "sys_time.h"
#include "gui_guider.h"
#include "cJSON_common\cJSON.h"


// 闹钟检测相关声明
int get_alarm_count(void); // 获取闹钟数量
void alarm_status_check(void);
int audio_app_mode_check(void);
// 时间更新配置参数
#define TIME_UPDATE_INTERVAL_MS     1000    // 时间轮询间隔(毫秒)
#define TIME_RETRY_INTERVAL_MS      1000    // 时间重试间隔(毫秒)
#define TIME_MAX_RETRY_COUNT        60      // 最大重试次数

// 外部函数声明 - 来自alarm.c
extern uint8_t sys_time_to_weekday(struct sys_time *t);
// 外部函数声明
extern int get_sys_time(struct sys_time *t);  // 系统时间获取函数

// 函数声明
void start_time_update_with_net_wait(void);
void start_weather_update_with_net_wait(void);
// 声明用于页面跳转的函数
void switch_to_main_page(void);
void switch_to_image_page(void);

extern void alarm_ui_init(void);
// 网络状态控制变量
static bool g_network_failed = false;         // 网络连接失败标志
static int  g_time_retry_count = 0;           // 时间获取重试计数器
static int high_temp = 0;
static int low_temp = 0;

// 闹钟警告定时器ID
static uint16_t alarm_warning_hide_timer_id = 0;

// 全局UI对象
lv_ui guider_ui;

// 获取星期几的中文名称
static const char *get_weekday_name(int weekday)
{
    static const char *weekday_names[] = {
        "", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期日",
    };

    if (weekday >= 1 && weekday <= 7) {
        return weekday_names[weekday];
    }
    return "星期未更新";
}

typedef struct {
    char week_str[16];
    volatile bool time_update_pending;
} time_update_data_t;

static time_update_data_t g_weekday_data = {0};


/**
 * 时间更新数据结构
 * 用于线程安全的时间数据传递
 */
typedef struct {
    char        time_str[16];                 // 格式化时间 "HH:MM"
    char        date_str[16];                 // 格式化日期 "YYYY-MM-DD"
    char        week_str[16];                 // 格式化星期
    volatile bool time_updated;               // 时间更新标志位
} time_data_t;

static time_data_t g_time_data = {0};         // 全局时间数据

/**
 * 线程安全的时间数据更新
 * 仅获取和格式化时间，不操作UI
 */
static void time_data_update(void)
{
    struct sys_time current_time = {0};  // 显式初始化为0
    int ret = get_sys_time(&current_time);
    uint8_t weekday = sys_time_to_weekday(&current_time);

    // 获取系统时间失败检查
    if (ret != 0) {
        printf("err get time = %d\n", ret);
        return;
    }

    // 检查关键字段是否有效（如年份合理）
    if (current_time.year < 2024 || current_time.year > 2100) {
        printf("err year: %d\n", current_time.year);
        return;
    }

    // 每分钟打印一次详细时间信息
    static uint8_t last_minute = 0xFF;
    if (last_minute != current_time.min) {
        last_minute = current_time.min;
//        printf("[时间同步] 当前时间: %04d-%02d-%02d %02d:%02d:%02d\n",
//               current_time.year, current_time.month, current_time.day,
//               current_time.hour, current_time.min, current_time.sec);
    }
    // 校验时间字段合法性，防止异常值导致字符串过长
    if (current_time.hour < 0 || current_time.hour >= 24 ||
        current_time.min < 0 || current_time.min >= 60 ||
        current_time.sec < 0 || current_time.sec >= 60
       ) {
//        printf("[时间错误] 无效时间值: %d:%d\n", current_time.hour, current_time.min, current_time.sec);
        return;  // 放弃此次更新，避免缓冲区溢出
    }

    // 格式化时间字符串
    snprintf(g_time_data.time_str, sizeof(g_time_data.time_str),
             "%02d:%02d", current_time.hour, current_time.min);

    // 格式化日期字符串
    snprintf(g_time_data.date_str, sizeof(g_time_data.date_str),
             "%04d/%02d/%02d", current_time.year, current_time.month, current_time.day);

    snprintf(g_time_data.week_str, sizeof(g_time_data.week_str), "%s", get_weekday_name(weekday));

    // 设置更新标志
    g_time_data.time_updated = true;
}

/**
 * 处理时间UI更新
 * 在主线程中执行，确保UI操作的线程安全性
 */
void time_ui_update(void)
{
    if (g_time_data.time_updated) {
        g_time_data.time_updated = false;

        // 更新时间显示（320_240 主界面为大号 label_time）
        if (guider_ui.screen_main_label_time) {
            lv_label_set_text(guider_ui.screen_main_label_time, g_time_data.time_str);
            lv_obj_invalidate(guider_ui.screen_main_label_time);
        } else {
            printf("[UI错误] screen_main_label_time 对象无效\n");
        }
        // 更新下拉框时间显示
        if (guider_ui.screen_dropdown_time_span) {
            lv_span_set_text(guider_ui.screen_dropdown_time_span, g_time_data.time_str);
            lv_obj_invalidate(guider_ui.screen_dropdown_time_span);
            //            printf("[UI更新] 时间显示已更新: %s\n", g_time_data.time_str);
        } else {
            printf("[UI错误] screen_dropdown_time_span 对象无效\n");
        }

        // 注释：日期显示功能可根据需要启用
        if (guider_ui.screen_main_span_date_span) {
            lv_span_set_text(guider_ui.screen_main_span_date_span, g_time_data.date_str);
            lv_obj_invalidate(guider_ui.screen_main_span_date_span);
//             printf("[UI更新] 日期显示已更新: %s\n", g_time_data.time_str);
        } else {
            printf("[UI错误] screen_main_span_date 对象无效\n");
        }
        // 注释：日期显示功能可根据需要启用
        if (guider_ui.screen_main_span_week_span) {
            lv_span_set_text(guider_ui.screen_main_span_week_span, g_time_data.week_str);
//             printf("===============================\n");
//             printf("==week_str = %s=============\n",g_time_data.week_str);
//             printf("===============================\n");
            lv_obj_invalidate(guider_ui.screen_main_span_week_span);
//             printf("[UI更新] 星期显示已更新: %s\n", g_time_data.week_str);
        } else {
            printf("[UI错误] screen_main_span_week_span 对象无效\n");
        }

    }
}

/**
 * 时间更新定时器回调函数
 * 周期性触发时间数据更新
 */
static void time_update_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    time_data_update();  // 仅更新数据，不直接操作UI
}


static void network_connected_init_async(void *data);
/**
 * 网络连接检查与时间服务初始化
 * 网络未连接时进行重试
 */
static void network_check_and_init(void *priv)
{
    LV_UNUSED(priv);

    if (lv_demo_system_net_connect_success()) {  // 检查网络连接状态
        //printf("[网络状态] 已连接，初始化时间服务...\n");
        lv_async_call(network_connected_init_async, NULL);
    } else {
        g_time_retry_count++;
        //printf("[网络状态] 时间更新尝试 %d: 网络未连接\n", g_time_retry_count);

        if (g_time_retry_count < TIME_MAX_RETRY_COUNT) {
            // 继续重试
            sys_timeout_add_to_task("sys_timer", NULL, network_check_and_init, TIME_RETRY_INTERVAL_MS);
        } else {
            printf("[网络错误] 超过最大重试次数(%d)，时间服务初始化失败\n",
                   TIME_MAX_RETRY_COUNT);
            g_network_failed = true;
        }
    }
}

/**
 * 启动时间更新流程
 * 开始网络等待和时间同步
 */
void start_time_update_with_net_wait(void)
{
    g_time_retry_count = 0;
    //printf("\n[时间服务] 启动时间更新流程，等待网络连接...\n");
    sys_timeout_add_to_task("sys_timer", NULL, network_check_and_init, 0); // 立即开始第一次检查
}


/**
 * 时间更新包装函数
 * 用于异步调用时间更新
 */
static void time_update_async_cb(void *data)
{
    LV_UNUSED(data);
    time_data_update();
    time_ui_update();    // 立即更新UI
}

/**
 * 网络连接后的初始化操作
 * 启动时间更新定时器
 */
static void network_connected_init(void)
{
    // 异步更新时间
    lv_async_call(time_update_async_cb, NULL);

    // 创建时间更新定时器
    static lv_timer_t *time_update_timer = NULL;
    if (!time_update_timer) {
        time_update_timer = lv_timer_create(time_update_timer_cb, TIME_UPDATE_INTERVAL_MS, NULL);
    }

    int fetch_weather_data(void);  // 调用天气函数
    if (fetch_weather_data() && ++g_time_retry_count <= TIME_MAX_RETRY_COUNT) {  // 调用天气函数
        sys_timeout_add_to_task("sys_timer", NULL, network_check_and_init, TIME_RETRY_INTERVAL_MS);
    }
}

/**
 * 网络连接初始化异步包装函数
 */
static void network_connected_init_async(void *data)
{
    LV_UNUSED(data);
    network_connected_init();
}

/******************************************************************************************
**************************************获取天气值*******************************************
*******************************************************************************************/

/** 天气图标与 span 之间的水平间距 */
#define WEATHER_ROW_ICON_TEXT_GAP  6

static void screen_main_layout_weather_row_centered(void)
{
    lv_obj_t *scr = guider_ui.screen_main;
    lv_obj_t *img = guider_ui.screen_main_img_sun;
    lv_obj_t *span = guider_ui.screen_main_span_weather;

    if (!scr || !img || !span) {
        return;
    }

    lv_spangroup_refr_mode(span);
    lv_obj_update_layout(scr);

    lv_coord_t sw = lv_obj_get_width(scr);
    lv_coord_t iw = lv_obj_get_width(img);
    lv_coord_t tw = lv_obj_get_width(span);
    lv_coord_t total = iw + (lv_coord_t)WEATHER_ROW_ICON_TEXT_GAP + tw;
    lv_coord_t x0 = (sw - total) / 2;
    if (x0 < 0) {
        x0 = 0;
    }

    lv_obj_set_pos(img, x0, WEATHER_ICON_Y);
    lv_obj_set_pos(span, x0 + iw + (lv_coord_t)WEATHER_ROW_ICON_TEXT_GAP, WEATHER_ICON_Y);
}

/**
 * 根据天气文本显示相应的图标
 * @param weather 天气文本
 */
static void update_weather_icon(const char *weather)
{
    // 隐藏所有天气图标
//    lv_obj_add_flag(guider_ui.screen_main_img_sun, LV_OBJ_FLAG_HIDDEN);
//    lv_obj_add_flag(guider_ui.screen_main_img_rain, LV_OBJ_FLAG_HIDDEN);
//    lv_obj_add_flag(guider_ui.screen_main_img_snow, LV_OBJ_FLAG_HIDDEN);
//    lv_obj_add_flag(guider_ui.screen_main_img_wind, LV_OBJ_FLAG_HIDDEN);
//    lv_obj_add_flag(guider_ui.screen_main_img_windy, LV_OBJ_FLAG_HIDDEN);
//    lv_obj_add_flag(guider_ui.screen_main_img_overcast, LV_OBJ_FLAG_HIDDEN);

//    // 设置所有图标到太阳图标的位置 (85, 206)
//    lv_obj_set_pos(guider_ui.screen_main_img_rain, WEATHER_ICON_X, WEATHER_ICON_Y);
//    lv_obj_set_pos(guider_ui.screen_main_img_snow, WEATHER_ICON_X, WEATHER_ICON_Y);
//    lv_obj_set_pos(guider_ui.screen_main_img_wind, WEATHER_ICON_X, WEATHER_ICON_Y);
//    lv_obj_set_pos(guider_ui.screen_main_img_windy, WEATHER_ICON_X, WEATHER_ICON_Y);
//    lv_obj_set_pos(guider_ui.screen_main_img_overcast, WEATHER_ICON_X, WEATHER_ICON_Y);

    // 根据天气关键字显示相应的图标
    if (weather == NULL) {
        // 如果天气文本为空，显示默认的windy图标
        lv_img_set_src(guider_ui.screen_main_img_sun, &_windy_alpha_25x20);
        return;
    } else {
        // 晴天/太阳图标
        if (strstr(weather, "晴") != NULL) {
            lv_img_set_src(guider_ui.screen_main_img_sun, &_sun_alpha_25x20);
        }
        // 雨天图标
        else if (strstr(weather, "雨") != NULL) {
            lv_img_set_src(guider_ui.screen_main_img_sun, &_rain_alpha_25x20);
        }
        // 雪天图标
        else if (strstr(weather, "雪") != NULL) {
            lv_img_set_src(guider_ui.screen_main_img_sun, &_snow_alpha_25x20);
        }
        // 风图标
        else if (strstr(weather, "风") != NULL) {
            lv_img_set_src(guider_ui.screen_main_img_sun, &_windy_alpha_25x20);
        }
        // 阴天图标
        else if (strstr(weather, "阴") != NULL || strstr(weather, "多云") != NULL) {
            lv_img_set_src(guider_ui.screen_main_img_sun, &_overcast_alpha_25x20);
        }
        // 默认显示windy图标
        else {
            lv_img_set_src(guider_ui.screen_main_img_sun, &_windy_alpha_25x20);
        }
    }
    lv_obj_clear_flag(guider_ui.screen_main_img_sun, LV_OBJ_FLAG_HIDDEN);
}

int update_weather_ui_from_json(const char *json_data)
{
    int ret = -1;
    if (!json_data) {
        //printf("[错误] 无效的JSON数据\n");
        return ret;
    }

    // 解析JSON数据
    cJSON *root = cJSON_Parse(json_data);
    if (!root) {
        //printf("[错误] JSON解析失败: %s\n", cJSON_GetErrorPtr());
        return ret;
    }

    // 获取结果数组
    cJSON *results = cJSON_GetObjectItem(root, "results");
    if (!results || !cJSON_IsArray(results) || cJSON_GetArraySize(results) == 0) {
        //printf("[错误] 未找到有效的结果数组\n");
        cJSON_Delete(root);
        return ret;
    }

    // 获取第一个结果
    cJSON *first_result = cJSON_GetArrayItem(results, 0);
    if (!first_result) {
        //printf("[错误] 未找到天气结果\n");
        cJSON_Delete(root);
        return ret;
    }

    // 获取每日天气数组
    cJSON *daily = cJSON_GetObjectItem(first_result, "daily");
    if (!daily || !cJSON_IsArray(daily) || cJSON_GetArraySize(daily) == 0) {
        //printf("[错误] 未找到每日天气数据\n");
        cJSON_Delete(root);
        return ret;
    }

    // 获取今天的天气数据（第一条数据）
    cJSON *today = cJSON_GetArrayItem(daily, 0);
    if (!today) {
        //printf("[错误] 未找到今天的天气数据\n");
        cJSON_Delete(root);
        return ret;
    }

    cJSON *location = cJSON_GetObjectItem(first_result, "location");

    if (!location || !today) {
        //printf("[错误] location 或 daily 信息缺失\n");
        cJSON_Delete(root);
        return ret;
    }

    const char *city = cJSON_GetStringValue(cJSON_GetObjectItem(location, "name"));
    const char *weather = cJSON_GetStringValue(cJSON_GetObjectItem(today, "text_day"));
    const char *temp_day = cJSON_GetStringValue(cJSON_GetObjectItem(today, "high"));
    const char *temp_night = cJSON_GetStringValue(cJSON_GetObjectItem(today, "low"));
    const char *humidity = cJSON_GetStringValue(cJSON_GetObjectItem(today, "humidity"));
    const char *wind = cJSON_GetStringValue(cJSON_GetObjectItem(today, "wind_direction"));
    const char *wind_speed = cJSON_GetStringValue(cJSON_GetObjectItem(today, "wind_scale"));

    lv_img_set_src(guider_ui.screen_main_img_location, &_location_set_alpha_15x18);
    lv_obj_clear_flag(guider_ui.screen_main_img_location, LV_OBJ_FLAG_HIDDEN);

    lv_span_set_text(guider_ui.screen_main_span_direction_span, city);
    lv_style_set_text_color(&guider_ui.screen_main_span_direction_span->style, lv_color_hex(0xffffff));

//    // 动态更新location图标位置，使其始终显示在城市文字左侧
//    lv_obj_add_flag(guider_ui.screen_main_img_location, LV_OBJ_FLAG_HIDDEN);
//    lv_obj_align_to(guider_ui.screen_main_img_location_set, guider_ui.screen_main_span_direction, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
//    lv_obj_clear_flag(guider_ui.screen_main_img_location_set, LV_OBJ_FLAG_HIDDEN);

    printf("[城市] city = %s\n", city);
    // 更新天气和温度
    if (weather && temp_day && temp_night) {
        char weather_temp_str[64];

        snprintf(weather_temp_str, sizeof(weather_temp_str), "%s %s~%s℃", weather, temp_night, temp_day);
        lv_span_set_text(guider_ui.screen_main_span_weather_span, weather_temp_str);

        // 添加天气图标更新
        printf("[tq] weather_str = %s\n", weather);
        update_weather_icon(weather);
        screen_main_layout_weather_row_centered();
        //printf("[成功] 已更新温度数据\n");
        ret = 0;
    }

    // 释放JSON对象
    cJSON_Delete(root);
    return ret;
}

int fetch_weather_data(void)
{
    int ret = -1;
    // 1. 检查网络连接状态
    if (!lv_demo_system_net_connect_success()) {
        return ret;
    }

    char *buf = (char *)malloc(2048);
    if (buf == NULL) {
        printf("mem alloc err\n");
        return ret;
    }
    memset(buf, 0, 2048);

    // 2. 获取天气数据
    int result = qyai_weather_forecast_get(NULL, 0, buf, 2048);

    // 3. 处理结果
    if (result <= 0) {
        if (result == -1) {
            printf("[天气错误] 未绑定账号\n");
        } else {
            printf("[天气错误] 获取天气数据失败。\n");
        }
    } else {
        printf("\n%s\n", buf);
        // 4. 解析JSON
        ret = update_weather_ui_from_json(buf);
        if (ret) {
            printf("[天气错误] 获取天气数据异常\n");
        }
    }
    //printf("=========== 天气数据获取结束 ===========\n\n");
    free(buf);
    return ret;
}

/**
 * 闹钟状态检测函数
 * 检查当前是否有设置的闹钟，并相应地更新图标显示
 */
void alarm_status_check(void)
{
    // 检查UI对象是否有效
    if (!guider_ui.screen_main_img_no_alarm) {
        return;
    }
    if (!guider_ui.screen_main_img_set_alarm) {
        return;
    }
    // 检查UI对象是否有效
    if (!guider_ui.screen_dropdown_img_no_alarm) {
        return;
    }
    if (!guider_ui.screen_dropdown_img_set_alarm) {
        return;
    }
    lv_obj_set_pos(guider_ui.screen_main_img_set_alarm, ALARM_ICON_X, ALARM_ICON_Y);
    lv_obj_set_pos(guider_ui.screen_main_img_no_alarm, ALARM_ICON_X, ALARM_ICON_Y);
    lv_obj_set_pos(guider_ui.screen_dropdown_img_set_alarm, ALARM_ICON_X, ALARM_ICON_Y);
    lv_obj_set_pos(guider_ui.screen_dropdown_img_no_alarm, ALARM_ICON_X, ALARM_ICON_Y);
    // 调用get_alarm_count函数获取当前闹钟数量
    int alarm_count = get_alarm_count();
    // 根据是否有闹钟来显示或隐藏无闹钟图标
    if (alarm_count > 0) {
        // 有闹钟
        lv_obj_add_flag(guider_ui.screen_main_img_no_alarm, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(guider_ui.screen_main_img_set_alarm, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(guider_ui.screen_dropdown_img_no_alarm, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(guider_ui.screen_dropdown_img_set_alarm, LV_OBJ_FLAG_HIDDEN);
        //printf("[闹钟图标] 有闹钟\n");
    } else {
        // 无闹钟
        lv_obj_add_flag(guider_ui.screen_main_img_set_alarm, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(guider_ui.screen_main_img_no_alarm, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(guider_ui.screen_dropdown_img_set_alarm, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(guider_ui.screen_dropdown_img_no_alarm, LV_OBJ_FLAG_HIDDEN);
        //printf("[闹钟图标] 没有闹钟\n");
    }
}

/**
 * @brief 检查网络连接状态的外部调用函数
 * @return 1表示网络连接成功，0表示网络连接失败
 */
int ai_speaker_check_network_status(void)
{
    extern int lv_demo_system_net_interface(void);
    int net_interface = lv_demo_system_net_interface();
    int net_status = lv_demo_system_net_connect_success();
    //printf("[网络状态检查] 当前网络连接状态：%s（状态码：%d）\n", net_status ? "已连接" : "未连接", net_status);

    // 添加WiFi图标显示逻辑
    if (guider_ui.screen_main_img_no_wifi) {
        if (net_interface) {//4G
            if (net_status) {
                lv_img_set_src(guider_ui.screen_main_img_no_wifi, &_set_4g_alpha_18x16);
            } else {
                lv_img_set_src(guider_ui.screen_main_img_no_wifi, &_no_4g_alpha_18x16);
            }
        } else {//wifi
            if (net_status) {
                lv_img_set_src(guider_ui.screen_main_img_no_wifi, &_set_wifi_alpha_18x16);
                // 异步更新时间
                lv_async_call(time_update_async_cb, NULL);
            } else {
                lv_img_set_src(guider_ui.screen_main_img_no_wifi, &_no_wifi_alpha_18x16);
            }
        }
        lv_obj_clear_flag(guider_ui.screen_main_img_no_wifi, LV_OBJ_FLAG_HIDDEN);
//        lv_obj_set_pos(guider_ui.screen_main_img_set_wifi, WIFI_ICON_X, WIFI_ICON_Y);
//        lv_obj_set_pos(guider_ui.screen_main_img_no_wifi, WIFI_ICON_X, WIFI_ICON_Y);
//        if (net_status) {
//            // 网络已连接
//            lv_obj_add_flag(guider_ui.screen_main_img_no_wifi, LV_OBJ_FLAG_HIDDEN);
//            lv_obj_clear_flag(guider_ui.screen_main_img_set_wifi, LV_OBJ_FLAG_HIDDEN);
//            //printf("[WiFi图标] 网络已连接\n");
//        } else {
//            // 网络未连接
//            lv_obj_add_flag(guider_ui.screen_main_img_set_wifi, LV_OBJ_FLAG_HIDDEN);
//            lv_obj_clear_flag(guider_ui.screen_main_img_no_wifi, LV_OBJ_FLAG_HIDDEN);
//            //printf("[WiFi图标] 网络未连接\n");
//        }
    }
    // 添加WiFi图标显示逻辑
    if (guider_ui.screen_dropdown_img_no_wifi) {
        if (net_interface) {//4G
            if (net_status) {
                lv_img_set_src(guider_ui.screen_dropdown_img_no_wifi, &_set_4g_alpha_18x16);
            } else {
                lv_img_set_src(guider_ui.screen_dropdown_img_no_wifi, &_no_4g_alpha_18x16);
            }
        } else {//wifi
            if (net_status) {
                lv_img_set_src(guider_ui.screen_dropdown_img_no_wifi, &_set_wifi_alpha_18x16);
            } else {
                lv_img_set_src(guider_ui.screen_dropdown_img_no_wifi, &_no_wifi_alpha_18x16);
            }
        }
        lv_obj_clear_flag(guider_ui.screen_dropdown_img_no_wifi, LV_OBJ_FLAG_HIDDEN);
    }
    return net_status;
}

/**
 * @brief 检查蓝牙连接状态的外部调用函数
 * @return 1表示蓝牙连接成功，0表示蓝牙连接失败
 */
int BT_check_bluetooth_status(void)
{
    // 使用实际的蓝牙连接检查函数
    extern char lv_demo_system_bt_connect(void);
    int bt_status = lv_demo_system_bt_connect();
    //printf("[蓝牙状态检查] 当前蓝牙连接状态：%s（状态码：%d）\n", bt_status ? "已连接" : "未连接", bt_status);
    if (bt_status) {
        // 蓝牙已连接
        lv_img_set_src(guider_ui.screen_dropdown_img_no_BT, &_BT_alpha_14x16);
        lv_obj_clear_flag(guider_ui.screen_dropdown_img_no_BT, LV_OBJ_FLAG_HIDDEN);
    } else {
        // 蓝牙未连接
        lv_img_set_src(guider_ui.screen_dropdown_img_no_BT, &_no_BT_alpha_14x16);
        lv_obj_clear_flag(guider_ui.screen_dropdown_img_no_BT, LV_OBJ_FLAG_HIDDEN);
    }
    return bt_status;
}

// 更新电池电量显示 - 作为唯一控制电池图标显示的函数
static void update_battery_display(int percent, int charging, int chargfull)
{
    // 先隐藏所有图标
    if (guider_ui.screen_main_img_fully_battery) {
        lv_obj_add_flag(guider_ui.screen_main_img_fully_battery, LV_OBJ_FLAG_HIDDEN);
    }
    if (guider_ui.screen_main_img_charging_battery) {
        lv_obj_add_flag(guider_ui.screen_main_img_charging_battery, LV_OBJ_FLAG_HIDDEN);
    }
    if (guider_ui.screen_main_img_low_battery) {
        lv_obj_add_flag(guider_ui.screen_main_img_low_battery, LV_OBJ_FLAG_HIDDEN);
    }
    if (guider_ui.screen_main_img_normol_battery) {
        lv_obj_add_flag(guider_ui.screen_main_img_normol_battery, LV_OBJ_FLAG_HIDDEN);
    }
    // 先隐藏所有图标
    if (guider_ui.screen_dropdown_img_fully_battery) {
        lv_obj_add_flag(guider_ui.screen_dropdown_img_fully_battery, LV_OBJ_FLAG_HIDDEN);
    }
    if (guider_ui.screen_dropdown_img_charging_battery) {
        lv_obj_add_flag(guider_ui.screen_dropdown_img_charging_battery, LV_OBJ_FLAG_HIDDEN);
    }
    if (guider_ui.screen_dropdown_img_low_battery) {
        lv_obj_add_flag(guider_ui.screen_dropdown_img_low_battery, LV_OBJ_FLAG_HIDDEN);
    }
    if (guider_ui.screen_dropdown_img_normal_battery) {
        lv_obj_add_flag(guider_ui.screen_dropdown_img_normal_battery, LV_OBJ_FLAG_HIDDEN);
    }


    // 统一设置图标位置
    if (guider_ui.screen_main_img_fully_battery) {
        lv_obj_set_pos(guider_ui.screen_main_img_fully_battery, BATTERY_ICON_X, BATTERY_ICON_Y);
    }
    if (guider_ui.screen_main_img_charging_battery) {
        lv_obj_set_pos(guider_ui.screen_main_img_charging_battery, BATTERY_ICON_X, BATTERY_ICON_Y);
    }
    if (guider_ui.screen_main_img_low_battery) {
        lv_obj_set_pos(guider_ui.screen_main_img_low_battery, BATTERY_ICON_X, BATTERY_ICON_Y);
    }
    if (guider_ui.screen_main_img_normol_battery) {
        lv_obj_set_pos(guider_ui.screen_main_img_normol_battery, BATTERY_ICON_X, BATTERY_ICON_Y);
    }
    // 统一设置图标位置
    if (guider_ui.screen_dropdown_img_fully_battery) {
        lv_obj_set_pos(guider_ui.screen_dropdown_img_fully_battery, BATTERY_ICON_X, BATTERY_ICON_Y);
    }
    if (guider_ui.screen_dropdown_img_charging_battery) {
        lv_obj_set_pos(guider_ui.screen_dropdown_img_charging_battery, BATTERY_ICON_X, BATTERY_ICON_Y);
    }
    if (guider_ui.screen_dropdown_img_low_battery) {
        lv_obj_set_pos(guider_ui.screen_dropdown_img_low_battery, BATTERY_ICON_X, BATTERY_ICON_Y);
    }
    if (guider_ui.screen_dropdown_img_normal_battery) {
        lv_obj_set_pos(guider_ui.screen_dropdown_img_normal_battery, BATTERY_ICON_X, BATTERY_ICON_Y);
    }


    // 根据充电状态和电量百分比显示正确的图标
    if (charging) {  // 正在充电
        if (chargfull) {  // 已充满电
            if (guider_ui.screen_main_img_fully_battery) {
                lv_obj_clear_flag(guider_ui.screen_main_img_fully_battery, LV_OBJ_FLAG_HIDDEN);
                //printf("电池电量显示: 充电且满电图标\n");
            }
            if (guider_ui.screen_dropdown_img_fully_battery) {
                lv_obj_clear_flag(guider_ui.screen_dropdown_img_fully_battery, LV_OBJ_FLAG_HIDDEN);
                //printf("电池电量显示: 充电且满电图标\n");
            }
        } else {  // 充电中但未充满
            if (guider_ui.screen_main_img_charging_battery) {
                lv_obj_clear_flag(guider_ui.screen_main_img_charging_battery, LV_OBJ_FLAG_HIDDEN);
                //printf("电池电量显示: 充电图标\n");
            }
            if (guider_ui.screen_dropdown_img_charging_battery) {
                lv_obj_clear_flag(guider_ui.screen_dropdown_img_charging_battery, LV_OBJ_FLAG_HIDDEN);
                //printf("电池电量显示: 充电图标\n");
            }
        }
    } else {  // 未充电
        if (percent <= 22) {  // 低电量
            if (guider_ui.screen_main_img_low_battery) {
                lv_obj_clear_flag(guider_ui.screen_main_img_low_battery, LV_OBJ_FLAG_HIDDEN);
                //printf("电池电量显示: 低电量图标 (%d%%)\n", percent);
            }
            if (guider_ui.screen_dropdown_img_low_battery) {
                lv_obj_clear_flag(guider_ui.screen_dropdown_img_low_battery, LV_OBJ_FLAG_HIDDEN);
                printf("电池电量显示: 低电量图标 (%d%%)\n", percent);
            }
        } else {  // 正常电量
            if (guider_ui.screen_main_img_normol_battery) {
                lv_obj_clear_flag(guider_ui.screen_main_img_normol_battery, LV_OBJ_FLAG_HIDDEN);
                //printf("电池电量显示: 正常图标 (%d%%)\n", percent);
            }
            if (guider_ui.screen_dropdown_img_normal_battery) {
                lv_obj_clear_flag(guider_ui.screen_dropdown_img_normal_battery, LV_OBJ_FLAG_HIDDEN);
                //printf("电池电量显示: 正常图标 (%d%%)\n", percent);
            }
        }
    }
}

// 电池电量定时器回调函数 - 每分钟更新一次
static void battery_update_timer_cb(void *p)
{
    int percent = ((int)p >> 2);
    int charging = ((int)p >> 1) & 0x1;
    int chargfull = ((int)p & 0x1);
    update_battery_display(percent, charging, chargfull);
}

// 电池电量定时器回调函数 - 每分钟更新一次
void battery_update_sync(void *p)
{
    lv_async_call(battery_update_timer_cb, p);
}

static void screen_main_scr_size_changed_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    screen_main_layout_weather_row_centered();
}

static bool s_screen_main_weather_layout_cb_registered;

void screen_main_register_weather_layout(void)
{
    if (!guider_ui.screen_main || !guider_ui.screen_main_img_sun || !guider_ui.screen_main_span_weather) {
        return;
    }
    if (!s_screen_main_weather_layout_cb_registered) {
        lv_obj_add_event_cb(guider_ui.screen_main, screen_main_scr_size_changed_cb, LV_EVENT_SIZE_CHANGED, NULL);
        s_screen_main_weather_layout_cb_registered = true;
    }
    screen_main_layout_weather_row_centered();
}

#endif



// 初始化主页面事件
void screen_main_event_init(lv_ui *ui)
{
    if (!ui || !ui->screen_main) {
        return;
    }
    lv_obj_set_scroll_dir(ui->screen_main, LV_DIR_NONE);
#ifdef APPLAYER_ENABLE
    screen_main_register_weather_layout();
#endif
}
void screen_main_event_setup(void)
{
    screen_main_event_init(&guider_ui);
}
