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

//获取内容列表
int http_ai_get_content_list(void (*set_callbak)(char *buf, int len));
//获取内容数据
int http_ai_get_content_data(char *class_id, char *keyword, void (*set_callbak)(char *buf, int len));

static OS_SEM web_sync_sem;
#define SYNC_TIME_OUT (500)

static void parse_content_data(const char *json_str)
{
    json_object *root_obj = NULL;
    json_object *data_obj = NULL;
    json_object *data_array = NULL;
    json_object *item_obj = NULL;
    json_object *parm = NULL;
    int i, array_len;

    //解析JSON字符串
    root_obj = json_tokener_parse(json_str);
    if (!root_obj) {
        printf("Failed to parse JSON\n");
        return;
    }

    //提取顶层字段
    printf("-----------------------------------------\n");
    parm = json_object_object_get(root_obj, "errorCode");
    if (parm) {
        printf("errorCode: %d\n", json_object_get_int(parm));
    }

    parm = json_object_object_get(root_obj, "message");
    if (parm) {
        printf("message: %s\n", json_object_get_string(parm));
    }

    //提取data对象中的分页信息
    data_obj = json_object_object_get(root_obj, "data");
    if (data_obj) {
        parm = json_object_object_get(data_obj, "current_page");
        if (parm) {
            printf("current_page: %d\n", json_object_get_int(parm));
        }

        parm = json_object_object_get(data_obj, "total");
        if (parm) {
            printf("total: %d\n", json_object_get_int(parm));
        }
    }

    //提取data数组中的内容
    data_array = json_object_object_get(data_obj, "data");
    if (data_array && json_object_is_type(data_array, json_type_array)) {
        array_len = json_object_array_length(data_array);
        printf("\nFound %d items:\n", array_len);

        for (i = 0; i < array_len; i++) {
            item_obj = json_object_array_get_idx(data_array, i);
            if (!item_obj) {
                continue;
            }

            printf("-----------------------------------------\n");
            parm = json_object_object_get(item_obj, "id");
            if (parm) {
                printf("id: %d\n", json_object_get_int(parm));
            }

            parm = json_object_object_get(item_obj, "title");
            if (parm) {
                printf("title: %s\n", json_object_get_string(parm));
            }

            parm = json_object_object_get(item_obj, "author");
            if (parm) {
                printf("author: %s\n", json_object_get_string(parm));
            }

            //提取content_text
            parm = json_object_object_get(item_obj, "content_text");
            if (parm) {
                const char *content = json_object_get_string(parm);
                printf("content_text: %s\n", content);
            }

            parm = json_object_object_get(item_obj, "create_at");
            if (parm) {
                printf("create_at: %s\n", json_object_get_string(parm));
            }
        }
    }

    printf("-----------------------------------------\n");
    json_object_put(root_obj); //释放内存
}

void http_get_content_list_cb(char *buf, int len)
{
    printf("%s:", buf);
    os_sem_post(&web_sync_sem);
}

void http_get_content_data_cb(char *buf, int len)
{
    printf("%s:", buf);
    parse_content_data(buf);
    os_sem_post(&web_sync_sem);
}

//获取内容列表
void http_ai_get_content_list_app(void)
{
    http_ai_get_content_list(http_get_content_list_cb);
    if (OS_TIMEOUT == os_sem_pend(&web_sync_sem, SYNC_TIME_OUT)) {
        printf("wrror: sem waitTime OUT: %d !!!", __LINE__);
    }
}

//获取内容数据
void http_ai_get_content_data_app(void)
{
    char *class_id = "1";
    char *keyword = "乌鸦喝水";
    http_ai_get_content_data(class_id, keyword, http_get_content_data_cb);
    if (OS_TIMEOUT == os_sem_pend(&web_sync_sem, SYNC_TIME_OUT)) {
        printf("wrror: sem waitTime OUT: %d !!!", __LINE__);
    }
}

void content_play_main(void)
{
    os_sem_create(&web_sync_sem, 0);
}

late_initcall(content_play_main);

