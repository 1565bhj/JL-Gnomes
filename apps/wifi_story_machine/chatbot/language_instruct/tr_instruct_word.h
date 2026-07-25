#ifndef __TR_INSTRUCT_WORD_H__
#define __TR_INSTRUCT_WORD_H__

#ifdef CONFIG_INSTRUCTION_TR
//=================土耳其语指令词=====================//

//=================系统指令=====================//
#define SYS_CLOSE_TR \
    "kapat","kapatın","durdur","çık","sohbeti kapat","diyaloğu durdur", \
    "konuşmayı bitir","artık seninle konuşmak istemiyorum"

#define SYS_STR_POWER_OFF_TR \
    "cihazı kapat"

#define SYS_30MIN_TR \
    "bir buçuk saat sonra","bir buçuk saat içinde", \
    "otuz dakika sonra","otuz dakika içinde", \
    "yarım saat sonra","yarım saat içinde"

#define SYS_90MIN_TR \
    "bir buçuk saat sonra","bir buçuk saat içinde"

#define SYS_HOUR_TIMEOUT_TR \
    "bir saat sonra","bir saat içinde"

#define SYS_MIN_TIMEOUT_TR \
    "bir dakika sonra","birkaç dakika içinde"

#define SYS_COUNTOWN_TIMEOUT_TR \
    "geri sayım","zamanlayıcı ayarla"

#define SYS_COUNTOWN_TIMEOUT_CLOSE_TR \
    "geri sayımı iptal et","zamanlayıcıyı sil","zamanlayıcıyı kapat"

//=================音乐指令=====================//
#define SYS_MUSIC_PLAY_TR \
    "müzik çal","şarkı oynat","müzik oynat"

#define SYS_MUSIC_CONTINUE_TR \
    "oynatmaya devam et","müziği devam ettir","şarkıyı sürdür"

#define SYS_MUSIC_STOP_TR \
    "duraklat","müziği duraklat","şarkıyı duraklat"

#define SYS_MUSIC_CLOSE_TR \
    "müziği durdur","müziği kapat","şarkıyı bitir"

#define SYS_MUSIC_NEXT_TR \
    "sonraki şarkı","sonraki parça","şarkıyı değiştir","şarkıyı atla"

#define SYS_MUSIC_LAST_TR \
    "önceki şarkı","önceki parça"

#define SYS_MUSIC_LOOP_TR \
    "tek şarkıyı tekrarla","tek parça tekrar modu"

#define SYS_MUSIC_LOOP_CLOSE_TR \
    "tekrar modunu kapat","tekrar modundan çık","tekrarı iptal et", \
    "tek şarkı tekrarını kapat"

//=================设备控制指令=====================//
#define SYS_MUSIC_APP_AI_TR \
    "AI modu","akıllı AI modu","akıllı asistan modu"

#define SYS_MUSIC_APP_UDISK_TR \
    "USB oynat","USB modu"

#define SYS_MUSIC_APP_SD_TR \
    "SD kart oynat","TF kart oynat","SD modu","TF modu"

#define SYS_MUSIC_APP_BT_TR \
    "Bluetooth modu","Bluetooth ile oynat"

#define SYS_MUSIC_APP_AUX_TR \
    "Hat giriş modu","AUX modu"

//=================闹钟指令=====================//
#define SYS_ALARM_DEL_TR \
    "alarmı sil","alarmı iptal et","alarmı kapat","hatırlatıcıyı sil"

#define SYS_ALARM_DEL_ALL_TR \
    "tüm alarmları sil","tüm alarmları iptal et","tüm hatırlatıcıları kapat"

#define SYS_ALARM_LOOK_TR \
    "alarmları göster","hangi alarmlar ayarlı?","alarm sayfasını aç"

//=================音量调节指令=====================//
#define SYS_VOLUME_SET_TR  "ses seviyesini ayarla"

#define SYS_MUSIC_VOLUME_TR \
    "sesi kıs","müziğin sesini kıs", \
    "sesi aç","müziğin sesini aç"

#define SYS_MUSIC_MAX_VOLUME_TR \
    "maksimum ses seviyesi","sesi sonuna kadar aç"

#define SYS_MUSIC_MIN_VOLUME_TR \
    "minimum ses seviyesi","en düşük ses"

#endif
#endif // __TR_INSTRUCT_WORD_H__