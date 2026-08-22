#ifndef _IFLY_AIUI_SDK_COMMON_H
#define _IFLY_AIUI_SDK_COMMON_H

#include "oal_type.h"
#include "cJson.h"

#define IFLY_AIUI_DEVICE_ID_LEN 128

#define CONFIG_AIUI_DEV_FTR_N

#define IFLY_AIUI_OTA_ENABLE    1   // aiui ota¹¦ÄÜ¿ª¹Ø
// 使用weak属性避免重复定义错误
__attribute__((weak)) char AIUI_CLOCK_ENABLE = 1;    // 闹钟功能开关, 0:关闭， 1:开启

typedef void (*iot_clock_cb)(cJSON *data_json, cJSON *timestamp_json); // iot 闹钟消息回调

typedef struct {
    const char *appid;
    const char *apikey;
    const char *apisecret;
    const char *productkey;
    char device_id[IFLY_AIUI_DEVICE_ID_LEN + 1];
} t_ifly_aiui_sdk_common_data;

/**
 * @brief 注册IoT闹钟回调函数。(功能不需要，可以不调用)
 * @param cb 闹钟回调函数指针
 * @return void。
 */
void ifly_aiui_iot_clock_register_callback(iot_clock_cb cb);
/**
 * @brief AIUI参数初始化。
 * @param *data 指向t_ifly_aiui_sdk_common_data类型的结构体指针，
 * 其中appid和apikey可在aiui开放平台申请，
 * device_id是用户唯一ID（32位字符串，包括英文小写字母与数字，开发者需保证该值与终端用户一一对应）
 * @return void。
 */
void ifly_aiui_sdk_common_data_init(t_ifly_aiui_sdk_common_data* data);
/**
 * @brief 获取appid。
 * @param NULL
 * @return 指向appid的内存指针。
 */
int *ifly_aiui_sdk_common_get_appid();
/**
 * @brief 获取apikey。
 * @param NULL
 * @return 指向apikey的内存指针。
 */
int *ifly_aiui_sdk_common_get_api_key();
/**
 * @brief 获取apisecret。
 * @param NULL
 * @return 指向apisecret的内存指针。
 */
int *ifly_aiui_sdk_common_get_api_secret();
/**
 * @brief 获取productkey。
 * @param NULL
 * @return 指向productkey的内存指针。
 */
int *ifly_aiui_sdk_common_get_product_key();
/**
 * @brief 获取deviceid。
 * @param NULL
 * @return 指向deviceid的内存指针。
 */
int *ifly_aiui_sdk_common_get_deviceid();

#endif
