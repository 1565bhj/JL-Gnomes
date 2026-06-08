#include "system/includes.h"
#include "server/server_core.h"
#include "server/audio_server.h"
#include "uni_ssp.h"
#include "uni_record.h"
#include "uni_kws.h"
#include "uni_record.h"
#include "app_config.h"

#if (defined CONFIG_ASR_ALGORITHM) && (CONFIG_ASR_ALGORITHM == KWS_YUNZS_ALGORITHM)

#define USER_MIC_COUNT   (1)

typedef struct _record_desc {
    int                     status;
    struct server         *mic_h;
    cbuffer_t               cbuf;
    char                   *cbuf_data;
    OS_SEM                  in_sem;
    OS_SEM                  w_sem;
    int                     to_online;
} uni_record_desc_t;

#define MIC_CHANGE_TWO   (1)
#define CBUF_SIZE       (1024*10)
#define W_CBUF_SIZE     (1024*4)
#define REC_TASK_NAME   "uni_rec"
#define WRITE_TASK_NAME   "uni_write"

#define PCM_FRAME_LEN      (512)


static uni_record_desc_t   _rec_dec = {0};
static uni_record_desc_t   *rec_dec_h = &_rec_dec;

static int g_uni_record_suspend_flag = 0;

static int uni_enc_out_cb(void* data, int len);
static void _rec_task_handle(void *para);
static void _write_task_handle(void *para);

user_kws_data_cb g_user_mic_data_cb = NULL;
user_kws_data_cb g_user_kws_data_cb = NULL;

int _rec_vfs_fwrite(void *file, void *buf, u32 len)
{
    if (rec_dec_h->status != 1) {
        return len;
    }

#if 1
    if (cbuf_is_write_able(&rec_dec_h->cbuf, len) < len) {
        printf("%s cbuf is full.\n", __func__);
    } else {
        //printf("%s, len: %d\n", __func__, len);
        cbuf_write(&rec_dec_h->cbuf, buf, len);
        os_sem_post(&rec_dec_h->in_sem);
    }
#endif
    // while(cbuf_is_write_able(&rec_dec_h->cbuf, len) < len){
    //  os_time_dly(1);
    // }
    // cbuf_write(&rec_dec_h->cbuf, buf, len);
    return len;
}

static int _rec_vfs_fclose(void *file)
{
    return 0;
}




static const struct audio_vfs_ops _rec_vfs_ops = {
    .fwrite = _rec_vfs_fwrite,
    .fclose = _rec_vfs_fclose,
};


static void _rec_event_handler(void *priv, int argc, int *argv)
{
    printf("get event from rec , event id is [%d]\n", argv[0]);
    puts("uni rec: AUDIO_SERVER_EVENT_ERR\n");
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
        puts("uni rec: AUDIO_SERVER_EVENT_ERR\n");
        break;
    case AUDIO_SERVER_EVENT_END:
        puts("uni rec: AUDIO_SERVER_EVENT_END\n");
        break;
    case AUDIO_SERVER_EVENT_SPEAK_START:
        puts("uni rec: AUDIO_SERVER_EVENT_SPEAK_START\n");
        printf("get event from rec , event id is AUDIO_SERVER_EVENT_SPEAK_STARTn");
        break;
    case AUDIO_SERVER_EVENT_SPEAK_STOP:
        puts("uni rec: AUDIO_SERVER_EVENT_SPEAK_STOP\n");
        break;
    default:
        break;
    }
}

int uni_record_suspend(void)
{
    if (rec_dec_h->status != 1) {
        return -1;
    }

    if (g_uni_record_suspend_flag) {
        return 0;
    }

    union audio_req req = {0};
    req.enc.cmd = AUDIO_ENC_STOP;
    server_request(rec_dec_h->mic_h, AUDIO_REQ_ENC, &req);

    memset(&req, 0, sizeof(req));
    req.enc.cmd = AUDIO_ENC_CLOSE;
    server_request(rec_dec_h->mic_h, AUDIO_REQ_ENC, &req);
    cbuf_clear(&rec_dec_h->cbuf);
    g_uni_record_suspend_flag = 1;
    return 0;
}

int uni_record_resume(void)
{
#define REC_SAMPLE_RATE   (16000)

    // if(rec_dec_h->status != 1){
    //     return -1;
    // }

    if (g_uni_record_suspend_flag == 0) {
        return -1;
    }

    int ret = -1;
    union audio_req EncodeReq = {0};
    int volume = 80; // 70;

    EncodeReq.enc.cmd               =   AUDIO_ENC_OPEN; //命令
    EncodeReq.enc.channel           =   USER_MIC_COUNT + 1; //2;        //采样通道数目
    EncodeReq.enc.volume            =   volume;         //录音音量
    EncodeReq.enc.output_buf        =   NULL;           //缓存buf
    EncodeReq.enc.output_buf_len    =   8 * 1024;       //缓存buf大小
    EncodeReq.enc.sample_rate       =   REC_SAMPLE_RATE;    //采样率
    EncodeReq.enc.format            =   "pcm";          //录音数据格式
    EncodeReq.enc.sample_source     =   "mic";  //采样源
    EncodeReq.enc.frame_size        =   512 * EncodeReq.enc.channel;
    EncodeReq.enc.vfs_ops           =   &_rec_vfs_ops;//文件操作方法集
    EncodeReq.enc.msec              =   0;              //采样时间
    EncodeReq.enc.frame_head_reserve_len = 0;
    EncodeReq.enc.use_vad             = 0; //2;
    EncodeReq.enc.vad_auto_refresh        = 1;

    //EncodeReq.enc.channel_bit_map = BIT(1)|BIT(3);
#if (USER_MIC_COUNT == 2)
    EncodeReq.enc.channel_bit_map   = BIT(0) | BIT(1) | BIT(2);
#else
    EncodeReq.enc.channel_bit_map   = BIT(0) | BIT(2);
#endif
    //EncodeReq.enc.channel_bit_map = BIT(1)|BIT(3);
    //EncodeReq.enc.channel_bit_map = 3;
    // EncodeReq.enc.channel_bit_map    = BIT(1)|BIT(2);
    // EncodeReq.enc.channel_bit_map    = BIT(0)|BIT(1)|BIT(2);

    ret = server_request(rec_dec_h->mic_h, AUDIO_REQ_ENC, &EncodeReq);
    g_uni_record_suspend_flag = 0;
    return ret;
}


static int uni_record_start(void)
{
    // #define REC_SAMPLE_RATE   (16000)
    int ret = -1;
    // union audio_req EncodeReq = {0};
    // int volume = 70;

    rec_dec_h->status = 0;
    rec_dec_h->cbuf_data = malloc(CBUF_SIZE);
    if (rec_dec_h->cbuf_data == NULL) {
        goto __err_out1;
    }

    cbuf_init(&rec_dec_h->cbuf, rec_dec_h->cbuf_data, CBUF_SIZE);

    rec_dec_h->mic_h = server_open("audio_server", "enc");
    if (rec_dec_h->mic_h == NULL) {
        goto __err_out2;
    }

    server_register_event_handler(rec_dec_h->mic_h, NULL, _rec_event_handler);
#if 0
    EncodeReq.enc.cmd               =   AUDIO_ENC_OPEN; //命令
    EncodeReq.enc.channel           =   2;      //采样通道数目
    EncodeReq.enc.volume            =   volume;         //录音音量
    EncodeReq.enc.output_buf        =   NULL;           //缓存buf
    EncodeReq.enc.output_buf_len    =   8 * 1024;       //缓存buf大小
    EncodeReq.enc.sample_rate       =   REC_SAMPLE_RATE;    //采样率
    EncodeReq.enc.format            =   "pcm";          //录音数据格式
    EncodeReq.enc.sample_source     =   "mic";  //采样源
    EncodeReq.enc.frame_size        =   512 * EncodeReq.enc.channel;
    EncodeReq.enc.vfs_ops           =   &_rec_vfs_ops;//文件操作方法集
    EncodeReq.enc.msec              =   0;              //采样时间
    EncodeReq.enc.frame_head_reserve_len = 0;
    EncodeReq.enc.use_vad             = 0; //2;
    EncodeReq.enc.vad_auto_refresh        = 1;

    //EncodeReq.enc.channel_bit_map = BIT(1)|BIT(3);
    EncodeReq.enc.channel_bit_map   = BIT(0) | BIT(1); //BIT(0)|BIT(2);
    //EncodeReq.enc.channel_bit_map = BIT(1)|BIT(3);
    //EncodeReq.enc.channel_bit_map = 3;
    // EncodeReq.enc.channel_bit_map    = BIT(1)|BIT(2);
    // EncodeReq.enc.channel_bit_map    = BIT(0)|BIT(1)|BIT(2);

    ret = server_request(rec_dec_h->mic_h, AUDIO_REQ_ENC, &EncodeReq);
#endif
    g_uni_record_suspend_flag = 1;
    ret = uni_record_resume();
    if (ret) {
        goto __err_out2;
    }

    os_sem_create(&rec_dec_h->in_sem, 0);
    rec_dec_h->status = 1;

    return 0;

__err_out2:
    server_close(rec_dec_h->mic_h);
__err_out1:
    if (rec_dec_h->cbuf_data) {
        free(rec_dec_h->cbuf_data);
        rec_dec_h->cbuf_data = NULL;
    }
    return -1;
}


static int uni_record_stop(void)
{
    union audio_req EncodeReq = {0};

    if (rec_dec_h->status != 1) {
        return -1;
    }

    rec_dec_h->status = 0;
    EncodeReq.enc.cmd = AUDIO_ENC_CLOSE;
    server_request(rec_dec_h->mic_h, AUDIO_REQ_ENC, &EncodeReq);
    server_close(rec_dec_h->mic_h);

    if (rec_dec_h->cbuf_data) {
        free(rec_dec_h->cbuf_data);
        rec_dec_h->cbuf_data = NULL;
    }
    os_sem_del(&rec_dec_h->in_sem, 1);
    return 0;
}

static void _uni_data_order(char* buffer, u32 len)
{
    u32 i = 0, cnt = 0;
    char mix_data[2];
    cnt = (len / 4);
    for (i = 0; i < cnt; i++) {
        mix_data[0] = buffer[i * 4 + 0];
        mix_data[1] = buffer[i * 4 + 1];

        buffer[i * 4 + 0] = buffer[i * 4 + 2];
        buffer[i * 4 + 1] = buffer[i * 4 + 3];
        buffer[i * 4 + 2] = mix_data[0];
        buffer[i * 4 + 3] = mix_data[1];
    }
}

static u32 _uni_data_3_to_2(char* buffer, u32 len)
{
    u32 i = 0, cnt = 0, idx = 0;
    cnt = (len / 6);
    for (i = 0; i < cnt; i++) {
        buffer[idx++] = buffer[i * 6 + 0];
        buffer[idx++] = buffer[i * 6 + 1];
        buffer[idx++] = buffer[i * 6 + 2];
        buffer[idx++] = buffer[i * 6 + 3];
    }
    return idx;
}

#define CHANNEL_NUM       (USER_MIC_COUNT+1) //(2) //1mic+ 1aec
#define FRAME_SAMPLES     (512)
#define UNI_BUFF_SIZE     (FRAME_SAMPLES*CHANNEL_NUM)
void AisEventCb(AikEvent event, void *args);
extern int uni_local_rec_data_put(void *buf, u32 len);
void uni_ssp_task(void *arg)
{
    printf("enter uni_ssp_task \n");
    void *hd;

    int ret;
    short *out = NULL;
    int out_size;
    short *buf = malloc(UNI_BUFF_SIZE);
    if (buf == NULL) {
        printf("<%s> malloc fail.\n", __func__);
        goto ssp_err;
    }

    out = malloc(FRAME_SAMPLES);
    if (out == NULL) {
        printf("<%s> malloc fail.\n", __func__);
        goto ssp_err;
    }

    uni_ais_event_set_cb(AisEventCb);
    ret = uni_asr_init();

    ret = uni_ssp_init();
    if (ret) {
        printf("<%s> ssp init fail.\n", __func__);
        goto ssp_err;
    }
    rec_dec_h->status = 0;
    rec_dec_h->cbuf_data = malloc(CBUF_SIZE);
    if (rec_dec_h->cbuf_data == NULL) {
        goto ssp_err;
    }

    os_sem_create(&rec_dec_h->in_sem, 0);
    cbuf_init(&rec_dec_h->cbuf, rec_dec_h->cbuf_data, CBUF_SIZE);

    ret = uni_record_start();
    if (ret) {
        printf("<%s> record start fail.\n", __func__);
        goto rec_err;
    }

    rec_dec_h->status = 1;
    int prev, total = 0, cnt = 0;
    float cos = 0.0;
    int i = 0;
    char c1, c2;
    while (1) {
        //putchar('S');
        os_sem_pend(&rec_dec_h->in_sem, 20);
        if (cbuf_get_data_size(&rec_dec_h->cbuf) < UNI_BUFF_SIZE) {
            continue;
        }

        ret = cbuf_read(&rec_dec_h->cbuf, buf, UNI_BUFF_SIZE);
        if (ret < UNI_BUFF_SIZE) {
            continue;
        }

//      if (g_user_mic_data_cb)
//          g_user_mic_data_cb(buf, ret);

//#ifdef WIFI_PCM_STREAN_SOCKET_ENABLE
//        static short wifi_stream_buf[UNI_BUFF_SIZE] ALIGNED(4);
//        int vl_idx = 0;
//        if(wifi_pcm_stream_socket_valid()){
//            short *pl_pcm_out = (short *)wifi_stream_buf;
//            for (volatile int i = 0, vl_idx = 0; i < 256; i++) {
//                *pl_pcm_out++ = buf[vl_idx + 1];//mic1
//                *pl_pcm_out++ = buf[vl_idx + 2];//ref
//                *pl_pcm_out++ = buf[vl_idx];//ref
////                *pl_pcm_out++ = aec_out_buf[i];
//                vl_idx += 3;
//            }
//            wifi_pcm_stream_socket_send((u8*)wifi_stream_buf, UNI_BUFF_SIZE);
//        }
//#endif

//#if (USER_MIC_COUNT == 2)
//      ret = _uni_data_3_to_2(buf, ret);
//#endif
//      _uni_data_order(buf, ret);


        short tmp;
        for (volatile int i = 0, vl_idx = 0; i < 256; i++) {
            tmp = buf[vl_idx + 1];
            buf[vl_idx + 1] = buf[vl_idx];//mic1
            buf[vl_idx] = tmp;
            vl_idx += 2;
        }

//
//#ifdef WIFI_PCM_STREAN_SOCKET_ENABLE
//        static short wifi_stream_buf[UNI_BUFF_SIZE] ALIGNED(4);
//        int vl_idx = 0;
//        if(wifi_pcm_stream_socket_valid()){
////            short *pl_pcm_out = (short *)wifi_stream_buf;
////            for (volatile int i = 0, vl_idx = 0; i < 256; i++) {
////                *pl_pcm_out++ = buf[vl_idx];//mic1
////                *pl_pcm_out++ = buf[vl_idx + 1];//ref
//////                *pl_pcm_out++ = aec_out_buf[i];
////                vl_idx += 2;
////            }
//            wifi_pcm_stream_socket_send((u8*)buf, 256 * 2 * 2);
//        }
//#endif

        //prev = jiffies_msec();
        //printf("before ssp\n");
        ret = uni_ssp_process(buf, &out, &out_size);
//        //printf("after ssp ret is [%d]\n",ret);
//      if(ret){
//          printf("<%s> ssp process auth timeout.\n", __func__);
//          continue;
//      }
//        out = buf;
//        out_size = 512;

//#ifdef WIFI_PCM_STREAN_SOCKET_ENABLE
//        static short wifi_stream_buf[UNI_BUFF_SIZE] ALIGNED(4);
//        int vl_idx = 0;
//        if(wifi_pcm_stream_socket_valid()){
//            short *pl_pcm_out = (short *)wifi_stream_buf;
//            for (volatile int i = 0, vl_idx = 0; i < 256; i++) {
//                *pl_pcm_out++ = out[vl_idx];//mic1
//                vl_idx++;
//
//            }
//            wifi_pcm_stream_socket_send((u8*)wifi_stream_buf, 256 * 2);
//        }
//#endif

        // total += jiffies_msec() - prev;
        // cnt++;
        // if(cnt == 60){
        //  cos = (float)total/(16*60);
        //  printf("ssp rtf: %f\n", cos);
        //  cnt = 0;
        //  total = 0;
        // }
//      if(ret){
//          printf("<%s> ssp process fail.\n", __func__);
//          continue;
//      }
        putchar('P');
        //发给本地识别
        //printf("send data to local asr\n");
//      uni_local_rec_data_put(out, out_size);

//        printf("before ssp = %d, %d\n",out_size,UNI_BUFF_SIZE);
        ret = uni_asr_process((void*)out, out_size);
        if (ret < 0) {
            printf("%s asr process auth timeout.\n", __func__);
        }
        void sys_vad_write_data(void *buf, int len);
        sys_vad_write_data(out, out_size);

//      //如果需要获取降噪+AEC 后的数据，可以在这里加一个接口包数据存到ringbuffer中
//      if (g_user_kws_data_cb)
//          g_user_kws_data_cb(out, out_size);
    }
    uni_record_stop();
rec_err:
    uni_ssp_release();
ssp_err:
    if (buf) {
        free(buf);
    }
    if (out) {
        free(out);
    }
    printf("<%s> exit.\n", __func__);
    return;
}

int uni_record_init(void)
{

    thread_fork("uni_ssp", 15, 4096 * 2, 0, NULL, uni_ssp_task, NULL);
    return 0;
}

int uni_record_uninit(void)
{

    return 0;
}

void user_set_mic_data_cb(user_kws_data_cb ucb)
{
    g_user_mic_data_cb = ucb;
}

void user_set_kws_data_cb(user_kws_data_cb ucb)
{
    g_user_kws_data_cb = ucb;
}

// 录音控制
int user_kws_request(int req_type, void *arg)
{
    if (rec_dec_h->status != 1) {
        return -1;
    }
    return server_request(rec_dec_h->mic_h, req_type, arg);
}

#endif
