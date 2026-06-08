#pragma once

#include "app.h"
//#include "Cae1Proxy.h"

/*
* 功能：初始化CAE实例
* 参数：
*     vtn_cfg    引擎配置文件[in]
*     ivw_cb     输出唤醒结果的回调[in]
*     iat_cb     输出识别音频的回调[in]
*     user_data  用户私有数据[in]
* 返回值： 0 成功，其他失败
*/
int  CAENew(const char *vtn_cfg,
            ivw_res_cb ivw_cb,
            iat_audio_cb iat_cb,
            void *user_data);

/*
* 功能：写入音频数据
* 参数：
*     audioData     录音数据地址[in]
*     audioLen      录音数据长度[in]
* 返回值： 0 成功，其他失败
*/
int  CAEAudioWrite(const void *audio_data, unsigned int audio_len);

/*
* 功能：获取CAE版本号
* 参数：
* 返回值：CAE版本号字符串
*/
const char  *CAEGetVersion();

/*
* 功能：销毁实例
* 参数：
*     cae           实例地址[in]
* 返回值：0 成功，其他失败
*/
int  CAEDestroy();