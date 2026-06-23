/*
 * screen_content_pinyin.c
 * 拼音详情 overlay：字母卡 + 词组 + 声调 + 翻页/重播
 * 由 SC_PINYIN_ENABLE 宏控制编译开关
 */

#include "screen_content_pinyin.h"

#if SC_PINYIN_ENABLE

#include "lvgl.h"
#include "gui_guider.h"
#include "custom.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include <stdint.h>

/* 拼音功能：由 SC_PINYIN_ENABLE 控制，screen_content.c 已在外层做 CONFIG_LVGL_CONTENT_LIST_ENABLE 保护 */

/* ========================================================================== */
/* 拼音兜底数据表                                                               */
/* ========================================================================== */

static const pinyin_detail_t s_pinyin_details[] = {
    /* 单韵母 (id_base=10000) */
    { 10000, "a",  4, {"ā", "á", "ǎ", "à"}, 2, {"bà ba",  "mā ma" }, {"爸爸", "妈妈"} },
    { 10001, "o",  4, {"ō", "ó", "ǒ", "ò"}, 2, {"wǒ men", "bō li" }, {"我们", "玻璃"} },
    { 10002, "e",  4, {"ē", "é", "ě", "è"}, 2, {"é zi",   "kě yǐ" }, {"鹅子", "可以"} },
    { 10003, "i",  4, {"ī", "í", "ǐ", "ì"}, 2, {"yī shēng", "dì di" }, {"医生", "弟弟"} },
    { 10004, "u",  4, {"ū", "ú", "ǔ", "ù"}, 2, {"tù zi",  "dú shū"}, {"兔子", "读书"} },
    { 10005, "ü",  4, {"ǖ", "ǘ", "ǚ", "ǜ"}, 2, {"lǘ zi",  "nǚ ér" }, {"驴子", "女儿"} },

    /* 复韵母 (id_base=10100) */
    { 10100, "ai", 4, {"āi", "ái", "ǎi", "ài"}, 2, {"ài hào", "bái yún"}, {"爱好", "白云"} },
    { 10101, "ei", 4, {"ēi", "éi", "ěi", "èi"}, 2, {"bēi zi", "měi lì" }, {"杯子", "美丽"} },
    { 10102, "ui", 4, {"uī", "uí", "uǐ", "uì"}, 2, {"huì yì", "suí biàn"}, {"会议", "随便"} },
    { 10103, "ao", 4, {"āo", "áo", "ǎo", "ào"}, 2, {"bào zhǐ", "hǎo de" }, {"报纸", "好的"} },
    { 10104, "ou", 4, {"ōu", "óu", "ǒu", "òu"}, 2, {"dòu jiāng", "gǒu xióng"}, {"豆浆", "狗熊"} },
    { 10105, "iu", 4, {"iū", "iú", "iǔ", "iù"}, 2, {"niú nǎi", "liú xíng"}, {"牛奶", "流行"} },
    { 10106, "ie", 4, {"iē", "ié", "iě", "iè"}, 2, {"xié zi", "jiě jie"}, {"鞋子", "姐姐"} },
    { 10107, "üe", 4, {"üē", "üé", "üě", "üè"}, 2, {"xuě bái", "yuè liang"}, {"雪白", "月亮"} },
    { 10108, "er", 4, {"ēr", "ér", "ěr", "èr"}, 2, {"èr shí", "ěr duo" }, {"二十", "耳朵"} },

    /* 前鼻韵母 (id_base=10200) */
    { 10200, "an", 4, {"ān", "án", "ǎn", "àn"}, 2, {"lán tiān", "àn jiàn"}, {"蓝天", "案件"} },
    { 10201, "en", 4, {"ēn", "én", "ěn", "èn"}, 2, {"mén kǒu", "rèn zhēn"}, {"门口", "认真"} },
    { 10202, "in", 4, {"īn", "ín", "ǐn", "ìn"}, 2, {"xīn nián", "yīn yuè"}, {"新年", "音乐"} },
    { 10203, "un", 4, {"ūn", "ún", "ǔn", "ùn"}, 2, {"chūn tiān", "wèn tí"}, {"春天", "问题"} },
    { 10204, "ün", 4, {"ǖn", "ǘn", "ǚn", "ǜn"}, 2, {"jūn zi", "yùn dòng"}, {"君子", "运动"} },

    /* 后鼻韵母 (id_base=10300) */
    { 10300, "ang", 4, {"āng", "áng", "ǎng", "àng"}, 2, {"bāng máng", "shàng wǔ"}, {"帮忙", "上午"} },
    { 10301, "eng", 4, {"ēng", "éng", "ěng", "èng"}, 2, {"dēng pào", "péng you"}, {"灯泡", "朋友"} },
    { 10302, "ing", 4, {"īng", "íng", "ǐng", "ìng"}, 2, {"xīng xing", "tīng huà"}, {"星星", "听话"} },
    { 10303, "ong", 4, {"ōng", "óng", "ǒng", "òng"}, 2, {"dōng tiān", "hóng sè"}, {"冬天", "红色"} },

    /* 声母 (id_base=10400, 无声调发音) */
    { 10400, "b",  0, {0}, 2, {"bà ba", "bō luó"}, {"爸爸", "菠萝"} },
    { 10401, "p",  0, {0}, 2, {"pó po", "pí jiǔ"}, {"婆婆", "啤酒"} },
    { 10402, "m",  0, {0}, 2, {"mā ma", "mǎ lù" }, {"妈妈", "马路"} },
    { 10403, "f",  0, {0}, 2, {"fā shēng", "fēng yè"}, {"发生", "枫叶"} },
    { 10404, "d",  0, {0}, 2, {"dà mǐ", "dú shū"}, {"大米", "读书"} },
    { 10405, "t",  0, {0}, 2, {"tù zi", "tī tuǐ"}, {"兔子", "踢腿"} },
    { 10406, "n",  0, {0}, 2, {"nǎi nai", "nóng fū"}, {"奶奶", "农夫"} },
    { 10407, "l",  0, {0}, 2, {"lǎo shī", "lán tiān"}, {"老师", "蓝天"} },
    { 10408, "g",  0, {0}, 2, {"gē ge", "guó qí"}, {"哥哥", "国旗"} },
    { 10409, "k",  0, {0}, 2, {"kǎ piàn", "kě ài"}, {"卡片", "可爱"} },
    { 10410, "h",  0, {0}, 2, {"hǎo de", "hé liú"}, {"好的", "河流"} },
    { 10411, "j",  0, {0}, 2, {"jī dàn", "jiě jie"}, {"鸡蛋", "姐姐"} },
    { 10412, "q",  0, {0}, 2, {"qí mǎ", "qiū tiān"}, {"骑马", "秋天"} },
    { 10413, "x",  0, {0}, 2, {"xī guā", "xué xí"}, {"西瓜", "学习"} },
    { 10414, "zh", 0, {0}, 2, {"zhī shi", "zhú zi"}, {"知识", "竹子"} },
    { 10415, "ch", 0, {0}, 2, {"chī fàn", "chē zhàn"}, {"吃饭", "车站"} },
    { 10416, "sh", 0, {0}, 2, {"shū běn", "shǒu zhǐ"}, {"书本", "手指"} },
    { 10417, "r",  0, {0}, 2, {"rén men", "rì jì"}, {"人们", "日记"} },
    { 10418, "z",  0, {0}, 2, {"zǎo shang", "zì diǎn"}, {"早上", "字典"} },
    { 10419, "c",  0, {0}, 2, {"cài yuán", "cū liáng"}, {"菜园", "粗粮"} },
    { 10420, "s",  0, {0}, 2, {"sōng shù", "sǎn bù"}, {"松树", "散步"} },
    { 10421, "y",  0, {0}, 2, {"yī fu", "yú gāng"}, {"衣服", "鱼缸"} },
    { 10422, "w",  0, {0}, 2, {"wǎn fàn", "wén jù"}, {"晚饭", "文具"} },

    /* 整体认读 (id_base=10500, 无声调发音) */
    { 10500, "zhi",  0, {0}, 2, {"zhī jiān", "zhì qì"}, {"之间", "志气"} },
    { 10501, "chi",  0, {0}, 2, {"chī fàn", "chí zǐ"},  {"吃饭", "池子"} },
    { 10502, "shi",  0, {0}, 2, {"shī rén", "shì jiè"}, {"诗人", "世界"} },
    { 10503, "ri",   0, {0}, 2, {"rì lì", "rì jì"},     {"日历", "日记"} },
    { 10504, "zi",   0, {0}, 2, {"zì jǐ", "zǐ xì"},     {"自己", "仔细"} },
    { 10505, "ci",   0, {0}, 2, {"cí diǎn", "cì xù"},   {"词典", "次序"} },
    { 10506, "si",   0, {0}, 2, {"sī xiǎng", "sì jì"},  {"思想", "四季"} },
    { 10507, "yi",   0, {0}, 2, {"yī fu", "yī shēng"},  {"衣服", "医生"} },
    { 10508, "wu",   0, {0}, 2, {"wǔ shù", "wū yún"},   {"武术", "乌云"} },
    { 10509, "yu",   0, {0}, 2, {"yǔ yī", "yú gāng"},   {"雨衣", "鱼缸"} },
    { 10510, "ye",   0, {0}, 2, {"yě cài", "yè zi"},    {"野菜", "叶子"} },
    { 10511, "yue",  0, {0}, 2, {"yuè liang", "yuè qǔ"}, {"月亮", "乐曲"} },
    { 10512, "yuan", 0, {0}, 2, {"yuán dì", "yuǎn fāng"}, {"园地", "远方"} },
    { 10513, "yin",  0, {0}, 2, {"yīn yuè", "yín hé"},  {"音乐", "银河"} },
    { 10514, "yun",  0, {0}, 2, {"yún cǎi", "yùn dòng"}, {"云彩", "运动"} },
    { 10515, "ying", 0, {0}, 2, {"yīng xióng", "yíng huǒ"}, {"英雄", "营火"} },
};

/* ========================================================================== */
/* 拼音列表兜底数据                                                             */
/* ========================================================================== */

static const char *const pinyin_dy_titles[] = {
    "a", "o", "e", "i", "u", "ü"
};
static const char *const pinyin_fy_titles[] = {
    "ai", "ei", "ui", "ao", "ou", "iu", "ie", "üe", "er"
};
static const char *const pinyin_qb_titles[] = {
    "an", "en", "in", "un", "ün"
};
static const char *const pinyin_hb_titles[] = {
    "ang", "eng", "ing", "ong"
};
static const char *const pinyin_sm_titles[] = {
    "b", "p", "m", "f", "d", "t", "n", "l", "g", "k", "h",
    "j", "q", "x", "zh", "ch", "sh", "r", "z", "c", "s", "y", "w"
};
static const char *const pinyin_zt_titles[] = {
    "zhi", "chi", "shi", "ri", "zi", "ci", "si",
    "yi", "wu", "yu", "ye", "yue", "yuan", "yin", "yun", "ying"
};

static const pinyin_local_fallback_t s_local_fallbacks[] = {
    { 104, 10000, (int)(sizeof(pinyin_dy_titles) / sizeof(pinyin_dy_titles[0])), pinyin_dy_titles },
    { 105, 10100, (int)(sizeof(pinyin_fy_titles) / sizeof(pinyin_fy_titles[0])), pinyin_fy_titles },
    { 106, 10200, (int)(sizeof(pinyin_qb_titles) / sizeof(pinyin_qb_titles[0])), pinyin_qb_titles },
    { 107, 10300, (int)(sizeof(pinyin_hb_titles) / sizeof(pinyin_hb_titles[0])), pinyin_hb_titles },
    { 108, 10400, (int)(sizeof(pinyin_sm_titles) / sizeof(pinyin_sm_titles[0])), pinyin_sm_titles },
    { 109, 10500, (int)(sizeof(pinyin_zt_titles) / sizeof(pinyin_zt_titles[0])), pinyin_zt_titles },
};

/* ========================================================================== */
/* 查询函数                                                                     */
/* ========================================================================== */

const pinyin_detail_t *pinyin_find_detail_by_id(int id)
{
    int n = (int)(sizeof(s_pinyin_details) / sizeof(s_pinyin_details[0]));
    for (int i = 0; i < n; i++) {
        if (s_pinyin_details[i].id == id) {
            return &s_pinyin_details[i];
        }
    }
    return NULL;
}

const pinyin_local_fallback_t *pinyin_find_local_fallback(int class_id)
{
    int n = (int)(sizeof(s_local_fallbacks) / sizeof(s_local_fallbacks[0]));
    for (int i = 0; i < n; i++) {
        if (s_local_fallbacks[i].class_id == class_id) {
            return &s_local_fallbacks[i];
        }
    }
    return NULL;
}

int pinyin_is_pinyin_item(int item_id)
{
    return (item_id >= 10000 && item_id < 10600) ? 1 : 0;
}

/* ========================================================================== */
/* JSON 构建                                                                  */
/* ========================================================================== */

int pinyin_build_detail_json(const pinyin_detail_t *d, char *out, int out_size)
{
    if (!d || !out || out_size <= 0) {
        return 0;
    }
    int len = 0;
    int n;

    n = snprintf(out + len, out_size - len,
                 "{\"errorCode\":0,\"message\":\"local\","
                 "\"data\":{\"id\":%d,\"kind\":\"pinyin\","
                 "\"title\":\"%s\",\"letter\":\"%s\",\"tone_count\":%d,\"tones\":[",
                 d->id, d->letter, d->letter, d->tone_count);
    if (n < 0 || n >= out_size - len) {
        return 0;
    }
    len += n;

    for (int i = 0; i < d->tone_count; i++) {
        n = snprintf(out + len, out_size - len,
                     "%s\"%s\"", (i == 0) ? "" : ",",
                     d->tones[i] ? d->tones[i] : "");
        if (n < 0 || n >= out_size - len) {
            return 0;
        }
        len += n;
    }

    n = snprintf(out + len, out_size - len, "],\"phrases\":[");
    if (n < 0 || n >= out_size - len) {
        return 0;
    }
    len += n;

    for (int i = 0; i < d->phrase_count; i++) {
        n = snprintf(out + len, out_size - len,
                     "%s{\"pinyin\":\"%s\",\"text\":\"%s\"}",
                     (i == 0) ? "" : ",",
                     d->phrase_pinyin[i] ? d->phrase_pinyin[i] : "",
                     d->phrase_text[i] ? d->phrase_text[i] : "");
        if (n < 0 || n >= out_size - len) {
            return 0;
        }
        len += n;
    }

    n = snprintf(out + len, out_size - len, "]}}");
    if (n < 0 || n >= out_size - len) {
        return 0;
    }
    len += n;
    return len;
}

int pinyin_build_local_fallback_json(const pinyin_local_fallback_t *fb,
                                     char *out, int out_size)
{
    if (!fb || !out || out_size <= 0) {
        return 0;
    }
    int len = 0;
    int n = snprintf(out + len, out_size - len,
                     "{\"errorCode\":0,\"message\":\"local\","
                     "\"data\":{\"current_page\":1,\"data\":[");
    if (n < 0 || n >= out_size - len) {
        return 0;
    }
    len += n;

    for (int i = 0; i < fb->count; i++) {
        const char *title = fb->titles[i] ? fb->titles[i] : "";
        n = snprintf(out + len, out_size - len,
                     "%s{\"id\":%d,\"title\":\"%s\"}",
                     (i == 0) ? "" : ",",
                     fb->id_base + i,
                     title);
        if (n < 0 || n >= out_size - len) {
            return 0;
        }
        len += n;
    }

    n = snprintf(out + len, out_size - len,
                 "],\"total\":%d}}", fb->count);
    if (n < 0 || n >= out_size - len) {
        return 0;
    }
    len += n;
    return len;
}

/* ========================================================================== */
/* LVGL 拼音详情 overlay                                                        */
/* ========================================================================== */

static int s_pinyin_overlay_item_idx = -1;
static lv_obj_t *s_pinyin_overlay = NULL;
static lv_obj_t *s_pinyin_content_box = NULL;
static lv_obj_t *s_pinyin_btn_prev = NULL;
static lv_obj_t *s_pinyin_btn_next = NULL;
static lv_obj_t *s_pinyin_btn_replay = NULL;
static int s_pinyin_content_id = -1;

/* 外部列表数据引用（由 show_for_item_idx 传入） */
static const void *s_ext_items = NULL;
static int s_ext_count = 0;
static int s_ext_stride = 0;

static int pinyin_get_item_id(int idx)
{
    if (!s_ext_items || idx < 0 || idx >= s_ext_count) {
        return -1;
    }
    const char *base = (const char *)s_ext_items;
    const int *id_ptr = (const int *)(base + idx * s_ext_stride);
    return *id_ptr;
}

/* TTS 文本缓冲 */
static char s_pinyin_tts_text[256];

/* 前置声明 */
static void pinyin_overlay_fill_content(const pinyin_detail_t *d);
static void pinyin_overlay_play_tts(const pinyin_detail_t *d);
static void pinyin_overlay_load_idx(int idx);
static void pinyin_overlay_prev_handler(lv_event_t *e);
static void pinyin_overlay_next_handler(lv_event_t *e);
static void pinyin_overlay_replay_handler(lv_event_t *e);
static void pinyin_overlay_back_handler(lv_event_t *e);

/* -------------------------------------------------------------------------- */
/* overlay 关闭                                                                */
/* -------------------------------------------------------------------------- */

void pinyin_overlay_close(void)
{
    page_switch_content_tts_force_clear();

    if (s_pinyin_overlay && lv_obj_is_valid(s_pinyin_overlay)) {
        lv_obj_del(s_pinyin_overlay);
    }
    s_pinyin_overlay     = NULL;
    s_pinyin_content_box = NULL;
    s_pinyin_btn_prev    = NULL;
    s_pinyin_btn_next    = NULL;
    s_pinyin_btn_replay  = NULL;
    s_pinyin_overlay_item_idx = -1;
    s_ext_items = NULL;
    s_ext_count = 0;
}

int pinyin_overlay_is_showing(void)
{
    return (s_pinyin_overlay && lv_obj_is_valid(s_pinyin_overlay)) ? 1 : 0;
}

/* -------------------------------------------------------------------------- */
/* TTS 播放                                                                    */
/* -------------------------------------------------------------------------- */

static void pinyin_overlay_play_tts(const pinyin_detail_t *d)
{
    if (!d) {
        return;
    }

    int pos = 0;
    /* 韵母有声调发音：朗读4个声调 */
    for (int i = 0; i < d->tone_count && pos < (int)sizeof(s_pinyin_tts_text) - 1; i++) {
        if (d->tones[i] && d->tones[i][0]) {
            pos += snprintf(s_pinyin_tts_text + pos, sizeof(s_pinyin_tts_text) - pos,
                            "\"%s\"。", d->tones[i]);
        }
    }
    /* 朗读词组汉字 */
    for (int i = 0; i < d->phrase_count && pos < (int)sizeof(s_pinyin_tts_text) - 1; i++) {
        if (d->phrase_text[i] && d->phrase_text[i][0]) {
            pos += snprintf(s_pinyin_tts_text + pos, sizeof(s_pinyin_tts_text) - pos,
                            "\"%s\"。", d->phrase_text[i]);
        }
    }
    if (pos == 0) {
        return;
    }

    page_switch_content_tts_force_clear();
    page_switch_content_tts_request(s_pinyin_tts_text);
}

/* -------------------------------------------------------------------------- */
/* 按钮工厂                                                                     */
/* -------------------------------------------------------------------------- */

static lv_obj_t *pinyin_make_replay_btn(lv_obj_t *parent, lv_event_cb_t cb)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 40, 40);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x10B981), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_opa(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x059669), LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *lab = lv_label_create(btn);
    lv_label_set_text(lab, "重播");
    lv_obj_set_style_text_color(lab, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(lab, &lv_font_MiSansDemibold_18, LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(lab, 1, LV_PART_MAIN);
    lv_obj_center(lab);

    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
    return btn;
}

/* -------------------------------------------------------------------------- */
/* 按钮状态刷新                                                                 */
/* -------------------------------------------------------------------------- */

static void pinyin_overlay_update_btn_state(void)
{
    if (s_pinyin_btn_prev && lv_obj_is_valid(s_pinyin_btn_prev)) {
        if (s_pinyin_overlay_item_idx <= 0) {
            lv_obj_add_state(s_pinyin_btn_prev, LV_STATE_DISABLED);
            lv_obj_clear_flag(s_pinyin_btn_prev, LV_OBJ_FLAG_CLICKABLE);
        } else {
            lv_obj_clear_state(s_pinyin_btn_prev, LV_STATE_DISABLED);
            lv_obj_add_flag(s_pinyin_btn_prev, LV_OBJ_FLAG_CLICKABLE);
        }
    }
    if (s_pinyin_btn_next && lv_obj_is_valid(s_pinyin_btn_next)) {
        if (s_pinyin_overlay_item_idx >= s_ext_count - 1) {
            lv_obj_add_state(s_pinyin_btn_next, LV_STATE_DISABLED);
            lv_obj_clear_flag(s_pinyin_btn_next, LV_OBJ_FLAG_CLICKABLE);
        } else {
            lv_obj_clear_state(s_pinyin_btn_next, LV_STATE_DISABLED);
            lv_obj_add_flag(s_pinyin_btn_next, LV_OBJ_FLAG_CLICKABLE);
        }
    }
}

/* -------------------------------------------------------------------------- */
/* 填充内容：字母卡 + 词组 + 声调                                               */
/* -------------------------------------------------------------------------- */

static void pinyin_overlay_fill_content(const pinyin_detail_t *d)
{
    if (!s_pinyin_content_box || !lv_obj_is_valid(s_pinyin_content_box) || !d) {
        return;
    }
    lv_obj_clean(s_pinyin_content_box);

    lv_obj_update_layout(s_pinyin_content_box);
    lv_coord_t box_w = lv_obj_get_width(s_pinyin_content_box);
    lv_coord_t box_h = lv_obj_get_height(s_pinyin_content_box);
    if (box_w <= 0) {
        box_w = lv_disp_get_hor_res(NULL) - 20;
    }
    if (box_h <= 0) {
        box_h = lv_disp_get_ver_res(NULL) - 80;
    }

    const lv_coord_t card_w     = (lv_coord_t)(box_w * 28 / 100);
    const lv_coord_t card_h     = (lv_coord_t)(box_h * 55 / 100);
    const lv_coord_t phrase_w   = (lv_coord_t)(box_w * 38 / 100);
    const lv_coord_t phrase_h   = card_h;
    const lv_coord_t group_gap  = (lv_coord_t)(box_w * 4 / 100);
    const lv_coord_t group_w    = card_w + group_gap + phrase_w;
    const lv_coord_t mid_gap    = 8;
    const lv_coord_t tone_h     = (lv_coord_t)(box_h * 22 / 100);
    const lv_coord_t total_h    = card_h + mid_gap + tone_h;

    lv_coord_t group_x = (box_w - group_w) / 2;
    if (group_x < 0) {
        group_x = 0;
    }
    lv_coord_t top_y = (box_h - total_h) / 2;
    if (top_y < 0) {
        top_y = 0;
    }

    /* 字母卡 */
    lv_obj_t *card = lv_obj_create(s_pinyin_content_box);
    lv_obj_set_size(card, card_w, card_h);
    lv_obj_set_pos(card, group_x, top_y);
    lv_obj_set_style_bg_color(card, lv_color_hex(0xE5E7EB), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(card, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_width(card, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(card, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_all(card, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(card, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *card_lab = lv_label_create(card);
    lv_label_set_text(card_lab, d->letter ? d->letter : "");
    lv_obj_set_style_text_color(card_lab, lv_color_hex(0xDC2626), LV_PART_MAIN);
    lv_obj_set_style_text_font(card_lab, &lv_font_MiSansDemibold_24, LV_PART_MAIN);
    lv_obj_center(card_lab);

    /* 词组列 */
    lv_coord_t phrase_x = group_x + card_w + group_gap;

    int valid_n = 0;
    for (int i = 0; i < d->phrase_count; i++) {
        if (d->phrase_pinyin[i] && d->phrase_text[i]) {
            valid_n++;
        }
    }

    if (valid_n > 0) {
        const lv_coord_t row_py_h = 18;
        const lv_coord_t row_tx_h = 22;
        const lv_coord_t row_inner_gap = 2;
        const lv_coord_t row_outer_gap = 8;
        const lv_coord_t per_h = row_py_h + row_inner_gap + row_tx_h;
        lv_coord_t total_text_h = valid_n * per_h + (valid_n - 1) * row_outer_gap;
        lv_coord_t cy = top_y + (phrase_h - total_text_h) / 2;
        if (cy < top_y) {
            cy = top_y;
        }

        const int kMaxChars = 8;
        for (int i = 0; i < d->phrase_count; i++) {
            if (!d->phrase_pinyin[i] || !d->phrase_text[i]) {
                continue;
            }

            char hanzi_buf[kMaxChars][8];
            int hanzi_n = 0;
            const char *hp = d->phrase_text[i];
            while (*hp && hanzi_n < kMaxChars) {
                unsigned char c = (unsigned char) * hp;
                int clen;
                if ((c & 0x80) == 0x00) {
                    clen = 1;
                } else if ((c & 0xE0) == 0xC0) {
                    clen = 2;
                } else if ((c & 0xF0) == 0xE0) {
                    clen = 3;
                } else if ((c & 0xF8) == 0xF0) {
                    clen = 4;
                } else {
                    clen = 1;
                }
                if (clen > 7) {
                    clen = 1;
                }
                memcpy(hanzi_buf[hanzi_n], hp, clen);
                hanzi_buf[hanzi_n][clen] = '\0';
                hanzi_n++;
                hp += clen;
            }

            char pinyin_buf[kMaxChars][16];
            int pinyin_n = 0;
            const char *pp = d->phrase_pinyin[i];
            while (*pp && pinyin_n < kMaxChars) {
                while (*pp == ' ') {
                    pp++;
                }
                if (!*pp) {
                    break;
                }
                int j = 0;
                while (*pp && *pp != ' ' && j < 15) {
                    pinyin_buf[pinyin_n][j++] = *pp++;
                }
                pinyin_buf[pinyin_n][j] = '\0';
                pinyin_n++;
            }

            int pair_n = pinyin_n < hanzi_n ? pinyin_n : hanzi_n;
            if (pair_n <= 0) {
                lv_obj_t *py_lab = lv_label_create(s_pinyin_content_box);
                lv_label_set_text(py_lab, d->phrase_pinyin[i]);
                lv_obj_set_style_text_color(py_lab, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                lv_obj_set_style_text_font(py_lab, &lv_font_MiSansDemibold_18, LV_PART_MAIN);
                lv_obj_set_width(py_lab, phrase_w);
                lv_obj_set_style_text_align(py_lab, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
                lv_obj_set_pos(py_lab, phrase_x, cy);

                lv_obj_t *tx_lab = lv_label_create(s_pinyin_content_box);
                lv_label_set_text(tx_lab, d->phrase_text[i]);
                lv_obj_set_style_text_color(tx_lab, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                lv_obj_set_style_text_font(tx_lab, &lv_font_MiSansDemibold_18, LV_PART_MAIN);
                lv_obj_set_style_text_letter_space(tx_lab, 2, LV_PART_MAIN);
                lv_obj_set_width(tx_lab, phrase_w);
                lv_obj_set_style_text_align(tx_lab, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
                lv_obj_set_pos(tx_lab, phrase_x, cy + row_py_h + row_inner_gap);

                cy += per_h + row_outer_gap;
                continue;
            }

            lv_coord_t cell_w   = phrase_w / pair_n;
            lv_coord_t group_used = cell_w * pair_n;
            lv_coord_t group_off  = (phrase_w - group_used) / 2;
            if (group_off < 0) {
                group_off = 0;
            }

            for (int j = 0; j < pair_n; j++) {
                lv_coord_t col_x = phrase_x + group_off + j * cell_w;

                lv_obj_t *py_lab = lv_label_create(s_pinyin_content_box);
                lv_label_set_text(py_lab, pinyin_buf[j]);
                lv_obj_set_style_text_color(py_lab, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                lv_obj_set_style_text_font(py_lab, &lv_font_MiSansDemibold_18, LV_PART_MAIN);
                lv_obj_set_width(py_lab, cell_w);
                lv_obj_set_style_text_align(py_lab, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
                lv_obj_set_pos(py_lab, col_x, cy);

                lv_obj_t *tx_lab = lv_label_create(s_pinyin_content_box);
                lv_label_set_text(tx_lab, hanzi_buf[j]);
                lv_obj_set_style_text_color(tx_lab, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                lv_obj_set_style_text_font(tx_lab, &lv_font_MiSansDemibold_18, LV_PART_MAIN);
                lv_obj_set_width(tx_lab, cell_w);
                lv_obj_set_style_text_align(tx_lab, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
                lv_obj_set_pos(tx_lab, col_x, cy + row_py_h + row_inner_gap);
            }
            cy += per_h + row_outer_gap;
        }
    }

    /* 声调行（韵母才显示） */
    if (d->tone_count > 0) {
        lv_coord_t tone_y      = top_y + card_h + mid_gap;
        lv_coord_t line_w      = (lv_coord_t)(box_w * 90 / 100);
        lv_coord_t line_x      = (box_w - line_w) / 2;
        const lv_coord_t line_thickness = 2;

        lv_obj_t *line_top = lv_obj_create(s_pinyin_content_box);
        lv_obj_set_size(line_top, line_w, line_thickness);
        lv_obj_set_pos(line_top, line_x, tone_y);
        lv_obj_set_style_bg_color(line_top, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(line_top, LV_OPA_70, LV_PART_MAIN);
        lv_obj_set_style_border_width(line_top, 0, LV_PART_MAIN);
        lv_obj_set_style_outline_width(line_top, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(line_top, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(line_top, 0, LV_PART_MAIN);
        lv_obj_set_scrollbar_mode(line_top, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(line_top, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *line_bot = lv_obj_create(s_pinyin_content_box);
        lv_obj_set_size(line_bot, line_w, line_thickness);
        lv_obj_set_pos(line_bot, line_x, tone_y + tone_h - line_thickness);
        lv_obj_set_style_bg_color(line_bot, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(line_bot, LV_OPA_70, LV_PART_MAIN);
        lv_obj_set_style_border_width(line_bot, 0, LV_PART_MAIN);
        lv_obj_set_style_outline_width(line_bot, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(line_bot, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(line_bot, 0, LV_PART_MAIN);
        lv_obj_set_scrollbar_mode(line_bot, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(line_bot, LV_OBJ_FLAG_SCROLLABLE);

        lv_coord_t step = line_w / d->tone_count;
        for (int i = 0; i < d->tone_count; i++) {
            lv_obj_t *t = lv_label_create(s_pinyin_content_box);
            lv_label_set_text(t, d->tones[i] ? d->tones[i] : "");
            lv_obj_set_style_text_color(t, lv_color_hex(0x10B981), LV_PART_MAIN);
            lv_obj_set_style_text_font(t, &lv_font_MiSansDemibold_18, LV_PART_MAIN);
            lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
            lv_obj_set_width(t, step);
            lv_coord_t tly = tone_y + (tone_h - 28) / 2;
            lv_obj_set_pos(t, line_x + i * step, tly);
        }
    }
}

/* -------------------------------------------------------------------------- */
/* 导航                                                                        */
/* -------------------------------------------------------------------------- */

static void pinyin_overlay_load_idx(int idx)
{
    if (idx < 0 || idx >= s_ext_count) {
        return;
    }
    int item_id = pinyin_get_item_id(idx);
    if (item_id < 0) {
        return;
    }
    const pinyin_detail_t *d = pinyin_find_detail_by_id(item_id);
    if (!d) {
        return;
    }
    s_pinyin_overlay_item_idx = idx;
    s_pinyin_content_id = item_id;
    pinyin_overlay_fill_content(d);
    pinyin_overlay_update_btn_state();
    pinyin_overlay_play_tts(d);
}

static void pinyin_overlay_prev_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    if (s_pinyin_overlay_item_idx > 0) {
        pinyin_overlay_load_idx(s_pinyin_overlay_item_idx - 1);
    }
}

static void pinyin_overlay_next_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    if (s_pinyin_overlay_item_idx < s_ext_count - 1) {
        pinyin_overlay_load_idx(s_pinyin_overlay_item_idx + 1);
    }
}

static void pinyin_overlay_replay_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    if (s_pinyin_overlay_item_idx < 0 || s_pinyin_overlay_item_idx >= s_ext_count) {
        return;
    }
    int item_id = pinyin_get_item_id(s_pinyin_overlay_item_idx);
    if (item_id < 0) {
        return;
    }
    const pinyin_detail_t *d = pinyin_find_detail_by_id(item_id);
    if (!d) {
        return;
    }
    pinyin_overlay_play_tts(d);
}

static void pinyin_overlay_back_handler(lv_event_t *e)
{
    (void)e;
    pinyin_overlay_close();
}

/* -------------------------------------------------------------------------- */
/* 入口：显示拼音详情 overlay                                                    */
/* -------------------------------------------------------------------------- */

void pinyin_overlay_show_for_item_idx(int idx,
                                      const void *content_items,
                                      int content_count,
                                      int item_struct_size)
{
    pinyin_overlay_close();
    if (!content_items || content_count <= 0) {
        return;
    }
    if (idx < 0 || idx >= content_count) {
        return;
    }

    s_ext_items = content_items;
    s_ext_count = content_count;
    s_ext_stride = item_struct_size;

    int item_id = pinyin_get_item_id(idx);
    if (item_id < 0) {
        return;
    }
    const pinyin_detail_t *d = pinyin_find_detail_by_id(item_id);
    if (!d) {
        return;
    }

    s_pinyin_overlay_item_idx = idx;
    s_pinyin_content_id = item_id;

    lv_coord_t scr_w = lv_disp_get_hor_res(NULL);
    lv_coord_t scr_h = lv_disp_get_ver_res(NULL);

    s_pinyin_overlay = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(s_pinyin_overlay, 0, 0);
    lv_obj_set_size(s_pinyin_overlay, scr_w, scr_h);
    lv_obj_set_style_bg_color(s_pinyin_overlay, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_pinyin_overlay, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_opa(s_pinyin_overlay, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(s_pinyin_overlay, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(s_pinyin_overlay, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(s_pinyin_overlay, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(s_pinyin_overlay, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *back_btn = lv_img_create(s_pinyin_overlay);
    lv_img_set_src(back_btn, &_return_alpha_22x22);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_pos(back_btn, 8, 8);
    lv_obj_add_event_cb(back_btn, pinyin_overlay_back_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_clear_flag(back_btn, LV_OBJ_FLAG_SCROLLABLE);

    s_pinyin_content_box = lv_obj_create(s_pinyin_overlay);
    lv_obj_set_size(s_pinyin_content_box, scr_w - 20, scr_h - 68);
    lv_obj_set_pos(s_pinyin_content_box, 10, 36);
    lv_obj_set_style_bg_opa(s_pinyin_content_box, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_pinyin_content_box, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_width(s_pinyin_content_box, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(s_pinyin_content_box, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(s_pinyin_content_box, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(s_pinyin_content_box, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *btn_row = lv_obj_create(s_pinyin_overlay);
    lv_obj_set_size(btn_row, scr_w, 50);
    lv_obj_set_pos(btn_row, 0, scr_h - 52);
    lv_obj_set_style_bg_opa(btn_row, 0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(btn_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_left(btn_row, 20, LV_PART_MAIN);
    lv_obj_set_style_pad_right(btn_row, 20, LV_PART_MAIN);
    lv_obj_set_style_pad_top(btn_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(btn_row, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(btn_row, LV_OBJ_FLAG_SCROLLABLE);

    /* 上一页 - 图片按钮 */
    s_pinyin_btn_prev = lv_img_create(btn_row);
    lv_obj_add_flag(s_pinyin_btn_prev, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(s_pinyin_btn_prev, &_last2_alpha_50x50);
    lv_obj_set_size(s_pinyin_btn_prev, 50, 50);
    lv_obj_set_style_img_opa(s_pinyin_btn_prev, 217, LV_PART_MAIN);
    lv_obj_set_style_img_opa(s_pinyin_btn_prev, 80, LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_add_event_cb(s_pinyin_btn_prev, pinyin_overlay_prev_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_clear_flag(s_pinyin_btn_prev, LV_OBJ_FLAG_SCROLLABLE);

    s_pinyin_btn_replay = pinyin_make_replay_btn(btn_row, pinyin_overlay_replay_handler);

    /* 下一页 - 图片按钮 */
    s_pinyin_btn_next = lv_img_create(btn_row);
    lv_obj_add_flag(s_pinyin_btn_next, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(s_pinyin_btn_next, &_next2_alpha_50x50);
    lv_obj_set_size(s_pinyin_btn_next, 50, 50);
    lv_obj_set_style_img_opa(s_pinyin_btn_next, 217, LV_PART_MAIN);
    lv_obj_set_style_img_opa(s_pinyin_btn_next, 80, LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_add_event_cb(s_pinyin_btn_next, pinyin_overlay_next_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_clear_flag(s_pinyin_btn_next, LV_OBJ_FLAG_SCROLLABLE);

    pinyin_overlay_fill_content(d);
    pinyin_overlay_update_btn_state();
    pinyin_overlay_play_tts(d);

    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(NULL);
    printf("[pinyin] overlay shown idx=%d id=%d letter=%s\n", idx, item_id, d->letter);
}

#endif /* SC_PINYIN_ENABLE */
