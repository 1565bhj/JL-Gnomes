#ifndef __EN_INSTRUCT_WORD_H__
#define __EN_INSTRUCT_WORD_H__

#ifdef CONFIG_INSTRUCTION_EN
//=================英文指令词=====================//

//=================System Instructions=====================//
#define SYS_CLOSE_EN \
    "turn off","close it","close","shut it down","shut down","shutdown","stop it","stop","byebye","goodbye", \
    "Turn Off","Close it","Close","Shut it down","Shut down","Shutdown","Stop it","Stop","Byebye","Goodbye", \
    "system_exit","system exit", \
    "System_exit","System exit"

#define SYS_STR_POWER_OFF_EN \
    "power off","Power off", \
    "turn off power","Turn off power"

#define SYS_30MIN_EN \
    "After half an hour", \
    "after half an hour"

#define SYS_90MIN_EN \
    "After one and a half hours", \
    "after one and a half hours"

#define SYS_HOUR_TIMEOUT_EN \
    "hours after"

#define SYS_MIN_TIMEOUT_EN \
    "minutes later"

#define SYS_COUNTOWN_TIMEOUT_EN \
    "Countdown", \
    "countdown"

#define SYS_COUNTOWN_TIMEOUT_CLOSE_EN \
    "Cancel timer","Delete timer","Cancel countdown","Delete countdown", \
    "cancel timer","delete timer","cancel countdown","delete countdown"

//=================Music Instructions=====================//
#define SYS_MUSIC_PLAY_EN \
    "Play","Play song","Play music", \
    "play","play song","play music"

#define SYS_MUSIC_CONTINUE_EN \
    "Continue playing","Continue playing song","Continue playing music", \
    "continue playing","continue playing song","continue playing music", \
    "music_resume","music resume", \
    "Music_resume","Music resume"

#define SYS_MUSIC_STOP_EN \
    "Pause","Pause playback","Pause music", \
    "pause","pause playback","pause music", \
    "music_pause","music pause", \
    "Music_pause","Music pause"

#define SYS_MUSIC_CLOSE_EN \
    "Stop song","Stop playback","Stop music","Turn off music","Music off","Turn off song","Song off","Music stop","Stop music", \
    "stop song","stop playback","stop music","turn off music","music off","turn off song","song off","music stop","stop music", \
    "music_stop","music stop", \
    "Music_stop","Music stop"

#define SYS_MUSIC_NEXT_EN \
    "Next song","Play next","Change one","Change song","Change music","Skip song","Change track", \
    "next song","play next","change one","change song","change music","skip song","change track", \
    "music_next_track","music next track", \
    "Music_next_track","Music next track"

#define SYS_MUSIC_LAST_EN \
    "Previous song","Play previous","Play previous song","Play previous music", \
    "previous song","play previous","play previous song","play previous music", \
    "music_previous_track","music previous track", \
    "Music_previous_track","Music previous track"

#define SYS_MUSIC_LOOP_EN \
    "Single track loop","Single song loop","Single song cycle", \
    "single track loop","single song loop","single song cycle", \
    "music_loop_single","music loop single", \
    "Music_loop_single","Music loop single"

#define SYS_MUSIC_LOOP_CLOSE_EN \
    "Turn off loop","Exit loop","Cancel loop", \
    "Turn off single track loop","Exit single track loop","Cancel single track loop", \
    "Turn off single song loop","Exit single song loop","Cancel single song loop", \
    "turn off loop","exit loop","cancel loop", \
    "turn off single track loop","exit single track loop","cancel single track loop", \
    "turn off single song loop","exit single song loop","cancel single song loop"

//=================Device Control Instructions=====================//
#define SYS_MUSIC_APP_AI_EN \
    "AI mode","AI agent mode","Agent mode", \
    "Ai mode","Ai agent mode","agent mode", \
    "ai mode","ai agent mode","agent mode"

#define SYS_MUSIC_APP_UDISK_EN \
    "Play USB disk","USB disk play mode","USB disk mode", \
    "play USB disk","usb disk play mode","usb disk mode", \
    "source_usb","source usb", \
    "Source_usb","Source usb"

#define SYS_MUSIC_APP_SD_EN \
    "Play SD card","Play TF card","SD card play mode","TF card play mode","SD card mode","TF card mode", \
    "SD card play mode","TF card play mode","SD card mode","TF card mode","SD mode","TF mode", \
    "play SD card","play TF card","sd card play mode","tf card play mode","sd card mode","tf card mode", \
    "sd card play mode","tf card play mode","SD card mode","TF card mode","sd mode","tf mode", \
    "source_sdcard","source sdcard", \
    "Source_sdcard","Source sdcard"

#define SYS_MUSIC_APP_BT_EN \
    "Play Bluetooth","Bluetooth mode","Bluetooth play mode", \
    "play Bluetooth","bluetooth mode","bluetooth play mode"

#define SYS_MUSIC_APP_AUX_EN \
    "Line mode","Line input mode", \
    "line mode","line input mode"

//=================Alarm Instructions=====================//
#define SYS_ALARM_DEL_EN \
    "Delete alarm","Cancel alarm","Cancel reminder","Turn off alarm", \
    "delete alarm","cancel alarm","cancel reminder","turn off alarm"

#define SYS_ALARM_DEL_ALL_EN \
    "Delete all alarms","Cancel all alarms","Cancel all reminders","Turn off all alarms", \
    "delete all alarms","cancel all alarms","cancel all reminders","turn off all alarms"

#define SYS_ALARM_LOOK_EN \
    "View alarms","What alarms are there","Alarm page","Enter alarm page","View alarm page", \
    "view alarms","what alarms are there","alarm page","enter alarm page","view alarm page"

//=================Volume Control Instructions=====================//
#define SYS_VOLUME_SET_EN  "volume_set_"

#define SYS_MUSIC_VOLUME_EN \
    "A little quieter","Music a little quieter","Turn volume down","Lower volume", \
    "A little louder","Music a little louder","Turn volume up","Increase volume", \
    "a little quieter","music a little quieter","turn volume down","lower volume", \
    "a little louder","music a little louder","turn volume up","increase volume", \
    "volume_down","volume down","set volume_down","set volume down", \
    "volume_up","volume up","set volume_up","set volume up", \
    "Volume_down","Volume down","Set volume_down","Set volume down", \
    "Volume_up","Volume up","Set volume_up","Set volume up"

#define SYS_MUSIC_MAX_VOLUME_EN \
    "max volume","maximum volume","Max volume","Maximum volume"

#define SYS_MUSIC_MIN_VOLUME_EN \
    "min volume","minimum volume","Min volume","Minimum volume"

#endif // CONFIG_KWS_ENGLISH

#endif // __EN_INSTRUCT_WORD_H__
