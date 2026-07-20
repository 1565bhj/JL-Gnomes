#ifndef HTTP_ALL_API_H
#define HTTP_ALL_API_H

#include "string.h"
#include "http_chunck_stream_api.h"

// 获取内容列表相关的函数声明
int http_ai_get_content_list(int page, char *buf, int buf_size, void (*set_callbak)(void *http, char *buf, int len, int payloadlen, char fin));
int http_ai_get_content_list_task(int page, char *buf, int buf_size, void (*set_callbak)(void *http, char *buf, int len, int payloadlen, char fin));//任务方式则在回调函数处理：len <= 0则失败

// 获取内容信息相关的函数声明
int http_ai_get_content_data(int class_id, char *keyword, int page, char *buf, int buf_size, void (*set_callbak)(void *http, char *buf, int len, int payloadlen, char fin));
int http_ai_get_content_data_task(int class_id, char *keyword, int page, char *buf, int buf_size, void (*set_callbak)(void *http, char *buf, int len, int payloadlen, char fin));//任务方式则在回调函数处理：len <= 0则失败

// 获取内容信息相关的函数声明
int http_ai_get_content_details_data(int id, int page, char *buf, int buf_size, void (*set_callbak)(void *http, char *buf, int len, int payloadlen, char fin));
int http_ai_get_content_details_data_task(int id, int page, char *buf, int buf_size, void (*set_callbak)(void *http, char *buf, int len, int payloadlen, char fin));//任务方式则在回调函数处理：len <= 0则失败

// 智能体设置相关函数
int http_ai_intell_set_str(char *str, void (*set_callbak)(char *buf, int role_idx));
int http_ai_intell_set_id(int id_index, void (*set_callbak)(char *buf, int role_idx));

// 智能体ID设定使用任务创建+函数回调
int http_ai_intell_set_id_task(int id_index, void (*set_callbak)(char *buf, int role_idx));
int http_ai_intell_set_success_status(void);

// 智能体设置相关函数（可被外部调用）
int http_ai_intell_set_str(char *str, void (*set_callbak)(char *buf, int role_idx));
int http_ai_intell_set_id(int id_index, void (*set_callbak)(char *buf, int role_idx));
int http_ai_intell_set_success_status(void);

// 获取账号信息
int http_qyai_user_info_get_task_create(void (*callback)(void *user_info));


// TTS 播放控制相关函数
int http_tts_request(char *utf8_str, int utf8_str_size);
int http_tts_reply(char *utf8_str, int utf8_str_size);
int http_tts_reply_type(int type);
int http_tts_play_request_stop(void);
int http_tts_play_stop_force(void);
int http_tts_loop(int enable);
int http_tts_is_playing(void);
int http_tts_play_wait(void);
void http_tts_play_no_wait_set(char enable);
void http_tts_play_buf_static_set(char *buf, int len);
void http_tts_play_buf_callback_reg(int (*cb)(char *text, char *buf, int len, int payloadlen, char fin));
int http_tts_text_buf_free_size(void);
void http_tts_text_buf_clean(void);
int http_tts_play_callback(void *http, char *buf, int len, int payloadlen, char fin);

// 获取图片URL地址列表
// 参数：page - 页码, Page_size - 每页大小, set_callbak - 接收数据的回调函数
// 返回值：成功>0(实际的数据长度)，失败<=0
int http_ai_get_img_url(int page, int Page_size, void (*set_callbak)(void *http, char *buf, int len, int payloadlen, char fin));

// 下载图片
// 参数：req_url - 图片的完整URL地址, set_callbak - 接收数据的回调函数
// 返回值：成功>0(实际的数据长度)，失败返回小于0
int http_ai_download_img(char *req_url, void (*set_callbak)(void *http, char *buf, int len, int payloadlen, char fin));

// 获取天气预报（使用内部缓冲区）
// 参数：utf8 - 城市名称(UTF-8编码), utf8_size - 城市名称长度, buf - 接收数据的缓冲区, buf_size - 缓冲区大小
// 返回值：成功返回>0，失败返回0，未绑定账号返回-1
int qyai_weather_forecast_get(char *utf8, int utf8_size, char *buf, int buf_size);

// 获取天气预报（使用回调函数）
// 参数：utf8 - 城市名称(UTF-8编码), utf8_size - 城市名称长度, cb - 接收数据的回调函数
// 返回值：成功返回>0，失败返回0，未绑定账号返回-1
int http_weather_forecast_get(char *utf8, int utf8_size, int (*cb)(void *http, char *buf, int len, int payloadlen, char fin));

// 获取实时天气（使用回调函数）
// 参数：utf8 - 城市名称(UTF-8编码), utf8_size - 城市名称长度, cb - 接收数据的回调函数
// 返回值：成功返回>0，失败返回0，未绑定账号返回-1
int http_weather_now_get(char *utf8, int utf8_size, int (*cb)(void *http, char *buf, int len, int payloadlen, char fin));

// 用户信息获取相关函数，返回值为当前智能体ID
int http_qyai_user_info_get(void (*callback)(void *http, char *buf, int len, int payloadlen, char fin));


// 获取内容信息相关函数
/*
第1个参数是回调函数：void (*set_callback)(void *http, char *buf, int len, int payloadlen, char fin);
后面参数如："id=124"
例子：http_ai_get_content_v2_details_data_task_parms(3, set_callback, "id=124", "page=10");
*/
int http_ai_get_content_v2_details_data_task_parms(int argc, ...);
int http_ai_get_content_v2_details_data_task(int id, int page, char *buf, int buf_size, void (*set_callback)(void *http, char *buf, int len, int payloadlen, char fin));


#endif

