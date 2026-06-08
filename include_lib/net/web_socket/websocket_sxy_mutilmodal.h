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
	void *out_img_buf;
	int out_img_buf_size;
	void *txt;
	int txt_len;
	int width;
	int height;
}MUTIL_MDAL;


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
//中断传输
void qyai_mutilmodal_int_transfer(void);

#endif //__WEBSOCKET_SXY_MUTILMODAL_H__


