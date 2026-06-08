#include "app_config.h"
#include "system/includes.h"
#include "device/device.h"
#include "fs/fs.h"
#include "wifi/wifi_connect.h"

#include "web_socket/websocket_sxy_mutilmodal.h"
#include "jpg_dat_test.h"

//用SD卡做存储介质进行读写, 注释掉则使用静态数组存储的图片数据
#define USE_SD_CARD_TO_TEST

#define MUTILMODAL_TASK_NAME "mutil_modal_main_test"
#define LIMIT_NUM (3)
static uint8_t img_tx_buf[WEBSOCKET_CHUNK_SIZE * 30];
static uint8_t img_rx_buf[WEBSOCKET_CHUNK_SIZE * 400];
static uint8_t text_rx_buf[WEBSOCKET_CHUNK_SIZE * 30];

uint8_t *rxvoice_buf_ptr = img_rx_buf;
uint8_t *rxtext_buf_ptr = text_rx_buf;

static int img_tx_len;
static int recv_cnt_temp;
static int recv_pcm_total;
static int recv_text_total;
static int recv_jpg_total;

FILE *read_fd;
FILE *write_fd;
static char filename[256];

uint8_t customer_opt_mode = 0;
int count1, count2, count3, count4 = 0;
extern int on_off;
extern int storage_device_ready(void);

/*
 * Interrupt Context
 */
static void mutil_modal_dat_out_cb(uint8_t data_type, int chunk_len, uint8_t *dat_buffer, uint8_t seq_num, bool last_pkg)
{
    if (chunk_len > 0) {
#ifdef CONFIG_QYAI_MUTIL_MODE_DEBUG_INFO
        printf("app recv data_type:%d, chunk_len:%d, seq_num:%d, last_pkg:%d\r\n", data_type, chunk_len, seq_num, last_pkg);
#endif
        switch (data_type) {
        case DATA_TYPE_JPG: //文生图、文字+图生图、渲染图(固定模式)
            printf("app recv data_type:%d, last_pkg:%d\r\n", data_type, last_pkg);
            memcpy(img_rx_buf + seq_num * WEBSOCKET_CHUNK_SIZE, dat_buffer, chunk_len);
            recv_jpg_total = recv_jpg_total + chunk_len;
            if (last_pkg) { //最后一个数据包
                printf("app recv_jpg_total:%d\r\n", recv_jpg_total);
            }
            break;

        case DATA_TYPE_PCM: //万物识图模式
            memcpy(rxvoice_buf_ptr + seq_num * WEBSOCKET_CHUNK_SIZE, dat_buffer, chunk_len);
            recv_cnt_temp = recv_cnt_temp + chunk_len;

            //简单的存储片段(片段拼接)
            if (last_pkg) {
                rxvoice_buf_ptr = rxvoice_buf_ptr + recv_cnt_temp;
                recv_pcm_total = recv_pcm_total + recv_cnt_temp;
                recv_cnt_temp = 0;
                printf("app recv_pcm_total:%d\r\n", recv_pcm_total);
            }
            break;

        case DATA_TYPE_TEXT: //万物识图模式
            //简单的存储片段(片段拼接)
            if (!last_pkg) {
                memcpy(rxtext_buf_ptr, dat_buffer, chunk_len);
                rxtext_buf_ptr = rxtext_buf_ptr + chunk_len;
                recv_text_total = recv_text_total + chunk_len;
            } else {
                printf("对话结束\r\n");
            }
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
    int ret;
#if 1 //单个模式测试

#if 0 //文生图测试
    uint8_t text1[] = "生成一张键盘图片";
    int text1_len = strlen(text1);
    printf("text1_len:%d\r\n", text1_len);

    count1 = 0;
    while (1) {
        if (on_off == 0) { //用按键来控制
            os_time_dly(200);
            continue;
        }

        printf("*****************文生图测试开始*************************************************\r\n");

        memset(img_rx_buf,  0, sizeof(img_rx_buf));
        memset(filename, 0, sizeof(filename));
        recv_jpg_total = 0;

        ret = qyai_mutilmodal_text2_img(text1, text1_len, 400, 600);
        if (!ret) {
            snprintf(filename, sizeof(filename), "%simg/text2img_mode_%d.jpg", CONFIG_ROOT_PATH, count1);

            //写到SD卡
            write_fd = fopen(filename, "wb");
            if (write_fd) {
                fwrite(img_rx_buf, 1, recv_jpg_total, write_fd);
                fclose(write_fd);
                printf("保存文件: %s (大小: %d 字节), 次数：%d\n", filename, recv_jpg_total, count1++);
            } else {
                printf("错误: 无法写入文件\r\n");
                count1 = 0;
                break;
            }
        } else {
            printf("the server exception:%d\r\n", ret);
            count1 = 0;
        }
        printf("*****************文生图测试结束, 次数:%d*************************************************\r\n", count1);

        on_off = 0;
#if 0
        //连续老化LIMIT_NUM次
        if (count1 == LIMIT_NUM) {
            while (1) {
                printf("===========idle1\r\n");
                os_time_dly(100);
            }
        }
#endif
    }
#endif

#if 0 //文+图=图(大模型根据文字进行编辑或渲染)
    //uint8_t text2[] = "把这张图转成动漫风格";
    //uint8_t text2[] = "给这只猫带上一顶帽子";
    uint8_t text2[] = "把这只猫头部变成动漫风格，然后身体是电影写真风格！再带上一顶帽子！";
    int text2_len = strlen(text2);
    printf("text2_len:%d\r\n", text2_len);
    count2 = 0;
    customer_opt_mode = OPT_MODE_TEXT_IMG2_IMG;

    //读取图片文件
    read_fd = fopen(CONFIG_ROOT_PATH"img/cat1.jpg", "r");
    if (read_fd) {
        fseek(read_fd, 0, SEEK_END);
        img_tx_len = ftell(read_fd); //获取整个文件大小
        fseek(read_fd, 0, SEEK_SET);
        fread(img_tx_buf, 1, img_tx_len, read_fd);
        fclose(read_fd);
    } else {
        printf("文件打开失败: %s\r\n", CONFIG_ROOT_PATH"img/cat1.jpg");
        return ;
    }

    while (1) {
        if (on_off == 0) {
            os_time_dly(200);
            continue;
        }
        printf("*****************文+图=图 测试开始*************************************************\r\n");

        memset(img_rx_buf,  0, sizeof(img_rx_buf));
        memset(filename, 0, sizeof(filename));
        recv_jpg_total = 0;

        ret = qyai_mutilmodal_textimg2_img(customer_opt_mode, text2, text2_len, img_tx_buf, img_tx_len);
        if (!ret) {
            snprintf(filename, sizeof(filename), "%simg/textimg2_img_mode_cat1_%d.jpg", CONFIG_ROOT_PATH, count2);

            //写到SD卡
            write_fd = fopen(filename, "wb");
            if (write_fd) {
                fwrite(img_rx_buf, 1, recv_jpg_total, write_fd);
                fclose(write_fd);
                printf("保存文件: %s (大小: %d 字节), 次数：%d\n", filename, recv_jpg_total, count2++);
            } else {
                printf("错误: 无法写入文件\r\n");
                count2 = 0;
                break;
            }
        } else if (ret == -1) { //websocket创建失败
            printf("websocket client create fail\r\n");
            count2 = 0;
        } else if (ret == -2) {
            printf("websocket send fail\r\n");
            count2 = 0;
        } else if (ret == -3) {
            printf("websocket send timeout\r\n");
            count2 = 0;
        } else if (ret == -4) { //reserve
            ;
        } else {;}

        printf("*****************文+图=图 测试结束,次数:%d*************************************************\r\n", count2);
        on_off = 0;
#if 0
        //连续老化LIMIT_NUM次
        if (count2 == LIMIT_NUM) {
            while (1) {
                printf("===========idle2\r\n");
                os_time_dly(100);
            }
        }
#endif
    }
#endif

#if 1 //识图模式
#ifdef USE_SD_CARD_TO_TEST
    //uint8_t text3[] = "这张图里面都有什么啊，给我科普一下？";
    uint8_t text3[] = "这张图里面有几种蔬菜，简要回答，十个字以内就行？";
    int text3_len = strlen(text3);
    printf("text3_len:%d\r\n", text3_len);

    customer_opt_mode = OPT_MODE_TEXT_REC_IMG;

    //读取图片文件
    //read_fd = fopen(CONFIG_ROOT_PATH"img/cup.jpg", "r");
    read_fd = fopen(CONFIG_ROOT_PATH"img/shucai_800x480.jpg", "r");
    //read_fd = fopen(CONFIG_ROOT_PATH"img/squre_800x480.jpg", "r");
    if (read_fd) {
        fseek(read_fd, 0, SEEK_END);
        img_tx_len = ftell(read_fd); //获取整个文件大小
        printf("img_tx_len:%d\r\n", img_tx_len);
        fseek(read_fd, 0, SEEK_SET);
        fread(img_tx_buf, 1, img_tx_len, read_fd);
        fclose(read_fd);
    } else {
        //printf("文件打开失败: %s\r\n", CONFIG_ROOT_PATH"img/cup.jpg");
        //printf("文件打开失败: %s\r\n", CONFIG_ROOT_PATH"img/shucai_800x480.jpg");
        printf("文件打开失败: %s\r\n", CONFIG_ROOT_PATH"img/squre_800x480.jpg");
        return ;
    }

    count3 = 0;
    while (1) {
        if (on_off == 0) {
            os_time_dly(200);
            continue;
        }
        printf("*****************万物识图 测试开始*************************************************\r\n");

        rxvoice_buf_ptr = img_rx_buf;
        rxtext_buf_ptr = text_rx_buf;
        memset(img_rx_buf,  0, sizeof(img_rx_buf));
        memset(text_rx_buf, 0, sizeof(text_rx_buf));
        memset(filename, 0, sizeof(filename));
        recv_pcm_total = 0;
        recv_text_total = 0;

        ret = qyai_mutilmodal_textimg2_img(customer_opt_mode, text3, text3_len, img_tx_buf, img_tx_len);

        if (ret == 0) {
            snprintf(filename, sizeof(filename), "%simg/rec_img_mode_%d.mp3", CONFIG_ROOT_PATH, count3);

            //服务器返回的音频文件写到SD卡
            write_fd = fopen(filename, "wb");
            if (write_fd) {
                fwrite(img_rx_buf, 1, recv_pcm_total, write_fd);
                fclose(write_fd);
                printf("保存文件: %s (大小: %d 字节), 次数：%d\n", filename, recv_pcm_total, count3++);
            } else {
                printf("错误: 无法写入文件\r\n");
                count3 = 0;
            }

            printf("text_rx_buf: %s\r\n", text_rx_buf);

        } else if (ret == -1) { //websocket创建失败
            printf("websocket client create fail\r\n");
            count3 = 0;
        } else if (ret == -2) {
            printf("websocket send fail\r\n");
            count3 = 0;
        } else if (ret == -3) {
            printf("websocket send timeout\r\n");
            count3 = 0;
        } else if (ret == -4) {
            printf("system error occurred!\r\n");
            count3 = 0;
        } else {;}//reserve

        printf("*****************万物识图 测试结束，次数:%d*************************************************\r\n", count3);
        on_off = 0;
#if 0
        //连续老化LIMIT_NUM次
        if (count3 == LIMIT_NUM) {
            while (1) {
                printf("===========idle3\r\n");
                os_time_dly(100);
            }
        }
#endif
    }
#else  //直接使用静态数组里面的图片数据
    uint8_t text3[] = "这张图里面都有什么啊，给我科普一下？";
    int text3_len = strlen(text3);
    printf("text3_len:%d\r\n", text3_len);
    customer_opt_mode = OPT_MODE_TEXT_REC_IMG;

    while (1) {
        if (on_off == 0) {
            os_time_dly(200);
            continue;
        }
        printf("*****************万物识图 测试开始*************************************************\r\n");

        rxvoice_buf_ptr = img_rx_buf;
        rxtext_buf_ptr = text_rx_buf;
        memset(img_rx_buf,  0, sizeof(img_rx_buf));
        memset(text_rx_buf, 0, sizeof(text_rx_buf));
        recv_pcm_total = 0;
        recv_text_total = 0;

        //蔬菜图，图片信息复杂
        ret = qyai_mutilmodal_textimg2_img(customer_opt_mode, text3, text3_len, shucai_dat_buf, sizeof(shucai_dat_buf));

        //方形图，图片信息单一
        //ret=qyai_mutilmodal_textimg2_img(customer_opt_mode, text3, text3_len, squre_dat_buf, sizeof(squre_dat_buf));

        //没有存储介质，直接扔掉服务器返回的音频数据；打印出对应的文本
        printf("text_rx_buf: %s\r\n", text_rx_buf);

        if (ret == 0) {
            printf("successful, opt_mode:%d\r\n", customer_opt_mode);
        } else if (ret == -1) { //websocket创建失败
            printf("websocket client create fail\r\n");
            count3 = 0;
        } else if (ret == -2) {
            printf("websocket send fail\r\n");
            count3 = 0;
        } else if (ret == -3) {
            printf("websocket send timeout\r\n");
            count3 = 0;
        } else if (ret == -4) { //reserve
            ;
        } else {;}

        printf("*****************万物识图 测试结束，次数:%d*************************************************\r\n", count3);
        on_off = 0;
    }
#endif // USE_SD_CARD_TO_TEST
#endif

#if 0   //固定渲染模式
    //enum Render_mode_t render_mode_opt=RENDER_MODE_MOVIE; //动画电影风格
    enum Render_mode_t render_mode_opt = RENDER_MODE_3D;
    //读取图片文件
    //read_fd = fopen(CONFIG_ROOT_PATH"img/cup.jpg", "r");
    read_fd = fopen(CONFIG_ROOT_PATH"img/shucai.jpg", "r");
    if (read_fd) {
        fseek(read_fd, 0, SEEK_END);
        img_tx_len = ftell(read_fd); //获取整个文件大小
        fseek(read_fd, 0, SEEK_SET);
        fread(img_tx_buf, 1, img_tx_len, read_fd);
        fclose(read_fd);
    } else {
        //printf("文件打开失败: %s\r\n", CONFIG_ROOT_PATH"img/cup.jpg");
        printf("文件打开失败: %s\r\n", CONFIG_ROOT_PATH"img/shucai.jpg");
        return ;
    }

    count4 = 0;
    while (1) {
        if (on_off == 0) {
            os_time_dly(200);
            continue;
        }
        printf("*****************固定渲染模式测试开始*************************************************\r\n");

        memset(img_rx_buf,  0, WEBSOCKET_CHUNK_SIZE * 20);
        recv_jpg_total = 0;

        ret = qyai_mutilmodal_img2_img(render_mode_opt, img_tx_buf, img_tx_len);
        if (!ret) {
            snprintf(filename, sizeof(filename), "%simg/render_mode_%d.jpg", CONFIG_ROOT_PATH, count4);

            //写到SD卡
            write_fd = fopen(filename, "wb");
            if (write_fd) {
                fwrite(img_rx_buf, 1, recv_jpg_total, write_fd);
                fclose(write_fd);
                printf("保存文件: %s (大小: %d 字节), 次数：%d\n", filename, recv_jpg_total, count4++);
            } else {
                printf("错误: 无法写入文件\r\n");
                count4 = 0;
            }
        } else {
            printf("the server exception\r\n");
            count4 = 0;
        }

        printf("*****************固定渲染模式测试结束:%d*************************************************\r\n", count4);
        on_off = 0;
#if 0
        //连续老化LIMIT_NUM次
        if (count4 == LIMIT_NUM) {
            while (1) {
                printf("===========idle4\r\n");
                os_time_dly(100);
            }
        }
#endif
    }
#endif

#else //多模式混合测试(待)

#endif
}

static void mutil_modal_test(void)
{
    int ret;
    int msg[4];
    int wifi_state_code;

#ifdef USE_SD_CARD_TO_TEST
    printf("========================开始挂载SD卡文件系统\r\n");
    while (!storage_device_ready()) { //等待sd文件系统挂载完成
        os_time_dly(100);
        printf("======detect sd card!\r\n");
    }
    printf("========================成功挂载SD卡文件系统\r\n");
#endif // USE_SD_CARD_TO_TEST

    //注册数据输出回调接口
    qyai_mutilmodal_reg_dat_out_cb(mutil_modal_dat_out_cb);

    printf("========================网络传输测试\r\n");
    thread_fork("http_websocket_start", 9, 2048, 0, NULL, mutil_modal_thread_main, NULL);

    while (1) {
        os_time_dly(100);
    }
}

static int mutil_modal_main(void)
{
    os_task_create(mutil_modal_test, NULL, 12, 8192, 256, MUTILMODAL_TASK_NAME);
    return 0;
}

late_initcall(mutil_modal_main);

