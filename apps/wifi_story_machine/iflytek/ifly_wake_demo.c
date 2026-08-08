#include "server/audio_server.h"
#include "ivAEC.h"
#include "ivAECParam.h"
#include "ifly_AlgEngine.h"
#include "w_ivw.h"
#include "MiddleErrorCode.h"
#include "w_ivw.h"
#include "app_config.h"
#include "app_core.h"
#include <time.h>
#include "asm/ladc.h"
#include "system/timer.h"
#include "ifly_aiui.h"
#include "ifly_aiui_api.h"
#include "ifly_aiui_net.h"
#include "ifly_aiui_net.h"
#include "event/key_event.h"
#include "ai_uart_ctrol.h"

#if (defined CONFIG_ASR_ALGORITHM) && (CONFIG_ASR_ALGORITHM == IFLYTEK_ALGORITHM)

#ifndef CONFIG_AISP_AEC_USE_LINEIN
#define CONFIG_AISP_AEC_USE_LINEIN 0
#endif

char aisp_mic_channel_set = 0;
static char aisp_mic_suspend = 0;
//#define ASR_WORDS_CHANGE //唤醒词更改：唤醒词、配网模式

#define IFLY_AUDIO_RECORD_MIC_COUNT  AUDIO_RECORD_MIC_COUNT //单麦或双麦

#define CAE_USE_SRAM_MORE_FTR

#if defined CONFIG_SFC_ENABLE || defined MUSIC_MEM_BUF_SMALL_ENABLE

#define IFLY_AUDIO_RECORD_ONCE_SR_POINTS 128
#define IFLY_AUDIO_RECORD_BUF_POINTS (32 * IFLY_AUDIO_RECORD_ONCE_SR_POINTS)
#if CONFIG_AISP_AEC_USE_LINEIN
#define IFLY_LINEIN_RECORD_BUF_POINTS (48 * IFLY_AUDIO_RECORD_ONCE_SR_POINTS)
#endif
#define IFLY_AUIDIO_RECORD_MIC_CH_NUM_MAX 2

#define  AEC_FRAME_SIZE    (128)
#define  IVW_FRAME_SIZE    (320)

#define IVW_FRAME_CNT       (20)

#define CAE_MLP_BIN_SIZE_MAX     (127 * 1024) //唤醒词mlp文件大小
#define CAE_KEYWORD_BIN_SIZE_MAX (4 * 1024)   //唤醒词文件大小
#define CAE_AEC_BUF_SIZE_MAX     (40 * 1024)

#undef WIFI_PCM_STREAN_SOCKET_ENABLE

#else
#define IFLY_AUDIO_RECORD_ONCE_SR_POINTS 128
#define IFLY_AUDIO_RECORD_BUF_POINTS (256 * IFLY_AUDIO_RECORD_ONCE_SR_POINTS)
#if CONFIG_AISP_AEC_USE_LINEIN
#define IFLY_LINEIN_RECORD_BUF_POINTS (48 * IFLY_AUDIO_RECORD_ONCE_SR_POINTS)
#endif
#define IFLY_AUIDIO_RECORD_MIC_CH_NUM_MAX 2

#define  AEC_FRAME_SIZE    (128)
#define  IVW_FRAME_SIZE    (320)
#define IVW_FRAME_CNT       (25)

#define CAE_MLP_BIN_SIZE_MAX     (205 * 1024) //唤醒词mlp文件大小
#define CAE_KEYWORD_BIN_SIZE_MAX (5 * 1024)   //唤醒词文件大小
#define CAE_AEC_BUF_SIZE_MAX     (40 * 1024)
#endif

//#define AEC_GAIN(v)     (int)(CONFIG_AISP_AEC_ADC_GAIN)
//#define MIC0_GAIN(v)    (int)(CONFIG_AISP_MIC_ADC_GAIN)   //固定CONFIG_AISP_MIC_ADC_GAIN-80
//#define MIC1_GAIN(v)    (MIC0_GAIN(v))

#define MIC_MAX_GAIN 95

int aisp_aec_gain_set(unsigned char volume);
SEC_USED(.sram) ALIGNE(4) static cbuffer_t gs_mic_cbuf;

#if (defined CONFIG_SFC_ENABLE && __SDRAM_SIZE__ == 2 * 1024 * 1024)
#define SET_MIC_BUF_SEC SEC_USED(.sram)
#else
#define SET_MIC_BUF_SEC
#endif

SET_MIC_BUF_SEC s16 mic_buf[IFLY_AUDIO_RECORD_BUF_POINTS * (IFLY_AUIDIO_RECORD_MIC_CH_NUM_MAX + 1)];
OS_SEM read_available_sem;

static St_tagAlgParm g_AlgEngineObj;
static ivPointer g_maeHandle = NULL;
static WIVW_INST g_ivwInstance;
static int g_ivwThreadID;

static const char *g_mlpRes;
static const char *g_keywordRes;
static char *g_mlpResCtx = NULL;
static char *g_fillerResCtx = NULL;

static char aisp_open_init = 0;
static char asr_open_init = 0;
extern bool s_start_send;

extern int aisp_aec_gain;
extern int aisp_mic0_gain;
extern int aisp_mic1_gain;

static uint8_t run_flag = 0;
static struct server *server_handle = NULL;
#if CONFIG_AISP_AEC_USE_LINEIN
static struct server *linein_server_handle = NULL;
#endif
static int ifly_sample_rate;
static TAECParam g_tAECParam;
static volatile char ifly_send_pcm;

int bt_music_play_get_pause(void);

extern int keyword_start;
extern int keyword_wifi_enter_congfig;

static int start_t;
static volatile char sys_vad_active = 0;//有效检测语音

extern const int speech_energy_min;
extern char speech_digital_vol_agc;

void led_eya_wake(char start);
void led_eya_sleep(void);
int sys_vad_clear(int enable);
void sys_vad_lock(int lock);

int aisp_all_mic_gain_set(int volume, int aec_gian, int mic0_gian, int mic1_gian);
int aisp_mic_gain_set(unsigned char volume);

SEC_USED(.sram) ALIGNE(4) static char ga_cae_mlp_res_buf[CAE_MLP_BIN_SIZE_MAX] = {0};
SEC_USED(.sram) ALIGNE(4) static char ga_cae_filter_res_buf[CAE_KEYWORD_BIN_SIZE_MAX] = {0};
SEC_USED(.sram) ALIGNE(4) static char ga_cae_aec_buf[CAE_AEC_BUF_SIZE_MAX] = {0};

SEC_USED(.sram) ALIGNE(4) static short ifly_audio_record_buf_read[IFLY_AUDIO_RECORD_ONCE_SR_POINTS * 3];
SEC_USED(.sram) ALIGNE(4) static short ifly_asr_buf_feed_to_engine[IFLY_AUDIO_RECORD_ONCE_SR_POINTS * 3];

#if CONFIG_AISP_AEC_USE_LINEIN
SEC_USED(.sram) ALIGNE(4) static short ifly_linein_record_buf_read[IFLY_AUDIO_RECORD_ONCE_SR_POINTS];
static cbuffer_t gs_linein_cbuf;
static s16 linein_buf[IFLY_LINEIN_RECORD_BUF_POINTS];
#endif

SEC_USED(.sram) ALIGNE(4) static short g_aecOutput[AEC_FRAME_SIZE] = {0};
SEC_USED(.sram) ALIGNE(4) static s16 s_aduo_buf[IVW_FRAME_SIZE * IVW_FRAME_CNT] = {0};
SEC_USED(.sram) ALIGNE(4) static cbuffer_t s_audio_cbuf = {0};

SEC_USED(.sram) ALIGNE(4) s16 s_aiui_buf[IFLY_AIUI_VOICE_ONCE_SEND_BYTES * 10] = {0};

cbuffer_t s_aiui_cbuf = {0};
#if (ASR_USED_AVD == ASR_IFLY_AVD)
SEC_USED(.sram) ALIGNE(4) s16 s_vad_buf[IFLY_AIUI_VOICE_ONCE_SEND_BYTES * 10] = {0};
cbuffer_t s_vad_cbuf = {0};
#endif // USER_VAD_ENABLE

static u8 last_key_supspend = 0;
OS_SEM aiui_sem;
// static uint8_t esr_timeout = 1; // 默认超时状态

#ifdef WIFI_PCM_STREAN_SOCKET_ENABLE
static short wifi_stream_buf[IFLY_AUDIO_RECORD_ONCE_SR_POINTS * 4];//mic2 + mic2 + mic3 + aec
static short aec_out_buf[IFLY_AUDIO_RECORD_ONCE_SR_POINTS];
#else
static short *aec_out_buf = NULL;
#endif

enum send_pcm_type {
    SEND_PCM_INI = 0,
    SEND_PCM_START,
    SEND_PCM_DOING,
    SEND_PCM_STOP,
    SEND_PCM_PAUSE,
};

extern void config_network_start(void);
extern int wifi_sta_is_connected(void);
extern int wifi_pcm_stream_socket_valid(void);
extern void wifi_pcm_stream_socket_send(u8 *buf, u32 len);

extern int websockets_send_audio_buf_start(void);
extern int websockets_send_audio_buf_end(void);
extern int websockets_send_audio_buf_push(char *buf, int len);
extern int websocket_client_thread_create(void *priv);
extern int websockets_client_next_dialogue_init(void);
extern int websockets_dialogue_timeout_init(int time_out, char use_voice_note);
extern int websockets_close_request(char force_close);
extern void websockets_dialogue_timeout_del(void);
extern int websockets_wait_recv_end_msg(void);

extern int wifi_sta_is_connected(void);
extern void sdfile_save_test(char *buf, int len, char close);
extern void music_buf_play_set_stop(void);
extern void websockets_free_lbuf_buf(void);
extern void music_buf_play_free_lbuf(void);
extern int music_play_res_file(const char *name);
extern int music_play_hello(char index);
extern int music_buf_play_stop(void);
extern int music_buf_play_stop_waite(void);
extern int music_play_waite(void);
extern int net_music_num_clear(void);
extern void net_music_play_set_stop(void);
extern void music_buf_play_stop_all(void);
extern int auto_sleep_check_clear(void);

void aisp_resume(void);

int audio_app_all_play_stop_status(char mode);

void  __attribute__((weak)) aisp_timeout_callback(void)
{

}

int  __attribute__((weak)) aisp_wake_callback(int status)
{
    return 0;
}

int  __attribute__((weak)) aisp_wake_no_play_notice(void)
{
    return 0;
}

//定制唤醒词更改，返回值：1唤醒词， 2配网模式
int  __attribute__((weak)) aisp_wake_world_callback(int index)
{
    return 0;
}

void  __attribute__((weak)) aisp_audio_pcm_callback(char *buf, int size)
{
}

//返回0开唤醒词，1关闭唤醒词
int  __attribute__((weak)) aisp_asr_disable(void)
{
    return 0;
}

//start：1 VAD开始，0 VAD结束
void  __attribute__((weak)) aisp_vad_callback(int start)
{

}

int __attribute__((weak)) websockets_send_500ms_audio_buf_push(char *buf, int len, int clear)
{
    return 0;
}

int __attribute__((weak)) websockets_send_audio_buf_init(void)
{
    return 0;
}

int __attribute__((weak))  websockets_nobind_check(void)
{
    return 0;
}

//start 1开始录音，0结束录音
int  __attribute__((weak)) aisp_record_callbak(int start)
{
    return 0;
}

static void ifly_audio_record_enc_server_event_handler(void *priv, int argc, int *argv)
{
    //printf("ifly_audio_record_enc_server_event_handler, argv[0]=0x%x\r\n", argv[0]);
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
    case AUDIO_SERVER_EVENT_END:
        break;
    case AUDIO_SERVER_EVENT_SPEAK_START:
        break;
    case AUDIO_SERVER_EVENT_SPEAK_STOP:
        break;
    default:
        break;
    }
}

static int ifly_audio_record_vfs_fwrite(void *file, void *data, u32 vp_len)
{
    cbuffer_t *pl_cbuf = (cbuffer_t *)file;
    u32 vl_writed_len = cbuf_write(pl_cbuf, data, vp_len);
    //printf("jsyan vp_len:%d\r\n",vp_len);
    if (vl_writed_len != vp_len) {
        printf("ifly_audio_record_vfs_fwrite, busy, wlen=%d, len=%d\r\n", vl_writed_len, vp_len);
        cbuf_clear(&gs_mic_cbuf);
#if CONFIG_AISP_AEC_USE_LINEIN
        cbuf_clear(&gs_linein_cbuf);
#endif
        return vp_len;
    }
//    os_sem_set(&read_available_sem, 0);
    os_sem_post(&read_available_sem);
    return vp_len;
}

static int ifly_audio_record_vfs_fclose(void *file)
{
    return 0;
}

static const struct audio_vfs_ops ifly_audio_record_vfs_ops = {
    .fwrite = ifly_audio_record_vfs_fwrite,
    .fclose = ifly_audio_record_vfs_fclose,
};

int checkResFile(const char* file_path)
{
    int ret = MIDDLE_OK;
    FILE* fp = fopen(file_path, "rb");
    if (!fp) {
        ret = OPEN_RES_FILE_ERROR;
    }
    fclose(fp);

    return ret;
}

int readRes(const char* file_name, char** p, char* pp_res_sram_buf)
{
    int ret = MIDDLE_OK;
    FILE* fp = fopen(file_name, "rb");
    do {
        if (fp == NULL) {
            printf("open res file:%s failed\n", file_name);
            ret = -1;
            break;
        }
        fseek(fp, 0, SEEK_END);
        int fileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        char *fileCtx;
        if (pp_res_sram_buf == NULL) {
            fileCtx = (char*)malloc(fileSize);
        } else {
            fileCtx = pp_res_sram_buf;
        }
        memset(fileCtx, 0, fileSize);
        int readNum = fread(fileCtx, 1, fileSize, fp);
        if (readNum != fileSize) {
            printf("read file context error.file name:%s file size:%d, read size:%d\n", file_name, fileSize, readNum);
            ret = -1;
            break;
        }
        *p = fileCtx;
    } while (0);
    fclose(fp);
    fp = NULL;

    return ret;
}

int initIvw80(const char* mlp_file, const char* filler_file)
{
    int ret = MIDDLE_OK;
    do {
        ret = readRes(mlp_file, &g_mlpResCtx, ga_cae_mlp_res_buf);
        if (ret) {
            printf("read res file:%s error!\n", mlp_file);
            ret = IVW_READ_MLP_ERROR;
            break;
        }

        ret = readRes(filler_file, &g_fillerResCtx, ga_cae_filter_res_buf);
        if (ret) {
            printf("read res file:%s error!\n", filler_file);
            ret = IVW_READ_FILLER_ERROR;
            break;
        }

        int instSize = wIvwGetInstSize();
        g_ivwInstance = (WIVW_INST)malloc(instSize);
        if (!g_ivwInstance) {
            printf("malloc ivw instance error.\n");
            ret = IVW_MALLOC_INST_ERROR;
            break;
        }
        memset(g_ivwInstance, 0x00, instSize);
        ret = wIvwCreate(g_ivwInstance, (const char*)g_mlpResCtx, (const char*)g_fillerResCtx);
        if (ret) {
            printf("wIvwCreate failed. ret=%d\n", ret);
            ret = IVW_CREATE_INST_ERROR;
            break;
        }

    } while (0);

    return ret;
}

void record_server_init()
{
    if (!server_handle) {
        server_handle = server_open("audio_server", "enc");
        if (!server_handle) {
            return;
        }
        server_register_event_handler(server_handle, NULL, ifly_audio_record_enc_server_event_handler);
    }
#if CONFIG_AISP_AEC_USE_LINEIN
    if (!linein_server_handle) {
        linein_server_handle = server_open("audio_server", "enc");
        if (!linein_server_handle) {
            printf("linein server open failed\n");
            return;
        }
        server_register_event_handler(linein_server_handle, NULL,
                                      ifly_audio_record_enc_server_event_handler);
    }
#endif

    aisp_resume();
}

void record_config()
{
    printf("jsyan record_config.\r\n");
    cbuf_init(&gs_mic_cbuf, mic_buf, sizeof(mic_buf)); //原始音频麦克风buf
#if CONFIG_AISP_AEC_USE_LINEIN
    cbuf_init(&gs_linein_cbuf, linein_buf, sizeof(linein_buf)); //linein通道buf
#endif
    cbuf_init(&s_audio_cbuf, s_aduo_buf, sizeof(s_aduo_buf)); //前端处理后的音频buf
    os_sem_create(&read_available_sem, 0);
}

int initAec()
{
    int ret = MIDDLE_OK;
    short nWritemode = 0;
    int iWorkModel = 0;
    memset(&g_tAECParam, 0, sizeof(TAECParam));
#ifdef CAE_USE_SRAM_MORE_FTR
    g_tAECParam.nSize = CAE_AEC_BUF_SIZE_MAX;
    g_tAECParam.pBuffer = ga_cae_aec_buf;
#else
    g_tAECParam.nSize = 40 * 1024;
    g_tAECParam.pBuffer = malloc(g_tAECParam.nSize);
#endif
    do {
        if (!g_tAECParam.pBuffer) {
            ret = AEC_MALLOC_ERROR;
            break;
        }
        ret = AECCreate((ivHandle ivPtr)&g_maeHandle, &g_tAECParam);
        if (ret != ivErr_OK) {
            ret = AEC_CREATE_ERROR;
            break;
        }
        ret = AECSetParam((ivHandle)g_maeHandle, SPEEX_AEC_AES_WRITE_MODE, &nWritemode);
        if (ret != ivErr_OK) {
            printf("ace set param SPEEX_AEC_AES_WRITE_MODE failed.\n");
            ret = AEC_SET_PARAM_ERROR;
            break;
        }

        ret = AECSetParam((ivHandle)g_maeHandle, SPEEX_ECHO_CANCEALLATION_MODEL, &iWorkModel);
        if (ret != ivErr_OK) {
            printf("ace set param SPEEX_ECHO_CANCEALLATION_MODEL failed.\n");
            ret = AEC_SET_PARAM_ERROR;
            break;
        }
    } while (0);

    return ret;
}

int unInitAec()
{
    int ret = MIDDLE_OK;
    if (g_maeHandle) {
        ret = AECReset(g_maeHandle);
        g_maeHandle = NULL;
#ifndef CAE_USE_SRAM_MORE_FTR
        free(g_tAECParam.pBuffer);
#endif
        g_tAECParam.pBuffer = NULL;
    }

    return ret;
}

//双麦前端
int initAlg()
{
    int ret = 0;
    memset(&g_AlgEngineObj, 0, sizeof(g_AlgEngineObj));

    g_AlgEngineObj.g_tAECParam = 0;
    ret = NCInit(&g_AlgEngineObj, 2, AEC_FRAME_SIZE);
    if (ret) {
        printf("Ifly_NCInit failed, ret=%d\n", ret);
        return ret;
    }
    return ret;
}

int parseJson(const char* json, const char* key, char* value, int len)
{
    const char *p1, *p2, *p;
    p = strstr(json, key);
    if (NULL == p) {
        return -1;
    }
    p += strlen(key);
    while (*p) {
        if (*p != '"' && *p != ':' && *p != ' ') {
            break;
        }
        p++;
    }
    p1 = p;
    //printf("p1 = %s\n", p1);
    while (*p) {
        if (*p == '"' || *p == ',' || *p == '}') {
            break;
        }
        p++;
    }
    p2 = p;
    //printf("p2 = %s\n", p2);
    if (p2 - p1 < len) {
        len = p2 - p1;
    } else {
        len = len - 1;
    }
    strncpy(value, p1, len);
    value[len] = '\0';

    return len;
}

void parseIvwResult(const char* res, int len)
{
    int err = 0;
    int msg[4] = {0};
    char val[64] = {0};
    char net_status = (char)sys_connect_net_success();
    int ret = parseJson(res, "iresid", val, sizeof(val));
    if (0 > ret) {
        printf("parse wakeup json error.not find iresid field.\n");
        return;
    }
    int vl_resid = atoi(val);
    printf("->iresid=%d\r\n", vl_resid);

    /*
    ret = parseJson(res, "ncmThreshold", val, sizeof(val));//"ncmThreshold":642 是当前唤醒词资源的门限
    if (0 > ret) {
        printf("parse wakeup json error.not find iresid field.\n");
        return;
    }
    int vl_ncmThreshold = atoi(val);

    ret = parseJson(res, "ncm", val, sizeof(val));//"ncm":1014 是本地唤醒得分,得分超过门限即唤醒，浅定制可以做的控制这个唤醒的阈值
    if (0 > ret) {
        printf("parse wakeup json error.not find iresid field.\n");
        return;
    }
    int vl_ncm = atoi(val);
    printf("-> iresid = %d , ncmThreshold = %d, ncm = %d \n",vl_resid,vl_ncmThreshold,vl_ncm);
    */
#ifdef CONFIG_KWS_XIAOYUXIAOYU
    if (vl_resid == 1) {
        vl_resid = 0;//唤醒
    } else if (vl_resid == 2) {
        if (net_status) {//联网后不允许进入配网，防止误触发
            return;
        }
        vl_resid = 8;//配网
    } else {
        return;
    }
#endif
#ifdef CONFIG_KWS_ENGLISH
    //if (vl_resid == 2) {//U mi U mi
    if (vl_resid == 0) {//hello you you
        vl_resid = 0;//唤醒
    } else if (vl_resid == 3) {
        if (net_status) {//联网后不允许进入配网，防止误触发
            return;
        }
        vl_resid = 8;//配网
    } else {
        return;
    }
#endif
#ifdef CONFIG_LVGL_UI_ENABLE
    if (!vl_resid) {
        lv_demo_ai_dialogue_start_mode(0);
    }
#endif
    if (aisp_asr_disable()) {//关闭唤醒词则唤醒词不发消息
        return;
    }
    msg[0] = vl_resid;
    // err = os_taskq_post("esr_queue_task", 1, msg);//采用该接口发送信息时可变参数个数不能超过8个
    err = os_taskq_post_type("esr_queue_task", Q_USER, 1, msg);
}

void aisp_wake(char index)//0:小飞小飞，小语小语，8：配网模式
{
    int msg[4] = {0};
    if (index == 8) {
        keyword_start = 1;
    }
    msg[0] = index;
    os_taskq_post_type("esr_queue_task", Q_USER, 1, msg);
}

int ivwCallBackWakeup(const char* param, void* userparam)
{
    int ret = MIDDLE_OK;
    printf("wakeup param:%s\r\n", param);
    if (!param) {
        printf("ivwCallBackWakeup param is null.\n");
        return IVW_CB_PARAM_ERROR;
    }

    int len = strlen(param);
    parseIvwResult(param, len);

    return ret;
}

const char *ifly_get_mlp_res_path()
{
#ifdef CONFIG_KWS_ENGLISH
    return CONFIG_IFLY_EN_MLP_RES_NAME;
#endif
#ifdef CONFIG_KWS_XIAOYUXIAOYU
    return CONFIG_IFLY_MLP_RES_NAME;
#else
    return CONFIG_IFLY_XF_MLP_RES_NAME;
#endif
}

const char *ifly_get_keyword_res_path()
{
#ifdef CONFIG_KWS_ENGLISH
    return CONFIG_IFLY_EN_KEYWORD_RES_NAME;
#endif
#ifdef CONFIG_KWS_XIAOYUXIAOYU
    return CONFIG_IFLY_KEYWORD_RES_NAME;
#else
    return CONFIG_IFLY_XF_KEYWORD_RES_NAME;
#endif
}

void initIvw()
{
    int ret = MIDDLE_OK;
    g_mlpRes = ifly_get_mlp_res_path();
    if (checkResFile(g_mlpRes)) {
        printf("check mlp:[%s] res failed!\n", g_mlpRes);
        return;
    }

    g_keywordRes = ifly_get_keyword_res_path();
    if (checkResFile(g_keywordRes)) {
        printf("check keyword:[%s] res failed!\n", g_mlpRes);
        return;
    }

    printf("call initIvw80\n");
    ret = initIvw80(g_mlpRes, g_keywordRes);
    if (ret) {
        printf("initIvw80 failed.\n");
        return;
    }
    printf("finish create ivw instance.\r\n");

}

//前端和唤醒引擎初始化
int ifly_engine_init()
{
#if IFLY_AUDIO_RECORD_MIC_COUNT == 1
    initAec();
#else
    initAlg();
    // 输出引擎版本
    char ver[64] = {0};
    NCGetVersion(ver);
    printf("jsyan AlgEngine version:%s\n", ver);
#endif

    vtn_net_auth_init(ifly_aiui_sdk_common_get_appid(), ifly_aiui_sdk_common_get_deviceid());//参数输入用户appid
    printf("vtn_sdk_version:%s\n", vtn_net_auth_get_version());
    initIvw();
    int ret = wIvwRegisterCallBacks(g_ivwInstance, CallBackFuncNameWakeUp, ivwCallBackWakeup, NULL);
    if (ret) {
        printf("register CallBackFuncNameWakeUp failed.\n");
    }
    // 设置引擎工作模式
    int off_siranbi = 0;
    ret = wIvwSetParameter(g_ivwInstance, PARAM_W_DEC_USE_SIRANBI, off_siranbi);
    if (ret) {
        printf("wIvwSetParameter failed, engine error code:%d\r\n", ret);
    }
    ret = wIvwStart(g_ivwInstance);
    if (ret) {
        printf("start ivw instance failed.\n");
    }
}

void ifly_engine_uninit(void)
{
#if IFLY_AUDIO_RECORD_MIC_COUNT == 1
    unInitAec();
#else
    NCUnit(&g_AlgEngineObj);
#endif
    if (g_ivwInstance) {
        wIvwStop(g_ivwInstance);
        wIvwDestroy(g_ivwInstance);
        free(g_ivwInstance);
        g_ivwInstance = NULL;
    }
}

int speech_digital_vol_agc_enable(char enable)
{
    static char init_agc = 0;
    if (!init_agc && speech_digital_vol_agc) { //开启数字增益：远场识别
        init_agc = speech_digital_vol_agc;
    }
    if (enable) {
        speech_digital_vol_agc = init_agc ? init_agc : enable;
    } else {
        if (!init_agc) { //不开启数字增益：近场识别
            speech_digital_vol_agc = enable;
        }
    }
    return speech_digital_vol_agc;
}

SEC_USED(.volatile_ram_code) ALIGNE(4) void aecProcess2(const void* pdata, void *aec_out, int aec_out_size)
{
    int ret = MIDDLE_OK;
    ivInt16 nAECOut = AEC_FRAME_SIZE;

#if IFLY_AUDIO_RECORD_MIC_COUNT == 1
    ret = AECAppendAudioData((ivHandle)g_maeHandle, (ivCPointer)(pdata), ivNull, AEC_FRAME_SIZE);
    ret = AECRunStep((ivHandle)g_maeHandle, (ivPointer)g_aecOutput, &nAECOut);
    if (ret) {
        printf("AECRunStep error...\n");
        return;
    }
#else
    ret = Ifly_AlgProcess(&g_AlgEngineObj, pdata, g_aecOutput, nAECOut);
    if (ret != AEC_FRAME_SIZE) {
        printf("aecProcess2 Ifly_AlgProcess error...\n");
        return;
    }
#endif // IFLY_AUDIO_RECORD_MIC_COUNT
    unsigned short len = nAECOut * 2;

    if (speech_digital_vol_agc) { //数字放大
        int GAIN_MAX = speech_digital_vol_agc == 1 ? 8 : speech_digital_vol_agc;
//#define GAIN_MAX    8
        static double pcm_gain = 1;//数字放大倍数，初始值为3
        static int pcm_max = 0;//记录最大值，最大值不能超过32000
        static int pcm_min = 0;//记录最小值，最大值不能超过-32000
        int pcm_cont = IFLY_AUDIO_RECORD_ONCE_SR_POINTS;
        int tmp;
        short *val = (short *)g_aecOutput;
        for (volatile int j = 0; j < pcm_cont; j++) {
            tmp = (*val) * pcm_gain;
            if (tmp > pcm_max) {
                pcm_max = tmp;
            }
            if (tmp < pcm_min) {
                pcm_min = tmp;
            }
            if (pcm_max > 32000 || pcm_min < -32000) {
                pcm_gain /= 1.5;
                pcm_max /= 1.5;
                pcm_min /= 1.5;
                tmp = (*val) * pcm_gain;
            }
            *val++ = (short)tmp;
        }
        pcm_gain += 0.01;
        if (pcm_gain > GAIN_MAX) {
            pcm_gain = GAIN_MAX;
        }
    }

    if (aec_out && aec_out_size >= len) {
        memcpy(aec_out, g_aecOutput, len);
    }

    if (!cbuf_is_write_able(&s_audio_cbuf, len)) {
        // cbuf_read_updata(&s_audio_cbuf, len);
        cbuf_clear(&s_audio_cbuf);
    }
    u32 vl_write_len = cbuf_write(&s_audio_cbuf, (void*)g_aecOutput, len);
    if (vl_write_len != len) {
        printf("cbuffer is full\n");
    }
    return;
}

int pcm_send_set_status(char start)
{
    if (start) {
        if (ifly_send_pcm != SEND_PCM_START && ifly_send_pcm != SEND_PCM_DOING) {
            keyword_start = 1;
            ifly_send_pcm = SEND_PCM_START;
        }
    } else if (ifly_send_pcm == SEND_PCM_DOING || ifly_send_pcm == SEND_PCM_START) {
        ifly_send_pcm = SEND_PCM_STOP;
    }
}

int pcm_send_get_status(void)
{
    return ifly_send_pcm == SEND_PCM_START || ifly_send_pcm == SEND_PCM_DOING;
}

static int first_500ms_len = 0;
static int first_500ms_copy = 0;
void pcm_buf_clear(void)
{
    if (ifly_send_pcm != SEND_PCM_PAUSE) {
        ifly_send_pcm = SEND_PCM_INI;
        first_500ms_len = 0;
        first_500ms_copy = true;
    }
}

void aisp_wake_set_noice(char noice_note)
{
    if (sys_connect_net_success() && !keyword_wifi_enter_congfig) {
        if (ifly_send_pcm != SEND_PCM_PAUSE) {
            //通知app挂起
            struct key_event key = {0};
            key.type = KEY_EVENT_USER;
            if (last_key_supspend != KEY_SUPSPEND) {
                key.action = KEY_EVENT_LONG;
                key.value = KEY_SUPSPEND;
                if (!ai_speaker_app()) {
                    printf("-> ! ai_speaker_app\n");
                    key_event_notify(KEY_EVENT_FROM_USER, &key);
                    last_key_supspend = KEY_SUPSPEND;
                }
                os_time_dly(10);
            }
            auto_sleep_check_clear();
            //通知app挂起
            keyword_start = 0;
            sys_vad_clear(0);
            sys_vad_lock(1);
            ifly_send_pcm = SEND_PCM_STOP;

            music_buf_play_accept(0);
            websockets_close_request(1);
            websockets_dialogue_timeout_del();
            websockets_free_lbuf_buf();

            if (alarm_music_play_stop()) { //闹钟在响停止闹钟
                music_play_stop(NULL);
                music_buf_play_stop_all();
                music_buf_play_set_stop();
                http_tts_play_stop_force();
                //net_music_play_set_stop();
                music_buf_play_free_lbuf();
                music_buf_play_stop_waite();//等待关闭完成再播放提示音
            }
            net_music_num_clear();

            led_eya_wake(1);//LED眼睛唤醒显示

            char hello_index = rand() % 5;
            if (noice_note) {
                aisp_mic_gain_suspend();
                if (!music_play_hello(hello_index)) {
                    music_play_waite();
                }
                aisp_mic_gain_resum();
                music_buf_play_set_stop();//停止TTS使用PIPE BUF
            }

            websockets_send_audio_buf_init();
            websockets_send_500ms_audio_buf_push(NULL, 0, 1);

            ifly_send_pcm = SEND_PCM_START;
            keyword_start = 1;
            pcm_buf_clear();
            sys_vad_lock(0);
            sys_vad_clear(1);
#ifdef AI_UART_CMD_CTROL_ENABLE
            ai_uart_cmd_data_push(AI_UART_CMD_DIALUOGE_START, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
            at_uart_cmd_send(AI_UART_CMD_DIALUOGE_START, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
            play_face_emoji(AI_UART_CMD_DIALUOGE_START);
#endif
            websocket_client_thread_create(1);
            music_buf_play_accept(1);
        }
    }
}

static void key_vad_pcm_send_set(char start, char noice_note, int vad_end_enable)
{
    char hello_index = rand() % 5;
    if (start) {
        printf("--->KAY VAD SPEAK_START\n");
        //通知app挂起
        struct key_event key = {0};
        key.type = KEY_EVENT_USER;
        if (last_key_supspend != KEY_SUPSPEND) {
            key.action = KEY_EVENT_LONG;
            key.value = KEY_SUPSPEND;
            if (!ai_speaker_app()) {
                printf("-> ! ai_speaker_app\n");
                key_event_notify(KEY_EVENT_FROM_USER, &key);
                last_key_supspend = KEY_SUPSPEND;
                os_time_dly(10);
            }
        }
        auto_sleep_check_clear();

        music_buf_play_accept(0);
        websockets_close_request(1);
        websockets_dialogue_timeout_del();
        websockets_free_lbuf_buf();

        if (alarm_music_play_stop()) { //闹钟在响停止闹钟
            music_play_stop(NULL);
            music_buf_play_stop_all();
            music_buf_play_set_stop();
            http_tts_play_stop_force();
            //net_music_play_set_stop();
            music_buf_play_free_lbuf();
            music_buf_play_stop_waite();//等待关闭完成再播放提示音
        }
        net_music_num_clear();

        led_eya_wake(1);//LED眼睛唤醒显示

#ifdef CONFIG_LVGL_UI_ENABLE
        lv_demo_ai_dialogue_start_mode(1);
        lv_demo_ai_dialogue_start(hello_index);
#endif

#ifdef AI_UART_CMD_CTROL_ENABLE
        ai_uart_cmd_data_push(AI_UART_CMD_DIALUOGE_START, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
        at_uart_cmd_send(AI_UART_CMD_DIALUOGE_START, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
        play_face_emoji(AI_UART_CMD_DIALUOGE_START);
#endif
        if (noice_note) {
            aisp_mic_gain_suspend();
            if (!music_play_hello(hello_index)) {
                music_play_waite();
            }
            aisp_mic_gain_resum();
            music_buf_play_set_stop();//停止TTS使用PIPE BUF
        }
        //websocket_client_thread_create(1);
        websockets_send_audio_buf_init();
        websockets_send_500ms_audio_buf_push(NULL, 0, 1);
        websocket_client_thread_create_new(1, 1);
        music_buf_play_accept(1);
        if (vad_end_enable) {
            keyword_start = 1;
        } else {
            keyword_start = 2;
        }
        ifly_send_pcm = SEND_PCM_START;
    } else if (qyai_chat_addr_version() <= 1) {
        if (ifly_send_pcm == SEND_PCM_DOING || ifly_send_pcm == SEND_PCM_START) {
            ifly_send_pcm = SEND_PCM_STOP;
#ifdef CONFIG_UI_PLAY_EMOJI
            play_face_emoji(AI_UART_CMD_REC_END);
#endif
        }
        printf("--->KAY VAD SPEAK_STOP\n");
        keyword_start = 0;
        sys_vad_active = 0;
    }
}

void key_vad_pcm_send_set_status(char start, char noice_note)
{
    key_vad_pcm_send_set(start, noice_note, 0);
}

void key_vad_start_vad_end_auto(char start, char noice_note)
{
    key_vad_pcm_send_set(start, noice_note, 1);
}

void sys_vad_event_handler(void *priv)
{
    aisp_vad_callback((int)priv);
    if (keyword_start == 2) {
        sys_vad_active = 1;
        return;
    }
    if (priv) {
        sys_vad_active = 1;
#if ASR_CONTINIU_EN
        if (keyword_start) {
#else   //播放完当前对话才能继续
        if (keyword_start && (ifly_send_pcm == SEND_PCM_STOP || ifly_send_pcm == SEND_PCM_INI)
            && music_buf_play_stop_status() && music_play_stop_status() && !websockets_wait_recv_end_msg()) {
#endif // ASR_CONTINIU_EN
#ifdef CONFIG_UI_PLAY_EMOJI
            play_face_emoji(AI_UART_CMD_REC_START);
#endif
#ifdef AI_UART_CMD_CTROL_ENABLE
            ai_uart_cmd_data_push(AI_UART_CMD_REC_START, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
            at_uart_cmd_send(AI_UART_CMD_REC_START, NULL);
#endif
            music_buf_play_set_stop();
            aisp_record_callbak(1);
            websockets_send_audio_buf_init();
            websocket_client_thread_create(0);
            music_buf_play_accept(1);
            printf("--->SPEAK_START\n");
            ifly_send_pcm = SEND_PCM_START;
        } else {
            printf("->keyword_start %d %d %d %d %d\n", keyword_start,
                   ifly_send_pcm,
                   music_buf_play_stop_status(),
                   music_play_stop_status(),
                   !websockets_wait_recv_end_msg());
        }
    } else if (qyai_chat_addr_version() <= 1) {
        if (ifly_send_pcm == SEND_PCM_DOING || ifly_send_pcm == SEND_PCM_START) {
            printf("--->SPEAK_STOP = %d \n", ifly_send_pcm);
            ifly_send_pcm = SEND_PCM_STOP;
            aisp_record_callbak(0);
#ifdef CONFIG_UI_PLAY_EMOJI
            play_face_emoji(AI_UART_CMD_REC_END);
#endif
        }
        sys_vad_active = 0;
    }
}

SEC_USED(.volatile_ram_code) ALIGNE(4) void ivwProcess()
{
    int ret = MIDDLE_OK;
    int rlen = 0, vl_write_len;
    char buf[IVW_FRAME_SIZE] = {0};
#if CONFIG_AISP_AEC_USE_LINEIN
    unsigned int vl_mic_frame_len = IFLY_AUDIO_RECORD_ONCE_SR_POINTS * sizeof(short) * IFLY_AUDIO_RECORD_MIC_COUNT;
    unsigned int vl_linein_frame_len = IFLY_AUDIO_RECORD_ONCE_SR_POINTS * sizeof(short);
#else
    unsigned int vl_frame_len = IFLY_AUDIO_RECORD_ONCE_SR_POINTS * sizeof(short) * (IFLY_AUDIO_RECORD_MIC_COUNT + 1);
#endif
    volatile int vad_active = 0;
    volatile unsigned int t1 = 0;
    volatile short pd_mic2_test = 0xFFFF;
    volatile short pd_mic1_test = 0xFFFF;
    volatile short pd_aec_ref_test = 0xFFFF;
    volatile short pd_test = 0xFFFF;
    char opus = !strcmp(qyai_chat_audio_enc_format(), "opus");
    int ASR_FIRST_BEFORE_BYTES = ifly_sample_rate * 2 / 2;//500ms
    char *first_500ms_buf = NULL;
    if (opus) {
        ASR_FIRST_BEFORE_BYTES = ASR_FIRST_BEFORE_BYTES > 0 ? ASR_FIRST_BEFORE_BYTES : (16000 * 2 / 2);
        first_500ms_buf = malloc(ASR_FIRST_BEFORE_BYTES); //500ms
        first_500ms_copy = 0;
        if (first_500ms_buf) {
            first_500ms_copy = true;
        }
        first_500ms_len = 0;
    }
    printf("-->asr_task start====================\r\n");
    start_t = timer_get_ms();

    keyword_start = 0;

#if (ASR_USED_AVD == ASR_WEBRTC_AVD) || (ASR_USED_AVD == ASR_MY_AVD) || (ASR_USED_AVD == ASR_JL_AVD)
    void *vad_cbuf = sys_vad_init(ifly_sample_rate, IFLY_AUDIO_RECORD_BUF_POINTS * (IFLY_AUIDIO_RECORD_MIC_CH_NUM_MAX + 1));
    if (vad_cbuf) {
        sys_vad_create(NULL);
    } else {
        printf("->vad_cbuf init err\n");
    }
#endif
    while (1) {
        //if(thread_kill_req()){
        if (!asr_open_init) {
            printf("ivwProcess thread_kill_req\r\n");
            break;
        }
        // if(run_flag != 1){
        //  // os_sem_pend(&read_available_sem, 0);
        //     os_time_dly(1);
        //  continue;
        // }

        //get audio
#if CONFIG_AISP_AEC_USE_LINEIN
        if ((cbuf_get_data_size(&gs_mic_cbuf) < vl_mic_frame_len) || (cbuf_get_data_size(&gs_linein_cbuf) < vl_linein_frame_len)) {
#else
        if (cbuf_get_data_size(&gs_mic_cbuf) < vl_frame_len) {
#endif
            os_sem_pend(&read_available_sem, 3);
            continue;
        }
        extern int is_production_test_enter(char wake);
        extern int is_production_test_mic2_enter(void);
        extern int is_production_test_mic1_enter(void);
        extern int is_production_test_aec_enter(void);

        pd_test = is_production_test_enter(0) ? 0 : 0xFFFF;
        if (pd_test == 0) {
            pd_mic2_test = is_production_test_mic2_enter() ? 0 : 0xFFFF;
            pd_mic1_test = is_production_test_mic1_enter() ? 0 : 0xFFFF;
            //pd_aec_ref_test = is_production_test_aec_enter();
        }

        int vl_read_len;
#if CONFIG_AISP_AEC_USE_LINEIN
        vl_read_len = cbuf_read(&gs_mic_cbuf, ifly_audio_record_buf_read, vl_mic_frame_len);
        int linein_read_len = cbuf_read(&gs_linein_cbuf, ifly_linein_record_buf_read, vl_linein_frame_len);
        if ((vl_read_len != vl_mic_frame_len) || (linein_read_len != vl_linein_frame_len)) {
            printf("audio frame read error, mic=%d/%d, linein=%d/%d\r\n", vl_read_len, vl_mic_frame_len,
                   linein_read_len, vl_linein_frame_len);
            continue;
        }
#else
        vl_read_len = cbuf_read(&gs_mic_cbuf, ifly_audio_record_buf_read, vl_frame_len);
        if (vl_read_len != vl_frame_len) {
            printf("audio frame read error, len=%d/%d\r\n", vl_read_len, vl_frame_len);
            continue;
        }
#endif

#if MIC2_AEC != 0 //清楚如何改之后请删除下面4行代码
#error "回采不是MIC0通道，需要改下面的重组数据顺序，否则无法唤醒和打断，注意-MIC数据输出格式：低通道在前，高通道在后。算法数据存储格式：MIC1 MIC2 AEC"
#error "默认单麦使用MIC1通道，双麦使用MIC1通道和MIC2通道，回采统一使用MIC0通道。清楚如何改之后请删除下面4行代码"
#endif
        //重组数据
        short *pl_pcm_out = (short *)ifly_asr_buf_feed_to_engine;
#if CONFIG_AISP_AEC_USE_LINEIN
        if (pd_test == 0) { //厂测模式
            for (volatile int i = 0; i < IFLY_AUDIO_RECORD_ONCE_SR_POINTS; i++) {
#if IFLY_AUDIO_RECORD_MIC_COUNT == 1
                *pl_pcm_out++ = pd_mic2_test & ifly_audio_record_buf_read[i];
                *pl_pcm_out++ = pd_test & ifly_linein_record_buf_read[i];
#else
                //2 mics
                *pl_pcm_out++ = pd_mic2_test & ifly_audio_record_buf_read[i * 2];//mic1
                *pl_pcm_out++ = pd_mic1_test & ifly_audio_record_buf_read[i * 2 + 1];//mic2
                *pl_pcm_out++ = pd_test & ifly_linein_record_buf_read[i];//ref
#endif // IFLY_AUDIO_RECORD_MIC_COUNT
            }
        } else { //正常模式
            for (volatile int i = 0; i < IFLY_AUDIO_RECORD_ONCE_SR_POINTS; i++) {
#if IFLY_AUDIO_RECORD_MIC_COUNT == 1
                *pl_pcm_out++ = ifly_audio_record_buf_read[i];
                *pl_pcm_out++ = ifly_linein_record_buf_read[i];//ref
#else
                //2 mics
                *pl_pcm_out++ = ifly_audio_record_buf_read[i * 2];//mic1
                *pl_pcm_out++ = ifly_audio_record_buf_read[i * 2 + 1];//mic2
                *pl_pcm_out++ = ifly_linein_record_buf_read[i];//ref
#endif // IFLY_AUDIO_RECORD_MIC_COUNT
            }
        }
#else
        int vl_idx = 0;
        if (pd_test == 0) { //厂测模式
            for (volatile int i = 0; i < IFLY_AUDIO_RECORD_ONCE_SR_POINTS; i++) {
#if IFLY_AUDIO_RECORD_MIC_COUNT == 1
                *pl_pcm_out++ = ifly_audio_record_buf_read[vl_idx + 1];
                *pl_pcm_out++ = pd_test & ifly_audio_record_buf_read[vl_idx];
                vl_idx += 2;
#else
                *pl_pcm_out++ = pd_mic2_test & ifly_audio_record_buf_read[vl_idx + 1];
                *pl_pcm_out++ = pd_mic1_test & ifly_audio_record_buf_read[vl_idx + 2];
                *pl_pcm_out++ = pd_test & ifly_audio_record_buf_read[vl_idx];
                vl_idx += 3;
#endif
            }
        } else { //正常模式
            for (volatile int i = 0; i < IFLY_AUDIO_RECORD_ONCE_SR_POINTS; i++) {
#if IFLY_AUDIO_RECORD_MIC_COUNT == 1
                *pl_pcm_out++ = ifly_audio_record_buf_read[vl_idx + 1];
                *pl_pcm_out++ = ifly_audio_record_buf_read[vl_idx];
                vl_idx += 2;
#else
                *pl_pcm_out++ = ifly_audio_record_buf_read[vl_idx + 1];
                *pl_pcm_out++ = ifly_audio_record_buf_read[vl_idx + 2];
                *pl_pcm_out++ = ifly_audio_record_buf_read[vl_idx];
                vl_idx += 3;
#endif
            }
        }
#endif

#ifdef WIFI_PCM_STREAN_SOCKET_ENABLE
        if (!wifi_pcm_stream_socket_valid()) {
#endif
#ifdef CONFIG_IFLY_AEC_ASR_FOLLOW_VAD
            //=========================检测语音的幅值小于SPEECH_ENERGY_MIN(48)时判断为环境静音，不做AEC和语音识别处理，降低功耗15mA-20mA左右
            if (!sys_vad_active && (ifly_send_pcm == SEND_PCM_STOP || ifly_send_pcm == SEND_PCM_INI)) { //无语音活动
                if (!t1) {
                    t1 = timer_get_ms();
                }
                volatile char max_vad_min = 0;
                for (volatile int j = 0, idx = 0, vol_min = 0; j < IFLY_AUDIO_RECORD_ONCE_SR_POINTS; j++) {
                    if (ifly_asr_buf_feed_to_engine[idx] >= speech_energy_min) { //mic1
                        max_vad_min = 1;//有效检测语音幅值
                        break;
                    }
#if IFLY_AUDIO_RECORD_MIC_COUNT == 2
                    if (ifly_asr_buf_feed_to_engine[++idx] >= speech_energy_min) { //mic2
                        max_vad_min = 1;//有效检测语音幅值
                        break;
                    }
#endif
                    idx += 2;
                }
                if (!max_vad_min) {
                    if (timer_get_ms() - t1 > 2000) { //2秒内没有语音活动
                        vad_active = 0;
                        t1 = 0;
                    }
                } else {
                    vad_active = 1;
                    t1 = 0;
                }
            } else {
                vad_active = 1;
                t1 = 0;
            }
            if (!vad_active) {
                continue;
            }
#endif
#ifdef WIFI_PCM_STREAN_SOCKET_ENABLE
        }
#endif

#ifdef WIFI_PCM_STREAN_SOCKET_ENABLE
        //alg
        aecProcess2(ifly_asr_buf_feed_to_engine, aec_out_buf, sizeof(aec_out_buf));
#else
        aecProcess2(ifly_asr_buf_feed_to_engine, NULL, 0);
#endif

#ifdef WIFI_PCM_STREAN_SOCKET_ENABLE
        if (wifi_pcm_stream_socket_valid()) {
            pl_pcm_out = (char*)wifi_stream_buf;
            for (volatile int i = 0, vl_idx = 0; i < IFLY_AUDIO_RECORD_ONCE_SR_POINTS; i++) {
#if IFLY_AUDIO_RECORD_MIC_COUNT == 1
                *pl_pcm_out++ = ifly_asr_buf_feed_to_engine[vl_idx];//mic1
                *pl_pcm_out++ = ifly_asr_buf_feed_to_engine[vl_idx + 1];//ref
                *pl_pcm_out++ = aec_out_buf[i];
                vl_idx += 2;
#else
                *pl_pcm_out++ = ifly_asr_buf_feed_to_engine[vl_idx];//mic1
                *pl_pcm_out++ = ifly_asr_buf_feed_to_engine[vl_idx + 1];//mic2
                *pl_pcm_out++ = ifly_asr_buf_feed_to_engine[vl_idx + 2];//ref
                *pl_pcm_out++ = aec_out_buf[i];
                vl_idx += 3;
#endif
            }
            wifi_pcm_stream_socket_send(wifi_stream_buf, IFLY_AUDIO_RECORD_MIC_COUNT == 1 ? \
                                        (IFLY_AUDIO_RECORD_ONCE_SR_POINTS * 3 * 2) : \
                                        (IFLY_AUDIO_RECORD_ONCE_SR_POINTS * 4 * 2));
        }
#endif

        //ivw
        if (cbuf_get_data_size(&s_audio_cbuf) >= IVW_FRAME_SIZE) {
            rlen = cbuf_read(&s_audio_cbuf, buf, IVW_FRAME_SIZE);
            if (rlen == IVW_FRAME_SIZE) {
                if (!aisp_asr_disable() && keyword_start != 2) {
                    if (g_ivwInstance) {
                        ret = wIvwWrite(g_ivwInstance, buf, IVW_FRAME_SIZE);
                        if (ret) {
                            printf("wIvwWrite error, engine ret=%d\n", ret);
                            //                    continue;
                        }
                    }
                }
                aisp_audio_pcm_callback(buf, IVW_FRAME_SIZE);
#if (ASR_USED_AVD == ASR_WEBRTC_AVD) || (ASR_USED_AVD == ASR_MY_AVD) || (ASR_USED_AVD == ASR_JL_AVD)
                if (vad_cbuf) {
                    sys_vad_send_msg(0x1, vad_cbuf, buf, IVW_FRAME_SIZE);
                }
#elif (ASR_USED_AVD == ASR_IFLY_AVD)

                if (!cbuf_is_write_able(&s_vad_cbuf, IVW_FRAME_SIZE)) {
                    cbuf_read_updata(&s_vad_cbuf, IVW_FRAME_SIZE);
                    //cbuf_clear(&s_vad_cbuf);
                }
                vl_write_len = cbuf_write(&s_vad_cbuf, (void*)buf, IVW_FRAME_SIZE);
                if (vl_write_len != IVW_FRAME_SIZE) {
                    printf("vad buf, busy, wlen=%d\r\n", vl_write_len);
                }
#endif

                if (first_500ms_buf && first_500ms_copy) {
                    if ((first_500ms_len + rlen) <= ASR_FIRST_BEFORE_BYTES) { //还没满buf则继续填满
                        memcpy((char*)first_500ms_buf + first_500ms_len, (char*)buf, rlen);
                        first_500ms_len += rlen;
                    } else if (rlen < ASR_FIRST_BEFORE_BYTES) { //满buf则继续更新最新数据在buf最后区域
                        if (first_500ms_len > rlen) {
                            memcpy((char*)first_500ms_buf, (char*)first_500ms_buf + rlen, first_500ms_len - rlen);
                            memcpy((char*)first_500ms_buf + first_500ms_len - rlen, (char*)buf, rlen);
                        } else {
                            memcpy((char*)first_500ms_buf, (char*)buf, rlen);
                            first_500ms_len = rlen;
                        }
                    }
                }

                //if(keyword_start)
                {
                    if (ifly_send_pcm == SEND_PCM_START) {
                        if (!sys_connect_net_success()) {
                            printf("err net connect\n\n");
                            goto reinit;
                        }
                        //music_buf_play_set_stop();
                        //websockets_free_lbuf_buf();
                        music_buf_play_free_lbuf();
                        if (qyai_audio_send_start()) {
                            if (qyai_audio_send_start()) {
                                printf("err pcm_buf_start\n\n");
                                continue;
                            }
                        }
                        if (first_500ms_copy) {
                            if (!qyai_audio_send_push((u8 *)first_500ms_buf, first_500ms_len)) {
//                                sdfile_onefile_save_test(first_500ms_buf, first_500ms_len, 0);//sd卡存储PCM
                                first_500ms_copy = 0;
                                ifly_send_pcm = SEND_PCM_DOING;
                            }
                        } else {
                            if (!qyai_audio_send_push((u8 *)buf, rlen)) {
                                ifly_send_pcm = SEND_PCM_DOING;
                            }
                        }
                    } else if (ifly_send_pcm == SEND_PCM_DOING) {
                        if (qyai_audio_send_push((u8 *)buf, rlen) == -2) { //发送过程中缓存满且掉线
                            goto reinit;
                        }
//                        sdfile_onefile_save_test(buf, rlen, 0);//sd卡存储PCM
                    } else if (ifly_send_pcm == SEND_PCM_STOP) {
                        qyai_audio_send_push((u8 *)buf, rlen);
//                        sdfile_onefile_save_test(buf, rlen, 1);//sd卡存储PCM
                        qyai_audio_send_end();
reinit:
                        ifly_send_pcm = SEND_PCM_INI;
                        first_500ms_len = 0;
                        first_500ms_copy = true;

                    } else if (!opus) {
                        websockets_send_500ms_audio_buf_push(buf, rlen, 0);//
                    }
                }
            }
        }
    }
#if (ASR_USED_AVD == ASR_WEBRTC_AVD) || (ASR_USED_AVD == ASR_MY_AVD) || (ASR_USED_AVD == ASR_JL_AVD)
    if (vad_cbuf) {
        sys_vad_uninit(vad_cbuf);
        vad_cbuf = NULL;
    }
#endif
    if (first_500ms_buf) {
        free(first_500ms_buf);
    }
    return;
}

int aisp_dialuoge_request(char *file_path)
{
    music_buf_play_accept(0);
    http_tts_play_stop_force();
    music_buf_play_set_stop();
    websockets_free_lbuf_buf();
    music_buf_play_free_lbuf();
    websockets_close_request(1);
    music_buf_play_stop_waite();//等待关闭完成再播放提示音
    net_music_num_clear();

#define FLEN_SIZE 4096
    char *buf = malloc(FLEN_SIZE);
    int remain = 0;
    int ssize = 0;
    int ret = 0;
    char cnt = 3;
    if (buf) {
        FILE *fd = fopen(file_path, "r");
        if (fd) {
            websockets_send_audio_buf_init();
            websockets_send_500ms_audio_buf_push(NULL, 0, 1);
            websocket_client_thread_create(1);
            os_time_dly(1);
            websockets_send_audio_buf_start();
            remain = flen(fd);
            while (remain > 0) {
                ssize = MIN(FLEN_SIZE, remain);
                ret = fread(buf, ssize, 1, fd);
                if (ret <= 0) {
                    break;
                }
                if (websockets_send_audio_buf_push((u8 *)buf, ret)) {
                    os_time_dly(1);
                    if (--cnt == 0) {
                        break;
                    }
                }
                cnt = 3;
                remain -= ret;
            }
            websockets_send_audio_buf_end();
            music_buf_play_accept(1);
            fclose(fd);
        }
        free(buf);
    }
    return 0;
}

void aisp_all_pause(char stop)
{
    int to = 100;
    if (stop) {
        ifly_send_pcm = SEND_PCM_PAUSE;
        sys_vad_clear(0);
    } else {
        ifly_send_pcm = SEND_PCM_INI;
        sys_vad_clear(1);
    }
    keyword_start = 0;
    sys_vad_active = 0;

    union audio_req req = {0};
    if (server_handle) {
        req.enc.cmd = AUDIO_ENC_GET_STATUS;
        server_request(server_handle, AUDIO_REQ_ENC, &req);
        int status = req.enc.status;
        memset(&req, 0, sizeof(req));
        if (status == AUDIO_ENC_START && stop) {
            req.enc.cmd = AUDIO_ENC_PAUSE;
            server_request(server_handle, AUDIO_REQ_ENC, &req);
#if CONFIG_AISP_AEC_USE_LINEIN
            if (linein_server_handle) {
                server_request(linein_server_handle, AUDIO_REQ_ENC, &req);
            }
#endif
            printf("-> AUDIO_ENC_PAUSE \n");
        } else if (status == AUDIO_ENC_PAUSE && stop == 0) {
            req.enc.cmd = AUDIO_ENC_START;
            server_request(server_handle, AUDIO_REQ_ENC, &req);
#if CONFIG_AISP_AEC_USE_LINEIN
            if (linein_server_handle) {
                server_request(linein_server_handle, AUDIO_REQ_ENC, &req);
            }
#endif
            printf("-> AUDIO_ENC_START \n");
        }
    }
    music_buf_play_set_stop();
    http_tts_play_stop_force();
    websockets_free_lbuf_buf();
    music_buf_play_free_lbuf();
    websockets_close_request(1);
    music_buf_play_stop_waite();//等待关闭完成再播放提示音
    net_music_num_clear();
    while (!music_buf_play_stop_status() && --to) {
        os_time_dly(1);
    }
}

//本地唤醒识别线程
void create_awake_task()
{
    int ret = MIDDLE_OK;
    ret = thread_fork("ivw_thread", 3, 3840, 0, &g_ivwThreadID, ivwProcess, NULL);
    if (OS_NO_ERR != ret) {
        printf("create ivw thread failed.\n");
    }
}

void aisp_clear(char stop_net_music)
{
    keyword_start = 0;
    sys_vad_active = 0;
    sys_vad_clear(0);
    sys_vad_lock(1);
    music_buf_play_accept(0);
    ifly_send_pcm = SEND_PCM_STOP;
    music_buf_play_set_stop();
    http_tts_play_stop_force();
    if (stop_net_music) {
        net_music_play_set_stop();
    }
    sys_vad_clear(0);
    websockets_free_lbuf_buf();
    music_buf_play_free_lbuf();
    websockets_close_request(1);
    websockets_dialogue_timeout_del();
    music_buf_play_stop_waite();//等待关闭完成再播放提示音
    net_music_num_clear();
    music_buf_play_accept(1);
    sys_vad_lock(0);
    sys_vad_clear(1);
}

void aisp_timeout_exit(void)
{
    aisp_wake(20);//恢复APP
    aisp_timeout_callback();
}

void aisp_app_resum(void)
{
    aisp_wake(20);//恢复APP
}

void aisp_app_suspend(void)
{
    aisp_wake(21);//挂起APP
}

static void aisp_mic_suspend_check(void)
{
    if (aisp_mic_suspend && audio_app_all_play_stop_status(0)) {
        printf("err aisp_mic_suspend \n");
    }
}

void esr_queue_task(void *priv)
{
    int err = 0;
    int msg[8] = {0};   //接收消息队列buf
    int alow_enter_wifi_cfg = 0;
    int index = 0;
    sys_timer_add_to_task("app_core", NULL, aisp_mic_suspend_check, 10 * 1000);
    while (1) {
        //阻塞等待消息
        err = os_taskq_pend("taskq", msg, ARRAY_SIZE(msg));
        if (err != OS_TASKQ || msg[0] != Q_USER) {
            continue;
        }
        printf("-->recv msg kid[%d]\n", msg[1]);
        struct key_event key = {0};
        key.type = KEY_EVENT_USER;
        int net_ch = 0;
#if (defined CONFIG_WIFI_ENABLE && defined CONFIG_LTE_PHY_ENABLE)
        net_ch = sys_net_channel_read();
        if (net_ch < 0) {
            net_ch = 0;
        }
        if (net_ch == 1) {
            websockets_client_best_one_packge_set(1024);
        }
#endif
        index = aisp_wake_world_callback(msg[1]);
        if (index == 1) {
            goto __wakeup;
        } else if (index == 2) {
            goto __wifi_cfg;
        } else if (index) {
            continue;
        }
        switch (msg[1]) {
//keyword[0]=xiao3 fei1 xiao3 fei1, state=30, threshold=1450, low=10000, bound=3, subcm=1
//keyword[1]=da3 kai1 deng1 guang1, state=33, threshold=650, low=10000, bound=4, subcm=1
//keyword[2]=qie1 huan4 deng1 guang1, state=42, threshold=600, low=10000, bound=4, subcm=1
//keyword[3]=guan1 bi deng1 guang1, state=33, threshold=650, low=10000, bound=3, subcm=1
//keyword[4]=da3 kai1 xiang1 xun1, state=33, threshold=650, low=10000, bound=2, subcm=1
//keyword[5]=guan1 bi xiang1 xun1, state=33, threshold=700, low=10000, bound=2, subcm=1
//keyword[6]=xian3 shi4 shi1 du1, state=30, threshold=650, low=10000, bound=2, subcm=1
//keyword[7]=xian3 shi4 kong1 qi4 zhi3 shu4, state=45, threshold=550, low=10000, bound=2, subcm=1
//keyword[8]=pei4 wang3 mo2 shi4, state=27, threshold=600, low=10000, bound=2, subcm=1
//keyword[9]=lan2 ya2 yin1 xiang3 mo2 shi4, state=45, threshold=650, low=10000, bound=2, subcm=1
//keyword[10]=tiao2 gao1 yin1 liang4, state=33, threshold=700, low=10000, bound=4, subcm=1
//keyword[11]=tiao2 di1 yin1 liang4, state=33, threshold=700, low=10000, bound=4, subcm=1
        case 0:; // wake up
__wakeup:
            ;
            extern int sys_connect_net_success(void);
            extern int is_production_test_enter(char wake);
            if (is_production_test_enter(1)) {
                break;
            }
            auto_sleep_check_clear();
            if (sys_connect_net_success() && !keyword_wifi_enter_congfig) {
#ifdef AI_UART_CMD_CTROL_ENABLE
                ai_uart_cmd_data_push(AI_UART_CMD_DIALUOGE_START, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
                at_uart_cmd_send(AI_UART_CMD_DIALUOGE_START, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
                play_face_emoji(AI_UART_CMD_DIALUOGE_START);
#endif
                alow_enter_wifi_cfg = 0;
                if (ifly_send_pcm != SEND_PCM_PAUSE) {
                    //通知app挂起
                    if (last_key_supspend != KEY_SUPSPEND) {
                        key.action = KEY_EVENT_LONG;
                        key.value = KEY_SUPSPEND;
                        if (!ai_speaker_app()) {
                            printf("-> ! ai_speaker_app\n");
                            key_event_notify(KEY_EVENT_FROM_USER, &key);
                            last_key_supspend = KEY_SUPSPEND;
                            os_time_dly(10);
                        }
                    }

                    struct application *app = get_current_app();
                    if (app && (!strcmp(app->name, "bt_music"))) { //蓝牙播放模式下延时
                        os_time_dly(50);
                    }
                    keyword_start = 0;
                    sys_vad_clear(0);
                    sys_vad_lock(1);
                    ifly_send_pcm = SEND_PCM_STOP;

                    music_buf_play_accept(0);
                    websockets_close_request(1);
                    websockets_dialogue_timeout_del();
                    websockets_free_lbuf_buf();

                    if (alarm_music_play_stop()) { //闹钟在响停止闹钟
                        music_play_stop(NULL);
                        music_buf_play_stop_all();
                        music_buf_play_set_stop();
                        http_tts_play_stop_force();
                        //net_music_play_set_stop();
                        music_buf_play_free_lbuf();
                        music_buf_play_stop_waite();//等待关闭完成再播放提示音
                    }
                    net_music_num_clear();
                    led_eya_wake(1);//LED眼睛唤醒显示

                    char hello_index = rand() % 5;
#ifdef CONFIG_LVGL_UI_ENABLE
                    lv_demo_ai_dialogue_start(hello_index);
#endif
                    if (!aisp_wake_no_play_notice()) {
                        aisp_mic_gain_suspend();
                        if (!music_play_hello(hello_index)) {
                            music_play_waite();
                        }
                        aisp_mic_gain_resum();
                        music_buf_play_set_stop();//停止TTS使用PIPE BUF
                    }
                    keyword_start = 1;
                    pcm_buf_clear();
                    sys_vad_lock(0);
                    sys_vad_clear(1);
                    if (aisp_wake_callback(msg[1])) {
                        keyword_start = 0;
                    } else {
                        websockets_send_audio_buf_init();
                        websockets_send_500ms_audio_buf_push(NULL, 0, 1);
                        websocket_client_thread_create(1);
                    }
                    music_buf_play_accept(1);
                }
            } else if (keyword_wifi_enter_congfig) {
                alow_enter_wifi_cfg = 0;
                if (aisp_wake_callback(-1)) {
                    break;
                }
                if (keyword_wifi_enter_congfig == 3) { //正在连接WiFi
                    if (net_ch == 0) {
                        music_play_res_file("WifiConting.mp3");
                    } else {
                        music_play_res_file("NetConting.mp3");
                    }
                } else { //进入配网
                    if (net_ch == 0) {
                        music_play_res_file("WifiNote.mp3");
                    } else {
                        music_play_res_file("NetNote.mp3");
                    }
                }
            } else { //网络异常
                alow_enter_wifi_cfg = 1;
                if (aisp_wake_callback(-1)) {
                    break;
                }
                if (net_ch == 0) {
                    music_play_res_file("WifiConErr.mp3");
                } else {
                    music_play_res_file("NetDiscon.mp3");
                }
            }
            break;
        case 1:
            aisp_wake_callback(msg[1]);
            break;
        case 2:
            aisp_wake_callback(msg[1]);
            break;
        case 3:
            aisp_wake_callback(msg[1]);
            break;
        case 4:
            aisp_wake_callback(msg[1]);
            break;
        case 5:
            aisp_wake_callback(msg[1]);
            break;
        case 6:
            aisp_wake_callback(msg[1]);
            break;
        case 7:
            aisp_wake_callback(msg[1]);
            break;
        case 8:; // 配网模式
__wifi_cfg:
            ;
            extern int is_production_test_enter(char wake);
            if (is_production_test_enter(1)) {
                break;
            }
            if (keyword_start || alow_enter_wifi_cfg || websockets_nobind_check() || (keyword_wifi_enter_congfig == 3 && !sys_connect_net_success)) {
#ifdef AI_UART_CMD_CTROL_ENABLE
                ai_uart_cmd_data_push(AI_UART_CMD_WIFI_CONFIG, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
                at_uart_cmd_send(AI_UART_CMD_WIFI_CONFIG, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
                play_face_emoji(AI_UART_CMD_WIFI_CONFIG);
#endif
                if (!ai_speaker_app()) {
                    extern int audio_app_mode_switch_set(char *name, char note);
                    audio_app_mode_switch_set("ai_speaker", 0);
                }
                keyword_start = 0;
                keyword_wifi_enter_congfig = 1;
                sys_vad_clear(0);
                sys_vad_lock(1);
                ifly_send_pcm = SEND_PCM_STOP;

                websockets_client_dialogue_exit();
                websockets_close_request(1);
                websockets_dialogue_timeout_del();

                music_buf_play_stop_all();
                music_buf_play_set_stop();
                http_tts_play_stop_force();
                net_music_play_set_stop();
                websockets_free_lbuf_buf();
                music_buf_play_free_lbuf();

                music_buf_play_stop_waite();//等待关闭完成再播放提示音
                net_music_num_clear();
                pcm_buf_clear();
                if (net_ch == 0) {
                    music_play_res_file("WifiNote.mp3");
                } else {
                    music_play_res_file("NetNote.mp3");
                }
                if (!is_in_config_network_state()) {
                    config_network_start();
                }
                sys_vad_lock(0);
                sys_vad_clear(1);
#if (defined TCFG_LED_STATUES_VBAT_NET_EN && TCFG_LED_STATUES_VBAT_NET_EN == 1)
                extern void led_status_set(int status);//0初始化 1网络异常-快闪  2电池<20%-快闪  3网络正常和电池>=20%-常亮
                led_status_set(1);
#endif
            }
            aisp_wake_callback(msg[1]);
            break;
        case 9:
            break;
        case 10://调高音量
            aisp_wake_callback(msg[1]);
            if (keyword_start) {
                aisp_clear(0);
                key.action = KEY_EVENT_CLICK;
                key.value = KEY_VOLUME_INC;
                key_event_notify(KEY_EVENT_FROM_USER, &key);
                extern int music_play_anser_OK(void);
                sys_timeout_add_to_task("sys_timer", NULL, music_play_anser_OK, 500);
            }
            break;
        case 11://调低音量
            aisp_wake_callback(msg[1]);
            if (keyword_start) {
                aisp_clear(0);
                key.action = KEY_EVENT_CLICK;
                key.value = KEY_VOLUME_DEC;
                key_event_notify(KEY_EVENT_FROM_USER, &key);
                extern int music_play_anser_OK(void);
                sys_timeout_add_to_task("sys_timer", NULL, music_play_anser_OK, 500);
            }
            break;
        case 20:;//恢复APP
            //printf("-> last_key_supspend = %d \n",last_key_supspend);
            extern int ai_speaker_app(void);
            websockets_close_request(1);
            if (last_key_supspend != KEY_RESUM) {
                last_key_supspend = KEY_RESUM;
                key.action = KEY_EVENT_LONG;
                key.value = KEY_RESUM;
                if (!ai_speaker_app()) {
                    key_event_notify(KEY_EVENT_FROM_USER, &key);
                }
            }
            break;
        case 21:
            //通知app挂起
            if (last_key_supspend != KEY_SUPSPEND) {
                key.action = KEY_EVENT_LONG;
                key.value = KEY_SUPSPEND;
                if (!ai_speaker_app()) {
                    printf("-> ! ai_speaker_app\n");
                    key_event_notify(KEY_EVENT_FROM_USER, &key);
                    last_key_supspend = KEY_SUPSPEND;
                }
            }
            break;
        case 22:
//                keyword_start = 0;
//                sys_vad_clear(0);
//                sys_vad_lock(1);
//                ifly_send_pcm = SEND_PCM_STOP;
//                music_buf_play_set_stop();
//                http_tts_play_stop_force();
//                net_music_play_set_stop();
//                sys_vad_clear(0);
//                websockets_free_lbuf_buf();
//                music_buf_play_free_lbuf();
//                websockets_close_request(1);
//                websockets_dialogue_timeout_del();
//                music_buf_play_stop_waite();//等待关闭完成再播放提示音
//                net_music_num_clear();
//                sys_vad_lock(0);
//                sys_vad_clear(1);
            break;
        default:
            break;
        }
    }
}

void app_esr_take()
{
    static char esr_queue_task_init = 0;
    if (!esr_queue_task_init) {
        if (thread_fork("esr_queue_task", 27,   1024, 256, NULL, esr_queue_task, NULL) != OS_NO_ERR) {
            printf("esr_queue_task thread fork fail\n");
        } else {
            esr_queue_task_init = 1;
        }
    }
}

int aisp_open(u16 sample_rate)
{
    if (aisp_open_init) {
        printf("---------aisp_already open---------\n");
        aisp_clear(0);
        return 0;
    }
    aisp_open_init = 1;
    ifly_sample_rate = sample_rate;
    qyai_audio_sample_rate_set(sample_rate);
    printf("---------aisp_open---------\n");

    if (!asr_open_init) {
        asr_open_init = 1;
        record_config();

        user_aiui_init();

        //前端和唤醒引擎初始化
        ifly_engine_init();

        //本地唤醒识别线程
        create_awake_task();

        // 唤醒识别处理任务
        app_esr_take();

        // aiui处理线程
#if (ASR_USED_AVD == ASR_IFLY_AVD)
        user_app_recognize_task();
#endif

        //    user_app_main();
        //    aisp_clear(0);
    }
    record_server_init();
    return 0;
}

void aisp_suspend(void)
{
    union audio_req req = {0};

    if (!run_flag || !server_handle) {
        return;
    }

    printf("---------aisp_suspend---------\n");
    run_flag = 0;
    req.enc.cmd = AUDIO_ENC_STOP;
    server_request(server_handle, AUDIO_REQ_ENC, &req);

    memset(&req, 0, sizeof(req));
    req.enc.cmd = AUDIO_ENC_CLOSE;
    server_request(server_handle, AUDIO_REQ_ENC, &req);

#if CONFIG_AISP_AEC_USE_LINEIN
    if (linein_server_handle) {
        memset(&req, 0, sizeof(req));
        req.enc.cmd = AUDIO_ENC_STOP;
        server_request(linein_server_handle, AUDIO_REQ_ENC, &req);

        memset(&req, 0, sizeof(req));
        req.enc.cmd = AUDIO_ENC_CLOSE;
        server_request(linein_server_handle, AUDIO_REQ_ENC, &req);
    }
#endif

    cbuf_clear(&gs_mic_cbuf);
#if CONFIG_AISP_AEC_USE_LINEIN
    cbuf_clear(&gs_linein_cbuf);
#endif
    cbuf_clear(&s_audio_cbuf);
}

void aisp_resume(void)
{
    union audio_req req_mic = {0};
#if CONFIG_AISP_AEC_USE_LINEIN
    union audio_req req_linein = {0};
#endif

    if (run_flag || !server_handle) {
        return;
    }
#if CONFIG_AISP_AEC_USE_LINEIN
    if (!linein_server_handle) {
        return;
    }
#endif

    run_flag = 1;
    // os_sem_set(&read_available_sem, 0);
    // os_sem_post(&read_available_sem);

    req_mic.enc.cmd = AUDIO_ENC_OPEN;
    req_mic.enc.sample_rate = ifly_sample_rate;
#if CONFIG_AISP_AEC_USE_LINEIN
    req_mic.enc.format = "pcm";
#else
    req_mic.enc.format = qyai_mic_audio_enc_format();//"pcm";
    if (!strcmp(req_mic.enc.format, "opus")) {
        req_mic.enc.no_header = 1;//1表示没有头部的意思
    }
#endif

#if IFLY_AUDIO_RECORD_MIC_COUNT == 1
#if CONFIG_AISP_AEC_USE_LINEIN
    req_mic.enc.channel = 1;
    if (aisp_mic_channel_set) {
        req_mic.enc.channel_bit_map = BIT(aisp_mic_channel_set);
    } else {
        req_mic.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL);
    }
#else
    req_mic.enc.channel = 2;
    if (aisp_mic_channel_set) {
        req_mic.enc.channel_bit_map = BIT(aisp_mic_channel_set) | BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
    } else {
        req_mic.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL) | BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
    }
#endif
#else
#if CONFIG_AISP_AEC_USE_LINEIN
    req_mic.enc.channel = 2;
    req_mic.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL) | BIT(CONFIG_AISP_MIC1_ADC_CHANNEL);
#else
    req_mic.enc.channel = 3;
    req_mic.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL) | BIT(CONFIG_AISP_MIC1_ADC_CHANNEL) | BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
#endif
#endif // IFLY_AUDIO_RECORD_MIC_COUNT

    req_mic.enc.frame_size = IFLY_AUDIO_RECORD_ONCE_SR_POINTS * 2 * req_mic.enc.channel;
    req_mic.enc.sample_source = "mic";
    req_mic.enc.volume = CONFIG_AISP_MIC_ADC_GAIN;
    req_mic.enc.vfs_ops = &ifly_audio_record_vfs_ops;
    req_mic.enc.output_buf_len = req_mic.enc.frame_size * 3;
    req_mic.enc.file = (FILE *)&gs_mic_cbuf;
    int err = server_request(server_handle, AUDIO_REQ_ENC, &req_mic);
    if (err) {
        printf("mic encoder open failed: %d\n", err);
        run_flag = 0;
        return;
    }

#if CONFIG_AISP_AEC_USE_LINEIN
    req_linein.enc.cmd = AUDIO_ENC_OPEN;
    req_linein.enc.channel = 1;
    req_linein.enc.channel_bit_map = TCFG_LINEIN_CHANNEL_MAP;
    req_linein.enc.frame_size = IFLY_AUDIO_RECORD_ONCE_SR_POINTS * sizeof(short);
    req_linein.enc.output_buf_len = req_linein.enc.frame_size * 3;
    req_linein.enc.volume = CONFIG_AISP_LINEIN_ADC_GAIN;
    req_linein.enc.sample_rate = ifly_sample_rate;
    req_linein.enc.format = "pcm";
    req_linein.enc.sample_source = "linein";
    req_linein.enc.vfs_ops = &ifly_audio_record_vfs_ops;
    req_linein.enc.file = (FILE *)&gs_linein_cbuf;
    err = server_request(linein_server_handle, AUDIO_REQ_ENC, &req_linein);
    if (err) {
        union audio_req close_req = {0};
        printf("linein encoder open failed: %d\n", err);
        close_req.enc.cmd = AUDIO_ENC_CLOSE;
        server_request(server_handle, AUDIO_REQ_ENC, &close_req);
        run_flag = 0;
        return;
    }
#endif

    union audio_req req_vol = {0};
    //麦克风增益分开设置
    if (aisp_mic_channel_set) {
        req_vol.enc.channel_bit_map = BIT(aisp_mic_channel_set);
    } else {
        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL);
    }
    req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
    req_vol.enc.volume = CONFIG_AISP_MIC_ADC_GAIN;
    server_request(server_handle, AUDIO_REQ_ENC, &req_vol);

#if IFLY_AUDIO_RECORD_MIC_COUNT == 2
    req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC1_ADC_CHANNEL);
    req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
    req_vol.enc.volume = CONFIG_AISP_MIC_ADC_GAIN;
    server_request(server_handle, AUDIO_REQ_ENC, &req_vol);
#endif // IFLY_AUDIO_RECORD_MIC_COUNT

    memset(&req_vol, 0, sizeof(req_vol));
    req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
    req_vol.enc.volume = CONFIG_AISP_AEC_ADC_GAIN;
#if CONFIG_AISP_AEC_USE_LINEIN
    req_vol.enc.channel_bit_map = TCFG_LINEIN_CHANNEL_MAP;
    server_request(linein_server_handle, AUDIO_REQ_ENC, &req_vol);
#else
    req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
    server_request(server_handle, AUDIO_REQ_ENC, &req_vol);
#endif

    //SFR(JL_ANA->DAA_CON0, 2,  2, 0x3);//设置DACVDD-3V
}

int aisp_mic_gain_suspend(void)
{
    u32 rets, reti, cpu_id;
    __asm__ volatile("%0 = rets ;" : "=r"(rets));
    printf("->aisp_mic_gain_suspend = 0x%x\n", rets);
    aisp_mic_suspend = true;
    int pa_volume = 0;
    sys_volume_read(&pa_volume);//获取音量
    aisp_mic_gain_set(pa_volume);//设置回采和mic的增益
    speech_digital_vol_agc_enable(1);//只有播放音乐才加数字增益
    //speech_digital_vol_agc_enable(speech_digital_vol_agc);//只有播放音乐才加数字增益
    return 0;
}

int aisp_mic_gain_resum(void)
{
    u32 rets, reti, cpu_id;
    __asm__ volatile("%0 = rets ;" : "=r"(rets));
    printf("->aisp_mic_gain_resum = 0x%x\n", rets);
    aisp_mic_suspend = false;
    union audio_req req_vol = {0};
    if (server_handle && run_flag) {
        speech_digital_vol_agc_enable(0);//只有播放音乐才加数字增益
        if (aisp_mic_channel_set) {
            req_vol.enc.channel_bit_map = BIT(aisp_mic_channel_set);
        } else {
            req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL);
        }
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = CONFIG_AISP_MIC_ADC_GAIN;//恢复增益到默认
        server_request(server_handle, AUDIO_REQ_ENC, &req_vol);

#if IFLY_AUDIO_RECORD_MIC_COUNT == 2
        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC1_ADC_CHANNEL);//接近喇叭的MIC需要减5左右
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = CONFIG_AISP_MIC_ADC_GAIN;//恢复增益到默认
        server_request(server_handle, AUDIO_REQ_ENC, &req_vol);
#endif

        memset(&req_vol, 0, sizeof(req_vol));
#if CONFIG_AISP_AEC_USE_LINEIN
        req_vol.enc.channel_bit_map = TCFG_LINEIN_CHANNEL_MAP;
#else
        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
#endif
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = CONFIG_AISP_AEC_ADC_GAIN;//恢复增益到默认
#if CONFIG_AISP_AEC_USE_LINEIN
        server_request(linein_server_handle, AUDIO_REQ_ENC, &req_vol);
#else
        server_request(server_handle, AUDIO_REQ_ENC, &req_vol);
#endif
    }
    return 0;
}

int aisp_mic_gain_set(unsigned char volume)
{
    union audio_req req_vol = {0};

    if (server_handle && run_flag) {
        aisp_aec_gain = AEC_GAIN(volume);
        aisp_mic0_gain = MIC0_GAIN(volume);
        aisp_mic1_gain = MIC1_GAIN(volume);

        aisp_aec_gain = aisp_aec_gain >= MIC_MAX_GAIN ? MIC_MAX_GAIN : (aisp_aec_gain < 30 ? 30 : aisp_aec_gain);
        aisp_mic0_gain = aisp_mic0_gain >= MIC_MAX_GAIN ? MIC_MAX_GAIN : (aisp_mic0_gain < 10 ? 10 : aisp_mic0_gain);
        aisp_mic1_gain = aisp_mic1_gain >= MIC_MAX_GAIN ? MIC_MAX_GAIN : (aisp_mic1_gain < 10 ? 10 : aisp_mic1_gain);
        printf("-> volume = %d, aec = %d, mic0 = %d, mic1 = %d \n", volume, aisp_aec_gain, aisp_mic0_gain, aisp_mic1_gain);

        if (aisp_mic_channel_set) {
            req_vol.enc.channel_bit_map = BIT(aisp_mic_channel_set);
        } else {
            req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL);
        }
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = aisp_mic0_gain;//CONFIG_AISP_MIC_ADC_GAIN;
        server_request(server_handle, AUDIO_REQ_ENC, &req_vol);

#if IFLY_AUDIO_RECORD_MIC_COUNT == 2
        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC1_ADC_CHANNEL);//接近喇叭的MIC需要减5左右
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = aisp_mic1_gain;//CONFIG_AISP_MIC_ADC_GAIN;
        server_request(server_handle, AUDIO_REQ_ENC, &req_vol);
#endif // IFLY_AUDIO_RECORD_MIC_COUNT

        memset(&req_vol, 0, sizeof(req_vol));
#if CONFIG_AISP_AEC_USE_LINEIN
        req_vol.enc.channel_bit_map = TCFG_LINEIN_CHANNEL_MAP;
#else
        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
#endif
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = aisp_aec_gain;
#if CONFIG_AISP_AEC_USE_LINEIN
        server_request(linein_server_handle, AUDIO_REQ_ENC, &req_vol);
#else
        server_request(server_handle, AUDIO_REQ_ENC, &req_vol);
#endif
    }
    return 0;
}

int aisp_all_mic_gain_set(int volume, int aec_gian, int mic0_gian, int mic1_gian)
{
    union audio_req req_vol = {0};

    if (server_handle && run_flag && aec_gian) {
        aec_gian = aec_gian >= MIC_MAX_GAIN ? MIC_MAX_GAIN : (aec_gian < 30 ? 30 : aec_gian);
        mic0_gian = mic0_gian >= MIC_MAX_GAIN ? MIC_MAX_GAIN : (mic0_gian < 10 ? 10 : mic0_gian);
        mic1_gian = mic1_gian >= MIC_MAX_GAIN ? MIC_MAX_GAIN : (mic1_gian < 10 ? 10 : mic1_gian);
        printf("-> volume = %d, aec_gain = %d, mic0_gian = %d, mic1_gian = %d\n", volume, aec_gian, mic0_gian, mic1_gian);

        if (aisp_mic_channel_set) {
            req_vol.enc.channel_bit_map = BIT(aisp_mic_channel_set);
        } else {
            req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL);
        }
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = mic0_gian;//CONFIG_AISP_MIC_ADC_GAIN;
        server_request(server_handle, AUDIO_REQ_ENC, &req_vol);

#if IFLY_AUDIO_RECORD_MIC_COUNT == 2
        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC1_ADC_CHANNEL);//接近喇叭的MIC需要减5左右
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = mic1_gian;//CONFIG_AISP_MIC_ADC_GAIN;
        server_request(server_handle, AUDIO_REQ_ENC, &req_vol);
#endif // IFLY_AUDIO_RECORD_MIC_COUNT

        memset(&req_vol, 0, sizeof(req_vol));
#if CONFIG_AISP_AEC_USE_LINEIN
        req_vol.enc.channel_bit_map = TCFG_LINEIN_CHANNEL_MAP;
#else
        req_vol.enc.channel_bit_map = BIT(CONFIG_AISP_MIC2_AEC_ADC_CHANNEL);
#endif
        req_vol.enc.cmd = AUDIO_ENC_SET_VOLUME;
        req_vol.enc.volume = aec_gian;
#if CONFIG_AISP_AEC_USE_LINEIN
        server_request(linein_server_handle, AUDIO_REQ_ENC, &req_vol);
#else
        server_request(server_handle, AUDIO_REQ_ENC, &req_vol);
#endif
    }
    return 0;
}

void aisp_close(void)
{
    union audio_req req = {0};
    if (!server_handle || !aisp_open_init) {
        return;
    }
    aisp_open_init = 0;
    printf("---------aisp_close---------\n");
    aisp_suspend();
    server_close(server_handle);
    server_handle = NULL;
#if CONFIG_AISP_AEC_USE_LINEIN
    if (linein_server_handle) {
        server_close(linein_server_handle);
        linein_server_handle = NULL;
    }
#endif

//    asr_open_init = 0;
//    os_sem_post(&read_available_sem);
//    thread_kill(&g_ivwThreadID, KILL_WAIT);
//    ifly_engine_uninit();

    aisp_clear(0);
    printf("---------aisp_close ok---------\n");
}
#endif

