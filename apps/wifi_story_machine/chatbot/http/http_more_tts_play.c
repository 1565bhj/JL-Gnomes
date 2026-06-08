#include "system/includes.h"
#include "http_chunck_stream_api.h"
#include "fs/fs.h"
#include "character_coding.h"
#include "http_chunck_stream_api.h"
//#include "app_config.h"

extern void http_tts_play_buf_callback_reg(int (*cb)(char *text, char *buf, int len, int payloadlen, char fin));
extern void http_tts_play_no_wait_set(char no_wait_flag);
extern void qyai_music_buf_play_accept(char enable);
extern int qyai_music_buf_play_push(char *buf, int len, unsigned short index, char type);
extern int http_tts_text_buf_free_size(void);
extern void http_tts_text_buf_clean(void);
extern void music_buf_play_end_index(unsigned short index);

static int tts_buf_count = 0;

#define AC791

int tts_buf_callback_reg(char *text, char *buf, int len, int payloadlen, char fin)
{
    if (buf) {
#ifdef AC791
        music_buf_play_accept(1);
        if (text) {
            txt_buf_play_push(text, strlen(text) + 1, tts_buf_count, 0);
        }
        music_buf_play_push(buf, len, tts_buf_count++, 0);
#else
        qyai_music_buf_play_accept(1);
        if (text) {
            qyai_txt_buf_play_push(text, strlen(text) + 1, tts_buf_count, 0)
        }
        qyai_music_buf_play_push(buf, len, tts_buf_count++, 0);
#endif
        return len;
    } else if (payloadlen) { // 最后空帧， payloadlen为最后一个index
        music_buf_play_end_index(payloadlen);
    }
    return 0;
}

void tts_buf_cnt_clean(void)
{
    tts_buf_count = 0;
}

/***************************************************
多个文字合成（文字多必须分句处理）使用步骤
1、注册http buf回调函数,清空文本缓存,设置不用等待
http_tts_play_buf_callback_reg(tts_buf_callback_reg);
tts_buf_cnt_clean();
http_tts_text_buf_clean();
http_tts_play_no_wait_set(1);

2、分句后每一句调用 http_tts_request(), 最多处理strlen() = 2048个，因此在调用 http_tts_request() 前，先使用 http_tts_text_buf_free_size()看看剩余空间够不够
http_tts_request(tts, strlen(tts) + 1);

一遍检测剩余空间同时发完所有文本

3、播放就回调到对话那边的文本回到

例子：
void app_test()
{
    int tts_buf_callback_reg(char *text, char *buf, int len, int payloadlen, char fin);
    http_tts_play_buf_callback_reg(tts_buf_callback_reg);
    tts_buf_cnt_clean();
    http_tts_text_buf_clean();
    http_tts_play_no_wait_set(1);
    if (http_tts_text_buf_free_size() > 256) {
        http_tts_request("你哈山东理工还是觉得发卡机手打饭卡里", 0);
    }
    if (http_tts_text_buf_free_size() > 256) {
        http_tts_request("空额王鹏飞好久啊恶妇热缩啊苏沪代发", 0);
    }
    if (http_tts_text_buf_free_size() > 256) {
        http_tts_request("啊饿货节是覅u哦光和热飞亚达卡拉是粉红老大房", 0);
    }
    if (http_tts_text_buf_free_size() > 256) {
        http_tts_request("留点时间华东数控水电费手机", 0);
    }
    if (http_tts_text_buf_free_size() > 256) {
        http_tts_request("大佛第三方机构留点时间到啦是撒老大很积极", 0);
    }
}
*****************************************************/



