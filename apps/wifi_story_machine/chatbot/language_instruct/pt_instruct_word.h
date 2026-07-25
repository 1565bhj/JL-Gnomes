#ifndef __PT_INSTRUCT_WORD_H__
#define __PT_INSTRUCT_WORD_H__

#ifdef CONFIG_INSTRUCTION_PT
//=================葡萄牙语指令词=====================//

//=================系统指令=====================//
#define SYS_CLOSE_PT \
    "fechar","desligar","parar","sair","fechar conversa","parar bate-papo", \
    "finalizar diálogo","não quero mais falar com você"

#define SYS_STR_POWER_OFF_PT \
    "desligar aparelho"

#define SYS_30MIN_PT \
    "uma hora e meia depois","daqui a uma hora e meia", \
    "trinta minutos depois","daqui a trinta minutos", \
    "meia hora depois","daqui a meia hora"

#define SYS_90MIN_PT \
    "uma hora e meia depois","daqui a uma hora e meia"

#define SYS_HOUR_TIMEOUT_PT \
    "uma hora depois","daqui a uma hora"

#define SYS_MIN_TIMEOUT_PT \
    "um minuto depois","daqui a alguns minutos"

#define SYS_COUNTOWN_TIMEOUT_PT \
    "contagem regressiva","configurar temporizador"

#define SYS_COUNTOWN_TIMEOUT_CLOSE_PT \
    "cancelar contagem regressiva","apagar temporizador","desligar temporizador"

//=================音乐指令=====================//
#define SYS_MUSIC_PLAY_PT \
    "reproduzir música","tocar música","colocar música"

#define SYS_MUSIC_CONTINUE_PT \
    "continuar reprodução","retomar música","continuar a faixa"

#define SYS_MUSIC_STOP_PT \
    "pausar","pausar música","pausar faixa"

#define SYS_MUSIC_CLOSE_PT \
    "parar música","desligar música","encerrar faixa"

#define SYS_MUSIC_NEXT_PT \
    "próxima música","próxima faixa","trocar música","pular música"

#define SYS_MUSIC_LAST_PT \
    "música anterior","faixa anterior"

#define SYS_MUSIC_LOOP_PT \
    "repetir apenas uma música","modo repetir uma faixa"

#define SYS_MUSIC_LOOP_CLOSE_PT \
    "desativar repetição","sair do modo repetição","cancelar repetição", \
    "desligar repetição de uma única faixa"

//=================设备控制指令=====================//
#define SYS_MUSIC_APP_AI_PT \
    "modo IA","modo inteligente IA","modo assistente inteligente"

#define SYS_MUSIC_APP_UDISK_PT \
    "reproduzir USB","modo USB"

#define SYS_MUSIC_APP_SD_PT \
    "reproduzir cartão SD","reproduzir cartão TF","modo SD","modo TF"

#define SYS_MUSIC_APP_BT_PT \
    "modo Bluetooth","reproduzir via Bluetooth"

#define SYS_MUSIC_APP_AUX_PT \
    "modo entrada de linha","modo AUX"

//=================闹钟指令=====================//
#define SYS_ALARM_DEL_PT \
    "apagar alarme","cancelar alarme","desligar alarme","apagar lembrete"

#define SYS_ALARM_DEL_ALL_PT \
    "apagar todos os alarmes","cancelar todos os alarmes","desligar todos os despertadores"

#define SYS_ALARM_LOOK_PT \
    "ver alarmes","quais alarmes estão configurados","abrir página de alarmes"

//=================音量调节指令=====================//
#define SYS_VOLUME_SET_PT  "ajustar volume para"

#define SYS_MUSIC_VOLUME_PT \
    "abaixar volume","diminuir som da música", \
    "aumentar volume","subir som da música"

#define SYS_MUSIC_MAX_VOLUME_PT \
    "volume máximo","som no máximo","o mais alto possível"

#define SYS_MUSIC_MIN_VOLUME_PT \
    "volume mínimo","som no mínimo","o mais baixo possível"

#endif
#endif // __PT_INSTRUCT_WORD_H__