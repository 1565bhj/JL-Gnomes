#include "ui/ui.h"
#include "ui_api.h"
#include "system/timer.h"
#include "server/server_core.h"
#include "os/os_api.h"
#include "asm/gpio.h"
#include "system/includes.h"
#include "server/audio_server.h"
#include "storage_device.h"
#include "font/font_textout.h"
#include "ui/includes.h"
#include "ui_action_video.h"
#include "font/font_all.h"
#include "font/language_list.h"
#include "ename.h"
#include "asm/rtc.h"
#include "lcd_drive.h"
#include "yuv_soft_scalling.h"
#include "event/key_event.h"
#include "lcd_te_driver.h"
#include "app_music.h"
#include "lcd_config.h"
#include "lv_disp.h"
#include "app_config.h"
#include "ai_uart_ctrol.h"

#ifdef CONFIG_UI_ENABLE

/****************************LVGL：设备接口*********************************/
extern int is_module_production_test_enter(void);
//模块厂测情况下退出LVGL所有接口功能（模块厂测没有初始化LVGL）
#define PRODUCTION_TEST_FUNC_CHECK() if (is_module_production_test_enter()) return 0;


// 添加滑动方向定义
#define SWIPE_NONE      0   // 无滑动
#define SWIPE_LEFT      1   // 向左滑动
#define SWIPE_RIGHT     2   // 向右滑动
#define SWIPE_UP        3   // 向上滑动
#define SWIPE_DOWN      4   // 向下滑动
#define SWIPE_CLICK     5   // 点击

// 添加滑动阈值定义
#define SWIPE_THRESHOLD 80  // 滑动判断阈值

// 添加点击检测阈值定义
#define CLICK_THRESHOLD 10  // 点击判断阈值（移动距离小于此值判定为点击）

static uint32_t last_touch_time = 0;  // 最后一次触摸的时间戳
// 添加触摸起始坐标记录
static short touch_start_x = -1;
static short touch_start_y = -1;
static unsigned char current_swipe_direction = SWIPE_NONE;
static unsigned char last_touch_status = 0;
static short last_touch_x = 0;
static short last_touch_y = 0;
// 添加滑动处理标志位
static unsigned char swipe_handled_in_current_touch = 0;

/**
 * @brief 触摸接口初始化
 * @param NULL
 * @return NULL
 */
void lvgl_touch_pad_init(void)//触摸初始化
{
#ifdef TCFG_TOUCH_GST820_ENABLE
    int check_GST820(void);
    check_GST820();
#endif
#ifdef TCFG_TOUCH_GST328_ENABLE
    int check_GST328(void);
    check_GST328();
#endif
#ifdef TCFG_TOUCH_AXS15252_ENABLE
    int check_axs15252(void);
    check_axs15252();
#endif
}

/**
 * @brief 获取当前滑动方向
 * @param NULL
 * @return 当前滑动方向（SWIPE_NONE, SWIPE_LEFT, SWIPE_RIGHT, SWIPE_UP, SWIPE_DOWN, SWIPE_CLICK）
 */
unsigned char lvgl_get_swipe_direction(void)
{
    return current_swipe_direction;
}

/**
 * @brief 在需要使用滑动功能的地方调用此函数获取并处理滑动方向
 * @return 当前检测到的滑动方向
 */
static void lvgl_handle_page_swipe(void)
{
    unsigned char dir = lvgl_get_swipe_direction();

    if ((dir == SWIPE_LEFT || dir == SWIPE_RIGHT || dir == SWIPE_UP || dir == SWIPE_DOWN || dir == SWIPE_CLICK) &&
        lv_demo_is_ring_page()) {//闹钟和计时器时间到情况下才停止播放
        lv_demo_alarms_ring_clean();
    }

    switch (dir) {
    case SWIPE_LEFT:
        printf("[滑动处理] 检测到向左滑动\n");
        // 这里可以添加页面右移的具体实现
        break;
    case SWIPE_RIGHT:
        printf("[滑动处理] 检测到向右滑动\n");
        // 这里可以添加页面左移的具体实现
        break;
    case SWIPE_UP:
        printf("[滑动处理] 检测到向上滑动\n");
        if (is_conversation_page()) {
            aisp_clear(1);
        }
        switch_to_main_page();
        printf("[滑动处理] 切换到主页面\n");

        break;
    case SWIPE_DOWN:
        printf("[滑动处理] 检测到向下滑动\n");
        switch_to_dropdown_page();
        printf("[滑动处理] 切换到下拉框页面\n");
        break;
    case SWIPE_CLICK:
        printf("[滑动处理] 检测到点击操作\n");

        if (lv_demo_is_main_page()) {
            // 调用screen_main.c中的图标点击处理函数
            void lv_screen_open_icon_screen_set(short x, short y);
            // 使用最后触摸的坐标
            lv_screen_open_icon_screen_set(last_touch_x, last_touch_y);
        }
        break;
    default:
        // 无滑动或无效滑动，无需处理
        break;
    }

    // 在处理完滑动后立即重置滑动方向，确保一次滑动只处理一次
    current_swipe_direction = SWIPE_NONE;
    printf("[滑动处理] 重置滑动方向为无滑动\n");

}

/**
 * @brief 触摸最后一次时间获取
 * @param NULL
 * @return 最后一次数模时间，单位ms
 */
int lvgl_touch_last_get_time(void)
{
    return last_touch_time;
}

/**
 * @brief 触摸坐标获取
 * @param x横坐标指针地址,y纵坐标指针地址,status : true按压，false释放
 * @return NULL
 */
void lvgl_touch_x_y_status_get(short *x, short *y, unsigned char *status)//获取触摸：status : true->一直按，false->释放
{
#ifdef TCFG_TOUCH_GST820_ENABLE
    GST820_get_xy(x, y, status);
    // 添加xy坐标交换
    short temp = *x;
    *x = *y;
    *y = temp;
    *y = LCD_H - *y;
#endif

#ifdef TCFG_TOUCH_GST328_ENABLE
    GST328_get_xy(x, y, status);
    // 添加xy坐标交换
    short temp = *x;
    *x = *y;
    *y = temp;
#endif

#ifdef TCFG_TOUCH_AXS15252_ENABLE
    void axs15252_get_xy(short *x, short *y, char *press);
    axs15252_get_xy(x, y, (char *)status);
#endif
#if 1
    static unsigned char last_status = 0;
    if (*status || last_status != *status) {
//        printf("lvgl touch --->  x = %d, y = %d, status = %d \n", *x, *y, *status);
    }
    last_status = *status;
#endif

    // 实现页面移动方向判断逻辑
    if (*status) {  // 触摸按下状态
        last_touch_time = timer_get_ms();  // 有触摸时刷新时间戳，供 is_touch_active() 使用
        if (!last_touch_status) {  // 新的触摸开始
            // 记录触摸起始坐标
            touch_start_x = *x;
            touch_start_y = *y;
            // 重置滑动方向和处理标志
            current_swipe_direction = SWIPE_NONE;
            swipe_handled_in_current_touch = 0;
        } else {  // 之前也处于按下状态
            // 计算坐标差值
            // 改为使用起始点计算差值，而不是与上一点的差值
            int delta_x = *x - touch_start_x;
            int delta_y = *y - touch_start_y;

            // 判断滑动方向（基于阈值和主要移动方向）
            // 增加条件：只有在current_swipe_direction仍为SWIPE_NONE且本次触摸尚未处理过滑动时才更新滑动方向
            if (current_swipe_direction == SWIPE_NONE && !swipe_handled_in_current_touch) {
                if (abs(delta_x) > abs(delta_y) && abs(delta_x) > SWIPE_THRESHOLD) {
                    // 水平方向滑动
                    current_swipe_direction = (delta_x > 0) ? SWIPE_RIGHT : SWIPE_LEFT;
                } else if (abs(delta_y) > abs(delta_x) && abs(delta_y) > SWIPE_THRESHOLD &&
                           ((touch_start_y >= 0 && touch_start_y <= 15) || (touch_start_y >= (LCD_H - 20) && touch_start_y <= LCD_H))) {
                    // 垂直方向滑动（添加开始触摸点阈值：顶部0-15或底部215-240范围才响应上下滑动）
                    current_swipe_direction = (delta_y > 0) ? SWIPE_DOWN : SWIPE_UP;
                }
                // 如果未达到阈值，保持SWIPE_NONE状态
            }
        }

        // 记录当前坐标作为下一次的参考
        last_touch_x = *x;
        last_touch_y = *y;
    } else {  // 触摸释放状态
        // 检查是否为点击（触摸开始到结束的移动距离小于阈值）
        if (last_touch_status && touch_start_x >= 0 && touch_start_y >= 0) {
            int move_dist_x = abs(*x - touch_start_x);
            int move_dist_y = abs(*y - touch_start_y);

            // 如果移动距离很小，判定为点击
            if (move_dist_x < CLICK_THRESHOLD && move_dist_y < CLICK_THRESHOLD) {
                current_swipe_direction = SWIPE_CLICK;
            }
        }

        // 触摸释放时，重置触摸状态
        touch_start_x = -1;
        touch_start_y = -1;
        // 重置滑动处理标志
        swipe_handled_in_current_touch = 0;
    }

    // 更新上一次触摸状态
    last_touch_status = *status;

    // 处理滑动方向
    if (current_swipe_direction != SWIPE_NONE) {
        lvgl_handle_page_swipe();
        // 设置标志位，表示本次触摸已经处理过滑动
        swipe_handled_in_current_touch = 1;
    }
}

/**
 * @brief 显示驱动回调函数
 * @param disp_drv 驱动器
 * @param area 显示参数
 * @return NULL
 */
void lvgl_disp_rounder_cb(lv_disp_drv_t *disp_drv, lv_area_t *area)
{
#if TCFG_LCD_AXS15252_ENABLE //4字节对齐
    /**
     * AXS15252 等屏 1x4 像素开窗：列窗口需满足 x1=4*N、x2=4*N+3（宽度为 4 的倍数）。
     */
    lv_coord_t hor_max = disp_drv->hor_res - 1;
    area->x1 &= ~(lv_coord_t)3;
    {
        lv_coord_t x2 = area->x2;
        lv_coord_t r = x2 & 3;
        if (r != 3) {
            x2 += (lv_coord_t)((3 - r + 4) & 3);
        }
        if (x2 > hor_max) {
            x2 = hor_max;
        }
        area->x2 = x2;
    }

    if (area->x1 > area->x2) {
        area->x1 = area->x2 & ~(lv_coord_t)3;
    }
#endif
}

/**
 * @brief LVGL异步强制刷新
 * @param NULL
 * @return NULL
 */
static void lvgl_flush_async(void)
{
#ifdef CONFIG_LVGL_UI_ENABLE
    lcd_lvgl_able(1);
    if (lv_scr_act()) { //判断当前屏幕存在
        lv_obj_invalidate(lv_scr_act());  // 使当前屏幕失效
        lv_refr_now(NULL);//强行刷新屏幕
    }
#endif
}

/**
 * @brief LVGL显示屏强制刷新
 * @param NULL
 * @return NULL
 */
void lvgl_flush_enable(void)
{
#ifdef CONFIG_LVGL_UI_ENABLE
    lv_async_call(lvgl_flush_async, NULL);
#endif
}

/**
 * @brief LVGL显示屏强制刷新
 * @param NULL
 * @return NULL
 */
void lvgl_flaush_disable(void *priv)
{
#ifdef CONFIG_LVGL_UI_ENABLE
    lcd_lvgl_able(0);
#ifdef CONFIG_UI_PLAY_EMOJI
    play_face_emoji(AI_UART_CMD_EMOJI_RAND0 + (int)priv);
#endif
#endif
}

/**
 * @brief 显示屏初始化
 * @param NULL
 * @return NULL
 */
void lvgl_disp_init(void)//显示屏初始化
{
    user_lcd_init();
}

/**
 * @brief 获取显示屏的分辨率
 * @param NULL
 * @return NULL
 */
void lvgl_disp_get_width_hight(int *width, int *hight)//显示屏分辨率获取
{
    if (width && hight) {
        *width = LCD_W;
        *hight = LCD_H;
    }
}

/**
 * @brief 显示屏刷新接口
 * @param NULL
 * @return NULL
 */
void lvgl_disp_flush(int xs, int xe, int ys, int ye, unsigned char *img)//往显示器些显示数据
{
    lcd_lvgl_full(xs, xe, ys, ye, img);
}

/**
 * @brief lvgl demo 主任务初始化
 * @param NULL
 * @return NULL
 */
void lvgl_test_demo(void)
{
    lvgl_main_task_init();//跑lvgl demo，在库里
}


/****************************LVGL：字库接口*********************************/
#define FONT24_PATH  CONFIG_UI_FONT_RES_FILE_PATH"Font24.bin"
#define FONT18_PATH  CONFIG_UI_FONT_RES_FILE_PATH"Font18.bin"
#define FONT20_PATH  CONFIG_UI_FONT_RES_FILE_PATH"Font20.bin"
#define FONT16_PATH  CONFIG_UI_FONT_RES_FILE_PATH"Font16.bin"

/**
 * @brief 字库24号读取
 * @param buf缓冲区，offset偏移量，size读取长度
 * @return 0成功，非0失败
 */
int lvgl_font24_file_read(char *buf, int offset, int size)
{
    PRODUCTION_TEST_FUNC_CHECK();
    int ret  = 0;
    static FILE *font24_fd = NULL;
    static int font24_seek_addr = 0;
    if (!font24_fd) {
        font24_fd = fopen(FONT24_PATH, "r");
    }
    if (!font24_fd) {
        printf("\n\n fopen err : %s\n", FONT24_PATH);
        return -1;
    }
    if (font24_seek_addr != offset) {
        fseek(font24_fd, offset, SEEK_SET);
        font24_seek_addr = offset;
    }
    ret = fread(buf, 1, size, font24_fd);
    if (ret <= 0) {
        return 0;
    }
    font24_seek_addr += ret;
    /*
    #define LVGL_FONT24_ADDR 0x5e3000 //Font固定预留区地址，务必和isd_config_rule.c的Font24.bin_ADR一致
        ret = size;
        if(norflash_read(NULL, buf, size, LVGL_FONT24_ADDR + offset) < 0){
            return 0;
        }
    */
    return ret;
}

/**
 * @brief 字库18号读取
 * @param buf缓冲区，offset偏移量，size读取长度
 * @return 0成功，非0失败
 */
int lvgl_font18_file_read(char *buf, int offset, int size)
{
    PRODUCTION_TEST_FUNC_CHECK();
    int ret  = 0;
    static FILE *font18_fd = NULL;
    static int font18_seek_addr = 0;
    if (!font18_fd) {
        font18_fd = fopen(FONT18_PATH, "r");
    }
    if (!font18_fd) {
        printf("\n\n fopen err : %s\n", FONT18_PATH);
        return -1;
    }
    if (font18_seek_addr != offset) {
        fseek(font18_fd, offset, SEEK_SET);
        font18_seek_addr = offset;
    }
    ret = fread(buf, 1, size, font18_fd);
    if (ret <= 0) {
        return 0;
    }
    font18_seek_addr += ret;
    /*
    #define LVGL_FONT18_ADDR 0x5e3000 //Font固定预留区地址，务必和isd_config_rule.c的Font18.bin_ADR一致
        ret = size;
        if(norflash_read(NULL, buf, size, LVGL_FONT18_ADDR + offset) < 0){
            return 0;
        }
    */
    return ret;
}


/****************************LVGL：音乐接口*********************************/
/**
 * @brief 显示暂停和播放图标
 * @param pause 1暂停，0播放
 * @return 成功返回0，失败返回非0
 */
int lv_demo_music_play_pause_button(char pause)
{
    PRODUCTION_TEST_FUNC_CHECK();
    screen_music_btn_play_pause_show(pause);
    screen_podcast_btn_play_pause_show(pause);
    return 0;
}

/**
 * @brief 更新音乐歌曲标题
 * @param title 文字
 * @return 成功返回0，失败返回非0
 */
int lv_demo_music_update_title(char *title)
{
    PRODUCTION_TEST_FUNC_CHECK();
    music_update_title(title);
    podcast_update_title(title);
    return 0;
}

/**
 * @brief 更新音乐歌曲艺术家
 * @param title 文字
 * @return 成功返回0，失败返回非0
 */
int lv_demo_music_update_artist(char *artist)
{
    PRODUCTION_TEST_FUNC_CHECK();
    music_update_artist(artist);
    return 0;
}

/**
 * @brief 更新音乐歌曲歌词
 * @param title 文字
 * @return 成功返回0，失败返回非0
 */
int lv_demo_music_update_lyric(char *lyric)
{
    PRODUCTION_TEST_FUNC_CHECK();
    music_update_genre(lyric);
    return 0;
}


/**
 * @brief 更新音乐歌曲总共时间
 * @param sec 时间秒
 * @return 成功返回0，失败返回非0
 */
int lv_demo_music_update_total_time(int sec)
{
    PRODUCTION_TEST_FUNC_CHECK();
    music_update_time(sec);
    podcast_update_time(sec);
    return 0;
}

/**
 * @brief 更新音乐歌曲当前时间
 * @param sec 时间秒
 * @return 成功返回0，失败返回非0
 */
int lv_demo_music_update_current_time(int sec)
{
    PRODUCTION_TEST_FUNC_CHECK();
    music_update_current_time(sec);
    podcast_update_current_time(sec);
    return 0;
}

/**
 * @brief 清空音乐类型，所有状态回复初始化
 * @param NULL
 * @return 成功返回0，失败返回非0
 */
int lv_demo_music_clean(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    return 0;
}

/**
 * @brief 处理暂停/播放功能
 * @param stop ：1停止，0播放
 * @return NULL
 */
void lv_demo_music_play_pause(char stop, char *type)
{
    printf("->lv_demo_music_play_pause : %s\n", type);
    struct key_event key = {0};
    key.type = KEY_EVENT_USER;
    key.action = KEY_EVENT_CLICK;

    if (ai_speaker_app()) {
        if (stop) {
            net_music_play_puase();
        } else {
            if (music_buf_play_supspend()) {
                net_music_play_resum();
            } else {
                net_music_play_next(type);
            }
        }
    } else {
        key.type = KEY_EVENT_USER;
        key.action = KEY_EVENT_CLICK;
        key.value = stop ? KEY_SUPSPEND : KEY_RESUM;
        key_event_notify(KEY_EVENT_FROM_USER, &key);
    }
}

/**
 * @brief 处理上下曲播放功能
 * @param next ：1下一曲，0上一曲
 * @return NULL
 */
void lv_demo_music_play(char next, char *type)
{
    printf("->lv_demo_music_play next : %s\n", type);
    struct key_event key = {0};
    key.type = KEY_EVENT_USER;
    key.action = KEY_EVENT_CLICK;
    if (next) {
        if (ai_speaker_app()) {
            net_music_play_next(type);
        } else {
            key.type = KEY_EVENT_USER;
            key.action = KEY_EVENT_CLICK;
            key.value = KEY_DOWN;
            key_event_notify(KEY_EVENT_FROM_USER, &key);
        }
    } else {
        if (ai_speaker_app()) {
            if (net_music_play_last_check()) {
                net_music_play_last_request();
            } else {
                net_music_play_next(type);
            }
        } else {
            key.type = KEY_EVENT_USER;
            key.action = KEY_EVENT_CLICK;
            key.value = KEY_UP;
            key_event_notify(KEY_EVENT_FROM_USER, &key);
        }
    }
}
/**
 * @brief 处理循环播放功能
 * @param enable ：1开启，0关闭
 * @return NULL
 */
void lv_demo_music_play_loop(char enable)
{
    struct key_event key = {0};
    key.type = KEY_EVENT_USER;
    key.action = KEY_EVENT_CLICK;
    if (ai_speaker_app()) {
        int net_music_play_loop_fore_set(char loop);
        if (enable) {
            net_music_play_loop_fore_set(true);
        } else {
            net_music_play_loop_fore_set(false);
        }
    } else {
        key.type = KEY_EVENT_USER;
        key.action = KEY_EVENT_CLICK;
        key.value = enable ? KEY_LOOP : KEY_EXIT_LOOP;
        key_event_notify(KEY_EVENT_FROM_USER, &key);
    }
}

/**
 * @brief 是否是音乐正在播放
 * @param NULL
 * @return 是返回true，否false
 */
int lv_demo_is_app_music_stop(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    int audio_app_all_play_stop_status(char mode);
    return audio_app_all_play_stop_status(1);
}

/**
 * @brief 检测APP是否正在处于网络播放功能
 * @param NULL
 * @return 1是 0否
 */
int lv_demo_net_music_is_start(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    return net_music_play_start_status();
}

/**
 * @brief 检测APP是否正在处于停止网络播放功能
 * @param NULL
 * @return 1是 0否
 */
int lv_demo_net_music_play_stop_status(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    return net_music_play_stop_status();
}

/**
 * @brief 检测APP是否正在播放音乐功能
 * @param NULL
 * @return 1是 0否
 */
void *lv_demo_net_music_play_type(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    return net_music_play_type_get();
}



/****************************LVGL：界面接口*********************************/

/**
 * @brief 主界面切换
 * @param NULL
 * @return 成功返回0，失败返回非0
 */
int lv_demo_switch_to_main_page(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    switch_to_main_page();
    return 0;
}

/**
 * @brief 音乐面切换
 * @param NULL
 * @return 成功返回0，失败返回非0
 */
int lv_demo_switch_to_music_page(void *priv)
{
    PRODUCTION_TEST_FUNC_CHECK();
    if (!priv || !strcmp(priv, "net_music") || !strcmp(priv, "bt_music") || !strcmp(priv, "sd_music") || !strcmp(priv, "aux_music") || !strcmp(priv, "usbdisk_music")) {
        switch_to_music_page(priv ? priv : "net_music");
    } else {
        printf("music type : %s \n", priv);
        char *type = strstr(priv, "musicians");
        if (type) {
            switch_to_music_page("net_music");
        } else {
            type = strstr(priv, "podcast");
            if (type) {
                type += strlen("podcast");
                if (*type == ':' || *type == '_') {
                    type++;
                }
                switch_to_podcast_page(type);//播客
            }
        }
    }
    return 0;
}

/**
 * @brief 定时器面切换
 * @param NULL
 * @return 成功返回0，失败返回非0
 */
int lv_demo_switch_to_timer_page(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    switch_to_timer_page();
    return 0;
}

/**
 * @brief 闹钟面切换
 * @param NULL
 * @return 成功返回0，失败返回非0
 */
int lv_demo_switch_to_alarm_page(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    switch_to_alarm_page();
    return 0;
}

/**
 * @brief 响铃面切换（闹钟和计时器）
 * @param NULL
 * @return 成功返回0，失败返回非0
 */
int lv_demo_switch_to_ring_page(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    switch_to_ring_page();
    return 0;
}

/**
 * @brief 切换到WiFi密码页面
 * @param NULL
 * @return 成功返回0，失败返回非0
 */
int lv_demo_switch_to_wifi_psw_page(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    //switch_to_wifi_psw_page();
    return 0;
}

/**
 * @brief 配网页面切换
 * @param NULL
 * @return 成功返回0，失败返回非0
 */
int lv_demo_switch_to_net_page(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    //switch_to_net_page();
    return 0;
}

/**
 * @brief 下拉框页面切换
 * @param NULL
 * @return 成功返回0，失败返回非0
 */
int lv_demo_switch_to_dropdown_page(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    switch_to_dropdown_page();
    return 0;
}
/*
 .....
*/

/****************************LVGL：是否处于某个界面接口**********************/

/**
 * @brief 是否是主页面
 * @param NULL
 * @return 是返回true，否false
 */
int lv_demo_is_main_page(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    return is_main_page();
}
/**
 * @brief 是否是对话页面
 * @param NULL
 * @return 是返回true，否false
 */
int lv_demo_is_conversation_page(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    return is_conversation_page();
}
/**
 * @brief 是否是睡眠音乐页面
 * @param NULL
 * @return 是返回true，否false
 */
int lv_demo_is_sleep_music_page(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    return 0;//return is_sleep_music_page();
}

/**
 * @brief 是否是音乐页面
 * @param NULL
 * @return 是返回true，否false
 */
int lv_demo_is_music_page(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    return is_music_page();
}

/**
 * @brief 是否是倒计时页面
 * @param NULL
 * @return 是返回true，否false
 */
int lv_demo_is_timer_page(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    return is_timer_page();
}

/**
 * @brief 是否是闹钟页面
 * @param NULL
 * @return 是返回true，否false
 */
int lv_demo_is_alarms_page(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    return is_alarms_page();
}

/**
 * @brief 是否是下拉/控制中心页面
 * @param NULL
 * @return 是返回非0，否0
 */
int lv_demo_is_dropdown_page(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    return is_dropdown_page();
}

/**
 * @brief 是否是响铃页面
 * @param NULL
 * @return 是返回true，否false
 */
int lv_demo_is_ring_page(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    return is_ring_page();
}


/*
 .....
*/


/****************************LVGL：闹钟接口*********************************/

/**
 * @brief 清空闹铃切面
 * @param NULL
 * @return 成功返回0，失败返回非0
 */
int lv_demo_alarms_ring_clean(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    void alarm_music_play_stop(void);
    alarm_music_play_stop();
    lv_demo_switch_to_main_page();
    printf("停止铃声播放\n");
    return 0;
}

/**
 * @brief 语音新增闹钟（语音设置的服务器下发闹钟数据）
 * @param NULL
 * @return 成功返回0，失败返回非0
 */
int lv_demo_alarms_add_flush_from_server(int hour, int minute, int second, int cyc)
{
    PRODUCTION_TEST_FUNC_CHECK();
    return Voice_dynamic_add_new_alarm(hour, minute, second, cyc);
}

/**
 * @brief 语音删除闹钟（语音设置的服务器下发闹钟数据）
 * @param NULL
 * @return 成功返回0，失败返回非0
 */
int lv_demo_alarms_del_flush_from_server(int hour, int minute, int second)
{
    PRODUCTION_TEST_FUNC_CHECK();
    return Voice_delete_alarm(hour, minute, second);
}

/**
 * @brief 语音删除所有闹钟（语音设置的服务器下发闹钟数据）
 * @param NULL
 * @return 成功返回0，失败返回非0
 */
int lv_demo_all_alarms_del_flush_from_server(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    clear_all_alarms();
    return 0;
}

/**
 * @brief 删除不循环的单个闹钟
 * @param NULL
 * @return 成功返回0，失败返回非0
 */
int lv_demo_delete_once_alarm(int hour, int minute, int second)
{
    PRODUCTION_TEST_FUNC_CHECK();
    Delete_once_alarm_ui(hour, minute, second);
    return 0;
}

/****************************LVGL：计时器接口*********************************/

/**
 * @brief 计时器添加
 * @param sec秒
 * @return 成功返回0，失败返回非0
 */
int lv_demo_timer_add_sec(int sec)
{
    PRODUCTION_TEST_FUNC_CHECK();
#if TOMATO_TIMER_ENABLE
    Voice_dynamic_add_new_tomato_timer(sec);
#else
    Voice_dynamic_add_new_timer(sec);
#endif
    return 0;
}


/****************************LVGL：AI对话文本接口*********************************/

/**
 * @brief AI对话的语言识别STT文本
 * @param NULL
 * @return 成功返回0，失败返回非0
 */
int lv_demo_ai_dialogue_stt(char *text, int len)
{
    PRODUCTION_TEST_FUNC_CHECK();
    async_user_add_chat_message(text);
    return 0;
}

/**
 * @brief AI对话的回复TTS文本
 * @param NULL
 * @return 成功返回0，失败返回非0
 */
int lv_demo_ai_dialogue_tts(char *text, int len)
{
    PRODUCTION_TEST_FUNC_CHECK();
    async_ai_add_chat_message(text);
    return 0;
}


/**
 * @brief 启动AI对话
 * @param mode ： 0唤醒，1按键唤醒
 * @return 成功返回0，失败返回非0
 */
int lv_demo_ai_dialogue_start_mode(char mode)
{
    PRODUCTION_TEST_FUNC_CHECK();
    async_user_add_chat_message(" ");
    return 0;
}

/**
 * @brief AI对话的回复TTS文本
 * @param NULL
 * @return 成功返回0，失败返回非0
 */
int lv_demo_ai_dialogue_start(char index)
{
    PRODUCTION_TEST_FUNC_CHECK();
#ifndef CONFIG_USE_TTS_REPLY_ENABLE
    char *hello[] = {
        "来喽",
        "在呢",
        "你说",
        "我在听",
        "嗯？",
    };
    async_ai_add_chat_message(hello[index]);
#else
    async_ai_add_chat_message("正在聆听...");
#endif
    async_speak_listen_icon_show(1);
    switch_to_conversation_page();
    return 0;
}

/**
 * @brief 启动AI对话
 * @param mode ： 0唤醒，1按键唤醒
 * @return 成功返回0，失败返回非0
 */
int lv_demo_ai_dialogue_speak_listen(char mode)
{
    PRODUCTION_TEST_FUNC_CHECK();
    async_speak_listen_icon_show(mode);
    return 0;
}


/****************************LVGL：电池接口*********************************/
/**
 * @brief 电池检测接口
 * @param percent ：0-100 代表电池的百分比
 * @param charge_in ：1充电插入, 0充电拔出
 * @param charge_full ：1已充满, 0未充满
 * @return 0
 */
int lv_demo_battery_percent_update(int percent, int charge_in, int charge_full)
{
    PRODUCTION_TEST_FUNC_CHECK();
    int p = ((int)percent << 2) | (charge_in << 1) | charge_full;
    void battery_update_sync(void *p);
    battery_update_sync((void *)p);
    return 0;
}

/****************************LVGL：升级接口*********************************/
/**
 * @brief 电池检测接口
 * @param percent ：0-100 代表系统升级过程的百分比
 * @param err_code ：0-正在升级，1升级成功，-1升级失败，-2升级超时(失败)
 * @return 0
 */
int lv_demo_system_update_percent(int percent, int err_code)
{
    PRODUCTION_TEST_FUNC_CHECK();
    switch (err_code) {
    case 0://正在升级
        break;
    case 1://升级成功
        break;
    case -1://升级失败
        break;
    case -2://升级超时(失败)
        break;
    }
    return 0;
}

/****************************LVGL：网络接口*********************************/
/**
 * @brief 检测网络网卡接口
 * @param NULL
 * @return 0 : wifi接口，1 : 4G网卡接口
 */
int lv_demo_system_net_interface(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    int net_ch = 0;
#ifdef CONFIG_LTE_PHY_ENABLE
    net_ch = sys_net_channel_read();
#endif
    return net_ch;
}

/**
 * @brief 网络检测是否连上互联网接口
 * @param NULL
 * @return 0 : 没联网，1 : 已经联网
 */
int lv_demo_system_net_connect_success(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    return sys_connect_net_success();
}

/****************************LVGL：BT蓝牙接口*********************************/
/**
 * @brief 蓝牙连接检测接口
 * @param NULL
 * @return 0 : 无连接，1 : 已连接
 */
int lv_demo_system_bt_connect(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    extern char bt_connect_check(void);
    return bt_connect_check();
}

/**
 * @brief 获取蓝牙正在播放接口
 * @param NULL
 * @return 0 : 停止播放，1 : 正在播放
 */
int lv_demo_system_bt_music_playing(void)
{
    PRODUCTION_TEST_FUNC_CHECK();
    extern int bt_music_play_get_pause(void);
    return !bt_music_play_get_pause();
}


#endif


