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

//闹钟上报
int http_ai_dev_alarm_dat_post(char *reminder_time, char *reminder_text, void (*set_callbak)(int err_code, char *buf));

static OS_SEM web_sync_sem;
#define SYNC_TIME_OUT (500)

void http_dev_alarm_report_cb(int code, char *msg)
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
void http_ai_dev_alarm_report_app(void)
{
    char *reminder_time = "2026-01-28 15:00:00";
    char *reminder_text = "30分钟后提醒我吃药";

    http_ai_dev_alarm_dat_post(reminder_time, reminder_text, http_dev_alarm_report_cb);
    if (OS_TIMEOUT == os_sem_pend(&web_sync_sem, SYNC_TIME_OUT)) {
        printf("wrror: sem waitTime OUT: %d !!!", __LINE__);
    }
}

void alarm_report_main(void)
{
    os_sem_create(&web_sync_sem, 0);
}

late_initcall(alarm_report_main);

