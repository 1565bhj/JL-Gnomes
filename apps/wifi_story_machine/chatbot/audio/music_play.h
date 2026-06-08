#ifndef __MUSIC_PLAY_H__
#define __MUSIC_PLAY_H__

#include "server/audio_server.h"
#include "server/server_core.h"

int music_play_init(void);

void music_play_uninit(void);

//获取断点数据
int music_play_get_breakpoint(struct audio_dec_breakpoint *bp);

//设置音量大小
int music_play_set_volume(int step);

//获取解码器状态
int music_play_get_status(void);

//暂停/继续播放
int music_play_pause(void);

//停止播放
int music_play_stop(void);

//解码文件
int music_play_file(const char *path);

//解码资源文件
int music_play_res_file(const char *name);


#endif
