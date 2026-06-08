#ifndef __OAL_RTC_H__
#define __OAL_RTC_H__

#include "oal_type.h"
typedef struct {
    OAL_U8  rtc_sec;  //Seconds after minutes - [0,59].
    OAL_U8  rtc_min;  //Minutes after the hour - [0,59].
    OAL_U8  rtc_hour; //Hours after midnight - [0,23].
    OAL_U8  rtc_day;  //Day of the month - [1,31].
    OAL_U8  rtc_mon;  //Months - [1,12].
    OAL_U8  rtc_week; //Days in a week - [0,6].
    OAL_U16 rtc_year;
} OAL_RTC_TIME;

OAL_RESULT OAL_RtcSetTimeBySec(long time_var);
OAL_RESULT OAL_RtcSetTime(OAL_RTC_TIME *time);
OAL_RESULT OAL_RtcGetTime(OAL_RTC_TIME *time);
OAL_RESULT OAL_RtcGetTimeEX(OAL_RTC_TIME *pp_time);
OAL_RESULT OAL_RtcGetTimeBySec(long *time_var);
OAL_RESULT OAL_RtcTimeSync();
unsigned int OAL_GetTickCount();


#endif
