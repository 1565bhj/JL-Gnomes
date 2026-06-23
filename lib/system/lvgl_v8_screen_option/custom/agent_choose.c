#include "lvgl.h"
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"
#include "app_config.h"
#include <stdio.h>
#include <string.h>

#if CONFIG_LVGL_AGENT_ENABLE
#ifndef AGENT_INTRO_TTS_BUF_SZ
#define AGENT_INTRO_TTS_BUF_SZ  512
#endif

extern int http_tts_request(char *utf8_str, int utf8_str_size);
extern int http_tts_play_wait(void);
extern int aisp_mic_gain_suspend(void);
extern int aisp_mic_gain_resum(void);
extern int http_ai_intell_set_id(int id_index, void (*set_callbak)(char *buf, int len));

#define AGENT_CHOOSE_MAX_COUNT 6              // 智能体数量

// 定义位置结构
typedef struct {
    int32_t x;
    int32_t y;
} agent_choose_position_t;


// 所有智能体选择位置 实际坐标
static agent_choose_position_t agent_choose_positions[AGENT_CHOOSE_MAX_COUNT] = {
    {26, 46},   //1
    {127, 48},  //2
    {227, 48},  //3
    {28, 147},  //4
    {127, 147}, //5
    {227, 147}, //6
};

/** 与 http_ai_intell_set_id 顺序一致；TTS 须用此完整文案，勿用 label（界面可能省略号截断） */
static const char *const agent_role_canonical_names[AGENT_CHOOSE_MAX_COUNT] = {
    "语文老师",
    "数学老师",
    "英语老师",
    "体育老师",
    "历史老师",
    "科学老师",
};

/** 与 agent_click 中 http_ai_intell_set_id / Flash 记忆一致 */
static const int agent_choose_http_ids[AGENT_CHOOSE_MAX_COUNT] = {
    5061, 5062, 5058, 5057, 5053, 5060,
};

static int agent_choose_slot_from_target(lv_obj_t *target, const lv_ui *ui)
{
    if (target == ui->screen_agent_cont_1 || target == ui->screen_agent_agent_1 || target == ui->screen_agent_label_name_1) {
        return 0;
    }
    if (target == ui->screen_agent_cont_2 || target == ui->screen_agent_agent_2 || target == ui->screen_agent_label_name_2) {
        return 1;
    }
    if (target == ui->screen_agent_cont_3 || target == ui->screen_agent_agent_3 || target == ui->screen_agent_label_name_3) {
        return 2;
    }
    if (target == ui->screen_agent_cont_4 || target == ui->screen_agent_agent_4 || target == ui->screen_agent_label_name_4) {
        return 3;
    }
    if (target == ui->screen_agent_cont_5 || target == ui->screen_agent_agent_5 || target == ui->screen_agent_label_name_5) {
        return 4;
    }
    if (target == ui->screen_agent_cont_6 || target == ui->screen_agent_agent_6 || target == ui->screen_agent_label_name_6) {
        return 5;
    }
    return -1;
}

/** 先刷出选中框位置，再执行后续阻塞逻辑（网络 / TTS） */
static void agent_choose_show_selector_at(lv_ui *ui, int slot_idx)
{
    lv_obj_set_pos(ui->screen_agent_img_choose,
                   agent_choose_positions[slot_idx].x,
                   agent_choose_positions[slot_idx].y);
    lv_obj_invalidate(ui->screen_agent_img_choose);
    lv_obj_invalidate(ui->screen_agent);
    lv_refr_now(NULL);
}

/** 在线 TTS：你好，我是xxx（与 http 在线合成示例一致：suspend → request → play_wait → resum） */
static void agent_play_greeting_tts(const char *full_role_name)
{
    const char *name = (full_role_name && full_role_name[0]) ? full_role_name : "智能体";
    static char buf[AGENT_INTRO_TTS_BUF_SZ];
    int n = snprintf(buf, sizeof(buf), "你好，我是%s", name);
    if (n <= 0 || n >= (int)sizeof(buf)) {
        buf[sizeof(buf) - 1] = '\0';
    }
    aisp_mic_gain_suspend();
    if (http_tts_request(buf, (int)strlen(buf) + 1) >= 0) {
        http_tts_play_wait();
    }
    aisp_mic_gain_resum();
}

/** Guider 的 cont 继承 lv_obj 默认 SCROLLABLE/弹性滚动，易在滑动时被带偏；固定格子不滚动 */
static void agent_choose_freeze_cont(lv_obj_t *cont)
{
    if (!cont) {
        return;
    }
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(cont, LV_DIR_NONE);
}

/** 覆盖 Guider 导出默认文案（界面可省略）；TTS 用 agent_role_canonical_names */
static void agent_choose_apply_default_role_names(lv_ui *ui)
{
    lv_label_set_text(ui->screen_agent_label_name_1, agent_role_canonical_names[0]);
    lv_label_set_text(ui->screen_agent_label_name_2, agent_role_canonical_names[1]);
    lv_label_set_text(ui->screen_agent_label_name_3, agent_role_canonical_names[2]);
    lv_label_set_text(ui->screen_agent_label_name_4, agent_role_canonical_names[3]);
    lv_label_set_text(ui->screen_agent_label_name_5, agent_role_canonical_names[4]);
    lv_label_set_text(ui->screen_agent_label_name_6, agent_role_canonical_names[5]);
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
/* 当前已生效的智能体 slot；-1 表示尚未配置过，用于过滤同一智能体的重复点击 */
static int s_current_slot = -1;

/**
 * @brief 智能体选择点击事件处理函数
 */
static void agent_click_event_handler(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    int slot = agent_choose_slot_from_target(target, ui);

    if (slot < 0) {
        return;
    }

    /* 已经是当前选中的智能体：不重复设置 id、不重复写 Flash、不重复播 TTS */
    if (slot == s_current_slot) {
        printf("智能体 %d 已选中，忽略重复点击\n", slot + 1);
        return;
    }

    printf("点击了智能体 %d\n", slot + 1);
    agent_choose_show_selector_at(ui, slot);
    http_ai_intell_set_id(agent_choose_http_ids[slot], NULL);

    /* Flash 字段 agent_intell_id 是 unsigned char，直接存 http_id(4 位数)会被截断；
     * 这里只存 1-based slot 索引(1..6)，0 保留为"未记忆" */
    if ((slot + 1) != user_agent_intell_id_read()) {
        user_agent_intell_id_write(slot + 1);
    }

    s_current_slot = slot;
    agent_play_greeting_tts(agent_role_canonical_names[slot]);
}
/** 按 Flash 中已保存的智能体 slot 恢复选中框位置并同步 http_ai_intell_set_id
 *  Flash 中存的是 1-based slot 索引(1..AGENT_CHOOSE_MAX_COUNT)，0 表示未记忆 */
void agent_choose_restore_from_saved_id(lv_ui *ui)
{
    int slot = 0;
    int saved = user_agent_intell_id_read();

    if (saved >= 1 && saved <= AGENT_CHOOSE_MAX_COUNT) {
        slot = saved - 1;
    }
    agent_choose_show_selector_at(ui, slot);
    http_ai_intell_set_id(agent_choose_http_ids[slot], NULL);
    /* 把恢复出来的 slot 标记为"当前已生效"，避免随后点同一个角色又触发一次 TTS */
    s_current_slot = slot;
}
static unsigned char s_done = 0;
/** 首次进入 agent 页时从 Flash 恢复选中框并同步 http_ai_intell（全生命周期仅一次，静态 s_done） */
void agent_choose_restore_from_saved_id_first_enter_only(lv_ui *ui)
{
    if (!ui || s_done) {
        return;
    }
    s_done = 1;
    agent_choose_restore_from_saved_id(ui);
}

/**
 * @brief 初始化智能体选择屏幕的事件处理
 */
void agent_choose_event_init(lv_ui *ui)
{
    if (!ui) {
        return;
    }
    agent_choose_apply_default_role_names(ui);
    lv_obj_set_scroll_dir(ui->screen_agent, LV_DIR_NONE);


    agent_choose_freeze_cont(ui->screen_agent_cont_1);
    agent_choose_freeze_cont(ui->screen_agent_cont_2);
    agent_choose_freeze_cont(ui->screen_agent_cont_3);
    agent_choose_freeze_cont(ui->screen_agent_cont_4);
    agent_choose_freeze_cont(ui->screen_agent_cont_5);
    agent_choose_freeze_cont(ui->screen_agent_cont_6);

    // 为 cont1-6 及其内部图片和名称注册点击事件
    lv_obj_add_event_cb(ui->screen_agent_cont_1, agent_click_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_agent_agent_1, agent_click_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_agent_label_name_1, agent_click_event_handler, LV_EVENT_CLICKED, ui);

    lv_obj_add_event_cb(ui->screen_agent_cont_2, agent_click_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_agent_agent_2, agent_click_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_agent_label_name_2, agent_click_event_handler, LV_EVENT_CLICKED, ui);


    lv_obj_add_event_cb(ui->screen_agent_cont_3, agent_click_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_agent_agent_3, agent_click_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_agent_label_name_3, agent_click_event_handler, LV_EVENT_CLICKED, ui);


    lv_obj_add_event_cb(ui->screen_agent_cont_4, agent_click_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_agent_agent_4, agent_click_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_agent_label_name_4, agent_click_event_handler, LV_EVENT_CLICKED, ui);


    lv_obj_add_event_cb(ui->screen_agent_cont_5, agent_click_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_agent_agent_5, agent_click_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_agent_label_name_5, agent_click_event_handler, LV_EVENT_CLICKED, ui);

    lv_obj_add_event_cb(ui->screen_agent_cont_6, agent_click_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_agent_agent_6, agent_click_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->screen_agent_label_name_6, agent_click_event_handler, LV_EVENT_CLICKED, ui);


    // 为 return 图片注册点击事件
    lv_obj_add_event_cb(ui->screen_agent_img_return, return_click_event_handler, LV_EVENT_CLICKED, ui);
    ui_subpage_return_set_large_touch_area(ui->screen_agent_img_return, (lv_coord_t)UI_SUBPAGE_RETURN_EXT_PAD);

}

void agent_choose_event_setup(void)
{
    agent_choose_event_init(&guider_ui);
}
#endif
