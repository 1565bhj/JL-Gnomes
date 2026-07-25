#ifndef __YUE_INSTRUCT_WORD_H__
#define __YUE_INSTRUCT_WORD_H__

#ifdef CONFIG_INSTRUCTION_YUE
//=================粤语指令词=====================//

//=================系统指令=====================//
#define SYS_CLOSE_YUE \
    "熄咗佢","关咗佢","停低","退出","倾计停咗佢","唔好讲嘢", \
    "结束对话","唔想同你倾计"

#define SYS_STR_POWER_OFF_YUE \
    "熄咗部机"

#define SYS_30MIN_YUE \
    "一个半钟之后","等一个半钟", \
    "卅分钟之后","等卅分钟", \
    "半个钟之后","等半个钟"

#define SYS_90MIN_YUE \
    "一个半钟之后","等一个半钟"

#define SYS_HOUR_TIMEOUT_YUE \
    "一个钟之后","等一个钟"

#define SYS_MIN_TIMEOUT_YUE \
    "一阵之后","等几分钟"

#define SYS_COUNTOWN_TIMEOUT_YUE \
    "倒数计时","校计时器"

#define SYS_COUNTOWN_TIMEOUT_CLOSE_YUE \
    "取消倒数","删咗计时器","熄咗计时器"

//=================音乐指令=====================//
#define SYS_MUSIC_PLAY_YUE \
    "播歌","放音乐","唱首歌"

#define SYS_MUSIC_CONTINUE_YUE \
    "继续播歌","唔好停音乐","继续放"

#define SYS_MUSIC_STOP_YUE \
    "暂停","停低首歌","暂停音乐"

#define SYS_MUSIC_CLOSE_YUE \
    "停咗音乐","熄咗音乐","唔好播歌"

#define SYS_MUSIC_NEXT_YUE \
    "下一首歌","转下首歌","跳过呢首歌"

#define SYS_MUSIC_LAST_YUE \
    "上一首歌","返转头首歌"

#define SYS_MUSIC_LOOP_YUE \
    "循环单首歌","重复呢首歌"

#define SYS_MUSIC_LOOP_CLOSE_YUE \
    "取消循环","唔好重复","退出循环模式", \
    "唔好净系重复一首歌"

//=================设备控制指令=====================//
#define SYS_MUSIC_APP_AI_YUE \
    "智能模式","AI模式","助手模式"

#define SYS_MUSIC_APP_UDISK_YUE \
    "播USB","USB模式"

#define SYS_MUSIC_APP_SD_YUE \
    "播SD卡","播TF卡","SD模式","TF模式"

#define SYS_MUSIC_APP_BT_YUE \
    "蓝牙模式","用蓝牙播歌"

#define SYS_MUSIC_APP_AUX_YUE \
    "线路输入模式","AUX模式"

//=================闹钟指令=====================//
#define SYS_ALARM_DEL_YUE \
    "删咗闹钟","取消闹钟","熄咗闹钟","删咗提醒"

#define SYS_ALARM_DEL_ALL_YUE \
    "删晒所有闹钟","取消全部闹钟","熄晒所有提醒"

#define SYS_ALARM_LOOK_YUE \
    "睇下闹钟","有咩闹钟","打开闹钟页面"

//=================音量调节指令=====================//
#define SYS_VOLUME_SET_YUE  "校音量到"

#define SYS_MUSIC_VOLUME_YUE \
    "细声啲","收细音乐声", \
    "大声啲","开大音乐声"

#define SYS_MUSIC_MAX_VOLUME_YUE \
    "最大声","开到最尽"

#define SYS_MUSIC_MIN_VOLUME_YUE \
    "最细声","收到最细"

#endif
#endif // __YUE_INSTRUCT_WORD_H__