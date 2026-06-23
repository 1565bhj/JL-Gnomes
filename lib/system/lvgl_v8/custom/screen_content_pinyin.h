#ifndef __SCREEN_CONTENT_PINYIN_H__
#define __SCREEN_CONTENT_PINYIN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* 拼音功能总开关：1=启用拼音详情 overlay，0=关闭（编译时跳过全部拼音代码） */
#ifndef SC_PINYIN_ENABLE
#define SC_PINYIN_ENABLE  1
#endif

#if SC_PINYIN_ENABLE

#include <stdint.h>

/* -------------------------------------------------------------------------- */
/* 拼音兜底数据                                                                */
/* -------------------------------------------------------------------------- */

#define PINYIN_TONE_MAX    4
#define PINYIN_PHRASE_MAX  2

typedef struct {
    int          id;
    const char  *letter;
    int          tone_count;                          /* 0 或 4 */
    const char  *tones[PINYIN_TONE_MAX];
    int          phrase_count;
    const char  *phrase_pinyin[PINYIN_PHRASE_MAX];
    const char  *phrase_text  [PINYIN_PHRASE_MAX];
} pinyin_detail_t;

typedef struct {
    int                 class_id;
    int                 id_base;
    int                 count;
    const char *const  *titles;
} pinyin_local_fallback_t;

/* -------------------------------------------------------------------------- */
/* 对外接口                                                                    */
/* -------------------------------------------------------------------------- */

/** 根据 id 查找拼音兜底详情，未找到返回 NULL */
const pinyin_detail_t *pinyin_find_detail_by_id(int id);

/** 根据 class_id 查找拼音列表兜底数据，未找到返回 NULL */
const pinyin_local_fallback_t *pinyin_find_local_fallback(int class_id);

/** 把拼音兜底详情拼成 JSON 字符串（与服务器响应格式兼容），返回写入字节数 */
int pinyin_build_detail_json(const pinyin_detail_t *d, char *out, int out_size);

/** 把拼音列表兜底数据拼成 JSON 字符串，返回写入字节数 */
int pinyin_build_local_fallback_json(const pinyin_local_fallback_t *fb, char *out, int out_size);

/** 判断 item_id 是否属于拼音兜底范围（id >= 10000） */
int pinyin_is_pinyin_item(int item_id);

/** 显示拼音详情 overlay（全屏弹出，含字母卡+词组+声调+导航按钮） */
void pinyin_overlay_show_for_item_idx(int idx,
                                      const void *content_items, /* sc_content_item_t* 数组 */
                                      int content_count,
                                      int item_struct_size);

/** 关闭拼音详情 overlay */
void pinyin_overlay_close(void);

/** 拼音 overlay 是否正在显示 */
int pinyin_overlay_is_showing(void);

#else /* !SC_PINYIN_ENABLE */

#define pinyin_find_detail_by_id(id)             NULL
#define pinyin_find_local_fallback(cid)           NULL
#define pinyin_build_detail_json(d, o, s)         0
#define pinyin_build_local_fallback_json(f, o, s) 0
#define pinyin_is_pinyin_item(id)                 0
#define pinyin_overlay_show_for_item_idx(a,b,c,d) ((void)0)
#define pinyin_overlay_close()                    ((void)0)
#define pinyin_overlay_is_showing()               0

#endif /* SC_PINYIN_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* __SCREEN_CONTENT_PINYIN_H__ */
