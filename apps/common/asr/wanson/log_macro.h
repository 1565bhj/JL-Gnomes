#ifndef LOG_MACRO_H
#define LOG_MACRO_H

// #include <stdio.h>
// #include <stdlib.h>
/******************** 基础定义 ********************/
#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO  1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_ERROR 3

#ifndef GLOBAL_LOG_LEVEL
#define GLOBAL_LOG_LEVEL LOG_LEVEL_DEBUG
#endif

#ifndef TAG
#define TAG "DEFAULT_TAG"
#endif

// 颜色
#define LOG_COLOR_DEBUG   "\033[1;36m"  // 青色
#define LOG_COLOR_INFO    "\033[1;32m"  // 绿色
#define LOG_COLOR_WARN    "\033[1;33m"  // 黄色
#define LOG_COLOR_ERROR   "\033[1;31m"  // 红色
#define LOG_COLOR_RESET   "\033[0m"     // 重置颜色

#define WANSON_LOG(level, format, ...) \
    do { \
        if (level >= GLOBAL_LOG_LEVEL) { \
            const char* color = (level == LOG_LEVEL_DEBUG) ? LOG_COLOR_DEBUG : \
                               (level == LOG_LEVEL_INFO)  ? LOG_COLOR_INFO  : \
                               (level == LOG_LEVEL_WARN)  ? LOG_COLOR_WARN  : LOG_COLOR_ERROR; \
            const char* level_str = (level == LOG_LEVEL_DEBUG) ? "DEBUG" : \
                                   (level == LOG_LEVEL_INFO)  ? "INFO"  : \
                                   (level == LOG_LEVEL_WARN)  ? "WARN"  : "ERROR"; \
            printf("%s[%s][%s] " format "%s", \
                   color, TAG, level_str, ##__VA_ARGS__, LOG_COLOR_RESET); \
        } \
    } while(0)

#define WANSON_LOG_D(format, ...) WANSON_LOG(LOG_LEVEL_DEBUG, format "\n", ##__VA_ARGS__)
#define WANSON_LOG_I(format, ...) WANSON_LOG(LOG_LEVEL_INFO,  format "\n", ##__VA_ARGS__)
#define WANSON_LOG_W(format, ...) WANSON_LOG(LOG_LEVEL_WARN,  format "\n", ##__VA_ARGS__)
#define WANSON_LOG_E(format, ...) WANSON_LOG(LOG_LEVEL_ERROR, format "\n", ##__VA_ARGS__)

#endif // LOG_MACRO_H