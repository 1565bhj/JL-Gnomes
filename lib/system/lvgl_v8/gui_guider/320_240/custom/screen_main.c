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

// 图标位置宏定义
#define WEATHER_ICON_X           85
#define WEATHER_ICON_Y           206
#define ALARM_ICON_X             202
#define ALARM_ICON_Y             19
#define WIFI_ICON_X              248
#define WIFI_ICON_Y              18
#define BLUETOOTH_ICON_X         226
#define BLUETOOTH_ICON_Y         19
#define BATTERY_ICON_X           274
#define BATTERY_ICON_Y           19


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
static bool g_network_initialized = false;    // 网络初始化完成标志
static bool g_network_failed = false;         // 网络连接失败标志
static int  g_time_retry_count = 0;           // 时间获取重试计数器
static int high_temp = 0;
static int low_temp = 0;

// 闹钟警告定时器ID
static uint16_t alarm_warning_hide_timer_id = 0;

// 全局UI对象
lv_ui guider_ui;

// 获取星期几的中文名称
static const char* get_weekday_name(int weekday) {
    static const char* weekday_names[] = {
        "","星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期日",
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

static void weekday_data_update(void){
//// 计算并格式化星期几
////    struct sys_time current_weekday = {0};  // 显式初始化为0
//#if 1
//    //方式一：直接拷贝
//    uint8_t weekday = sys_time_to_weekday(&current_weekday);
//    snprintf(g_weekday_data.week_str, sizeof(g_weekday_data.week_str), "%s", get_weekday_name(weekday));
//    lv_span_set_text(guider_ui.screen_span_week_span, g_weekday_data.week_str);
//    printf("===============================\n");
//    printf("==week_str = %s=============\n",g_weekday_data.week_str);
//    printf("===============================\n");
//#else
//    //方式二：不拷贝传指针
//    const char *w = get_weekday_name(sys_time_to_weekday(&current_weekday));
//    lv_span_set_text(guider_ui.screen_span_week_span, w);
//    printf("===============================\n");
//    printf("==week_str = %s=============\n",w);
//    printf("===============================\n");
//#endif


}



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
        printf("[时间错误] 获取系统时间失败，错误码: %d\n", ret);
        return;
    }

    // 检查关键字段是否有效（如年份合理）
    if (current_time.year < 2024 || current_time.year > 2100) {
        printf("[时间错误] 年份异常: %d\n", current_time.year);
        return;
    }

    // 每分钟打印一次详细时间信息
    static uint8_t last_minute = 0xFF;
    if (last_minute != current_time.min) {
        last_minute = current_time.min;
        printf("[时间同步] 当前时间: %04d-%02d-%02d %02d:%02d:%02d\n",
               current_time.year, current_time.month, current_time.day,
               current_time.hour, current_time.min, current_time.sec);
    }
    // 校验时间字段合法性，防止异常值导致字符串过长
    if (current_time.hour < 0 || current_time.hour >= 24 ||
        current_time.min < 0 || current_time.min >= 60 ||
        current_time.sec < 0 || current_time.sec >= 60
) {
        printf("[时间错误] 无效时间值: %d:%d\n", current_time.hour, current_time.min, current_time.sec);
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
//    weekday_data_update();
}

/**
 * 处理时间UI更新
 * 在主线程中执行，确保UI操作的线程安全性
 */
void time_ui_update(void)
{
    if (g_time_data.time_updated) {
        g_time_data.time_updated = false;

        // 更新时间显示
        if (guider_ui.screen_main_span_time_span) {
            lv_span_set_text(guider_ui.screen_main_span_time_span, g_time_data.time_str);
            lv_obj_invalidate(guider_ui.screen_main_span_time_span);
//            printf("[UI更新] 时间显示已更新: %s\n", g_time_data.time_str);
        } else {
            printf("[UI错误] screen_main_span_time 对象无效\n");
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
    if (g_network_initialized) {
        printf("[网络初始化] 时间服务已初始化，跳过重复操作\n");
        return;
    }

    printf("[网络初始化] 开始时间服务初始化...\n");
    g_network_initialized = true;

    // 异步更新时间
    printf("[网络初始化] 正在同步系统时间...\n");
    lv_async_call(time_update_async_cb, NULL);

    // 创建时间更新定时器
    lv_timer_create(time_update_timer_cb, TIME_UPDATE_INTERVAL_MS, NULL);
    printf("[时间服务] 已启动时间更新定时器，间隔: %dms\n", TIME_UPDATE_INTERVAL_MS);
}

/**
 * 网络连接初始化异步包装函数
 */
static void network_connected_init_async(void *data)
{
    LV_UNUSED(data);
    network_connected_init();
}

/**
 * 网络连接检查与时间服务初始化
 * 网络未连接时进行重试
 */
static void network_check_and_init(void *priv)
{
    LV_UNUSED(priv);

    if (sys_connect_net_success()) {  // 检查网络连接状态
        printf("[网络状态] 已连接，初始化时间服务...\n");
        lv_async_call(network_connected_init_async, NULL);
        extern void fetch_weather_data(void);  // 调用天气函数
        fetch_weather_data();  // 调用天气函数
    } else {
        g_time_retry_count++;
        printf("[网络状态] 时间更新尝试 %d: 网络未连接\n", g_time_retry_count);

        if (g_time_retry_count < TIME_MAX_RETRY_COUNT) {
            // 继续重试
            sys_timeout_add_to_task("sys_timer",NULL, network_check_and_init, TIME_RETRY_INTERVAL_MS);
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
    printf("\n[时间服务] 启动时间更新流程，等待网络连接...\n");
    sys_timeout_add_to_task("sys_timer",NULL, network_check_and_init, 0);  // 立即开始第一次检查
}


/******************************************************************************************
**************************************获取天气值*******************************************
*******************************************************************************************/


/**
 * 根据天气文本显示相应的图标
 * @param weather 天气文本
 */
static void update_weather_icon(const char *weather) {
    // 隐藏所有天气图标
    lv_obj_add_flag(guider_ui.screen_main_img_sun, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(guider_ui.screen_main_img_rain, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(guider_ui.screen_main_img_snow, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(guider_ui.screen_main_img_wind, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(guider_ui.screen_main_img_windy, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(guider_ui.screen_main_img_overcast, LV_OBJ_FLAG_HIDDEN);

    // 设置所有图标到太阳图标的位置 (85, 206)
    lv_obj_set_pos(guider_ui.screen_main_img_rain, WEATHER_ICON_X, WEATHER_ICON_Y);
    lv_obj_set_pos(guider_ui.screen_main_img_snow, WEATHER_ICON_X, WEATHER_ICON_Y);
    lv_obj_set_pos(guider_ui.screen_main_img_wind, WEATHER_ICON_X, WEATHER_ICON_Y);
    lv_obj_set_pos(guider_ui.screen_main_img_windy, WEATHER_ICON_X, WEATHER_ICON_Y);
    lv_obj_set_pos(guider_ui.screen_main_img_overcast, WEATHER_ICON_X, WEATHER_ICON_Y);

    // 根据天气关键字显示相应的图标
    if (weather == NULL) {
        // 如果天气文本为空，显示默认的windy图标
        lv_obj_clear_flag(guider_ui.screen_main_img_windy, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    // 晴天/太阳图标
    if (strstr(weather, "晴") != NULL) {
        lv_obj_clear_flag(guider_ui.screen_main_img_sun, LV_OBJ_FLAG_HIDDEN);

    }
    // 雨天图标
    else if (strstr(weather, "雨") != NULL) {
        lv_obj_clear_flag(guider_ui.screen_main_img_rain, LV_OBJ_FLAG_HIDDEN);

    }
    // 雪天图标
    else if (strstr(weather, "雪") != NULL) {
        lv_obj_clear_flag(guider_ui.screen_main_img_snow, LV_OBJ_FLAG_HIDDEN);

    }
    // 风图标
    else if (strstr(weather, "风") != NULL) {
        lv_obj_clear_flag(guider_ui.screen_main_img_wind, LV_OBJ_FLAG_HIDDEN);

    }
    // 阴天图标
    else if (strstr(weather, "阴") != NULL || strstr(weather, "多云") != NULL) {
        lv_obj_clear_flag(guider_ui.screen_main_img_overcast, LV_OBJ_FLAG_HIDDEN);

    }
    // 默认显示windy图标
    else {
        lv_obj_clear_flag(guider_ui.screen_main_img_windy, LV_OBJ_FLAG_HIDDEN);

    }
}

void update_weather_ui_from_json(const char *json_data)
{

    if (!json_data) {
        printf("[错误] 无效的JSON数据\n");
        return;
    }

    // 解析JSON数据
    cJSON *root = cJSON_Parse(json_data);
    if (!root) {
        printf("[错误] JSON解析失败: %s\n", cJSON_GetErrorPtr());
        return;
    }

    // 获取结果数组
    cJSON *results = cJSON_GetObjectItem(root, "results");
    if (!results || !cJSON_IsArray(results) || cJSON_GetArraySize(results) == 0) {
        printf("[错误] 未找到有效的结果数组\n");
        cJSON_Delete(root);
        return;
    }

    // 获取第一个结果
    cJSON *first_result = cJSON_GetArrayItem(results, 0);
    if (!first_result) {
        printf("[错误] 未找到天气结果\n");
        cJSON_Delete(root);
        return;
    }

    // 获取每日天气数组
    cJSON *daily = cJSON_GetObjectItem(first_result, "daily");
    if (!daily || !cJSON_IsArray(daily) || cJSON_GetArraySize(daily) == 0) {
        printf("[错误] 未找到每日天气数据\n");
        cJSON_Delete(root);
        return;
    }

    // 获取今天的天气数据（第一条数据）
    cJSON *today = cJSON_GetArrayItem(daily, 0);
    if (!today) {
        printf("[错误] 未找到今天的天气数据\n");
        cJSON_Delete(root);
        return;
    }
//    // 解析天气
//
//        cJSON *weather = cJSON_GetObjectItem(today, "text_day");
//        if (weather && cJSON_IsString(weather) && weather->valuestring) {
//            printf("[天气] : %s\n", weather);
//        } else {
//            cJSON_Delete(root);
//            return;
//        }
//
// // 解析最高温度
//
//        cJSON *high = cJSON_GetObjectItem(today, "high");
//        if (high && cJSON_IsString(high) && high->valuestring) {
//            high_temp = atoi(high->valuestring);
//            printf("[温度] 最高温: %d°C\n", high_temp);
//
//        } else {
//            printf("[警告] 未找到最高温度数据\n");
//            cJSON_Delete(root);
//            return;
//        }
//
//
//    // 解析最低温度
//
//        cJSON *low = cJSON_GetObjectItem(today, "low");
//        if (low && cJSON_IsString(low) && low->valuestring) {
//            low_temp = atoi(low->valuestring);
//            printf("[温度] 最低温: %d°C\n", low_temp);
//
//        } else {
//            cJSON_Delete(root);
//            return;
//        }
    cJSON *location = cJSON_GetObjectItem(first_result, "location");

    if (!location || !today) {
        printf("[错误] location 或 daily 信息缺失\n");
        cJSON_Delete(root);
        return;
    }

    const char *city = cJSON_GetStringValue(cJSON_GetObjectItem(location, "name"));
    const char *weather = cJSON_GetStringValue(cJSON_GetObjectItem(today, "text_day"));
    const char *temp_day = cJSON_GetStringValue(cJSON_GetObjectItem(today, "high"));
    const char *temp_night = cJSON_GetStringValue(cJSON_GetObjectItem(today, "low"));
    const char *humidity = cJSON_GetStringValue(cJSON_GetObjectItem(today, "humidity"));
    const char *wind = cJSON_GetStringValue(cJSON_GetObjectItem(today, "wind_direction"));
    const char *wind_speed = cJSON_GetStringValue(cJSON_GetObjectItem(today, "wind_scale"));

    char city_str[64];
    snprintf(city_str, sizeof(city_str), "%s", city);
    // UI元素
    lv_span_set_text(guider_ui.screen_main_span_direction_span, city_str);
    // 动态更新location图标位置，使其始终显示在城市文字左侧
    lv_obj_add_flag(guider_ui.screen_main_img_location, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(guider_ui.screen_main_img_location_set, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align_to(guider_ui.screen_main_img_location_set, guider_ui.screen_main_span_direction, LV_ALIGN_OUT_LEFT_MID, -10, 0);

    printf("[城市] city = %s\n",city);
    printf("[城市] city_str = %s\n",city_str);
    // 更新天气和温度
    if (weather && temp_day && temp_night) {
        char weather_str[64];
        char weather_temp_str[64];
        snprintf(weather_str, sizeof(weather_str), "%s", weather);
        snprintf(weather_temp_str, sizeof(weather_temp_str), "%s %s~%s℃", weather, temp_night, temp_day);
        lv_span_set_text(guider_ui.screen_main_span_weather_span, weather_temp_str);
        lv_style_set_text_color(&guider_ui.screen_main_span_direction_span->style, lv_color_hex(0xffffff));
        // 添加天气图标更新
        printf("[tq] weather_str = %s\n",weather);
        update_weather_icon(weather_str);
    }

    // 释放JSON对象
    cJSON_Delete(root);
    printf("[成功] 已更新温度数据\n");
}

void fetch_weather_data(void)
{
    char *buf = (char *)malloc(2048);
    if (buf == NULL) {
        printf("[内存错误] 无法为天气缓冲区分配内存。\n");
        return;
    }
    memset(buf, 0, 2048);

    printf("\n=========== 天气数据获取开始 ===========\n");

    // 1. 检查网络连接状态
    if (!sys_connect_net_success()) {
        printf("===================================================\n");
        printf("[网络错误] 网络未连接。请检查您的连接设置。\n");
        printf("===================================================\n");
        free(buf);
        return;
    }

    printf("[网络] 连接已建立。正在获取天气信息...\n");

    // 2. 获取天气数据
    int result = qyai_weather_forecast_get(NULL, 0, buf, 2048);

    // 3. 处理结果
    if (result <= 0) {
        if (result == -1) {
            printf("[天气错误] 未绑定账号。请绑定您的账号以使用天气服务。\n");
        } else {
            printf("[天气错误] 获取天气数据失败。请稍后再试。\n");
        }
    } else {
        printf("[天气数据] 获取成功。内容如下：\n");
        printf("---------------------------------------------------\n");
        printf("%s\n", buf);
        printf("---------------------------------------------------\n");

        // 4. 解析JSON
        update_weather_ui_from_json(buf);
    }
    printf("=========== 天气数据获取结束 ===========\n\n");
    free(buf);
}

static void on_network_connected_init_wrapper (void *data);
// === 定时器回调函数 ===
void weather_check_net_and_fetch(void)
{
    if (sys_connect_net_success()) {
        printf("[Network] Connected. Fetching weather data...\n");
        // 获取天气数据
        fetch_weather_data();  // 调用天气函数

        //        // 执行网络连接后的初始化（如果还没有执行），使用异步调用确保线程安全
        lv_async_call(on_network_connected_init_wrapper, NULL);
    } else {
        printf("[Network] Attempt: Not connected.\n");
        sys_timeout_add_to_task("sys_timer",NULL, weather_check_net_and_fetch, 2000);
    }
}
// === 外部调用入口 ===
void start_weather_update_with_net_wait(void)
{
//    weather_retry_count = 0;
    printf("\n[天气初始化] 开始天气更新。等待网络连接...\n");
    // 立即开始第一次检查
    sys_timeout_add_to_task("sys_timer",NULL, weather_check_net_and_fetch, 2000);
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
        printf("[闹钟图标] 有闹钟\n");
    } else {
        // 无闹钟
        lv_obj_add_flag(guider_ui.screen_main_img_set_alarm, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(guider_ui.screen_main_img_no_alarm, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(guider_ui.screen_dropdown_img_set_alarm, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(guider_ui.screen_dropdown_img_no_alarm, LV_OBJ_FLAG_HIDDEN);
        printf("[闹钟图标] 没有闹钟\n");
    }
}

/**
 * @brief 检查网络连接状态的外部调用函数
 * @return 1表示网络连接成功，0表示网络连接失败
 */
int ai_speaker_check_network_status(void)
{
    int net_status = sys_connect_net_success();
    printf("[网络状态检查] 当前网络连接状态：%s（状态码：%d）\n", net_status ? "已连接" : "未连接", net_status);

    // 添加WiFi图标显示逻辑
    if (guider_ui.screen_main_img_no_wifi && guider_ui.screen_main_img_set_wifi) {
        lv_obj_set_pos(guider_ui.screen_main_img_set_wifi, WIFI_ICON_X, WIFI_ICON_Y);
        lv_obj_set_pos(guider_ui.screen_main_img_no_wifi, WIFI_ICON_X, WIFI_ICON_Y);
        if (net_status) {
            // 网络已连接
            lv_obj_add_flag(guider_ui.screen_main_img_no_wifi, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(guider_ui.screen_main_img_set_wifi, LV_OBJ_FLAG_HIDDEN);
            printf("[WiFi图标] 网络已连接\n");
        } else {
            // 网络未连接
            lv_obj_add_flag(guider_ui.screen_main_img_set_wifi, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(guider_ui.screen_main_img_no_wifi, LV_OBJ_FLAG_HIDDEN);
            printf("[WiFi图标] 网络未连接\n");
        }
    }
    // 添加WiFi图标显示逻辑
    if (guider_ui.screen_dropdown_img_no_wifi && guider_ui.screen_dropdown_img_set_wifi) {
        lv_obj_set_pos(guider_ui.screen_dropdown_img_set_wifi, WIFI_ICON_X, WIFI_ICON_Y);
        lv_obj_set_pos(guider_ui.screen_dropdown_img_no_wifi, WIFI_ICON_X, WIFI_ICON_Y);
        if (net_status) {
            // 网络已连接
            lv_obj_add_flag(guider_ui.screen_dropdown_img_no_wifi, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(guider_ui.screen_dropdown_img_set_wifi, LV_OBJ_FLAG_HIDDEN);
            printf("[WiFi图标] 网络已连接\n");
        } else {
            // 网络未连接
            lv_obj_add_flag(guider_ui.screen_dropdown_img_set_wifi, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(guider_ui.screen_dropdown_img_no_wifi, LV_OBJ_FLAG_HIDDEN);
            printf("[WiFi图标] 网络未连接\n");
        }
    }
    return net_status;
}
extern bool BT_music_is_playing(void);
extern void BT_music_is_playing_status(bool flag);
/**
 * @brief 检查蓝牙连接状态的外部调用函数
 * @return 1表示蓝牙连接成功，0表示蓝牙连接失败
 */
int BT_check_bluetooth_status(void)
{
    // 使用实际的蓝牙连接检查函数
    extern char bt_connect_check(void);
    char bt_status_char = bt_connect_check();
    int bt_status = (bt_status_char != 0); // 转换为int类型，非0表示已连接
    printf("[蓝牙状态检查] 当前蓝牙连接状态：%s（状态码：%d）\n", bt_status ? "已连接" : "未连接", bt_status);

    // 添加蓝牙图标显示逻辑
    if (guider_ui.screen_main_img_no_BT && guider_ui.screen_main_img_set_BT) {
        // 设置蓝牙图标的位置，这里假设放在适当的位置
        lv_obj_set_pos(guider_ui.screen_main_img_set_BT, BLUETOOTH_ICON_X, BLUETOOTH_ICON_Y);  // 位置根据UI设计调整
        lv_obj_set_pos(guider_ui.screen_main_img_no_BT, BLUETOOTH_ICON_X, BLUETOOTH_ICON_Y);   // 位置根据UI设计调整
        lv_obj_set_pos(guider_ui.screen_dropdown_img_set_BT, BLUETOOTH_ICON_X, BLUETOOTH_ICON_Y);  // 位置根据UI设计调整
        lv_obj_set_pos(guider_ui.screen_dropdown_img_no_BT, BLUETOOTH_ICON_X, BLUETOOTH_ICON_Y);   // 位置根据UI设计调整
//        sys_timeout_add_to_task("sys_timer",0 ,BT_music_is_playing_status,2000);
        if (bt_status) {
            // 蓝牙已连接
            lv_obj_add_flag(guider_ui.screen_main_img_no_BT, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(guider_ui.screen_main_img_set_BT, LV_OBJ_FLAG_HIDDEN);

            lv_obj_add_flag(guider_ui.screen_dropdown_img_no_BT, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(guider_ui.screen_dropdown_img_set_BT, LV_OBJ_FLAG_HIDDEN);

            printf("[蓝牙图标] 蓝牙已连接\n");

        } else {
            // 蓝牙未连接
            lv_obj_add_flag(guider_ui.screen_main_img_set_BT, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(guider_ui.screen_main_img_no_BT, LV_OBJ_FLAG_HIDDEN);

            lv_obj_add_flag(guider_ui.screen_dropdown_img_set_BT, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(guider_ui.screen_dropdown_img_no_BT, LV_OBJ_FLAG_HIDDEN);
            printf("[蓝牙图标] 蓝牙未连接\n");
        }
    }
//    extern void lv_demo_music_update_play_pause_button_flush(void*p);
//    if(BT_music_is_playing()){
//        printf("===========================================\n");
//        printf("=======没有蓝牙音乐播放=====\n");
//        lv_async_call(lv_demo_music_update_play_pause_button_flush, 0);
//        printf("===========================================\n");
//    }else{
//        printf("===========================================\n");
//        printf("=======有蓝牙音乐播放=====\n");
//        lv_async_call(lv_demo_music_update_play_pause_button_flush, 1);
//        printf("===========================================\n");
//    }

    return bt_status;
}


//充电检测相关
static int chargfull;
#define TCFG_CHARING_STATUS_PORT        IO_PORTC_06//IO_PORT_PR_00
#define TCFG_CHARGFULL_STATUS_PORT      IO_PORT_PR_00
#define DEBOUNCE_THRESHOLD 3  // 防抖阈值
// 更新电池电量显示 - 作为唯一控制电池图标显示的函数
void update_battery_display(u8 percent) {
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


    int charging_status = is_charging_now();

    // 根据充电状态和电量百分比显示正确的图标
    if (charging_status) {  // 正在充电
        if (chargfull) {  // 已充满电
            if (guider_ui.screen_main_img_fully_battery) {
                lv_obj_clear_flag(guider_ui.screen_main_img_fully_battery, LV_OBJ_FLAG_HIDDEN);

                printf("电池电量显示: 充电且满电图标\n");
            }
            if (guider_ui.screen_dropdown_img_fully_battery) {
                lv_obj_clear_flag(guider_ui.screen_dropdown_img_fully_battery, LV_OBJ_FLAG_HIDDEN);
                printf("电池电量显示: 充电且满电图标\n");
            }
        } else {  // 充电中但未充满
            if (guider_ui.screen_main_img_charging_battery) {
                lv_obj_clear_flag(guider_ui.screen_main_img_charging_battery, LV_OBJ_FLAG_HIDDEN);
                printf("电池电量显示: 充电图标\n");
            }
            if (guider_ui.screen_dropdown_img_charging_battery) {
                lv_obj_clear_flag(guider_ui.screen_dropdown_img_charging_battery, LV_OBJ_FLAG_HIDDEN);
                printf("电池电量显示: 充电图标\n");
            }
        }
    } else {  // 未充电
        if (percent <= 22) {  // 低电量
            if (guider_ui.screen_main_img_low_battery) {
                lv_obj_clear_flag(guider_ui.screen_main_img_low_battery, LV_OBJ_FLAG_HIDDEN);
                printf("电池电量显示: 低电量图标 (%d%%)\n", percent);
            }
            if (guider_ui.screen_dropdown_img_low_battery) {
                lv_obj_clear_flag(guider_ui.screen_dropdown_img_low_battery, LV_OBJ_FLAG_HIDDEN);
                printf("电池电量显示: 低电量图标 (%d%%)\n", percent);
            }
        } else {  // 正常电量
            if (guider_ui.screen_main_img_normol_battery) {
                lv_obj_clear_flag(guider_ui.screen_main_img_normol_battery, LV_OBJ_FLAG_HIDDEN);
                printf("电池电量显示: 正常图标 (%d%%)\n", percent);
            }
            if (guider_ui.screen_dropdown_img_normal_battery) {
                lv_obj_clear_flag(guider_ui.screen_dropdown_img_normal_battery, LV_OBJ_FLAG_HIDDEN);
                printf("电池电量显示: 正常图标 (%d%%)\n", percent);
            }
        }
    }
}

// 电池电量定时器回调函数 - 每分钟更新一次
void battery_update_timer_cb(lv_timer_t *timer) {
    // 获取并更新电池电量显示
    extern int sys_get_vbat_percent(void);
    u8 battery_percent = sys_get_vbat_percent();
    update_battery_display(battery_percent);
    printf("定时更新电池电量: %d%%\n", battery_percent);
}


static bool already_notified_full = false;
static int chargfull_dete(void);
static void charge_status_check(void);
// 初始化充电状态检测
void charge_status_init(void) {
    // 配置GPIO为输入模式
    gpio_direction_input(TCFG_CHARING_STATUS_PORT);
    gpio_set_die(TCFG_CHARING_STATUS_PORT, 1);
    gpio_set_pull_up(TCFG_CHARING_STATUS_PORT, 0);
    gpio_set_pull_down(TCFG_CHARING_STATUS_PORT, 0);

    gpio_direction_input(TCFG_CHARGFULL_STATUS_PORT);
    gpio_set_die(TCFG_CHARGFULL_STATUS_PORT, 1);
    gpio_set_pull_up(TCFG_CHARGFULL_STATUS_PORT, 0);
    gpio_set_pull_down(TCFG_CHARGFULL_STATUS_PORT, 1);

    // 启动状态检测任务
    sys_timer_add_to_task("sys_timer", NULL, charge_status_check, 500);
    sys_timer_add_to_task("sys_timer", NULL, chargfull_dete, 50);
}


// 移除满电检测函数中的图标操作代码
static int chargfull_dete(void)
{
    static char dete_full_cnt = 0;
    char dete_full = gpio_read(TCFG_CHARGFULL_STATUS_PORT);

    if(dete_full){
        dete_full_cnt++;
        if(dete_full_cnt > 30){//大于1.5秒
            chargfull = true;
            chargfull = true;
//            printf("充满电充满电\n");
            // 移除直接操作图标的代码
        }
    }else{
        dete_full_cnt = 0;
    }

    return 0;
}
// 修改充电状态检查函数，移除图标操作代码
static void charge_status_check(void) {
    int charging_status = gpio_read(TCFG_CHARING_STATUS_PORT);
    static char last_charging_status = -1;
    static char last_raw_status = -1;
    static char debounce_cnt = 0;
    static char pull_down_close = 0;

    if (charging_status && !pull_down_close) {
        pull_down_close = 1;
        gpio_set_pull_down(TCFG_CHARGFULL_STATUS_PORT, 0);
    } else if (!charging_status && pull_down_close) {
        pull_down_close = 0;
        gpio_set_pull_down(TCFG_CHARGFULL_STATUS_PORT, 1);
    }
    // 防抖处理
    if (last_raw_status != charging_status) {
        debounce_cnt = 1;
        last_raw_status = charging_status;
    } else {
        debounce_cnt++;
    }

    // 未达到防抖阈值，不更新状态
    if (debounce_cnt < DEBOUNCE_THRESHOLD) {
        return;
    }

    if (charging_status) {  // 高电平 = 正在充电
        // 只在检测到充电器刚插入时唤醒屏幕，之后允许正常进入低功耗
        if (last_charging_status != charging_status) {
            extern int auto_sleep_check_clear(void);
            auto_sleep_check_clear();
            music_play_res_file("Charging.mp3");
        }
        // 如果已充满，且还没有播报过
        if (chargfull && !already_notified_full) {
            already_notified_full = true;
            printf("[充电检测] 电池已充满\n");
            music_play_res_file("ChargeFull.mp3");
            // 移除直接操作图标的代码
        }
        // 移除直接操作充电图标的代码
    } else {  // 低电平 = 没插电
        //printf("[充电检测]低电平 = 没插电\n");
        // 移除直接操作图标的代码

        // 一旦拔掉充电器，下次再插入可以重新提示
        already_notified_full = false;
        chargfull = 0;
    }

    // 每次状态变化时更新电池显示
    if (last_charging_status != charging_status) {
        extern int sys_get_vbat_percent(void);
        update_battery_display(sys_get_vbat_percent());
    }

    last_charging_status = charging_status;
}

int is_charging_now(void) {
    return gpio_read(TCFG_CHARING_STATUS_PORT);  // 高电平表示在充电
}
#endif



// 音乐按钮点击事件处理函数
static void btn_music_click_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if(!ui || !ui->screen_main_btn_music){
        return;
    }
    if(code == LV_EVENT_CLICKED) {
        printf("音乐按钮被点击，跳转到音乐页面\n");
#ifdef APPLAYER_ENABLE
        if(audio_app_mode_check() == -1){
            return;
        }else if(audio_app_mode_check() == 0){
            switch_to_music_page("net_music");
        }else if(audio_app_mode_check() == 1){
            switch_to_music_page("bt_music");
        }else if(audio_app_mode_check() == 2){
            switch_to_music_page("sd_music");
        }
#else
        switch_to_music_page("net_music");
#endif
    }
}

// 对话按钮点击事件处理函数
static void btn_conversation_click_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if(!ui || !ui->screen_main_btn_conversation){
        return;
    }
    if(code == LV_EVENT_CLICKED) {
        printf("对话按钮被点击，跳转到对话页面\n");
        switch_to_conversation_page();
    }
}

// 闹钟按钮点击事件处理函数
static void btn_alarm_click_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if(!ui || !ui->screen_main_btn_alarm){
        return;
    }
    if(code == LV_EVENT_CLICKED) {
        printf("闹钟按钮被点击，跳转到闹钟页面\n");
        switch_to_alarm_page();
    }
}

// 倒计时按钮点击事件处理函数
static void btn_timer_click_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    if(!ui || !ui->screen_main_btn_timer){
        return;
    }
    if(code == LV_EVENT_CLICKED) {
        printf("倒计时按钮被点击，跳转到倒计时页面\n");
        switch_to_timer_page();
    }
}

// 初始化主页面事件
void screen_main_event_init(lv_ui *ui) {
    if (!ui) return;
    
    lv_obj_set_scroll_dir(guider_ui.screen_main, LV_DIR_NONE);
    // 配置容器，确保它不会拦截子按钮的点击事件
    if (ui->screen_main_cont) {
        // 设置容器为不可点击状态，让事件能够传递给子元素
        lv_obj_clear_flag(ui->screen_main_cont, LV_OBJ_FLAG_CLICKABLE);
        printf("容器配置完成，确保不拦截子按钮事件\n");
    }
    
    // 先移除可能已存在的事件处理函数，防止多次注册
    if (ui->screen_main_btn_music) {
        lv_obj_remove_event_cb(ui->screen_main_btn_music, btn_music_click_handler);
    }
    if (ui->screen_main_btn_conversation) {
        lv_obj_remove_event_cb(ui->screen_main_btn_conversation, btn_conversation_click_handler);
    }
    if (ui->screen_main_btn_alarm) {
        lv_obj_remove_event_cb(ui->screen_main_btn_alarm, btn_alarm_click_handler);
    }
    if (ui->screen_main_btn_timer) {
        lv_obj_remove_event_cb(ui->screen_main_btn_timer, btn_timer_click_handler);
    }
    
    // 为音乐按钮绑定点击事件
    if (ui->screen_main_btn_music) {
        lv_obj_add_event_cb(ui->screen_main_btn_music, btn_music_click_handler, LV_EVENT_CLICKED, ui);
        printf("音乐按钮点击事件绑定完成\n");
    }
    
    // 为对话按钮绑定点击事件
    if (ui->screen_main_btn_conversation) {
        lv_obj_add_event_cb(ui->screen_main_btn_conversation, btn_conversation_click_handler, LV_EVENT_CLICKED, ui);
        printf("对话按钮点击事件绑定完成\n");
    }
    
    // 为闹钟按钮绑定点击事件
    if (ui->screen_main_btn_alarm) {
        lv_obj_add_event_cb(ui->screen_main_btn_alarm, btn_alarm_click_handler, LV_EVENT_CLICKED, ui);
        printf("闹钟按钮点击事件绑定完成\n");
    }
    
    // 为倒计时按钮绑定点击事件
    if (ui->screen_main_btn_timer) {
        lv_obj_add_event_cb(ui->screen_main_btn_timer, btn_timer_click_handler, LV_EVENT_CLICKED, ui);
        printf("倒计时按钮点击事件绑定完成\n");
    }
}

void screen_main_event_setup(void){
    screen_main_event_init(&guider_ui);
}
 
