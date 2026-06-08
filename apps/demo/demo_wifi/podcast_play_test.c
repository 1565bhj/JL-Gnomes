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

//获取播客列表
int http_ai_get_podcast_list(void (*set_callbak)(char *buf, int len));
//获取播客内容
int http_ai_get_podcast_data(char *class_id, char *keyword, char *page_idx, char *per_page, void (*set_callbak)(char *buf, int len));

static OS_SEM web_sync_sem;
#define SYNC_TIME_OUT (500)

typedef struct {
    int id;
    char *title;
    char *audio_url;
} PodcastItem;

typedef struct {
    PodcastItem *items;
    int item_count;
    int per_page;
    int total;
} PodcastData;

//解析JSON
PodcastData *parse_json(const char *json_str)
{
    json_object *root = json_tokener_parse(json_str);
    if (!root) {
        printf("Error parsing JSON\n");
        return NULL;
    }

    PodcastData *data = malloc(sizeof(PodcastData));
    if (!data) {
        json_object_put(root);
        return NULL;
    }
    memset(data, 0, sizeof(PodcastData));

    //获取data对象
    json_object *data_obj = json_object_object_get(root, "data");
    if (!json_object_is_type(data_obj, json_type_object)) {
        printf("Invalid data object\n");
        json_object_put(root);
        free(data);
        return NULL;
    }

    //提取per_page和total
    json_object *per_page = json_object_object_get(data_obj, "per_page");
    json_object *total = json_object_object_get(data_obj, "total");

    if (json_object_is_type(per_page, json_type_int)) {
        data->per_page = json_object_get_int(per_page);
    }

    if (json_object_is_type(total, json_type_int)) {
        data->total = json_object_get_int(total);
    }

    //获取data数组
    json_object *data_array = json_object_object_get(data_obj, "data");
    if (!json_object_is_type(data_array, json_type_array)) {
        printf("Invalid data array\n");
        json_object_put(root);
        free(data);
        return NULL;
    }

    data->item_count = json_object_array_length(data_array);
    data->items = malloc(data->item_count * sizeof(PodcastItem));
    if (!data->items) {
        json_object_put(root);
        free(data);
        return NULL;
    }
    memset(data->items, 0, data->item_count * sizeof(PodcastItem));

    //解析每个数据项
    for (int i = 0; i < data->item_count; i++) {
        json_object *item = json_object_array_get_idx(data_array, i);
        if (!json_object_is_type(item, json_type_object)) {
            continue;
        }

        //id
        json_object *id = json_object_object_get(item, "id");
        if (json_object_is_type(id, json_type_int)) {
            data->items[i].id = json_object_get_int(id);
        }

        //title
        json_object *title = json_object_object_get(item, "title");
        if (json_object_is_type(title, json_type_string)) {
            const char *title_str = json_object_get_string(title);
            data->items[i].title = strdup(title_str);
        }

        //audio_url
        json_object *audio_url = json_object_object_get(item, "audio_url");
        if (json_object_is_type(audio_url, json_type_string)) {
            const char *audio_url_str = json_object_get_string(audio_url);
            data->items[i].audio_url = strdup(audio_url_str);
        }
    }

    json_object_put(root);
    return data;
}

//释放内存函数
void free_podcast_data(PodcastData *data)
{
    if (!data) {
        return;
    }

    for (int i = 0; i < data->item_count; i++) {
        free(data->items[i].title);
        free(data->items[i].audio_url);
    }
    free(data->items);
    free(data);
}

void http_get_podcast_list_cb(char *buf, int len)
{
    printf("%s:", buf);
    os_sem_post(&web_sync_sem);
}

void http_get_podcast_data_cb(char *buf, int len)
{
    //printf("%s:", buf);
    PodcastData *data = parse_json(buf);
    if (!data) {
        printf("memory errror!!!");
        return ;
    }

    //打印分页信息
    printf("分页信息:\n");
    printf("每页条数: %d\n", data->per_page);
    printf("总条数: %d\n", data->total);
    printf("\n");

    //打印提取的数据
    for (int i = 0; i < data->item_count; i++) {
        printf("================================================");
        printf("Item %d:\n", i + 1);
        printf("  ID: %d\n", data->items[i].id);
        printf("  Title: %s\n", data->items[i].title);
        //使用API：net_music_dec_file()进行播放
        printf("  Audio URL: %s\n", data->items[i].audio_url);
        printf("================================================");
        printf("\n\n");
    }

    free_podcast_data(data);
    os_sem_post(&web_sync_sem);
}

//获取播客列表
void http_ai_get_podcast_list_app(void)
{
    http_ai_get_podcast_list(http_get_podcast_list_cb);
    if (OS_TIMEOUT == os_sem_pend(&web_sync_sem, SYNC_TIME_OUT)) {
        printf("wrror: sem waitTime OUT: %d !!!", __LINE__);
    }
}

//获取播客内容数据
void http_ai_get_podcast_data_app(void)
{
    char *class_id = "1";
    //char *keyword = "教育"; //指定内容
    char *keyword = "";    //不指定内容的
    char *page_idx = "1";   //页码索引
    char *perPage = "18";   //每页获取多少个数据项
    http_ai_get_podcast_data(class_id, keyword, page_idx, perPage, http_get_podcast_data_cb);
    if (OS_TIMEOUT == os_sem_pend(&web_sync_sem, SYNC_TIME_OUT)) {
        printf("wrror: sem waitTime OUT: %d !!!", __LINE__);
    }
}

void podcast_play_main(void)
{
    os_sem_create(&web_sync_sem, 0);
}

late_initcall(podcast_play_main);

