#ifndef __GU_INSTRUCT_WORD_H__
#define __GU_INSTRUCT_WORD_H__

#ifdef CONFIG_INSTRUCTION_GU
//=================古吉拉特语指令词=====================//

//=================系统指令=====================//
#define SYS_CLOSE_GU \
    "બંધ કર","બંધ કરી દો","રોકો","બહાર નીકળો","વાતચીત બંધ કરો","વાતચીત રોકો", \
    "વાતચીત પૂરી કરો","મારે તારી સાથે વાત નથી કરવી"

#define SYS_STR_POWER_OFF_GU \
    "મશીન બંધ કર"

#define SYS_30MIN_GU \
    "દોઢ કલાક પછી","દોઢ કલાક બાદ", \
    "ત્રીસ મિનિટ પછી","ત્રીસ મિનિટ બાદ", \
    "અડધો કલાક પછી","અડધો કલાક બાદ"

#define SYS_90MIN_GU \
    "દોઢ કલાક પછી","દોઢ કલાક બાદ"

#define SYS_HOUR_TIMEOUT_GU \
    "કલાક પછી","એક કલાક પછી"

#define SYS_MIN_TIMEOUT_GU \
    "મિનિટ પછી","થોડી મિનિટ પછી"

#define SYS_COUNTOWN_TIMEOUT_GU \
    "ઊલટી ગણતરી","ટાઈમર સેટ કરો"

#define SYS_COUNTOWN_TIMEOUT_CLOSE_GU \
    "ઊલટી ગણતરી રદ કરો","ટાઈમર કાઢી નાખો","ટાઈમર રદ કરો"

//=================音乐指令=====================//
#define SYS_MUSIC_PLAY_GU \
    "ગીત ચલાવો","સંગીત ચલાવો","ગીત વગાડો"

#define SYS_MUSIC_CONTINUE_GU \
    "ચલાવતા રહો","ગીત ચલાવતા રહો","સંગીત ચલાવતા રહો"

#define SYS_MUSIC_STOP_GU \
    "થોડું રોકો","ગીતને ક્ષણિક રોકો"

#define SYS_MUSIC_CLOSE_GU \
    "ગીત બંધ કરો","સંગીત બંધ કરો","ગીત રોકી દો"

#define SYS_MUSIC_NEXT_GU \
    "આગલું ગીત","ગીત બદલો","આગલું ગીત ચલાવો","ગીત છોડો"

#define SYS_MUSIC_LAST_GU \
    "પાછલું ગીત","પાછલું ગીત ચલાવો"

#define SYS_MUSIC_LOOP_GU \
    "એક જ ગીત વારંવાર વગાડો"

#define SYS_MUSIC_LOOP_CLOSE_GU \
    "રિપીટ મોડ બંધ કરો","લૂપ મોડમાંથી બહાર નીકળો","રિપીટ રદ કરો", \
    "એક ગીતનું રિપીટ બંધ કરો"

//=================设备控制指令=====================//
#define SYS_MUSIC_APP_AI_GU \
    "AI મોડ","સ્માર્ટ AI મોડ","સ્માર્ટ સહાયક મોડ"

#define SYS_MUSIC_APP_UDISK_GU \
    "USB ચલાવો","USB મોડ"

#define SYS_MUSIC_APP_SD_GU \
    "SD કાર્ડ ચલાવો","TF કાર્ડ ચલાવો","SD કાર્ડ મોડ","TF કાર્ડ મોડ"

#define SYS_MUSIC_APP_BT_GU \
    "બ્લૂટૂથ મોડ","બ્લૂટૂથ દ્વારા ચલાવો"

#define SYS_MUSIC_APP_AUX_GU \
    "લાઈન મોડ","ઓક્સ ઇનપુટ મોડ"

//=================闹钟指令=====================//
#define SYS_ALARM_DEL_GU \
    "એલાર્મ કાઢી નાખો","એલાર્મ રદ કરો","એલાર્મ બંધ કરો","રિમાઇન્ડર રદ કરો"

#define SYS_ALARM_DEL_ALL_GU \
    "બધા એલાર્મ કાઢી નાખો","બધા એલાર્મ રદ કરો","બધા એલાર્મ બંધ કરો"

#define SYS_ALARM_LOOK_GU \
    "એલાર્મ જુઓ","કયા એલાર્મ છે","એલાર્મ પેજ ખોલો"

//=================音量调节指令=====================//
#define SYS_VOLUME_SET_GU  "અવાજનું સ્તર સેટ કરો"

#define SYS_MUSIC_VOLUME_GU \
    "અવાજ ઓછો કરો","ગીતનો અવાજ ઓછો કરો", \
    "અવાજ વધારો","ગીતનો અવાજ વધારો"

#define SYS_MUSIC_MAX_VOLUME_GU \
    "મહત્તમ અવાજ","સૌથી વધુ અવાજ સ્તર"

#define SYS_MUSIC_MIN_VOLUME_GU \
    "ન્યૂનતમ અવાજ","સૌથી ઓછો અવાજ સ્તર"

#endif
#endif // __GU_INSTRUCT_WORD_H__