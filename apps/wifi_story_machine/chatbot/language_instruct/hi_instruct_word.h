#ifndef __HI_INSTRUCT_WORD_H__
#define __HI_INSTRUCT_WORD_H__

#ifdef CONFIG_INSTRUCTION_HI
//=================印地语指令词=====================//

//=================系统指令=====================//
#define SYS_CLOSE_HI \
    "बंद करो","बंद कर दो","रुक जाओ","चले जाओ","बातचीत बंद करो","बातचीत रोको", \
    "बातचीत खत्म करो","मैं तुमसे बात नहीं करना चाहता"

#define SYS_STR_POWER_OFF_HI \
    "मशीन बंद करो"

#define SYS_30MIN_HI \
    "डेढ़ घंटे बाद","डेढ़ घंटे के बाद", \
    "तीस मिनट बाद","तीस मिनट के बाद", \
    "आधे घंटे बाद","आधे घंटे के बाद"

#define SYS_90MIN_HI \
    "डेढ़ घंटे बाद","डेढ़ घंटे के बाद"

#define SYS_HOUR_TIMEOUT_HI \
    "घंटे बाद","एक घंटे बाद"

#define SYS_MIN_TIMEOUT_HI \
    "मिनट बाद","कुछ मिनट बाद"

#define SYS_COUNTOWN_TIMEOUT_HI \
    "उलटी गिनती","टाइमर लगाओ"

#define SYS_COUNTOWN_TIMEOUT_CLOSE_HI \
    "उलटी गिनती रद्द करो","टाइमर हटाओ","टाइमर रद्द करो"

//=================音乐指令=====================//
#define SYS_MUSIC_PLAY_HI \
    "चलाओ गाना","संगीत चलाओ","गाना बजाओ"

#define SYS_MUSIC_CONTINUE_HI \
    "चलते रहो","गाना जारी रखो","संगीत जारी रखो"

#define SYS_MUSIC_STOP_HI \
    "थोड़ा रोको","गाना अस्थायी रोकें"

#define SYS_MUSIC_CLOSE_HI \
    "गाना बंद करो","संगीत बंद करो","गाना रोक दो"

#define SYS_MUSIC_NEXT_HI \
    "अगला गाना","गाना बदलो","अगला गाना चलाओ","गाना छोड़ो"

#define SYS_MUSIC_LAST_HI \
    "पिछला गाना","पिछला गाना चलाओ"

#define SYS_MUSIC_LOOP_HI \
    "एक गाना बार बार चलाओ"

#define SYS_MUSIC_LOOP_CLOSE_HI \
    "दोहराव बंद करो","दोहराव मोड बंद करो","दोहराव रद्द करो", \
    "एक गाने का दोहराव बंद करो"

//=================设备控制指令=====================//
#define SYS_MUSIC_APP_AI_HI \
    "AI मोड","स्मार्ट AI मोड","स्मार्ट असिस्टेंट मोड"

#define SYS_MUSIC_APP_UDISK_HI \
    "USB चलाओ","USB मोड"

#define SYS_MUSIC_APP_SD_HI \
    "SD कार्ड चलाओ","TF कार्ड चलाओ","SD कार्ड मोड","TF कार्ड मोड"

#define SYS_MUSIC_APP_BT_HI \
    "ब्लूटूथ मोड","ब्लूटूथ से चलाओ"

#define SYS_MUSIC_APP_AUX_HI \
    "लाइन मोड","ऑक्स इनपुट मोड"

//=================闹钟指令=====================//
#define SYS_ALARM_DEL_HI \
    "अलार्म हटाओ","अलार्म रद्द करो","अलार्म बंद करो","रिमाइंडर रद्द करो"

#define SYS_ALARM_DEL_ALL_HI \
    "सभी अलार्म हटाओ","सभी अलार्म रद्द करो","सभी अलार्म बंद करो"

#define SYS_ALARM_LOOK_HI \
    "अलार्म देखो","कौन से अलार्म लगे हैं","अलार्म पेज खोलो"

//=================音量调节指令=====================//
#define SYS_VOLUME_SET_HI  "आवाज़ सेट करो"

#define SYS_MUSIC_VOLUME_HI \
    "आवाज़ कम करो","गाने की आवाज़ कम करो", \
    "आवाज़ बढ़ाओ","गाने की आवाज़ बढ़ाओ"

#define SYS_MUSIC_MAX_VOLUME_HI \
    "पूरी आवाज़","सबसे ज्यादा आवाज़","अधिकतम वॉल्यूम"

#define SYS_MUSIC_MIN_VOLUME_HI \
    "कम आवाज़","सबसे कम आवाज़","न्यूनतम वॉल्यूम"

#endif
#endif // __HI_INSTRUCT_WORD_H__