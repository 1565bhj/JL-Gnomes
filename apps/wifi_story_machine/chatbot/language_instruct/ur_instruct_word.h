#ifndef __UR_INSTRUCT_WORD_H__
#define __UR_INSTRUCT_WORD_H__

#ifdef CONFIG_INSTRUCTION_UR
//=================乌尔都语指令词=====================//

//=================系统指令=====================//
#define SYS_CLOSE_UR \
    "بند کرو","بند کر دو","رک جاؤ","چلے جاؤ","بات چیت بند کرو","بات چیت روکو", \
    "بات چیت ختم کرو","میں تم سے بات نہیں کرنا چاہتا"

#define SYS_STR_POWER_OFF_UR \
    "مشین بند کرو"

#define SYS_30MIN_UR \
    "ڈیڑھ گھنٹے بعد","ڈیڑھ گھنٹے کے بعد", \
    "تیس منٹ بعد","تیس منٹ کے بعد", \
    "آدھے گھنٹے بعد","آدھے گھنٹے کے بعد"

#define SYS_90MIN_UR \
    "ڈیڑھ گھنٹے بعد","ڈیڑھ گھنٹے کے بعد"

#define SYS_HOUR_TIMEOUT_UR \
    "گھنٹے بعد","ایک گھنٹے بعد"

#define SYS_MIN_TIMEOUT_UR \
    "منٹ بعد","کچھ منٹ بعد"

#define SYS_COUNTOWN_TIMEOUT_UR \
    "الٹی گنتی","ٹائمر لگاؤ"

#define SYS_COUNTOWN_TIMEOUT_CLOSE_UR \
    "الٹی گنتی منسوخ کرو","ٹائمر ہٹاؤ","ٹائمر منسوخ کرو"

//=================音乐指令=====================//
#define SYS_MUSIC_PLAY_UR \
    "گانا چلاؤ","موسیقی چلاؤ","گانا بجاؤ"

#define SYS_MUSIC_CONTINUE_UR \
    "چلتے رہو","گانا جاری رکھو","موسیقی جاری رکھو"

#define SYS_MUSIC_STOP_UR \
    "تھوڑا رکو","گانا عارضی طور پر روکو"

#define SYS_MUSIC_CLOSE_UR \
    "گانا بند کرو","موسیقی بند کرو","گانا روک دو"

#define SYS_MUSIC_NEXT_UR \
    "اگلا گانا","گانا بدلو","اگلا گانا چلاؤ","گانا چھوڑو"

#define SYS_MUSIC_LAST_UR \
    "پچھلا گانا","پچھلا گانا چلاؤ"

#define SYS_MUSIC_LOOP_UR \
    "ایک ہی گانا بار بار چلاؤ"

#define SYS_MUSIC_LOOP_CLOSE_UR \
    "دہراؤ موڈ بند کرو","لوپ موڈ سے باہر نکل جاؤ","دہراؤ منسوخ کرو", \
    "ایک گانے کا دہراؤ بند کرو"

//=================设备控制指令=====================//
#define SYS_MUSIC_APP_AI_UR \
    "AI موڈ","سمارٹ AI موڈ","سمارٹ اسسٹنٹ موڈ"

#define SYS_MUSIC_APP_UDISK_UR \
    "USB چلاؤ","USB موڈ"

#define SYS_MUSIC_APP_SD_UR \
    "SD کارڈ چلاؤ","TF کارڈ چلاؤ","SD کارڈ موڈ","TF کارڈ موڈ"

#define SYS_MUSIC_APP_BT_UR \
    "بلوٹوتھ موڈ","بلوٹوتھ سے چلاؤ"

#define SYS_MUSIC_APP_AUX_UR \
    "لائن موڈ","آکس ان پٹ موڈ"

//=================闹钟指令=====================//
#define SYS_ALARM_DEL_UR \
    "الارم ہٹاؤ","الارم منسوخ کرو","الارم بند کرو","ریمائنڈر منسوخ کرو"

#define SYS_ALARM_DEL_ALL_UR \
    "سب الارم ہٹاؤ","سب الارم منسوخ کرو","سب الارم بند کرو"

#define SYS_ALARM_LOOK_UR \
    "الارم دیکھو","کون سے الارم لگے ہیں","الارم صفحہ کھولو"

//=================音量调节指令=====================//
#define SYS_VOLUME_SET_UR  "آواز سیٹ کرو"

#define SYS_MUSIC_VOLUME_UR \
    "آواز کم کرو","گانے کی آواز کم کرو", \
    "آواز بڑھاؤ","گانے کی آواز بڑھاؤ"

#define SYS_MUSIC_MAX_VOLUME_UR \
    "زیادہ سے زیادہ آواز","بلند ترین آواز کی سطح"

#define SYS_MUSIC_MIN_VOLUME_UR \
    "کم سے کم آواز","کم ترین آواز کی سطح"

#endif
#endif // __UR_INSTRUCT_WORD_H__