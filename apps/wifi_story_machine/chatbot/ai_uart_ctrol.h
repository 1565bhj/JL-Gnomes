#ifndef __AI_UART_CMD_CTROL___
#define __AI_UART_CMD_CTROL___

#include "AT_UART_CMD.h"
typedef enum datatype {
//范围0：0 无， 1 息屏， 2 - 23 表情 类型
    AI_UART_CMD_EMOJI_DATA = 0,   //0：无-等待
    AI_UART_CMD_EMOJI_REST,       //1：休息-息屏
    AI_UART_CMD_EMOJI_HAPPY,      //2：开心笑
    AI_UART_CMD_EMOJI_SMILE,      //3：友好微笑
    AI_UART_CMD_EMOJI_EXCITE,     //4：兴奋期待
    AI_UART_CMD_EMOJI_QUIET,      //5：安静平和
    AI_UART_CMD_EMOJI_THINK,      //6：专注思考
    AI_UART_CMD_EMOJI_SAD,        //7：难过伤心
    AI_UART_CMD_EMOJI_GIEVAN,     //8：委屈想哭
    AI_UART_CMD_EMOJI_ANGRY,      //9：生气发火
    AI_UART_CMD_EMOJI_FRET,       //10：懊恼烦躁
    AI_UART_CMD_EMOJI_AMAZE,      //11：惊讶诧异
    AI_UART_CMD_EMOJI_DOUBT,      //12：疑惑不解
    AI_UART_CMD_EMOJI_SHY,        //13：害羞腼腆
    AI_UART_CMD_EMOJI_SLEEP,      //14：困倦疲惫
    AI_UART_CMD_EMOJI_REVEL,      //15：陶醉享受
    AI_UART_CMD_EMOJI_NAUGHTY,    //16：调皮捣蛋
    AI_UART_CMD_EMOJI_FEAR,       //17：恐惧害怕
    AI_UART_CMD_EMOJI_PROUD,      //18：骄傲自豪
    AI_UART_CMD_EMOJI_DEPPRESS,   //19：沮丧失落
    AI_UART_CMD_EMOJI_DESIRE,     //20：急切渴望
    AI_UART_CMD_EMOJI_GENTLE,     //21：温柔亲切
    AI_UART_CMD_EMOJI_INQUIST,    //22：好奇探索
    AI_UART_CMD_EMOJI_CUTE,       //23：可爱
    AI_UART_CMD_EMOJI_COLL,       //24：耍酷
    AI_UART_CMD_EMOJI_EMBARRASSED,//25：尴尬
    AI_UART_CMD_EMOJI_FANTASY,    //26：幻想发呆
    AI_UART_CMD_EMOJI_DIZZY,      //27：晕

    AI_UART_CMD_EMOJI_SHAKE = 45,  //45：摇晃
    //...预留

//范围2：    48 - 63 为AI对话 类型
    AI_UART_CMD_DIALUOGE_START = 48, //48：启动AI对话
    AI_UART_CMD_REC_START,           //49：正在录音
    AI_UART_CMD_REC_END,             //50：结束录音
    AI_UART_CMD_DIALUOGE_PLAY_START, //51：AI对话音频开始播放
    AI_UART_CMD_DIALUOGE_PLAY_END,   //52：AI对话音频结束播放
    AI_UART_CMD_AI_DIALUOGE_STOP,    //53：终止AI对话

    //...预留

//范围3：    64 - 79 为数据类型
    AI_UART_CMD_STT = 64,       //64：语音识别-stt
    AI_UART_CMD_TTS,            //65：AI对话-tts
    AI_UART_CMD_ALARM,          //66：闹钟-alarm设置 : {"alarm_cyc": 1,"alarm_ring": "2025-07-01 21:00:00"}
    AI_UART_CMD_ALARM_RING,     //67：闹钟时间到 : {"alarm_ring": "2025-07-01 21:00:00"}
    AI_UART_CMD_TIME_COUNT_DOWN,//68：计时器(单位秒) : {"time_count_down": 180}
    AI_UART_CMD_TIME_COUNT_RING,//69：计时器时间到(单位秒) : {"time_ring": 180}
    AI_UART_CMD_ALARM_DEL,      //70：删除闹钟 : {"alarm_del": "2025-07-01 21:00:00"}
    AI_UART_CMD_ALARM_DEL_ALL,  //71：删除所有闹钟
    AI_UART_CMD_NTP_TIME,       //72：获取基于NTP的本地RTC时间
    AI_UART_CMD_WEATHER_FORECASTL,//73：获取天气预报
    AI_UART_CMD_AI_INTELL_AGENT,  //74：设置智能体
    AI_UART_CMD_AI_INTELL_AGENT_INFO,//75：打印智能体信息
    //...预留

//范围4：    70 - 100 为系统命令 + 音乐命令 类型
    AI_UART_CMD_PWR_ON = 79,    //79：开机
    AI_UART_CMD_PWR_OFF,        //80：关机
    AI_UART_CMD_DIALUOGE_CLOSE, //81：对话关闭

    AI_UART_CMD_MUSIC_START,    //82：播放音乐
    AI_UART_CMD_MUSIC_PUASE,    //83：暂停播放
    AI_UART_CMD_MUSIC_CONTINUE, //84：继续播放
    AI_UART_CMD_MUSIC_STOP,     //85：停止播放
    AI_UART_CMD_MUSIC_NEXT,     //86：下一首
    AI_UART_CMD_MUSIC_LAST,     //87：上一首
    AI_UART_CMD_MUSIC_LOOP,     //88：单曲循环

    AI_UART_CMD_VOLUME_DEC,     //89：小声一点
    AI_UART_CMD_VOLUME_INC,     //90：大声一点
    AI_UART_CMD_VOLUME_SET,     //91：音量调到10 - 100，如音频值是60，则数据是：{"volume":60}
    AI_UART_CMD_MUSIC_LOOP_EXIT,//92：取消单曲循环

    AI_UART_CMD_NET_MUSIC_PLAY, //93：正在播放网络音乐
    AI_UART_CMD_NET_MUSIC_STOP, //94：停止播放网络音乐

    AI_UART_CMD_VBAT_FULL,      //95：满电电量
    AI_UART_CMD_VBAT_MIDIUM,    //96：中等电量
    AI_UART_CMD_VBAT_LOW,       //97：低电量
    AI_UART_CMD_VBAT_CHARGING,  //98：正在充电
    AI_UART_CMD_SYS_UPDATE,     //99：系统正在升级

//范围4：    100 - 121 为设备控制命令 类型
    AI_UART_CMD_LED_OPEN = 100,     //100：打开灯光
    AI_UART_CMD_LED_CLOSE,          //101：关闭灯光
    AI_UART_CMD_LED_EARTH_OPEN,     //102：打开球体灯
    AI_UART_CMD_LED_EARTH_CLOSE,    //103：关闭球体灯
    AI_UART_CMD_LED_BREATH_OPEN,    //104：打开呼吸灯
    AI_UART_CMD_LED_BIRGHT_INC,     //105：灯光亮一点/亮一点（控制灯光）
    AI_UART_CMD_LED_BIRGHT_DEC,     //106：灯光暗一点/暗一点（控制灯光）
    AI_UART_CMD_LED_BIRGHT_MAX,     //107：灯光最亮（控制灯光）

    AI_UART_CMD_UDISK_MODE,         //108：播放U盘
    AI_UART_CMD_SD_TF_MODE,         //109：播放SD卡，播放TF卡
    AI_UART_CMD_BT_MODE,            //110：播放蓝牙
    AI_UART_CMD_AUX_MODE,           //111：线路模式
    AI_UART_CMD_AI_MODE,            //112：AI模式
    AI_UART_CMD_MODE_CHENG,         //113：模式切换
    AI_UART_CMD_WIFI_CONFIG,        //114：wifi配网模式

    AI_UART_CMD_TIME_SHOWN_OPEN = 116,//116：打开时间显示
    AI_UART_CMD_TIME_SHOWN_CLOSE,    //117：关闭时间显示
    AI_UART_CMD_SHOWN_BIRGHT_INC,    //118：显示亮一点（控制显示）
    AI_UART_CMD_SHOWN_BIRGHT_DEC,    //119：显示暗一点（控制显示）

    AI_UART_CMD_MOTOR_START,        //120：马达开启
    AI_UART_CMD_MOTOR_END,          //121：马达停止
    //...预留

//范围5：    136 - 200 为设备设置和信息查询命令 类型
    AI_UART_CMD_GET_WIFI_SSID = 136,    //136：获取wifi已连接路由器SSID信息，{"ssid":"test-wifi","pwd":"123456"}
    AI_UART_CMD_GET_BT_SSID,            //137：获取蓝牙设备名称信息，{"bt_name":"QYAI_XXXX","ble_name":"QYAI_AC_XXXX"}

    AI_UART_CMD_SET_WIFI_SSID,          //138：wifi连接路由器SSID信息，{"ssid":"test-wifi","pwd":"123456"}
    AI_UART_CMD_SET_BT_SSID,            //139：蓝牙设备名称信息

    AI_UART_CMD_GET_VBAT,               //140：电量查询，带有数据则为电量值
    AI_UART_CMD_GET_NET_CONNET,         //141：查看wifi联网状态，联网成功：{"net_status":1}，无网络：{"net_status":0}
    AI_UART_CMD_GET_BT_CONNET,          //142：查看蓝牙连接状态，蓝牙已经连接：{"bt_status":1}，蓝牙未经连接：{"bt_status":0}

    AI_UART_CMD_DISCON_BT,              //143：断开蓝牙
    AI_UART_CMD_CONNECT_BT,             //144：连接蓝牙

    AI_UART_CMD_BT_DISCON,              //145：蓝牙已断开
    AI_UART_CMD_BT_CONNECTED,           //146：已连接蓝牙
    AI_UART_CMD_WIFI_DISCON,            //147：WiFi已断开
    AI_UART_CMD_WIFI_CONNECTED,         //148：已连接WIFI
    AI_UART_CMD_BT_CONNECTING,          //149：正在连接蓝牙
    AI_UART_CMD_WIFI_CONNECTING,        //150：正在连接WIFI


    //...预留

//范围1：    200 - 219 为其他随机表情 类型，总共12个
    AI_UART_CMD_EMOJI_DRAW_LOTS = 200,      //200：抽签
    AI_UART_CMD_EMOJI_DRAW_LOTS_SUCCESS = 201,//201：抽签完毕
    AI_UART_CMD_EMOJI_DRAW_LOTS_RANDOM1,      //202：抽签表情随机1
    AI_UART_CMD_EMOJI_DRAW_LOTS_RANDOM2,      //203：抽签表情随机2
    AI_UART_CMD_EMOJI_DRAW_LOTS_RANDOM3,      //204：抽签表情随机3
    AI_UART_CMD_EMOJI_DRAW_LOTS_RANDOM4,      //205：抽签表情随机4
    AI_UART_CMD_EMOJI_DRAW_LOTS_RANDOM5,      //206：抽签表情随机5

//范围1：    220 - 239 为其他随机表情 类型，总共12个
    AI_UART_CMD_EMOJI_RAND0 = 220,      //220：表情随机1
    AI_UART_CMD_EMOJI_RAND1,            //221：表情随机2
    AI_UART_CMD_EMOJI_RAND2,            //222：表情随机3
    AI_UART_CMD_EMOJI_RAND3,            //223：表情随机4
    AI_UART_CMD_EMOJI_RAND4,            //224：表情随机5

    AT_UART_CMD_LIGHT_OPEN = 300,            //300灯光控制
    AT_UART_CMD_LIGHT_CLOSE,
    AT_UART_CMD_LIGHT_BREATH,
    AT_UART_CMD_LIGHT_BREATH_INC,
    AT_UART_CMD_LIGHT_BREATH_DEC,
    AT_UART_CMD_LIGHT_BREATH_SET,

} DataType;

// 动画类型枚举
enum {
    ANIMATION_TYPE_EMOJI = 0,     // 表情动画
    ANIMATION_TYPE_DRAW_LOTS,     // 抽签动画
    ANIMATION_TYPE_RANDOM,        // 随机动画
    ANIMATION_TYPE_GYROSCOPE,     // 陀螺仪动画
    ANIMATION_TYPE_SLEEP,         // 睡眠动画
    ANIMATION_TYPE_OTHER          // 其他动画
};


/*
1、AI模组下发串口协议：2字节头部 + 2字节和校验 + 1字数据类型 + 1字预留类型 +  2字节数据长度  + n字节数据
                      |   2BYTE  |   2BYTE     |  1BYTE      |     1BYTE   |     2BYTE       | nBYTE ........  |
                      |   0xA55A |    sum      |  datatype   |     resv    |     len         | data0 ... datan |

头部head = 0xA55A
和校验sum：datatype + resv + len + data-0-n求和，即：除了head和sum之外的所有数据求和
数据类型datatype：详见 enum datatype
预留类型resv：0
数据长度len: 数据长度
数据data：len个字节的data0 ... datan
注意：数据存放格式：小端模式
*/

//发送
#define SEND_HEAD   0xA55A
#pragma pack(push, 4) //务必4字节对齐
struct ai_tx_uart_info { //7字节
    unsigned short head;//0xA55A
    unsigned short checksum;//和校验sum：len + data求和
    unsigned char datatype;//数据类型
    unsigned char resv;//预留：默认为0
    unsigned short len;//len数据的长度
    unsigned char data[0];//数据
};
#pragma pack(pop)

//接收
#define RECV_HEAD   0xABBA
#pragma pack(push, 1)
struct ai_rx_uart_head {
    unsigned short head;//0xABAB
    unsigned char err_type;//出错类型
    unsigned char resv;//预留
};
#pragma pack(pop)

#endif
