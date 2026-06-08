#ifndef _OAL_LOG_H_
#define _OAL_LOG_H_

#include "oal_type.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// String
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define OAL_LOG_NONE    0x00
#define OAL_LOG_DEBUG   0x01
#define OAL_LOG_INFO    0x02
#define OAL_LOG_ERROR   0x04

int get_printf_level();
void set_printf_level(int level);

#define OAL_assert(x) if(!(x))OAL_log("!!!!!! OAL_assert: %d, %s, %d",x, __FILE__, __LINE__)

#define OAL_printf printf

#define OAL_log(format, ...) \
do { \
    if(OAL_LOG_NONE != get_printf_level()) \
        printf("[OAL_log] " format "", ##__VA_ARGS__ ); \
}while(0)

#define OAL_log_color(color, format, ...) \
do { \
    if(OAL_LOG_NONE != get_printf_level()) \
        printf("[OAL_log_color] \033[%sm" format "\033[0m",color, ##__VA_ARGS__ ); \
}while(0)

// "\033[41;36m something here \033[0m"

#define OAL_logx(level, format, ...) do{ \
    if(level & get_printf_level()) \
        printf("[%s:OAL_logx][%s(%d)]:"format, #level, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
}while(0)

#endif


