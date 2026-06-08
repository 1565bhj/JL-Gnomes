#include "app_config.h"
#include "system/includes.h"
#include "device/device.h"
#include "fs/fs.h"
#include "wifi/wifi_connect.h"
#include "os/os_api.h"
#include "system/init.h"
#include "websocket_sxy_mutilmodal.h"

#ifdef CONFIG_QYAI_MUTILMODAL_ENABLE

typedef struct mutil_info {
    void *img_buf;
    int img_buf_size;
    int img_offset;

    void *audio_buf;
    int audio_buf_size;
    int audio_seq;

    void *txt_buf;
    int txt_seq;

    void *recv_buf;
    int recv_buf_buf_size;

    void (*img_callback)(char *buf, int len);
    OS_SEM sem;
} mutil_info;

static mutil_info mutil_data;

#define __this (&mutil_data)

// static uint8_t txt_send_flag = 0;
static void txt_callback(char *txt, int txt_len)
{
    if (txt_len <= 0) {
        return;
    }
    printf("txt_callback: 发送到ui显示的文本长度%d: %s\n", txt_len, txt);
}

/*
 * Interrupt Context
 */
static void mutil_modal_dat_out_cb(uint8_t data_type, uint8_t *data_buffer, int data_len, uint16_t seq_num, bool last_pkg)
{
    int *dlen = NULL;
    if (data_len > 0) {
        printf("app recv data_type:%d, data_len:%d, seq_num:%d, last_pkg:%d\r\n", data_type, data_len, seq_num, last_pkg);
        switch (data_type) {
        case DATA_TYPE_JPG: //文生图、文字+图生图、渲染图(固定模式)
            if (seq_num == 0) {
                __this->img_offset = 0;
                dlen = __this->img_buf;
                *dlen = 0;
            }
            if (__this->img_buf && (__this->img_offset + 4 + data_len) <= __this->img_buf_size) {
                memcpy((int)__this->img_buf + 4 + __this->img_offset, data_buffer, data_len);
                __this->img_offset += data_len;
            }
            if (last_pkg) {
                dlen = __this->img_buf;         //前4个字节是有效长度
                *dlen = __this->img_offset;
            }
            break;

        case DATA_TYPE_PCM: //万物识图模式
            printf("---> DATA_TYPE_PCM seq : %d \n", seq_num);
            if (!seq_num) {
                qyai_audio_pipe_buf_play_stop();
                qyai_audio_pipe_buf_play_clear();
                qyai_audio_pipe_buf_play_start();
            }
            qyai_audio_pipe_buf_play_write(data_buffer, data_len, 1 * 1024 * 1024);
            __this->audio_seq++;
            break;

        case DATA_TYPE_TEXT: //万物识图模式
            printf("---> DATA_TYPE_TEXT seq : %d \n", seq_num);
            if (seq_num == 0) {//写文本
            }
            if (!last_pkg) {//写文本
            } else {
                qyai_audio_pipe_buf_play_last_pack();
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

#define MUTIL_TASK_NAME "mutil_task"
static int mutil_modal_task_send_msg(void *priv)
{
    int ret;
    char retry = 0;

    do {
        ret = os_taskq_post(MUTIL_TASK_NAME, 1, priv);
        if (ret != OS_NO_ERR) {
            if (ret != OS_Q_FULL) {
                return -1;
            }
            os_time_dly(5);
            retry++;
        } else {
            break;
        }
    } while (retry < 5);
    if (retry == 5) {
        printf("warning : OS_Q_FULL\n");
    }
    os_sem_set(&__this->sem, 0);
//    if(os_sem_pend(&__this->sem, 3000)) {
//        qyai_mutilmodal_int_transfer();//停止传输
//    }
    return ret;
}

int mutil_modal_request(enum Opt_mode_t Optmode, enum Render_mode_t Rendermode,
                        void *img_buf, int img_buf_size,
                        void *out_img_buf, int out_img_buf_size,
                        void *txt, int width, int height)
{
    static MUTIL_MDAL mutil ALIGNED(4);
    mutil.Optmode = Optmode;
    mutil.Rendermode = Rendermode;
    mutil.img_buf = img_buf;
    mutil.img_buf_size = img_buf_size;
    mutil.out_img_buf = out_img_buf;
    mutil.out_img_buf_size = out_img_buf_size;
    mutil.txt = txt;
    mutil.width = width;
    mutil.height = height;
    // printf("---------mutil.img_buf%p\r\n", mutil.img_buf);
    return mutil_modal_task_send_msg(&mutil);
}

void mutil_modal_img_callback_reg(void (*cb)(char *buf, int len))
{
    __this->img_callback = cb;
}

int mutil_modal_wait_jpg_done(int timeout_ticks)
{
    if (!os_sem_valid(&__this->sem)) {
        return -1;
    }
    return os_sem_pend(&__this->sem, timeout_ticks);
}

int mutil_modal_stop(void)
{
    printf("mutil_modal_stop\n");
    qyai_audio_pipe_play_stop();
    //qyai_music_buf_play_accept(0);
    qyai_mutilmodal_int_transfer();//停止传输
    return 0;
}

////测试
//void mutil_modal_test(void)
//{
//    mutil_modal_request(OPT_MODE_TEXT_REC_IMG, 0,
//                        veg_dat_buf, sizeof(veg_dat_buf),
//                        NULL, 0,
//                        "这张图有什么内容，要求回复30个字以内", 320, 256);
//}

static void mutil_modal_task(void)
{
    int ret;
    int msg[32];

    os_sem_create(&__this->sem, 0);
    //注册数据输出回调接口
    qyai_mutilmodal_reg_dat_out_cb(mutil_modal_dat_out_cb);
    while (1) {
        int res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        if (res == OS_TASK_DEL_IDLE) {
            break;
        } else if (res != OS_TASKQ) {
            continue;
        }
        //printf("---> msg[1] 0x%x\n",msg[1]);
        if (msg[0] == Q_USER) {
            MUTIL_MDAL *mutil = (MUTIL_MDAL *)msg[1];
            if (!mutil) {
                continue;
            }
            __this->img_buf = mutil->out_img_buf;
            __this->img_buf_size = mutil->out_img_buf_size;
            __this->audio_seq = 0;
            put_buf(mutil, sizeof(*mutil));
            switch (mutil->Optmode) {
            case OPT_MODE_TEXT2IMG:
                printf("OPT_MODE_TEXT2IMG\n");
                ret = qyai_mutilmodal_text2_img(mutil->txt, strlen(mutil->txt), mutil->width, mutil->height);
                if (ret) {
                    printf("OPT_MODE_TEXT2IMG err");
                }
                break;
            case OPT_MODE_TEXT_IMG2_IMG:
                printf("OPT_MODE_TEXT_IMG2_IMG\n");
                ret = qyai_mutilmodal_textimg2_img(mutil->Optmode, mutil->txt, strlen(mutil->txt), mutil->img_buf, mutil->img_buf_size);
                if (ret) {
                    printf("OPT_MODE_TEXT_IMG2_IMG err");
                }
                break;
            case OPT_MODE_TEXT_REC_IMG:
                printf("OPT_MODE_TEXT_REC_IMG\n");
                // printf("---------mutil.img_buf%p---------------\r\n", mutil->img_buf);
                // put_buf(mutil->img_buf, 16);
                ret = qyai_mutilmodal_textimg2_img(mutil->Optmode, mutil->txt, strlen(mutil->txt), mutil->img_buf, mutil->img_buf_size);
                if (ret) {
                    printf("OPT_MODE_TEXT_REC_IMG err");
                }
                break;
            case OPT_MODE_IMG2_IMG:
                printf("OPT_MODE_IMG2_IMG\n");
                ret = qyai_mutilmodal_img2_img(mutil->Rendermode, mutil->img_buf, mutil->img_buf_size);
                if (ret) {
                    printf("OPT_MODE_IMG2_IMG err");
                }
            default :
                printf("mutilmodal kill\n");
                qyai_mutilmodal_int_transfer();//停止传输
                break;
            }
            if (__this->audio_seq > 0 && ret) {
                qyai_audio_pipe_buf_play_last_pack();
            }
            if (__this->img_callback && !ret && __this->img_offset) {
                __this->img_callback(__this->img_buf + 4, __this->img_offset);
            }
            os_sem_post(&__this->sem);
        }
    }
}

static int mutil_modal_main(void)
{
    thread_fork(MUTIL_TASK_NAME, 20, 2048, 128, 0, mutil_modal_task, NULL);
    return 0;
}
late_initcall(mutil_modal_main);
#endif
