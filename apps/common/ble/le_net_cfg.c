/*********************************************************************************************
    *   Filename        : le_server_module.c

    *   Description     :

    *   Author          :

    *   Email           : zh-jieli.com

    *   Last modifiled  : 2017-01-17 11:14

    *   Copyright:(c)JIELI  2011-2016  @ , All Rights Reserved.
*********************************************************************************************/

// *****************************************************************************
/* EXAMPLE_START(le_counter): LE Peripheral - Heartbeat Counter over GATT
 *
 * @text All newer operating systems provide GATT Client functionality.
 * The LE Counter examples demonstrates how to specify a minimal GATT Database
 * with a custom GATT Service and a custom Characteristic that sends periodic
 * notifications.
 */
// *****************************************************************************
#include "system/app_core.h"
#include "system/includes.h"
#include "system/timer.h"
#include "event/net_event.h"
#include "app_config.h"
#include "asm/crc16.h"
#include "btstack/btstack_task.h"
#include "btstack/bluetooth.h"
#include "btcontroller_modules.h"
#include "bt_common.h"
#include "system/wait.h"
#include "third_party/common/ble_user.h"
#include "le_net_cfg.h"
#include "le_common.h"
#include "ble_ota_scan.h"
#include "event/key_event.h"
#include "net_update.h"
#include "update/http_update.h"

#if (defined CONFIG_WIFI_ENABLE || defined CONFIG_EXT_WIFI_ENABLE)
#include "wifi/wifi_def.h"
#endif
#if BLE_OTA_SCAN_ENABLED
#include "wifi/wifi_connect.h"
#endif

#if (TCFG_BLE_DEMO_SELECT == DEF_BLE_DEMO_NET_CFG)

#define TEST_SEND_HANDLE_VAL         ATT_CHARACTERISTIC_ae82_01_VALUE_HANDLE

#if 1
extern void printf_buf(u8 *buf, u32 len);
#define log_info          printf
#define log_info_hexdump  printf_buf
#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

/* #define LOG_TAG_CONST       BT_BLE */
/* #define LOG_TAG             "[LE_S_DEMO]" */
/* #define LOG_ERROR_ENABLE */
/* #define LOG_DEBUG_ENABLE */
/* #define LOG_INFO_ENABLE */
/* #define LOG_DUMP_ENABLE */
/* #define LOG_CLI_ENABLE */
/* #include "debug.h" */

//------
#define ATT_LOCAL_PAYLOAD_SIZE    (128)                   //note: need >= 20
#define ATT_SEND_CBUF_SIZE        (1024)                   //note: need >= 20,缓存大小，可修改
#define ATT_RAM_BUFSIZE           (ATT_CTRL_BLOCK_SIZE + ATT_LOCAL_PAYLOAD_SIZE + ATT_SEND_CBUF_SIZE)                   //note:
static u8 att_ram_buffer[ATT_RAM_BUFSIZE] __attribute__((aligned(4)));
//---------------

//---------------
#define ADV_INTERVAL_MIN          (160)

static volatile hci_con_handle_t con_handle;
static void ble_module_enable(u8 en);

//连接参数设置
static const uint8_t connection_update_enable = 1; ///0--disable, 1--enable
static uint8_t connection_update_cnt = 0; //
static const struct conn_update_param_t connection_param_table[] = {
    {16, 24, 10, 600},//11
    {12, 28, 10, 600},//3.7
    {8,  20, 10, 600},
    /* {12, 28, 4, 600},//3.7 */
    /* {12, 24, 30, 600},//3.05 */
};
#define CONN_PARAM_TABLE_CNT      (sizeof(connection_param_table)/sizeof(struct conn_update_param_t))

// 广播包内容
/* static u8 adv_data[ADV_RSP_PACKET_MAX];//max is 31 */
// scan_rsp 内容
/* static u8 scan_rsp_data[ADV_RSP_PACKET_MAX];//max is 31 */

#if (ATT_RAM_BUFSIZE < 64)
#error "adv_data & rsp_data buffer error!!!!!!!!!!!!"
#endif

#define adv_data       &att_ram_buffer[0]
#define scan_rsp_data  &att_ram_buffer[32]

extern void bt_ble_get_mac(char *mac);
static char gap_device_name[BT_NAME_LEN_MAX] = "wl80_ble_test";
static u8 complete = 0;
static u8 gap_device_name_len = 0;
static u8 ble_work_state = 0;
static u8 adv_ctrl_en;
static OS_MUTEX mutex;
static void (*app_recieve_callback)(void *priv, void *buf, u16 len) = NULL;
static void (*app_ble_state_callback)(void *priv, ble_state_e state) = NULL;
static void (*ble_resume_send_wakeup)(void) = NULL;
static u32 channel_priv;

static int app_send_user_data_check(u16 len);
static int app_send_user_data_do(void *priv, u8 *data, u16 len);
static int app_send_user_data(u16 handle, const u8 *data, u16 len, u8 handle_type);
extern u8 is_in_config_network_state(void);

#if BLE_OTA_SCAN_ENABLED
static volatile u8 ota_wifi_ready;
static volatile u8 ota_scan_requested;
static volatile u8 ota_scan_stop_requested;
static volatile u8 ota_scan_running;
static u8 ota_adv_active;
static u8 ota_provision_pending;
static u8 ota_cfg_exit_pending;
static u16 ota_scan_timer;
static u16 ota_cfg_exit_timer;
static u32 ota_cfg_generation;
static u32 ota_scan_retry_ms;
static u32 ota_scan_stats_ms;
static u32 ota_scan_rx_log_ms;
static u32 ota_scan_reject_log_ms;
static volatile u32 ota_scan_reports;
static volatile u32 ota_scan_matches;
static u8 ota_scan_rx_log_seq;
static u8 ota_scan_rx_log_total;

static void ble_module_enable_locked(u8 en);
static void ble_ota_scan_reset_url(void);

/* All controller scan commands share the provisioning mutex. */
static int ble_ota_scan_stop(void)
{
    int ret;
    if (!ota_scan_running) {
        return 0;
    }
    ret = ble_op_scan_enable2(0, 0);
    if (ret != BLE_CMD_RET_SUCESS) {
        printf("[BLE_OTA][%u ms] SCAN_STOP_FAILED ret=%d\n", sys_timer_get_ms(), ret);
        return ret;
    }
    ota_scan_running = 0;
    ble_ota_scan_reset_url();
    printf("[BLE_OTA][%u ms] SCAN_STOP reports=%u matched=%u\n",
           sys_timer_get_ms(), ota_scan_reports, ota_scan_matches);
    return 0;
}

#define BLE_OTA_ADV_COMPANY_ID       0xFFFF
#define BLE_OTA_ADV_MAX_PAYLOAD      17
#define BLE_OTA_ADV_RESULT_NONE      0
#define BLE_OTA_ADV_RESULT_OK        1
#define BLE_OTA_ADV_RESULT_FORMAT    (-1)
#define BLE_OTA_ADV_RESULT_FRAGMENT  (-2)
#define BLE_OTA_ADV_RESULT_CRC       (-3)
#define BLE_OTA_SCAN_CRC8_POLY       (0x07U)
#define BLE_OTA_URL_MAX_LEN          64
#define BLE_OTA_URL_MAX_FRAGMENTS    4
#define BLE_OTA_URL_TIMEOUT_MS       10000

static u8 ota_url_buffer[BLE_OTA_URL_MAX_LEN + 1];
static u8 ota_url_last_ready[BLE_OTA_URL_MAX_LEN + 1];
static u16 ota_url_len;
static u16 ota_url_last_ready_len;
static volatile u8 ota_http_start_pending;
static u8 ota_url_total;
static u8 ota_url_next_seq;
static u8 ota_url_active;
static u32 ota_url_last_ms;
static u32 ota_url_ready_ms;
static u32 ota_url_drop_log_ms;

static void ble_ota_scan_reset_url(void)
{
    ota_url_len = 0;
    ota_url_total = 0;
    ota_url_next_seq = 0;
    ota_url_active = 0;
    ota_url_last_ms = 0;
}

static void ble_ota_scan_reset_session(void)
{
    ble_ota_scan_reset_url();
    ota_url_last_ready_len = 0;
    ota_url_ready_ms = 0;
    ota_http_start_pending = 0;
}

static void ble_ota_scan_url_drop(u32 now, int reason)
{
    if ((u32)(now - ota_url_drop_log_ms) >= 1000) {
        ota_url_drop_log_ms = now;
        printf("[BLE_OTA][%u ms] OTA_URL_DROP reason=%d\n", now, reason);
    }
    ble_ota_scan_reset_url();
}

/* Stage 4: collect ordered fragments and print the complete URL only. */
static void ble_ota_scan_collect_url(u8 seq, u8 total, const u8 *payload,
                                     u8 payload_len, u32 now)
{
    u16 i;
    if (total > BLE_OTA_URL_MAX_FRAGMENTS || !payload_len) {
        ble_ota_scan_url_drop(now, -1);
        return;
    }
    if (ota_url_active &&
        (u32)(now - ota_url_last_ms) >= BLE_OTA_URL_TIMEOUT_MS) {
        ble_ota_scan_reset_url();
    }
    if (seq == 0) {
        ble_ota_scan_reset_url();
        ota_url_active = 1;
        ota_url_total = total;
    } else if (!ota_url_active || total != ota_url_total || seq != ota_url_next_seq) {
        ble_ota_scan_url_drop(now, -2);
        return;
    }
    if (ota_url_len + payload_len > BLE_OTA_URL_MAX_LEN) {
        ble_ota_scan_url_drop(now, -3);
        return;
    }
    memcpy(ota_url_buffer + ota_url_len, payload, payload_len);
    ota_url_len += payload_len;
    ota_url_next_seq = seq + 1;
    ota_url_last_ms = now;
    if (ota_url_next_seq != ota_url_total) {
        return;
    }
    ota_url_buffer[ota_url_len] = '\0';
    if ((ota_url_len < 7 || memcmp(ota_url_buffer, "http://", 7)) &&
        (ota_url_len < 8 || memcmp(ota_url_buffer, "https://", 8))) {
        ble_ota_scan_url_drop(now, -4);
        return;
    }
    for (i = 0; i < ota_url_len; i++) {
        if (ota_url_buffer[i] < 0x20 || ota_url_buffer[i] > 0x7e) {
            ble_ota_scan_url_drop(now, -4);
            return;
        }
    }
    if (ota_url_len == ota_url_last_ready_len &&
        !memcmp(ota_url_buffer, ota_url_last_ready, ota_url_len) &&
        (u32)(now - ota_url_ready_ms) < BLE_OTA_URL_TIMEOUT_MS) {
        printf("[BLE_OTA][%u ms] OTA_URL_DUPLICATE len=%u\n", now, ota_url_len);
    } else {
        memcpy(ota_url_last_ready, ota_url_buffer, ota_url_len + 1);
        ota_url_last_ready_len = ota_url_len;
        ota_url_ready_ms = now;
        printf("[BLE_OTA][%u ms] OTA_URL_READY len=%u url=%s\n",
               now, ota_url_len, ota_url_buffer);
        ota_http_start_pending = 1;
        ota_scan_stop_requested = 1;
        ota_scan_requested = 0;
        printf("[BLE_OTA][%u ms] OTA_HTTP_START_REQUEST url=%s\n",
               now, ota_url_buffer);
    }
    ble_ota_scan_reset_url();
}

void ble_ota_scan_toggle(void)
{
    if (ota_scan_running) {
        ota_scan_stop_requested = 1;
        ota_scan_requested = 0;
        printf("[BLE_OTA][%u ms] SCAN_EXIT_REQUEST source=key_double_click\n", sys_timer_get_ms());
        return;
    }
    if (ota_scan_requested) {
        ota_scan_requested = 0;
        printf("[BLE_OTA][%u ms] SCAN_REQUEST_CANCELLED source=key_double_click\n", sys_timer_get_ms());
        return;
    }
    ota_scan_stop_requested = 0;
    ota_scan_requested = 1;
    printf("[BLE_OTA][%u ms] SCAN_REQUEST source=key_double_click\n", sys_timer_get_ms());
}

/* CRC-8/ATM, polynomial 0x07, initial value 0. */
static u8 ble_ota_scan_crc8(const u8 *data, u8 len)
{
    u8 crc = 0;
    u8 i;
    while (len--) {
        crc ^= *data++;
        for (i = 0; i < 8; i++) {
            crc = (crc & 0x80) ? ((crc << 1) ^ BLE_OTA_SCAN_CRC8_POLY) : (crc << 1);
        }
    }
    return crc;
}

/* Manufacturer payload: FF FF OTA seq total payload_len payload crc8. */
static int ble_ota_scan_ota_packet(const u8 *data, u8 len,
                                   u8 *seq, u8 *total, u8 *payload_len,
                                   u8 *payload_crc, const u8 **payload)
{
    static const u8 marker[] = {'O', 'T', 'A'};
    unsigned int offset = 0;
    unsigned int field_len;
    unsigned int manufacturer_len;
    unsigned int crc_len;
    const u8 *manufacturer;
    while (offset < len) {
        field_len = data[offset];
        if (!field_len) {
            break;
        }
        if (field_len > len - offset - 1) {
            return BLE_OTA_ADV_RESULT_FORMAT;
        }
        if (field_len >= 10 &&
            data[offset + 1] == 0xff &&
            data[offset + 2] == (BLE_OTA_ADV_COMPANY_ID & 0xff) &&
            data[offset + 3] == (BLE_OTA_ADV_COMPANY_ID >> 8) &&
            !memcmp(data + offset + 4, marker, sizeof(marker))) {
            manufacturer = data + offset + 2;
            manufacturer_len = field_len - 1;
            *seq = manufacturer[5];
            *total = manufacturer[6];
            *payload_len = manufacturer[7];
            if (!*total || *seq >= *total || *payload_len > BLE_OTA_ADV_MAX_PAYLOAD ||
                manufacturer_len != (unsigned int)(9 + *payload_len)) {
                return BLE_OTA_ADV_RESULT_FRAGMENT;
            }
            crc_len = 8 + *payload_len;
            *payload_crc = manufacturer[crc_len];
            if (ble_ota_scan_crc8(manufacturer, crc_len) != *payload_crc) {
                return BLE_OTA_ADV_RESULT_CRC;
            }
            *payload = manufacturer + 8;
            return BLE_OTA_ADV_RESULT_OK;
        }
        offset += field_len + 1;
    }
    return BLE_OTA_ADV_RESULT_NONE;
}

static void ble_ota_scan_report(const u8 *packet, u16 size)
{
    int result;
    u8 len;
    u8 seq;
    u8 total;
    u8 payload_len;
    u8 payload_crc;
    const u8 *payload;
    u32 now;
    bd_addr_t address;
    if (!ota_scan_running || !ota_wifi_ready || is_in_config_network_state() || size < 12) {
        return;
    }
    len = packet[11];
    if (packet[1] < 10 || (u16)packet[1] + 2 > size ||
        len > 31 || len > packet[1] - 10 || packet[2] > 3) {
        return;
    }
    ota_scan_reports++;
    result = ble_ota_scan_ota_packet(packet + 12, len, &seq, &total,
                                     &payload_len, &payload_crc, &payload);
    if (result != BLE_OTA_ADV_RESULT_OK) {
        if (result < 0) {
            now = sys_timer_get_ms();
            if ((u32)(now - ota_scan_reject_log_ms) >= 1000) {
                ota_scan_reject_log_ms = now;
                printf("[BLE_OTA][%u ms] OTA_ADV_REJECT reason=%d\n", now, result);
            }
        }
        return;
    }
    ota_scan_matches++;
    now = sys_timer_get_ms();
    if (ota_scan_matches != 1 && seq == ota_scan_rx_log_seq &&
        total == ota_scan_rx_log_total &&
        (u32)(now - ota_scan_rx_log_ms) < 1000) {
        return;
    }
    ota_scan_rx_log_ms = now;
    ota_scan_rx_log_seq = seq;
    ota_scan_rx_log_total = total;
    gap_event_advertising_report_get_address(packet, address);
    music_play_res_file("RecvBLE.mp3");
    printf("[BLE_OTA][%u ms] OTA_ADV_RX seq=%u/%u payload=%u crc=%02X rssi=%d addr=%02X:%02X:%02X:%02X:%02X:%02X matched=%u\n",
           now, seq, total, payload_len, payload_crc,
           (s8)packet[10], address[0], address[1], address[2], address[3],
           address[4], address[5], ota_scan_matches);
    printf("[BLE_OTA][%u ms] OTA_ADV_RAW size=%u: ", now, size);
    log_info_hexdump((u8 *)packet, size);
    ble_ota_scan_collect_url(seq, total, payload, payload_len, now);
}

void ble_ota_scan_wifi_state(unsigned char ready)
{
    ota_wifi_ready = !!ready;
}

static void ble_ota_scan_poll(void *priv)
{
    int ret;
    int ready;
    u32 now = sys_timer_get_ms();
    os_mutex_pend(&mutex, 0);
    ready = ota_wifi_ready &&           // WiFi 网络事件已经通知联网成功。
            wifi_is_on() &&             // WiFi 模块仍然处于开启状态。
            wifi_get_sta_connect_state() == WIFI_STA_NETWORK_STACK_DHCP_SUCC && // WiFi 不只是连接上路由器，还必须已经完成 DHCP，具备正常网络能力。
            !is_in_config_network_state() &&   // 当前不能处于 BLE 配网状态。这样可以避免 BLE 配网广播和 OTA 被动扫描同时使用 BLE 控制器。
            !con_handle &&              // 当前没有 BLE GATT 连接。OTA 广播扫描阶段只接收广播，不允许已有连接占用相关资源。
            !adv_ctrl_en &&             // 当前没有启用 BLE 配网广播。
            !ota_adv_active &&          // 当前没有其他 BLE 可连接广播活动。
            !complete &&                // BLE 配网流程没有处于完成但尚未完全退出的状态。
            !ota_cfg_exit_pending &&    // 配网退出定时器还没有执行完成时，不启动扫描，避免配网广播关闭和 OTA 扫描启动发生冲突。
            !ota_provision_pending;     // 当前没有等待启动 BLE 配网广播。
#ifdef CONFIG_LTE_PHY_ENABLE
    extern int sys_net_channel_read(void);
    ready = ready && sys_net_channel_read() == 0;
#endif
    if (!ready) {
        if (ota_scan_running && (u32)(now - ota_scan_retry_ms) >= 1000) {
            ota_scan_retry_ms = now;
            ble_ota_scan_stop();
        }
        ota_scan_stop_requested = 0;
        if (ota_scan_requested) {
            ota_scan_requested = 0;
            printf("[BLE_OTA][%u ms] SCAN_REQUEST_REJECT prerequisites\n", now);
        }
        if (ota_http_start_pending) {
            ota_http_start_pending = 0;
            printf("[BLE_OTA][%u ms] OTA_HTTP_START_CANCELLED prerequisites\n", now);
        }
        if (!ota_scan_running && ota_provision_pending) {
            ble_module_enable_locked(1);
        }
    } else if (ota_scan_stop_requested) {
        if (!ota_scan_running) {
            ota_scan_stop_requested = 0;
        } else if (ble_ota_scan_stop() == 0) {
            music_play_res_file("ExitRecvBC.mp3");
            ota_scan_stop_requested = 0;
            printf("[BLE_OTA][%u ms] SCAN_EXIT\n", now);
            if (ota_http_start_pending) {
                int http_ret;
                ota_http_start_pending = 0;
                http_ret = http_ota_init_with_url((const char *)ota_url_last_ready);
                if (http_ret) {
                    printf("[BLE_OTA][%u ms] OTA_HTTP_START_FAILED ret=%d\n", now, http_ret);
                } else {
                    printf("[BLE_OTA][%u ms] OTA_HTTP_START url=%s\n",
                           now, ota_url_last_ready);
                }
            }
        }
    } else if (ota_scan_requested && !ota_scan_running &&
               (u32)(now - ota_scan_retry_ms) >= 1000) {
        music_play_res_file("EnterRecvBC.mp3");
        ota_scan_retry_ms = now;
        ret = ble_op_set_scan_filter_policy(0);
        if (ret == BLE_CMD_RET_SUCESS) {
            /*
                扫描间隔interval：100 ms；
                扫描窗口window：30 ms；
                被动扫描；
                不主动发送 Scan Request。
            */
            ret = ble_op_set_scan_param(0, ADV_SCAN_MS(100), ADV_SCAN_MS(30));
        }
        if (ret == BLE_CMD_RET_SUCESS) {
            ret = ble_op_scan_enable2(1, 0);    // 启动扫描
        }
        if (ret == BLE_CMD_RET_SUCESS) {
            ota_scan_reports = 0;
            ota_scan_matches = 0;
            ota_scan_rx_log_seq = 0xff;
            ota_scan_rx_log_total = 0;
            ota_scan_stats_ms = now;
            ota_scan_running = 1;
            ota_scan_requested = 0;
            ota_scan_stop_requested = 0;
            ble_ota_scan_reset_session();
            ota_scan_retry_ms = now - 1000; /* Allow an immediate stop on disconnect. */
            printf("[BLE_OTA][%u ms] SCAN_START passive interval=100ms window=30ms\n", now);
        } else {
            printf("[BLE_OTA][%u ms] SCAN_START_FAILED ret=%d\n", now, ret);
        }
    }
    if (ota_scan_running && (u32)(now - ota_scan_stats_ms) >= 5000) {
        ota_scan_stats_ms = now;
        printf("[BLE_OTA][%u ms] SCAN_STATS reports=%u matched=%u\n",
               now, ota_scan_reports, ota_scan_matches);
    }
    os_mutex_post(&mutex);
}

void ble_ota_scan_stack_ready(void)
{
    if (!os_mutex_valid(&mutex)) {
        os_mutex_create(&mutex);
    }
    if (!ota_scan_timer) {
        ota_scan_timer = sys_timer_add_to_task("sys_timer", NULL, ble_ota_scan_poll, 200);
        printf("[BLE_OTA][%u ms] SCAN_INIT timer=%u\n", sys_timer_get_ms(), ota_scan_timer);
    }
}

static void ble_ota_cfg_exit_timeout(void *priv)
{
    os_mutex_pend(&mutex, 0);
    /* An old, already queued callback must not close a new provisioning session. */
    if (ota_cfg_exit_pending && (u32)priv == ota_cfg_generation) {
        ota_cfg_exit_pending = 0;
        ota_cfg_exit_timer = 0;
        ble_module_enable_locked(0);
    }
    os_mutex_post(&mutex);
}
#endif

// Complete Local Name  默认的蓝牙名字
extern const char *bt_get_local_name();

enum {
    BLE_WIFI_CONNECTED = 0,//配网成回复命令
    BLE_WIFI_CONNECTING,//正在配网回复命令
    BLE_WIFI_CONNECT_FAILED,//配网失败回复命令
    BLE_WIFI_CONNECT_NO_FOUND_SSID, //未找到ssid回复命令
    BLE_WIFI_CONNECT_ASSOCIAT_FAIL, //密码错误回复命令
    BLE_WIFI_CONNECT_SERVER_FAIL, //网络问题连不上服务器
};

static const u8 rsp_cmd_head[6] = {0x4A, 0x4C, 0x00, 0x0e, 0x10, 0x02};

//配网成回复命令
static const u8 rsp_cmd0[] = {
    0x4A, 0x4C, 0x00, 0x0e, 0x10, 0x02,
    '{', '"', 's', 't', 'a', 't', 'u', 's', '"', ':', '0', '}',
    0x7C, 0xC3, 0xFF,
};

//正在配网回复命令
static const u8 rsp_cmd1[] = {
    0x4A, 0x4C, 0x00, 0x0e, 0x10, 0x02,
    '{', '"', 's', 't', 'a', 't', 'u', 's', '"', ':', '1', '}',
    0x4F, 0xF2, 0xFF,
};

//配网失败回复命令
static const u8 rsp_cmd2[] = {
    0x4A, 0x4C, 0x00, 0x0e, 0x10, 0x02,
    '{', '"', 's', 't', 'a', 't', 'u', 's', '"', ':', '2', '}',
    0x1A, 0xA1, 0xFF,
};

//未找到ssid回复命令
static const u8 rsp_cmd3[] = {
    0x4A, 0x4C, 0x00, 0x0e, 0x10, 0x02,
    '{', '"', 's', 't', 'a', 't', 'u', 's', '"', ':', '3', '}',
    0x29, 0x90, 0xFF,
};

//密码错误回复命令
static const u8 rsp_cmd4[] = {
    0x4A, 0x4C, 0x00, 0x0e, 0x10, 0x02,
    '{', '"', 's', 't', 'a', 't', 'u', 's', '"', ':', '4', '}',
    0xB0, 0x07, 0xFF,
};

//连不上网回复命令
static const u8 rsp_cmd5[] = {//此命令数组不能使用，没有修改校验值
    0x4A, 0x4C, 0x00, 0x0e, 0x10, 0x02,
    '{', '"', 's', 't', 'a', 't', 'u', 's', '"', ':', '5', '}',
    0xB0, 0x07, 0xFF,
};

//mac地址回复命令
static u8 rsp_mac_cmd[512] = {0};

void *format_date(void)
{
    static char output[12];
    const char *months[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    char month_str[4];
    int day, year;
    int month = 0;
    const char *date_str = __DATE__;
    sscanf(date_str, "%3s %d %d", month_str, &day, &year);
    for (int i = 0; i < 12; i++) {
        if (strcmp(month_str, months[i]) == 0) {
            month = i + 1;
            break;
        }
    }
    sprintf(output, "%04d.%02d.%02d", year, month, day);
    return output;
}

int rsp_cmd_status_str(unsigned char *buf, int buf_size, int status)
{
    int Length = 0;
    int size = 0;
    int ret = 0;
    u16 crc = 0;

    memset(buf, 0, buf_size);
    memcpy(buf, rsp_cmd_head, 6);
    size += 6;//加上length长度2字节 + 4字节

    ret = snprintf((char *)buf + size, buf_size - size, "{\"status\":%d,\"version\":\"%s.%s\"}", status, format_date(), OTA_VERSON);
    //ret = snprintf((char *)&buf[size], buf_size - size, "{\"status\":%d}", status);
    Length += ret + 2;
    size += ret;

    buf[2] = (Length >> 8) & 0xFF;//length
    buf[3] = Length & 0xFF;//length

    crc = CRC16(&buf[2], Length + 2);//crc16
    buf[size++] = (crc >> 8) & 0xFF;
    buf[size++] = crc & 0xFF;
    buf[size++] = 0xFF;//end
    //put_buf(buf, size);
    printf("rsp_cmd %d : %s\n", size, buf + 6);
    return size;
}

void user_app_send_user_data(void *ATT_VALUE, void *rsp_cmd, void *ATT_OP_type)
{
    int len = rsp_cmd_status_str(rsp_mac_cmd, sizeof(rsp_mac_cmd), (int)rsp_cmd);
    app_send_user_data(ATT_VALUE, rsp_mac_cmd, len, ATT_OP_type);
}

void user_app_send_user_data_connet_server_err(void)
{
    user_app_send_user_data(ATT_CHARACTERISTIC_ae82_01_VALUE_HANDLE, BLE_WIFI_CONNECT_SERVER_FAIL, ATT_OP_AUTO_READ_CCC);
}

//------------------------------------------------------
static void send_request_connect_parameter(u8 table_index)
{
    struct conn_update_param_t *param = (void *)&connection_param_table[table_index];//static ram

    log_info("update_request:-%d-%d-%d-%d-\n", param->interval_min, param->interval_max, param->latency, param->timeout);
    if (con_handle) {
        ble_user_cmd_prepare(BLE_CMD_REQ_CONN_PARAM_UPDATE, 2, con_handle, param);
    }
}

static void check_connetion_updata_deal(void)
{
    if (connection_update_enable) {
        if (connection_update_cnt < CONN_PARAM_TABLE_CNT) {
            send_request_connect_parameter(connection_update_cnt);
        }
    }
}

static void connection_update_complete_success(u8 *packet)
{
    int con_handle, conn_interval, conn_latency, conn_timeout;

    con_handle = hci_subevent_le_connection_update_complete_get_connection_handle(packet);
    conn_interval = hci_subevent_le_connection_update_complete_get_conn_interval(packet);
    conn_latency = hci_subevent_le_connection_update_complete_get_conn_latency(packet);
    conn_timeout = hci_subevent_le_connection_update_complete_get_supervision_timeout(packet);

    log_info("conn_interval = %d\n", conn_interval);
    log_info("conn_latency = %d\n", conn_latency);
    log_info("conn_timeout = %d\n", conn_timeout);
}

static void set_ble_work_state(ble_state_e state)
{
    if (state != ble_work_state) {
        log_info("ble_work_st:%x->%x\n", ble_work_state, state);
        ble_work_state = state;
        if (app_ble_state_callback) {
            app_ble_state_callback((void *)channel_priv, state);
        }
    }
}

static ble_state_e get_ble_work_state(void)
{
    return ble_work_state;
}

static void cbk_sm_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    sm_just_event_t *event = (void *)packet;
    u32 tmp32;

    switch (packet_type) {
    case HCI_EVENT_PACKET:
        switch (hci_event_packet_get_type(packet)) {
        case SM_EVENT_JUST_WORKS_REQUEST:
            sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
            log_info("Just Works Confirmed.\n");
            break;
        case SM_EVENT_PASSKEY_DISPLAY_NUMBER:
            log_info_hexdump(packet, size);
            memcpy(&tmp32, event->data, 4);
            log_info("Passkey display: %06u.\n", tmp32);
            break;
        }
        break;
    }
}

static void can_send_now_wakeup(void)
{
    putchar('E');
    if (ble_resume_send_wakeup) {
        ble_resume_send_wakeup();
    }
}

static const char *const phy_result[] = {
    "None",
    "1M",
    "2M",
    "Coded",
};

static void server_profile_start(u16 con_handle)
{
    ble_user_cmd_prepare(BLE_CMD_ATT_SEND_INIT, 4, con_handle, att_ram_buffer, ATT_RAM_BUFSIZE, ATT_LOCAL_PAYLOAD_SIZE);
    set_ble_work_state(BLE_ST_CONNECT);
    /* set_connection_data_phy(CONN_SET_CODED_PHY, CONN_SET_CODED_PHY); */
}

/*
 * @section Packet Handler
 *
 * @text The packet handler is used to:
 *        - stop the counter after a disconnect
 *        - send a notification when the requested ATT_EVENT_CAN_SEND_NOW is received
 */

/* LISTING_START(packetHandler): Packet Handler */
static void cbk_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    int mtu;
    u32 tmp;
    u8 status;

    switch (packet_type) {
    case HCI_EVENT_PACKET:
        switch (hci_event_packet_get_type(packet)) {

#if BLE_OTA_SCAN_ENABLED
        case GAP_EVENT_ADVERTISING_REPORT:
            ble_ota_scan_report(packet, size);
            break;
#endif

        /* case DAEMON_EVENT_HCI_PACKET_SENT: */
        /* break; */
        case ATT_EVENT_HANDLE_VALUE_INDICATION_COMPLETE:
            log_info("ATT_EVENT_HANDLE_VALUE_INDICATION_COMPLETE\n");
        case ATT_EVENT_CAN_SEND_NOW:
            can_send_now_wakeup();
            break;

        case HCI_EVENT_LE_META:
            switch (hci_event_le_meta_get_subevent_code(packet)) {
            case HCI_SUBEVENT_LE_ENHANCED_CONNECTION_COMPLETE:
                status = hci_subevent_le_enhanced_connection_complete_get_status(packet);
                if (status) {
                    log_info("LE_SLAVE CONNECTION FAIL!!! %0x\n", status);
                    set_ble_work_state(BLE_ST_DISCONN);
                    break;
                }
                con_handle = hci_subevent_le_enhanced_connection_complete_get_connection_handle(packet);
#if BLE_OTA_SCAN_ENABLED
                ota_adv_active = 0; /* Connectable advertising stops on connection. */
#endif
                log_info("HCI_SUBEVENT_LE_ENHANCED_CONNECTION_COMPLETE : %0x\n", con_handle);
                log_info("conn_interval = %d\n", hci_subevent_le_enhanced_connection_complete_get_conn_interval(packet));
                log_info("conn_latency = %d\n", hci_subevent_le_enhanced_connection_complete_get_conn_latency(packet));
                log_info("conn_timeout = %d\n", hci_subevent_le_enhanced_connection_complete_get_supervision_timeout(packet));
                server_profile_start(con_handle);
                break;

            case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
                con_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
#if BLE_OTA_SCAN_ENABLED
                ota_adv_active = 0;
#endif
                log_info("HCI_SUBEVENT_LE_CONNECTION_COMPLETE: %0x\n", con_handle);
                connection_update_complete_success(packet + 8);
                server_profile_start(con_handle);
                break;

            case HCI_SUBEVENT_LE_CONNECTION_UPDATE_COMPLETE:
                connection_update_complete_success(packet);
                break;

            case HCI_SUBEVENT_LE_DATA_LENGTH_CHANGE:
                log_info("APP HCI_SUBEVENT_LE_DATA_LENGTH_CHANGE\n");
                /* set_connection_data_phy(CONN_SET_CODED_PHY, CONN_SET_CODED_PHY); */
                break;

            case HCI_SUBEVENT_LE_PHY_UPDATE_COMPLETE:
                log_info("APP HCI_SUBEVENT_LE_PHY_UPDATE %s\n", hci_event_le_meta_get_phy_update_complete_status(packet) ? "Fail" : "Succ");
                log_info("Tx PHY: %s\n", phy_result[hci_event_le_meta_get_phy_update_complete_tx_phy(packet)]);
                log_info("Rx PHY: %s\n", phy_result[hci_event_le_meta_get_phy_update_complete_rx_phy(packet)]);
                break;
            }
            break;

        case HCI_EVENT_DISCONNECTION_COMPLETE:
            printf("[BLE][%u ms] BLE_DISCONNECTED handle=%04x reason=%02x cfg=%u adv_ctrl=%u\n",
                   sys_timer_get_ms(), con_handle, packet[5], is_in_config_network_state(), adv_ctrl_en);
            log_info("HCI_EVENT_DISCONNECTION_COMPLETE: %0x\n", packet[5]);
            con_handle = 0;
            ble_user_cmd_prepare(BLE_CMD_ATT_SEND_INIT, 4, con_handle, 0, 0, 0);
            set_ble_work_state(BLE_ST_DISCONN);
#if !CONFIG_POWER_ON_ENABLE_BLE
            if (is_in_config_network_state()) {
#else
            {
#endif
                os_mutex_pend(&mutex, 0);
                bt_ble_adv_enable(1);
                complete = 0;
                os_mutex_post(&mutex);
            }
            connection_update_cnt = 0;
            break;

        case ATT_EVENT_MTU_EXCHANGE_COMPLETE:
            mtu = att_event_mtu_exchange_complete_get_MTU(packet) - 3;
            log_info("ATT MTU = %u\n", mtu);
            ble_user_cmd_prepare(BLE_CMD_ATT_MTU_SIZE, 1, mtu);
            break;

        case HCI_EVENT_VENDOR_REMOTE_TEST:
            log_info("--- HCI_EVENT_VENDOR_REMOTE_TEST\n");
            break;

        case L2CAP_EVENT_CONNECTION_PARAMETER_UPDATE_RESPONSE:
            tmp = little_endian_read_16(packet, 4);
            log_info("-update_rsp: %02x\n", tmp);
            if (tmp) {
                connection_update_cnt++;
                log_info("remoter reject!!!\n");
                check_connetion_updata_deal();
            } else {
                connection_update_cnt = CONN_PARAM_TABLE_CNT;
            }
            break;

        case HCI_EVENT_ENCRYPTION_CHANGE:
            log_info("HCI_EVENT_ENCRYPTION_CHANGE= %d\n", packet[2]);
            break;
        }
        break;
    }
}

extern char *app_user_product_uuid(void);
extern char *user_product_batch_num(void);

#ifdef CONFIG_LTE_PHY_ENABLE
#if (CONFIG_LTE_VENDOR == VENDOR_GT108)
int Total_size = 0;
int ble_connected_net_notify2(void)
{
    int net_ch = 3;
    int ret;
    int Length = 0;
    u16 crc;
    unsigned char mac[6];
    Total_size = 0;
    //设备信息缓冲区
    char iccid_0[20 + 1] = {0};
    char iccid_1[20 + 1] = {0};
    char wifi_mac[12 + 1] = {0};
    char imei[15 + 1] = {0};

    //获取BLE MAC地址
    bt_ble_get_mac(mac);
    memset(rsp_mac_cmd, 0, sizeof(rsp_mac_cmd));
    //初始化协议头
    rsp_mac_cmd[0] = 0x4A;//"J"
    rsp_mac_cmd[1] = 0x4C;//"L"
    rsp_mac_cmd[4] = 0x10;//一级命令
    rsp_mac_cmd[5] = 0x02;//二级命令
    //read info from flash
    //lte_dev_info_opt(1, iccid_0, iccid_1, wifi_mac, imei);
    lte_dev_info_opt(1, NULL, NULL, NULL, imei, NULL);
    Total_size = 6;  // 协议头固定6字节：length长度2字节 + 4字节

    //0-wifi网络，1-4G网络
    net_ch = sys_net_channel_read();
    if (net_ch == 0) {
        printf("Network Type: Use WiFi Network!!!");
    } else if (net_ch == 1) {
        printf("Network Type: Use 4G Network!!!");
    }

    /* 生成JSON格式数据 */
    ret = sprintf(&rsp_mac_cmd[Total_size], "{\"mode\":%d,\"mac\":\"%02X%02X%02X%02X%02X%02X\"", (255 - net_ch), mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
    Total_size += ret;
    Length += ret;
    char *uuid = app_user_product_uuid();
    if (!uuid) {
        printf("attention: use default product uuid!!!\r\n");
        //use product uuid of qy-ai
        uuid = user_product_uuid_default();
    }

    char *batch_num = user_product_batch_num();
    if (uuid && batch_num) {
        ret = sprintf(&rsp_mac_cmd[Total_size], ",\"uuid\":\"%s\"", uuid);
        Total_size += ret;
        Length += ret;

        ret = sprintf(&rsp_mac_cmd[Total_size], ",\"batch_num\":\"%s\"", batch_num);
        Total_size += ret;
        Length += ret;
    }
#if 0
    ret = sprintf(&rsp_mac_cmd[Total_size], ",\"iccid_0\":\"%s\"", iccid_0);
    Total_size += ret;
    Length += ret;

    ret = sprintf(&rsp_mac_cmd[Total_size], ",\"iccid_1\":\"%s\"", iccid_1);
    Total_size += ret;
    Length += ret;

    ret = sprintf(&rsp_mac_cmd[Total_size], ",\"lte_mac\":\"%s\"", wifi_mac);
    Total_size += ret;
    Length += ret;
#endif
    ret = sprintf(&rsp_mac_cmd[Total_size], ",\"imei\":\"%s\"", imei);
    Total_size += ret;
    Length += ret;

    ret = sprintf(&rsp_mac_cmd[Total_size], "\}");
    if (ret > 0) {
        Total_size += ret;
        Length += ret; //闭合符(1字节)
        Length += 2; //协议头长度字段的 2 字节

        rsp_mac_cmd[2] = (Length >> 8) & 0xFF;//length
        rsp_mac_cmd[3] = Length & 0xFF;//length

        crc = CRC16(&rsp_mac_cmd[2], Length + 2);//crc16
        rsp_mac_cmd[Total_size++] = (crc >> 8) & 0xFF;
        rsp_mac_cmd[Total_size++] = crc & 0xFF;
        rsp_mac_cmd[Total_size++] = 0xFF;//end
        printf("rsp_mac_cmd: %d-%d-%s", rsp_mac_cmd[2] * 256 + rsp_mac_cmd[3], Total_size, &rsp_mac_cmd[6]);
    }

    return 0;
}
#endif
#endif

/* LISTING_END */

/*
 * @section ATT Read
 *
 * @text The ATT Server handles all reads to constant data. For dynamic data like the custom characteristic, the registered
 * att_read_callback is called. To handle long characteristics and long reads, the att_read_callback is first called
 * with buffer == NULL, to request the total value length. Then it will be called again requesting a chunk of the value.
 * See Listing attRead.
 */

/* LISTING_START(attRead): ATT Read */

// ATT Client Read Callback for Dynamic Data
// - if buffer == NULL, don't copy data, just return size of value
// - if buffer != NULL, copy data and return number bytes copied
// @param offset defines start of attribute value
static uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    uint16_t  att_value_len = 0;
    uint16_t handle = att_handle;

    log_info("read_callback, handle= 0x%04x,buffer= %08x\n", handle, (u32)buffer);

    switch (handle) {
    case ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE:
        att_value_len = gap_device_name_len;

        if ((offset >= att_value_len) || (offset + buffer_size) > att_value_len) {
            break;
        }

        if (buffer) {
            memcpy(buffer, &gap_device_name[offset], buffer_size);
            att_value_len = buffer_size;
            log_info("\n------read gap_name: %s \n", gap_device_name);
        }
        break;

    case ATT_CHARACTERISTIC_ae82_01_CLIENT_CONFIGURATION_HANDLE:
        if (buffer) {
            buffer[0] = att_get_ccc_config(handle);
            buffer[1] = 0;
        }
        att_value_len = 2;
        break;
    case ATT_CHARACTERISTIC_ae83_01_VALUE_HANDLE:
#ifdef CONFIG_LTE_PHY_ENABLE
        //只读特性的UUID
#if (CONFIG_LTE_VENDOR == VENDOR_GT108)
        //第一次回调，buffer指针为NULL(让设备先准备好数据)
        ble_connected_net_notify2();
        att_value_len = Total_size;
        if ((offset >= att_value_len) || (offset + buffer_size) > att_value_len) {
            break;
        }

        //第二次回调，buffer才不为空
        if (buffer) {
            memcpy(buffer, &rsp_mac_cmd[offset], buffer_size);
            //返回实际拷贝的数据长度
            att_value_len = buffer_size;
        }
#endif
#endif
        break;
    default:
        break;
    }

    log_info("att_value_len= %d\n", att_value_len);
    return att_value_len;
}

static u16 user_buf_offset = 0, user_data_size = 0;
static u8 user_buf[128];
extern int get_page_turning_profile(char *username, char *password);
extern int get_payment_bind_msg(char *uuid, char *clientid, char *bind_token);

static int check_profile(void)
{
    char username[64] = {0}, clientid[64] = {0}, bind_token[64] = {0};
    if (get_page_turning_profile(username, NULL)) {
        return false;
    }
    if (get_payment_bind_msg(NULL, clientid, bind_token)) {
        return false;
    }
    return true;
}

extern int bt_net_config_set_ssid_pwd(const char *data);
extern int bt_net_config_set_ssid_pwd_get(char **ssid, char **pwd, char clean);
extern u16 CRC16(const void *ptr, u32 len);

static void check_net_info_if_recv_complete(void)
{
    if (user_buf[user_buf_offset - 1] == 0xFF && user_buf_offset == user_data_size + 7) {
        user_buf[sizeof(user_buf) - 1] = 0;
        u16 crc16 = user_buf[user_buf_offset - 2] + (user_buf[user_buf_offset - 3] << 8);
        put_buf(&user_buf[2], user_data_size + 2);
        if (crc16 == CRC16(&user_buf[2], user_data_size + 2)) {
            //去掉2byte crc 1byte 结束符
            user_buf[user_buf_offset - 3] = 0;
            if (0 == bt_net_config_set_ssid_pwd((const char *)&user_buf[6])) {
                //app_send_user_data(ATT_CHARACTERISTIC_ae82_01_VALUE_HANDLE, rsp_cmd1, sizeof(rsp_cmd1), ATT_OP_AUTO_READ_CCC);
                user_app_send_user_data(ATT_CHARACTERISTIC_ae82_01_VALUE_HANDLE, BLE_WIFI_CONNECTING, ATT_OP_AUTO_READ_CCC);
                complete = 1;
                user_buf_offset = 0;
                user_data_size = 0;
                memset(user_buf, 0, sizeof(user_buf));

#if (defined CONFIG_WIFI_ENABLE && defined CONFIG_LTE_PHY_ENABLE)
                int net_ch = sys_net_channel_read();
                if (net_ch == 1) {
                    int ble_connected_net_notify(void);

                    int sys_net_channel_change_reset(void);
                    sys_timeout_add(NULL, ble_connected_net_notify, 200);
#if (CONFIG_LTE_VENDOR == VENDOR_GT108)
                    int ble_connected_net_notify2(void);
                    sys_timeout_add(NULL, ble_connected_net_notify2, 200);
#endif
//                    sys_timeout_add(NULL, sys_net_channel_change_reset, 2000);
                    struct net_event net = {0};
                    net.arg = "net";
                    net.event = NET_NTP_GET_TIME_SUCC + 34;
                    net_event_notify(NET_EVENT_FROM_USER, &net);
                }
#elif defined CONFIG_LTE_PHY_ENABLE
                int ble_connected_net_notify(void);
                sys_timeout_add(NULL, ble_connected_net_notify, 1000);

                struct net_event net = {0};
                net.arg = "net";
                net.event = NET_NTP_GET_TIME_SUCC + 34;
                net_event_notify(NET_EVENT_FROM_USER, &net);
#endif
            }
        } else {
            //app_send_user_data(ATT_CHARACTERISTIC_ae82_01_VALUE_HANDLE, rsp_cmd2, sizeof(rsp_cmd2), ATT_OP_AUTO_READ_CCC);
            user_app_send_user_data(ATT_CHARACTERISTIC_ae82_01_VALUE_HANDLE, BLE_WIFI_CONNECT_FAILED, ATT_OP_AUTO_READ_CCC);
            user_buf_offset = 0;
            user_data_size = 0;
            memset(user_buf, 0, sizeof(user_buf));
        }
    }
}

static int payment_msg_send(void *priv)
{
    char username[64] = {0}, clientid[64] = {0}, bind_token[64] = {0};
    u16 len;
    u8 msg_cmd[128] = {0};
    char tmp_buf[256] = {0};

    get_page_turning_profile(username, NULL);
    get_payment_bind_msg(NULL, clientid, bind_token);

    sprintf(tmp_buf, "{\"clientid\":\"%s\",\"username\":\"%s\",\"bind_token\":\"%s\"}", clientid, username, bind_token);
    len = strlen(tmp_buf) + 2;
    strcpy((char *)msg_cmd, "JL");
    msg_cmd[2] = (u8)len >> 8;
    msg_cmd[3] = (u8)len;
    msg_cmd[4] = 0x10;
    msg_cmd[5] = 0x04;
    strcpy((char *)&msg_cmd[6], tmp_buf);
    extern u16 CRC16(const void *ptr, u32 len);
    u16 crc16 = CRC16(&msg_cmd[2], len + 2);
    msg_cmd[len + 4] = crc16 >> 8;
    msg_cmd[len + 5] = crc16;
    msg_cmd[6 + len] = 0xff;
    put_buf(msg_cmd, len + 7);
    app_send_user_data(ATT_CHARACTERISTIC_ae82_01_VALUE_HANDLE, msg_cmd, len + 7, ATT_OP_AUTO_READ_CCC);
    sys_timeout_add(0, (void (*)(void *))ble_module_enable, 100);
    return 0;
}

#if (defined CONFIG_WIFI_ENABLE || defined CONFIG_EXT_WIFI_ENABLE) && !IP_NAPT
#define BLE_WIFI_SCAN_MAX_AP  15

int ble_send_wifi_scan_result(struct wifi_scan_ssid_info *info, u32 num)
{
    u8 frame[512];
    u32 i;
    u32 count = (num > BLE_WIFI_SCAN_MAX_AP) ? BLE_WIFI_SCAN_MAX_AP : num;
    char json_buf[800];
    u32 json_offset = 0;
    u16 Length, crc, total_len;

    memset(frame, 0, sizeof(frame));
    memset(json_buf, 0, sizeof(json_buf));

    // 构建JSON数据
    json_offset += snprintf(json_buf + json_offset, sizeof(json_buf) - json_offset, "{\"wifi_list\":[");
    for (i = 0; i < count; i++) {
        if (i > 0) {
            json_offset += snprintf(json_buf + json_offset, sizeof(json_buf) - json_offset, ",");
        }
        json_offset += snprintf(json_buf + json_offset, sizeof(json_buf) - json_offset,
                                "{\"ssid\":\"%s\",\"rssi\":%d}", info[i].ssid, info[i].rssi_db);
    }
    json_offset += snprintf(json_buf + json_offset, sizeof(json_buf) - json_offset, "]}");

    printf("ble_send_wifi_scan_result: %s (len=%u)\n", json_buf, json_offset);

    // 帧头: 'J' 'L'
    frame[0] = 'J';
    frame[1] = 'L';

    // 命令码: 0x10(一级) + 0x06(二级: WiFi扫描结果)
    frame[4] = 0x10;
    frame[5] = 0x06;

    // 复制JSON数据到帧中
    memcpy(&frame[6], json_buf, json_offset);

    // 计算长度字段: JSON数据长度 + 命令码2字节
    Length = json_offset + 2;
    frame[2] = (Length >> 8) & 0xFF;
    frame[3] = Length & 0xFF;

    // 计算CRC16 (从长度字段开始，包含长度+命令码+JSON数据)
    crc = CRC16(&frame[2], Length + 2);
    total_len = 6 + json_offset;    // 帧头6 + JSON数据
    frame[total_len++] = (crc >> 8) & 0xFF;
    frame[total_len++] = crc & 0xFF;
    frame[total_len++] = 0xFF;      // 结束符

    // 通过ae82 NOTIFY特征值发送给小程序
    printf("BLE send wifi scan result frame (%u bytes):\n", total_len);
    put_buf(frame, total_len);
    app_send_user_data(ATT_CHARACTERISTIC_ae82_01_VALUE_HANDLE, frame, total_len, ATT_OP_AUTO_READ_CCC);

    return 0;
}
#endif

/* LISTING_END */
/*
 * @section ATT Write
 *
 * @text The only valid ATT write in this example is to the Client Characteristic Configuration, which configures notification
 * and indication. If the ATT handle matches the client configuration handle, the new configuration value is stored and used
 * in the heartbeat handler to decide if a new value should be sent. See Listing attWrite.
 */

/* LISTING_START(attWrite): ATT Write */
static int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    int result = 0;
    u16 tmp16;
    u16 handle = att_handle;
    log_info("write_callback, handle= 0x%04x,size = %d\n", handle, buffer_size);

    switch (handle) {

    case ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE:
        break;

    case ATT_CHARACTERISTIC_ae82_01_CLIENT_CONFIGURATION_HANDLE:
        set_ble_work_state(BLE_ST_NOTIFY_IDICATE);
        check_connetion_updata_deal();
        log_info("\n------write ccc:%04x,%02x\n", handle, buffer[0]);
        att_set_ccc_config(handle, buffer[0]);
        user_buf_offset = 0;
        user_data_size = 0;
        memset(user_buf, 0, sizeof(user_buf));
        complete = 0;
        break;

    case ATT_CHARACTERISTIC_ae81_01_VALUE_HANDLE:
        printf("\n-ae81_rx(%d):", buffer_size);
        printf_buf(buffer, buffer_size);

        if (!user_buf_offset) {
            if (buffer[0] == 'J' && buffer[1] == 'L' && buffer[4] == 0x10 && buffer[5] == 0x01) {
                user_data_size = (buffer[2] << 8) + buffer[3];
                user_buf_offset += buffer_size;
                if (buffer_size < sizeof(user_buf) && user_data_size < 32 + 64 + 24) {
                    memcpy(user_buf, buffer, buffer_size);
                    check_net_info_if_recv_complete();
                    break;
                }
            }
#if (defined CONFIG_WIFI_ENABLE || defined CONFIG_EXT_WIFI_ENABLE) && !IP_NAPT
            // 小程序发送帧: JL + 0x00,0x02 + 0x10,0x05 + CRC16(2B) + 0xFF
            if (buffer[0] == 'J' && buffer[1] == 'L' && buffer[4] == 0x10 && buffer[5] == 0x05) {
                printf("\n>>> BLE recv scan WiFi request (0x10, 0x05)\n");

                // 临时拷贝完整帧用于CRC校验
                u8 tmp_frame[32];
                u16 tmp_data_size = (buffer[2] << 8) + buffer[3];
                memset(tmp_frame, 0, sizeof(tmp_frame));
                if (buffer_size <= sizeof(tmp_frame)) {
                    memcpy(tmp_frame, buffer, buffer_size);
                }

                // CRC16校验：从长度字段开始，长度 = data_size + 2
                u16 recv_crc = (tmp_frame[buffer_size - 3] << 8) | tmp_frame[buffer_size - 2];
                u16 calc_crc = CRC16(&tmp_frame[2], tmp_data_size + 2);

                if (recv_crc == calc_crc) {
                    printf(">>> Scan request CRC OK, triggering WiFi scan...\n");
#ifdef CONFIG_WIFI_ENABLE
                    extern int wifi_scan_start_request(void (*done_cb)(void *ssid, int num), int waite_timerms);
                    wifi_scan_start_request(ble_send_wifi_scan_result, 0);
#endif
                } else {
                    printf(">>> Scan request CRC FAIL! recv=0x%04X calc=0x%04X\n", recv_crc, calc_crc);
                }
                break;  // 不进入后续的配网/支付处理
            }
#endif
            //////////
#if (defined PAYMENT_AUDIO_SDK_ENABLE)
            if (buffer[0] == 'J' && buffer[1] == 'L' && buffer[4] == 0x10 && buffer[5] == 0x03) {
                user_data_size = (buffer[2] << 8) + buffer[3];
                if (user_data_size == 4) {
                    wait_completion(check_profile, payment_msg_send, NULL, NULL);
                    break;
                }
            }
#endif
            ///////////
        } else {
            if (user_buf_offset + buffer_size < sizeof(user_buf)) {
                memcpy(user_buf + user_buf_offset, buffer, buffer_size);
                user_buf_offset += buffer_size;
                if (user_buf_offset < user_data_size + 7) {
                    break;
                }
                check_net_info_if_recv_complete();
                break;
            }
        }

        puts("error : bt net config fail !!!\n");

        user_buf_offset = 0;
        user_data_size = 0;
        memset(user_buf, 0, sizeof(user_buf));
        //app_send_user_data(ATT_CHARACTERISTIC_ae82_01_VALUE_HANDLE, rsp_cmd2, sizeof(rsp_cmd2), ATT_OP_AUTO_READ_CCC);
        user_app_send_user_data(ATT_CHARACTERISTIC_ae82_01_VALUE_HANDLE, BLE_WIFI_CONNECT_FAILED, ATT_OP_AUTO_READ_CCC);
        break;

    case ATT_CHARACTERISTIC_ae87_01_VALUE_HANDLE:
        //todo
        printf("buffer_size:%d", buffer_size);
        put_buf(buffer, buffer_size);

#if (CONFIG_LTE_VENDOR == VENDOR_GT108)
        put_buf(buffer, buffer_size);
        //模拟输入子系统(上报按键事件)
        struct key_event key_ins = {0};
        key_ins.type = SYS_KEY_EVENT;

        /*
         *协议头J+L、数据长度（2字节，小端模式）、功能码 0x10，子功能码 0x01、参数（2字节），
         *最后字节 0x00 表示 4G 模式
        */
        //{0x4a, 0x4c, 0x00, 0x02, 0x10, 0x01, 0x00, 0x00};
        //{0x4a, 0x4c, 0x00, 0x02, 0x10, 0x01, 0x00, 0x01};
        //{0x4a, 0x4c, 0x00, 0x02, 0x10, 0x01, 0x00, 0xff};

        if (buffer_size == 8 && buffer[4] == 0x10) { //0x10-模式切换
            switch (buffer[7]) {
            case 0x00: //切换到4G模式
                printf("phone Switch to 4G mode\n");
                //0为wifi，1为4G
                sys_net_channel_write(buffer[7]);

                key_ins.action = KEY_EVENT_FOURTH_CLICK;
                key_ins.value = KEY_OK;
                if (key_event_remap(&key_ins)) {
                    key_event_notify(KEY_EVENT_FROM_KEY, &key_ins);
                }
                break;

            case 0x01: //切换到WIFI模式
                log_info("phone Switch to WIFI mode\n");
                sys_net_channel_write(buffer[7]);

                key_ins.action = KEY_EVENT_FOURTH_CLICK;
                key_ins.value = KEY_OK;
                if (key_event_remap(&key_ins)) {
                    key_event_notify(KEY_EVENT_FROM_KEY, &key_ins);
                }
                break;

            case 0xFF: //重启设备
                log_info("Reboot device\n");
                key_ins.action = KEY_EVENT_MORE_CLICK;
                key_ins.value = KEY_OK;
                if (key_event_remap(&key_ins)) {
                    key_event_notify(KEY_EVENT_FROM_KEY, &key_ins);
                }
                break;

            default:
                log_info("Unknown command: 0x%02X\n", buffer[7]);
                break;
            }
        } else if (buffer_size == 8 && buffer[4] == 0x11) { //0x11-外设操作相关，比如切换SIM卡
            int net_ch = sys_net_channel_read();
            if (net_ch == 0) {
                break;
            }
            switch (buffer[7]) { //其他参数
            case 0x00: //切换到卡1
                printf("phone Switch to card1\n");

                key_ins.action = KEY_EVENT_FIRTH_CLICK;
                key_ins.value = KEY_OK;
                key_ins.msg = buffer[7] + 1;
                if (key_event_remap(&key_ins)) {
                    key_event_notify(KEY_EVENT_FROM_KEY, &key_ins);
                }
                break;

            case 0x01: //切换到卡2
                printf("phone Switch to card2\n");
                key_ins.action = KEY_EVENT_FIRTH_CLICK;
                key_ins.value = KEY_OK;
                key_ins.msg = buffer[7] + 1;
                if (key_event_remap(&key_ins)) {
                    key_event_notify(KEY_EVENT_FROM_KEY, &key_ins);
                }
                break;

            default:
                log_info("Unknown command: 0x%02X\n", buffer[7]);
                break;
            }
        } else {
            printf("Length less than 8 bytes, illegal!!!");
        }
#endif
        break;
    default:
        break;
    }

    return 0;
}

static int app_send_user_data(u16 handle, const u8 *data, u16 len, u8 handle_type)
{
    u32 ret = APP_BLE_NO_ERROR;

    if (!con_handle) {
        return APP_BLE_OPERATION_ERROR;
    }

    os_mutex_pend(&mutex, 0);

    if (!att_get_ccc_config(handle + 1)) {
        log_info("fail,no write ccc!!!,%04x\n", handle + 1);
        os_mutex_post(&mutex);
        return APP_BLE_NO_WRITE_CCC;
    }

    ret = ble_user_cmd_prepare(BLE_CMD_ATT_SEND_DATA, 4, handle, data, len, handle_type);
    if (ret == BLE_BUFFER_FULL) {
        ret = APP_BLE_BUFF_FULL;
    }

    os_mutex_post(&mutex);

    if (ret) {
        log_info("app_send_fail:%d !!!!!!\n", ret);
    }
    return ret;
}

//------------------------------------------------------
static int make_set_adv_data(void)
{
    u8 offset = 0;
    u8 *buf = adv_data;
    u8 mac[6];

    le_controller_get_mac(mac);

    /* offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_FLAGS, 0x18, 1); */
    /* offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_FLAGS, 0x1A, 1); */
    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_FLAGS, 0x06, 1);
    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_COMPLETE_16BIT_SERVICE_UUIDS, 0xAF00, 2);
    offset += make_eir_packet_data(&buf[offset], offset, HCI_EIR_DATATYPE_MANUFACTURER_SPECIFIC_DATA, mac, 6);

    if (offset > ADV_RSP_PACKET_MAX) {
        puts("***adv_data overflow!!!!!!\n");
        return -1;
    }
    log_info("adv_data(%d):", offset);
    log_info_hexdump(buf, offset);
    ble_user_cmd_prepare(BLE_CMD_ADV_DATA, 2, offset, buf);
    return 0;
}

static int make_set_rsp_data(void)
{
    u8 offset = 0;
    u8 *buf = scan_rsp_data;
    u8 mac[6];

    le_controller_get_mac(mac);

    u8 name_len = gap_device_name_len;
    u8 vaild_len = ADV_RSP_PACKET_MAX - (offset + 2);
    if (name_len > vaild_len) {
        name_len = vaild_len;
    }
    offset += make_eir_packet_data(&buf[offset], offset, HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME, (void *)gap_device_name, name_len);
    offset += make_eir_packet_data(&buf[offset], offset, HCI_EIR_DATATYPE_MANUFACTURER_SPECIFIC_DATA, mac, 6);

    if (offset > ADV_RSP_PACKET_MAX) {
        puts("***rsp_data overflow!!!!!!\n");
        return -1;
    }

    log_info("rsp_data(%d):", offset);
    log_info_hexdump(buf, offset);
    ble_user_cmd_prepare(BLE_CMD_RSP_DATA, 2, offset, buf);
    return 0;
}

//广播参数设置
static void advertisements_setup_init()
{
    uint8_t adv_type = ADV_IND;
    uint8_t adv_channel = ADV_CHANNEL_ALL;
    int   ret = 0;

    ble_user_cmd_prepare(BLE_CMD_ADV_PARAM, 3, ADV_INTERVAL_MIN, adv_type, adv_channel);

    ret |= make_set_adv_data();
    ret |= make_set_rsp_data();

    if (ret) {
        puts("advertisements_setup_init fail !!!!!!\n");
        return;
    }
}

#define PASSKEY_ENTER_ENABLE      0 //输入passkey使能，可修改passkey
//重设passkey回调函数，在这里可以重新设置passkey
//passkey为6个数字组成，十万位、万位。。。。个位 各表示一个数字 高位不够为0
static void reset_passkey_cb(u32 *key)
{
#if 1
    u32 newkey = rand32();//获取随机数

    newkey &= 0xfffff;
    if (newkey > 999999) {
        newkey = newkey - 999999; //不能大于999999
    }
    *key = newkey; //小于或等于六位数
    printf("set new_key= %06u\n", *key);
#else
    *key = 123456; //for debug
#endif
}

extern void reset_PK_cb_register(void (*reset_pk)(u32 *));
void ble_sm_setup_init(io_capability_t io_type, u8 auth_req, uint8_t min_key_size, u8 security_en)
{
    //setup SM: Display only
    sm_init();
    sm_set_io_capabilities(io_type);
    sm_set_authentication_requirements(auth_req);
    sm_set_encryption_key_size_range(min_key_size, 16);
    sm_set_request_security(security_en);
    sm_event_callback_set(&cbk_sm_packet_handler);

    if (io_type == IO_CAPABILITY_DISPLAY_ONLY) {
        reset_PK_cb_register(reset_passkey_cb);
    }
}

extern void le_device_db_init(void);
void ble_profile_init(void)
{
    printf("ble profile init\n");
    /* Auto-connect WiFi can start scanning without ever calling bt_ble_init(). */
    if (!os_mutex_valid(&mutex)) {
        os_mutex_create(&mutex);
    }
    le_device_db_init();

#if PASSKEY_ENTER_ENABLE
    ble_sm_setup_init(IO_CAPABILITY_DISPLAY_ONLY, SM_AUTHREQ_BONDING | SM_AUTHREQ_MITM_PROTECTION, 7, TCFG_BLE_SECURITY_EN);
#else
    ble_sm_setup_init(IO_CAPABILITY_NO_INPUT_NO_OUTPUT, SM_AUTHREQ_BONDING | SM_AUTHREQ_MITM_PROTECTION, 7, TCFG_BLE_SECURITY_EN);
#endif

    /* setup ATT server */
    att_server_init(profile_data, att_read_callback, att_write_callback);
    att_server_register_packet_handler(cbk_packet_handler);
    /* gatt_client_register_packet_handler(packet_cbk); */

    // register for HCI events
    hci_event_callback_set(&cbk_packet_handler);
    /* ble_l2cap_register_packet_handler(packet_cbk); */
    /* sm_event_packet_handler_register(packet_cbk); */
    le_l2cap_register_packet_handler(&cbk_packet_handler);
}

static int set_adv_enable(void *priv, u32 en)
{
    ble_state_e next_state, cur_state;
    int ret;

#if BLE_OTA_SCAN_ENABLED
    if (en && ota_scan_running) {
        return APP_BLE_OPERATION_ERROR;
    }
#endif
    if (!adv_ctrl_en && en) {
        return APP_BLE_OPERATION_ERROR;
    }

    if (con_handle) {
        return APP_BLE_OPERATION_ERROR;
    }

    if (en) {
        next_state = BLE_ST_ADV;
    } else {
        next_state = BLE_ST_IDLE;
    }

    cur_state =  get_ble_work_state();
    switch (cur_state) {
    case BLE_ST_ADV:
    case BLE_ST_IDLE:
    case BLE_ST_INIT_OK:
    case BLE_ST_NULL:
    case BLE_ST_DISCONN:
        break;
    default:
        return APP_BLE_OPERATION_ERROR;
        break;
    }

    if (cur_state == next_state) {
        return APP_BLE_NO_ERROR;
    }
    log_info("adv_en:%d\n", en);
    if (en) {
        advertisements_setup_init();
    }
    ret = ble_user_cmd_prepare(BLE_CMD_ADV_ENABLE, 1, en);
    if (ret != BLE_CMD_RET_SUCESS) {
        printf("[BLE][%u ms] BLE_ADV_CMD_FAILED en=%u ret=%d\n", sys_timer_get_ms(), en, ret);
        return APP_BLE_OPERATION_ERROR;
    }
#if BLE_OTA_SCAN_ENABLED
    ota_adv_active = !!en;
#endif
    set_ble_work_state(next_state);
    return APP_BLE_NO_ERROR;
}

static int ble_disconnect(void *priv)
{
    if (con_handle) {
        if (BLE_ST_SEND_DISCONN != get_ble_work_state()) {
            log_info(">>>ble send disconnect\n");
            set_ble_work_state(BLE_ST_SEND_DISCONN);
            ble_user_cmd_prepare(BLE_CMD_DISCONNECT, 1, con_handle);
        } else {
            log_info(">>>ble wait disconnect...\n");
        }
        return APP_BLE_NO_ERROR;
    } else {
        return APP_BLE_OPERATION_ERROR;
    }
}

static int get_buffer_vaild_len(void *priv)
{
    u32 vaild_len = 0;
    ble_user_cmd_prepare(BLE_CMD_ATT_VAILD_LEN, 1, &vaild_len);
    return vaild_len;
}

static int app_send_user_data_do(void *priv, u8 *data, u16 len)
{
#if PRINT_DMA_DATA_EN
    if (len < 128) {
        log_info("-le_tx(%d):");
        log_info_hexdump(data, len);
    } else {
        putchar('L');
    }
#endif
    return app_send_user_data(ATT_CHARACTERISTIC_ae82_01_VALUE_HANDLE, data, len, ATT_OP_AUTO_READ_CCC);
}

static int app_send_user_data_check(u16 len)
{
    u32 buf_space = get_buffer_vaild_len(0);
    if (len <= buf_space) {
        return 1;
    }
    return 0;
}

static int regiest_wakeup_send(void *priv, void *cbk)
{
    ble_resume_send_wakeup = cbk;
    return APP_BLE_NO_ERROR;
}

static int regiest_recieve_cbk(void *priv, void *cbk)
{
    channel_priv = (u32)priv;
    app_recieve_callback = cbk;
    return APP_BLE_NO_ERROR;
}

static int regiest_state_cbk(void *priv, void *cbk)
{
    channel_priv = (u32)priv;
    app_ble_state_callback = cbk;
    return APP_BLE_NO_ERROR;
}

void bt_ble_adv_enable(u8 enable)
{
    set_adv_enable(0, enable);
}

/* Caller holds mutex; scan and provisioning advertising never start together. */
static void ble_module_enable_locked(u8 en)
{
    log_info("mode_en:%d\n", en);
    if (en) {
#if BLE_OTA_SCAN_ENABLED
        ota_wifi_ready = 0;
        ota_provision_pending = 1;
        if (ble_ota_scan_stop()) {
            ota_scan_retry_ms = sys_timer_get_ms();
            return; /* Timer retries the stop before enabling provisioning. */
        }
        ota_provision_pending = 0;
#endif
        printf("+++++++++++++enable ble adv+++++++++++++");
        extern u8 get_ble_gatt_role(void);
        if (1 == get_ble_gatt_role()) {
            ble_stack_gatt_role(0);
            // register for HCI events
            hci_event_callback_set(&cbk_packet_handler);
            le_l2cap_register_packet_handler(&cbk_packet_handler);
        }
        complete = 0;
        adv_ctrl_en = 1;
        bt_ble_adv_enable(1);
    } else {
#if BLE_OTA_SCAN_ENABLED
        ota_provision_pending = 0;
#endif
        printf("+++++++++++++disable ble adv+++++++++++++");
        if (con_handle) {
            if (complete) {
                return;
            }
            adv_ctrl_en = 0;
            ble_disconnect(NULL);
        } else {
            bt_ble_adv_enable(0);
            adv_ctrl_en = 0;
        }
        complete = 0;
    }
}

static void ble_module_enable(u8 en)
{
    os_mutex_pend(&mutex, 0);
#if BLE_OTA_SCAN_ENABLED
    if (en) {
        ++ota_cfg_generation;
        if (ota_cfg_exit_timer) {
            sys_timeout_del(ota_cfg_exit_timer);
            ota_cfg_exit_timer = 0;
        }
        ota_cfg_exit_pending = 0;
    }
#endif
    ble_module_enable_locked(en);
    os_mutex_post(&mutex);
}

static const char ble_ext_name[] = "(BLE)";
static char bl_name[32];

void bt_ble_init(void)
{
    log_info("***** ble_init******\n");
//    const char *name_p = "BLUFI_DEVICE";//bt_get_local_name();

    unsigned char mac[6];

    bt_ble_get_mac(mac);
#ifdef BT_DEV_NAME_LONE_ENABLE
    sprintf(bl_name, "%s%02X%02X%02X%02X%02X%02X", BLE_DEV_NAME, mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
#else
    sprintf(bl_name, "%s%02X%02X%02X", BLE_DEV_NAME, mac[3], mac[4], mac[5]);
#endif

#if (defined CONFIG_WIFI_ENABLE || defined CONFIG_EXT_WIFI_ENABLE)
#if defined CONFIG_EXT_WIFI_ENABLE
    //strcat(bl_name, "-SCAN");//支持ble wifi扫描
    strcat(bl_name, "-5G");//支持5Gwifi SCAN
#else
    strcat(bl_name, "-SCAN");//支持ble wifi扫描
#endif
#endif

    if (is_production_test_enter(0)) {
#ifdef PRODUCTION_ALL_TEST_BT_NAME
        sprintf(bl_name, "%s%s", BLE_DEV_NAME, PRODUCTION_ALL_TEST_BT_NAME);
#endif
    }
    const char *name_p = bl_name;
    printf("-> ble_name : %s \n", bl_name);
    u8 ext_name_len = 0;//sizeof(ble_ext_name) - 1;

    if (!os_mutex_valid(&mutex)) {
        os_mutex_create(&mutex);
    }

    gap_device_name_len = strlen(name_p);
    if (gap_device_name_len > BT_NAME_LEN_MAX - ext_name_len) {
        gap_device_name_len = BT_NAME_LEN_MAX - ext_name_len;
    }

    //增加后缀，区分名字
    memset(gap_device_name, 0, sizeof(gap_device_name));
    memcpy(gap_device_name, name_p, gap_device_name_len);
//    memcpy(&gap_device_name[gap_device_name_len], "(BLE)", ext_name_len);
    gap_device_name_len += ext_name_len;

    log_info("ble name(%d): %s \n", gap_device_name_len, gap_device_name);

    set_ble_work_state(BLE_ST_INIT_OK);
    ble_module_enable(1);
}
char *ble_get_name(void)
{
    return bl_name;
}
static void stack_exit(void)
{
    ble_module_enable(0);
    /* set_ble_work_state(BLE_ST_SEND_STACK_EXIT); */
    /* ble_user_cmd_prepare(BLE_CMD_STACK_EXIT, 1, 0); */
    /* set_ble_work_state(BLE_ST_STACK_EXIT_COMPLETE); */
}

void bt_ble_exit(void)
{
    log_info("***** ble_exit******\n");

    stack_exit();
}

void ble_app_disconnect(void)
{
    ble_disconnect(NULL);
}

//添加配网成功回传mac地址
int ble_connected_net_notify(void)
{
    int size = 0;
    int ret;
    int Length = 0;
    unsigned char mac[6];
    u16 crc;

    bt_ble_get_mac(mac);
    memset(rsp_mac_cmd, 0, sizeof(rsp_mac_cmd));
    rsp_mac_cmd[0] = 0x4A;//"J"
    rsp_mac_cmd[1] = 0x4C;//"L"
    rsp_mac_cmd[4] = 0x10;//一级命令
    rsp_mac_cmd[5] = 0x02;//二级命令
    size += 6;//加上length长度2字节 + 4字节
//
    //{"status":0,"mac":"3D907698D940-$2y$10$c-B1N1"}
#if (CONFIG_LTE_VENDOR == VENDOR_GT108)
    ret = sprintf(&rsp_mac_cmd[size], "{\"status\":0, \"mode\":255,\"mac\":\"%02X%02X%02X%02X%02X%02X\"", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
#else
    ret = sprintf(&rsp_mac_cmd[size], "{\"status\":0,\"mac\":\"%02X%02X%02X%02X%02X%02X\"", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
#endif
    size += ret;
    Length += ret;
    char *uuid = app_user_product_uuid();
    if (!uuid) {
        printf("attention: use default product uuid!!!\r\n");
        //use product uuid of qy-ai
        uuid = user_product_uuid_default();
    }

    char *batch_num = user_product_batch_num();
    if (uuid && batch_num) {
        ret = sprintf(&rsp_mac_cmd[size], ",\"uuid\":\"%s\"", uuid);
        size += ret;
        Length += ret;
        ret = sprintf(&rsp_mac_cmd[size], ",\"batch_num\":\"%s\"", batch_num);
        size += ret;
        Length += ret;
        ret = sprintf(&rsp_mac_cmd[size], ",\"language\":\"%s\"", qyai_chat_language());
        size += ret;
        Length += ret;
        ret = sprintf(&rsp_mac_cmd[size], ",\"version\":\"%s.%s\"", format_date(), OTA_VERSON);
        size += ret;
        Length += ret;

        if (user_rand_mac_20()) {
            ret = sprintf(&rsp_mac_cmd[size], ",\"device_id\":\"%s%02x%02x%02x%02x%02x%02x\"", user_rand_mac_20(), mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
            size += ret;
            Length += ret;
        }

        extern char *qyai_appid_version(void);
        ret = sprintf(&rsp_mac_cmd[size], ",\"appid\":\"%s\"", qyai_appid_version());
        size += ret;
        Length += ret;
    }
    ret = sprintf(&rsp_mac_cmd[size], "\}");
    if (ret > 0) {
        size += ret;
        Length += ret + 2;//value

        rsp_mac_cmd[2] = (Length >> 8) & 0xFF;//length
        rsp_mac_cmd[3] = Length & 0xFF;//length

        crc = CRC16(&rsp_mac_cmd[2], Length + 2);//crc16
        rsp_mac_cmd[size++] = (crc >> 8) & 0xFF;
        rsp_mac_cmd[size++] = crc & 0xFF;
        rsp_mac_cmd[size++] = 0xFF;//end
        //printf("-> %s \n", &rsp_mac_cmd[6]);
        printf("rsp_mac_cmd: %d-%d-%s", rsp_mac_cmd[2] * 256 + rsp_mac_cmd[3], size, &rsp_mac_cmd[6]);
        ret = app_send_user_data(ATT_CHARACTERISTIC_ae82_01_VALUE_HANDLE, rsp_mac_cmd, size, ATT_OP_AUTO_READ_CCC);
        if (ret != APP_BLE_NO_ERROR) {
            printf("[BLE][%u ms] BLE_NOTIFY_SUCCESS_FAILED len=%d ret=%d\n", sys_timer_get_ms(), size, ret);
        }
    } else {
        //app_send_user_data(ATT_CHARACTERISTIC_ae82_01_VALUE_HANDLE, rsp_cmd0, sizeof(rsp_cmd0), ATT_OP_AUTO_READ_CCC);
        user_app_send_user_data(ATT_CHARACTERISTIC_ae82_01_VALUE_HANDLE, BLE_WIFI_CONNECTED, ATT_OP_AUTO_READ_CCC);
    }
    return 0;
}

void ble_cfg_net_result_notify(int event)
{
    if (!complete) {
        return;
    }

    if (event == NET_CONNECT_TIMEOUT_NOT_FOUND_SSID) {
        printf("*********ble end NET_CONNECT_TIMEOUT_NOT_FOUND_SSID*********");
        //app_send_user_data(ATT_CHARACTERISTIC_ae82_01_VALUE_HANDLE, rsp_cmd3, sizeof(rsp_cmd3), ATT_OP_AUTO_READ_CCC);
        user_app_send_user_data(ATT_CHARACTERISTIC_ae82_01_VALUE_HANDLE, BLE_WIFI_CONNECT_NO_FOUND_SSID, ATT_OP_AUTO_READ_CCC);
    } else if (event == NET_CONNECT_ASSOCIAT_FAIL || event == NET_CONNECT_ASSOCIAT_TIMEOUT) {
        printf("*********ble send NET_CONNECT_ASSOCIAT_FAIL*********");
        //app_send_user_data(ATT_CHARACTERISTIC_ae82_01_VALUE_HANDLE, rsp_cmd4, sizeof(rsp_cmd4), ATT_OP_AUTO_READ_CCC);
        user_app_send_user_data(ATT_CHARACTERISTIC_ae82_01_VALUE_HANDLE, BLE_WIFI_CONNECT_ASSOCIAT_FAIL, ATT_OP_AUTO_READ_CCC);
    } else if (event == NET_EVENT_CONNECTED) {
        printf("*********lbe send net connect success, notify phone!!!*********");
//        app_send_user_data(ATT_CHARACTERISTIC_ae82_01_VALUE_HANDLE, rsp_cmd0, sizeof(rsp_cmd0), ATT_OP_AUTO_READ_CCC);
//        user_app_send_user_data(ATT_CHARACTERISTIC_ae82_01_VALUE_HANDLE, BLE_WIFI_CONNECTED, ATT_OP_AUTO_READ_CCC);
        ble_connected_net_notify();
        complete = 0;
#if !defined PAYMENT_AUDIO_SDK_ENABLE && !CONFIG_POWER_ON_ENABLE_BLE
#if BLE_OTA_SCAN_ENABLED
        os_mutex_pend(&mutex, 0);
        ++ota_cfg_generation;
        if (ota_cfg_exit_timer) {
            sys_timeout_del(ota_cfg_exit_timer);
        }
        ota_cfg_exit_timer = sys_timeout_add((void *)ota_cfg_generation, ble_ota_cfg_exit_timeout, 1000);
        ota_cfg_exit_pending = 1;
        if (!ota_cfg_exit_timer) {
            printf("[BLE_OTA] CFG_EXIT_TIMER_FAILED\n");
        }
        os_mutex_post(&mutex);
#else
        sys_timeout_add(0, (void (*)(void *))ble_module_enable, 1000);
#endif
#endif
    }
}

static const struct ble_server_operation_t mi_ble_operation = {
    .adv_enable = set_adv_enable,
    .disconnect = ble_disconnect,
    .get_buffer_vaild = get_buffer_vaild_len,
    .send_data = (void *)app_send_user_data_do,
    .regist_wakeup_send = regiest_wakeup_send,
    .regist_recieve_cbk = regiest_recieve_cbk,
    .regist_state_cbk = regiest_state_cbk,
};

void ble_get_server_operation_table(struct ble_server_operation_t **interface_pt)
{
    *interface_pt = (void *)&mi_ble_operation;
}

#endif
