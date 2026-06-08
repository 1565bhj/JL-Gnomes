#include "app_config.h"
#include "system/includes.h"
#include "device/device.h"
#include <time.h>
#include <sys/time.h>
#include "syscfg/syscfg_id.h"
#include "event/key_event.h"
#include "event/net_event.h"
#include "os/os_api.h"
#include "event/device_event.h"
#include "asm/system_reset_reason.h"
#include "ai_uart_ctrol.h"

#ifdef CONFIG_WIFI_ENABLE
#include "wifi/wifi_connect.h"
#endif

enum utc {
    //西区时间
    UTC_WEST12 = -12,
    UTC_WEST11 = -11,
    UTC_WEST10 = -10,
    UTC_WEST9 = -9,
    UTC_WEST8 = -8,
    UTC_WEST7 = -7,
    UTC_WEST6 = -6,
    UTC_WEST5 = -5,
    UTC_WEST4 = -4,
    UTC_WEST3 = -3,
    UTC_WEST2 = -2,
    UTC_WEST1 = -1,
    UTC0  = 0,
    //东区时间
    UTC_EAST1 = 1,
    UTC_EAST2 = 2,
    UTC_EAST3 = 3,
    UTC_EAST4 = 4,
    UTC_EAST5 = 5,
    UTC_EAST6 = 6,
    UTC_EAST7 = 7,
    UTC_EAST8 = 8,
    UTC_EAST9 = 9,
    UTC_EAST10 = 10,
    UTC_EAST11 = 11,
    UTC_EAST12 = 12,
};
enum msg {
    ALARM_INIT = 0,
    ALARM_UPDATE_TIME,
    ALARM_UPDATE_TIME_RTC,
    ALARM_ADD,
    ALARM_SAVE,
    ALARM_DEL,
    ALARM_CLEAN,
    ALARM_NOTIFY,
    ALARM_UPDATE2SERVER,
};

static u8 UTC_USE = UTC_EAST8;

//#define UTC_USE             (UTC_EAST8) //东八区时间
#define UTC_USE_DIFF_SEC    (UTC_USE*60*60) //东八区时间差的秒数

#define ALARM_NUM   10  //最多10个闹钟，10*12 < 256

struct alarm_info { //12字节
    unsigned char enable: 1; //是否使能闹钟
    unsigned char music: 1; //闹钟自定义音乐，0系统音乐
    unsigned char index: 4; //设置的闹钟index
    unsigned char resver: 2; //预留
    unsigned char cyc; //是否循环的闹钟: 0不循环，BIT(0)每天循环，BIT(1)-BIT(7)对应星期一到星期日
    unsigned char resver2; //预留
    struct sys_time time;
};
#define ALARM_MP3_URL_SIZE  1024
static struct sys_time alarm_wakeup_time = {0};
static char now_weekday = 0;
static char mp3_url[ALARM_MP3_URL_SIZE];
static char alarm_content_buf[240];
static struct alarm_info alarm_tab[ALARM_NUM] ALIGNED(4) = {0};
static OS_MUTEX mutex;

static int alarm_msg(int message, char notice);
extern int ntp_client_get_time_all(struct tm *s_tm, int recv_to);
static void alarm_rings(void *priv);
void ntptime_update(void);
void ntptime_update_rtc(void);
int64_t http_ntp_utc_time_get(void);

// 外部可调用的删除闹钟函数，通过索引删除
int alarm_delete_by_index(int index);
// 外部可调用的删除闹钟函数，通过时间删除
int alarm_delete_by_time(struct sys_time *time);
#define SECOND_OF_DAY (24*60*60)
static const char *weekday[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
static const char DayOfMon[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void sys_sec_to_date(long lSec, struct sys_time *tTime)
{
    unsigned short i, j, iDay;
    unsigned long lDay;

    lDay = lSec / SECOND_OF_DAY;
    lSec = lSec % SECOND_OF_DAY;

    i = 1970;
    while (lDay > 365) {
        if (((i % 4 == 0) && (i % 100 != 0)) || (i % 400 == 0)) {
            lDay -= 366;
        } else {
            lDay -= 365;
        }
        i++;
    }
    if ((lDay == 365) && !(((i % 4 == 0) && (i % 100 != 0)) || (i % 400 == 0))) {
        lDay -= 365;
        i++;
    }
    tTime->year = i;
    for (j = 0; j < 12; j++) {
        if ((j == 1) && (((i % 4 == 0) && (i % 100 != 0)) || (i % 400 == 0))) {
            iDay = 29;
        } else {
            iDay = DayOfMon[j];
        }
        if (lDay >= iDay) {
            lDay -= iDay;
        } else {
            break;
        }
    }
    tTime->month = j + 1;
    tTime->day = lDay + 1;
    tTime->hour = ((lSec / 3600)) % 24; //这里注意，世界时间已经加上北京时间差8，
    tTime->min = (lSec % 3600) / 60;
    tTime->sec = (lSec % 3600) % 60;
}
uint8_t sys_time_to_weekday(struct sys_time *t)
{
    uint32_t u32WeekDay = 0;
    uint32_t u32Year = t->year;
    uint8_t u8Month = t->month;
    uint8_t u8Day = t->day;
    if (u8Month < 3U) {
        /*D = { [(23 x month) / 9] + day + 4 + year + [(year - 1) / 4] - [(year - 1) / 100] + [(year - 1) / 400] } mod 7*/
        u32WeekDay = (((23U * u8Month) / 9U) + u8Day + 4U + u32Year + ((u32Year - 1U) / 4U) - ((u32Year - 1U) / 100U) + ((u32Year - 1U) / 400U)) % 7U;
    } else {
        /*D = { [(23 x month) / 9] + day + 4 + year + [year / 4] - [year / 100] + [year / 400] - 2 } mod 7*/
        u32WeekDay = (((23U * u8Month) / 9U) + u8Day + 4U + u32Year + (u32Year / 4U) - (u32Year / 100U) + (u32Year / 400U) - 2U) % 7U;
    }
    if (0U == u32WeekDay) {
        u32WeekDay = 7U;
    }
    return (uint8_t)u32WeekDay;
}
static void time_ntp_utc(struct sys_time *t, char ntp2utc)
{
    char days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int offset = UTC_USE;
    int leap, max_day;

    // 1. 转换年份和月份格式
    if (ntp2utc) {
        t->year += 1900;
        t->month += 1;
        t->hour += offset;
    } else {
        t->hour -= offset;
    }

    // 2. 调整小时和日期
    if (t->hour >= 24) {
        t->hour -= 24;
        t->day++;
    } else if (t->hour < 0) {
        t->hour += 24;
        t->day--;
    }

    // 3. 处理日期借位
    while (t->day <= 0) {
        if (--t->month <= 0) {
            t->month = 12;
            t->year--;
        }
        leap = ((t->year % 4 == 0 && t->year % 100 != 0) || t->year % 400 == 0);
        t->day += (t->month == 2) ? (leap ? 29 : 28) : days[t->month - 1];
    }

    // 4. 处理日期进位
    while (1) {
        leap = ((t->year % 4 == 0 && t->year % 100 != 0) || t->year % 400 == 0);
        max_day = (t->month == 2) ? (leap ? 29 : 28) : days[t->month - 1];
        if (t->day <= max_day) {
            break;
        }
        t->day -= max_day;
        if (++t->month > 12) {
            t->month = 1;
            t->year++;
        }
    }

    // 5. 转换回NTP格式（如果需要）
    if (!ntp2utc) {
        t->year -= 1900;
        t->month -= 1;
    }
}
void time_day_add(struct sys_time *t, int add)
{
    char days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int leap, max_day, remaining;
    t->day += add;
    while (t->day > 0) {
        leap = ((t->year % 4 == 0 && t->year % 100 != 0) || t->year % 400 == 0);
        max_day = (t->month == 2) ? (leap ? 29 : 28) : days[t->month - 1];

        if (t->day <= max_day) {
            break;
        }

        t->day -= max_day;
        if (++t->month > 12) {
            t->month = 1;
            t->year++;
        }
    }
}

/**
 * 带时区的时间戳转换函数（完整实现，包含星期和所有功能）
 * @param timep 时间戳指针
 * @param timezone 时区偏移（小时），例如：8表示UTC+8，-5表示UTC-5
 * @param result 指向struct tm的指针，用于存储转换结果
 * @param year_real 转换结果年月为真实时间
 * @return 返回result指针
 */
int timestamp_to_tm(time_t *timep, int timezone, struct tm* result, char year_real)
{
    char days_in_month[] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };

    // 应用时区偏移
    time_t local_timestamp = *timep + (timezone * 3600);
    int seconds_per_day = 86400;
    int days, remaining_seconds;
    int year, month, day, hour, minute, second;
    int i;
    int month_days;

    if (result == NULL) {
        return NULL;
    }

    /* 计算天数（从1970-01-01开始） */
    days = local_timestamp / seconds_per_day;
    remaining_seconds = local_timestamp % seconds_per_day;
    if (remaining_seconds < 0) {
        remaining_seconds += seconds_per_day;
        days--;
    }

    /* 计算时分秒 */
    hour = remaining_seconds / 3600;
    minute = (remaining_seconds % 3600) / 60;
    second = remaining_seconds % 60;

    /* 计算年份 */
    year = 1970;
    while (1) {
        // 判断闰年
        int is_leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        int year_days = is_leap ? 366 : 365;
        if (days >= year_days) {
            days -= year_days;
            year++;
        } else {
            break;
        }
    }

    /* 计算月份和日期 */
    for (month = 1; month <= 12; month++) {
        // 获取当月天数
        month_days = days_in_month[month - 1];
        if (month == 2) {
            int is_leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
            if (is_leap) {
                month_days = 29;
            }
        }
        if (days >= month_days) {
            days -= month_days;
        } else {
            break;
        }
    }
    day = days + 1;

    /* 填充struct tm结构体 */
    result->tm_sec = second;
    result->tm_min = minute;
    result->tm_hour = hour;
    result->tm_mday = day;
    result->tm_mon = year_real ? month : (month - 1);
    result->tm_year = year_real ? year : (year - 1900);

    /* 计算星期几 (1970-01-01是星期四，即tm_wday=4) */
    result->tm_wday = (4 + (local_timestamp / seconds_per_day)) % 7;
    if (result->tm_wday < 0) {
        result->tm_wday += 7;
    }

    /* 计算年中第几天 */
    result->tm_yday = 0;
    for (i = 1; i < month; i++) {
        int month_days_count = days_in_month[i - 1];
        if (i == 2) {
            int is_leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
            if (is_leap) {
                month_days_count = 29;
            }
        }
        result->tm_yday += month_days_count;
    }
    result->tm_yday += (day - 1);

    result->tm_isdst = 0;

    return result->tm_yday;
}


static int utc_time_update(void *alarm_t, char index, struct sys_time *new_time)//校准UTC网络时间
{
    struct sys_time test_rtc_time = {0};
    struct sys_time *alarm_time = (struct sys_time *)alarm_t;
    struct tm s_tm = {0};
    time_t timestamp;
    int ret = -1;
    int64_t time_sec = 0;

    /* 打开RTC设备 */
    os_mutex_pend(&mutex, 1000);
    void *rtc_hdl = dev_open("rtc", NULL);
    if (!rtc_hdl) {
        printf("err in rtc_hdl_open rtc\n");
        os_mutex_post(&mutex);
        return -1;
    }
    extern void set_alarm_ctrl(u8 set_alarm);
    /* 注册闹钟响铃回调函数 */
    set_rtc_isr_callback(alarm_rings, (void*)index);

    struct sys_time get_alarm_time;
    dev_ioctl(rtc_hdl, IOCTL_GET_ALARM, (u32)&get_alarm_time);


    /* 获取时间信息 */
    dev_ioctl(rtc_hdl, IOCTL_GET_SYS_TIME, (u32)&test_rtc_time);
    extern int sys_connect_net_success(void);
    if (sys_connect_net_success()) {
        if (qyai_chat_addr_version() >= 1) {
            time_sec = http_ntp_utc_time_get();
            if (time_sec <= 0) {
                os_time_dly(10);
                time_sec = http_ntp_utc_time_get();
            }
            if (time_sec > 0) {
                timestamp = time_sec;
                timestamp_to_tm(&timestamp, 0, &s_tm, 0);
            }
        }
        ret = time_sec > 0 ? 0 : -1;
        if (ret) {
            ret = ntp_client_get_time_all(&s_tm, 5000);
        }
        if (!ret) {
            test_rtc_time.year = s_tm.tm_year;
            test_rtc_time.month = s_tm.tm_mon;
            test_rtc_time.day = s_tm.tm_mday;
            test_rtc_time.hour = s_tm.tm_hour;
            test_rtc_time.min = s_tm.tm_min;
            test_rtc_time.sec = s_tm.tm_sec;

            if (timestamp) {
                UTC_USE = UTC0;
            }
            time_ntp_utc(&test_rtc_time, 1);

            if (timestamp) {
                UTC_USE = qyai_chat_addr_version() == 0 ? UTC_EAST8 : UTC0;
            }
            printf("get_NTP_time: %d-%d-%d %d:%d:%d\n",
                   test_rtc_time.year,
                   test_rtc_time.month,
                   test_rtc_time.day,
                   test_rtc_time.hour,
                   test_rtc_time.min,
                   test_rtc_time.sec);
            dev_ioctl(rtc_hdl, IOCTL_SET_SYS_TIME, (u32)&test_rtc_time);
            now_weekday = sys_time_to_weekday(&test_rtc_time);
            if (new_time) {
                memcpy(new_time, &test_rtc_time, sizeof(test_rtc_time));
            }
        } else {
            ntptime_update_rtc();
        }
    } else {
        if (new_time) {
            memcpy(new_time, &test_rtc_time, sizeof(test_rtc_time));
        }
        printf("get_local_rtc_time: %d-%d-%d %d:%d:%d\n",
               test_rtc_time.year,
               test_rtc_time.month,
               test_rtc_time.day,
               test_rtc_time.hour,
               test_rtc_time.min,
               test_rtc_time.sec);
    }
    /* 设置时间信息  */
    if (alarm_time) {

        /* 打开闹钟开关 */
        set_alarm_ctrl(1);
        /* 设置闹钟 */
        dev_ioctl(rtc_hdl, IOCTL_SET_ALARM, (u32)alarm_time);
        printf("alarm_time: %d-%d-%d %d:%d:%d\n",
               alarm_time->year,
               alarm_time->month,
               alarm_time->day,
               alarm_time->hour,
               alarm_time->min,
               alarm_time->sec);

        struct sys_time get_alarm_time;
        dev_ioctl(rtc_hdl, IOCTL_GET_ALARM, (u32)&get_alarm_time);
        printf("get alarm_time: %d-%d-%d %d:%d:%d\n",
               get_alarm_time.year,
               get_alarm_time.month,
               get_alarm_time.day,
               get_alarm_time.hour,
               get_alarm_time.min,
               get_alarm_time.sec);
    }

    dev_close(rtc_hdl);
    os_mutex_post(&mutex);
    return ret;
}
static int alarm_wkup_rtc_time_init(struct sys_time *alarm_wkup_time)
{
    struct sys_time test_rtc_time = {0};
    struct tm s_tm = {0};
    int ret = 0;

    if (system_reset_reason_get() == SYS_RST_ALM_WKUP) {
        /* 打开RTC设备 */
        os_mutex_pend(&mutex, 1000);
        void *rtc_hdl = dev_open("rtc", NULL);
        if (!rtc_hdl) {
            printf("err in rtc_hdl_open rtc\n");
            os_mutex_post(&mutex);
            return -1;
        }
        extern void set_alarm_ctrl(u8 set_alarm);
        /* 注册闹钟响铃回调函数 */
        set_rtc_isr_callback(alarm_rings, (void*)index);

        if (alarm_wakeup_time.year == 0) { //开机第一次先读取，第二次调用则设置RTC时间
            dev_ioctl(rtc_hdl, IOCTL_GET_ALARM, (u32)&alarm_wakeup_time);
            printf("->get alarm_time: %d-%d-%d %d:%d:%d\n",
                   alarm_wakeup_time.year,
                   alarm_wakeup_time.month,
                   alarm_wakeup_time.day,
                   alarm_wakeup_time.hour,
                   alarm_wakeup_time.min,
                   alarm_wakeup_time.sec);
        }
        dev_ioctl(rtc_hdl, IOCTL_SET_SYS_TIME, (u32)&alarm_wakeup_time);
        now_weekday = sys_time_to_weekday(&alarm_wakeup_time);
        /* 获取时间信息 */
        dev_ioctl(rtc_hdl, IOCTL_GET_SYS_TIME, (u32)&test_rtc_time);
        printf("->set_local_time init: %d-%d-%d %d:%d:%d\n",
               test_rtc_time.year,
               test_rtc_time.month,
               test_rtc_time.day,
               test_rtc_time.hour,
               test_rtc_time.min,
               test_rtc_time.sec);
        dev_close(rtc_hdl);
        os_mutex_post(&mutex);
        if (alarm_wkup_time) {
            memcpy(alarm_wkup_time, &test_rtc_time, sizeof(struct sys_time));
        }
    }
    return ret;
}
static void alarm_add(struct alarm_info *alarm, void *alarm_content)
{
    struct alarm_info *alarm_tmp;
    char set = 0;
    os_mutex_pend(&mutex, 1000);

    if ((alarm->cyc != 0) && (alarm->cyc != 1)) {//星期一到星期日
        if ((alarm->cyc & 0xFE) != 0xFE) {//星期一到星期日
            printf("-> save111 alarm: %d-%d-%d %d:%d:%d, cyc = %x\n",
                   alarm->time.year,
                   alarm->time.month,
                   alarm->time.day,
                   alarm->time.hour,
                   alarm->time.min,
                   alarm->time.sec,
                   alarm->cyc);

            char find = 0;
            printf("-> now_weekday = %d \n", now_weekday);
            for (char i = now_weekday; i <= 7; i++) {
                if ((alarm->cyc & BIT(i))) {
                    if (i != now_weekday) {
                        time_day_add(&alarm->time, i - now_weekday);
                    }
                    find = 1;
                    ///printf("-> save4444444 i =%d \n", i);
                    break;
                }
            }
            if (!find) {
                for (char i = 1; i < now_weekday; i++) {
                    if ((alarm->cyc & BIT(i))) {
                        if (i != now_weekday) {
                            time_day_add(&alarm->time, 7 - now_weekday + i);
                        }
                        find = 1;
                        //printf("-> save3333 i =%d \n", i);
                        break;
                    }
                }
            }
            if (!find) {
                time_day_add(&alarm->time, 7);
            }
            printf("-> save222 alarm : %d-%d-%d %d:%d:%d, cyc = %x\n",
                   alarm->time.year,
                   alarm->time.month,
                   alarm->time.day,
                   alarm->time.hour,
                   alarm->time.min,
                   alarm->time.sec,
                   alarm->cyc);
        }
    }

    syscfg_read(CFG_USER_ALARM, &alarm_tab, sizeof(alarm_tab));
    for (int i = 0; i < ALARM_NUM; i++) {
        alarm_tmp = &alarm_tab[i];
        if (!alarm_tmp->enable) {
            memcpy(alarm_tmp, alarm, sizeof(struct alarm_info));
            alarm_tmp->enable = 1;
            printf("-> save alarm %d : %d-%d-%d %d:%d:%d, cyc = %d\n", i,
                   alarm_tmp->time.year,
                   alarm_tmp->time.month,
                   alarm_tmp->time.day,
                   alarm_tmp->time.hour,
                   alarm_tmp->time.min,
                   alarm_tmp->time.sec,
                   alarm_tmp->cyc);
            syscfg_write(CFG_USER_ALARM, &alarm_tab, sizeof(alarm_tab));
            if (alarm_content && strlen(alarm_content) < sizeof(alarm_content_buf)) {
                printf("-> alarm_time_write index = %d : %s \n", index, alarm_content_buf);
                syscfg_write(CFG_USER_ALARM_CONTENT0 + i, alarm_content, strlen(alarm_content) + 1);
            } else {
                memset(alarm_content_buf, 0xFF, sizeof(alarm_content_buf));
                alarm_content_buf[0] = 0;
                syscfg_write(CFG_USER_ALARM_CONTENT0 + i, alarm_content_buf, sizeof(alarm_content_buf));
            }
            set = 1;
            break;
        }
    }
    if (!set) { //全写满了，替换第一个了
        memcpy(&alarm_tab[0], alarm, sizeof(struct alarm_info));
        syscfg_write(CFG_USER_ALARM, &alarm_tab, sizeof(alarm_tab));
        set = 1;
    }
    os_mutex_post(&mutex);
    if (set) {
        alarm_msg(ALARM_ADD, 0);//设置闹钟
    }
}
static int alarm_del(struct alarm_info *alarm, int only_hm)
{
    struct alarm_info *alarm_tmp;
    char del = 0;
    os_mutex_pend(&mutex, 1000);
    syscfg_read(CFG_USER_ALARM, &alarm_tab, sizeof(alarm_tab));
    for (int i = 0; i < ALARM_NUM; i++) {
        alarm_tmp = &alarm_tab[i];
        if ((alarm->cyc == alarm_tmp->cyc) || only_hm) {//星期一到星期日
            if (alarm->time.hour == alarm_tmp->time.hour &&
                (alarm->time.min >= alarm_tmp->time.min && alarm->time.min <= (alarm_tmp->time.min + 2))) {
                goto  __del;
            }
        } else if (alarm->time.hour == alarm_tmp->time.hour && (alarm->time.min >= alarm_tmp->time.min && alarm->time.min <= (alarm_tmp->time.min + 2))) { //允许2的分钟之内删除，不允许两个闹钟间隔时间小于等于2分钟
            if (alarm->time.year == alarm_tmp->time.year &&
                alarm->time.month == alarm_tmp->time.month &&
                alarm->time.day == alarm_tmp->time.day &&
                alarm->time.hour == alarm_tmp->time.hour &&
                (alarm->time.min >= alarm_tmp->time.min && alarm->time.min <= (alarm_tmp->time.min + 2))) { //允许2的分钟之内删除，不允许两个闹钟间隔时间小于等于2分钟

__del:
                printf("-> find delet alarm %d : %d-%02d-%02d %d:%d:%d\n", i,
                       alarm_tmp->time.year, alarm_tmp->time.month, alarm_tmp->time.day,
                       alarm_tmp->time.hour, alarm_tmp->time.min, alarm_tmp->time.sec);
#ifdef AI_UART_CMD_CTROL_ENABLE
#define ALARM_DEL_CJSON "{\"alarm_del\": \"%d-%02d-%02d %02d:%02d:%02d\"}"
                char sbuf[64];
                snprintf(sbuf, sizeof(sbuf), ALARM_DEL_CJSON, alarm->time.year, alarm->time.month, alarm->time.day, alarm->time.hour, alarm->time.min, alarm->time.sec);
                ai_uart_cmd_data_push(AI_UART_CMD_ALARM_DEL, sbuf, strlen(sbuf) + 1);
#endif
#ifdef AT_UART_CMD_ENABLE
#define ALARM_DEL_CJSON "{\"alarm_del\": \"%d-%02d-%02d %02d:%02d:%02d\"}"
                char sbuf[64];
                snprintf(sbuf, sizeof(sbuf), ALARM_DEL_CJSON, alarm->time.year, alarm->time.month, alarm->time.day, alarm->time.hour, alarm->time.min, alarm->time.sec);
                at_uart_cmd_send(AI_UART_CMD_ALARM_DEL, sbuf);
#endif
                memset(alarm_tmp, 0, sizeof(struct alarm_info));
                del = 1;
            } else if (alarm_tmp->cyc) {
                char del_weekday = sys_time_to_weekday(&alarm->time);
                if (alarm_tmp->cyc == 1) {
                    alarm_tmp->cyc = 0xFE;
                    alarm_tmp->cyc &= ~BIT(del_weekday);
                } else {
                    alarm_tmp->cyc &= ~BIT(del_weekday);
                }
                del = 1;
            }
        }
    }
    if (del) {
        syscfg_write(CFG_USER_ALARM, &alarm_tab, sizeof(alarm_tab));
    }
    os_mutex_post(&mutex);
    if (del) {
        alarm_msg(ALARM_SAVE, 0); // 保存更新
        alarm_msg(ALARM_ADD, 0);  // 重新设置闹钟
    }
    return del;
}
static void alarm_del_in_isr(char index)
{
    if (index < ALARM_NUM) {
//        os_mutex_pend(&mutex, 1000);
//        syscfg_read(CFG_USER_ALARM, &alarm_tab, sizeof(alarm_tab));
        memset(&alarm_tab[index], 0, sizeof(struct alarm_info));
//        syscfg_write(CFG_USER_ALARM, &alarm_tab, sizeof(alarm_tab));
//        os_mutex_post(&mutex);
    }
}
static void alarm_save(void)
{
    os_mutex_pend(&mutex, 1000);
    syscfg_write(CFG_USER_ALARM, &alarm_tab, sizeof(alarm_tab));
    os_mutex_post(&mutex);
}
void alarm_clean(void)
{
    os_mutex_pend(&mutex, 1000);
    set_alarm_ctrl(0);/* 关闭闹钟开关 */
    memset(&alarm_tab, 0, sizeof(alarm_tab));
    syscfg_write(CFG_USER_ALARM, &alarm_tab, sizeof(alarm_tab));
#ifdef AI_UART_CMD_CTROL_ENABLE
    ai_uart_cmd_data_push(AI_UART_CMD_ALARM_DEL_ALL, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
    at_uart_cmd_send(AI_UART_CMD_ALARM_DEL_ALL, NULL);
#endif
    os_mutex_post(&mutex);
    alarm_msg(ALARM_ADD, 0);//设置闹钟
    alarm_msg(ALARM_UPDATE2SERVER, 0);

#ifdef CONFIG_LVGL_UI_ENABLE
    lv_demo_all_alarms_del_flush_from_server();
#endif
}
/**
 * @brief 通过索引删除指定的闹钟
 * @param index 闹钟索引，范围0到ALARM_NUM-1
 * @return 成功返回0，失败返回非0
 */
int alarm_delete_by_index(int index)
{
    if (index < 0 || index >= ALARM_NUM) {
        printf("alarm_delete_by_index: invalid index %d\n", index);
        return -1;
    }

    os_mutex_pend(&mutex, 1000);
    syscfg_read(CFG_USER_ALARM, &alarm_tab, sizeof(alarm_tab));

    // 检查闹钟是否存在且启用
    if (alarm_tab[index].enable) {
        struct alarm_info alarm_tmp;
        memcpy(&alarm_tmp, &alarm_tab[index], sizeof(struct alarm_info));
        os_mutex_post(&mutex);

        // 使用现有的alarm_del函数删除闹钟
        alarm_del(&alarm_tmp, 0);
        return 0;
    }

    os_mutex_post(&mutex);
    return 0;
}

/**
 * @brief 通过时间删除指定的闹钟
 * @param time 闹钟时间信息
 * @return 成功返回0，失败返回非0
 */
int alarm_delete_by_time(struct sys_time *time)
{
    if (!time) {
        printf("alarm_delete_by_time: NULL time pointer\n");
        return -1;
    }

    struct alarm_info alarm_tmp = {0};
    memcpy(&alarm_tmp.time, time, sizeof(struct sys_time));

    // 使用现有的alarm_del函数删除闹钟
    alarm_del(&alarm_tmp, 0);
    return 0;
}

/**
 * @brief 通过小时和分钟删除闹钟，并可指定闹钟下标
 * @param index 闹钟下标（0 到 ALARM_NUM - 1），如果为-1则自动选择
 * @param hour 小时（0-23）
 * @param min 分钟（0-59）
 * @param cycle 重复循环（0:单次，1:每天, BIT(1)-BIT(7)：对应星期一到星期日）
 * @return 0:成功，-1:失败
 */
int alarm_del_by_hour_min(int8_t index, uint8_t hour, uint8_t min, uint8_t cycle)
{
    struct sys_time current_time = {0};
    struct alarm_info alarm = {0};
    int ret = 0;

    // 获取当前时间
    if (get_sys_time(&current_time) != 0) {
        printf("alarm_del_by_hour_min: failed to get system time\n");
        return -1;
    }

    // 构建闹钟时间（使用当前日期，设置指定的时和分）
    memcpy(&alarm.time, &current_time, sizeof(struct sys_time));
    alarm.time.hour = hour;
    alarm.time.min = min;
    alarm.time.sec = 0; // 秒设为0

    printf("alarm_del_by_hour_min: setting alarm for %04d-%02d-%02d %02d:%02d:%02d\n",
           alarm.time.year, alarm.time.month, alarm.time.day,
           alarm.time.hour, alarm.time.min, alarm.time.sec);

    // 设置闹钟属性
    alarm.enable = 1;
    alarm.cyc = cycle;
    alarm.music = 0; // 使用系统音乐
    alarm_del(&alarm, 1);

    printf("alarm del by hour_min: deld alarm at %02d:%02d, cycle: %d", hour, min, cycle);

    return 0;
}
/**
 * @brief 通过小时和分钟添加闹钟，并可指定闹钟下标
 * @param index 闹钟下标（0 到 ALARM_NUM - 1），如果为-1则自动选择
 * @param hour 小时（0-23）
 * @param min 分钟（0-59）
 * @param cycle 重复循环（0:单次，1:每天, BIT(1)-BIT(7)：对应星期一到星期日）
 * @return 0:成功，-1:失败
 */
int alarm_add_by_hour_min(int8_t index, uint8_t hour, uint8_t min, uint8_t cycle)
{
    struct sys_time current_time = {0};
    struct alarm_info alarm = {0};
    int ret = 0;

    // 获取当前时间
    if (get_sys_time(&current_time) != 0) {
        printf("alarm_add_by_hour_min: failed to get system time\n");
        return -1;
    }

    // 构建闹钟时间（使用当前日期，设置指定的时和分）
    memcpy(&alarm.time, &current_time, sizeof(struct sys_time));
    alarm.time.hour = hour;
    alarm.time.min = min;
    alarm.time.sec = 0; // 秒设为0

    // 检查是否今天的闹钟时间已过，如果已过则设置为明天
    if ((hour * 60 * 60 + min * 60) <= (current_time.hour * 60 * 60 + current_time.min * 60 + current_time.sec)) {
        if ((cycle == 0) || (cycle == 1)) {//星期一到星期日
            time_day_add(&alarm.time, 1);
        }
    }
    printf("alarm_add_by_hour_min: setting alarm for %04d-%02d-%02d %02d:%02d:%02d\n",
           alarm.time.year, alarm.time.month, alarm.time.day,
           alarm.time.hour, alarm.time.min, alarm.time.sec);

    // 设置闹钟属性
    alarm.enable = 1;
    alarm.cyc = cycle;
    alarm.music = 0; // 使用系统音乐
    alarm_add(&alarm, NULL);

    printf("alarm add by hour_min: added alarm at %02d:%02d, cycle: %d", hour, min, cycle);

    return 0;
}
static void alarm_init(void)
{
    if (syscfg_read(CFG_USER_ALARM, &alarm_tab, sizeof(alarm_tab)) < 0) {
        os_mutex_pend(&mutex, 1000);
        memset(&alarm_tab, 0, sizeof(alarm_tab));
        syscfg_write(CFG_USER_ALARM, &alarm_tab, sizeof(alarm_tab));
        os_mutex_post(&mutex);
    }
    if (system_reset_reason_get() == SYS_RST_ALM_WKUP) {
        if (alarm_tab[0].index) {
            struct alarm_info alarm_tmp;
            memcpy(&alarm_wakeup_time, &alarm_tab[alarm_tab[0].index - 1].time, sizeof(alarm_wakeup_time));
            memcpy(&alarm_tmp, &alarm_tab[alarm_tab[0].index - 1], sizeof(alarm_tmp));
            if (!alarm_tmp.cyc) {
                alarm_del(&alarm_tmp, 0);
            }
        }
    }
    alarm_wkup_rtc_time_init(NULL);
}
static void alarm_first_set(void)
{
    struct alarm_info *alarm_tmp;
    struct alarm_info *alarm_tmp1;
    struct sys_time time = {0};
    struct tm timeinfo = {0};
    time_t time1 = 0;
    time_t time_min = 0;
    char min_index = (char) -1;
    char update_en = 0;

    os_mutex_pend(&mutex, 1000);
    syscfg_read(CFG_USER_ALARM, &alarm_tab, sizeof(alarm_tab));
    for (int i = 0; i < ALARM_NUM; i++) { //查询清理重复闹钟
        alarm_tmp = &alarm_tab[i];
        for (int j = i + 1; j < ALARM_NUM; j++) {
            alarm_tmp1 = &alarm_tab[j];
            if (alarm_tmp1->time.year != 0 &&
                alarm_tmp1->time.year == alarm_tmp->time.year &&
                alarm_tmp1->time.month == alarm_tmp->time.month &&
                alarm_tmp1->time.day == alarm_tmp->time.day &&
                alarm_tmp1->time.hour == alarm_tmp->time.hour &&
                alarm_tmp1->time.min == alarm_tmp->time.min &&
                alarm_tmp1->time.sec == alarm_tmp->time.sec) {

                memset(alarm_tmp1, 0, sizeof(struct alarm_info));
                update_en = 1;
                printf("-> more alarm %d : %d-%d-%d %d:%d:%d\n", j,
                       alarm_tmp->time.year,
                       alarm_tmp->time.month,
                       alarm_tmp->time.day,
                       alarm_tmp->time.hour,
                       alarm_tmp->time.min,
                       alarm_tmp->time.sec);
            }
        }
    }
    if (update_en) {
        syscfg_write(CFG_USER_ALARM, &alarm_tab, sizeof(alarm_tab));
    }
    for (int i = 0; i < ALARM_NUM; i++) {
        alarm_tmp = &alarm_tab[i];
        if (alarm_tmp->enable) {
            memcpy(&time, &alarm_tmp->time, sizeof(time));
            time_ntp_utc(&time, 0);
            timeinfo.tm_year = time.year,
            timeinfo.tm_mon = time.month,
            timeinfo.tm_mday = time.day,
            timeinfo.tm_hour = time.hour,
            timeinfo.tm_min = time.min,
            timeinfo.tm_sec = time.sec;
            time1 = mktime(&timeinfo);

            printf("-> read alarm %d : %d-%d-%d %d:%d:%d\n", i,
                   alarm_tmp->time.year,
                   alarm_tmp->time.month,
                   alarm_tmp->time.day,
                   alarm_tmp->time.hour,
                   alarm_tmp->time.min,
                   alarm_tmp->time.sec);

            if (!time_min) {
                time_min = time1;
                min_index = i;
            }
            if (time1 < time_min) {
                time_min = time1;
                min_index = i;
            }
        }
    }
    memcpy(&time, &alarm_tab[min_index >= 0 ? min_index : 0].time, sizeof(time));
    if (min_index >= 0) {
        alarm_tab[0].index = min_index + 1;
        syscfg_write(CFG_USER_ALARM, &alarm_tab, sizeof(alarm_tab));
    }
    os_mutex_post(&mutex);
    if (min_index >= 0) {
        printf("-> setting alarm %d : %d-%d-%d %d:%d:%d\n", min_index, time.year, time.month, time.day, time.hour, time.min, time.sec);
        utc_time_update(&time, min_index, NULL);
#ifdef AI_UART_CMD_CTROL_ENABLE
#define ALARM_CJSON "{\"alarm_cyc\": %d,\"alarm_time\": \"%d-%02d-%02d %02d:%02d:%02d\"}"


        char sbuf[64];
        sprintf(sbuf, ALARM_CJSON, alarm_tab[min_index >= 0 ? min_index : 0].cyc, time.year, time.month, time.day, time.hour, time.min, time.sec);
        ai_uart_cmd_data_push(AI_UART_CMD_ALARM, sbuf, strlen(sbuf) + 1);
#endif
#ifdef AT_UART_CMD_ENABLE
#define ALARM_CJSON "{\"alarm_cyc\": %d,\"alarm_time\": \"%d-%02d-%02d %02d:%02d:%02d\"}"
        char sbuf[64];
        snprintf(sbuf, sizeof(sbuf), ALARM_CJSON, alarm_tab[min_index >= 0 ? min_index : 0].cyc, time.year, time.month, time.day, time.hour, time.min, time.sec);
        at_uart_cmd_send(AI_UART_CMD_ALARM, sbuf);
#endif
    } else {
        printf("-> setting no alarm \n");
        set_alarm_ctrl(0);
    }
}
static void alarm_chack_time(struct sys_time *new_time)//检测时间小于当前时间则删除闹钟
{
    struct alarm_info *alarm_tmp;
    struct sys_time time;
    struct tm timeinfo = {0};
    time_t time1 = 0;
    time_t time_new = 0;
    char save = 0;

    memcpy(&time, new_time, sizeof(struct sys_time));
    time_ntp_utc(&time, 0);
    timeinfo.tm_year = time.year,
    timeinfo.tm_mon = time.month,
    timeinfo.tm_mday = time.day,
    timeinfo.tm_hour = time.hour,
    timeinfo.tm_min = time.min,
    timeinfo.tm_sec = time.sec;
    time_new = mktime(&timeinfo);
//    printf("-> time_new : %d\n",time_new);

    os_mutex_pend(&mutex, 1000);
    syscfg_read(CFG_USER_ALARM, &alarm_tab, sizeof(alarm_tab));
    for (int i = 0; i < ALARM_NUM; i++) {
        alarm_tmp = &alarm_tab[i];
        if (alarm_tmp->enable) {
            memcpy(&time, &alarm_tmp->time, sizeof(time));
            time_ntp_utc(&time, 0);
            timeinfo.tm_year = time.year,
            timeinfo.tm_mon = time.month,
            timeinfo.tm_mday = time.day,
            timeinfo.tm_hour = time.hour,
            timeinfo.tm_min = time.min,
            timeinfo.tm_sec = time.sec;
            time1 = mktime(&timeinfo);
//            printf("-> time1 : %d\n",time1);

//            printf("-> read alarm %d : %d-%d-%d %d:%d:%d\n",i,
//            alarm_tmp->time.year,
//            alarm_tmp->time.month,
//            alarm_tmp->time.day,
//            alarm_tmp->time.hour,
//            alarm_tmp->time.min,
//            alarm_tmp->time.sec);

            if (time1 <= time_new) {
                if (alarm_tmp->cyc) {
                    alarm_tmp->time.year = new_time->year;
                    alarm_tmp->time.month = new_time->month;
                    alarm_tmp->time.day = new_time->day;
                    if (alarm_tmp->time.hour < new_time->hour ||
                        (alarm_tmp->time.hour == new_time->hour && alarm_tmp->time.min < new_time->min) ||
                        (alarm_tmp->time.hour == new_time->hour && alarm_tmp->time.min == new_time->min &&
                         alarm_tmp->time.sec <= new_time->sec)) {
                        if ((alarm_tmp->cyc & 0xFE) != 0xFE) {//星期一到星期日
                            char find = 0;
                            for (char i = now_weekday + 1; i <= 7; i++) {
                                if ((alarm_tmp->cyc & BIT(i))) {
                                    time_day_add(&alarm_tmp->time, i - now_weekday);
                                    find = 1;
                                    break;
                                }
                            }
                            if (!find) {
                                for (char i = 1; i < now_weekday; i++) {
                                    if ((alarm_tmp->cyc & BIT(i))) {
                                        time_day_add(&alarm_tmp->time, 7 - now_weekday + i);
                                        find = 1;
                                        break;
                                    }
                                }
                            }
                            if (!find) {
                                time_day_add(&alarm_tmp->time, 7);
                            }
                        } else {
                            time_day_add(&alarm_tmp->time, 1);//alarm_tab[index].time.day += 1;
                        }
                    }
                    printf("-> updata alarm\n");
                } else {
                    memset(alarm_tmp, 0, sizeof(struct alarm_info));//清空闹钟
                    printf("-> memset 0  alarm\n");
                }
                save = 1;
            }
        }
    }
    if (save) {
        syscfg_write(CFG_USER_ALARM, &alarm_tab, sizeof(alarm_tab));
    }
    os_mutex_post(&mutex);
}
/* 闹钟中断函数 */
static void alarm_rings(void *priv)
{
    char index = (char)priv;
    printf("->alarm clock is rings\n");
    if (alarm_tab[index].enable) {
        alarm_msg(ALARM_NOTIFY, index);//通知闹钟事件
    }
    if (alarm_tab[index].cyc) {
        if ((alarm_tab[index].cyc & 0xFE) != 0xFE) {//星期一到星期日
            char find = 0;
            for (char i = now_weekday + 1; i <= 7; i++) {
                if ((alarm_tab[index].cyc & BIT(i))) {
                    time_day_add(&alarm_tab[index].time, i - now_weekday);
                    find = 1;
                    break;
                }
            }
            if (!find) {
                for (char i = 1; i < now_weekday; i++) {
                    if ((alarm_tab[index].cyc & BIT(i))) {
                        time_day_add(&alarm_tab[index].time, 7 - now_weekday + i);
                        find = 1;
                        break;
                    }
                }
            }
            if (!find) {
                time_day_add(&alarm_tab[index].time, 7);
            }
        } else {
            time_day_add(&alarm_tab[index].time, 1);//alarm_tab[index].time.day += 1;
        }
    } else {
#ifdef CONFIG_LVGL_UI_ENABLE
        // 删除只响一次闹钟
        lv_demo_delete_once_alarm(alarm_tab[index].time.hour, alarm_tab[index].time.min, alarm_tab[index].time.sec);
#endif
        alarm_del_in_isr(index);
    }

    alarm_msg(ALARM_SAVE, 0);//保存更新闹钟
    alarm_msg(ALARM_ADD, 0);//重新设置闹钟
}
struct alarm_url_head {
    unsigned char size;
    unsigned char is_end;
};
#define VM_PAGE_SIZE        240
#define ALARM_HEAD_SIZE     sizeof(struct alarm_url_head)
static int alarm_save_url(char *url)
{
    if (!url) {
        return -1;
    }
    int buf_size = VM_PAGE_SIZE + ALARM_HEAD_SIZE;//flash的1个页-256,还以信息头，定一个页最多存储240字节
    char *buf = malloc(buf_size);
    if (!buf) {
        return -1;
    }
    int *url_size;
    int len = strlen(url) + 1;
    if (len >= (ALARM_MP3_URL_SIZE - ALARM_HEAD_SIZE * 4)) {
        printf("err url is tool long\n");
        if (buf) {
            free(buf);
        }
        return -1;
    }
    os_mutex_pend(&mutex, 1000);
    int index = CFG_USER_ALARM_URL0;
    int size;
    int offset = 0;
    struct alarm_url_head *head;
    while (len > 0) {
        size = MIN(len, VM_PAGE_SIZE);
        memset(buf, 0, buf_size);
        head = (struct alarm_url_head *)buf;
        head->size = size;
        head->is_end = (len > VM_PAGE_SIZE ? 0 : 1);
        memcpy(buf + ALARM_HEAD_SIZE, url + offset, size);
//        printf("-> write size %d , %d, %d\n",head->size,head->is_end,offset);
        syscfg_write(index++, buf, size + ALARM_HEAD_SIZE);
        offset += size;
        len -= size;
        if (index > CFG_USER_ALARM_URL3) {
            break;
        }
    }
    if (buf) {
        free(buf);
    }
    os_mutex_post(&mutex);
    return 0;
}
static int alarm_read_url(char *url, int url_buf_size)
{
    int ret = 0;
    if (!url || url_buf_size < 1024) {
        return -1;
    }
    int buf_size = VM_PAGE_SIZE + ALARM_HEAD_SIZE;//flash的1个页-256,还以信息头，定一个页最多存储240字节
    char *buf = malloc(buf_size);
    if (!buf) {
        return -1;
    }
    os_mutex_pend(&mutex, 1000);
    int index = CFG_USER_ALARM_URL0;
    int size;
    int offset = 0;
    struct alarm_url_head *head;
    while (1) {
        memset(buf, 0, buf_size);
        head = (unsigned char *)buf;
        if (syscfg_read(index++, buf, buf_size) < 0) {
            ret = -1;
            printf("read alarm url err\n");
            break;
        }
//        printf("-> read size %d , %d, %d\n",head->size,head->is_end,offset);
        if (head->is_end) { //结束包
            memcpy(url + offset, buf + ALARM_HEAD_SIZE, head->size);
            break;
        } else { //连续包
            memcpy(url + offset, buf + ALARM_HEAD_SIZE, head->size);
        }
        offset += head->size;
        if (index > CFG_USER_ALARM_URL3) {
            break;
        }
    }
    if (buf) {
        free(buf);
    }
    os_mutex_post(&mutex);
    printf("-> read alarm url ok\n");
    return ret;
}
int ntp2utc_add_alarm(int64_t sec, char cyc, char *music_url, void *alarm_content)
{
    if (sec <= 0) {
        return -1;
    }
    struct tm timeinfo;
    time_t timestamp;
    struct alarm_info alarm = {0};

    timestamp = sec;// + UTC_USE_DIFF_SEC;//服务器下发就是东八区时间戳
    printf("-> unix_timestamp = %d\n", timestamp);
    localtime_r(&timestamp, &timeinfo);

    alarm.enable = 1;
    alarm.cyc = cyc;
    alarm.music = music_url ? 1 : 0;
    if (music_url) {
        memset(mp3_url, 0, sizeof(mp3_url));
        memcpy(mp3_url, music_url, strlen(music_url) + 1);
        printf("alarm mp3_url : %s\n", mp3_url);
        alarm_save_url(mp3_url);
    }
    alarm.time.year = timeinfo.tm_year;
    alarm.time.month = timeinfo.tm_mon;
    alarm.time.day = timeinfo.tm_mday;
    alarm.time.hour = timeinfo.tm_hour;
    alarm.time.min = timeinfo.tm_min;
    alarm.time.sec = timeinfo.tm_sec;

    time_ntp_utc(&alarm.time, 1);

    alarm_add(&alarm, alarm_content);

#ifdef CONFIG_LVGL_UI_ENABLE
    lv_demo_switch_to_alarm_page();
    lv_demo_alarms_add_flush_from_server(alarm.time.hour, alarm.time.min, alarm.time.sec, alarm.cyc);
#endif
//    printf("-> a add alarm : %d-%d-%d %d:%d:%d\n",
//           alarm.time.year,
//           alarm.time.month,
//           alarm.time.day,
//           alarm.time.hour,
//           alarm.time.min,
//           alarm.time.sec);
    return 0;
}
int ntp2utc_time_update(int64_t sec)
{
    if (sec <= 0) {
        return -1;
    }
    struct tm timeinfo;
    time_t timestamp;
    struct sys_time test_rtc_time = {0};
    struct sys_time test_get_rtc_time = {0};

    timestamp = sec;// + UTC_USE_DIFF_SEC;//服务器下发就是东八区时间戳
    printf("-> utc_timestamp = %d\n", timestamp);
    localtime_r(&timestamp, &timeinfo);

    test_rtc_time.year = timeinfo.tm_year;
    test_rtc_time.month = timeinfo.tm_mon;
    test_rtc_time.day = timeinfo.tm_mday;
    test_rtc_time.hour = timeinfo.tm_hour;
    test_rtc_time.min = timeinfo.tm_min;
    test_rtc_time.sec = timeinfo.tm_sec;

    time_ntp_utc(&test_rtc_time, 1);

    /* 打开RTC设备 */
    os_mutex_pend(&mutex, 1000);

    void *rtc_hdl = dev_open("rtc", NULL);
    if (!rtc_hdl) {
        printf("err in rtc_hdl_open rtc\n");
        os_mutex_post(&mutex);
        return -1;
    }
    /* 获取时间信息 */
    dev_ioctl(rtc_hdl, IOCTL_GET_SYS_TIME, (u32)&test_get_rtc_time);

    if (test_get_rtc_time.year == test_rtc_time.year && test_get_rtc_time.month == test_rtc_time.month &&
        test_get_rtc_time.day == test_rtc_time.day && test_get_rtc_time.hour == test_rtc_time.hour && test_get_rtc_time.min == test_rtc_time.min &&
        ((test_get_rtc_time.sec - test_rtc_time.sec) < 5 && (test_get_rtc_time.sec - test_rtc_time.sec) > -5)) {
        printf("no need update time\n");
    } else {
        dev_ioctl(rtc_hdl, IOCTL_SET_SYS_TIME, (u32)&test_rtc_time);
        now_weekday = sys_time_to_weekday(&test_rtc_time);
        ntptime_update_rtc();
    }
    dev_close(rtc_hdl);
    os_mutex_post(&mutex);
    return 0;
}
int ntp2utc_delet_alarm(int64_t sec)
{
    if (sec <= 0) {
        return -1;
    }
    struct tm timeinfo;
    time_t timestamp;
    struct alarm_info alarm = {0};

    timestamp = sec;// + UTC_USE_DIFF_SEC;//服务器下发就是东八区时间戳
    printf("-> unix_timestamp = %d\n", timestamp);
    localtime_r(&timestamp, &timeinfo);

    alarm.time.year = timeinfo.tm_year;
    alarm.time.month = timeinfo.tm_mon;
    alarm.time.day = timeinfo.tm_mday;
    alarm.time.hour = timeinfo.tm_hour;
    alarm.time.min = timeinfo.tm_min;
    alarm.time.sec = timeinfo.tm_sec;

    time_ntp_utc(&alarm.time, 1);
    printf("-> delet alarm : %d-%02d-%02d %d:%d:%d\n",
           alarm.time.year, alarm.time.month, alarm.time.day,
           alarm.time.hour, alarm.time.min, alarm.time.sec);
    alarm_del(&alarm, 0);

#ifdef CONFIG_LVGL_UI_ENABLE
    lv_demo_switch_to_alarm_page();
    lv_demo_alarms_del_flush_from_server(alarm.time.hour, alarm.time.min, alarm.time.sec);
#endif
    return 0;
}

int ntp2utc_add_alarm_test(void)
{
    struct tm timeinfo;
    time_t timestamp;
    struct alarm_info alarm = {0};

    timestamp = time(NULL) - UTC_USE_DIFF_SEC;//格林时间
    printf("timestamp = %d\n", timestamp);
    localtime_r(&timestamp, &timeinfo);

    ntp2utc_add_alarm(timestamp + 10, 1, NULL, NULL);
    ntp2utc_add_alarm(timestamp + 30, 1, NULL, NULL);
}
/*
//buf存储结构体,长度为10个如下结构体长度
struct alarm_info { //12字节
    unsigned char enable: 1; //是否使能闹钟
    unsigned char music: 1; //闹钟自定义音乐，0系统音乐
    unsigned char index: 4; //设置的闹钟index
    unsigned char resver: 2; //预留
    unsigned char cyc; //是否循环的闹钟: 0不循环，BIT(0)每天循环，BIT(1)-BIT(7)对应星期一到星期日
    unsigned char resver2; //预留
    struct sys_time time;
};

使用:enable判断闹钟是否开启，cyc判断是否是每天闹钟使能或者工作日以及某一天
*/

void alarm_time_clear(void)
{
    os_mutex_pend(&mutex, 1000);
    if (syscfg_read(CFG_USER_ALARM, &alarm_tab, sizeof(alarm_tab)) >= 0) {
        memset(&alarm_tab, 0, sizeof(alarm_tab));
        syscfg_write(CFG_USER_ALARM, &alarm_tab, sizeof(alarm_tab));
    }
    os_mutex_post(&mutex);
    alarm_msg(ALARM_UPDATE2SERVER, 0);
#ifdef CONFIG_LVGL_UI_ENABLE
    lv_demo_all_alarms_del_flush_from_server();
#endif
}

void alarm_time_read(void *buf, int size)
{
    //printf("-> alarm_time_read size = %d , %d \n",size,sizeof(alarm_tab));
    if (buf && size >= sizeof(alarm_tab)) {
        os_mutex_pend(&mutex, 1000);
        syscfg_read(CFG_USER_ALARM, &alarm_tab, sizeof(alarm_tab));
        memcpy(buf, &alarm_tab, sizeof(alarm_tab));
        os_mutex_post(&mutex);
    }
}
int alarm_time_write(struct sys_time *time, char cyc)//time：时间，cyc：每天闹钟使能
{
    struct alarm_info alarm_tmp = {0};
    if (!time) {
        return 0;
    }
    memcpy(&alarm_tmp.time, time, sizeof(struct sys_time));
    alarm_tmp.enable = 1;
    alarm_tmp.cyc = cyc;
    alarm_tmp.music = 0;
    alarm_add(&alarm_tmp, NULL);
    return 0;
    //return ntp2utc_add_alarm(sec, cyc, NULL, NULL);
}
void *alarm_time_read_content_valid(int index)
{
    if (index >= 0 && index < ALARM_NUM) {
        os_mutex_pend(&mutex, 1000);
        memset(alarm_content_buf, 0, sizeof(alarm_content_buf));
        syscfg_read(CFG_USER_ALARM_CONTENT0 + index, alarm_content_buf, sizeof(alarm_content_buf));
        printf("-> alarm_time_read index = %d : %s \n", index, alarm_content_buf);
        os_mutex_post(&mutex);
        if (alarm_content_buf[0]) {
            return alarm_content_buf;
        }
    }
    return NULL;
}

//获取正在使用的闹钟index
int alarm_time_index(void)
{
    if (alarm_tab[0].index > 0 && alarm_tab[0].index <= ALARM_NUM) {
        return alarm_tab[0].index - 1;
    }
    return 0;
}

int alarm_time_del(struct sys_time *time)
{
    struct alarm_info alarm_tmp = {0};
    if (!time) {
        return 0;
    }
    memcpy(&alarm_tmp.time, time, sizeof(struct sys_time));
    return alarm_del(&alarm_tmp, 0);
    //return ntp2utc_delet_alarm(sec);
}
int get_sys_time(struct sys_time *t)
{
    //return utc_time_update(NULL, 0, t);
    void *rtc_hdl = dev_open("rtc", NULL);
    if (!rtc_hdl) {
        printf("err in rtc_hdl_open rtc\n");
        return -1;
    }
    dev_ioctl(rtc_hdl, IOCTL_GET_SYS_TIME, (u32)t);
    dev_close(rtc_hdl);
    return 0;
}

#define ALARM_TASK_NAME     "alarm_task"
static int alarm_msg(int message, char notice)
{
    int ret;
    char retry = 0;
    do {
        ret = os_taskq_post(ALARM_TASK_NAME, 2, message, notice);
        if (ret != OS_NO_ERR) {
            if (ret != OS_Q_FULL) {
                return -1;
            }
            os_time_dly(5);
            retry++;
        } else {
            break;
        }
    } while (retry < 5);
    if (retry == 5) {
        printf("warnning : music_buf OS_Q_FULL\n");
    }
    return ret;
}
void ntptime_update(void)
{
    alarm_msg(ALARM_UPDATE_TIME, 0);//重新校准时间
}
void ntptime_update_rtc(void)
{
    alarm_msg(ALARM_UPDATE_TIME_RTC, 0);//重新校准时间
}
char *alarm_get_url(char index)
{
    alarm_read_url(mp3_url, sizeof(mp3_url));
    if (mp3_url[0] != 0) {
        return mp3_url;
    }
    return NULL;
}
int unix_timestamp_callback(int64_t timesec, char *mp3_url, char cyc, void *alarm_content)
{
    ntp2utc_add_alarm(timesec, cyc, mp3_url, alarm_content);
    return 0;
}
int utc_timer_update_get(struct sys_time *time)
{
    if (time) {
        return utc_time_update(NULL, 0, time);
    }
    return -1;
}
int alarm_music_play_stop(void)
{
    if (music_play_alarm_ring()) {
#if 1 //闹钟唤醒响铃下、按下按键停止闹钟并关机
        if (system_reset_reason_get() == SYS_RST_ALM_WKUP) {//超时而且是闹钟唤醒
            sys_power_poweroff();
        }
#endif
        if (music_play_alarm_url_ring()) {
            net_music_dec_stop();
            net_music_play_set_stop();
            net_music_num_clear();
        }
        music_play_stop(NULL);
        music_buf_play_stop_all();
        music_buf_play_set_stop();
        music_buf_play_free_lbuf();
        music_play_res_file_unloop();
        mp3_buf_play_res_file_unloop();
        return 0;
    }
    return -1;
}
void alarm_music_play_end(void)
{
#ifdef CONFIG_LVGL_UI_ENABLE
    lv_demo_switch_to_main_page();
    music_play_res_file_unloop();
    mp3_buf_play_res_file_unloop();
#else
#ifdef CONFIG_UI_PLAY_EMOJI
    play_face_emoji(AI_UART_CMD_EMOJI_DATA);
#endif
#endif
}
static int alarm_time_update_to_server(void)//检测时间小于当前时间则删除闹钟
{
    struct alarm_info *alarm_tmp;
    int alarm_cnt = 0;
#define ALARM_CONTENT_OFFSET    2048
    char *alarm_content = NULL;
    char *alarm_table = malloc(ALARM_CONTENT_OFFSET + sizeof(alarm_content_buf));
    if (!alarm_table) {
        return -1;
    }
    alarm_content = alarm_table + ALARM_CONTENT_OFFSET;
    os_mutex_pend(&mutex, 1000);
    syscfg_read(CFG_USER_ALARM, &alarm_tab, sizeof(alarm_tab));
    snprintf(alarm_table, ALARM_CONTENT_OFFSET, "\"alarm\":[");
    for (int i = 0; i < ALARM_NUM; i++) {
        alarm_tmp = &alarm_tab[i];
        if (alarm_tmp->enable) {
            alarm_cnt++;
            memset(alarm_content, 0, sizeof(alarm_content_buf));
            syscfg_read(CFG_USER_ALARM_CONTENT0 + i, alarm_content, sizeof(alarm_content_buf));
            if (alarm_content[0]) {
                snprintf(alarm_table, ALARM_CONTENT_OFFSET, "%s{\"reminder_time\":\"%04d-%02d-%02d %02d:%02d:%02d\",\"cyc\":%d,\"reminder_text\":\"%s\"},", alarm_table,
                         alarm_tmp->time.year, alarm_tmp->time.month, alarm_tmp->time.day, alarm_tmp->time.hour, alarm_tmp->time.min, alarm_tmp->time.sec, alarm_tmp->cyc,
                         alarm_content);
            } else {
                snprintf(alarm_table, ALARM_CONTENT_OFFSET, "%s{\"reminder_time\":\"%04d-%02d-%02d %02d:%02d:%02d\",\"cyc\":%d,\"reminder_text\":\"\"},", alarm_table,
                         alarm_tmp->time.year, alarm_tmp->time.month, alarm_tmp->time.day, alarm_tmp->time.hour, alarm_tmp->time.min, alarm_tmp->time.sec, alarm_tmp->cyc);
            }
        }
    }
    os_mutex_post(&mutex);
    if (alarm_cnt) {
        int slen = strlen(alarm_table);
        if (alarm_table[slen - 1] == ',') {
            alarm_table[slen - 1] = 0;
        }
        strcat(alarm_table, "]");
        //printf("alarm_table: \n%s\n\n",alarm_table);
        extern int http_ai_dev_alarm_update_to_server(char *alarm_table, void (*set_callbak)(int err_code, char *buf));
        http_ai_dev_alarm_update_to_server(alarm_table, NULL);
    } else {
        extern int http_ai_dev_alarm_update_to_server(char *alarm_table, void (*set_callbak)(int err_code, char *buf));
        http_ai_dev_alarm_update_to_server("\"alarm\":[]", NULL);
    }
    free(alarm_table);
    return 0;
}
static void time_rtc_alarm_task(void *p)
{
    int err, res;
    int msg[4];
    char index;
    struct sys_time new_time = {0};//最新时间;
    UTC_USE = qyai_chat_addr_version() == 0 ? UTC_EAST8 : UTC0;
    os_mutex_create(&mutex);
    alarm_init();
    //alarm_clean();
    os_time_dly(200);//使用内部rtc在开机1秒内进行校准，需要再校准后进行设置时间才有效
    alarm_wkup_rtc_time_init(&new_time);
    if (alarm_wakeup_time.year && new_time.year) { //闹钟唤醒后检测时间，更新循环闹钟
        alarm_chack_time(&new_time);//更新循环闹钟
    }
    alarm_first_set();//开机重设闹钟
    sys_timer_add_to_task("sys_timer", NULL, ntptime_update, 30 * 60 * 1000); //30分钟校准时间一次
    while (1) {
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        if (res == OS_TASK_DEL_IDLE) {
            break;
        } else if (res == OS_TASKQ) {
            if (msg[0] == Q_USER) {
                switch (msg[1]) {
                case ALARM_UPDATE_TIME:
                    if (!utc_time_update(NULL, 0, &new_time)) {
                        alarm_chack_time(&new_time);//校对时间，整理闹钟
                    }
#ifdef TIMER_UART_CALIBRATION_ENABLE
                    time_uart_calibration_update();

#endif
#ifdef  USED_TM1629_SHOWN
                    void tm_1629_shown_time_update(void);
                    tm_1629_shown_time_update();
#endif
                    alarm_first_set();//校准1次时间添加同步1个闹钟
                    alarm_time_update_to_server();
                    break;
                case ALARM_UPDATE_TIME_RTC:
                    ;
#ifdef  USED_TM1629_SHOWN
                    void tm_1629_shown_time_update_rtc(void);
                    tm_1629_shown_time_update_rtc();
#endif
                    break;
                case ALARM_ADD:
                    if (!utc_time_update(NULL, 0, &new_time)) {
                        alarm_chack_time(&new_time);//校对时间，整理闹钟
                    }
                    alarm_first_set();//添加第一个闹钟，添加1次校准1次时间
                    alarm_time_update_to_server();
                    break;
                case ALARM_SAVE:
                    alarm_save();//更新闹钟到flash
                    break;
                case ALARM_UPDATE2SERVER:
                    alarm_time_update_to_server();
                    break;
                case ALARM_NOTIFY:
                    index = msg[2];
//                    struct key_event key = {0};
//                    aisp_app_suspend();
//                    key.type = KEY_EVENT_USER;
//                    key.action = KEY_EVENT_HOLD;
//                    key.value = index;
//                    key_event_notify(KEY_EVENT_FROM_USER, &key);
                    struct device_event event = {0};
                    event.event = DEVICE_EVENT_IN;
                    event.value = index;
                    device_event_notify(DEVICE_EVENT_FROM_ALM, &event);
#ifdef AI_UART_CMD_CTROL_ENABLE
#define ALARM_RING_CJSON    "{\"alarm_ring\": \"%d-%02d-%02d %02d:%02d:%02d\"}"
                    char sbuf[64];
                    index  = index > (ALARM_NUM - 1) ? (ALARM_NUM - 1) : index;
                    snprintf(sbuf, sizeof(sbuf), ALARM_RING_CJSON, alarm_tab[index].time.year,
                             alarm_tab[index].time.month,
                             alarm_tab[index].time.day,
                             alarm_tab[index].time.hour,
                             alarm_tab[index].time.min,
                             alarm_tab[index].time.sec);
                    ai_uart_cmd_data_push(AI_UART_CMD_ALARM_RING, sbuf, strlen(sbuf) + 1);
#endif
#ifdef AT_UART_CMD_ENABLE
#define ALARM_RING_CJSON    "{\"alarm_ring\": \"%d-%02d-%02d %02d:%02d:%02d\"}"
                    char sbuf[64];
                    index  = index > (ALARM_NUM - 1) ? (ALARM_NUM - 1) : index;
                    snprintf(sbuf, sizeof(sbuf), ALARM_RING_CJSON, alarm_tab[index].time.year,
                             alarm_tab[index].time.month,
                             alarm_tab[index].time.day,
                             alarm_tab[index].time.hour,
                             alarm_tab[index].time.min,
                             alarm_tab[index].time.sec);
                    at_uart_cmd_send(AI_UART_CMD_ALARM_RING, sbuf);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
                    play_face_emoji(AI_UART_CMD_ALARM_RING);
#endif
                    break;
                }
            }
        }
    }
}

static int time_rtc_main(void)
{
#ifdef CONFIG_NO_SDRAM_ENABLE
    os_task_create(time_rtc_alarm_task, NULL, 10, 700, 128, ALARM_TASK_NAME);
#else
    os_task_create(time_rtc_alarm_task, NULL, 10, 1800, 128, ALARM_TASK_NAME);
#endif // CONFIG_NO_SDRAM_ENABLE
    return 0;
}
late_initcall(time_rtc_main);
