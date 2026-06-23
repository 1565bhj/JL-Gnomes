#ifndef WIFI_COMMON_H
#define WIFI_COMMON_H

// 控制生成的WiFi容器个数，最多10个
#define MAX_WIFI_COUNT 10

// 控制是否默认隐藏已连接的WiFi容器，1为不隐藏，0为隐藏
#define DEFAULT_SHOW_CONNECTED_WIFI 0

// WiFi连接状态枚举
enum wifi_sta_connect_state {
    WIFI_STA_DISCONNECT,
    WIFI_STA_CONNECT_SUCC,
    WIFI_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID,
    WIFI_STA_CONNECT_ASSOCIAT_FAIL,
    WIFI_STA_CONNECT_ASSOCIAT_TIMEOUT,
    WIFI_STA_NETWORK_STACK_DHCP_SUCC,
    WIFI_STA_NETWORK_STACK_DHCP_TIMEOUT,
};

// WiFi扫描结果结构体
typedef struct {
    char ssid[32];
} wifi_scan_result_t;



// 外部变量声明
extern wifi_scan_result_t wifi_scan_results[MAX_WIFI_COUNT];
extern int current_wifi_count;
extern char current_wifi_name[256];
extern int current_wifi_index;
extern const char *wifi_passwords[];
extern char actual_wifi_names[20][32];
extern char actual_wifi_num;

// 外部函数声明
extern int wifi_enter_sta_mode(char *sta_ssid, char *sta_pwd);
/** save=1：DHCP 成功后将当前 SSID/密码写入 syscfg，供下次上电自动连接 */
extern void wifi_sta_connect(char *ssid, char *pwd, char save);
extern enum wifi_sta_connect_state wifi_get_sta_connect_state(void);
extern _Bool scan_and_print_wifi_networks(void);
/* 扫描在 wifi 线程异步完成后，切回 LVGL 线程刷新列表 */
void wifi_show_request_refresh_after_scan(void);
/* STA DHCP 成功时更新 LVGL WiFi 已连界面（关闭 WiFi UI 宏时同上为空桩） */
void lvgl_init_password_success(char *init_wifi_name);


#endif /* WIFI_COMMON_H */
