#include "app_config.h"

#ifdef PRODUCTION_YIERD_UART_CMD_ENABLE
#include "system/includes.h"
#include "syscfg/syscfg_id.h"
#include "net_update.h"

enum AiATCmd {
    AT_MAC_WIFI = 0,
    AT_MAC_BT,
    AT_MAC_BLE,
    AT_MAC_SYS,
    AT_MAC_WRITTEN,
    AT_MAC_ALL,

    AT_PWR_WIFI,
    AT_PWR_BT,
    AT_XOSC,
    AT_RF_ALL,

    AT_BOARD_NAME,
    AT_BOARD_VER,
    AT_FW_NAME,
    AT_FW_VER,
    AT_CUSTOM_ID,
    AT_PRODUCT_NAME,
    AT_BATCH_NUM,
    AT_BATCH,
    AT_SYS_INFO_ALL,

    AT_POWER,

    AT_TEST_START,
    AT_TEST_AUDIO,
    AT_TEST_IO,
    AT_TEST_NET,

    AT_SYS_RECOVER,

    AT_END,
};

enum AiATErrType {
    AT_UNKNOWN_CMD,
    AT_LOST_PARAM,
    AT_INVALID_VALUE,
    AT_LIMIT,
};

static struct AiAtCmdInfo {
    enum AiATCmd cmd;
    char *key;          // 匹配关键字
    char enable_req;    // 是否支持请求响应
    char enable_set;    // 是否支持设置响应
    char enable_ack;    // 是否支持主动上报
};

static const struct AiAtCmdInfo ai_at_cmd_list[] = {
    {AT_MAC_WIFI,   "MAC-WIFI",     true, false, false},
    {AT_MAC_BT,     "MAC-BT",       true, false, false},
    {AT_MAC_BLE,    "MAC-BLE",      true, false, false},
    {AT_MAC_SYS,    "MAC-SYS",      true, false, false},
    {AT_MAC_WRITTEN, "MAC-WRITTEN",  true, false, false},
    {AT_MAC_ALL,    "MAC-ALL",      true, false, false},

    {AT_PWR_WIFI,   "PWR-WIFI",  true, false, false},
    {AT_PWR_BT,     "PWR-BT",    true, false, false},
    {AT_XOSC,       "XOSC",      true, false, false},
    {AT_RF_ALL,     "RF-ALL",    true, false, false},

    {AT_BOARD_NAME,     "BOARD-NAME",    true, false, false},
    {AT_BOARD_VER,      "BOARD-VER",     true, false, false},
    {AT_FW_NAME,        "FW-NAME",       true, false, false},
    {AT_FW_VER,         "FW-VER",        true, false, false},
    {AT_CUSTOM_ID,      "CUSTOM-ID",     true, false, false},
    {AT_PRODUCT_NAME,   "PRODUCT-NAME",  true, false, false},
    {AT_BATCH_NUM,      "BATCH-NUM",     true, false, false},
    {AT_BATCH,          "BATCH",         true, false, false},
    {AT_SYS_INFO_ALL,   "SYS-INFO-ALL",  true, false, false},

    {AT_POWER,   "POWER",  true, false, false},

    {AT_TEST_START, "TEST-START",    true,  true, false},
    {AT_TEST_AUDIO, "TEST-AUDIO",    false, false, true},
    {AT_TEST_IO,    "TEST-IO",       false, false, true},
    {AT_TEST_NET,   "TEST-NET",      false, false, true},

    {AT_SYS_RECOVER, "SYS-RECOVER",   true, false, true},

    {AT_END, NULL, false, false, false},
};

int mac_2_str(u8* mac, char* str, int len)
{
    return snprintf(str, len, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

#define AI_AT_RET_BUF_SIZE  200
static int ai_at_ret(char* buf, int len)
{
    extern int ai_at_uart_send_data(char *buf, int len);
    return ai_at_uart_send_data(buf, len);
}

static int ai_at_ret_success(struct AiAtCmdInfo* info, char* param_value)
{
    char buf[AI_AT_RET_BUF_SIZE] = {0};
    snprintf(buf, AI_AT_RET_BUF_SIZE, "AT+%s=%s\r\n", info->key, param_value);
    return ai_at_ret(buf, strlen(buf));
}

static int ai_at_ret_status(struct AiAtCmdInfo* info, int status)
{
    char buf[AI_AT_RET_BUF_SIZE] = {0};
    snprintf(buf, AI_AT_RET_BUF_SIZE, "AT+%s=%d\r\n", info->key, status);
    return ai_at_ret(buf, strlen(buf));
}

static int ai_at_ret_err(enum AiATErrType err_type, struct AiAtCmdInfo* info, char* param_name, char* param_value)
{
#define RET_ERR_STR_SIZE   64
    char buf[RET_ERR_STR_SIZE] = {0};
    switch (err_type) {
    case AT_UNKNOWN_CMD:
        snprintf(buf, RET_ERR_STR_SIZE, "AT-ERR=%s\r\n", param_name);
        return ai_at_ret(buf, strlen(buf));
    case AT_LOST_PARAM:
        snprintf(buf, RET_ERR_STR_SIZE, "AT-ERR+%s=%s\r\n", info->key, param_name);
        return ai_at_ret(buf, strlen(buf));
    case AT_INVALID_VALUE:
        snprintf(buf, RET_ERR_STR_SIZE, "AT-ERR+%s=%s:%s\r\n", info->key, param_name, param_value);
        return ai_at_ret(buf, strlen(buf));
    case AT_LIMIT:
        snprintf(buf, RET_ERR_STR_SIZE, "AT-ERR+%s=limit:%s\r\n", info->key, param_value);
        return ai_at_ret(buf, strlen(buf));
    }
    return 0;
}

int get_wifi_rf_info(char* buf, int len)
{
    extern void wifi_get_mcs_dgain(u8 * mcs_dgain);
    extern int wifi_get_pa_trim_data(u8 * pa_data);
    u8 pa_data[7] = {0};
    u8 mcs_dgain[20] = {0};
    wifi_get_mcs_dgain(mcs_dgain);
    wifi_get_pa_trim_data(pa_data);
    return snprintf(buf, len, "%d,%d,%d,%d,%d,%d,%d;%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", \
                    pa_data[0], pa_data[1], pa_data[2], pa_data[3], pa_data[4], pa_data[5], pa_data[6], \
                    mcs_dgain[0], mcs_dgain[1], mcs_dgain[2], mcs_dgain[3], mcs_dgain[4], mcs_dgain[5], mcs_dgain[6], \
                    mcs_dgain[7], mcs_dgain[8], mcs_dgain[9], mcs_dgain[10], mcs_dgain[11], mcs_dgain[12], mcs_dgain[13], \
                    mcs_dgain[14], mcs_dgain[15], mcs_dgain[16], mcs_dgain[17], mcs_dgain[18], mcs_dgain[19]);
}

#include <ctype.h>
#include <string.h>

extern int wifi_get_flash_mac(char *mac);
// 判断用户是否写入过mac
int user_mac_has_written(void)
{
    printf("check user_mac_has_written\n");

    char sys_mac[6];
    char bt_mac[6];
    printf("check SYS MAC\n");
    if (syscfg_read(CFG_BT_MAC_ADDR, sys_mac, 6) != 6) {
        printf("check user_mac_has_written CFG_BT_MAC_ADDR failed\n");
        return 0;
    } else {
        put_buf(sys_mac, 6);
    }

    printf("check BT MAC\n");
    wifi_get_flash_mac(bt_mac);
    put_buf(bt_mac, 6);

    return strncmp(bt_mac, sys_mac, 6);
}

static int is_start_test = false;
static u16 test_code = 0;
int ai_at_is_test_start(void)
{
    return is_start_test;
}

int ai_at_test_finish(void)
{
    is_start_test = false;
    test_code = 0;
}

int ai_at_report_audio_test_result(int status)
{
    return ai_at_ret_status(ai_at_cmd_list + AT_TEST_AUDIO, status);
}

int ai_at_report_io_test_result(int status)
{
    return ai_at_ret_status(ai_at_cmd_list + AT_TEST_IO, status);
}

int ai_at_report_net_test_result(int status)
{
    return ai_at_ret_status(ai_at_cmd_list + AT_TEST_NET, status);
}

u16 ai_at_get_test_code(void)
{
    return test_code;
}

void ai_at_set_test_code(u16 tc)
{
    test_code = tc;
    is_start_test = true;
}

extern char *app_user_product_uuid(void);
//用户项目产品名称
extern char *app_user_product_name(void);
//用户批次号
extern int app_user_product_batch(void);
//用户批次号的数量，单位K
extern int app_user_product_batch_num(void);

__attribute__((weak)) char* app_cfg_get_board_name(void)      // 获取硬件型号名
{
    return "default-name";
}
__attribute__((weak)) char* app_cfg_get_board_version(void)   // 获取硬件版本名
{
    return "default-hwver";
}
__attribute__((weak)) char* app_cfg_get_software_name(void)   // 获取软件名
{
    return OTA_VERSON;
}
__attribute__((weak)) char* app_cfg_get_software_version(void)// 获取软件版本名
{
    return OTA_VERSON;
}
__attribute__((weak)) char* app_cfg_get_custon_id(void)       // 获取客户ID
{
    return "default-id";
}
__attribute__((weak)) char* app_cfg_get_product_name(void)    // 获取产品名
{
    return app_user_product_name();
}
__attribute__((weak)) int app_cfg_get_product_batch(void)     // 获取产品批次
{
    return app_user_product_batch();
}
__attribute__((weak)) int app_cfg_get_product_batch_num(void) // 获取产品批次数量
{
    return app_user_product_batch_num();
}


static int ai_at_callback_handle(struct AiAtCmdInfo* info, int is_set, char* data)
{
#define RET_BUF_SIZE    192
#define WIFI_RF_INFO_BUF_SIZE   128

//    extern char* app_cfg_get_board_name(void);      // 获取硬件型号名
//    extern char* app_cfg_get_board_version(void);   // 获取硬件版本名
//    extern char* app_cfg_get_software_name(void);   // 获取软件名
//    extern char* app_cfg_get_software_version(void);// 获取软件版本名
//    extern char* app_cfg_get_custon_id(void);       // 获取客户ID
//    extern char* app_cfg_get_product_name(void);    // 获取产品名
//    extern int app_cfg_get_product_batch(void);     // 获取产品批次
//    extern int app_cfg_get_product_batch_num(void); // 获取产品批次数量

    extern void wifi_get_xosc(u8 * xosc);
    extern void lib_make_ble_address(u8 * ble_address, u8 * edr_address);

    extern u8 fin_ble_power;
    extern u8 fin_bt_power;

    char ret[RET_BUF_SIZE] = {0};
    u8 mac[6] = {0};
    printf("[AI AT] handle cmd: %s, is_set: %d\n", info->key, is_set);
    switch (info->cmd) {
    case AT_MAC_WIFI: {
            wifi_get_flash_mac(mac);
            mac_2_str(mac, ret, RET_BUF_SIZE);
            return ai_at_ret_success(info, ret);
        }
        break;

    case AT_MAC_BT: {
            wifi_get_flash_mac(mac);
            mac_2_str(mac, ret, RET_BUF_SIZE);
            return ai_at_ret_success(info, ret);
        }
        break;

    case AT_MAC_BLE: {
            wifi_get_flash_mac(mac);
            u8 ble_mac[6];
            lib_make_ble_address(ble_mac, mac);
            mac_2_str(ble_mac, ret, RET_BUF_SIZE);
            return ai_at_ret_success(info, ret);
        }
        break;

    case AT_MAC_SYS: {
            if (syscfg_read(CFG_BT_MAC_ADDR, mac, 6) != 6) {
                return ai_at_ret_err(AT_LIMIT, info, NULL, "read fail");
            } else {
                mac_2_str(mac, ret, RET_BUF_SIZE);
                return ai_at_ret_success(info, ret);
            }
        }
        break;

    case AT_MAC_WRITTEN: {
            if (user_mac_has_written()) {
                return ai_at_ret_status(info, 1);
            } else {
                return ai_at_ret_status(info, 0);
            }
        }
        break;

    case AT_MAC_ALL: {
            wifi_get_flash_mac(mac);
            mac_2_str(mac, ret, RET_BUF_SIZE);
            ret[strlen(ret)] = ';';

            mac_2_str(mac, ret + strlen(ret), RET_BUF_SIZE - strlen(ret));
            ret[strlen(ret)] = ';';

            u8 ble_mac[6];
            lib_make_ble_address(ble_mac, mac);
            mac_2_str(ble_mac, ret + strlen(ret), RET_BUF_SIZE - strlen(ret));
            ret[strlen(ret)] = ';';

            if (syscfg_read(CFG_BT_MAC_ADDR, mac, 6) != 6) {
                snprintf(ret + strlen(ret), RET_BUF_SIZE - strlen(ret), "error");
            } else {
                mac_2_str(mac, ret + strlen(ret), RET_BUF_SIZE - strlen(ret));
            }
            ret[strlen(ret)] = ';';

            if (user_mac_has_written()) {
                snprintf(ret + strlen(ret), RET_BUF_SIZE - strlen(ret), "1");
            } else {
                snprintf(ret + strlen(ret), RET_BUF_SIZE - strlen(ret), "0");
            }

            return ai_at_ret_success(info, ret);

        }
        break;

    case AT_PWR_WIFI: {
            get_wifi_rf_info(ret, WIFI_RF_INFO_BUF_SIZE);
            return ai_at_ret_success(info, ret);
        }
        break;

    case AT_PWR_BT: {
            snprintf(ret, RET_BUF_SIZE, "%d,%d", fin_bt_power, fin_ble_power);
            return ai_at_ret_success(info, ret);
        }
        break;

    case AT_XOSC: {
            u8 xosc[2];
            wifi_get_xosc(xosc);
            snprintf(ret, RET_BUF_SIZE, "%d,%d", xosc[0], xosc[1]);
            return ai_at_ret_success(info, ret);
        }
        break;

    case AT_RF_ALL: {
            int len = get_wifi_rf_info(ret, WIFI_RF_INFO_BUF_SIZE);
            u8 xosc[2];
            wifi_get_xosc(xosc);
            snprintf(ret + len, RET_BUF_SIZE - WIFI_RF_INFO_BUF_SIZE, ";%d,%d;%d,%d", xosc[0], xosc[1], fin_bt_power, fin_ble_power);
            return ai_at_ret_success(info, ret);
        }
        break;

    case AT_BOARD_NAME: {
            return ai_at_ret_success(info, app_cfg_get_board_name());
        }
        break;

    case AT_BOARD_VER: {
            return ai_at_ret_success(info, app_cfg_get_board_version());
        }
        break;

    case AT_FW_NAME: {
            return ai_at_ret_success(info, app_cfg_get_software_name());
        }
        break;

    case AT_FW_VER: {
            return ai_at_ret_success(info, app_cfg_get_software_version());
        }
        break;

    case AT_CUSTOM_ID: {
            return ai_at_ret_success(info, app_cfg_get_custon_id());
        }
        break;

    case AT_PRODUCT_NAME: {
            return ai_at_ret_success(info, app_cfg_get_product_name());
        }
        break;

    case AT_BATCH: {
            return ai_at_ret_status(info, app_cfg_get_product_batch());
        }
        break;

    case AT_BATCH_NUM: {
            return ai_at_ret_status(info, app_cfg_get_product_batch_num());
        }
        break;

    case AT_SYS_INFO_ALL: {
            snprintf(ret, RET_BUF_SIZE, "%s;%s;%s;%s;%s;%s;%d;%d", \
                     app_cfg_get_board_name(), app_cfg_get_board_version(), \
                     app_cfg_get_software_name(), app_cfg_get_software_version(), \
                     app_cfg_get_custon_id(), app_cfg_get_product_name(), \
                     app_cfg_get_product_batch(), app_cfg_get_product_batch_num());
            return ai_at_ret_success(info, ret);
        }
        break;

    case AT_POWER: {
            int vbat_perc = 90;
#if TCFG_VBAT_CHECK_EN
            vbat_perc = vbat_check_percent();
#endif
            return ai_at_ret_status(info, vbat_perc);
        }

    case AT_TEST_START: {
            if (is_set) {
                test_code = atoi(data);
                ai_at_set_test_code(test_code);
                printf("[AI AT] start production test: %d\n", test_code);
                return ai_at_ret_status(info, 1);
            } else {
                return ai_at_ret_status(info, ai_at_is_test_start());
            }

        }
        break;
    case AT_SYS_RECOVER: {
            system_restore_factory_settings();
            return ai_at_ret_status(info, 1);
        }
        break;
    }
}


int ai_at_handle(char* buf, int len)
{
    printf("[AI AT] cmd: %s\n", buf);
    if (strncmp(buf, "AT?", 3) == 0) {
        ai_at_ret("AT=OK\r\n", 7);
        return 1;
    } else {
        if (strncmp(buf, "AT+", 3) != 0) {
            printf("[AI AT] unknow cmd: %s\n", buf);
            return 0;
        } else {
            buf += 3;
            len -= 3;
        }
        printf("[AI AT] cmd: %s\n", buf);
        int i;
        for (i = 0; ai_at_cmd_list[i].cmd != AT_END; i++) {
            int key_len = strlen(ai_at_cmd_list[i].key);
            int cmd_len = strlen(buf);

            if (cmd_len <= key_len) {
                //printf("[AI AT] skip cmd: %s\n", ai_at_cmd_list[i].key);
                continue;
            }

            if (strncmp(buf, ai_at_cmd_list[i].key, key_len) == 0) {
                if (buf[key_len] == '?') {
                    if (ai_at_cmd_list[i].enable_req) {
                        printf("[AI AT] attach req cmd: %s\n", ai_at_cmd_list[i].key);
                        ai_at_callback_handle(ai_at_cmd_list + i, false, NULL);
                    } else {
                        printf("[AI AT] disable req: %s\n", ai_at_cmd_list[i].key);
                        ai_at_ret_err(AT_LIMIT, ai_at_cmd_list + i, NULL, "req");
                    }
                } else if (buf[key_len] == '=') {
                    if (ai_at_cmd_list[i].enable_set) {
                        printf("[AI AT] attach set cmd: %s\n", ai_at_cmd_list[i].key);
                        ai_at_callback_handle(ai_at_cmd_list + i, true, buf + key_len + 1);
                    } else {
                        printf("[AI AT] disable set: %s\n", ai_at_cmd_list[i].key);
                        ai_at_ret_err(AT_LIMIT, ai_at_cmd_list + i, NULL, "set");
                    }
                } else {
                    printf("[AI AT] unknow key tail: %c\n", buf[key_len]);
                    ai_at_ret_err(AT_UNKNOWN_CMD, ai_at_cmd_list + 0, buf, NULL);
                }

                return 1;
            }
        }
        ai_at_ret_err(AT_UNKNOWN_CMD, ai_at_cmd_list + 0, buf, NULL);
    }
    return 0;
}


#endif
