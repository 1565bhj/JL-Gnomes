#ifndef __HE_INSTRUCT_WORD_H__
#define __HE_INSTRUCT_WORD_H__

#ifdef CONFIG_INSTRUCTION_HE
//=================希伯来语指令词=====================//

//=================系统指令=====================//
#define SYS_CLOSE_HE \
    "סגור","כבה","עצור","צא","סגור שיחה","עצור דיאלוג", \
    "סיים את השיחה","אני לא רוצה לדבר איתך יותר"

#define SYS_STR_POWER_OFF_HE \
    "כבה את המכשיר"

#define SYS_30MIN_HE \
    "אחרי שעה וחצי","בעוד שעה וחצי", \
    "אחרי שלושים דקות","בעוד שלושים דקות", \
    "אחרי חצי שעה","בעוד חצי שעה"

#define SYS_90MIN_HE \
    "אחרי שעה וחצי","בעוד שעה וחצי"

#define SYS_HOUR_TIMEOUT_HE \
    "אחרי שעה","בעוד שעה"

#define SYS_MIN_TIMEOUT_HE \
    "אחרי דקה","בעוד כמה דקות"

#define SYS_COUNTOWN_TIMEOUT_HE \
    "ספירה לאחור","הגדר טיימר"

#define SYS_COUNTOWN_TIMEOUT_CLOSE_HE \
    "בטל ספירה לאחור","מחק טיימר","כבה טיימר"

//=================音乐指令=====================//
#define SYS_MUSIC_PLAY_HE \
    "הפעל מוסיקה","הפעל שיר","נגן מוסיקה"

#define SYS_MUSIC_CONTINUE_HE \
    "המשך השמעה","חדש את המוסיקה","המשך את השיר"

#define SYS_MUSIC_STOP_HE \
    "השהה","השהה מוסיקה","השהה שיר"

#define SYS_MUSIC_CLOSE_HE \
    "עצור מוסיקה","כבה מוסיקה","סיים את השיר"

#define SYS_MUSIC_NEXT_HE \
    "השיר הבא","הרצועה הבאה","החלף שיר","דלג על שיר"

#define SYS_MUSIC_LAST_HE \
    "השיר הקודם","הרצועה הקודמת"

#define SYS_MUSIC_LOOP_HE \
    "חזור על שיר יחיד","מצב חזרה יחיד"

#define SYS_MUSIC_LOOP_CLOSE_HE \
    "כבה חזרה","צא ממצב חזרה","בטל חזרה", \
    "כבה חזרה של שיר יחיד"

//=================设备控制指令=====================//
#define SYS_MUSIC_APP_AI_HE \
    "מצב בינה מלאכותית","מצב עוזר חכם"

#define SYS_MUSIC_APP_UDISK_HE \
    "הפעל USB","מצב USB"

#define SYS_MUSIC_APP_SD_HE \
    "הפעל כרטיס SD","הפעל כרטיס TF","מצב SD","מצב TF"

#define SYS_MUSIC_APP_BT_HE \
    "מצב בלוטות'","הפעל דרך בלוטות'"

#define SYS_MUSIC_APP_AUX_HE \
    "מצב כניסת קו","מצב AUX"

//=================闹钟指令=====================//
#define SYS_ALARM_DEL_HE \
    "מחק שעון מעורר","בטל שעון מעורר","כבה שעון מעורר","מחק תזכורת"

#define SYS_ALARM_DEL_ALL_HE \
    "מחק את כל השעונים המעוררים","בטל את כל השעונים המעוררים","כבה את כל התזכורות"

#define SYS_ALARM_LOOK_HE \
    "הצג שעונים מעוררים","אילו שעונים מעוררים מוגדרים?","פתח עמוד שעונים מעוררים"

//=================音量调节指令=====================//
#define SYS_VOLUME_SET_HE  "הגדר עוצמת קול ל-"

#define SYS_MUSIC_VOLUME_HE \
    "הנמך עוצמת קול","הנמך את המוסיקה", \
    "הגבר עוצמת קול","הגבר את המוסיקה"

#define SYS_MUSIC_MAX_VOLUME_HE \
    "עוצמת קול מקסימלית","קול על מקסימום"

#define SYS_MUSIC_MIN_VOLUME_HE \
    "עוצמת קול מינימלית","קול על מינימום"

#endif
#endif // __HE_INSTRUCT_WORD_H__