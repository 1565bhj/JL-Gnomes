#ifndef __FR_INSTRUCT_WORD_H__
#define __FR_INSTRUCT_WORD_H__

#ifdef CONFIG_INSTRUCTION_FR
//=================法语指令词=====================//

//=================系统指令=====================//
#define SYS_CLOSE_FR \
    "fermer","éteindre","arrêter","quitter","fermer la conversation","arrêter la discussion", \
    "terminer le dialogue","je ne veux plus te parler"

#define SYS_STR_POWER_OFF_FR \
    "éteindre l'appareil"

#define SYS_30MIN_FR \
    "une heure et demie plus tard","dans une heure et demie", \
    "trente minutes plus tard","dans trente minutes", \
    "une demi-heure plus tard","dans une demi-heure"

#define SYS_90MIN_FR \
    "une heure et demie plus tard","dans une heure et demie"

#define SYS_HOUR_TIMEOUT_FR \
    "une heure plus tard","dans une heure"

#define SYS_MIN_TIMEOUT_FR \
    "une minute plus tard","dans quelques minutes"

#define SYS_COUNTOWN_TIMEOUT_FR \
    "compte à rebours","régler le minuteur"

#define SYS_COUNTOWN_TIMEOUT_CLOSE_FR \
    "annuler le compte à rebours","supprimer le minuteur","arrêter le minuteur"

//=================音乐指令=====================//
#define SYS_MUSIC_PLAY_FR \
    "lancer la musique","jouer la chanson","mettre de la musique"

#define SYS_MUSIC_CONTINUE_FR \
    "continuer la lecture","reprendre la musique","relancer la chanson"

#define SYS_MUSIC_STOP_FR \
    "mettre en pause","pause la musique","pause la chanson"

#define SYS_MUSIC_CLOSE_FR \
    "arrêter la musique","couper la musique","terminer la chanson"

#define SYS_MUSIC_NEXT_FR \
    "chanson suivante","titre suivant","changer de chanson","passer la chanson"

#define SYS_MUSIC_LAST_FR \
    "chanson précédente","titre précédent"

#define SYS_MUSIC_LOOP_FR \
    "répéter une seule chanson","répétition simple"

#define SYS_MUSIC_LOOP_CLOSE_FR \
    "désactiver la répétition","sortir du mode répétition","annuler la répétition", \
    "arrêter la répétition simple"

//=================设备控制指令=====================//
#define SYS_MUSIC_APP_AI_FR \
    "mode IA","mode intelligent IA","mode assistant intelligent"

#define SYS_MUSIC_APP_UDISK_FR \
    "lire l'USB","mode USB"

#define SYS_MUSIC_APP_SD_FR \
    "lire carte SD","lire carte TF","mode carte SD","mode carte TF"

#define SYS_MUSIC_APP_BT_FR \
    "mode Bluetooth","lire via Bluetooth"

#define SYS_MUSIC_APP_AUX_FR \
    "mode ligne entrée","mode AUX"

//=================闹钟指令=====================//
#define SYS_ALARM_DEL_FR \
    "supprimer l'alarme","annuler l'alarme","éteindre l'alarme","supprimer le rappel"

#define SYS_ALARM_DEL_ALL_FR \
    "supprimer toutes les alarmes","annuler toutes les alarmes","éteindre tous les réveils"

#define SYS_ALARM_LOOK_FR \
    "voir les alarmes","quelles alarmes sont réglées","ouvrir la page des alarmes"

//=================音量调节指令=====================//
#define SYS_VOLUME_SET_FR  "régler le volume à"

#define SYS_MUSIC_VOLUME_FR \
    "baisser le volume","baisser la musique", \
    "augmenter le volume","monter la musique"

#define SYS_MUSIC_MAX_VOLUME_FR \
    "volume maximum","son au maximum","plus fort possible"

#define SYS_MUSIC_MIN_VOLUME_FR \
    "volume minimum","son au minimum","plus faible possible"

#endif
#endif // __FR_INSTRUCT_WORD_H__