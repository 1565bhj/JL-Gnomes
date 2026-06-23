#include "lvgl.h"
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"
#include "lvgl_develop.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#if CONFIG_LVGL_CALCULATOR_ENABLE

#define CALC_BUF_MAX 64
#define CALC_RESULT_MAX 32
static char calc_buf[CALC_BUF_MAX] = "";
static int calc_just_calculated = 0;  /* 1：当前显示等号结果或 Error，仅再按数字 0-9 时清空再输入 */

/* 与 setup_scr_screen_calculator 中 btnm 的 pos/size/pad 一致，用于计算“=”叠放按钮位置 */
#define CALC_BTNM_X        1
#define CALC_BTNM_Y        52
#define CALC_BTNM_W        320
#define CALC_BTNM_H        189
#define CALC_BTNM_PAD      10
#define CALC_BTNM_ROW_GAP  8
#define CALC_BTNM_COL_GAP  8
#define CALC_BTNM_COLS     4
#define CALC_BTNM_ROWS     5

static void calc_btnm_draw_part_begin_cb(lv_event_t *e);

/* 表达式解析用：当前读取位置；解析时禁止越界 */
static const char *calc_parse_p;
static const char *calc_parse_end;  /* 解析上界，避免越界 */

static void skip_spaces(void)
{
    while (calc_parse_p < calc_parse_end && (*calc_parse_p == ' ' || *calc_parse_p == '\t')) {
        calc_parse_p++;
    }
}

/* 解析一个数（含可选的小数点、可选尾部 %） */
static int parse_number(double *out)
{
    skip_spaces();
    const char *s = calc_parse_p;
    if (s >= calc_parse_end || !*s) {
        return -1;
    }
    if ((*s < '0' || *s > '9') && *s != '.' && *s != '-') {
        return -1;
    }
    if (*s == '-' && s > calc_buf) {
        char prev = *(s - 1);
        if (prev != '+' && prev != '-' && prev != '*' && prev != '/') {
            return -1;
        }
    }
    char *end;
    double val = strtod(calc_parse_p, (char **)&end);
    if (end == calc_parse_p) {
        return -1;
    }
    if ((const char *)end > calc_parse_end) {
        return -1;
    }
    calc_parse_p = end;
    skip_spaces();
    if (calc_parse_p < calc_parse_end && *calc_parse_p == '%') {
        val /= 100.0;
        calc_parse_p++;
    }
    *out = val;
    return 0;
}

#define CALC_PARSE_MAX_DEPTH  16   /* 限制递归深度，防止栈溢出 */

/* factor := 可选负号 + 数字 */
static int parse_factor(double *out, int depth)
{
    if (depth >= CALC_PARSE_MAX_DEPTH) {
        return -1;
    }
    skip_spaces();
    if (calc_parse_p < calc_parse_end && *calc_parse_p == '-') {
        calc_parse_p++;
        if (parse_factor(out, depth + 1) != 0) {
            return -1;
        }
        *out = -(*out);
        return 0;
    }
    return parse_number(out);
}

/* term := factor ( *|/ factor )* */
static int parse_term(double *out)
{
    if (parse_factor(out, 0) != 0) {
        return -1;
    }
    for (;;) {
        skip_spaces();
        if (calc_parse_p >= calc_parse_end) {
            break;
        }
        char op = *calc_parse_p;
        if (op != '*' && op != '/') {
            break;
        }
        calc_parse_p++;
        double right;
        if (parse_factor(&right, 0) != 0) {
            return -1;
        }
        if (op == '*') {
            *out *= right;
        } else {
            if (fabs(right) < 1e-15) {
                return -1;
            }
            *out /= right;
        }
    }
    return 0;
}

/* expr := term ( +|- term )* */
static int parse_expr(double *out)
{
    if (parse_term(out) != 0) {
        return -1;
    }
    for (;;) {
        skip_spaces();
        if (calc_parse_p >= calc_parse_end) {
            break;
        }
        char op = *calc_parse_p;
        if (op != '+' && op != '-') {
            break;
        }
        calc_parse_p++;
        double right;
        if (parse_term(&right) != 0) {
            return -1;
        }
        if (op == '+') {
            *out += right;
        } else {
            *out -= right;
        }
    }
    return 0;
}

/**
 * @brief 执行“=”按键逻辑：解析 calc_buf 并计算结果后写回
 */
static void do_equals_action(lv_ui *ui)
{
    if (!ui || !ui->screen_calculator_label_calculate) {
        return;
    }
    static char result_buf[CALC_RESULT_MAX];
    calc_just_calculated = 0;
    if (!calc_buf[0]) {
        lv_label_set_text(ui->screen_calculator_label_calculate, "0");
        return;
    }
    calc_parse_p = calc_buf;
    calc_parse_end = calc_buf + strlen(calc_buf);  /* 解析不越界 */
    if (calc_parse_end > calc_buf + CALC_BUF_MAX) {
        calc_parse_end = calc_buf + CALC_BUF_MAX;
    }
    double result;
    if (parse_expr(&result) != 0) {
        calc_just_calculated = 1;
        lv_label_set_text(ui->screen_calculator_label_calculate, "Error");
        return;
    }
    skip_spaces();
    if (*calc_parse_p != '\0') {
        calc_just_calculated = 1;
        lv_label_set_text(ui->screen_calculator_label_calculate, "Error");
        return;
    }
    if (isnan(result) || isinf(result)) {
        calc_just_calculated = 1;
        lv_label_set_text(ui->screen_calculator_label_calculate, "Error");
        return;
    }
    if (fabs(result - (double)(long long)result) < 1e-12 &&
        (long long)result >= -9223372036854775807LL && (long long)result <= 9223372036854775807LL) {
        (void)snprintf(result_buf, sizeof(result_buf), "%lld", (long long)result);
    } else {
        (void)snprintf(result_buf, sizeof(result_buf), "%.10g", result);
    }
    result_buf[CALC_RESULT_MAX - 1] = '\0';
    /* 结果写回输入缓冲，长度受 CALC_BUF_MAX 保护，不依赖 strnlen */
    {
        size_t rlen = 0;
        while (rlen < CALC_RESULT_MAX - 1 && result_buf[rlen] != '\0') {
            rlen++;
        }
        if (rlen > CALC_BUF_MAX - 1) {
            rlen = CALC_BUF_MAX - 1;
        }
        memcpy(calc_buf, result_buf, rlen);
        calc_buf[rlen] = '\0';
    }
    lv_label_set_text(ui->screen_calculator_label_calculate, calc_buf);
    calc_just_calculated = 1;
}

/**
 * @brief “=”叠放按钮的点击回调
 */
static void calc_equals_btn_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (ui) {
        do_equals_action(ui);
    }
}

/**
 * @brief 将“=”改为占两行：右侧两格隐藏，叠放一个跨两行高度的独立按钮
 */
void apply_calculator_btnm_merged_equals(lv_ui *ui)
{
    /* 右侧一列两格都隐藏，留出两行一列的空位 */
    static const char *calc_btnm_map[] = {
        "C", "÷", "×", "←", "\n",
        "7", "8", "9", "－", "\n",
        "4", "5", "6", "+", "\n",
        "1", "2", "3", " ", "\n",
        "%", "0", ".", " ", ""
    };
    static const lv_btnmatrix_ctrl_t ctrl_map[] = {
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, LV_BTNMATRIX_CTRL_HIDDEN,
        0, 0, 0, LV_BTNMATRIX_CTRL_HIDDEN
    };
    lv_btnmatrix_set_map(ui->screen_calculator_btnm_calculate, calc_btnm_map);
    lv_btnmatrix_set_ctrl_map(ui->screen_calculator_btnm_calculate, ctrl_map);

    /* 数字白色、符号蓝色：默认白色，绘制时由回调按按键改符号为蓝色 */
    lv_obj_set_style_text_color(ui->screen_calculator_btnm_calculate, lv_color_hex(0xffffff), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(ui->screen_calculator_btnm_calculate, calc_btnm_draw_part_begin_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);

    /* 计算右侧一列、下两行的区域，叠放“=”按钮（占两行高度） */
    lv_coord_t inner_w = CALC_BTNM_W - 2 * CALC_BTNM_PAD;
    lv_coord_t inner_h = CALC_BTNM_H - 2 * CALC_BTNM_PAD;
    lv_coord_t col_w = (inner_w - (CALC_BTNM_COLS - 1) * CALC_BTNM_COL_GAP) / CALC_BTNM_COLS;
    lv_coord_t row_h = (inner_h - (CALC_BTNM_ROWS - 1) * CALC_BTNM_ROW_GAP) / CALC_BTNM_ROWS;
    lv_coord_t eq_x = CALC_BTNM_X + CALC_BTNM_PAD + (CALC_BTNM_COLS - 1) * (col_w + CALC_BTNM_COL_GAP);
    lv_coord_t eq_y = CALC_BTNM_Y + CALC_BTNM_PAD + (CALC_BTNM_ROWS - 2) * (row_h + CALC_BTNM_ROW_GAP);
    lv_coord_t eq_w = col_w;
    lv_coord_t eq_h = 2 * row_h + CALC_BTNM_ROW_GAP;

    lv_obj_t *eq_btn = lv_btn_create(ui->screen_calculator);
    lv_obj_set_pos(eq_btn, eq_x, eq_y);
    lv_obj_set_size(eq_btn, eq_w, eq_h);
    lv_obj_set_style_radius(eq_btn, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(eq_btn, lv_color_hex(0xffa100), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(eq_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(eq_btn, lv_color_hex(0xc9c9c9), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(eq_btn, calc_equals_btn_event_cb, LV_EVENT_CLICKED, ui);

    lv_obj_t *eq_label = lv_label_create(eq_btn);
    lv_label_set_text(eq_label, "=");
    lv_obj_set_style_text_color(eq_label, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(eq_label, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(eq_label);
}

/**
 * @brief 绘制时按按键区分颜色：数字和.保持白色，符号(C÷×←－+)用蓝色
 */
static void calc_btnm_draw_part_begin_cb(lv_event_t *e)
{
    lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);
    if (!lv_obj_draw_part_check_type(dsc, &lv_btnmatrix_class, LV_BTNMATRIX_DRAW_PART_BTN)) {
        return;
    }
    if (!dsc->label_dsc) {
        return;
    }
    lv_obj_t *obj = lv_event_get_target(e);
    const char *txt = lv_btnmatrix_get_btn_text(obj, (uint16_t)dsc->id);
    if (!txt || !txt[0]) {
        return;
    }
    /* 数字 0-9 和 . 保持白色，其余符号用蓝色 */
    unsigned char c = (unsigned char)txt[0];
    if ((c >= '0' && c <= '9') || c == '.' || c == '%') {
        dsc->label_dsc->color = lv_color_hex(0xffffff);
    } else {
        dsc->label_dsc->color = lv_color_hex(0xffa100);
    }
}

/**
 * @brief 计算器按键矩阵事件处理
 */
static void calculate_btnm_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_VALUE_CHANGED) {
        return;
    }

    lv_obj_t *btnm = lv_event_get_target(e);
    uint16_t btn_id = lv_btnmatrix_get_selected_btn(btnm);
    if (btn_id == LV_BTNMATRIX_BTN_NONE) {
        return;
    }

    const char *txt = lv_btnmatrix_get_btn_text(btnm, btn_id);
    if (!txt || txt[0] == '\0') {
        return;
    }

    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui || !ui->screen_calculator_label_calculate) {
        return;
    }

    /* 等号（或 Error）之后只有按数字键 0-9 才清空再输入，按符号键不清空 */
    if (calc_just_calculated) {
        calc_just_calculated = 0;
        if (txt[0] >= '0' && txt[0] <= '9') {
            calc_buf[0] = '\0';
        }
    }

    size_t len = strlen(calc_buf);

    if (strcmp(txt, "C") == 0) {
        calc_buf[0] = '\0';
    } else if (strcmp(txt, "←") == 0) {
        if (len > 0) {
            calc_buf[len - 1] = '\0';
        }
    } else if (len < CALC_BUF_MAX - 1) {
        if (strcmp(txt, "×") == 0) {
            calc_buf[len++] = '*';
        } else if (strcmp(txt, "÷") == 0) {
            calc_buf[len++] = '/';
        } else if (strcmp(txt, "－") == 0) {
            calc_buf[len++] = '-';
        } else {
            calc_buf[len++] = txt[0];
        }
        calc_buf[len] = '\0';
    }

    lv_label_set_text(ui->screen_calculator_label_calculate, calc_buf[0] ? calc_buf : "0");
}

/**
 * @brief return 图片点击事件处理函数
 */
static void return_click_event_handler(lv_event_t *e)
{
    printf("点击了返回按钮\n");
    // 这里添加返回按钮的点击处理逻辑
    switch_to_main_page();
}
/**
 * @brief 初始化计算器屏幕的事件处理
 */
void calculate_event_init(lv_ui *ui)
{
    if (!ui) {
        return;
    }
    lv_obj_set_scroll_dir(ui->screen_calculator, LV_DIR_NONE);
    lv_obj_set_scroll_dir(ui->screen_calculator_btnm_calculate, LV_DIR_NONE);

    apply_calculator_btnm_merged_equals(ui);
    lv_obj_add_event_cb(ui->screen_calculator_img_return, return_click_event_handler, LV_EVENT_CLICKED, ui);
    ui_subpage_return_set_large_touch_area(ui->screen_calculator_img_return, (lv_coord_t)UI_SUBPAGE_RETURN_EXT_PAD - 30);
    lv_obj_add_event_cb(ui->screen_calculator_btnm_calculate, calculate_btnm_event_handler, LV_EVENT_ALL, ui);
}

void calculate_event_setup(void)
{
    calculate_event_init(&guider_ui);
}

#else /* !CONFIG_LVGL_CALCULATOR_ENABLE */

void calculate_event_init(lv_ui *ui)
{
    (void)ui;
}

void calculate_event_setup(void)
{
}

#endif /* CONFIG_LVGL_CALCULATOR_ENABLE */
