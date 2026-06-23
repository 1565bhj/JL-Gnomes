#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdbool.h"
#include <stdint.h>
#include "gui_guider.h"
#include "custom.h"

#if CONFIG_LVGL_CONTENT_LIST_ENABLE

/* ========== 公共年级数组（多模式复用） ========== */
static const char *grades_1_to_9[] = {
    "一年级", "二年级", "三年级", "四年级", "五年级", "六年级", "七年级", "八年级", "九年级"
};
static const char *grades_1_to_6[] = {
    "一年级", "二年级", "三年级", "四年级", "五年级", "六年级"
};
static const char *grades_7_to_9[] = {
    "七年级", "八年级", "九年级"
};

/* ========== 列表0内容 ========== */
#define LIST_CONTENT_L0_COUNT    5
static const char *list_content_l0[] = {
    "英语园地", "语文园地", "优秀作文", "教学问答", "全科答疑"
};


#define ENGLISH_L1_COUNT    2
#define CHINESE_L1_COUNT    6
#define ESSAY_L1_COUNT      7
#define TEACH_L1_COUNT      8
#define ALLQA_L1_COUNT      12

#define PINYIN_L2_COUNT  6
#define TANGSHI_L2_COUNT  7
#define SONGCI_L2_COUNT  10
#define MASTERWORK_L2_COUNT  6

#ifdef ENGLISH_L1_COUNT
/* ========== 英语园地 (COL_ENGLISH_MODE) ========== */
static const char *english_l1[] = {
    "英语单词", "英语句子"
};
#endif

#ifdef CHINESE_L1_COUNT
/* ========== 语文园地 (COL_LIVES_MODE) ========== */
static const char *chinese_l1[] = {
    "拼音", "古诗", "文言文", "国学经典", "唐诗三百首", "宋词三百首"
};
#endif

#ifdef ESSAY_L1_COUNT
/* ========== 优秀作文 (COL_IDIOM_MODE) ========== */
static const char *essay_l1[] = {
    "说明文", "记叙文", "议论文", "写人作文", "写物作文", "想象作文", "记事作文"
};
#endif

#ifdef TEACH_L1_COUNT
/* ========== 教学问答 (COL_TAECH_MODE) ========== */
static const char *teach_l1[] = {
    "语文", "数学", "英语", "科学", "劳动",  "信息技术", "道德与法治", "体育与健康"
};
#endif

#ifdef ALLQA_L1_COUNT
/* ========== 全科答疑 (COL_HOTTOPIC_MODE) ========== */
static const char *allqa_l1[] = {
    "语文", "数学", "英语", "科学", "物理", "化学", "历史", "生物", "地理", "音乐", "体育", "道德与法治"
};
#endif

/* ========== 语文园地二级子项（与 ui_action 中 COL_LIVES_MODE 一致） ========== */
static const char *pinyin_l2[] = {
    "单韵母", "复韵母", "前鼻韵母", "后鼻韵母", "声母", "整体认读"
};

static const char *tangshi_l2[] = {
    "五言绝句", "七言绝句", "五言律诗", "七言律诗", "五言古诗", "七言古诗", "乐府"
};

static const char *songci_l2[] = {
    "卷一唐五代词", "卷二唐五代词", "卷三北宋词", "卷四北宋词", "卷五北宋词",
    "卷六北宋词", "卷七南宋词", "卷八南宋词", "卷九南宋词", "卷十南宋词"
};

static const char *masterwork_l2[] = {
    "论语", "三字经", "弟子规", "百家姓", "千字文", "增广贤文"
};

/* ========== class_id 映射（与参考 ui_action_dudu_col_list_scr 服务器 API id 一致） ========== */
/* 教学问答：teach_l1[] 顺序为 语数英科劳信息道法体健，与参考表科目顺序不同，按项对齐基址 */
static const int teach_base_ids[TEACH_L1_COUNT] = {5, 14, 23, 41, 59, 68, 32, 50};
static const int english_base_ids[ENGLISH_L1_COUNT] = {92, 98};
/* 语文园地：chinese_l1 为 拼音/古诗/文言文/国学/唐诗/宋词，与参考 lives 显示顺序不同 */
static const int lives_base_ids[CHINESE_L1_COUNT] = {104, 110, 119, 216, 128, 135};
/* 优秀作文：essay_l1 为 说明/记叙/议论/写人/写物/想象/记事 */
static const int idiom_base_ids[ESSAY_L1_COUNT] = {169, 172, 175, 145, 151, 157, 163};
/* 全科答疑：allqa_l1 顺序相对参考将「道法」放在最后 */
static const int hottopic_base_ids[ALLQA_L1_COUNT] = {
    178, 182, 186, 190, 194, 198, 199, 204, 205, 207, 208, 203
};

static int screen_list_leaf_class_id(int l0, int l1, int l2)
{
    if (l0 < 0 || l1 < 0 || l2 < 0) {
        return -1;
    }
    switch (l0) {
    case 0:
        if (l1 < ENGLISH_L1_COUNT) {
            return english_base_ids[l1] + l2;
        }
        break;
    case 1:
        if (l1 < CHINESE_L1_COUNT) {
            return lives_base_ids[l1] + l2;
        }
        break;
    case 2:
        if (l1 < ESSAY_L1_COUNT) {
            return idiom_base_ids[l1] + l2;
        }
        break;
    case 3:
        if (l1 < TEACH_L1_COUNT) {
            return teach_base_ids[l1] + l2;
        }
        break;
    case 4:
        if (l1 < ALLQA_L1_COUNT) {
            return hottopic_base_ids[l1] + l2;
        }
        break;
    default:
        break;
    }
    return -1;
}

/* ========== 全科答疑：科目下的细分类（与参考 get_level1_data HOTTOPIC 一致） ========== */
static const char *qa_chinese_l2[]   = {"语句类", "修辞类", "阅读类", "写作类"};
static const char *qa_math_l2[]      = {"代数", "几何", "概率", "应用"};
static const char *qa_english_l2[]   = {"词类", "简单句", "复合句", "名言"};
static const char *qa_science_l2[]   = {"生命科学", "物质科学", "地球宇宙", "实践研究"};
static const char *qa_physics_l2[]   = {"声学", "热学", "力学", "光学"};
static const char *qa_chemistry_l2[] = {"基础"};
static const char *qa_history_l2[]   = {"古代史", "中世纪史", "近代史", "现代史"};
static const char *qa_morals_l2[]    = {"基础"};
static const char *qa_biology_l2[]   = {"基础"};
static const char *qa_geography_l2[] = {"自然", "人文"};
static const char *qa_music_l2[]     = {"基础"};
static const char *qa_sports_l2[]    = {"通用"};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define SCREEN_LIST_SUB_SLOT_MAX    6
/* 比最大 L1/L2 条目数多留余量（全科答疑 L1=12，宋词卷=10） */
#define SCREEN_LIST_EXTRA_LABEL_MAX (ALLQA_L1_COUNT)

#define SUB_LABEL_COL0_X    15
#define SUB_LABEL_COL1_X    144
#define SUB_LABEL_Y0        11
#define SUB_LABEL_ROW_DY    64
#define SUB_LABEL_W         117
#define SUB_LABEL_H         51
/* LVGL：纵向滚动条 x 由 LV_PART_SCROLLBAR 的 pad_right 决定，减小则条带向右移 */
#define CONT2_SCROLLBAR_PAD_RIGHT_DELTA    12

static lv_obj_t *screen_list_extra_labels[SCREEN_LIST_EXTRA_LABEL_MAX];
static uint8_t screen_list_extra_n;
static bool screen_list_cont2_sb_pad_captured;
static lv_coord_t screen_list_cont2_sb_pad_right_base;

/* 与参考 ui_action_dudu_col_list_scr：一级选分类 → 二级列表；在二级再点 → 三级列表 */
static int screen_list_l0_sel = -1;
static int screen_list_l1_sel = -1;
/* 0=仅主界面 1=显示 L1 子类 2=显示 L2 细项 */
static uint8_t screen_list_sub_depth;

static void screen_list_show_sub_items(lv_ui *ui, const char *title, const char **items,
                                       uint32_t count, uint8_t sub_depth);
static void screen_list_get_l2_data(int l0, int l1, const char ***out_items, uint32_t *out_count);
static void screen_list_sub_label_click_cb(lv_event_t *e);

static void screen_list_clear_extra_labels(void)
{
    for (uint8_t i = 0; i < screen_list_extra_n; i++) {
        if (screen_list_extra_labels[i]) {
            lv_obj_del(screen_list_extra_labels[i]);
            screen_list_extra_labels[i] = NULL;
        }
    }
    screen_list_extra_n = 0;
}

static void screen_list_get_l1_data(int l0, const char ***items, uint32_t *count)
{
    switch (l0) {
    case 0:
        *items = (const char **)english_l1;
        *count = ENGLISH_L1_COUNT;
        break;
    case 1:
        *items = (const char **)chinese_l1;
        *count = CHINESE_L1_COUNT;
        break;
    case 2:
        *items = (const char **)essay_l1;
        *count = ESSAY_L1_COUNT;
        break;
    case 3:
        *items = (const char **)teach_l1;
        *count = TEACH_L1_COUNT;
        break;
    case 4:
        *items = (const char **)allqa_l1;
        *count = ALLQA_L1_COUNT;
        break;
    default:
        *items = NULL;
        *count = 0;
        break;
    }
}

/* 对应参考文件中 get_level1_data：由一级分类 + 二级选中项决定第三级列表数据 */
static void screen_list_get_l2_data(int l0, int l1, const char ***out_items, uint32_t *out_count)
{
    *out_items = NULL;
    *out_count = 0;
    switch (l0) {
    case 0: /* 英语园地 */
        *out_items = (const char **)grades_1_to_6;
        *out_count = 6;
        break;
    case 1: /* 语文园地：顺序与 chinese_l1 一致 */
        switch (l1) {
        case 0:
            *out_items = (const char **)pinyin_l2;
            *out_count = PINYIN_L2_COUNT;
            break;
        case 1:
        case 2:
            *out_items = (const char **)grades_1_to_9;
            *out_count = 9;
            break;
        case 3:
            *out_items = (const char **)masterwork_l2;
            *out_count = MASTERWORK_L2_COUNT;
            break;
        case 4:
            *out_items = (const char **)tangshi_l2;
            *out_count = TANGSHI_L2_COUNT;
            break;
        case 5:
            *out_items = (const char **)songci_l2;
            *out_count = SONGCI_L2_COUNT;
            break;
        default:
            break;
        }
        break;
    case 2: /* 优秀作文：前三项为初中年级，其余为小学年级 */
        if (l1 >= 0 && l1 <= 2) {
            *out_items = (const char **)grades_7_to_9;
            *out_count = 3;
        } else {
            *out_items = (const char **)grades_1_to_6;
            *out_count = 6;
        }
        break;
    case 3: /* 教学问答 */
        *out_items = (const char **)grades_1_to_9;
        *out_count = 9;
        break;
    case 4: /* 全科答疑 */
        switch (l1) {
        case 0:
            *out_items = (const char **)qa_chinese_l2;
            *out_count = 4;
            break;
        case 1:
            *out_items = (const char **)qa_math_l2;
            *out_count = 4;
            break;
        case 2:
            *out_items = (const char **)qa_english_l2;
            *out_count = 4;
            break;
        case 3:
            *out_items = (const char **)qa_science_l2;
            *out_count = 4;
            break;
        case 4:
            *out_items = (const char **)qa_physics_l2;
            *out_count = 4;
            break;
        case 5:
            *out_items = (const char **)qa_chemistry_l2;
            *out_count = 1;
            break;
        case 6:
            *out_items = (const char **)qa_history_l2;
            *out_count = 4;
            break;
        case 7:
            *out_items = (const char **)qa_biology_l2;
            *out_count = 1;
            break;
        case 8:
            *out_items = (const char **)qa_geography_l2;
            *out_count = 2;
            break;
        case 9:
            *out_items = (const char **)qa_music_l2;
            *out_count = 1;
            break;
        case 10:
            *out_items = (const char **)qa_sports_l2;
            *out_count = 1;
            break;
        case 11:
            *out_items = (const char **)qa_morals_l2;
            *out_count = 1;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

static void screen_list_style_sub_label_like(lv_obj_t *lbl)
{
    lv_obj_set_size(lbl, SUB_LABEL_W, SUB_LABEL_H);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_border_width(lbl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(lbl, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(lbl, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(lbl, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(lbl, 66, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(lbl, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(lbl, 14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(lbl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(lbl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(lbl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(lbl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void screen_list_show_sub_items(lv_ui *ui, const char *title, const char **items,
                                       uint32_t count, uint8_t sub_depth)
{
    lv_obj_t *slots[SCREEN_LIST_SUB_SLOT_MAX] = {
        ui->screen_list_label_1,
        ui->screen_list_label_2,
        ui->screen_list_label_3,
        ui->screen_list_label_4,
        ui->screen_list_label_5,
        ui->screen_list_label_6,
    };

    if (!ui || !items || count == 0) {
        return;
    }

    lv_label_set_text(ui->screen_list_label_name, title);
    screen_list_sub_depth = sub_depth;

    lv_obj_add_flag(ui->screen_list_cont_content1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui->screen_list_cont_content2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(ui->screen_list_cont_content2, 25, 40);
    lv_obj_set_size(ui->screen_list_cont_content2, 270, 195);
    lv_obj_set_scroll_dir(ui->screen_list_cont_content2, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(ui->screen_list_cont_content2, LV_SCROLLBAR_MODE_AUTO);

    if (!screen_list_cont2_sb_pad_captured) {
        screen_list_cont2_sb_pad_right_base =
            lv_obj_get_style_pad_right(ui->screen_list_cont_content2, LV_PART_SCROLLBAR);
        screen_list_cont2_sb_pad_captured = true;
    }
    {
        lv_coord_t pr = screen_list_cont2_sb_pad_right_base - CONT2_SCROLLBAR_PAD_RIGHT_DELTA;
        if (pr < 0) {
            pr = 0;
        }
        lv_obj_set_style_pad_right(ui->screen_list_cont_content2, pr,
                                   LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    }

    screen_list_clear_extra_labels();

    for (uint32_t i = 0; i < count; i++) {
        int32_t x = (i % 2u == 0u) ? SUB_LABEL_COL0_X : SUB_LABEL_COL1_X;
        int32_t y = SUB_LABEL_Y0 + (int32_t)(i / 2u) * SUB_LABEL_ROW_DY;
        lv_obj_t *lbl;

        if (i < SCREEN_LIST_SUB_SLOT_MAX) {
            lbl = slots[i];
            lv_obj_remove_event_cb(lbl, screen_list_sub_label_click_cb);
            lv_label_set_text(lbl, items[i]);
            lv_obj_set_pos(lbl, x, y);
            lv_obj_clear_flag(lbl, LV_OBJ_FLAG_HIDDEN);
        } else {
            if (screen_list_extra_n >= SCREEN_LIST_EXTRA_LABEL_MAX) {
                break;
            }
            lbl = lv_label_create(ui->screen_list_cont_content2);
            lv_label_set_text(lbl, items[i]);
            screen_list_style_sub_label_like(lbl);
            lv_obj_set_pos(lbl, x, y);
            screen_list_extra_labels[screen_list_extra_n++] = lbl;
        }

        lv_obj_add_flag(lbl, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(lbl, screen_list_sub_label_click_cb, LV_EVENT_CLICKED,
                            (void *)(intptr_t)i);
    }

    for (uint32_t i = count; i < SCREEN_LIST_SUB_SLOT_MAX; i++) {
        lv_obj_remove_event_cb(slots[i], screen_list_sub_label_click_cb);
        lv_obj_add_flag(slots[i], LV_OBJ_FLAG_HIDDEN);
    }

    lv_obj_update_layout(ui->screen_list_cont_content2);
}

static void screen_list_apply_l0_selection(lv_ui *ui, int l0_index)
{
    const char **items;
    uint32_t count;

    if (!ui || l0_index < 0 || l0_index >= LIST_CONTENT_L0_COUNT) {
        return;
    }

    screen_list_l0_sel = l0_index;
    screen_list_l1_sel = -1;
    screen_list_get_l1_data(l0_index, &items, &count);
    if (!items || count == 0) {
        return;
    }

    screen_list_show_sub_items(ui, list_content_l0[l0_index], items, count, 1);
}

static void screen_list_drill_to_l2(lv_ui *ui, int l1_index)
{
    const char **l2_items;
    uint32_t l2_count;
    const char **l1_items;
    uint32_t l1_count;

    if (!ui || screen_list_l0_sel < 0) {
        return;
    }

    screen_list_get_l1_data(screen_list_l0_sel, &l1_items, &l1_count);
    if (!l1_items || l1_index < 0 || (uint32_t)l1_index >= l1_count) {
        return;
    }

    screen_list_get_l2_data(screen_list_l0_sel, l1_index, &l2_items, &l2_count);
    if (!l2_items || l2_count == 0) {
        return;
    }

    screen_list_l1_sel = l1_index;
    screen_list_show_sub_items(ui, l1_items[l1_index], l2_items, l2_count, 2);
}

static void screen_list_sub_label_click_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    int idx = (int)(intptr_t)lv_event_get_user_data(e);
    lv_ui *ui = &guider_ui;

    if (screen_list_sub_depth == 1) {
        screen_list_drill_to_l2(ui, idx);
    } else if (screen_list_sub_depth == 2) {
        lv_obj_t *t = lv_event_get_target(e);
        const char *name = lv_label_get_text(t);
        const char **l1_items;
        uint32_t l1_count;
        const char *l1_name = "";

        screen_list_get_l1_data(screen_list_l0_sel, &l1_items, &l1_count);
        if (l1_items && screen_list_l1_sel >= 0 && (uint32_t)screen_list_l1_sel < l1_count) {
            l1_name = l1_items[screen_list_l1_sel];
        }
        int cid = screen_list_leaf_class_id(screen_list_l0_sel, screen_list_l1_sel, idx);
        (void)printf("[screen_list] leaf l0=%d l1=%d l2=%d class_id=%d text=%s l1=%s\n",
                     screen_list_l0_sel, screen_list_l1_sel, idx, cid, name, l1_name);
        screen_content_open_with_class_id(name, l1_name, cid);
    }
}

static void screen_list_l0_img_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    int idx = (int)(intptr_t)lv_event_get_user_data(e);
    screen_list_apply_l0_selection(&guider_ui, idx);
}

static void screen_list_return_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    lv_ui *ui = &guider_ui;

    if (screen_list_sub_depth == 2 && screen_list_l0_sel >= 0) {
        /* 参考 col_list：在二级列表时返回先回到一级子列表 */
        screen_list_l1_sel = -1;
        screen_list_apply_l0_selection(ui, screen_list_l0_sel);
        return;
    }

    if (screen_list_sub_depth == 1) {
        /* 一级子类列表 → 回到 L0 五个入口 */
        screen_list_sub_depth = 0;
        screen_list_l0_sel = -1;
        screen_list_l1_sel = -1;
        screen_list_clear_extra_labels();
        lv_obj_clear_flag(ui->screen_list_cont_content1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui->screen_list_cont_content2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(ui->screen_list_cont_content2, 405, 47);
        lv_label_set_text(ui->screen_list_label_name, "分类");
        return;
    }

    /* screen_list_sub_depth == 0：L0（LIST_CONTENT_L0_COUNT）入口页 → 选择页 */
    screen_list_l0_sel = -1;
    screen_list_l1_sel = -1;
    screen_list_clear_extra_labels();
    switch_to_main_page();
}

// 初始化事件
void screen_list_event_init(lv_ui *ui)
{
    if (!ui) {
        return;
    }
    lv_obj_set_scroll_dir(ui->screen_list, LV_DIR_NONE);
    lv_obj_set_scroll_dir(ui->screen_list_cont_content1, LV_DIR_VER);

    lv_obj_add_flag(ui->screen_list_cont_content2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(ui->screen_list_cont_content2, 405, 47);

    lv_obj_add_event_cb(ui->screen_list_study_english, screen_list_l0_img_event_cb,
                        LV_EVENT_CLICKED, (void *)(intptr_t)0);
    lv_obj_add_event_cb(ui->screen_list_chinese_classics, screen_list_l0_img_event_cb,
                        LV_EVENT_CLICKED, (void *)(intptr_t)1);
    lv_obj_add_event_cb(ui->screen_list_excellent_essay, screen_list_l0_img_event_cb,
                        LV_EVENT_CLICKED, (void *)(intptr_t)2);
    lv_obj_add_event_cb(ui->screen_list_study_QA, screen_list_l0_img_event_cb,
                        LV_EVENT_CLICKED, (void *)(intptr_t)3);
    lv_obj_add_event_cb(ui->screen_list_general_QA, screen_list_l0_img_event_cb,
                        LV_EVENT_CLICKED, (void *)(intptr_t)4);

    lv_obj_add_event_cb(ui->screen_list_img_return, screen_list_return_event_cb, LV_EVENT_CLICKED, NULL);
    ui_subpage_return_set_large_touch_area(ui->screen_list_img_return, (lv_coord_t)UI_SUBPAGE_RETURN_EXT_PAD);
}



void screen_list_event_setup(void)
{
    screen_list_event_init(&guider_ui);
}

#else /* !CONFIG_LVGL_CONTENT_LIST_ENABLE */

void screen_list_event_setup(void)
{
}

#endif /* CONFIG_LVGL_CONTENT_LIST_ENABLE */
