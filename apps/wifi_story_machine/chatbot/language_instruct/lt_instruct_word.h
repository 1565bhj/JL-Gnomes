#ifndef __IT_INSTRUCT_WORD_H__
#define __IT_INSTRUCT_WORD_H__

#ifdef CONFIG_INSTRUCTION_IT
//=================意大利语指令词=====================//

//=================系统指令=====================//
#define SYS_CLOSE_IT \
    "chiudi","spegni","ferma","esci","chiudi la conversazione","ferma il dialogo", \
    "termina la chiacchierata","non voglio più parlarti"

#define SYS_STR_POWER_OFF_IT \
    "spegni il dispositivo"

#define SYS_30MIN_IT \
    "un'ora e mezza dopo","tra un'ora e mezza", \
    "trenta minuti dopo","tra trenta minuti", \
    "mezz'ora dopo","tra mezz'ora"

#define SYS_90MIN_IT \
    "un'ora e mezza dopo","tra un'ora e mezza"

#define SYS_HOUR_TIMEOUT_IT \
    "un'ora dopo","tra un'ora"

#define SYS_MIN_TIMEOUT_IT \
    "un minuto dopo","tra qualche minuto"

#define SYS_COUNTOWN_TIMEOUT_IT \
    "conto alla rovescia","imposta il timer"

#define SYS_COUNTOWN_TIMEOUT_CLOSE_IT \
    "annulla conto alla rovescia","cancella il timer","ferma il timer"

//=================音乐指令=====================//
#define SYS_MUSIC_PLAY_IT \
    "riproduci musica","metti la canzone","suona la musica"

#define SYS_MUSIC_CONTINUE_IT \
    "continua la riproduzione","riprendi la musica","continua la canzone"

#define SYS_MUSIC_STOP_IT \
    "metti in pausa","pausa la musica","pausa la canzone"

#define SYS_MUSIC_CLOSE_IT \
    "ferma la musica","spegni la musica","termina la canzone"

#define SYS_MUSIC_NEXT_IT \
    "canzone successiva","brano successivo","cambia canzone","salta canzone"

#define SYS_MUSIC_LAST_IT \
    "canzone precedente","brano precedente"

#define SYS_MUSIC_LOOP_IT \
    "ripeti una sola canzone","modalità ripeti singolo"

#define SYS_MUSIC_LOOP_CLOSE_IT \
    "disattiva ripetizione","esci dalla modalità ripeti","annulla ripetizione", \
    "disattiva ripetizione singola"

//=================设备控制指令=====================//
#define SYS_MUSIC_APP_AI_IT \
    "modalità IA","modalità intelligente IA","modalità assistente intelligente"

#define SYS_MUSIC_APP_UDISK_IT \
    "riproduci USB","modalità USB"

#define SYS_MUSIC_APP_SD_IT \
    "riproduci scheda SD","riproduci scheda TF","modalità SD","modalità TF"

#define SYS_MUSIC_APP_BT_IT \
    "modalità Bluetooth","riproduci tramite Bluetooth"

#define SYS_MUSIC_APP_AUX_IT \
    "modalità linea ingresso","modalità AUX"

//=================闹钟指令=====================//
#define SYS_ALARM_DEL_IT \
    "cancella sveglia","annulla sveglia","spegni sveglia","cancella promemoria"

#define SYS_ALARM_DEL_ALL_IT \
    "cancella tutte le sveglie","annulla tutte le sveglie","spegni tutti i promemoria"

#define SYS_ALARM_LOOK_IT \
    "visualizza sveglie","quali sveglie sono impostate","apri pagina sveglie"

//=================音量调节指令=====================//
#define SYS_VOLUME_SET_IT  "imposta volume a"

#define SYS_MUSIC_VOLUME_IT \
    "abbassa volume","abbassa la musica", \
    "alza volume","alza la musica"

#define SYS_MUSIC_MAX_VOLUME_IT \
    "volume massimo","suono al massimo","il più forte possibile"

#define SYS_MUSIC_MIN_VOLUME_IT \
    "volume minimo","suono al minimo","il più basso possibile"

#endif
#endif // __IT_INSTRUCT_WORD_H__