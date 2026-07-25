#ifndef __ES_INSTRUCT_WORD_H__
#define __ES_INSTRUCT_WORD_H__

#ifdef CONFIG_INSTRUCTION_ES
//=================西班牙语指令词=====================//

//=================系统指令=====================//
#define SYS_CLOSE_ES \
    "cerrar","apagar","parar","salir","cerrar la conversación","parar la charla", \
    "finalizar el diálogo","no quiero hablar más contigo"

#define SYS_STR_POWER_OFF_ES \
    "apagar el dispositivo"

#define SYS_30MIN_ES \
    "una hora y media después","dentro de una hora y media", \
    "treinta minutos después","dentro de treinta minutos", \
    "media hora después","dentro de media hora"

#define SYS_90MIN_ES \
    "una hora y media después","dentro de una hora y media"

#define SYS_HOUR_TIMEOUT_ES \
    "una hora después","dentro de una hora"

#define SYS_MIN_TIMEOUT_ES \
    "un minuto después","dentro de unos minutos"

#define SYS_COUNTOWN_TIMEOUT_ES \
    "cuenta regresiva","configurar temporizador"

#define SYS_COUNTOWN_TIMEOUT_CLOSE_ES \
    "cancelar cuenta regresiva","borrar temporizador","apagar temporizador"

//=================音乐指令=====================//
#define SYS_MUSIC_PLAY_ES \
    "reproducir música","poner canción","escuchar música"

#define SYS_MUSIC_CONTINUE_ES \
    "seguir reproduciendo","continuar la música","retomar la canción"

#define SYS_MUSIC_STOP_ES \
    "pausar","pausar música","pausar canción"

#define SYS_MUSIC_CLOSE_ES \
    "parar la música","apagar la música","terminar la canción"

#define SYS_MUSIC_NEXT_ES \
    "siguiente canción","siguiente tema","cambiar canción","saltar canción"

#define SYS_MUSIC_LAST_ES \
    "canción anterior","tema anterior"

#define SYS_MUSIC_LOOP_ES \
    "repetir una sola canción","modo repetir una"

#define SYS_MUSIC_LOOP_CLOSE_ES \
    "desactivar repetición","salir del modo repetir","cancelar repetición", \
    "desactivar repetir una sola canción"

//=================设备控制指令=====================//
#define SYS_MUSIC_APP_AI_ES \
    "modo IA","modo inteligente IA","modo asistente inteligente"

#define SYS_MUSIC_APP_UDISK_ES \
    "reproducir USB","modo USB"

#define SYS_MUSIC_APP_SD_ES \
    "reproducir tarjeta SD","reproducir tarjeta TF","modo SD","modo TF"

#define SYS_MUSIC_APP_BT_ES \
    "modo Bluetooth","reproducir por Bluetooth"

#define SYS_MUSIC_APP_AUX_ES \
    "modo línea entrada","modo AUX"

//=================闹钟指令=====================//
#define SYS_ALARM_DEL_ES \
    "borrar alarma","cancelar alarma","apagar alarma","borrar recordatorio"

#define SYS_ALARM_DEL_ALL_ES \
    "borrar todas las alarmas","cancelar todas las alarmas","apagar todos los despertadores"

#define SYS_ALARM_LOOK_ES \
    "ver alarmas","qué alarmas hay configuradas","abrir página de alarmas"

//=================音量调节指令=====================//
#define SYS_VOLUME_SET_ES  "ajustar volumen a"

#define SYS_MUSIC_VOLUME_ES \
    "bajar volumen","bajar la música", \
    "subir volumen","subir la música"

#define SYS_MUSIC_MAX_VOLUME_ES \
    "volumen máximo","sonido al máximo","lo más alto posible"

#define SYS_MUSIC_MIN_VOLUME_ES \
    "volumen mínimo","sonido al mínimo","lo más bajo posible"

#endif
#endif // __ES_INSTRUCT_WORD_H__