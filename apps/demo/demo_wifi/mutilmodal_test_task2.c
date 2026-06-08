#include "app_config.h"
#include "system/includes.h"
#include "device/device.h"
#include "wifi/wifi_connect.h"
#include "web_socket/websocket_sxy_mutilmodal.h"

/*
 * 给客户提供的应用实例框架，使用qy-ai库封装的pipe-buffer方式来播放音频数据
 * 已经在AC792N平台上验证过接口
 */
//#define TEST_FOR_MUTILMODAL_USE_PIPE_BUFFER
#ifdef TEST_FOR_MUTILMODAL_USE_PIPE_BUFFER

#define MUTILMODAL_TASK_NAME "mutil_modal_main_test"
#define WEBSOCKET_CHUNK_SIZE      1024 //15 * 1024
static uint8_t img_tx_buf[WEBSOCKET_CHUNK_SIZE * 30]; //30
static int img_tx_len;

FILE *read_fd;
uint8_t customer_opt_mode = 0;

//每一轮开始之前
int qyai_audio_pipe_clear(void);
//播每一个chunk数据, payloadlen可以赋值给很大的值比如666666
int qyai_audio_pipe_buf_write(char *buf, int len, int payloadlen);
//最后一笔
int qyai_audio_pipe_last_pack(void);

//打断接口(唤醒打断/按键打断)
//01. 先调多模态的接口
qyai_mutilmodal_int_transfer();
//02. 再调pipe-buf终止接口
void qyai_audio_pipe_play_stop_all(void);

/*
 * Interrupt Context
 */
static void mutil_modal_dat_out_cb(uint8_t data_type, int chunk_len, uint8_t *dat_buffer, uint8_t seq_num, bool last_pkg)
{
    if (chunk_len > 0) {
        printf("app recv data_type:%d, chunk_len:%d, seq_num:%d, last_pkg:%d\r\n", data_type, chunk_len, seq_num, last_pkg);
        switch (data_type) {
        case DATA_TYPE_JPG: //文生图、文字+图生图、渲染图(固定模式)

            break;

        case DATA_TYPE_PCM: //万物识图模式
            printf("万物识图模式==================\n");
            //pipe-buf播放缓存
            qyai_audio_pipe_buf_write(dat_buffer, chunk_len, 666666);
            if (last_pkg) { //处理最后一个包
                qyai_audio_pipe_last_pack();
            }
            break;

        case DATA_TYPE_TEXT: //万物识图模式

            break;

        default:
            break;
        }
    } else {
        printf("error exception!\r\n");
    }
}

static void mutil_modal_thread_main(void *priv)
{
    printf("**********mutil_modal_thread_main***************\r\n");

    //pipe-buf初始化
    qyai_audio_pipe_clear();

    //示例：识图模式
    int ret = 0;
    uint8_t text3[] = "这张图里面都有什么";
    int text3_len = strlen(text3);
    printf("text3_len:%d\r\n", text3_len);

    //设定模式
    customer_opt_mode = OPT_MODE_TEXT_REC_IMG;

    //读取图片文件
    read_fd = fopen(CONFIG_ROOT_PATH"img/shucai.jpg", "r");

    if (read_fd) {
        fseek(read_fd, 0, SEEK_END);
        img_tx_len = ftell(read_fd); //获取整个文件大小
        fseek(read_fd, 0, SEEK_SET);
        fread(img_tx_buf, 1, img_tx_len, read_fd);
        fclose(read_fd);
        printf("img_tx_len:%d==========\r\n", img_tx_len);
    } else {
        printf("文件打开失败: %s\r\n", CONFIG_ROOT_PATH"img/shucai.jpg");
        return ;
    }

    printf("*****************万物识图 测试开始*************************************************\r\n");

    //阻塞式，同步接口
    ret = qyai_mutilmodal_textimg2_img(customer_opt_mode, text3, text3_len, img_tx_buf, img_tx_len);
    if (ret == 0) {
        printf("=======万物识图 测试成功=======\r\n");
    } else if (ret == -1) { //websocket创建失败
        printf("websocket client create fail\r\n");
    } else if (ret == -2) { //websocket发送数据失败
        printf("websocket send fail\r\n");
    } else if (ret == -3) { //websocket发送超时
        printf("websocket send timeout\r\n");
    } else if (ret == -4) { //reserve
        ;
    } else {;}

    printf("*****************万物识图 测试结束*************************************************\r\n");
}

static void mutil_modal_test(void)
{
    //注册多模态数据回调接口
    qyai_mutilmodal_reg_dat_out_cb(mutil_modal_dat_out_cb);

    thread_fork("http_websocket_start", 9, 2048, 0, NULL, mutil_modal_thread_main, NULL);

    //todo:
    while (1) {
        os_time_dly(100);
    }
}

int mutil_modal_main(void)
{
    os_task_create(mutil_modal_test, NULL, 12, 8192, 256, MUTILMODAL_TASK_NAME);
    return 0;
}

late_initcall(mutil_modal_main);
#endif
