#ifndef __NL_INSTRUCT_WORD_H__
#define __NL_INSTRUCT_WORD_H__

#ifdef CONFIG_INSTRUCTION_NL
//=================荷兰语指令词=====================//

//=================系统指令=====================//
#define SYS_CLOSE_NL \
    "uitzetten","sluiten","stoppen","verlaten","gesprek afbreken","gesprek stoppen", \
    "dialoog sluiten","ik wil niet meer met je praten"

#define SYS_STR_POWER_OFF_NL \
    "apparaat uitschakelen"

#define SYS_30MIN_NL \
    "anderhalf uur later","over anderhalf uur", \
    "dertig minuten later","over dertig minuten", \
    "een half uur later","over een half uur"

#define SYS_90MIN_NL \
    "anderhalf uur later","over anderhalf uur"

#define SYS_HOUR_TIMEOUT_NL \
    "een uur later","over een uur"

#define SYS_MIN_TIMEOUT_NL \
    "minuut later","over enkele minuten"

#define SYS_COUNTOWN_TIMEOUT_NL \
    "aftellen","timer instellen"

#define SYS_COUNTOWN_TIMEOUT_CLOSE_NL \
    "aftelling annuleren","timer wissen","timer uitschakelen"

//=================音乐指令=====================//
#define SYS_MUSIC_PLAY_NL \
    "muziek afspelen","nummer afspelen","muziek spelen"

#define SYS_MUSIC_CONTINUE_NL \
    "verder afspelen","muziek vervolgen","nummer verder laten lopen"

#define SYS_MUSIC_STOP_NL \
    "pauzeren","muziek pauzeren","nummer pauzeren"

#define SYS_MUSIC_CLOSE_NL \
    "muziek stoppen","muziek uitschakelen","nummer afbreken"

#define SYS_MUSIC_NEXT_NL \
    "volgend nummer","volgend liedje","nummer wisselen","overslaan"

#define SYS_MUSIC_LAST_NL \
    "vorig nummer","vorig liedje"

#define SYS_MUSIC_LOOP_NL \
    "enkel nummer herhalen","enkel herhalen"

#define SYS_MUSIC_LOOP_CLOSE_NL \
    "herhaling uitschakelen","herhaalmodus verlaten","herhaling annuleren", \
    "enkel herhalen uitschakelen"

//=================设备控制指令=====================//
#define SYS_MUSIC_APP_AI_NL \
    "AI modus","slimme AI modus","assistentmodus"

#define SYS_MUSIC_APP_UDISK_NL \
    "USB afspelen","USB modus"

#define SYS_MUSIC_APP_SD_NL \
    "SD kaart afspelen","TF kaart afspelen","SD modus","TF modus"

#define SYS_MUSIC_APP_BT_NL \
    "Bluetooth modus","afspelen via Bluetooth"

#define SYS_MUSIC_APP_AUX_NL \
    "Line-In modus","AUX modus"

//=================闹钟指令=====================//
#define SYS_ALARM_DEL_NL \
    "wekker wissen","wekker uitschakelen","herinnering verwijderen","wekker annuleren"

#define SYS_ALARM_DEL_ALL_NL \
    "alle wekkers wissen","alle wekkers uitschakelen","alle herinneringen verwijderen"

#define SYS_ALARM_LOOK_NL \
    "wekkers weergeven","welke wekkers staan ingesteld?","wekker menu openen"

//=================音量调节指令=====================//
#define SYS_VOLUME_SET_NL  "volume instellen op"

#define SYS_MUSIC_VOLUME_NL \
    "zachter zetten","muziek zachter zetten", \
    "luider zetten","muziek luider zetten"

#define SYS_MUSIC_MAX_VOLUME_NL \
    "maximaal volume","vol open draaien","maximaal luid"

#define SYS_MUSIC_MIN_VOLUME_NL \
    "minimaal volume","helemaal zacht","laagste volume"

#endif
#endif // __NL_INSTRUCT_WORD_H__