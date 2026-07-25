#ifndef __DE_INSTRUCT_WORD_H__
#define __DE_INSTRUCT_WORD_H__

#ifdef CONFIG_INSTRUCTION_DE
//=================德语指令词=====================//

//=================系统指令=====================//
#define SYS_CLOSE_DE \
    "ausmachen","abschalten","stoppen","verlassen","Gespräch beenden","Gespräch stoppen", \
    "Dialog schließen","Ich will nicht mehr mit dir sprechen"

#define SYS_STR_POWER_OFF_DE \
    "Gerät ausschalten"

#define SYS_30MIN_DE \
    "anderthalb Stunden später","nach anderthalb Stunden", \
    "dreißig Minuten später","nach dreißig Minuten", \
    "eine halbe Stunde später","nach einer halben Stunde"

#define SYS_90MIN_DE \
    "anderthalb Stunden später","nach anderthalb Stunden"

#define SYS_HOUR_TIMEOUT_DE \
    "Stunde später","nach einer Stunde"

#define SYS_MIN_TIMEOUT_DE \
    "Minute später","nach einigen Minuten"

#define SYS_COUNTOWN_TIMEOUT_DE \
    "Countdown","Timer einstellen"

#define SYS_COUNTOWN_TIMEOUT_CLOSE_DE \
    "Countdown abbrechen","Timer löschen","Timer abschalten"

//=================音乐指令=====================//
#define SYS_MUSIC_PLAY_DE \
    "Musik abspielen","Lied abspielen","Musik spielen"

#define SYS_MUSIC_CONTINUE_DE \
    "weiter abspielen","Musik fortsetzen","Lied weiterlaufen lassen"

#define SYS_MUSIC_STOP_DE \
    "pausieren","Musik pausieren","Lied pausieren"

#define SYS_MUSIC_CLOSE_DE \
    "Musik stoppen","Musik ausschalten","Lied beenden"

#define SYS_MUSIC_NEXT_DE \
    "nächstes Lied","nächster Song","Lied wechseln","überspringen"

#define SYS_MUSIC_LAST_DE \
    "vorheriges Lied","vorheriger Song"

#define SYS_MUSIC_LOOP_DE \
    "einzelnes Lied wiederholen","Einzelwiederholung"

#define SYS_MUSIC_LOOP_CLOSE_DE \
    "Wiederholung ausschalten","Wiederholung beenden","Wiederholung abbrechen", \
    "Einzelwiederholung abschalten"

//=================设备控制指令=====================//
#define SYS_MUSIC_APP_AI_DE \
    "AI Modus","Intelligenter AI Modus","Assistentenmodus"

#define SYS_MUSIC_APP_UDISK_DE \
    "USB abspielen","USB Modus"

#define SYS_MUSIC_APP_SD_DE \
    "SD Karte abspielen","TF Karte abspielen","SD Modus","TF Modus"

#define SYS_MUSIC_APP_BT_DE \
    "Bluetooth Modus","über Bluetooth abspielen"

#define SYS_MUSIC_APP_AUX_DE \
    "Line-In Modus","AUX Modus"

//=================闹钟指令=====================//
#define SYS_ALARM_DEL_DE \
    "Wecker löschen","Wecker abschalten","Erinnerung entfernen","Wecker abbrechen"

#define SYS_ALARM_DEL_ALL_DE \
    "alle Wecker löschen","alle Wecker abschalten","alle Erinnerungen entfernen"

#define SYS_ALARM_LOOK_DE \
    "Wecker anzeigen","Welche Wecker sind gesetzt?","Weckermenü öffnen"

//=================音量调节指令=====================//
#define SYS_VOLUME_SET_DE  "Lautstärke einstellen auf"

#define SYS_MUSIC_VOLUME_DE \
    "leiser machen","Musik leiser stellen", \
    "lauter machen","Musik lauter stellen"

#define SYS_MUSIC_MAX_VOLUME_DE \
    "maximale Lautstärke","voll aufdrehen","maximal laut"

#define SYS_MUSIC_MIN_VOLUME_DE \
    "minimale Lautstärke","ganz leise","niedrigste Lautstärke"

#endif
#endif // __DE_INSTRUCT_WORD_H__