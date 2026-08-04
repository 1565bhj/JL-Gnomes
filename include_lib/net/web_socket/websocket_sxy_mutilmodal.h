/*
 * Copyright (c) 2025 sxy-tech Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Implement Multimodal By WebSocket Protocal
 */
#ifndef __WEBSOCKET_SXY_MUTILMODAL_H__
#define __WEBSOCKET_SXY_MUTILMODAL_H__
//是否使用测试地址：正式发布，要注释掉，使用服务器正式地址
//#define CONFIG_USE_TEST_URL

//是否打印多模态通信过程中debug信息
#define CONFIG_QYAI_MUTIL_MODE_DEBUG_INFO (1)

//data_type
enum {
    DATA_TYPE_PCM = 0,  //PCM音频数据
    DATA_TYPE_TEXT,     //文本数据
    DATA_TYPE_JSON,     //json格式数据
    DATA_TYPE_OPUS,     //ogg-opus格式音频数据
    DATA_TYPE_JPG,      //jpg图片数据
    DATA_TYPE_GIF,      //gif视频数据
    DATA_TYPE_AVI,      //avi视频数据
    DATA_TYPE_RESERVE,  //预留
};

enum Opt_mode_t {
    OPT_MODE_TEXT2IMG = 0,  //文生图
    OPT_MODE_TEXT_IMG2_IMG, //文+图生图模式
    OPT_MODE_TEXT_REC_IMG,  //识图模式
    OPT_MODE_IMG2_IMG,      //固定渲染模式
    OPT_MODE_IMGS_TEXT_IMG, //多图融合模式
    OPT_MODE_RESERVER,
};

//render mode
enum Render_mode_t {
    RENDER_MODE_GHIBI = 0,  //网红日漫风
    RENDER_MODE_3D,         //3D风
    RENDER_MODE_REAL,       //写实风
    RENDER_MODE_ANGLE,      //天使风
    RENDER_MODE_CARTOON,    //动漫风
    RENDER_MODE_MAKOTO,     //日漫风
    RENDER_MODE_PRINCESS,   //公主风
    RENDER_MODE_DREAM,      //梦幻风
    RENDER_MODE_INK,        //水墨风
    RENDER_MODE_NEW_MONET,  //新莫奈花园
    RENDER_MODE_WATERCOLOR, //水彩风
    RENDER_MODE_MONET,      //莫奈花园
    RENDER_MODE_MARVEL,     //精致美漫
    RENDER_MODE_CYBER,      //赛博机械
    RENDER_MODE_KEREAN,     //精致韩漫
    RENDER_MODE_CHINESE_INK,    //国风-水墨
    RENDER_MODE_REMATIC,    //浪漫光影
    RENDER_MODE_CERAMIC,    //陶瓷娃娃
    RENDER_MODE_CHINESE_RED,    //中国红
    RENDER_MODE_CLAY,   //丑萌黏土
    RENDER_MODE_DOLL,   //可爱玩偶
    RENDER_MODE_GAME,   //3D-游戏_Z时代
    RENDER_MODE_MOVIE,  //动画电影
    RENDER_MODE_TOY,    //玩偶
};

typedef struct mutil_modal {
    enum Opt_mode_t Optmode;
    enum Render_mode_t Rendermode;
    void *img_buf;
    int img_buf_size;
    int img_size;
    void *out_img_buf;
    int out_img_buf_size;
    void *txt;
    int txt_len;
    int width;
    int height;
    void *img2_buf;        //多图融合：第二张图片缓冲区
    int img2_size;         //多图融合：第二张图片数据长度
} MUTIL_MDAL;


//以15K为一个块进行传输
#define WEBSOCKET_MAX_CHUNK_CNT 1000
#define WEBSOCKET_CHUNK_SIZE    (1024*15) //TODOD: 注意协议层传输最大包长的限制
#define WEBSOCKET_TIME_OUT_THE_FIRST_TRANSFER   (2500) //2500*10ms=25s
#define WEBSOCKET_TIME_OUT_BEHIND_TRANSFER  (1000) //1000*10ms=10s

/**
 * Callback function for data out phase of websocket.
 */
typedef void (*wbs_data_out_callback)(uint8_t data_type, uint8_t *data_buffer, int data_len, uint16_t seq_num, bool last_pkg);

void qyai_mutilmodal_reg_dat_out_cb(wbs_data_out_callback cb);
int qyai_mutilmodal_text2_img(uint8_t *text_in_buf, int text_in_len, int req_width, int req_height);
int qyai_mutilmodal_textimg2_img(uint8_t customer_opt_mode, uint8_t *text_in_buf, int text_in_len, uint8_t *img_in_buf, int img_in_len);
int qyai_mutilmodal_img2_img(uint8_t render_mode, uint8_t *img_in_buf, int img_in_len);
int qyai_mutilmodal_imgs_text_img(uint8_t render_mode, uint8_t *img_in_1_buf, int img_in_1_len, uint8_t *img_in_2_buf, int img_in_2_len, uint8_t *text_in_buf, int text_in_len);
//中断传输
void qyai_mutilmodal_int_transfer(void);

/*==============================================================================
 * 口语跟读打分(oral)
 *
 * 服务器地址: wss://<host>/multimodal/oral/<MAC>-<UUID>-<批次号>
 * 会话流程:
 *     建链 → 上行跟读原文(TEXT) → 流式上行录音(PCM) → 上行空帧(EOF)
 *     → 下行打分结果(JSON) → 下行评语语音(PCM) → 关闭连接
 *
 * 与文生图/识图/融合等接口不同，录音是流式上行的，无法在一个API里发完，
 * 所以拆成 start / push_pcm / finish 三段，会话期间连接一直保持:
 *     qyai_mutilmodal_oral_start()      建链并发送跟读原文(阻塞，需在任务中调用)
 *     qyai_mutilmodal_oral_push_pcm()   录音过程中持续推送PCM(可由采集线程调用)
 *     qyai_mutilmodal_oral_finish()     发EOF、等打分结果、关闭连接(阻塞)
 *     qyai_mutilmodal_oral_stop()       中断本轮会话(仍需由发起方调用finish收尾)
 * 打分结果通过 qyai_mutilmodal_oral_reg_result_cb() 注册的回调抛给应用层，
 * 评语语音走 qyai_mutilmodal_reg_dat_out_cb() 的 DATA_TYPE_PCM 数据流。
 *============================================================================*/
#define ORAL_COMMENT_MAX        128     //评语文本最大长度(含结束符)
#define ORAL_ASR_TEXT_MAX       128     //识别文本最大长度(含结束符)

//上行PCM分片大小: 3200字节≈100ms(16K采样/16bit/单声道)，与服务端测试脚本一致
#define ORAL_PCM_CHUNK_SIZE     3200

//打分结果
typedef struct oral_score_result {
    int  code;                          //服务器状态码: 200正常，其它为异常
    int  score;                         //1~10分，异常时为-1
    char comment[ORAL_COMMENT_MAX];     //评语文本(提示语)
    char asr_text[ORAL_ASR_TEXT_MAX];   //服务器识别到的跟读内容
} ORAL_SCORE_RESULT;

/**
 * 打分结果回调。
 * 注意: 在websocket接收线程上下文中调用，不能阻塞，也不能直接操作UI控件，
 *       应用层只做数据拷贝 + 发UI消息。
 */
typedef void (*wbs_oral_result_callback)(const ORAL_SCORE_RESULT *result);

void qyai_mutilmodal_oral_reg_result_cb(wbs_oral_result_callback cb);
int  qyai_mutilmodal_oral_start(uint8_t *text_in_buf, int text_in_len);
int  qyai_mutilmodal_oral_push_pcm(uint8_t *pcm_in_buf, int pcm_in_len);
int  qyai_mutilmodal_oral_finish(void);
void qyai_mutilmodal_oral_stop(void);
int  qyai_mutilmodal_oral_is_upload(void);
int  qyai_mutilmodal_oral_is_running(void);

#endif //__WEBSOCKET_SXY_MUTILMODAL_H__


