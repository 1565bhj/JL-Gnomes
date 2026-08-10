#include "app_config.h"

#ifdef CONFIG_EXT_WIFI_ENABLE
#include "system/includes.h"
#include "device/device.h"
#include "wireless_ext/wifi_dev.h"
#include "wireless_ext/wifi_connect.h"
#include "lwip.h"
#include "lwip/sockets.h"
#include "asm/port_waked_up.h"
#include "event/net_event.h"
#include "event/event.h"
#include "init.h"
#include "wifi/wifi_connect.h"
#include "event/net_event.h"
#include "syscfg/syscfg_id.h"
#include "wifi_dev.h"

#define AP_TEST_MODE    (0)
#define STA_TEST_MODE   (1)
#define P2P_TEST_MODE   (2)
#define EXT_WIFI_TEST_MODE  STA_TEST_MODE

#define AP_SSID "QYAI_MiFi_"          //配置 AP模式的SSID前缀
#define AP_PWD  "88888888"            //配置 AP模式的密码

#define WIFI_APP_TASK_NAME "ext_wifi_app_task"

enum WIFI_APP_MSG_CODE {
    WIFI_MSG_TICK_1_SEC,
    WIFI_MSG_SMP_CFG_COMPLETED,
    WIFI_MSG_STA_SCAN_COMPLETED,
    WIFI_MSG_STA_NETWORK_STACK_DHCP_SUCC,
};

enum EXT_WIFI_APP_MSG_CODE {
    EXT_WIFI_SETUP_STATUS_INIT = 0,
    EXT_WIFI_SETUP_START,
    EXT_WIFI_SETUP_DISCON,
    EXT_WIFI_SETUP_NO_FOUND_SSID,
    EXT_WIFI_SETUP_ASSOCIAT_FAIL,
};

static char ext_wifi_ssid[64];
static char ext_wifi_pwd[33];
static char ext_wifi_setup_status = 0;
static char wifi_status = 0;
static void *wifi_dev = NULL;

extern unsigned int time_lapse(unsigned int *handle, unsigned int time_out);
extern u32 ext_wifi_get_upload_rate(void);
extern u32 ext_wifi_get_download_rate(void);
extern int wifi_module_is_init(void);
void ext_wifi_sta_info_save(char *ssid, char *pwd);
int init_net_device_mac_addr(char *macaddr, char ap_mode); //如果AP模式需要配置SSID和MAC地址相关,需要在这里先产生MAC地址

enum wifi_sta_connect_state wifi_get_sta_connect_state(void)
{
    return wifi_status;
}

static void wifi_app_timer_func(void *p)
{
    if (wifi_module_is_init()) {
        printf("WIFI U= %d KB/s, D= %d KB/s\r\n", ext_wifi_get_upload_rate() / 1024, ext_wifi_get_download_rate() / 1024);
        malloc_stats();
#if 0
        if (ext_get_cur_wifi_info()->mode == STA_MODE) {
            get_rx_signal();
        }
#endif
    }
}

static void ext_wifi_discon(void)
{
    struct net_event net = {0};
    if (ext_wifi_setup_status == EXT_WIFI_SETUP_START) {//启动之后又DISCON：密码错误或没有WiFi
        net.arg = "net";
        net.event = NET_CONNECT_ASSOCIAT_FAIL;
        net_event_notify(NET_EVENT_FROM_USER, &net);
    }
    ext_wifi_setup_status = 0;
}

static int network_user_callback(void *network_ctx, enum WIFI_EVENT state, void *priv)
{
    struct cfg_info info = {0};
    struct net_event net = {0};
    u8 mac_addr[6];
    char ssid[48];

    switch (state) {
    case WIFI_EVENT_MODULE_INIT:
        puts("|ext_network_user_callback->WIFI_EVENT_MODULE_INIT\n");
        wifi_status = 0;
#ifndef CONFIG_AIC8800D40_ENABLE
//wifi module port seting
        info.port_status = 0;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);
        msleep(10);
        info.port_status = 1;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);

        info.port_status = 1;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_CS, (u32)&info);

        info.port_status = 1;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_WKUP, (u32)&info);

        msleep(100);

        init_net_device_mac_addr((char *)mac_addr, 1);
        sprintf((char *)ssid, AP_SSID"%02x%02x%02x", mac_addr[0], mac_addr[1], mac_addr[2]);

        info.mode = AP_MODE;
        info.ssid = ssid;
        info.pwd  = AP_PWD;
        info.force_default_mode = 1;
        dev_ioctl(wifi_dev, DEV_SET_DEFAULT_MODE, (u32)&info);
#endif
        break;

    case WIFI_EVENT_MODULE_START:
        puts("|ext_network_user_callback->WIFI_EVENT_MODULE_START\n");
#ifndef CONFIG_AIC8800D40_ENABLE

        init_net_device_mac_addr((char *)mac_addr, 1);
        sprintf((char *)ssid, AP_SSID"%02x%02x%02x", mac_addr[0], mac_addr[1], mac_addr[2]);

        info.mode = AP_MODE;
        info.ssid = ssid;
        info.pwd  = AP_PWD;
        info.force_default_mode = 1;
        dev_ioctl(wifi_dev, DEV_SAVE_DEFAULT_MODE, (u32)&info);

        wpa_supplicant_set_connect_timeout(30);
#endif
        wifi_status = 0;
        break;

    case WIFI_EVENT_MODULE_STOP:
        puts("|ext_network_user_callback->WIFI_EVENT_MODULE_STOP\n");
        net.arg = "net";
        net.event = NET_EVENT_STOP;
        net_event_notify(NET_EVENT_FROM_USER, &net);
        ext_wifi_setup_status = 0;
        break;

    case WIFI_EVENT_AP_START:
        puts("|ext_network_user_callback->WIFI_EVENT_AP_START\n");
        break;

    case WIFI_EVENT_AP_STOP:
        puts("|ext_network_user_callback->WIFI_EVENT_AP_STOP\n");
        break;

    case WIFI_EVENT_STA_START:
        puts("|ext_network_user_callback->WIFI_EVENT_STA_START\n");
        ext_wifi_setup_status = EXT_WIFI_SETUP_START;
        break;

    case WIFI_EVENT_MODULE_START_ERR:
        puts("|ext_network_user_callback->WIFI_EVENT_MODULE_START_ERR\n");
        break;

    case WIFI_EVENT_STA_STOP:
        puts("|ext_network_user_callback->WIFI_EVENT_STA_STOP\n");
        break;

    case WIFI_EVENT_STA_DISCONNECT:
        puts("|ext_network_user_callback->WIFI_STA_DISCONNECT\n");
        sys_timeout_add_to_task("sys_timer", NULL, ext_wifi_discon, 1000);
        break;

    case WIFI_EVENT_STA_SCAN_COMPLETED:
        puts("|ext_network_user_callback->WIFI_STA_SCAN_COMPLETED\n");
        break;

    case WIFI_EVENT_STA_CONNECT_SUCC:
        dev_ioctl(wifi_dev, DEV_GET_WIFI_CHANNEL, (u32)&info);
        printf("|ext_network_user_callback->WIFI_STA_CONNECT_SUCC,CH=%d\r\n", info.sta_channel);
        ext_wifi_setup_status = 0;

        ext_wifi_sta_info_save(ext_wifi_ssid, ext_wifi_pwd);

#ifdef CONFIG_FTP_SERVER_ENABLE
        void ftpd_server_init(const char *user, const char *pass, const char *ota_name, int fifo_size);
        ftpd_server_init("admin", "admin", "update-ota.ufw/update-ota-res.ufw", 4096);
#endif
        break;

    case WIFI_EVENT_MP_TEST_START:
        puts("|ext_network_user_callback->WIFI_EVENT_MP_TEST_START\n");
        break;

    case WIFI_EVENT_MP_TEST_STOP:
        puts("|ext_network_user_callback->WIFI_EVENT_MP_TEST_STOP\n");
        break;

    case WIFI_EVENT_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID:
        puts("|ext_network_user_callback->WIFI_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID\n");
        net.arg = "net";
        net.event = NET_CONNECT_TIMEOUT_NOT_FOUND_SSID;
        net_event_notify(NET_EVENT_FROM_USER, &net);
        ext_wifi_setup_status = 0;
        break;

    case WIFI_EVENT_STA_CONNECT_ASSOCIAT_FAIL:
        puts("|ext_network_user_callback->WIFI_STA_CONNECT_ASSOCIAT_FAIL\n");
        net.arg = "net";
        net.event = NET_CONNECT_ASSOCIAT_FAIL;
        net_event_notify(NET_EVENT_FROM_USER, &net);
        ext_wifi_setup_status = 0;
        break;

    case WIFI_EVENT_STA_CONNECT_ASSOCIAT_TIMEOUT:
        puts("|ext_network_user_callback->WIFI_EVENT_STA_CONNECT_ASSOCIAT_TIMEOUT\n");
        net.arg = "net";
        net.event = NET_CONNECT_ASSOCIAT_TIMEOUT;
        net_event_notify(NET_EVENT_FROM_USER, &net);
        ext_wifi_setup_status = 0;
        break;

    case WIFI_EVENT_STA_NETWORK_STACK_DHCP_SUCC:
        wifi_status = WIFI_MSG_STA_NETWORK_STACK_DHCP_SUCC;
        puts("|ext_network_user_callback->WIFI_EVENT_STA_NETWPRK_STACK_DHCP_SUCC\n");

        net_dhcp_status_set(NET_DHCP_SUCCESS);

#ifdef CONFIG_NO_SDRAM_ENABLE
        net.arg = "net";
        net.event = NET_EVENT_CONNECTED;
        net_event_notify(NET_EVENT_FROM_USER, &net);
#endif
        ext_wifi_setup_status = 0;
        break;

    case WIFI_EVENT_STA_NETWORK_STACK_DHCP_TIMEOUT:
        puts("|ext_network_user_callback->WIFI_EVENT_STA_NETWPRK_STACK_DHCP_TIMEOUT\n");
        net.arg = "net";
        net.event = NET_CONNECT_DHCP_TIMEOUT;
        net_event_notify(NET_EVENT_FROM_USER, &net);
        ext_wifi_setup_status = 0;
        break;

    case WIFI_EVENT_P2P_START:
        puts("|ext_network_user_callback->WIFI_EVENT_P2P_START\n");
        break;

    case WIFI_EVENT_P2P_STOP:
        puts("|ext_network_user_callback->WIFI_EVENT_P2P_STOP\n");
        break;

    case WIFI_EVENT_P2P_GC_DISCONNECTED:
        puts("|ext_network_user_callback->WIFI_EVENT_P2P_GC_DISCONNECTED\n");
#ifdef CONFIG_AIC8800D40_ENABLE
        wlan_enable_p2p(0);
#endif
        break;

    case WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_SUCC:
        puts("|ext_network_user_callback->WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_SUCC\n");
        break;

    case WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_TIMEOUT:
        puts("|ext_network_user_callback->WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_TIMEOUT\n");
        break;

    case WIFI_EVENT_SMP_CFG_TIMEOUT:
        puts("|ext_network_user_callback->WIFI_EVENT_SMP_CFG_TIMEOUT\n");
        net.arg = "net";
        net.event = NET_EVENT_SMP_CFG_TIMEOUT;
        net_event_notify(NET_EVENT_FROM_USER, &net);
        ext_wifi_setup_status = 0;
        break;

    case WIFI_EVENT_SMP_CFG_COMPLETED:
        puts("|ext_network_user_callback->WIFI_EVENT_SMP_CFG_COMPLETED\n");
        net.arg = "net";
        net.event = NET_SMP_CFG_COMPLETED;
        net_event_notify(NET_EVENT_FROM_USER, &net);
        ext_wifi_setup_status = 0;
        break;

    case WIFI_EVENT_PM_SUSPEND:
        puts("|ext_network_user_callback->WIFI_EVENT_PM_SUSPEND\n");
        break;

    case WIFI_EVENT_PM_RESUME:
        puts("|ext_network_user_callback->WIFI_EVENT_PM_RESUME\n");
        break;

    case WIFI_EVENT_AP_ON_ASSOC:
        struct eth_addr *hwaddr = (struct eth_addr *)network_ctx;
        printf("|ext_network_user_callback->WIFI_EVENT_AP_ON_ASSOC hwaddr = %02x:%02x:%02x:%02x:%02x:%02x \r\n\r\n",
               hwaddr->addr[0], hwaddr->addr[1], hwaddr->addr[2], hwaddr->addr[3], hwaddr->addr[4], hwaddr->addr[5]);
#if IP_NAPT_EXT && defined CONFIG_LTE_PHY_ENABLE
        extern void check_dhcps_client_ipaddr(u8 * hwaddr);
        check_dhcps_client_ipaddr(hwaddr->addr);
#endif
        break;

    case WIFI_EVENT_AP_ON_DISCONNECTED:
        hwaddr = (struct eth_addr *)network_ctx;
        printf("|ext_network_user_callback->WIFI_EVENT_AP_ON_DISCONNECTED hwaddr = %02x:%02x:%02x:%02x:%02x:%02x \r\n\r\n",
               hwaddr->addr[0], hwaddr->addr[1], hwaddr->addr[2], hwaddr->addr[3], hwaddr->addr[4], hwaddr->addr[5]);
        break;
    case WIFI_EVENT_STA_IP_GOT_IPV6_SUCC:
        printf("network_user_callback->WIFI_EVENT_STA_IP_GOT_IPV6_SUCC");
        break;

    default:
        break;
    }

    return 0;
}


int ext_lwip_event_user_cb(void *lwip_ctx, enum LWIP_EVENT event)
{
    switch (event) {
    case LWIP_EXT_WIRELESS_DHCP_BOUND_TIMEOUT:
        network_user_callback(NULL, WIFI_EVENT_STA_NETWORK_STACK_DHCP_TIMEOUT, NULL);
        break;
    case LWIP_EXT_WIRELESS_DHCP_BOUND_SUCC:
        network_user_callback(NULL, WIFI_EVENT_STA_NETWORK_STACK_DHCP_SUCC, NULL);
        break;
    }
    return 0;
}

static void wifi_set_lan_setting_info(void)
{
    struct lan_setting lan_setting_info = {
        .WIRELESS_IP_ADDR0  = 192,
        .WIRELESS_IP_ADDR1  = 168,
        .WIRELESS_IP_ADDR2  = 1,
        .WIRELESS_IP_ADDR3  = 1,

        .WIRELESS_NETMASK0  = 255,
        .WIRELESS_NETMASK1  = 255,
        .WIRELESS_NETMASK2  = 255,
        .WIRELESS_NETMASK3  = 0,

        .WIRELESS_GATEWAY0  = 192,
        .WIRELESS_GATEWAY1  = 168,
        .WIRELESS_GATEWAY2  = 1,
        .WIRELESS_GATEWAY3  = 1,

        .SERVER_IPADDR1  = 192,
        .SERVER_IPADDR2  = 168,
        .SERVER_IPADDR3  = 1,
        .SERVER_IPADDR4  = 1,

        .CLIENT_IPADDR1  = 192,
        .CLIENT_IPADDR2  = 168,
        .CLIENT_IPADDR3  = 1,
        .CLIENT_IPADDR4  = 2,

        .SUB_NET_MASK1   = 255,
        .SUB_NET_MASK2   = 255,
        .SUB_NET_MASK3   = 255,
        .SUB_NET_MASK4   = 0,
    };

    struct cfg_info info;
    info.__lan_setting_info = &lan_setting_info;
    dev_ioctl(wifi_dev, DEV_SET_LAN_SETTING, (u32)&info);
}


void ext_wifi_on(void)
{
    if (!wifi_dev) {
        return;
    }
    dev_ioctl(wifi_dev, DEV_NETWORK_START, 0);
}


void ext_wifi_off(void)
{
    if (!wifi_dev) {
        return;
    }
    dev_ioctl(wifi_dev, DEV_NETWORK_STOP, 0);
}

void ext_wifi_sta_connect(char *ssid, char *pwd, char save)
{
    struct cfg_info info = {0};
    if (!ssid || !wifi_dev) {
        return;
    }
    if (ext_wifi_ssid != ssid && ext_wifi_pwd != pwd) {
        memset(ext_wifi_ssid, 0, sizeof(ext_wifi_ssid));
        memset(ext_wifi_pwd, 0, sizeof(ext_wifi_pwd));
        strncpy(ext_wifi_ssid, ssid, sizeof(ext_wifi_ssid));
        if (pwd) {
            strncpy(ext_wifi_pwd, pwd, sizeof(ext_wifi_pwd));
        }
    }
    info.mode = STA_MODE;
    info.ssid = ext_wifi_ssid;
    info.pwd  = ext_wifi_pwd;
    info.force_default_mode = 0;
    dev_ioctl(wifi_dev, DEV_STA_MODE, (u32)&info);
}

void ext_wifi_sta_reconnect(void)
{
    if (!wifi_dev) {
        return;
    }
    struct cfg_info info = {0};
    info.mode = STA_MODE;
    dev_ioctl(wifi_dev, DEV_GET_CUR_WIFI_INFO, (u32)&info);
    if (info.ssid && info.ssid[0]) {
        dev_ioctl(wifi_dev, DEV_STA_MODE, (u32)&info);
    }
}
#define NONE_SSID "*#_~`^"
void ext_wifi_sta_info_save(char *ssid, char *pwd)
{
    if (!ssid || !wifi_dev) {
        return;
    }
    struct wifi_store_info default_store_info = {0};
    char index = STA_MODE - STA_MODE;
    default_store_info.mode = STA_MODE;
    syscfg_read(WIFI_INFO_IDX, (char *)&default_store_info, sizeof(struct wifi_store_info));

    if (!strcmp(default_store_info.ssid[index], ssid) && (!pwd || (!strcmp(default_store_info.pwd[index], pwd)))) {
        return;
    }

    struct wifi_stored_sta_info sta_info_get = {0};
    for (int i = 0; i < NETWORK_SSID_INFO_CNT; i++) {
        if (syscfg_read(WIFI_STA_INFO_IDX_START + i, (char *)&sta_info_get, sizeof(struct wifi_stored_sta_info)) >= 0) {
            printf("-> read wifi index %d , %s\n", i, &sta_info_get.ssid);
            if (!strcmp(&sta_info_get.ssid, ssid)) {
                if (pwd) {
                    strncpy(&sta_info_get.pwd, pwd, sizeof(sta_info_get.pwd));
                } else {
                    memset(&sta_info_get.pwd, 0, sizeof(sta_info_get.pwd));
                }
                syscfg_write(WIFI_STA_INFO_IDX_START + i, (char *)&sta_info_get, sizeof(struct wifi_stored_sta_info));
                printf("\n-> save wifi index %d , %s\n", i, ssid);
                goto __save;
            }
        }
    }
    for (int i = 0; i < NETWORK_SSID_INFO_CNT; i++) {
        syscfg_read(WIFI_STA_INFO_IDX_START + i, (char *)&sta_info_get, sizeof(struct wifi_stored_sta_info));
        if (!strcmp(&sta_info_get.ssid, NONE_SSID) || sta_info_get.ssid[0] == 0 || sta_info_get.ssid[0] == 0xFF) {
            strcpy(&sta_info_get.ssid, ssid);
            if (pwd) {
                strncpy(&sta_info_get.pwd, pwd, sizeof(sta_info_get.pwd));
            } else {
                memset(&sta_info_get.pwd, 0, sizeof(sta_info_get.pwd));
            }
            syscfg_write(WIFI_STA_INFO_IDX_START + i, (char *)&sta_info_get, sizeof(struct wifi_stored_sta_info));
            printf("-> save wifi index %d , %s\n", i, ssid);
            goto __save;
        }
    }
    strcpy(&sta_info_get.ssid, ssid);
    if (pwd) {
        strncpy(&sta_info_get.pwd, pwd, sizeof(sta_info_get.pwd));
    } else {
        memset(&sta_info_get.pwd, 0, sizeof(sta_info_get.pwd));
    }
    syscfg_write(WIFI_STA_INFO_IDX_START, (char *)&sta_info_get, sizeof(struct wifi_stored_sta_info));

__save:
    ;
    struct cfg_info info = {0};
    info.mode = STA_MODE;
    info.ssid = ext_wifi_ssid;
    info.pwd  = ext_wifi_pwd;
    info.force_default_mode = 1;
    dev_ioctl(wifi_dev, DEV_SAVE_DEFAULT_MODE, (u32)&info);

    struct wifi_store_info store_info = {0};
    index = STA_MODE - STA_MODE;
    store_info.mode = STA_MODE;
    store_info.connect_best_network = true;
    strncpy(&store_info.ssid[index], ssid, sizeof(store_info.ssid[index]));
    strncpy(&store_info.pwd[index], pwd, sizeof(store_info.pwd[index]));
    syscfg_write(WIFI_INFO_IDX, (char *)&store_info, sizeof(struct wifi_store_info));
}

int ext_wifi_sta_info_default_get(void)
{
    struct wifi_store_info store_info = {0};
    char index = STA_MODE - STA_MODE;
    if (syscfg_read(WIFI_INFO_IDX, (char *)&store_info, sizeof(struct wifi_store_info)) >= 0) {
        if (!store_info.ssid[index] || store_info.ssid[index] == 0xFF || !strcmp(&store_info.ssid[index], NONE_SSID)) {
            return -1;
        }
        memset(ext_wifi_ssid, 0, sizeof(ext_wifi_ssid));
        memset(ext_wifi_pwd, 0, sizeof(ext_wifi_pwd));
        strncpy(ext_wifi_ssid, &store_info.ssid[index], sizeof(ext_wifi_ssid));
        strncpy(ext_wifi_pwd, &store_info.pwd[index], sizeof(ext_wifi_pwd));
        printf("-> read wifi default mode ssid = %s, pwd = %s \n", &store_info.ssid[index], &store_info.pwd[index]);
        return 0;
    }
    return -1;
}

void ext_wifi_sta_info_clear(char *ssid)
{
    int net_ch = 0;
    struct wifi_stored_sta_info sta_info_get = {0};
    for (int i = 0; i < NETWORK_SSID_INFO_CNT; i++) {
        if (syscfg_read(WIFI_STA_INFO_IDX_START + i, (char *)&sta_info_get, sizeof(struct wifi_stored_sta_info)) >= 0) {
            if (!ssid || (ssid && !strcmp(&sta_info_get.ssid, ssid))) {
                strcpy(&sta_info_get.ssid, NONE_SSID);
                syscfg_write(WIFI_STA_INFO_IDX_START + i, (char *)&sta_info_get, sizeof(struct wifi_stored_sta_info));
                printf("-> reset wifi vm index %d \n", i);
            }
        }
    }
    struct wifi_store_info store_info = {0};
    if (syscfg_read(WIFI_INFO_IDX, (char *)&store_info, sizeof(struct wifi_store_info)) >= 0) {
        strcpy(&store_info.ssid[STA_MODE - STA_MODE], NONE_SSID);
        syscfg_write(WIFI_INFO_IDX, (char *)&store_info, sizeof(struct wifi_store_info));
    }
}

#if CONFIG_LVGL_OPTION_WIFI_ENABLE
// 存储WiFi名称的数组
static u8 actual_wifi_names[20][64] = {0};  // 初始化避免脏数据
static u8 actual_wifi_num = 0;  // 初始化避免脏数据
// 扫描状态标志
static bool is_scanning = false;
/* UI 触发的扫描：在 wifi_app_task 里发起 wifi_scan_req + 周期取结果（与 demo_wifi_scan_req_start 一致，避免在 LVGL 线程里阻塞/调用栈不对） */
static u16 s_ui_scan_kickoff_timeout_id = 0;
static u8 s_ui_scan_poll_count = 0;

int get_wifi_scan_ssid_num(void)
{
    return actual_wifi_num;
}

char *get_wifi_scan_ssid_name(int index)
{
    if (index < 0 || index >= actual_wifi_num) {
        return NULL; // 索引越界，返回 NULL
    }
    return &actual_wifi_names[index][0];
}

static int get_wifi_scan_ssid_info(void)
{
    struct wifi_scan_ssid_info *sta_ssid_info;
    u32 sta_ssid_num = 0;
//    sta_ssid_info = wifi_get_scan_result(&sta_ssid_num);

    // 存储WiFi名称到数组
    memset(actual_wifi_names, 0, sizeof(actual_wifi_names)); // 清空数组，避免残留数据
    for (u32 i = 0; i < sta_ssid_num; i++) {
        strncpy(actual_wifi_names[i], sta_ssid_info[i].ssid, sizeof(actual_wifi_names[i]) - 1);
        actual_wifi_names[i][sizeof(actual_wifi_names[i]) - 1] = '\0';
    }

    printf("\n=============================================================\n");
    printf("扫描到 %d 个WiFi网络:\n", sta_ssid_num);
    actual_wifi_num = sta_ssid_num;
    printf("=============================================================\n");

    // 打印 actual_wifi_names 数组中的所有WiFi名称
    printf("actual_wifi_names 数组内容:\n");
    for (u32 i = 0; i < sta_ssid_num; i++) {
        printf("actual_wifi_names[%d]: %s\n", i, actual_wifi_names[i]);
    }
    printf("=============================================================\n");

    for (u32 i = 0; i < sta_ssid_num; i++) {
        printf("WiFi %d:\n", i + 1);
        printf("  SSID: %s\n", sta_ssid_info[i].ssid);
        printf("  MAC地址: %02X:%02X:%02X:%02X:%02X:%02X\n",
               sta_ssid_info[i].mac_addr[0], sta_ssid_info[i].mac_addr[1],
               sta_ssid_info[i].mac_addr[2], sta_ssid_info[i].mac_addr[3],
               sta_ssid_info[i].mac_addr[4], sta_ssid_info[i].mac_addr[5]);
        printf("  信号强度: %d dBm\n", sta_ssid_info[i].rssi_db);
        printf("  信道: %d\n", sta_ssid_info[i].channel_number);
        if (sta_ssid_info[i].auth_mode == WIFI_AUTH_MODE_OPEN) {
            printf("  加密方式: 无加密\n");
        } else {
            printf("  加密方式: 加密\n");
        }
        printf("-------------------------------------------------------------\n");
    }

    /* 仅在有结果时清空缓存；过早清空会导致未联网时当次扫描永远拿不到列表（与 demo_wifi_ext 说明一致） */
    if (sta_ssid_num > 0) {
        wifi_clear_scan_result();
    }
    return sta_ssid_num;
}
/* 在 wifi_app_task 中周期执行，与 demo_wifi_get_scan_result 一致 */
static void wifi_scan_poll(void *priv)
{
    u32 sta_ssid_num = 0;

    (void)priv;
    s_ui_scan_poll_count++;

//    wifi_get_scan_result(&sta_ssid_num);
    printf("---> ui_wifi_scan_poll ssid_cnt : %u (tick %u)\n", sta_ssid_num, s_ui_scan_poll_count);

    if (sta_ssid_num > 0) {
        get_wifi_scan_ssid_info();
//        wifi_show_request_refresh_after_scan();
        is_scanning = false;

    } else if (s_ui_scan_poll_count >= 60) {
        get_wifi_scan_ssid_info();
        wifi_show_request_refresh_after_scan();
        is_scanning = false;
    }
    if (!is_scanning && s_ui_scan_kickoff_timeout_id) {
        sys_timer_del(s_ui_scan_kickoff_timeout_id);
        s_ui_scan_kickoff_timeout_id = 0;
    }
}

bool scan_and_print_wifi_networks(void)
{
    if (is_scanning || !wifi_dev) {
        return false;
    }
    is_scanning = true;
    s_ui_scan_poll_count = 0;
    /* 取消上一轮未结束的定时器 */
    if (s_ui_scan_kickoff_timeout_id) {
        sys_timer_del(s_ui_scan_kickoff_timeout_id);
        s_ui_scan_kickoff_timeout_id = 0;
    }


    dev_ioctl(wifi_dev, DEV_NET_SCANF, NULL);

    /*
     * 与 demo_wifi_scan_req_start 相同思路：在 wifi_app_task 上下文里 wifi_scan_req，
     * 再用 sys_timer 每 1s 取结果（见 system/timer.h：回调在发起 add 的同一线程执行，
     * LVGL 里阻塞 os_time_dly 会卡住界面且易导致扫描/协议栈时序异常）。
     */
    s_ui_scan_kickoff_timeout_id = sys_timer_add_to_task("sys_timer", NULL, wifi_scan_poll, 3000);
    if (s_ui_scan_kickoff_timeout_id == 0) {
        printf("scan: sys_timeout_add_to_task(wifi_app_task) 失败\n");
        is_scanning = false;
        return false;
    }

    return true;
}

void wifi_sta_connect(char *ssid, char *pwd, char save)
{
    ext_wifi_sta_connect(ssid, pwd, save);
}

void wifi_sta_info_clear_ssid(char *ssid)
{
    ext_wifi_sta_info_clear(ssid);
}

int wifi_enter_sta_mode(char *ssid, char *pwd)
{
    ext_wifi_sta_connect(ssid, pwd, 0);
}
#endif


static void ext_wifi_app_task(void *priv)
{
    int msg[32];
    int res;
    unsigned int timehdl = 0;
    struct cfg_info info = {0};
    struct net_event net = {0};

    os_time_dly(100);//等待电源问题
    //wifi app start
    wifi_dev = dev_open("wifi", NULL);
    if (!wifi_dev) {
        return;
    }
    info.cb = network_user_callback;
    info.net_priv = NULL;
    dev_ioctl(wifi_dev, DEV_SET_CB_FUNC, (u32)&info);

#if 0
    printf("\n >>>> DEV_SET_WIFI_POWER_SAVE<<<<   \n");
    dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER_SAVE, 0);//打开就启用低功耗模式, 只有STA模式才有用
#endif

#ifndef CONFIG_AIC8800D40_ENABLE
    printf("\n >>>> DEV_SET_WIFI_TX_PWR_BY_RATE<<<   \n");

    info.tx_pwr_lmt_enable = 0;//  解除WIFI发送功率限制
    dev_ioctl(wifi_dev, DEV_SET_WIFI_TX_PWR_LMT_ENABLE, (u32)&info);
    info.tx_pwr_by_rate = 1;// 设置WIFI根据不同datarate打不同power
    dev_ioctl(wifi_dev, DEV_SET_WIFI_TX_PWR_BY_RATE, (u32)&info);
#endif

    wifi_set_lan_setting_info();

#if !IP_NAPT_EXT || !defined CONFIG_LTE_PHY_ENABLE
    net.arg = "net";
    if (!ext_wifi_sta_info_default_get()) {
        net.event = NET_NTP_GET_TIME_SUCC + 32;
    } else {
        net.event = NET_EVENT_SMP_CFG_FIRST;
        printf("---> NET_EVENT_SMP_CFG_FIRST\n");
    }
    net_event_notify(NET_EVENT_FROM_USER, &net);
    ext_wifi_on();
#endif

#ifdef CONFIG_IPERF_ENABLE
    //网络测试工具，使用iperf
    extern void iperf_test(void);
    iperf_test();
#endif

#if IP_NAPT
    u8 mac_addr[6];
    char ssid[48];
    init_net_device_mac_addr((char *)mac_addr, 1);
    sprintf((char *)ssid, AP_SSID"%02x%02x%02x", mac_addr[0], mac_addr[1], mac_addr[2]);

    info.mode = AP_MODE;
    info.ssid = ssid;
    info.pwd  = AP_PWD;
    info.force_default_mode = 1;
    dev_ioctl(wifi_dev, DEV_AP_MODE, (u32)&info);
#endif
    if (!ext_wifi_sta_info_default_get()) {
        ext_wifi_sta_connect(ext_wifi_ssid, ext_wifi_pwd, 0);
    }
    sys_timer_add_to_task("sys_timer", NULL, wifi_app_timer_func, 5000);
}


//sdio驱动底层调用接口

//设置指定IO的强驱
int get_sdio_hd_value(void)
{
    printf("sdio hd level set\n");
    return 1;
}

//设置高速卡
int get_sdio_hs_enable(void)
{
    return 0;
}

//返回Hi3861L用的edge
int SDIO_DAT_EDGE_GET(void)
{
    return 0;
}

//CTU模式下连续读写报错时回调，用于过滤错误的报错信息
int sdio_wr_err_cb(int crc_status)
{
    if (crc_status == 1) {
        return 0;
    }
    printf("\n >>>crc_status = %d \n", crc_status);
    return -1;
}

//AIC8800需要软件判忙
int get_sdio_tx_ctu_enable(void)
{
#ifdef CONFIG_AIC8800D40_ENABLE
    return 0;
#else
    return 1;
#endif
}

int get_sdio_rx_ctu_enable(void)
{
    return 1;
}

void port_wakeup_reg_set_gpio_cb(int event, unsigned int gpio, int edge)
{
    if (event != 0) {
        gpio_direction_input(gpio);
        gpio_set_die(gpio, 1);
        if (edge == 0) {
            gpio_set_pull_down(gpio, 1);
            gpio_set_pull_up(gpio, 0);
        } else if (edge == 1) {
            gpio_set_pull_down(gpio, 0);
            gpio_set_pull_up(gpio, 1);
        }
    }
}

static OS_SEM busy_sem;

static void sdio_wait_busy_isr(void *priv)
{
//    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>1");
    os_sem_post(&busy_sem);
}

char wifi_get_rssi(void)
{
    printf("err %s\n", __func__);
    return 0;
}

int ble_wifi_scan_request(void (*done_cb)(void))
{
    printf("err %s\n", __func__);
    return 0;
}

//软件判忙操作
void  sdio_wait_busy(int gpio)
{

    int ret;
    static void *port_wakeup_hdl = NULL;
    if (!os_sem_valid(&busy_sem)) {
        os_sem_create(&busy_sem, 0);
    }

    port_wakeup_hdl = port_wakeup_reg(EVENT_IO_0, gpio, 1, sdio_wait_busy_isr);//不能动到IO 配置

    if (gpio_read(gpio)) {
        goto __exit;
    } else {
        ret = os_sem_pend(&busy_sem, 10);
        if (ret) {
            while (1) {
                puts("BUSY");
                msleep(100);
            }
        }
    }
__exit:
    port_wakeup_unreg(port_wakeup_hdl);
    os_sem_set(&busy_sem, 0);
}

int ext_wireless_net_init(void)//主要是create wifi 线程的
{
    puts("wifi early init \n\n\n\n\n\n");
    thread_fork(WIFI_APP_TASK_NAME, 10, 0x1000, 64, 0, ext_wifi_app_task, NULL);
    return 0;
}
//late_initcall(ext_wireless_net_init);

#endif

