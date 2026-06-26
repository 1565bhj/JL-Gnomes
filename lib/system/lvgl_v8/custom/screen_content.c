#include "lvgl.h"
#include "gui_guider.h"
#include "custom.h"
#include "screen_content_pinyin.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include <stdint.h>

#if CONFIG_LVGL_CONTENT_LIST_ENABLE
#define SC_CONTENT_BUF_SIZE  (32 * 1024)
#define SC_CONTENT_LINE_MAX  48
#define SC_TITLE_MAX         128
#define SC_BUBBLE_SIDE_PAD   12
#define SC_LETTER_SPACE_ROW  3

/* 详情页正文区翻页模式开关：
 *   1 = body_scroll 不允许用户拖动，由 "上一页/下一页" label 按一屏高度翻页；
 *   0 = body_scroll 是可上下滑的滚动容器（旧行为），"重播" label 仍可用。 */
#ifndef SC_DETAIL_PAGED
#define SC_DETAIL_PAGED  0
#endif

/* 详情页自动翻页开关：TTS 播报过程中根据当前句子所在页码自动滚动到对应页。
 *   1 = 开启：每句话开始播放前，若页码与上一句不同则自动翻页；
 *   0 = 关闭：不自动翻页，仅打印页码日志。
 * 用户一旦手动点击「上一页/下一页」或左右滑动翻页，s_auto_page_enabled
 * 即置 false，后续不再自动翻页；按「重播」可重新启用。 */
#ifndef SC_DETAIL_AUTO_PAGE
#define SC_DETAIL_AUTO_PAGE  0
#endif

/* 跨页句子归属规则：当一句话的开头和结尾分属不同页时，算在哪一页。
 *   1 = 算下一页（用句子最后一个字节的偏移量定位）；
 *   0 = 算当前页（用句子第一个字节的偏移量定位）。 */
#ifndef SC_AUTO_PAGE_CROSS_BOUNDARY
#define SC_AUTO_PAGE_CROSS_BOUNDARY  0
#endif

/* 自动翻页滚动动画时长（毫秒），设为 0 则无动画直接跳转 */
#ifndef SC_DETAIL_AUTO_PAGE_SPEED_MS
#define SC_DETAIL_AUTO_PAGE_SPEED_MS  800
#endif

static lv_obj_t *s_body_scroll;
static int s_req_class_id = -1;
static lv_style_t s_list_bubble_style;
static bool s_list_bubble_style_inited;
typedef struct {
    int  id;
    char title[SC_TITLE_MAX];
} sc_content_item_t;
static sc_content_item_t s_content_items[SC_CONTENT_LINE_MAX];
static int s_content_count;
static lv_obj_t *s_detail_overlay;
static char s_detail_heading[SC_TITLE_MAX];
/* 详情页里 body_scroll / body_lab 的引用：供顶部三个 label 翻页/重播回调使用。
 * 每次 detail_show 重新赋值，detail_close 置空。 */
static lv_obj_t *s_detail_body_scroll;
static lv_obj_t *s_detail_body_lab;
#if SC_DETAIL_PAGED
/* 右下角 "当前页/总页数" 提示 label。挂在 s_detail_overlay 下并打了 FLOATING flag
 * （不参与 overlay 的 flex 列布局），位置用 lv_obj_align 锁定到 overlay 内容区
 * 的右下角。overlay 一删，这个 label 跟着释放，指针在 detail_close 置空。 */
static lv_obj_t *s_detail_page_indicator;
/* 详情页正文字节总数，detail_show 时赋值、detail_close 清零。
 * TTS 任务据此与 s_detail_page_h / s_detail_content_h 一起估算每句话落在第几页。 */
static int s_sc_body_text_total;
/* detail_show 里 layout 完成后的 body_scroll 可见高度 / body_lab 内容高度。
 * TTS 任务与 s_sc_body_text_total 一起估算每字节对应的页号，不在 LVGL 线程里
 * 调用任何 lv_obj_get_height，避免跨线程问题。overlay 关闭时一并清零。 */
static lv_coord_t s_detail_page_h;
static lv_coord_t s_detail_content_h;
/* 自动翻页状态：
 *   s_auto_page_enabled  — 是否允许 TTS 播报过程中自动翻页；
 *                          detail_show 时启用，用户手动翻页后置 false，
 *                          重播时重新启用。
 *   s_auto_last_page     — 上一句句子所在的页号（0 表示未初始化）；
 *                          当新句子页号与该值不同时触发翻页。 */
#if SC_DETAIL_AUTO_PAGE
static bool s_auto_page_enabled;
static int  s_auto_last_page;
#endif
/* 前向声明：auto-page 回调在文件前部定义，需要调用后面才出现的函数 */
static void sc_detail_update_page_indicator(lv_coord_t cur_y);
#endif
static void screen_content_scroll_reset_layout(void)
{
    if (!s_body_scroll || !lv_obj_is_valid(s_body_scroll)) {
        return;
    }
    lv_obj_set_flex_flow(s_body_scroll, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(s_body_scroll, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(s_body_scroll, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_top(s_body_scroll, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(s_body_scroll, 6, LV_PART_MAIN);
}

static void screen_content_show_plain_message(const char *text)
{
    if (!s_body_scroll || !lv_obj_is_valid(s_body_scroll)) {
        return;
    }
    lv_obj_clean(s_body_scroll);
    screen_content_scroll_reset_layout();
    lv_obj_t *lbl = lv_label_create(s_body_scroll);
    lv_obj_set_width(lbl, LV_PCT(100));
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_label_set_text(lbl, text ? text : "");
    printf("[屏显-内容区] %s\n", (text && text[0]) ? text : "（空）");
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl, &lv_font_MiSansDemibold_18, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(lbl, 4, LV_PART_MAIN);
}

/* 控制最终内容页（详情 overlay 显示时）顶部三个 label
 * (上一页 / 重播 / 下一页) 的可见性。
 *
 * 这三个 label 是 setup_scr_screen_content 里创建的，挂在 screen_content 下，
 * 默认隐藏；只有当 s_detail_overlay 打开时才把它们 unhide 并 foreground
 * 到 overlay 之上。overlay 关掉时再隐藏。
 */
static void screen_content_return_cb(lv_event_t *event);
static void screen_content_perform_return_action(void);
static void screen_content_return_btn_raise_above_loading(void);

static void screen_content_topbar_set_visible(bool show)
{
    lv_obj_t *labels[] = {
        guider_ui.screen_content_label_last_page,
        guider_ui.screen_content_label_repeat,
        guider_ui.screen_content_label_next_page,
    };
    for (size_t i = 0; i < sizeof(labels) / sizeof(labels[0]); ++i) {
        if (!labels[i] || !lv_obj_is_valid(labels[i])) {
            continue;
        }
#if !SC_DETAIL_PAGED
        /* 滚动模式：详情打开时只显示「重播」，上一页/下一页保持隐藏 */
        if (show && i != 1) {
            lv_obj_add_flag(labels[i], LV_OBJ_FLAG_HIDDEN);
            continue;
        }
#endif
        if (show) {
            lv_obj_clear_flag(labels[i], LV_OBJ_FLAG_HIDDEN);
            /* overlay 是后建的兄弟节点，z-order 在 label 之上；
             * 这里把 label 提到最前面，保证显示在 overlay 之上。 */
            lv_obj_move_foreground(labels[i]);
        } else {
            lv_obj_add_flag(labels[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_background(labels[i]);
        }
    }
    if (!show) {
        screen_content_return_btn_raise_above_loading();
    }
}

/* 重播延时回调序号。
 * detail_close 在下面 #if APPLAYER_ENABLE 大块之前定义，但内部需要推进这个序号，
 * 所以把变量也放在这里，让 detail_close 能直接引用，避免再做前向声明。
 * 用途：用户每按一次"重播"++，关闭 overlay / 返回列表时也++；
 * 250ms 后到点执行的 worker 凭序号校验，不一致直接丢弃，
 * 避免覆盖更新一次的请求或摸到已被销毁的 body_lab。 */
#if APPLAYER_ENABLE
static unsigned s_detail_repeat_req_serial;
#endif

static void screen_content_detail_close(void)
{
#if APPLAYER_ENABLE
    /* overlay 关闭：让所有在飞的"重播延时发送"回调失效 */
    s_detail_repeat_req_serial++;
    if (s_detail_overlay && lv_obj_is_valid(s_detail_overlay)) {
        lv_obj_del(s_detail_overlay);
    }
    s_detail_overlay = NULL;
    /* overlay 一删，里头的 body_scroll/body_lab 都被释放了，引用要清 */
    s_detail_body_scroll = NULL;
    s_detail_body_lab = NULL;
#if SC_DETAIL_PAGED
    /* overlay 一删，子节点 page indicator 一起释放，指针置空就行 */
    s_detail_page_indicator = NULL;
    s_sc_body_text_total = 0;
    s_detail_page_h = 0;
    s_detail_content_h = 0;
#if SC_DETAIL_AUTO_PAGE
    /* overlay 关闭后，在飞的自动翻页异步回调跑到时会检查 overlay 指针
     * 是否有效并跳过，这里仅重置状态变量避免下次残留。 */
    s_auto_page_enabled = false;
    s_auto_last_page = 0;
#endif
#endif
    screen_content_topbar_set_visible(false);
#endif
}

#if APPLAYER_ENABLE

#include "system/includes.h"
#include "system/timer.h"
#include "cJSON_common\cJSON.h"
/* ============================================================================
 * TTS 内容播报（使用 http_all_api.h 流水线 TTS 接口） */
#include "net/http_chunck_stream/http_all_api.h"

extern void music_buf_play_set_stop(void);
extern void websockets_free_lbuf_buf(void);
extern void music_buf_play_free_lbuf(void);
extern void music_buf_play_accept(int accept);
extern void music_buf_play_stop_waite(void);
extern int music_buf_play_stop_status(void);
extern void music_buf_play_stop_all(void);

static OS_SEM s_tts_sem;
static char *s_tts_pending_heading;
static char *s_tts_pending_body;
static volatile int s_tts_pending_valid;
static volatile int s_tts_cmd_stop;
static volatile int s_tts_cmd_force_clear;
static volatile int s_tts_abort;
static int s_tts_task_created;

static int page_switch_content_tts_text_should_skip(const char *s)
{
    static const char *const skip_starts[] = {
        "加载中", "获取详情失败", "数据解析失败", "暂无讲解",
        "内存不足", "无效分类", "需启用", "暂无内容",
        "数据格式异常", "获取内容失败",
    };
    if (!s || !s[0]) {
        return 1;
    }
    for (unsigned i = 0; i < sizeof(skip_starts) / sizeof(skip_starts[0]); i++) {
        if (strncmp(s, skip_starts[i], strlen(skip_starts[i])) == 0) {
            return 1;
        }
    }
    return 0;
}

void page_switch_content_tts_stop(void)
{
    if (!s_tts_task_created) {
        return;
    }
    s_tts_abort = 1;
    s_tts_cmd_stop = 1;
    os_sem_post(&s_tts_sem);
}

void page_switch_content_tts_force_clear(void)
{
    if (!s_tts_task_created) {
        return;
    }
    s_tts_abort = 1;
    s_tts_cmd_stop = 1;
    s_tts_cmd_force_clear = 1;
    os_sem_post(&s_tts_sem);
}

#ifndef PS_CONTENT_TTS_MAX_SEGMENT_BYTES
#define PS_CONTENT_TTS_MAX_SEGMENT_BYTES  220
#endif

static int ps_tts_utf8_char_len(const unsigned char *p)
{
    if (!p || !p[0]) {
        return 0;
    }
    if (p[0] < 0x80u) {
        return 1;
    }
    if (p[0] < 0xE0u) {
        return 2;
    }
    if (p[0] < 0xF0u) {
        return 3;
    }
    return 4;
}

/* 检测中文逗号（，= EF BC 8C）和顿号（、= E3 80 81）。
 * 用于长句跨页时，在逗号处切分以触发自动翻页。 */
static int ps_tts_is_comma(const unsigned char *p, int *out_char_len)
{
    *out_char_len = 0;
    if (!p[0]) {
        return 0;
    }
    /* ， = EF BC 8C */
    if (p[0] == 0xEFu && p[1] == 0xBCu && p[2] == 0x8Cu) {
        *out_char_len = 3;
        return 1;
    }
    /* 、 = E3 80 81 */
    if (p[0] == 0xE3u && p[1] == 0x80u && p[2] == 0x81u) {
        *out_char_len = 3;
        return 1;
    }
    /* ASCII 逗号 ',' */
    if (p[0] == ',') {
        *out_char_len = 1;
        return 1;
    }
    return 0;
}

static int ps_tts_is_sentence_end(const unsigned char *p, int *out_char_len)
{
    *out_char_len = 0;
    if (!p[0]) {
        return 0;
    }
    if (p[0] == '\n') {
        *out_char_len = 1;
        return 1;
    }
    if (p[0] == 0xE3u && p[1] == 0x80u && p[2] == 0x82u) {
        *out_char_len = 3;
        return 1;
    }
    if (p[0] == 0xEFu && p[1] == 0xBCu &&
        (p[2] == 0x81u || p[2] == 0x9Fu || p[2] == 0x9Bu)) {
        *out_char_len = 3;
        return 1;
    }
    return 0;
}

/* ============================================================================
 * TTS 播放任务 - 独立 OS 任务，不阻塞 sys_timer / LVGL 线程
 * ============================================================================ */
static void do_tts_stop_internal(void)
{
    music_buf_play_accept(0);
    music_buf_play_set_stop();
    websockets_free_lbuf_buf();
    music_buf_play_free_lbuf();
    http_tts_play_stop_force();
    http_tts_text_buf_clean();
    aisp_clear(1);
    music_buf_play_stop_waite();
    int to = 100;
    while (!music_buf_play_stop_status() && --to) {
        os_time_dly(1);
    }
}

static void page_switch_content_tts_send_one_segment(const unsigned char *start, int byte_len,
        int page_num)
{
    char *seg;
    int retry;

    if (byte_len <= 0) {
        return;
    }
    seg = (char *)malloc((size_t)byte_len + 1u);
    if (!seg) {
        return;
    }
    memcpy(seg, start, (size_t)byte_len);
    seg[byte_len] = '\0';
    /* 只有回车换行的段不合成（如单独的 "\n" 或 "\r\n"） */
    {
        int only_nl = 1;
        for (int k = 0; k < byte_len; k++) {
            if (seg[k] != '\n' && seg[k] != '\r') {
                only_nl = 0;
                break;
            }
        }
        if (only_nl) {
            printf("[TTS] 跳过仅含换行的段(%d字节)\n", byte_len);
            free(seg);
            return;
        }
    }
    if (page_num > 0) {
        printf("[TTS] 合成音频片段(第%d页, %d字节): %s\n", page_num, byte_len, seg);
    } else {
        printf("[TTS] 合成音频片段(%d字节): %s\n", byte_len, seg);
    }
    retry = 200;
    while (http_tts_text_buf_free_size() < (byte_len + 64) && --retry) {
        if (s_tts_abort) {
            free(seg);
            return;
        }
        os_time_dly(1);
    }
    retry = 10;
    while (http_tts_request(seg, byte_len + 1) < 0 && --retry) {
        if (s_tts_abort) {
            free(seg);
            return;
        }
        os_time_dly(1);
    }
    free(seg);
    if (!s_tts_abort) {
        http_tts_play_wait();
        os_time_dly(10);
    }
}

#if SC_DETAIL_PAGED
/* 根据句子在正文中的字节偏移，估算它落在第几页。
 *
 * 归属规则由 SC_AUTO_PAGE_CROSS_BOUNDARY 控制：
 *   1 = 半句跨页算下一页（用句子最后一个字节定位）；
 *   0 = 半句跨页算当前页（用句子第一个字节定位）。
 * 使用像素高度直接估算：将字节偏移线性映射到像素位置，再除以页高。
 * 比"每页平均字节数"更精确，因为直接用实际布局尺寸计算。
 * total_bytes <= 0 时直接返回 0，调用方据此判断是否需要打印。 */
static int sc_calc_sentence_page(int byte_offset, int seg_byte_len,
                                 int total_bytes)
{
    int total_pages;
    int pos;
    int estimated_y;
    int page;

    if (total_bytes <= 0 || seg_byte_len <= 0) {
        return 0;
    }
    if (s_detail_page_h <= 0 || s_detail_content_h <= s_detail_page_h) {
        return 1;
    }
    total_pages = (s_detail_content_h + s_detail_page_h - 1) / s_detail_page_h;
    if (total_pages <= 1) {
        return 1;
    }
#if SC_AUTO_PAGE_CROSS_BOUNDARY
    /* 用句子最后一个字节定位 -> 跨页算下一页 */
    pos = byte_offset + seg_byte_len - 1;
    if (pos >= total_bytes) {
        pos = total_bytes - 1;
    }
#else
    /* 用句子第一个字节定位 -> 跨页算当前页 */
    pos = byte_offset;
    if (pos >= total_bytes) {
        pos = total_bytes - 1;
    }
#endif
    /* 线性插值：字节偏移 -> 估算像素高度 -> 页号 */
    estimated_y = (int)((int64_t)pos * s_detail_content_h / total_bytes);
    page = estimated_y / s_detail_page_h + 1;
    if (page < 1) {
        page = 1;
    }
    if (page > total_pages) {
        page = total_pages;
    }
    return page;
}
#if SC_DETAIL_AUTO_PAGE
/* 自动翻页异步回调：在 LVGL 线程里执行，由 TTS 任务通过 lv_async_call 投递。
 * user_data 里存的是 (intptr_t)page_num，即目标页号。
 *
 * 回调跑到时 overlay 可能已被用户关闭或切走，所以先做有效性检查。
 * scroll_to_y 直接按 page_h 整数倍跳转，与手动"下一页"行为一致。 */
static void sc_detail_auto_page_async_cb(void *user_data)
{
    int page_num = (int)(intptr_t)user_data;
    lv_coord_t page_h, new_y, max_y;

    if (!s_detail_overlay || !lv_obj_is_valid(s_detail_overlay)) {
        return;
    }
    if (!s_detail_body_scroll || !lv_obj_is_valid(s_detail_body_scroll)) {
        return;
    }
    page_h = lv_obj_get_height(s_detail_body_scroll);
    if (page_h <= 0 || page_num <= 0) {
        return;
    }
    new_y = (lv_coord_t)(page_num - 1) * page_h;
    max_y = lv_obj_get_scroll_bottom(s_detail_body_scroll)
            + lv_obj_get_scroll_y(s_detail_body_scroll);
    if (new_y > max_y) {
        new_y = max_y;
    }
    lv_obj_scroll_to_y(s_detail_body_scroll, new_y, LV_ANIM_OFF);
    sc_detail_update_page_indicator(new_y);
    printf("[自动翻页] 翻到第 %d 页\n", page_num);
}
#endif /* SC_DETAIL_AUTO_PAGE */
#endif /* SC_DETAIL_PAGED */

static void page_switch_content_tts_send_segmented(const char *utf8_text,
        int total_bytes)
{
    const unsigned char *src, *seg_start;
    int clen;

    if (!utf8_text || !utf8_text[0]) {
        return;
    }
    printf("[TTS] 分段开始，总文本(%d字节): %s\n", (int)strlen(utf8_text), utf8_text);
    src = (const unsigned char *)utf8_text;
    seg_start = src;
    int tts_buf_callback_reg(char *text, char *buf, int len, int payloadlen, char fin);
    http_tts_play_buf_callback_reg(tts_buf_callback_reg);
    tts_buf_cnt_clean();
    http_tts_text_buf_clean();
    http_tts_play_no_wait_set(1);

    while (*src) {
        if (s_tts_abort) {
            printf("[TTS] 分段被中断(abort)\n");
            return;
        }
        /* 句末标点（。！？\n）：立即切分 TTS 段 */
        if (ps_tts_is_sentence_end(src, &clen)) {
            int seg_len = (int)(src - seg_start) + clen;
            int pg = 0;
#if SC_DETAIL_PAGED
            if (total_bytes > 0) {
                pg = sc_calc_sentence_page((int)(seg_start - (const unsigned char *)utf8_text),
                                           seg_len, total_bytes);
            }
#if SC_DETAIL_AUTO_PAGE
            if (pg > 0 && s_auto_page_enabled && pg != s_auto_last_page) {
                s_auto_last_page = pg;
                lv_async_call(sc_detail_auto_page_async_cb, (void *)(intptr_t)pg);
            }
#endif
#endif
            page_switch_content_tts_send_one_segment(seg_start, seg_len, pg);
            src += clen;
            seg_start = src;
            if (!s_tts_abort) {
                os_time_dly(5);
            }
            continue;
        }
        /* 逗号分句：每积累两个逗号才切一刀。
         * 遇到逗号时，向前检查 seg_start 到 src 之间是否已有逗号，
         * 有则切分（当前逗号是第二个），无则继续累积（当前逗号是第一个）。 */
        if (ps_tts_is_comma(src, &clen)) {
            int should_cut = 0;
            const unsigned char *p = seg_start;
            while (p < src) {
                int pc;
                if (ps_tts_is_comma(p, &pc)) {
                    should_cut = 1; /* seg_start 到 src 之间已有逗号，当前是第二个 */
                    break;
                }
                int step = ps_tts_utf8_char_len(p);
                if (step <= 0) {
                    break;
                }
                p += step;
            }
            if (should_cut) {
                int seg_len = (int)(src - seg_start) + clen;
                int pg = 0;
#if SC_DETAIL_PAGED
                if (total_bytes > 0) {
                    pg = sc_calc_sentence_page((int)(seg_start - (const unsigned char *)utf8_text),
                                               seg_len, total_bytes);
                }
#if SC_DETAIL_AUTO_PAGE
                if (pg > 0 && s_auto_page_enabled && pg != s_auto_last_page) {
                    s_auto_last_page = pg;
                    lv_async_call(sc_detail_auto_page_async_cb, (void *)(intptr_t)pg);
                }
#endif
#endif
                page_switch_content_tts_send_one_segment(seg_start, seg_len, pg);
                src += clen;
                seg_start = src;
                if (!s_tts_abort) {
                    os_time_dly(5);
                }
                continue;
            }
        }
        if ((int)(src - seg_start) >= PS_CONTENT_TTS_MAX_SEGMENT_BYTES) {
            int seg_len = (int)(src - seg_start);
            int pg = 0;
#if SC_DETAIL_PAGED
            if (total_bytes > 0) {
                pg = sc_calc_sentence_page((int)(seg_start - (const unsigned char *)utf8_text),
                                           seg_len, total_bytes);
            }
#if SC_DETAIL_AUTO_PAGE
            if (pg > 0 && s_auto_page_enabled && pg != s_auto_last_page) {
                s_auto_last_page = pg;
                lv_async_call(sc_detail_auto_page_async_cb, (void *)(intptr_t)pg);
            }
#endif
#endif
            page_switch_content_tts_send_one_segment(seg_start, seg_len, pg);
            seg_start = src;
            if (!s_tts_abort) {
                os_time_dly(5);
            }
            continue;
        }
        {
            int step = ps_tts_utf8_char_len(src);
            if (step <= 0) {
                break;
            }
            src += step;
        }
    }
    if (src > seg_start) {
        int seg_len = (int)(src - seg_start);
        int pg = 0;
#if SC_DETAIL_PAGED
        if (total_bytes > 0) {
            pg = sc_calc_sentence_page((int)(seg_start - (const unsigned char *)utf8_text),
                                       seg_len, total_bytes);
        }
#if SC_DETAIL_AUTO_PAGE
        if (pg > 0 && s_auto_page_enabled && pg != s_auto_last_page) {
            s_auto_last_page = pg;
            lv_async_call(sc_detail_auto_page_async_cb, (void *)(intptr_t)pg);
        }
#endif
#endif
        page_switch_content_tts_send_one_segment(seg_start, seg_len, pg);
    }
}

static void page_switch_content_tts_task(void *priv)
{
    while (1) {
        os_sem_pend(&s_tts_sem, 0);

        // 处理停止命令
        if (s_tts_cmd_stop) {
            s_tts_cmd_stop = 0;
            do_tts_stop_internal();
        }

        // 处理强制清除命令
        if (s_tts_cmd_force_clear) {
            s_tts_cmd_force_clear = 0;
            music_buf_play_accept(0);
            music_buf_play_set_stop();
            websockets_free_lbuf_buf();
            music_buf_play_free_lbuf();
            http_tts_play_stop_force();
            http_tts_text_buf_clean();
            aisp_clear(1);
            music_buf_play_stop_waite();
            int to = 100;
            while (!music_buf_play_stop_status() && --to) {
                os_time_dly(10);
            }
        }

        // 处理 TTS 播放请求
        if (s_tts_pending_valid) {
            char *heading = s_tts_pending_heading;
            char *body = s_tts_pending_body;
            s_tts_pending_valid = 0;

            int have_h = (heading && heading[0]);
            int have_b = (body && body[0]);
            int skip_b = have_b && page_switch_content_tts_text_should_skip(body);

            if (!have_h && (!have_b || skip_b)) {
                if (heading) {
                    free(heading);
                }
                if (body) {
                    free(body);
                }
                continue;
            }

            s_tts_abort = 0;
            do_tts_stop_internal();
            http_tts_text_buf_clean();
            http_tts_play_no_wait_set(1);
            music_buf_play_accept(1);

            if (have_h && !s_tts_abort) {
                page_switch_content_tts_send_segmented(heading, 0);
            }
            if (have_b && !skip_b && !s_tts_abort) {
#if SC_DETAIL_PAGED
                page_switch_content_tts_send_segmented(body, s_sc_body_text_total);
#else
                page_switch_content_tts_send_segmented(body, 0);
#endif
            }
            http_tts_play_no_wait_set(0);

            if (heading) {
                free(heading);
            }
            if (body) {
                free(body);
            }
        }
    }
}

static void tts_submit_request(char *heading_dup, char *body_dup)
{
    if (!s_tts_task_created) {
        os_sem_create(&s_tts_sem, 0);
        os_task_create(page_switch_content_tts_task, NULL, 8, 1024, 0, "tts_play");
        s_tts_task_created = 1;
    }

    if (s_tts_pending_valid) {
        if (s_tts_pending_heading) {
            free(s_tts_pending_heading);
        }
        if (s_tts_pending_body) {
            free(s_tts_pending_body);
        }
    }

    s_tts_pending_heading = heading_dup;
    s_tts_pending_body = body_dup;
    s_tts_pending_valid = 1;

    os_sem_post(&s_tts_sem);
}

void page_switch_content_tts_request(const char *utf8_text)
{
    if (page_switch_content_tts_text_should_skip(utf8_text)) {
        return;
    }
    tts_submit_request(NULL, strdup(utf8_text));
}

void page_switch_content_tts_request_heading_and_body(const char *heading, const char *body)
{
    int have_h = (heading && heading[0]);
    int have_b = (body && body[0]);
    int skip_b = have_b && page_switch_content_tts_text_should_skip(body);

    if (!have_h && (!have_b || skip_b)) {
        return;
    }
    tts_submit_request(
        have_h ? strdup(heading) : NULL,
        (have_b && !skip_b) ? strdup(body) : NULL
    );
}
/* ============================================================================ */
/* ============================================================================ */


/* 列表请求：在 sys_timer 里只做网络+解析；结果通过 lv_async_call 回到 LVGL 线程刷界面 */
enum {
    SC_LIST_R_OK = 0,
    SC_LIST_R_MEM,
    SC_LIST_R_NET,
    SC_LIST_R_PARSE,
    SC_LIST_R_EMPTY,
    SC_LIST_R_FMT,
};
static int s_list_async_cid;
static int s_list_async_result;
/* 列表请求代数：防止上一次网络/异步回调在第二次进入 content 页后仍刷新界面 */
static unsigned s_list_expect_serial;
static unsigned s_list_async_serial;

static void screen_content_detail_show(const char *heading, const char *body_text);
static void screen_content_add_bubble_list(void);
#if SC_DETAIL_PAGED
static void screen_content_detail_scroll_by_page(int dir);
#endif

typedef struct {
    char heading[SC_TITLE_MAX];
    char *body;
    unsigned apply_serial;
} sc_detail_payload_t;

static unsigned s_detail_expect_serial;
static unsigned s_detail_req_serial;
static int s_detail_req_content_id;

static lv_obj_t *s_detail_fetch_loading_overlay;
static lv_obj_t *s_detail_fetch_loading_return_hit;

/** 详情「加载中」蒙层显示时，把内容页左上角返回键提到蒙层之上以便取消请求 */
static void screen_content_return_btn_raise_above_loading(void)
{
    lv_obj_t *btn = guider_ui.screen_content_img_return;
    lv_obj_t *parent = guider_ui.screen_content;
    uint32_t top_index;

    if (!btn || !lv_obj_is_valid(btn) || !parent || !lv_obj_is_valid(parent)) {
        return;
    }
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_move_foreground(btn);
    top_index = lv_obj_get_child_cnt(parent);
    if (top_index > 0) {
        lv_obj_move_to_index(btn, top_index - 1);
    }
    lv_obj_remove_event_cb(btn, screen_content_return_cb);
    lv_obj_add_event_cb(btn, screen_content_return_cb, LV_EVENT_CLICKED, NULL);
}

static void screen_content_detail_fetch_loading_hide(void)
{
    if (s_detail_fetch_loading_overlay && lv_obj_is_valid(s_detail_fetch_loading_overlay)) {
        lv_obj_del(s_detail_fetch_loading_overlay);
        s_detail_fetch_loading_overlay = NULL;
        s_detail_fetch_loading_return_hit = NULL;
    }
}

static void screen_content_detail_fetch_cancel(void)
{
    screen_content_detail_fetch_loading_hide();
    s_detail_req_content_id = 0;
    s_detail_expect_serial++;
}

static void screen_content_detail_fetch_loading_return_hit_cb(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
        return;
    }
    screen_content_perform_return_action();
}

/* 蒙层全屏可点会挡住 screen_content 下的返回 img；在蒙层上铺透明热区对齐返回键 */
static void screen_content_detail_fetch_loading_return_hit_place(void)
{
    lv_obj_t *btn = guider_ui.screen_content_img_return;
    lv_obj_t *overlay = s_detail_fetch_loading_overlay;

    if (!overlay || !lv_obj_is_valid(overlay) || !btn || !lv_obj_is_valid(btn)) {
        return;
    }
    if (!s_detail_fetch_loading_return_hit || !lv_obj_is_valid(s_detail_fetch_loading_return_hit)) {
        s_detail_fetch_loading_return_hit = lv_obj_create(overlay);
        lv_obj_remove_style_all(s_detail_fetch_loading_return_hit);
        lv_obj_add_flag(s_detail_fetch_loading_return_hit, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(s_detail_fetch_loading_return_hit, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_opa(s_detail_fetch_loading_return_hit, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_opa(s_detail_fetch_loading_return_hit, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_add_event_cb(s_detail_fetch_loading_return_hit,
                            screen_content_detail_fetch_loading_return_hit_cb,
                            LV_EVENT_CLICKED, NULL);
    }
    lv_obj_set_pos(s_detail_fetch_loading_return_hit, lv_obj_get_x(btn), lv_obj_get_y(btn));
    lv_obj_set_size(s_detail_fetch_loading_return_hit,
                    lv_obj_get_width(btn), lv_obj_get_height(btn));
    lv_obj_move_foreground(s_detail_fetch_loading_return_hit);
}

/* 详情接口请求期间盖在内容页上的「加载中」遮罩（列表仍在上一次绘制状态，避免误以为卡住） */
static void screen_content_detail_fetch_loading_show(void)
{
    lv_obj_t *parent = guider_ui.screen_content;
    lv_obj_t *lbl;

    if (!parent || !lv_obj_is_valid(parent)) {
        return;
    }
    /* 详情顶栏 label 若曾 foreground，第二次蒙层会盖住返回键 */
    screen_content_topbar_set_visible(false);
    if (!s_detail_fetch_loading_overlay || !lv_obj_is_valid(s_detail_fetch_loading_overlay)) {
        lv_coord_t pw = lv_obj_get_width(parent);
        lv_coord_t ph = lv_obj_get_height(parent);

        s_detail_fetch_loading_overlay = lv_obj_create(parent);
        lv_obj_remove_style_all(s_detail_fetch_loading_overlay);
        lv_obj_set_size(s_detail_fetch_loading_overlay, pw, ph);
        lv_obj_align(s_detail_fetch_loading_overlay, LV_ALIGN_TOP_LEFT, 0, 0);
        lv_obj_set_style_bg_color(s_detail_fetch_loading_overlay, lv_color_hex(0x000000), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(s_detail_fetch_loading_overlay, LV_OPA_50, LV_PART_MAIN);
        lv_obj_set_style_border_width(s_detail_fetch_loading_overlay, 0, LV_PART_MAIN);
        lv_obj_clear_flag(s_detail_fetch_loading_overlay, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(s_detail_fetch_loading_overlay, LV_OBJ_FLAG_CLICKABLE);

        lbl = lv_label_create(s_detail_fetch_loading_overlay);
        lv_label_set_text(lbl, "加载中...");
        lv_obj_set_style_text_font(lbl, &lv_font_MiSansDemibold_18, LV_PART_MAIN);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_center(lbl);
    }
    lv_obj_clear_flag(s_detail_fetch_loading_overlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(s_detail_fetch_loading_overlay);
    screen_content_detail_fetch_loading_return_hit_place();
    screen_content_return_btn_raise_above_loading();
    lv_obj_invalidate(parent);
    lv_refr_now(NULL);
}

/* 串口打印加载内容：过长时截断，避免刷屏 */
#ifndef SC_LOAD_LOG_PREVIEW_MAX
#define SC_LOAD_LOG_PREVIEW_MAX 1200
#endif

static void sc_load_log_body(const char *tag, const char *body, int body_len)
{
    if (!body || body_len <= 0) {
        printf("%s（空）\n", tag);
        return;
    }
    if (body_len > SC_LOAD_LOG_PREVIEW_MAX) {
        printf("%s 长度=%d，仅打印前%d个字符：\n%.*s\n……（已截断）\n",
               tag, body_len, SC_LOAD_LOG_PREVIEW_MAX, SC_LOAD_LOG_PREVIEW_MAX, body);
    } else {
        printf("%s 长度=%d：\n%s\n", tag, body_len, body);
    }
}

static char *sc_strdup_local(const char *source_str)
{
    size_t byte_count;
    char *dup_buf;

    if (!source_str) {
        return NULL;
    }
    byte_count = strlen(source_str) + 1;
    dup_buf = (char *)malloc(byte_count);
    if (dup_buf) {
        memcpy(dup_buf, source_str, byte_count);
    }
    return dup_buf;
}

static void screen_content_apply_list_fetch_cb(void *user_data)
{
    (void)user_data;

    screen_content_detail_fetch_loading_hide();

    if (!is_content_page() || !s_body_scroll || !lv_obj_is_valid(s_body_scroll)) {
        return;
    }
    if (s_list_async_serial != s_list_expect_serial) {
        return;
    }

    switch (s_list_async_result) {
    case SC_LIST_R_MEM:
        screen_content_show_plain_message("内存不足");
        break;
    case SC_LIST_R_NET:
        screen_content_show_plain_message("获取内容失败，请检查网络");
        break;
    case SC_LIST_R_PARSE:
        screen_content_show_plain_message("数据解析失败");
        break;
    case SC_LIST_R_FMT:
        screen_content_show_plain_message("数据格式异常");
        break;
    case SC_LIST_R_EMPTY:
        screen_content_show_plain_message("暂无内容");
        break;
    case SC_LIST_R_OK:
        screen_content_add_bubble_list();
        break;
    default:
        break;
    }
    s_req_class_id = -1;
    if (s_body_scroll && lv_obj_is_valid(s_body_scroll)) {
        lv_obj_scroll_to_y(s_body_scroll, 0, LV_ANIM_OFF);
    }
}

static void screen_content_apply_detail_cb(void *user_data)
{
    sc_detail_payload_t *payload = (sc_detail_payload_t *)user_data;

    if (!payload) {
        return;
    }
    if (payload->apply_serial != s_detail_expect_serial) {
        screen_content_detail_fetch_loading_hide();
        if (payload->body) {
            free(payload->body);
        }
        free(payload);
        return;
    }
    if (!is_content_page()) {
        screen_content_detail_fetch_loading_hide();
        if (payload->body) {
            free(payload->body);
        }
        free(payload);
        return;
    }
    screen_content_detail_show(payload->heading, payload->body ? payload->body : "");
    lv_refr_now(NULL);
#if APPLAYER_ENABLE
    page_switch_content_tts_request_heading_and_body(payload->heading, payload->body);
#endif
    if (payload->body) {
        free(payload->body);
    }
    free(payload);
}

static const uint32_t s_badge_colors[] = {
    0xFF6B6B, 0xFFD93D, 0x6BCB77, 0x4D96FF, 0xC77DFF, 0xFF9A3C, 0x00C9A7, 0xFF6FB5,
};
#define BADGE_COLOR_COUNT  ((int)(sizeof(s_badge_colors) / sizeof(s_badge_colors[0])))
static cJSON *screen_content_json_get_list_array(cJSON *json_data)
{
    cJSON *list_array;
    if (!json_data) {
        return NULL;
    }
    list_array = cJSON_GetObjectItem(json_data, "data");
    if (!list_array || !cJSON_IsArray(list_array)) {
        list_array = cJSON_GetObjectItem(json_data, "lists");
    }
    if (!list_array || !cJSON_IsArray(list_array)) {
        list_array = cJSON_GetObjectItem(json_data, "list");
    }
    if (list_array && cJSON_IsArray(list_array)) {
        return list_array;
    }
    return NULL;
}

static int screen_content_item_id_from_json(cJSON *json_item)
{
    cJSON *id_obj = cJSON_GetObjectItem(json_item, "id");
    if (cJSON_IsNumber(id_obj)) {
        return (int)id_obj->valuedouble;
    }
    if (cJSON_IsString(id_obj) && id_obj->valuestring) {
        return atoi(id_obj->valuestring);
    }
    return 0;
}

static void screen_content_clear_items(void)
{
    s_content_count = 0;
    memset(s_content_items, 0, sizeof(s_content_items));
}

static void screen_content_detail_back_cb(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
        return;
    }
    s_detail_expect_serial++;
#if APPLAYER_ENABLE
    /* 用户在详情页 -> 列表页时把 TTS pipe 彻底清掉。
     * 之前只调 stop 会让 cycbuf 旧数据残留：从列表页再点其它条目时，
     * 新一段 http_tts_request 第一次写就遇到 cycbuf 满 + retry。 */
    page_switch_content_tts_force_clear();
#endif
    screen_content_detail_close();
}

static void screen_content_detail_show(const char *heading, const char *body_text)
{
    const char *body_for_log = body_text ? body_text : "";

    screen_content_detail_close();
    screen_content_detail_fetch_loading_hide();
    lv_coord_t scr_w = lv_disp_get_hor_res(NULL);
    lv_coord_t scr_h = lv_disp_get_ver_res(NULL);
    s_detail_overlay = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(s_detail_overlay, 0, 0);
    lv_obj_set_size(s_detail_overlay, scr_w, scr_h);
    lv_obj_set_style_bg_color(s_detail_overlay, lv_color_hex(0x0D1117), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_detail_overlay, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_opa(s_detail_overlay, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(s_detail_overlay, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_left(s_detail_overlay, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_right(s_detail_overlay, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_top(s_detail_overlay, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(s_detail_overlay, 10, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(s_detail_overlay, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(s_detail_overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(s_detail_overlay, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(s_detail_overlay, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(s_detail_overlay, 8, LV_PART_MAIN);
    lv_obj_t *back = lv_img_create(s_detail_overlay);
    lv_img_set_src(back, &_return_alpha_22x22);
    lv_obj_add_flag(back, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(back, screen_content_detail_back_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_clear_flag(back, LV_OBJ_FLAG_SCROLLABLE);
    if (heading && heading[0]) {
        lv_obj_t *heading_label = lv_label_create(s_detail_overlay);
        lv_label_set_text(heading_label, heading);
        lv_obj_set_width(heading_label, scr_w - 24);
        lv_label_set_long_mode(heading_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_font(heading_label, &lv_font_MiSansDemibold_18, LV_PART_MAIN);
        lv_obj_set_style_text_color(heading_label, lv_color_hex(0x4FC3F7), LV_PART_MAIN);
        lv_obj_set_style_text_letter_space(heading_label, SC_LETTER_SPACE_ROW, LV_PART_MAIN);
        lv_obj_set_style_text_line_space(heading_label, 6, LV_PART_MAIN);
    }
    lv_obj_t *body_scroll = lv_obj_create(s_detail_overlay);
    lv_obj_set_width(body_scroll, scr_w - 39);
    lv_obj_set_flex_grow(body_scroll, 1);
    lv_obj_set_style_bg_opa(body_scroll, 0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(body_scroll, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(body_scroll, 0, LV_PART_MAIN);
    lv_obj_set_scroll_dir(body_scroll, LV_DIR_VER);
#if SC_DETAIL_PAGED
    /* 翻页模式：禁掉用户拖动 + 滚动条，靠"上一页/下一页"程序滚动；
     * lv_obj_scroll_to_y 不受 SCROLLABLE flag 影响，仍能生效。 */
    lv_obj_set_scrollbar_mode(body_scroll, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(body_scroll, LV_OBJ_FLAG_SCROLLABLE);
#else
    lv_obj_set_scrollbar_mode(body_scroll, LV_SCROLLBAR_MODE_AUTO);
#endif
    lv_obj_t *body_lab = lv_label_create(body_scroll);
    lv_obj_set_width(body_lab, scr_w - 47);
    lv_label_set_long_mode(body_lab, LV_LABEL_LONG_WRAP);
    lv_label_set_text(body_lab, body_text ? body_text : "");
    printf("[屏显-详情] 标题=%s\n", (heading && heading[0]) ? heading : "（无）");
    sc_load_log_body("[屏显-详情] 正文", body_for_log, (int)strlen(body_for_log));
    lv_obj_set_style_text_font(body_lab, &lv_font_MiSansDemibold_18, LV_PART_MAIN);
    lv_obj_set_style_text_color(body_lab, lv_color_hex(0xE0E0E0), LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(body_lab, 4, LV_PART_MAIN);
    lv_obj_set_style_text_line_space(body_lab, 8, LV_PART_MAIN);

    /* 缓存引用，供顶部 上一页/下一页/重播 回调访问 */
    s_detail_body_scroll = body_scroll;
    s_detail_body_lab = body_lab;

#if SC_DETAIL_PAGED
    /* 翻页之前必须把 body_scroll 的可见高度收缩到 "行高×整数行"，否则：
     *  - 每页底部会露出上一行的下半截、顶部露出下一行的上半截（半行字）；
     *  - 翻页滑动时同一行字会同时出现在新旧页边缘，看起来像"分裂/重影"。
     *
     * line_h = 字体行高 + text_line_space（label 上设置了 8）。
     * 先 update_layout 让 flex_grow=1 的 body_scroll 撑到自然高度，再换成
     * flex_grow=0 + 显式 height = lines*line_h；底部空出的几像素留给
     * overlay，FLEX_ALIGN_START 不会自动补，刚好让右下角页码 label 落位。 */
    lv_obj_update_layout(s_detail_overlay);
    lv_coord_t natural_h = lv_obj_get_height(body_scroll);
    lv_coord_t line_h = (lv_coord_t)lv_font_get_line_height(&lv_font_MiSansDemibold_18) + 8;
    if (line_h > 0 && natural_h >= line_h) {
        lv_coord_t lines = natural_h / line_h;
        lv_coord_t aligned_h = lines * line_h;
        if (aligned_h != natural_h) {
            lv_obj_set_flex_grow(body_scroll, 0);
            lv_obj_set_height(body_scroll, aligned_h);
            /* body_scroll 高度变了，scroll_bottom / page_h 都要重算，
             * 再走一遍 layout，下面页码初始化读到的就是收缩后的高度。 */
            lv_obj_update_layout(s_detail_overlay);
        }
    }

    /* 末页对齐：把 body_lab 底部用 pad_bottom 顶到 page_h 的整数倍，
     * 否则 max_y = content_h - page_h 不是 page_h 的整数倍，滚到 max_y
     * 时末页顶部仍会露上一页底部那行的下半。多出来的空白对用户没差，
     * 但 max_y 落在行边界后，"上一页/下一页"按整页跳完全干净。 */
    lv_coord_t page_h_now = lv_obj_get_height(body_scroll);
    lv_coord_t lab_h = lv_obj_get_height(body_lab);
    if (page_h_now > 0 && lab_h > page_h_now) {
        int total_pages = (lab_h + page_h_now - 1) / page_h_now;
        lv_coord_t target_h = (lv_coord_t)total_pages * page_h_now;
        lv_coord_t pad = target_h - lab_h;
        if (pad > 0) {
            lv_obj_set_style_pad_bottom(body_lab, pad, LV_PART_MAIN);
            lv_obj_update_layout(s_detail_overlay);
        }
    }

    /* 右下角页码：放在 overlay 下并打 FLOATING flag，使其不参与 overlay 的
     * flex 列布局，单独按 align 锚到内容区右下角；正文区那条 flex 行因此
     * 不会被这个 label 占走任何高度。 */
    s_detail_page_indicator = lv_label_create(s_detail_overlay);
    lv_obj_add_flag(s_detail_page_indicator, LV_OBJ_FLAG_FLOATING);
    lv_obj_set_style_text_font(s_detail_page_indicator, &lv_font_Barlow__12, LV_PART_MAIN);
    lv_obj_set_style_text_color(s_detail_page_indicator, lv_color_hex(0x9E9E9E), LV_PART_MAIN);
    lv_obj_set_style_pad_all(s_detail_page_indicator, 0, LV_PART_MAIN);
    lv_label_set_text(s_detail_page_indicator, "1/1");
    /* 略微往上抬 2px，让数字不顶到 overlay 底部 padding 边缘 */
    lv_obj_align(s_detail_page_indicator, LV_ALIGN_BOTTOM_RIGHT, 0, -2);

    sc_detail_update_page_indicator(0);
    /* 记录正文字节总数，供 TTS 任务估算每句话在第几页 */
    s_sc_body_text_total = body_text ? (int)strlen(body_text) : 0;
    /* 缓存 layout 后的页面/内容高度，TTS 任务直接读，不跨线程调 LVGL API */
    s_detail_page_h = (body_scroll && lv_obj_is_valid(body_scroll))
                      ? lv_obj_get_height(body_scroll) : 0;
    s_detail_content_h = (body_lab && lv_obj_is_valid(body_lab))
                         ? lv_obj_get_height(body_lab) : 0;
#if SC_DETAIL_AUTO_PAGE
    /* 详情页首次展示，自动翻页处于启用状态；从第 1 页开始跟踪 */
    s_auto_page_enabled = true;
    s_auto_last_page = 1;
#endif
#endif

    /* overlay 已建好，把顶部三个 label 显示出来并提到 overlay 之上 */
    screen_content_topbar_set_visible(true);
}

static void screen_content_detail_fetch_loading_hide_async_cb(void *unused)
{
    (void)unused;
    screen_content_detail_fetch_loading_hide();
}

static void screen_content_detail_fetch_worker(void *detail_req_serial_opaque)
{
    unsigned work_serial = (unsigned)(uintptr_t)detail_req_serial_opaque;
    int content_id;
    sc_detail_payload_t *pay;
    char *buf = NULL;
    int ret;

    if (work_serial != s_detail_expect_serial) {
        return;
    }
    content_id = s_detail_req_content_id;
    if (content_id <= 0) {
        return;
    }

    pay = (sc_detail_payload_t *)calloc(1, sizeof(*pay));
    if (!pay) {
        (void)lv_async_call(screen_content_detail_fetch_loading_hide_async_cb, NULL);
        return;
    }
    pay->apply_serial = work_serial;
    strncpy(pay->heading, s_detail_heading, sizeof(pay->heading) - 1);

    buf = (char *)malloc(SC_CONTENT_BUF_SIZE);
    if (!buf) {
        pay->body = sc_strdup_local("内存不足");
        goto post;
    }
    memset(buf, 0, SC_CONTENT_BUF_SIZE);
    ret = http_ai_get_content_details_data(content_id, 0, buf, SC_CONTENT_BUF_SIZE, NULL);
    if (ret <= 0) {
        printf("[内容详情] 内容id=%d 请求序号=%u 拉取详情接口返回值=%d（失败）\n",
               content_id, work_serial, ret);
        pay->body = sc_strdup_local("获取详情失败，请检查网络");
        free(buf);
        goto post;
    }
    {
        int end = ret < SC_CONTENT_BUF_SIZE ? ret : SC_CONTENT_BUF_SIZE - 1;

        buf[end] = '\0';
        printf("[内容详情] 内容id=%d 请求序号=%u 返回数据长度=%d\n", content_id, work_serial, ret);
        sc_load_log_body("[内容详情] 原始JSON", buf, end);
        {
            cJSON *root = cJSON_Parse(buf);

            if (!root) {
                printf("[内容详情] JSON解析失败\n");
                pay->body = sc_strdup_local("数据解析失败");
            } else {
                cJSON *data = cJSON_GetObjectItem(root, "data");
                cJSON *content_text = data ? cJSON_GetObjectItem(data, "content_text") : NULL;

                if (cJSON_IsString(content_text) && content_text->valuestring) {
                    /* 必须先拷贝：cJSON_Delete 会释放 valuestring，不能只保留指针 */
                    pay->body = sc_strdup_local(content_text->valuestring);
                }
                if (!pay->body) {
                    pay->body = sc_strdup_local("暂无讲解内容");
                }
                cJSON_Delete(root);
            }
            free(buf);
            buf = NULL;
        }
        if (pay->body) {
            sc_load_log_body("[内容详情] 讲解正文", pay->body, (int)strlen(pay->body));
        }
    }

post:
    if (work_serial != s_detail_expect_serial) {
        if (pay->body) {
            free(pay->body);
        }
        free(pay);
        return;
    }
    if (lv_async_call(screen_content_apply_detail_cb, pay) != LV_RES_OK) {
        if (pay->body) {
            free(pay->body);
        }
        free(pay);
        (void)lv_async_call(screen_content_detail_fetch_loading_hide_async_cb, NULL);
    }
}

static void screen_content_item_click_cb(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
        return;
    }
    int row_index = (int)(intptr_t)lv_event_get_user_data(event);
    if (row_index < 0 || row_index >= s_content_count) {
        return;
    }
    int item_id = s_content_items[row_index].id;
    if (item_id <= 0) {
        return;
    }
#if SC_PINYIN_ENABLE
    /* 拼音兜底项：直接显示拼音 overlay，不走网络请求详情 */
    if (pinyin_is_pinyin_item(item_id)) {
        pinyin_overlay_show_for_item_idx(row_index, s_content_items,
                                         s_content_count,
                                         (int)sizeof(sc_content_item_t));
        return;
    }
#endif
    /* 立即停止当前 TTS，避免解码线程继续写入 cycbuf */
    page_switch_content_tts_stop();
    strncpy(s_detail_heading, s_content_items[row_index].title, sizeof(s_detail_heading) - 1);
    s_detail_heading[sizeof(s_detail_heading) - 1] = '\0';
    s_detail_expect_serial++;
    s_detail_req_serial = s_detail_expect_serial;
    s_detail_req_content_id = item_id;
    screen_content_detail_fetch_loading_show();
    sys_timeout_add((void *)(uintptr_t)s_detail_req_serial, screen_content_detail_fetch_worker, 100);
}

static void list_bubble_style_ensure(void)
{
    if (s_list_bubble_style_inited) {
        return;
    }
    lv_style_init(&s_list_bubble_style);
    lv_style_set_radius(&s_list_bubble_style, 12);
    lv_style_set_bg_color(&s_list_bubble_style, lv_color_hex(0xFFFFFF));
    lv_style_set_bg_opa(&s_list_bubble_style, LV_OPA_20);
    lv_style_set_pad_top(&s_list_bubble_style, 4);
    lv_style_set_pad_bottom(&s_list_bubble_style, 4);
    lv_style_set_pad_left(&s_list_bubble_style, 0);
    lv_style_set_pad_right(&s_list_bubble_style, 0);
    lv_style_set_border_opa(&s_list_bubble_style, 0);
    lv_style_set_shadow_width(&s_list_bubble_style, 0);
    s_list_bubble_style_inited = true;
}

static void screen_content_add_bubble_list(void)
{
    if (!s_body_scroll || !lv_obj_is_valid(s_body_scroll) || s_content_count <= 0) {
        return;
    }
    lv_obj_clean(s_body_scroll);
    screen_content_scroll_reset_layout();
    list_bubble_style_ensure();
    lv_obj_update_layout(s_body_scroll);
    lv_coord_t cont_w = lv_obj_get_content_width(s_body_scroll);
    if (cont_w < 40) {
        cont_w = (lv_coord_t)(lv_obj_get_width(s_body_scroll) - 8);
    }
    lv_coord_t bubble_w = cont_w - 2 * SC_BUBBLE_SIDE_PAD - 8;
    if (bubble_w < 80) {
        bubble_w = cont_w - 16;
    }
    lv_obj_t *message_row = lv_obj_create(s_body_scroll);
    lv_obj_set_width(message_row, LV_PCT(100));
    lv_obj_set_height(message_row, LV_SIZE_CONTENT);
    lv_obj_set_scrollbar_mode(message_row, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(message_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(message_row, 0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(message_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_left(message_row, SC_BUBBLE_SIDE_PAD, LV_PART_MAIN);
    lv_obj_set_style_pad_right(message_row, SC_BUBBLE_SIDE_PAD, LV_PART_MAIN);
    lv_obj_set_style_pad_top(message_row, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(message_row, 2, LV_PART_MAIN);
    lv_obj_set_flex_flow(message_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(message_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_t *bubble = lv_obj_create(message_row);
    lv_obj_set_width(bubble, bubble_w);
    lv_obj_set_height(bubble, LV_SIZE_CONTENT);
    lv_obj_set_scrollbar_mode(bubble, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(bubble, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(bubble, &s_list_bubble_style, LV_PART_MAIN);
    lv_obj_set_flex_flow(bubble, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(bubble, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    const int H_PAD = 12;
    const int COL_GAP = 8;
    const int NUM_W = 28;
    const int ARROW_W = 20;
    int title_w = (int)bubble_w - H_PAD * 2 - NUM_W - ARROW_W - COL_GAP * 2;
    if (title_w < 48) {
        title_w = 48;
    }
    for (int i = 0; i < s_content_count; i++) {
        lv_obj_t *item = lv_obj_create(bubble);
        lv_obj_set_width(item, bubble_w);
        lv_obj_set_height(item, LV_SIZE_CONTENT);
        lv_obj_set_scrollbar_mode(item, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_flex_flow(item, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(item, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_left(item, H_PAD, LV_PART_MAIN);
        lv_obj_set_style_pad_right(item, H_PAD, LV_PART_MAIN);
        lv_obj_set_style_pad_top(item, 10, LV_PART_MAIN);
        lv_obj_set_style_pad_bottom(item, 10, LV_PART_MAIN);
        lv_obj_set_style_pad_column(item, COL_GAP, LV_PART_MAIN);
        lv_obj_set_style_radius(item, 0, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(item, 0, LV_PART_MAIN);
        lv_obj_set_style_border_opa(item, 0, LV_PART_MAIN);
        lv_obj_set_style_bg_color(item, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(item, LV_OPA_20, LV_PART_MAIN | LV_STATE_PRESSED);
        if (i < s_content_count - 1) {
            uint32_t sep_clr = s_badge_colors[i % BADGE_COLOR_COUNT];
            lv_obj_set_style_border_side(item, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
            lv_obj_set_style_border_color(item, lv_color_hex(sep_clr), LV_PART_MAIN);
            lv_obj_set_style_border_opa(item, LV_OPA_50, LV_PART_MAIN);
            lv_obj_set_style_border_width(item, 1, LV_PART_MAIN);
        }
        uint32_t badge_clr = s_badge_colors[i % BADGE_COLOR_COUNT];
        char num_buf[8];
        snprintf(num_buf, sizeof(num_buf), "%d", i + 1);
        lv_obj_t *num_cont = lv_obj_create(item);
        lv_obj_set_size(num_cont, NUM_W, NUM_W);
        lv_obj_set_style_radius(num_cont, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        lv_obj_set_style_bg_color(num_cont, lv_color_hex(badge_clr), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(num_cont, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_opa(num_cont, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(num_cont, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(num_cont, 0, LV_PART_MAIN);
        lv_obj_clear_flag(num_cont, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_t *num_lab = lv_label_create(num_cont);
        lv_label_set_text(num_lab, num_buf);
        lv_obj_center(num_lab);
        lv_obj_set_style_text_font(num_lab, &lv_font_MiSansDemibold_18, LV_PART_MAIN);
        lv_obj_set_style_text_color(num_lab, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_t *title_lab = lv_label_create(item);
        lv_label_set_text(title_lab, s_content_items[i].title);
        lv_obj_set_size(title_lab, title_w, LV_SIZE_CONTENT);
        lv_label_set_long_mode(title_lab, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_font(title_lab, &lv_font_MiSansDemibold_18, LV_PART_MAIN);
        lv_obj_set_style_text_color(title_lab, lv_color_hex(0xE8E8E8), LV_PART_MAIN);
        lv_obj_set_style_text_letter_space(title_lab, SC_LETTER_SPACE_ROW, LV_PART_MAIN);
        lv_obj_set_style_text_line_space(title_lab, 4, LV_PART_MAIN);
        lv_obj_t *arrow_lab = lv_label_create(item);
        lv_label_set_text(arrow_lab, ">");
        lv_obj_set_width(arrow_lab, ARROW_W);
        lv_obj_set_style_text_font(arrow_lab, &lv_font_MiSansDemibold_18, LV_PART_MAIN);
        lv_obj_set_style_text_color(arrow_lab, lv_color_hex(0x4FC3F7), LV_PART_MAIN);
        lv_obj_set_style_text_align(arrow_lab, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
        lv_obj_add_event_cb(item, screen_content_item_click_cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);
    }
    printf("[屏显-列表] 共%d条（与左侧序号一致）\n", s_content_count);
    for (int si = 0; si < s_content_count; si++) {
        printf("  第%d条 标题=%s\n", si + 1, s_content_items[si].title);
    }
    lv_obj_scroll_to_y(s_body_scroll, 0, LV_ANIM_OFF);
}

static void screen_content_list_fetch_worker(void *list_req_serial_opaque)
{
    unsigned work_serial = (unsigned)(uintptr_t)list_req_serial_opaque;
    int cid = s_req_class_id;
    char *buf;

    if (cid < 0) {
        return;
    }
    if (work_serial != s_list_expect_serial) {
        return;
    }

    s_list_async_cid = cid;
    screen_content_clear_items();

    buf = (char *)malloc(SC_CONTENT_BUF_SIZE);
    if (!buf) {
        s_list_async_result = SC_LIST_R_MEM;
        goto post;
    }
    memset(buf, 0, SC_CONTENT_BUF_SIZE);

    {
        int ret = http_ai_get_content_data(cid, NULL, 0, buf, SC_CONTENT_BUF_SIZE, NULL);

        if (ret <= 0) {
            printf("[内容列表] 分类id=%d 请求序号=%u 拉取列表接口返回值=%d（失败）\n",
                   cid, work_serial, ret);
            free(buf);
#if SC_PINYIN_ENABLE
            /* 网络失败时尝试拼音本地兜底 */
            {
                const pinyin_local_fallback_t *fb = pinyin_find_local_fallback(cid);
                if (fb) {
                    char *fb_buf2 = (char *)malloc(SC_CONTENT_BUF_SIZE);
                    if (fb_buf2) {
                        int fb_len = pinyin_build_local_fallback_json(fb, fb_buf2, SC_CONTENT_BUF_SIZE);
                        if (fb_len > 0) {
                            fb_buf2[fb_len] = '\0';
                            screen_content_clear_items();
                            cJSON *fb_root = cJSON_Parse(fb_buf2);
                            if (fb_root) {
                                cJSON *fb_data = cJSON_GetObjectItem(fb_root, "data");
                                cJSON *fb_list = screen_content_json_get_list_array(fb_data);
                                if (fb_list) {
                                    int fn = cJSON_GetArraySize(fb_list);
                                    if (fn > SC_CONTENT_LINE_MAX) {
                                        fn = SC_CONTENT_LINE_MAX;
                                    }
                                    for (int fi = 0; fi < fn; fi++) {
                                        cJSON *fi_item = cJSON_GetArrayItem(fb_list, fi);
                                        cJSON *ft = cJSON_GetObjectItem(fi_item, "title");
                                        const char *fts = (ft && cJSON_IsString(ft)) ? ft->valuestring : NULL;
                                        if (!fts || !fts[0]) {
                                            continue;
                                        }
                                        s_content_items[s_content_count].id = screen_content_item_id_from_json(fi_item);
                                        strncpy(s_content_items[s_content_count].title, fts, SC_TITLE_MAX - 1);
                                        s_content_items[s_content_count].title[SC_TITLE_MAX - 1] = '\0';
                                        s_content_count++;
                                    }
                                }
                                cJSON_Delete(fb_root);
                            }
                        }
                        free(fb_buf2);
                    }
                    if (s_content_count > 0) {
                        printf("[内容列表] 网络失败，使用拼音本地兜底 class_id=%d\n", cid);
                        s_list_async_result = SC_LIST_R_OK;
                        goto post;
                    }
                }
            }
#endif
            s_list_async_result = SC_LIST_R_NET;
            goto post;
        }
        {
            int end = ret < SC_CONTENT_BUF_SIZE ? ret : SC_CONTENT_BUF_SIZE - 1;
            cJSON *root;

            buf[end] = '\0';
            printf("[内容列表] 分类id=%d 请求序号=%u 返回数据长度=%d\n", cid, work_serial, ret);
            sc_load_log_body("[内容列表] 原始JSON", buf, end);
            root = cJSON_Parse(buf);
            free(buf);
            buf = NULL;
            if (!root) {
                printf("[内容列表] JSON解析失败\n");
                s_list_async_result = SC_LIST_R_PARSE;
                goto post;
            }
            {
                cJSON *data = cJSON_GetObjectItem(root, "data");
                cJSON *list = screen_content_json_get_list_array(data);

                if (!list) {
                    printf("[内容列表] JSON中未找到列表数组（检查 data / lists / list 等字段）\n");
                    s_list_async_result = SC_LIST_R_FMT;
                    cJSON_Delete(root);
                    goto post;
                }
                {
                    int n = cJSON_GetArraySize(list);

                    if (n > SC_CONTENT_LINE_MAX) {
                        n = SC_CONTENT_LINE_MAX;
                    }
                    for (int i = 0; i < n; i++) {
                        cJSON *item = cJSON_GetArrayItem(list, i);
                        cJSON *title_obj;
                        const char *t;

                        if (!item) {
                            continue;
                        }
                        title_obj = cJSON_GetObjectItem(item, "title");
                        if (!title_obj) {
                            title_obj = cJSON_GetObjectItem(item, "name");
                        }
                        t = (title_obj && cJSON_IsString(title_obj)) ? title_obj->valuestring : NULL;
                        if (!t || !t[0]) {
                            continue;
                        }
                        s_content_items[s_content_count].id = screen_content_item_id_from_json(item);
                        strncpy(s_content_items[s_content_count].title, t, SC_TITLE_MAX - 1);
                        s_content_items[s_content_count].title[SC_TITLE_MAX - 1] = '\0';
                        s_content_count++;
                    }
                }
                cJSON_Delete(root);
            }
#if SC_PINYIN_ENABLE
            /* 拼音本地兜底：服务器返回空数据时，用本地拼音数据填充 */
            if (s_content_count == 0) {
                const pinyin_local_fallback_t *fb = pinyin_find_local_fallback(cid);
                if (fb) {
                    char *fb_buf = (char *)malloc(SC_CONTENT_BUF_SIZE);
                    if (fb_buf) {
                        int fb_len = pinyin_build_local_fallback_json(fb, fb_buf, SC_CONTENT_BUF_SIZE);
                        if (fb_len > 0) {
                            fb_buf[fb_len] = '\0';
                            printf("[内容列表] 使用拼音本地兜底 class_id=%d, 条目数=%d\n",
                                   cid, fb->count);
                            screen_content_clear_items();
                            /* 重新解析兜底 JSON */
                            cJSON *fb_root = cJSON_Parse(fb_buf);
                            if (fb_root) {
                                cJSON *fb_data = cJSON_GetObjectItem(fb_root, "data");
                                cJSON *fb_list = screen_content_json_get_list_array(fb_data);
                                if (fb_list) {
                                    int fn = cJSON_GetArraySize(fb_list);
                                    if (fn > SC_CONTENT_LINE_MAX) {
                                        fn = SC_CONTENT_LINE_MAX;
                                    }
                                    for (int fi = 0; fi < fn; fi++) {
                                        cJSON *fi_item = cJSON_GetArrayItem(fb_list, fi);
                                        cJSON *ft = cJSON_GetObjectItem(fi_item, "title");
                                        const char *fts = (ft && cJSON_IsString(ft)) ? ft->valuestring : NULL;
                                        if (!fts || !fts[0]) {
                                            continue;
                                        }
                                        s_content_items[s_content_count].id = screen_content_item_id_from_json(fi_item);
                                        strncpy(s_content_items[s_content_count].title, fts, SC_TITLE_MAX - 1);
                                        s_content_items[s_content_count].title[SC_TITLE_MAX - 1] = '\0';
                                        s_content_count++;
                                    }
                                }
                                cJSON_Delete(fb_root);
                            }
                        }
                        free(fb_buf);
                    }
                }
            }
#endif
            if (s_content_count == 0) {
                s_list_async_result = SC_LIST_R_EMPTY;
            } else {
                s_list_async_result = SC_LIST_R_OK;
            }
            printf("[内容列表] 解析条目数=%d\n", s_content_count);
            for (int pi = 0; pi < s_content_count; pi++) {
                printf("  第%d条 id=%d 标题=%s\n", pi + 1, s_content_items[pi].id, s_content_items[pi].title);
            }
        }
    }

post:
    if (work_serial != s_list_expect_serial) {
        return;
    }
    s_list_async_serial = work_serial;
    if (lv_async_call(screen_content_apply_list_fetch_cb, NULL) != LV_RES_OK) {
        /* 无法投递 LVGL 时无法在此线程调用 lv_*，仅清请求标记 */
        s_req_class_id = -1;
    }
}

#endif /* APPLAYER_ENABLE */
static void screen_content_perform_return_action(void)
{
#if APPLAYER_ENABLE
    page_switch_content_tts_stop();
    screen_content_detail_fetch_cancel();
#endif
#if SC_PINYIN_ENABLE
    pinyin_overlay_close();
#endif
    s_list_expect_serial++;
    s_detail_expect_serial++;
    s_req_class_id = -1;
    screen_content_detail_close();
    switch_to_list_page();
}

static void screen_content_return_cb(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
        return;
    }
    screen_content_perform_return_action();
}

static void screen_content_on_screen_loaded_cb(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_SCREEN_LOADED) {
        return;
    }
    if (!s_body_scroll || !lv_obj_is_valid(s_body_scroll)) {
        return;
    }
#if APPLAYER_ENABLE
    if (s_req_class_id < 0) {
        screen_content_show_plain_message("无效分类，请返回重选");
        return;
    }
    screen_content_show_plain_message("加载中...");
    if (guider_ui.screen_content && lv_obj_is_valid(guider_ui.screen_content)) {
        lv_obj_invalidate(guider_ui.screen_content);
    }
    lv_obj_invalidate(s_body_scroll);
    lv_refr_now(NULL);
    sys_timeout_add((void *)(uintptr_t)s_list_expect_serial, screen_content_list_fetch_worker, 100);
#else
    screen_content_show_plain_message("需启用 APPLAYER 以请求网络内容");
#endif
}

static void screen_content_ensure_body_widgets(lv_ui *ui_ptr)
{
    if (!ui_ptr || !ui_ptr->screen_content) {
        return;
    }
    if (s_body_scroll && lv_obj_is_valid(s_body_scroll)) {
        return;
    }
    s_body_scroll = lv_obj_create(ui_ptr->screen_content);
    lv_obj_set_pos(s_body_scroll, CONFIG_LVGL_CONTENT_LIST_X_POS, 40);
    lv_obj_set_size(s_body_scroll, 270, 195);
    lv_obj_set_scroll_dir(s_body_scroll, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(s_body_scroll, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_border_width(s_body_scroll, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(s_body_scroll, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(s_body_scroll, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    screen_content_scroll_reset_layout();
}

void screen_content_set_title(const char *title_text)
{
    if (!guider_ui.screen_content_label_name) {
        return;
    }
    lv_label_set_text(guider_ui.screen_content_label_name, title_text ? title_text : "");
    printf("[屏显-标题栏] %s\n", (title_text && title_text[0]) ? title_text : "（空）");
}

void screen_content_open_with_class_id(const char *l2_title, const char *l1_title, int class_id)
{
    char title_buf[96];
    if (l2_title && l2_title[0] && l1_title && l1_title[0]) {
        snprintf(title_buf, sizeof(title_buf), "%s·%s", l2_title, l1_title);
    } else {
        snprintf(title_buf, sizeof(title_buf), "%s", l2_title ? l2_title : "");
    }
    screen_content_set_title(title_buf);
#if APPLAYER_ENABLE
    screen_content_detail_fetch_loading_hide();
#endif
#if SC_PINYIN_ENABLE
    pinyin_overlay_close();
#endif
    screen_content_detail_close();
    s_list_expect_serial++;
    s_detail_expect_serial++;
    s_req_class_id = class_id;
    switch_to_content_page();
}

#if SC_DETAIL_PAGED

/* 把右下角 "x/y" 页码刷新到 cur_y 这个滚动位置对应的页号。
 *
 * 让调用方传 cur_y 而不是函数自己 lv_obj_get_scroll_y(body_scroll)，是因为
 * scroll_by_page 可能先算出 new_y、再去刷数字，最后才真正 scroll_to_y；
 * 重播复位时传 0 也合理。
 *
 * 总页数用 body_lab 实际高度 / body_scroll 可见高度向上取整；这两个高度都依赖
 * flex 布局完成，所以 detail_show 里第一次调本函数前先 lv_obj_update_layout
 * 了一遍 overlay。 */
static void sc_detail_update_page_indicator(lv_coord_t cur_y)
{
    if (!s_detail_page_indicator || !lv_obj_is_valid(s_detail_page_indicator)) {
        return;
    }
    if (!s_detail_body_scroll || !lv_obj_is_valid(s_detail_body_scroll)) {
        return;
    }
    if (!s_detail_body_lab || !lv_obj_is_valid(s_detail_body_lab)) {
        return;
    }
    lv_coord_t page_h = lv_obj_get_height(s_detail_body_scroll);
    lv_coord_t content_h = lv_obj_get_height(s_detail_body_lab);
    int total = 1;
    int cur = 1;
    if (page_h > 0) {
        if (content_h > page_h) {
            total = (content_h + page_h - 1) / page_h;
        }
        /* 最后一页常常不满一屏：例如 page_h=100、content_h=850 时 total=9，
         * 但 max_y = 850-100 = 750，"下一页"按到 cur_y=750 只能算出
         * 750/100+1=8。检测到已经到底就直接用 total，避免末页永远 8/9。 */
        lv_coord_t max_y = (content_h > page_h) ? (content_h - page_h) : 0;
        if (cur_y >= max_y && max_y > 0) {
            cur = total;
        } else {
            cur = (cur_y / page_h) + 1;
        }
        if (cur < 1) {
            cur = 1;
        }
        if (cur > total) {
            cur = total;
        }
    }
    char buf[16];
    lv_snprintf(buf, sizeof(buf), "%d/%d", cur, total);
    lv_label_set_text(s_detail_page_indicator, buf);
}

/* 翻页：按一屏可见高度直接 scroll_to_y，无动画。
 * dir < 0 上一页，dir > 0 下一页；到边界自动夹到 [0, max_y]。
 * 任何手动翻页（点击「上一页/下一页」或左右滑动）均禁用自动翻页。 */
static void screen_content_detail_scroll_by_page(int dir)
{
#if SC_DETAIL_AUTO_PAGE
    /* 用户手动翻页后，后续不再自动翻页 */
    s_auto_page_enabled = false;
#endif
    if (!s_detail_body_scroll || !lv_obj_is_valid(s_detail_body_scroll)) {
        return;
    }
    if (!s_detail_body_lab || !lv_obj_is_valid(s_detail_body_lab)) {
        return;
    }
    lv_coord_t page_h = lv_obj_get_height(s_detail_body_scroll);
    if (page_h <= 0) {
        return;
    }
    lv_coord_t cur_y = lv_obj_get_scroll_y(s_detail_body_scroll);
    lv_coord_t max_y = cur_y + lv_obj_get_scroll_bottom(s_detail_body_scroll);
    lv_coord_t new_y;

    if (dir < 0) {
        new_y = cur_y - page_h;
        if (new_y < 0) {
            new_y = 0;
        }
    } else {
        new_y = cur_y + page_h;
        if (new_y > max_y) {
            new_y = max_y;
        }
    }
    if (new_y == cur_y) {
        return;
    }

    lv_obj_scroll_to_y(s_detail_body_scroll, new_y, LV_ANIM_OFF);
    sc_detail_update_page_indicator(new_y);
}

static void screen_content_label_last_page_cb(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
        return;
    }
    screen_content_detail_scroll_by_page(-1);
}

static void screen_content_label_next_page_cb(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
        return;
    }
    screen_content_detail_scroll_by_page(1);
}

/**
 * @brief 阅读详情页：左右滑动手势翻页（与顶部「上一页/下一页」一致）
 * @param dir >0 下一页，<0 上一页
 * @return 1 已处理，0 非详情页或未打开 overlay
 */
int screen_content_detail_swipe_page(int dir)
{
    if (!is_content_page()) {
        return 0;
    }
    if (!s_detail_overlay || !lv_obj_is_valid(s_detail_overlay)) {
        return 0;
    }
    if (dir > 0) {
        screen_content_detail_scroll_by_page(1);
    } else if (dir < 0) {
        screen_content_detail_scroll_by_page(-1);
    } else {
        return 0;
    }
    return 1;
}

#else /* !SC_DETAIL_PAGED */
int screen_content_detail_swipe_page(int dir)
{
    (void)dir;
    return 0;
}
#endif /* SC_DETAIL_PAGED */

#if APPLAYER_ENABLE
/* 重播延时回调（在 sys_timer 任务里执行，不在 LVGL 主循环）。
 * 翻页/滚动两种详情模式共用。 */
static void screen_content_label_repeat_send_worker(void *priv)
{
    unsigned my_serial = (unsigned)(uintptr_t)priv;
    const char *body;

    if (my_serial != s_detail_repeat_req_serial) {
        return;
    }
    if (!is_content_page()) {
        return;
    }
    if (!s_detail_body_lab || !lv_obj_is_valid(s_detail_body_lab)) {
        return;
    }
    body = lv_label_get_text(s_detail_body_lab);
    page_switch_content_tts_request_heading_and_body(s_detail_heading, body);
    printf("[屏显-详情] 重播：标题=%s\n", (s_detail_heading[0]) ? s_detail_heading : "（无）");
}
#endif

static void screen_content_label_repeat_cb(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
        return;
    }
#if APPLAYER_ENABLE
    if (!s_detail_body_lab || !lv_obj_is_valid(s_detail_body_lab)) {
        return;
    }
    if (s_detail_body_scroll && lv_obj_is_valid(s_detail_body_scroll)) {
        lv_obj_scroll_to_y(s_detail_body_scroll, 0, LV_ANIM_OFF);
#if SC_DETAIL_PAGED
        sc_detail_update_page_indicator(0);
#if SC_DETAIL_AUTO_PAGE
        /* 重播时重新启用自动翻页，从第 1 页开始跟踪 */
        s_auto_page_enabled = true;
        s_auto_last_page = 1;
#endif
#endif
    }
    page_switch_content_tts_force_clear();
    s_detail_repeat_req_serial++;
    sys_timeout_add((void *)(uintptr_t)s_detail_repeat_req_serial,
                    screen_content_label_repeat_send_worker, 80);
#endif
}

void screen_content_event_init(lv_ui *ui_ptr)
{
    if (!ui_ptr) {
        return;
    }
    lv_obj_set_scroll_dir(ui_ptr->screen_content, LV_DIR_NONE);
    lv_obj_add_event_cb(ui_ptr->screen_content_img_return, screen_content_return_cb, LV_EVENT_CLICKED, NULL);
    ui_subpage_return_set_large_touch_area(ui_ptr->screen_content_img_return, (lv_coord_t)UI_SUBPAGE_RETURN_EXT_PAD);
    lv_obj_add_event_cb(ui_ptr->screen_content, screen_content_on_screen_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);

    /* 「重播」：滚动/翻页模式详情打开时均显示并挂事件 */
    if (ui_ptr->screen_content_label_repeat) {
        lv_obj_add_flag(ui_ptr->screen_content_label_repeat, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(ui_ptr->screen_content_label_repeat,
                            screen_content_label_repeat_cb, LV_EVENT_CLICKED, NULL);
    }
#if SC_DETAIL_PAGED
    if (ui_ptr->screen_content_label_last_page) {
        lv_obj_add_flag(ui_ptr->screen_content_label_last_page, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(ui_ptr->screen_content_label_last_page,
                            screen_content_label_last_page_cb, LV_EVENT_CLICKED, NULL);
    }
    if (ui_ptr->screen_content_label_next_page) {
        lv_obj_add_flag(ui_ptr->screen_content_label_next_page, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(ui_ptr->screen_content_label_next_page,
                            screen_content_label_next_page_cb, LV_EVENT_CLICKED, NULL);
    }
#endif

    screen_content_ensure_body_widgets(ui_ptr);
}

void screen_content_event_setup(void)
{
    screen_content_event_init(&guider_ui);
}

#else /* !CONFIG_LVGL_CONTENT_LIST_ENABLE */

int screen_content_detail_swipe_page(int dir)
{
    (void)dir;
    return 0;
}

void screen_content_set_title(const char *title_text)
{
    (void)title_text;
}

void screen_content_open_with_class_id(const char *l2_title, const char *l1_title, int class_id)
{
    (void)l2_title;
    (void)l1_title;
    (void)class_id;
}

void screen_content_event_setup(void)
{
}

#endif /* CONFIG_LVGL_CONTENT_LIST_ENABLE */
