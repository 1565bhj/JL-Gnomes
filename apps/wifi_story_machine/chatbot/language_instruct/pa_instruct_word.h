#ifndef __PA_INSTRUCT_WORD_H__
#define __PA_INSTRUCT_WORD_H__

#ifdef CONFIG_INSTRUCTION_PA
//=================旁遮普语(古尔穆基字母)指令词=====================//

//=================系统指令=====================//
#define SYS_CLOSE_PA \
    "ਬੰਦ ਕਰ","ਬੰਦ ਕਰ ਦਿਓ","ਰੋਕੋ","ਬਾਹਰ ਨਿਕਲੋ","ਗੱਲਬਾਤ ਬੰਦ ਕਰੋ","ਗੱਲਬਾਤ ਰੋਕੋ", \
    "ਗੱਲਬਾਤ ਖਤਮ ਕਰੋ","ਮੈਨੂੰ ਤੇਰੇ ਨਾਲ ਗੱਲ ਨਹੀਂ ਕਰਨੀ"

#define SYS_STR_POWER_OFF_PA \
    "ਮਸ਼ੀਨ ਬੰਦ ਕਰ"

#define SYS_30MIN_PA \
    "ਡੇਢ ਘੰਟੇ ਬਾਅਦ","ਡੇਢ ਘੰਟੇ ਪਿੱਛੋਂ", \
    "ਤੀਹ ਮਿੰਟ ਬਾਅਦ","ਤੀਹ ਮਿੰਟ ਪਿੱਛੋਂ", \
    "ਅੱਧੇ ਘੰਟੇ ਬਾਅਦ","ਅੱਧੇ ਘੰਟੇ ਪਿੱਛੋਂ"

#define SYS_90MIN_PA \
    "ਡੇਢ ਘੰਟੇ ਬਾਅਦ","ਡੇਢ ਘੰਟੇ ਪਿੱਛੋਂ"

#define SYS_HOUR_TIMEOUT_PA \
    "ਘੰਟੇ ਬਾਅਦ","ਇੱਕ ਘੰਟੇ ਬਾਅਦ"

#define SYS_MIN_TIMEOUT_PA \
    "ਮਿੰਟ ਬਾਅਦ","ਕੁਝ ਮਿੰਟ ਬਾਅਦ"

#define SYS_COUNTOWN_TIMEOUT_PA \
    "ਉਲਟੀ ਗਿਣਤੀ","ਟਾਈਮਰ ਲਗਾਓ"

#define SYS_COUNTOWN_TIMEOUT_CLOSE_PA \
    "ਉਲਟੀ ਗਿਣਤੀ ਰੱਦ ਕਰੋ","ਟਾਈਮਰ ਹਟਾਓ","ਟਾਈਮਰ ਰੱਦ ਕਰੋ"

//=================音乐指令=====================//
#define SYS_MUSIC_PLAY_PA \
    "ਗਾਣਾ ਚਲਾਓ","ਸੰਗੀਤ ਚਲਾਓ","ਗਾਣਾ ਵਜਾਓ"

#define SYS_MUSIC_CONTINUE_PA \
    "ਚਲਦੇ ਰਹੋ","ਗਾਣਾ ਜਾਰੀ ਰੱਖੋ","ਸੰਗੀਤ ਜਾਰੀ ਰੱਖੋ"

#define SYS_MUSIC_STOP_PA \
    "ਥੋੜ੍ਹਾ ਰੋਕੋ","ਗਾਣਾ ਅਸਥਾਈ ਤੌਰ 'ਤੇ ਰੋਕੋ"

#define SYS_MUSIC_CLOSE_PA \
    "ਗਾਣਾ ਬੰਦ ਕਰੋ","ਸੰਗੀਤ ਬੰਦ ਕਰੋ","ਗਾਣਾ ਰੋਕ ਦਿਓ"

#define SYS_MUSIC_NEXT_PA \
    "ਅਗਲਾ ਗਾਣਾ","ਗਾਣਾ ਬਦਲੋ","ਅਗਲਾ ਗਾਣਾ ਚਲਾਓ","ਗਾਣਾ ਛੱਡੋ"

#define SYS_MUSIC_LAST_PA \
    "ਪਿਛਲਾ ਗਾਣਾ","ਪਿਛਲਾ ਗਾਣਾ ਚਲਾਓ"

#define SYS_MUSIC_LOOP_PA \
    "ਇੱਕੋ ਗਾਣਾ ਵਾਰ ਵਾਰ ਵਜਾਓ"

#define SYS_MUSIC_LOOP_CLOSE_PA \
    "ਦੁਹਰਾਓ ਮੋਡ ਬੰਦ ਕਰੋ","ਲੂਪ ਮੋਡ ਤੋਂ ਬਾਹਰ ਨਿਕਲੋ","ਦੁਹਰਾਓ ਰੱਦ ਕਰੋ", \
    "ਇੱਕ ਗਾਣੇ ਦਾ ਦੁਹਰਾਓ ਬੰਦ ਕਰੋ"

//=================设备控制指令=====================//
#define SYS_MUSIC_APP_AI_PA \
    "AI ਮੋਡ","ਸਮਾਰਟ AI ਮੋਡ","ਸਮਾਰਟ ਸਹਾਇਕ ਮੋਡ"

#define SYS_MUSIC_APP_UDISK_PA \
    "USB ਚਲਾਓ","USB ਮੋਡ"

#define SYS_MUSIC_APP_SD_PA \
    "SD ਕਾਰਡ ਚਲਾਓ","TF ਕਾਰਡ ਚਲਾਓ","SD ਕਾਰਡ ਮੋਡ","TF ਕਾਰਡ ਮੋਡ"

#define SYS_MUSIC_APP_BT_PA \
    "ਬਲੂਟੁੱਥ ਮੋਡ","ਬਲੂਟੁੱਥ ਰਾਹੀਂ ਚਲਾਓ"

#define SYS_MUSIC_APP_AUX_PA \
    "ਲਾਈਨ ਮੋਡ","ਔਕਸ ਇਨਪੁਟ ਮੋਡ"

//=================闹钟指令=====================//
#define SYS_ALARM_DEL_PA \
    "ਅਲਾਰਮ ਹਟਾਓ","ਅਲਾਰਮ ਰੱਦ ਕਰੋ","ਅਲਾਰਮ ਬੰਦ ਕਰੋ","ਰਿਮਾਈਂਡਰ ਰੱਦ ਕਰੋ"

#define SYS_ALARM_DEL_ALL_PA \
    "ਸਾਰੇ ਅਲਾਰਮ ਹਟਾਓ","ਸਾਰੇ ਅਲਾਰਮ ਰੱਦ ਕਰੋ","ਸਾਰੇ ਅਲਾਰਮ ਬੰਦ ਕਰੋ"

#define SYS_ALARM_LOOK_PA \
    "ਅਲਾਰਮ ਦੇਖੋ","ਕਿਹੜੇ ਅਲਾਰਮ ਲੱਗੇ ਹਨ","ਅਲਾਰਮ ਪੇਜ ਖੋਲ੍ਹੋ"

//=================音量调节指令=====================//
#define SYS_VOLUME_SET_PA  "ਅਵਾਜ਼ ਸੈੱਟ ਕਰੋ"

#define SYS_MUSIC_VOLUME_PA \
    "ਅਵਾਜ਼ ਘਟਾਓ","ਗਾਣੇ ਦੀ ਅਵਾਜ਼ ਘਟਾਓ", \
    "ਅਵਾਜ਼ ਵਧਾਓ","ਗਾਣੇ ਦੀ ਅਵਾਜ਼ ਵਧਾਓ"

#define SYS_MUSIC_MAX_VOLUME_PA \
    "ਵੱਧ ਤੋਂ ਵੱਧ ਅਵਾਜ਼","ਸਭ ਤੋਂ ਉੱਚੀ ਅਵਾਜ਼ ਪੱਧਰ"

#define SYS_MUSIC_MIN_VOLUME_PA \
    "ਘੱਟ ਤੋਂ ਘੱਟ ਅਵਾਜ਼","ਸਭ ਤੋਂ ਘੱਟ ਅਵਾਜ਼ ਪੱਧਰ"

#endif
#endif // __PA_INSTRUCT_WORD_H__