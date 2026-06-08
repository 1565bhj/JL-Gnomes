#ifndef __AIUI_CONVERT_URL_H__
#define __AIUI_CONVERT_URL_H__


typedef enum {
    Source_Kugou_E = 0, /* 酷狗的信源 */
    Source_Kuwo_E,  /* 酷我的信源 */
    Source_Max_Value_E,
} Media_Source_Type_e;
/*
    酷我信源激活
    appid， key 在https://aiui.xfyun.cn 应用信息中查询
    serial_num ：设备唯一ID
    itemID ：用来转换URL 的音乐ID，存放在item_info_t itemID字段中
*/
int aiui_active(const char *appid, const char *key, const char *serial_num);
/*
    信源音乐的存储方式itemID，需要将itemID通过酷我的API接口来获取url
    itemID ：用来转换URL 的音乐ID，存放在item_info_t itemID字段中
    code: 换取失败时的错误码
    type:信源类型
    rate: MP3的位率 ，0 ：24kaac 1： 128kmp3，此参数的目的是为了防止有些url不支持128kmp3(实际中还未碰到).
          存放在item_info_t allrate字段中，如果包含128kmp3,则该参数传1，否则传0
*/
const char *aiui_convert_url(const char *itemID, int *duration, int *vip, int *code, Media_Source_Type_e type, int rate);

#endif // __AIUI_CONVERT_URL_H__
