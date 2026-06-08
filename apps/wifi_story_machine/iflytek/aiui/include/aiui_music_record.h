#ifndef __AIUI_MUSIC_RECORD_H__
#define __AIUI_MUSIC_RECORD_H__

typedef struct aiui_music_info_s {
    char *song_id; //      | 是       | string   | 根据多媒体类型，赋值相应的ID：歌曲ID、伴奏ID、MV的ID         |
    int duration;  //     | 是       | integer  | 歌曲时长（单位ms）                                           |
    int play_time; //    | 是       | integer  | 歌曲实际播放时长（单位ms）                                   |
    time_t lvt; //          | 是       | string   | 用户播放时间format: 2019-09-19 00:00:00
} aiui_music_info_t;

/*
    酷狗音乐播放记录上报
    appid， key 在https://aiui.xfyun.cn 应用信息中查询
    serial_num ：设备唯一ID
    itemID ：用来转换URL 的音乐ID，存放在item_info_t itemID字段中
*/

int aiui_music_record(aiui_music_info_t info);

#endif // __AIUI_MUSIC_RECORD_H__