#ifndef BLE_OTA_SCAN_H
#define BLE_OTA_SCAN_H

#include "app_config.h"
#include "bt_common.h"

/* Stages 3-4: identify OTA advertisements and reassemble the URL only. */
#if CONFIG_BLE_OTA_SCAN_ENABLE && TCFG_USER_BLE_ENABLE && defined(CONFIG_WIFI_ENABLE) && \
    (TCFG_BLE_DEMO_SELECT == DEF_BLE_DEMO_NET_CFG)
#define BLE_OTA_SCAN_ENABLED 1
void ble_ota_scan_stack_ready(void);
/* Latch application connectivity; controller operations run in the timer task. */
void ble_ota_scan_wifi_state(unsigned char ready);
/* Toggle OTA broadcast reception from the active application. */
void ble_ota_scan_toggle(void);
#else
#define BLE_OTA_SCAN_ENABLED 0
static inline void ble_ota_scan_stack_ready(void) {}
static inline void ble_ota_scan_wifi_state(unsigned char ready) { (void)ready; }
static inline void ble_ota_scan_toggle(void) {}
#endif

#endif
