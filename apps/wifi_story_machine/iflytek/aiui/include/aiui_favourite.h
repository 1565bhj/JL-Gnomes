
#ifndef __AIUI_FAVOURITE_H__
#define __AIUI_FAVOURITE_H__

/**
* 收藏/取消接口
* @param[in] itemID 音乐ID
* @param[in] type 接口类型， 1 收藏， 2取消
* @return
*/
int aiui_collect_song(const char *itemID, int type);

#endif    // __AIUI_FAVOURITE_H__
