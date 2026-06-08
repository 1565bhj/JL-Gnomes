#ifndef _ASR_SCORE_H_
#define _ASR_SCORE_H_

/**************************************************************************
 * Copyright (C) 2023  Wanson
 **************************************************************************
 *
 * Description : asr_score.h
 * Author      : tang pei
 * Date        : 2025.12.12 16:45
 *
 **************************************************************************/

/**
*Features:
*Modify the threshold for identifying an instruction
*wake_score ： Instructions and threshold for wakeword grouping
*cmd_score  ： Instructions and threshold for command group
*
*1、When initializing the program, call the following API
*   extern int wanson_threshold_update(const void* custom_wake_ptr, unsigned short custom_wake_size,const void* custom_cmd_ptr, unsigned short custom_cmd_size);
*   if (wanson_threshold_update(&wake_score, sizeof(wake_score), &cmd_score, sizeof(cmd_score)) < 0) {
*       printf("wanson_threshold_update Failed!");
*   }
*
*2、Declare the function "void wanson_score_display_set(unsigned char value);"
*   And after the voice initialization is completed, call "wanson_score_display_set(1);"
*   to display the score of the current voice recognition result.
*
*3、When the score of the voice recognition result is greater than the recognition command score predefined in "asr_score.h", it indicates a successful recognition.
*   The function "int Wanson_ASR_Recog(short* buf, int buf_len, const char** text, float* score);" will return the recognition result and save it in "text".
**/


#define ASR_CMD_LEN              25 //Please do not modify

typedef struct {
    int score;
    char cmd[ASR_CMD_LEN];
} asr_score_t;

const asr_score_t wake_score[] = {
    {-64000, "接听电话"},
    {-64000, "挂断电话"},
    {-64000, "拒接电话"},
    {-64000, "重播电话"},
    {-64000, "打开助手"},
    {-64000, "下次再聊"},
    {-64000, "小千小千"},
    {-64000, "小艾再见"},
    {-64000, "小艾小艾"},
    {-64000, "你好小艾"},

};

const asr_score_t cmd_score[] = {
    {-27300, "接听电话"},
    {-27300, "挂断电话"},
    {-27300, "拒接电话"},
    {-27300, "重播电话"},
    {-27300, "打开助手"},
    {-27300, "下次再聊"},
    {-27300, "小千小千"},
    {-27300, "小艾再见"},
    {-27300, "小艾小艾"},
    {-27300, "你好小艾"},
    {-27300, "开始配网"},
    {-27300, "播放音乐"},
    {-27300, "播放儿歌"},
    {-27300, "播放故事"},
    {-27300, "上一首"},
    {-27300, "下一首"},
    {-27300, "继续播放"},
    {-27300, "暂停播放"},
    {-27300, "停止播放"},
    {-27300, "调大音量"},
    {-27300, "调小音量"},
    {-27300, "最大音量"},
    {-27300, "最小音量"},
    {-27300, "切换到普通模式"},
    {-27300, "切换到畅聊模式"},
    {-27300, "切换到文心一言"},
    {-27300, "切换到豆包"},
    {-27300, "切换到弟普色克"},
    {-27300, "当前是什么大模型"},
    {-27300, "切换到通义千问"},
    {-27300, "下一套遥控码"},
    {-27300, "打开空调"},
    {-27300, "关闭空调"},
    {-27300, "自动风速"},
    {-27300, "自动模式"},
    {-27300, "制冷模式"},
    {-27300, "制热模式"},
    {-27300, "除湿模式"},
    {-27300, "送风模式"},
    {-27300, "中等风速"},
    {-27300, "最小风速"},
    {-27300, "最大风速"},
    {-27300, "三十度"},
    {-27300, "二十九度"},
    {-27300, "二十八度"},
    {-27300, "二十七度"},
    {-27300, "二十六度"},
    {-27300, "二十五度"},
    {-27300, "二十四度"},
    {-27300, "二十三度"},
    {-27300, "二十二度"},
    {-27300, "二十一度"},
    {-27300, "二十度"},
    {-27300, "十九度"},
    {-27300, "十八度"},
    {-27300, "十七度"},
    {-27300, "十六度"},
    {-27300, "打开扫风"},
    {-27300, "关闭扫风"},
    {-27300, "定时一小时"},
    {-27300, "定时二小时"},
    {-27300, "定时两小时"},
    {-27300, "定时三小时"},
    {-27300, "定时四小时"},
    {-27300, "定时五小时"},
    {-27300, "定时六小时"},
    {-27300, "定时七小时"},
    {-27300, "定时八小时"},
    {-27300, "取消定时"},
    {-27300, "匹配空调"},
    {-27300, "退出匹配"},
    {-27300, "搜索空调"},
    {-27300, "停止搜索"},
    {-27300, "创维空调"},
    {-27300, "奥克斯空调"},
    {-27300, "科龙空调"},
    {-27300, "大金空调"},
    {-27300, "美的空调"},
    {-27300, "新飞空调"},
    {-27300, "三菱空调"},
    {-27300, "小米空调"},
    {-27300, "夏普空调"},
    {-27300, "格兰仕空调"},
    {-27300, "海尔空调"},
    {-27300, "惠而浦空调"},
    {-27300, "乐金空调"},
    {-27300, "t c l 空调"},
    {-27300, "新科空调"},
    {-27300, "春兰空调"},
    {-27300, "扬子空调"},
    {-27300, "长虹空调"},
    {-27300, "日立空调"},
    {-27300, "三星空调"},
    {-27300, "松下空调"},
    {-27300, "海信空调"},
    {-27300, "澳柯玛空调"},
    {-27300, "志高空调"},
    {-27300, "富士通空调"},
    {-27300, "格力空调"},
    {-27300, "中等音量"},

};

#endif