#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdbool.h"
#include "gui_guider.h"
#include "custom.h"

// 当前播放模式ID
extern int get_current_play_mode_id(void);
extern void set_current_play_mode_id(int mode_id);

// 音乐信息结构体
typedef struct {
    char title[64];
    char artist[64];
    char genre[64];
    uint32_t current_time;
    uint32_t last_time;
    uint32_t total_time;
    bool title_changed;
    bool artist_changed;
    bool genre_changed;
    bool current_time_changed;
    bool total_time_changed;
} music_info_t;


// 使用静态变量避免频繁的内存分配
static music_info_t music_info = {
    .title = "歌曲名未更新",
    .artist = "歌手名未更新",
    .genre = "歌词未更新",
    .current_time = 0,
    .total_time = 4 * 60 + 14, // 初始时间值 (4分14秒)
    .title_changed = false,
    .artist_changed = false,
    .genre_changed = false,
    .current_time_changed = false,
    .total_time_changed = false
};



// 定义固定缓冲区
#define LYRIC_BUFFER_SIZE 256
static char lyric_buffer[LYRIC_BUFFER_SIZE];
// 更新标志，用于批量更新
static volatile bool need_ui_update = false;
static uint16_t update_timer_id = 0;

void lv_demo_handle_music_pause(void);
void lv_demo_handle_music_play(void);
void lv_demo_handle_music_prev(void);
void lv_demo_handle_music_next(void);
void lv_demo_handle_music_mode_switch(void);

// 简单的UI更新函数，直接在适当的线程中调用
static void direct_ui_update(void)
{
    // 检查音乐页面根对象是否有效
    if (!guider_ui.screen_music) {
        return;
    }
    printf(" direct_ui_update简单的UI更新函数，直接在适当的线程中调用\n");
    // 更新标题
    if (music_info.title_changed && guider_ui.screen_music_span_title_span) {
        lv_span_set_text(guider_ui.screen_music_span_title_span, music_info.title);
        music_info.title_changed = false;
    }

    // 更新艺术家
    if (music_info.artist_changed && guider_ui.screen_music_span_singer_span) {
        lv_span_set_text(guider_ui.screen_music_span_singer_span, music_info.artist);
        music_info.artist_changed = false;
    }

    // 更新歌词/专辑
    if (music_info.genre_changed && guider_ui.screen_music_lyric) {
        // 在歌词前添加4个空格
        snprintf(lyric_buffer, LYRIC_BUFFER_SIZE, "%s", music_info.genre);
        lyric_buffer[LYRIC_BUFFER_SIZE - 1] = '\0';

        lv_label_set_text(guider_ui.screen_music_lyric, lyric_buffer);
        music_info.genre_changed = false;

        // 关键设置：设置为水平循环滚动模式
        lv_label_set_long_mode(guider_ui.screen_music_lyric, LV_LABEL_LONG_SCROLL_CIRCULAR);

        // 确保文本居中对齐，但需要配合标签高度限制以避免垂直滚动
        lv_obj_set_style_text_align(guider_ui.screen_music_lyric, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

        // 确保标签足够高以显示一行文本，避免垂直滚动
        lv_obj_set_height(guider_ui.screen_music_lyric, lv_font_get_line_height(lv_obj_get_style_text_font(guider_ui.screen_music_lyric, LV_PART_MAIN)) + 4);

        // 设置滚动动画时间（控制滚动速度）
        lv_obj_set_style_anim_time(guider_ui.screen_music_lyric, 3000, LV_PART_MAIN); // 3秒完成一次循环

        // 刷新对象以确保设置生效
        lv_obj_update_layout(guider_ui.screen_music_lyric);
    }
     if (music_info.total_time_changed && guider_ui.screen_music_span_end_span) {
        char time_str[16];
        uint32_t min = music_info.total_time / 60;
        uint32_t sec = music_info.total_time % 60;
        lv_snprintf(time_str, sizeof(time_str), "%d:%02d", min, sec);
        lv_span_set_text(guider_ui.screen_music_span_end_span, time_str);

        // 更新进度条范围
        lv_bar_set_range(guider_ui.screen_music_slider, 0, 100);
        music_info.total_time_changed = false;
    }

    // 更新当前播放时间
    if (music_info.current_time_changed && guider_ui.screen_music_span_start_span) {
        char time_str[16];
        uint32_t min = music_info.current_time / 60;
        uint32_t sec = music_info.current_time % 60;
        lv_snprintf(time_str, sizeof(time_str), "%d:%02d", min, sec);
        lv_span_set_text(guider_ui.screen_music_span_start_span, time_str);

        // 更新进度条位置
        if (music_info.total_time > 0) {
            uint32_t progress = (music_info.current_time * 100) / music_info.total_time;
            if (progress > 100) progress = 100;
            lv_bar_set_value(guider_ui.screen_music_slider, progress, LV_ANIM_OFF);
        }
        music_info.current_time_changed = false;
    }
}


// 定时器回调，用于批量更新UI
static void ui_update_timer_cb(void *param)
{
    (void)param;
    update_timer_id = 0;

    if (need_ui_update) {
        need_ui_update = false;
        direct_ui_update();
    }

}

// 请求UI更新（防抖处理）
static void request_ui_update(void)
{
    need_ui_update = true;

    // 如果定时器已经运行，不要重复启动
    if (update_timer_id == 0) {
        // 延迟50ms更新，避免频繁更新
        //update_timer_id = sys_timeout_add_to_task("sys_timer", NULL, ui_update_timer_cb, 50);
        lv_async_call(ui_update_timer_cb, NULL);
    }
}
static char original_title[128] = {0};
static bool original_title_initialized = false; // 添加初始化标志

#ifdef APPLAYER_ENABLE
#else
int put_buf(const char *str, int len){

}
#endif  
// 更新标题，歌曲名
void music_update_title(const char *title)
{
    if (!title) {
        return;
    }

    // 检查内容是否有变化
    if (strcmp(music_info.title, title) != 0) {
        // 保存原始未截断的标题用于后续匹配
        strncpy(original_title, title, sizeof(original_title) - 1);
        original_title[sizeof(original_title) - 1] = '\0';
        original_title_initialized = true; // 设置标志为已初始化

        // 处理显示用的标题
        strncpy(music_info.title, title, 18);
        music_info.title[18] = '\0';  // 确保终止符

        if (strlen(title) > 18) {
            strcat(music_info.title, "...");
        }

        put_buf(music_info.title, strlen(music_info.title) + 1);
        printf("music_info.title : %s\n", music_info.title);
        music_info.title_changed = true;
        request_ui_update();
    }
}

// 更新艺术家，歌手名
void music_update_artist(const char *artist)
{
    if (!artist) {
        return;
    }

    // 如果 original_title 还没有初始化，直接使用当前的 music_info.title
    if (!original_title_initialized) {
        printf("[警告] original_title 未初始化，使用当前标题进行匹配\n");
        // 创建临时缓冲区处理歌手名
        char temp_artist[48] = {0};
        strncpy(temp_artist, artist, 47);

        char *pos = strstr(temp_artist, music_info.title);

        if (pos != NULL) {
            size_t title_len = strlen(music_info.title);
            size_t remaining_len = strlen(pos + title_len);
            // 移除包含的歌曲名部分
            memmove(pos, pos + title_len, remaining_len + 1);

            // 处理可能的前后空格
            while (*pos == ' ') {
                memmove(pos, pos + 1, strlen(pos));
            }
            char *end = pos + strlen(pos) - 1;
            while (end > pos && *end == ' ') {
                *end-- = '\0';
            }
        }

        // 检查处理后的内容是否有变化
        if (strcmp(music_info.artist, temp_artist) != 0) {
            // 截断并添加省略号
            strncpy(music_info.artist, temp_artist, 21);
            music_info.artist[21] = '\0';

            if (strlen(temp_artist) > 21) {
                strcat(music_info.artist, "...");
            }
            music_info.artist_changed = true;
            request_ui_update();
        }
        return;
    }

    // 以下是原有的处理逻辑（当 original_title 已初始化时）
    // 创建临时缓冲区处理歌手名
    char temp_artist[48] = {0};
    strncpy(temp_artist, artist, 47);

    // 先尝试用原始标题匹配
    char *pos = strstr(temp_artist, original_title);

    // 如果原始标题没匹配到，尝试用显示的标题(可能带省略号)匹配
    if (pos == NULL) {
        pos = strstr(temp_artist, music_info.title);
    }

    if (pos != NULL) {
        // 计算剩余字符串长度
        size_t title_len = (pos == strstr(temp_artist, original_title)) ?
                          strlen(original_title) : strlen(music_info.title);

        size_t remaining_len = strlen(pos + title_len);
        // 移除包含的歌曲名部分
        memmove(pos, pos + title_len, remaining_len + 1);

        // 处理可能的前后空格
        while (*pos == ' ') {
            memmove(pos, pos + 1, strlen(pos));
        }
        char *end = pos + strlen(pos) - 1;
        while (end > pos && *end == ' ') {
            *end-- = '\0';
        }
    }

    // 检查处理后的内容是否有变化
    if (strcmp(music_info.artist, temp_artist) != 0) {
        // 截断并添加省略号
        strncpy(music_info.artist, temp_artist, 21);
        music_info.artist[21] = '\0';

        if (strlen(temp_artist) > 21) {
            strcat(music_info.artist, "...");
        }
        music_info.artist_changed = true;
        request_ui_update();
    }
}

// 更新歌词或专辑
void music_update_genre(const char *genre)
{

    if (!genre) {
        return;
    }
    // 检查内容是否有变化
    if (strcmp(music_info.genre, genre) != 0) {
        lv_snprintf(music_info.genre, sizeof(music_info.genre), "%s", genre);
        music_info.genre_changed = true;
        request_ui_update();
    }

}

// 更新总时长（注意：需在UI结构体中补充时间显示控件才能完全生效）
void music_update_time(uint32_t total_sec)
{
    if (total_sec == 0) {
        return;
    }

    // 检查时长是否有变化
    if (music_info.total_time != total_sec) {
        music_info.total_time = total_sec;
        music_info.total_time_changed = true;
        request_ui_update();
    }
}

// 更新进度条范围（保留兼容性）
void music_update_slider_range(uint32_t total_time)
{
    // 此函数现在通过 lv_demo_music_update_time 处理
    music_update_time(total_time);
}



// 更新当前播放时间（注意：需在UI结构体中补充时间显示控件才能完全生效）
void music_update_current_time(uint32_t curr_sec)
{
    // 检查当前播放时间是否有变化
    if (music_info.current_time != curr_sec) {
        music_info.current_time = curr_sec;
        music_info.current_time_changed = true;
        request_ui_update();
    }

}


// 获取当前播放时间
uint32_t music_get_current_time(void)
{
    return music_info.current_time;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* 磁带转动核心变量 */
static lv_timer_t *rotate_timer = NULL;
static uint32_t start_time = 0;       // 旋转开始的时间戳(ms)
static uint32_t pause_offset = 0;     // 暂停时已旋转的总时间(ms)
static const uint32_t ROTATE_PERIOD = 4000;  // 旋转一圈的时间(ms)
bool g_play_pause_btn_state = false;
/**
 * @brief 初始化磁带动画状态
 * 将所有动画相关的状态变量重置为初始值
 */
static void init_tape_animation_state(void)
{
    start_time = 0;       // 重置开始时间
    pause_offset = 0;     // 重置暂停偏移
    rotate_timer = NULL;  // 确保定时器为空
    g_play_pause_btn_state = true; // 设置默认状态为播放
    lv_img_set_angle(guider_ui.screen_music_img_tape, 0); // 重置角度为0度
}
/**
 * @brief 定时器回调：通过绝对时间计算角度（无累加误差）
 */
static void rotate_timer_cb(lv_timer_t *timer)
{
    // 计算当前已旋转的总时间（包含历史累计）
    uint32_t current_time = lv_tick_get() - start_time + pause_offset;

    // 计算角度：(总时间 / 周期) * 360度，取模360确保在0-359范围
    int32_t angle = (current_time * 360 * 10) / ROTATE_PERIOD % (360 * 10);

    // 设置角度
    lv_img_set_angle(guider_ui.screen_music_img_tape, angle);
}

/**
 * @brief 启动磁带动画函数
 * 开始磁带旋转动画，并设置相关图标状态
 */
static void tape_rotate_start(void)
{
    // 设置磁带条的角度为-50度
    lv_img_set_angle(guider_ui.screen_music_img_bar, -50);

    // 启动定时器（从上次停止的角度继续）
    if (!rotate_timer) {
        start_time = lv_tick_get();  // 记录当前启动时间
        rotate_timer = lv_timer_create(rotate_timer_cb, 10, NULL);  // 10ms刷新一次
        lv_timer_set_repeat_count(rotate_timer, -1);  // 无限循环
    }
}

/**
 * @brief 停止磁带动画函数
 * 停止磁带旋转动画，并保存当前状态
 */
static void tape_rotate_stop(void)
{
    // 停止定时器并记录已旋转的时间（关键：保存历史状态）
    if (rotate_timer) {
        pause_offset += lv_tick_get() - start_time;
        lv_timer_del(rotate_timer);
        rotate_timer = NULL;
    }

    // 220
    lv_img_set_angle(guider_ui.screen_music_img_bar, 220);
}
// 播放/暂停按钮点击事件处理函数
static void screen_music_btn_play_pause_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        // 切换按钮状态
        g_play_pause_btn_state = !g_play_pause_btn_state;
        if (g_play_pause_btn_state) {
            // 设置为播放
            lv_label_set_text(ui->screen_music_btn_play_pause_label, "I I");
            lv_obj_set_style_text_font(ui->screen_music_btn_play_pause_label, &lv_font_Barlow__15, LV_PART_MAIN|LV_STATE_DEFAULT);
            tape_rotate_start();
            // 启动歌词滚动
            lv_label_set_long_mode(ui->screen_music_lyric, LV_LABEL_LONG_SCROLL_CIRCULAR);
            lv_demo_handle_music_play();
        } else {
            // 设置为暂停
            lv_label_set_text(ui->screen_music_btn_play_pause_label, "   "LV_SYMBOL_PLAY " ");
            lv_obj_set_style_text_font(ui->screen_music_btn_play_pause_label, &lv_font_Barlow__11, LV_PART_MAIN|LV_STATE_DEFAULT);
            tape_rotate_stop();
            // 停止歌词滚动
            lv_label_set_long_mode(ui->screen_music_lyric, LV_LABEL_LONG_DOT);
            lv_demo_handle_music_pause();
        }
        break;
    }
    default:
        break;
    }
}
// 播放/暂停按钮显示处理函数
void async_screen_music_btn_play_pause_show(void* param){
    bool is_pause = (bool)(intptr_t)param;
    if(!is_pause){
       // 设置为播放
        g_play_pause_btn_state = 1;
        lv_label_set_text(guider_ui.screen_music_btn_play_pause_label, "I I");
        lv_obj_set_style_text_font(guider_ui.screen_music_btn_play_pause_label, &lv_font_Barlow__15, LV_PART_MAIN|LV_STATE_DEFAULT);
        tape_rotate_start();
        // 启动歌词滚动
        lv_label_set_long_mode(guider_ui.screen_music_lyric, LV_LABEL_LONG_SCROLL_CIRCULAR);
    }else{
        g_play_pause_btn_state = 0;
         // 设置为暂停
        lv_label_set_text(guider_ui.screen_music_btn_play_pause_label, "   "LV_SYMBOL_PLAY " ");
        lv_obj_set_style_text_font(guider_ui.screen_music_btn_play_pause_label, &lv_font_Barlow__11, LV_PART_MAIN|LV_STATE_DEFAULT);
        tape_rotate_stop();
        // 停止歌词滚动
        lv_label_set_long_mode(guider_ui.screen_music_lyric, LV_LABEL_LONG_DOT);
    }
}
// 异步调用播放/暂停按钮显示处理函数
void screen_music_btn_play_pause_show(bool is_pause){
    printf("异步调用播放/暂停按钮显示处理函数: %s\n", is_pause ? "暂停" : "播放");
    lv_async_call(async_screen_music_btn_play_pause_show, (void *)is_pause);
}
// 音量进度条状态
static bool g_volume_bar_visible = false;
// 音量进度条自动隐藏定时器
static lv_timer_t *g_volume_bar_timer = NULL;
// 音量进度条自动隐藏定时器回调函数
static void volume_bar_auto_hide_cb(lv_timer_t *timer)
{
    lv_ui *ui = (lv_ui *)timer->user_data;

    // 隐藏音量进度条
    lv_obj_add_flag(ui->screen_music_bar_volume, LV_OBJ_FLAG_HIDDEN);
    g_volume_bar_visible = false;

    // 删除定时器
    if (g_volume_bar_timer != NULL) {
        lv_timer_del(g_volume_bar_timer);
        g_volume_bar_timer = NULL;
    }
}
/**
 * 处理音量调节功能
 * 区分网络音乐和蓝牙音乐
 */
void volume_icon_click_handler(void){
    // 显示音量进度条
    extern lv_ui guider_ui;

    if(!guider_ui.screen_music_bar_volume){
        printf("[错误] 音量进度条UI对象不存在\n");
        return;
    }
    lv_obj_clear_flag(guider_ui.screen_music_bar_volume, LV_OBJ_FLAG_HIDDEN);

       // 删除定时器
    if (g_volume_bar_timer != NULL) {
        lv_timer_del(g_volume_bar_timer);
        g_volume_bar_timer = NULL;
    }
   // 创建新的定时器，5秒后自动隐藏
    g_volume_bar_timer = lv_timer_create(volume_bar_auto_hide_cb, 5000, &guider_ui);

    int32_t current_volume = lv_bar_get_value(guider_ui.screen_music_bar_volume);
}
/**
 * @brief 音量进度条事件处理函数（统一处理所有触控相关事件）
 * @param e LVGL事件对象（包含事件类型、目标对象等信息）
 */
static void screen_music_bar_volume_event_handler(lv_event_t *e)
{
    lv_obj_t *volume_bar = lv_event_get_target(e);  // 获取音量进度条对象
    lv_event_code_t event_code = lv_event_get_code(e);  // 获取当前事件类型

    // 处理按下和拖拽事件
    if(event_code == LV_EVENT_PRESSED || event_code == LV_EVENT_PRESSING) {
        // 获取当前输入设备和触摸点
        lv_indev_t *indev = lv_indev_get_act();
        if(indev == NULL) return;

        lv_point_t point;
        lv_indev_get_point(indev, &point);

        // 获取进度条的位置和尺寸
        lv_area_t bar_area;
        lv_obj_get_coords(volume_bar, &bar_area);
        int32_t bar_height = lv_area_get_height(&bar_area); // 先声明变量
        int32_t relative_y = point.y - bar_area.y1;
        int32_t volume = 100 - (relative_y * 100 / bar_height);

        static uint8_t last_volume_value = 0;
        if(volume < 10) volume = 10;
        if(volume > 100) volume = 100;
        if (last_volume_value && (last_volume_value == volume)) {
            printf("[音量事件] 音量未改变\n");
            return ;
        }
        last_volume_value = volume;
        lv_bar_set_value(volume_bar, volume, LV_ANIM_OFF);
        printf("[音量事件] 点击调节至: %d%% (触摸Y: %d, 进度条高: %d)\n", volume, relative_y, bar_height);
        volume_icon_click_handler();
    }
    else if(event_code == LV_EVENT_VALUE_CHANGED) {
        // 值改变事件
        int32_t current_volume = lv_bar_get_value(volume_bar);
        printf("[音量事件] 音量值已更改为: %d%%\n", (int)current_volume);
        volume_icon_click_handler();

    }
}
// 音量按钮点击事件处理函数
static void screen_music_img_volume_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        // 音量按钮点击事件处理逻辑
        printf("音量按钮被点击\n");

        // 调用音量图标点击处理函数
        volume_icon_click_handler();
        break;
    }
    default:
        break;
    }
}

// 上一首按钮点击事件处理函数
static void screen_music_img_prev_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        // 上一首按钮点击事件处理逻辑
        printf("上一首按钮被点击\n");
        lv_demo_handle_music_prev();
        break;
    }
    default:
        break;
    }
}

// 下一首按钮点击事件处理函数
static void screen_music_img_next_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        // 下一首按钮点击事件处理逻辑
        printf("下一首按钮被点击\n");
        lv_demo_handle_music_next();
        break;
    }
    default:
        break;
    }
}


// 变量：存储当前音乐模式的图标和标题
static const void *g_music_current_icon = &_play_random_alpha_20x20;  // 默认图标
static char g_music_current_title[32] = "默认音乐";              // 默认标题
static char g_music_current_singer[32] = "歌手加载中";           // 默认歌手
static char g_music_current_lyrics[32] = "歌词加载中";           // 默认歌词
// 音乐页面切换初始化函数
void async_music_page_init(void* param)
{
    // 根据新的模式ID更新模式名称和图标
    switch(get_current_play_mode_id()) {
        case MODE_NET_ID:
            g_music_current_icon = &_play_random_alpha_20x20;
            lv_imgbtn_set_src(guider_ui.screen_music_imgbtn_mode, LV_IMGBTN_STATE_RELEASED, NULL, g_music_current_icon, NULL);
            lv_label_set_text(guider_ui.screen_music_lyric, g_music_current_lyrics);
            // 确保标签足够高以显示一行文本，避免垂直滚动
            lv_obj_set_height(guider_ui.screen_music_lyric, lv_font_get_line_height(lv_obj_get_style_text_font(guider_ui.screen_music_lyric, LV_PART_MAIN)) + 4);
            return;
            // strcpy(g_music_current_title, "网络音乐");
            break;
        case MODE_BT_ID:
            g_music_current_icon = &_BT_alpha_20x20;
            strcpy(g_music_current_title, "蓝牙音乐");
            break;
        case MODE_SD_ID:
            g_music_current_icon = &_shunxu_alpha_20x20;
            strcpy(g_music_current_title, "SD卡音乐");
            break;
        default:
            // 保持默认值，无需重复赋值
            break;
    }
    // 使用全局变量设置UI，避免原代码中覆盖前面设置的问题
    lv_imgbtn_set_src(guider_ui.screen_music_imgbtn_mode, LV_IMGBTN_STATE_RELEASED, NULL, g_music_current_icon, NULL);
          // 刷新对象以确保设置生效
    lv_obj_update_layout(guider_ui.screen_music_imgbtn_mode);
    lv_span_set_text(guider_ui.screen_music_span_title_span, g_music_current_title);
    lv_span_set_text(guider_ui.screen_music_span_singer_span, g_music_current_singer);
    lv_label_set_text(guider_ui.screen_music_lyric, g_music_current_lyrics);
     // 确保标签足够高以显示一行文本，避免垂直滚动
    lv_obj_set_height(guider_ui.screen_music_lyric, lv_font_get_line_height(lv_obj_get_style_text_font(guider_ui.screen_music_lyric, LV_PART_MAIN)) + 4);
}
// 异步处理音乐页面切换初始化函数
void music_page_init(void){
    // 使用异步调用处理UI更新和定时器清理，避免线程冲突
    lv_async_call(async_music_page_init, NULL);
}
// 初始化音乐播放模式全局变量
music_play_mode_t g_current_play_mode = {
    .mode_id = MODE_NET_ID,
    .name = "网络模式",
    .mode_status = "随机播放",
    .is_clicked = false
};

// 获取当前播放模式ID
int get_current_play_mode_id(void) {
    return g_current_play_mode.mode_id;
}

// 设置当前播放模式ID
void set_current_play_mode_id(int mode_id) {
    g_current_play_mode.mode_id = mode_id;
}
// 获取当前播放模式状态
char *get_current_play_mode_status(void) {
    return g_current_play_mode.mode_status;
}
// 设置当前播放模式状态
void set_current_play_mode_status(char *status) {
    strcpy(g_current_play_mode.mode_status, status);
}
// 模式切换按钮点击事件处理函数
static void screen_music_imgbtn_mode_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        // 模式切换按钮点击事件处理逻辑
        printf("模式切换按钮被点击\n");
        // 切换点击状态
        g_current_play_mode.is_clicked = !g_current_play_mode.is_clicked;
        // 根据新的模式ID更新模式名称
        switch(get_current_play_mode_id()) {
            case MODE_NET_ID:
                strcpy(g_current_play_mode.name, "网络模式");
                // 根据按钮状态更新播放模式状态
                if(g_current_play_mode.is_clicked) {
                    set_current_play_mode_status("单曲循环");
                } else {
                    set_current_play_mode_status("随机播放");
                }
                break;
            case MODE_BT_ID:
                strcpy(g_current_play_mode.name, "蓝牙模式");
                set_current_play_mode_status("蓝牙播放");
                break;
            case MODE_SD_ID:
                strcpy(g_current_play_mode.name, "SD卡模式");
                // 根据按钮状态更新播放模式状态
                if(g_current_play_mode.is_clicked) {
                    set_current_play_mode_status("单曲循环");
                } else {
                    set_current_play_mode_status("顺序循环");
                }
                break;
            default:
                break;
        }
        if(strcmp(g_current_play_mode.name, "网络模式") == 0) {
            printf("网络模式：随机播放和单曲循环\n");
            lv_imgbtn_set_src(ui->screen_music_imgbtn_mode, LV_IMGBTN_STATE_RELEASED, NULL, &_play_random_alpha_20x20, NULL);
            lv_imgbtn_set_src(ui->screen_music_imgbtn_mode, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &_play_repeat_alpha_20x20, NULL);
        }else if(strcmp(g_current_play_mode.name, "蓝牙模式") == 0) {
            printf("蓝牙模式：蓝牙图标\n");
            lv_imgbtn_set_src(ui->screen_music_imgbtn_mode, LV_IMGBTN_STATE_RELEASED, NULL, &_BT_alpha_20x20, NULL);
            lv_imgbtn_set_src(ui->screen_music_imgbtn_mode, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &_BT_alpha_20x20, NULL);
        }else if(strcmp(g_current_play_mode.name, "SD卡模式") == 0) {
            printf("SD卡模式：顺序播放和随机播放\n");
            lv_imgbtn_set_src(ui->screen_music_imgbtn_mode, LV_IMGBTN_STATE_RELEASED, NULL, &_shunxu_alpha_20x20, NULL);
            lv_imgbtn_set_src(ui->screen_music_imgbtn_mode, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &_play_repeat_alpha_20x20, NULL);
        }

        printf("模式为%s\n", g_current_play_mode.name);
        lv_demo_handle_music_mode_switch();

        break;
    }
    default:
        break;
    }
}


/**
 * 处理音乐暂停功能
 * 区分网络音乐、蓝牙音乐和SD卡音乐
 */
void lv_demo_handle_music_pause(void)
{
#ifdef APPLAYER_ENABLE
    // 根据新的模式ID更新模式名称
    switch(get_current_play_mode_id()) {
        case MODE_NET_ID:
            net_music_play_pause(1);//1表示暂停
            printf("[音乐控制] 暂停网络音乐\n");
            break;
        case MODE_BT_ID:
            strcpy(g_current_play_mode.name, "蓝牙模式");
            bt_music_play_pause_toggle();
            printf("[音乐控制] 暂停蓝牙音乐\n");
            break;
        case MODE_SD_ID:
            #ifdef CONFIG_SD_MUSIC_MODE_ENABLE
                extern int sd_music_play_pause(char pause);
                sd_music_play_pause(1);//1表示暂停
            #endif
            printf("[音乐控制] 暂停SD卡音乐\n");
            break;
        default:
            break;
    }
#endif
}
/**
 * 处理音乐播放功能
 * 根据当前播放模式执行相应操作
 */
void lv_demo_handle_music_play(void){
#ifdef APPLAYER_ENABLE
    // 根据当前播放模式执行相应操作
    switch(get_current_play_mode_id()) {
        case MODE_NET_ID:
            net_music_play_pause(0);//0表示继续播放
            printf("[音乐控制] 播放网络音乐\n");
            break;
        case MODE_BT_ID:
            bt_music_play_pause_toggle();
            printf("[音乐控制] 播放蓝牙音乐\n");
            break;
        case MODE_SD_ID:
            #ifdef CONFIG_SD_MUSIC_MODE_ENABLE
                extern int sd_music_play_pause(char pause);
                sd_music_play_pause(0);//0表示继续播放
             #endif
            printf("[音乐控制] 播放SD卡音乐\n");
            break;
        default:
            break;
    }
#endif
}
/**
 * 处理上一曲播放功能
 * 根据当前播放模式执行相应操作
 */
void lv_demo_handle_music_prev(void){
#ifdef APPLAYER_ENABLE
    // 根据当前播放模式执行相应操作
    switch(get_current_play_mode_id()) {
        case MODE_NET_ID:
            if(net_music_play_last_chack()){
                net_music_play_last_request();
            }else{
                net_music_play_next();
            }
            printf("[音乐控制] 播放上一曲网络音乐\n");
            break;
        case MODE_BT_ID:
            BT_handle_music_play_prev();
            printf("[音乐控制] 播放上一曲蓝牙音乐\n");
            break;
        case MODE_SD_ID:
            #ifdef CONFIG_SD_MUSIC_MODE_ENABLE
                extern int sd_music_play_prev(void);
                sd_music_play_prev();
            #endif
            printf("[音乐控制] 播放上一曲SD卡音乐\n");
            break;
        default:
            break;
    }
#endif
}
/**
 * 处理下一曲播放功能
 * 根据当前播放模式执行相应操作
 */
void lv_demo_handle_music_next(void){
#ifdef APPLAYER_ENABLE
    // 根据当前播放模式执行相应操作
    switch(get_current_play_mode_id()) {
        case MODE_NET_ID:
            net_music_play_next();
            printf("[音乐控制] 播放下一曲网络音乐\n");
            break;
        case MODE_BT_ID:
            BT_handle_music_play_next();
            printf("[音乐控制] 播放下一曲蓝牙音乐\n");
            break;
        case MODE_SD_ID:
            #ifdef CONFIG_SD_MUSIC_MODE_ENABLE
                extern int sd_music_play_next(void);
                sd_music_play_next();
            #endif
            printf("[音乐控制] 播放下一曲SD卡音乐\n");
            break;
        default:
            break;
    }
#endif
}

/**
 * 处理播放模式切换功能
 * 根据当前播放模式执行相应操作
 */
void lv_demo_handle_music_mode_switch(void){
#ifdef APPLAYER_ENABLE
    // 根据当前播放模式执行相应操作
    switch(get_current_play_mode_id()) {
        case MODE_NET_ID:
            printf("[网络音乐]当前播放模式状态：%s\n", get_current_play_mode_status());
            if(strcmp(get_current_play_mode_status(), "单曲循环") == 0){
                net_music_play_loop();
            }else if(strcmp(get_current_play_mode_status(), "随机循环") == 0){
                net_music_play_loop_clear();
            }
            break;
        case MODE_BT_ID:
            printf("[蓝牙音乐]当前播放模式状态：%s\n", get_current_play_mode_status());

            break;
        case MODE_SD_ID:
            #ifdef CONFIG_SD_MUSIC_MODE_ENABLE
                printf("[SD卡音乐]当前播放模式状态：%s\n", get_current_play_mode_status());
                if(strcmp(get_current_play_mode_status(), "单曲循环") == 0){
                    sd_music_play_loop_set(1);
                }else if(strcmp(get_current_play_mode_status(), "顺序播放") == 0){
                    sd_music_play_loop_set(0);
                }
            #endif
            break;
        default:
            break;
    }
#endif
}

void music_event_init(lv_ui *ui) {
    // 添加这行代码完全禁止页面滚动
    lv_obj_set_scroll_dir(ui->screen_music, LV_DIR_NONE);
    // 禁用音量进度条的点击事件
    lv_obj_clear_flag(guider_ui.screen_music_slider, LV_OBJ_FLAG_CLICKABLE);
    // 为播放/暂停按钮添加点击事件
    lv_obj_remove_event_cb(ui->screen_music_btn_play_pause, NULL);
    lv_obj_add_event_cb(ui->screen_music_btn_play_pause, screen_music_btn_play_pause_event_handler, LV_EVENT_ALL, ui);
    // 初始化磁带动画状态
    init_tape_animation_state();
    if (g_play_pause_btn_state) {
        // 设置为播放
        lv_label_set_text(ui->screen_music_btn_play_pause_label, "I I");
        lv_obj_set_style_text_font(ui->screen_music_btn_play_pause_label, &lv_font_Barlow__15, LV_PART_MAIN|LV_STATE_DEFAULT);
        tape_rotate_start();
        // 启动歌词滚动
        lv_label_set_long_mode(ui->screen_music_lyric, LV_LABEL_LONG_SCROLL_CIRCULAR);
    }
    // 为音量按钮添加点击事件
    lv_obj_remove_event_cb(ui->screen_music_img_volume, NULL);
    lv_obj_add_event_cb(ui->screen_music_img_volume, screen_music_img_volume_event_handler, LV_EVENT_CLICKED, ui);

    // 为上一首按钮添加点击事件
    lv_obj_remove_event_cb(ui->screen_music_img_prev, NULL);
    lv_obj_add_event_cb(ui->screen_music_img_prev, screen_music_img_prev_event_handler, LV_EVENT_CLICKED, ui);

    // 为下一首按钮添加点击事件
    lv_obj_remove_event_cb(ui->screen_music_img_next, NULL);
    lv_obj_add_event_cb(ui->screen_music_img_next, screen_music_img_next_event_handler, LV_EVENT_CLICKED, ui);

    // 为模式切换按钮添加点击事件
    lv_obj_remove_event_cb(ui->screen_music_imgbtn_mode, NULL);
    lv_obj_add_event_cb(ui->screen_music_imgbtn_mode, screen_music_imgbtn_mode_event_handler, LV_EVENT_CLICKED, ui);

    // 为音量进度条添加上下移动（滑动）事件
    lv_obj_remove_event_cb(ui->screen_music_bar_volume, NULL);
    lv_obj_add_event_cb(ui->screen_music_bar_volume, screen_music_bar_volume_event_handler, LV_EVENT_ALL, ui);

}

void music_event_setup(void) {
    music_event_init(&guider_ui);
}



