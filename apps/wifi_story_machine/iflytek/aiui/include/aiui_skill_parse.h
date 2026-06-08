#ifndef __AIUI_SKILL_PARSE_H__
#define __AIUI_SKILL_PARSE_H__
#include "cJson.h"

#define AIUI_CUSTOM_SKILL_WEATHER_INFO_LEN 10
typedef struct {
    int v_temperature;
    int v_airdat;
    char a_pm25[AIUI_CUSTOM_SKILL_WEATHER_INFO_LEN];
    char a_humidity[AIUI_CUSTOM_SKILL_WEATHER_INFO_LEN];
} t_aiui_custom_skill_weather_info;

int aiui_skill_parse(const cJSON *intent, const char *authid);

void  aiui_set_check_play_state(int value);

int aiui_get_check_play_state();
int aiui_check_is_session_end();
void aiui_reset_session_end_flag();
#endif // __AIUI_SKILL_PARSE_H__