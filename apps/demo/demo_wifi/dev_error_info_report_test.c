#include "app_config.h"
#include "system/includes.h"
#include "device/device.h"
#include "fs/fs.h"
#include "device/gpio.h"
#include "os/os_api.h"
#include "system/app_core.h"
#include "event/key_event.h"
#include "event/device_event.h"
#include "json_c/json.h"
#include "json_c/json_tokener.h"

//设备通信异常信息上报
int http_ai_dev_err_info_report(char *reason, void (*set_callbak)(int err_code, char *buf));

static OS_SEM web_sync_sem;
#define SYNC_TIME_OUT (500)

void http_dev_err_info_report_cb(int code, char *msg)
{
    if (code == 0) {
        printf("======OK======");
        printf("msg: %s", msg);
        printf("==============");
    } else {
        printf("======error cause======");
    }
    os_sem_post(&web_sync_sem);
}

//闹钟数据上报
void http_ai_dev_err_info_report_app(void)
{
    char *reason = "异常断开";
    http_ai_dev_err_info_report(reason, http_dev_err_info_report_cb);
    if (OS_TIMEOUT == os_sem_pend(&web_sync_sem, SYNC_TIME_OUT)) {
        printf("wrror: sem waitTime OUT: %d !!!", __LINE__);
    }
}

void err_info_report_main(void)
{
    os_sem_create(&web_sync_sem, 0);
}

late_initcall(err_info_report_main);

