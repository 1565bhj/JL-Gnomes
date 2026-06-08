#ifndef __AIUI_MEDIA_PARSE_H__
#define __AIUI_MEDIA_PARSE_H__
#include "cJson.h"
#include "aiui_convert_url.h"

#define AIUI_ITEM_NUM_MAX      (40)


typedef struct item_info_s {
    char *itemID;
    char *name;
    char *playUrl;
    char *allrate;
    int vip; // 0 正常, 1 会员, 2付费
    int duration;
    time_t lvt;
    int need_report;
    int already_report;
    int rate;
    Media_Source_Type_e e_type;
} item_info_t;

typedef struct playlist_s {
    int item_pos;
    int item_num;
    item_info_t item[AIUI_ITEM_NUM_MAX];
} playlist_t;

const char *aiui_parse_tts_url(const char *data);

int aiui_parse_media_info(const cJSON *data);

int aiui_parse_iot_media_info(char *data);

playlist_t *get_media_info();

void aiui_media_free();
#endif // __AIUI_MEDIA_PARSE_H__
