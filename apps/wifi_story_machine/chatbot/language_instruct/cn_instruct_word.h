#ifndef __CN_INSTRUCT_WORD_H__
#define __CN_INSTRUCT_WORD_H__

#ifdef CONFIG_INSTRUCTION_CN
//=================中文指令词=====================//

//=================系统指令=====================//
#define SYS_CLOSE_CN \
    "关了","关闭","关掉","停了","退下","对话关了","对话关闭","关掉对话","对话关掉","对话停了", \
    "结束对话","关闭对话","滚","不想和你说了","不想和你说话了"

#define SYS_STR_POWER_OFF_CN \
    "关机"

#define SYS_30MIN_CN \
    "一个半小时过后","一个半小时以后","一个半小时后", \
    "半个小时过后","半个小时以后","半个小时后", \
    "半小时过后","半小时以后","半小时后"

#define SYS_90MIN_CN \
    "一个小时半过后","一个小时半以后","一个小时半后"

#define SYS_HOUR_TIMEOUT_CN \
    "个小时过后","个小时以后","个小时后","小时过后","小时以后","小时后"

#define SYS_MIN_TIMEOUT_CN \
    "分钟过后","分钟以后","分钟后"

#define SYS_COUNTOWN_TIMEOUT_CN \
    "倒计时","计时"

#define SYS_COUNTOWN_TIMEOUT_CLOSE_CN \
    "取消计时","删除计时","取消倒计时","删除倒计时"

//=================音乐指令=====================//
#define SYS_MUSIC_PLAY_CN \
    "播放","播放歌曲","播放音乐"

#define SYS_MUSIC_CONTINUE_CN \
    "继续播放","继续播歌","继续播放音乐"

#define SYS_MUSIC_STOP_CN \
    "暂停","暂停播放","暂停播放音乐"

#define SYS_MUSIC_CLOSE_CN \
    "停止播歌","停止播放","停止播放音乐","关闭音乐","音乐关闭","关闭歌曲","歌曲关闭","音乐停止","停止音乐"

#define SYS_MUSIC_NEXT_CN \
    "下一首","播放下一首","换一首","换一首歌","换一首音乐","切歌","换歌"

#define SYS_MUSIC_LAST_CN \
    "上一首","播放上一首","播放上一首歌","播放上一首音乐"

#define SYS_MUSIC_LOOP_CN \
    "单曲循环"

#define SYS_MUSIC_LOOP_CLOSE_CN \
    "关闭循环","退出循环","取消循环", \
    "关闭单曲循环","退出单曲循环","取消单曲循环"

//=================设备控制指令=====================//
#define SYS_MUSIC_APP_AI_CN \
    "AI模式","Ai模式","AI智能体模式","智能体模式"

#define SYS_MUSIC_APP_UDISK_CN \
    "播放U盘","U盘播放模式","U盘模式"

#define SYS_MUSIC_APP_SD_CN \
    "播放SD卡","播放TF卡","SD卡播放模式","TF卡播放模式","SD卡模式","TF卡模式", \
    "Sd卡播放模式","Tf卡播放模式","Sd卡模式","Tf卡模式"

#define SYS_MUSIC_APP_BT_CN \
    "播放蓝牙","蓝牙模式","蓝牙播放模式"

#define SYS_MUSIC_APP_AUX_CN \
    "线路模式","线路输入模式"

//=================闹钟指令=====================//
#define SYS_ALARM_DEL_CN \
    "删除闹钟","取消闹钟","取消提醒","关闭闹钟"

#define SYS_ALARM_DEL_ALL_CN \
    "删除所有闹钟","取消所有闹钟","取消所有提醒","关闭所有闹钟"

#define SYS_ALARM_LOOK_CN \
    "查看闹钟","有哪些闹钟","闹钟页面","进入闹钟页面","查看闹钟页面"

//=================音量调节指令=====================//
#define SYS_VOLUME_SET_CN  "音量调到"

#define SYS_MUSIC_VOLUME_CN \
    "小声一点","音乐小声一点","音量调低","音量调小", \
    "大声一点","音乐大声一点","音量调高","音量调大"

#define SYS_MUSIC_MAX_VOLUME_CN \
    "最大音量","最高音量","音量最大","音量最高","最大声音","最高声音","声音最大","声音最高"

#define SYS_MUSIC_MIN_VOLUME_CN \
    "最小音量","最低音量","音量最小","音量最低","最小声音","最低声音","声音最小","声音最低"

#endif
#endif // __CN_INSTRUCT_WORD_H__
