#ifndef LV_GL_DEVELOP_H
#define LV_GL_DEVELOP_H


/** 1：睡眠音乐屏顶部「30分钟」「90分钟」两个 label（与 Guider 生成代码里 #if GUIDER_UI_SLEEP_MUSIC_DURATION_LABELS 一致）；0：二者均不编译/不创建 */
#if CONFIG_LVGL_SLEEP_MUSIC_ENABLE
#define GUIDER_UI_SLEEP_MUSIC_DURATION_LABELS   1
/** 睡眠音乐屏：两个定时档位（分钟），与 label 文案及倒计时一致；可在本文件或 app_config 先 #undef 再 #define 覆盖 */
#ifndef SLEEP_MUSIC_DURATION_MINUTES_SHORT
#define SLEEP_MUSIC_DURATION_MINUTES_SHORT  30
#endif
#ifndef SLEEP_MUSIC_DURATION_MINUTES_LONG
#define SLEEP_MUSIC_DURATION_MINUTES_LONG   90
#endif
#define SLEEP_MUSIC_DURATION_MS_SHORT       ((uint32_t)(SLEEP_MUSIC_DURATION_MINUTES_SHORT * 60U * 1000U))
#define SLEEP_MUSIC_DURATION_MS_LONG        ((uint32_t)(SLEEP_MUSIC_DURATION_MINUTES_LONG * 60U * 1000U))
#define SLEEP_MUSIC_DURATION_BLUE_SHORT     ((int)SLEEP_MUSIC_DURATION_MINUTES_SHORT)
#define SLEEP_MUSIC_DURATION_BLUE_LONG      ((int)SLEEP_MUSIC_DURATION_MINUTES_LONG)
#endif


#if CONFIG_LVGL_OPTION_WIFI_ENABLE
/** WiFi 密码页键盘输入框：1 以替换字符显示（如 *）；0 明文显示（便于调试）。仅 CONFIG_LVGL_OPTION_WIFI_ENABLE 时生效。 */
#define CONFIG_LVGL_WIFI_PASSWORD_MASK_ENABLE       0
/** WiFi 密码页「显隐密码」imgbtn_psw：1 创建控件；0 不创建。仅 CONFIG_LVGL_OPTION_WIFI_ENABLE 时生效。 */
#define CONFIG_LVGL_WIFI_PASSWORD_IMGBTN_ENABLE     0
#endif


#ifndef CONFIG_LVGL_CONTENT_LIST_296_ENABLE
#define CONFIG_LVGL_CONTENT_LIST_296_ENABLE     0
#endif

#if CONFIG_LVGL_CONTENT_LIST_296_ENABLE
#define CONFIG_LVGL_CONTENT_LIST_X_POS     10
#else
#define CONFIG_LVGL_CONTENT_LIST_X_POS     25
#endif

/* 1：缩放随位置连续变化，松手右滑→下一项到中间、左滑→上一项；0：仅大小两档(与 OPTION_ICON_MOVE 对齐)，且左右滑切换项与上式对调 */
#define OPTION_ICON_ZOOM_GRADIENT    1  /* 改为 1：连续缩放 + 另一套左右滑方向 */

/* 主界面静音状态角标（screen_main_img_nomute）；0 关闭以省对象与逻辑 */
#ifndef UI_SHOW_SCREEN_MAIN_NOMUTE_ICON
#define UI_SHOW_SCREEN_MAIN_NOMUTE_ICON  1
#endif
/* 下拉页静音角标（screen_dropdown_img_nomute）；与上项独立 */
#ifndef UI_SHOW_SCREEN_DROPDOWN_NOMUTE_ICON
#define UI_SHOW_SCREEN_DROPDOWN_NOMUTE_ICON  1
#endif




#endif  /* LV_GL_DEVELOP_H */
