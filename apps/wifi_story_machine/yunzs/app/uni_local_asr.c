#include "system/includes.h"
#include "uni_kws.h"
#include "uni_record.h"
#include "app_config.h"


#if (defined CONFIG_ASR_ALGORITHM) && (CONFIG_ASR_ALGORITHM == KWS_YUNZS_ALGORITHM)
#define FRAME_SAMPLES     (512)
#define CBUF_SIZE       (FRAME_SAMPLES*10)


static cbuffer_t               cbuf;
static char                   *cbuf_data;
static OS_SEM                  in_sem;

static int g_uni_local_asr_init_flag = 0;
user_kws_event_cb g_user_kws_cb = NULL;

int uni_local_rec_data_put(void *buf, u32 len)
{

    if (cbuf_is_write_able(&cbuf, len) < len) {
        printf("%s cbuf is full.\n", __func__);
        return -1;
    } else {
        cbuf_write(&cbuf, buf, len);
    }
    return 0;
}

void AisEventCb(AikEvent event, void *args)
{
    switch (event) {
    case  AIK_EVENT_KWS_WAKEUP: {
            AikEventKwsArgs *kws = (AikEventKwsArgs *)args;
            printf("KWS#####offline_result:[wakeup_normal]#####command[%s]#####score[%f]#####{wakeup}#index[%d]#is_oneshot[%d]\n", kws->word, kws->score, kws->kws_index, kws->is_oneshot);
            if (g_user_kws_cb) {
                g_user_kws_cb(kws->word, kws->score, WAKEUP_WORD, kws->is_oneshot);
            } else {
                uni_mode_switch(CMD_MODE);
            }
            break;
        }
    case AIK_EVENT_KWS_COMMAND: {
            AikEventKwsArgs *kws = (AikEventKwsArgs *)args;
            printf("KWS#####offline_result:[asr_normal]#####command[%s]#####score[%f]#####{asr}#index[%d]#is_oneshot[%d]\n", kws->word, kws->score, kws->kws_index, kws->is_oneshot);
            if (g_user_kws_cb) {
                g_user_kws_cb(kws->word, kws->score, CMD_WORD, kws->is_oneshot);
            }
            break;
        }
    case AIK_EVENT_NONE:
        printf("AIK_EVENT_NONE\n");
        break;
    case  AIK_EVENT_START:
        printf("AIK_EVENT_START\n");
        break;
    case  AIK_EVENT_STOP:
        printf("AIK_EVENT_STOP\n");
        break;
    case  AIK_EVENT_EXIT:
        printf("AIK_EVENT_EXIT\n");
        break;
    case  AIK_EVENT_KWS_TIMEOUT:
        if (g_user_kws_cb == NULL) {
            uni_mode_switch(WAKEUP_MODE);
        }
        printf("AIK_EVENT_KWS_TIMEOUT\n");
        break;
    case  AIK_EVENT_HEARTBEAT:
        printf("AIK_EVENT_HEARTBEAT\n");
        break;
    default:
        printf("Nothing\n");
        break;
    }
    return;
}

extern int uni_record_init(void);
void local_asr_task(void *param)
{
    int ret;
    printf("%s start.\n", __func__);
    short *data = malloc(512);
    if (data == NULL) {
        printf("%s malloc fail.\n", __func__);
        goto step_1;
    }

    uni_ais_event_set_cb(AisEventCb);
    ret = uni_asr_init();
    if (ret) {
        printf("%s asr init fail.\n", __func__);
        goto step_2;
    }

    printf("%s asr init success.\n", __func__);

    ret = uni_record_init();
    if (ret) {
        printf("%s record init fail.\n", __func__);
        goto step_3;
    }

    int prev, total = 0, cnt = 0;
    float cos = 0.0;
    while (1) {
        //putchar('K');
        if (cbuf_get_data_size(&cbuf) < FRAME_SAMPLES) {
            os_time_dly(1);
            continue;
        }
        ret = cbuf_read(&cbuf, (void*)data, FRAME_SAMPLES);
        //printf("ret from cubf read is [%d]\n",ret);
        if (ret < FRAME_SAMPLES) {
            os_time_dly(1);
            continue;
        }
        //prev = jiffies_msec();
        //printf("going to asr process\n",ret);
//#ifdef WIFI_PCM_STREAN_SOCKET_ENABLE
//        static short wifi_stream_buf[UNI_BUFF_SIZE] ALIGNED(4);
//        int vl_idx = 0;
//        if(wifi_pcm_stream_socket_valid()){
//            short *pl_pcm_out = (short *)wifi_stream_buf;
//            for (volatile int i = 0, vl_idx = 0; i < 256; i++) {
//                *pl_pcm_out++ = data[vl_idx];//mic1
//                *pl_pcm_out++ = data[vl_idx + 1];//ref
////                *pl_pcm_out++ = aec_out_buf[i];
//                vl_idx += 2;
//            }
//            wifi_pcm_stream_socket_send((u8*)wifi_stream_buf, 256 * 2 * 2);
//        }
//#endif

        ret = uni_asr_process((void*)data, ret);
        if (ret < 0) {
            printf("%s asr process auth timeout.\n", __func__);
            goto step_3;
        }

//#ifdef WIFI_PCM_STREAN_SOCKET_ENABLE
//      static short wifi_stream_buf[512] ALIGNED();
//        int vl_idx = 0;
//        if(wifi_pcm_stream_socket_valid()){
//            short *pl_pcm_out = (char*)wifi_stream_buf;
//            for (volatile int i = 0, vl_idx = 0; i < 256; i++) {
//                *pl_pcm_out++ = data[vl_idx];//mic1
//                *pl_pcm_out++ = data[vl_idx + 1];//ref
////                *pl_pcm_out++ = aec_out_buf[i];
//                vl_idx += 2;
//            }
//            wifi_pcm_stream_socket_send((u8*)wifi_stream_buf, 256 * 2 * 2);
//        }
//#endif

        // total += jiffies_msec() - prev;
        // cnt++;
        // if(cnt == 60){
        //  cos = (float)total/(16*60);
        //  printf("kws rtf: %f\n", cos);
        //  cnt = 0;
        //  total = 0;
        // }
        //putchar('W');
    }

step_3:
    uni_asr_deinit();
step_2:
    if (data) {
        free(data);
    }
step_1:
    g_uni_local_asr_init_flag = 0;
    printf("%s exit.\n", __func__);
}

int uni_local_asr_init(void)
{
    int ret;

    if (g_uni_local_asr_init_flag) {
        return 0;
    }

    cbuf_data = malloc(CBUF_SIZE);
    if (cbuf_data == NULL) {
        printf("%s malloc fail.\n", __func__);
        return -1;
    }

    cbuf_init(&cbuf, cbuf_data, CBUF_SIZE);

    ret = thread_fork("uni_kws", 15, 4096, 0, NULL, local_asr_task, NULL);
    if (ret < 0) {
        printf("%s fork fail.\n", __func__);
        if (cbuf_data) {
            free(cbuf_data);
        }
        return -1;
    }
    g_uni_local_asr_init_flag = 1;
    return 0;
}

void user_set_kws_event_cb(user_kws_event_cb ucb)
{
    g_user_kws_cb = ucb;
}
#endif
