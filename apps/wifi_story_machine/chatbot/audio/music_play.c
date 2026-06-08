#include "system/includes.h"
#include "server/audio_server.h"
#include "server/server_core.h"
#include "app_config.h"
#include "storage_device.h"
#include "reverb_deal.h"
#include "event/key_event.h"
#include "event/device_event.h"
#include "system/wait.h"
#include "system/app_core.h"
#include "os/os_api.h"
#include "volume.h"
#include "lbuf.h"
#include "ai_uart_ctrol.h"


#define WAIT_COMPLETION_SCAN_TIM        100
#define SLEEP_MUSIC_TIME                30 //睡眠音乐播放度搜后暂停播放（单位分钟）

struct music_play_hdl {
    char res_file_path[64];
    char play_loop;
    char volume;
    char play_start;
    int sleep_music_sec;
    int play_time;
    int total_time;
    int wait_completion_id;
    int wait_completion_time;
    FILE *file;
    struct server *dec_server;
    OS_SEM wait_sem;
    OS_MUTEX mutex;
    void (*callback)(void);
};

static struct music_play_hdl music_handler = {0};

#define __this  (&music_handler)

int http_tts_reply(char *utf8_str, int utf8_str_size);
int http_tts_reply_type(int type);
int http_tts_play_wait(void);
int music_play_file(const char *path);
int websocket_client_thread_create(void *priv, char key);
int music_sleep_play(void *priv);

void music_play_set_callback(void (*cb)(void))
{
    if (!__this->callback) {
        __this->callback = cb;
    } else {
        printf("error: callback function(registered) is NULL");
    }
}

void music_play_dialog_timeout_cb(void)
{
    if (__this->callback) {
        printf("->music_play_dialog_timeout_cb\n");
        __this->callback();
        __this->callback = NULL;
    } else {
        printf("callback function is NULL");
    }
}

static void music_play_dec_server_event_handler(void *priv, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
        printf("->local_music: AUDIO_SERVER_EVENT_ERR\n");
        os_sem_post(&__this->wait_sem);
        sys_vad_clear(1);
    case AUDIO_SERVER_EVENT_END:
        printf("->local_music: AUDIO_SERVER_EVENT_END\n");
        music_play_stop(priv);
        if (__this->callback) {
            printf("->local_music***: AUDIO_SERVER_EVENT_END\n");
            __this->callback();
            __this->callback = NULL;
        }
        os_sem_post(&__this->wait_sem);
        sys_vad_clear(1);
        if (__this->play_loop) {
            sys_timeout_add_to_task("sys_timer", __this->res_file_path, music_play_file, 500);
        } else if (__this->play_time >= (__this->total_time - 1)) {
            if (__this->sleep_music_sec && (timer_get_sec() - __this->sleep_music_sec) < (SLEEP_MUSIC_TIME * 60)) {
                sys_timeout_add_to_task("sys_timer", (void*)1, music_sleep_play, 1000);
            }
        } else {
            __this->sleep_music_sec = 0;
        }
        break;
    case AUDIO_SERVER_EVENT_CURR_TIME:
        printf("->play_time: %d\n", argv[1]);
        __this->play_time = argv[1];
        break;
    }
}

int music_play_init(void)
{
    if (__this->dec_server) {
        return 1;
    }
    printf("->music_play_init\n");

    memset(__this, 0, sizeof(struct music_play_hdl));

    os_sem_create(&__this->wait_sem, 0);
    os_mutex_create(&__this->mutex);

    sys_volume_read(&__this->volume);

    __this->dec_server = server_open("audio_server", "dec");
    if (!__this->dec_server) {
        return -1;
    }
    server_register_event_handler_to_task(__this->dec_server, __this->dec_server, music_play_dec_server_event_handler, "sys_timer");

    return 0;
}

void music_play_uninit(void)
{
    music_play_stop(NULL);
    server_close(__this->dec_server);
    __this->dec_server = NULL;
    memset(__this, 0, sizeof(struct music_play_hdl));
    log_i("music_play_uninit\n");
}


//获取断点数据
int music_play_get_breakpoint(struct audio_dec_breakpoint *bp)
{
    int err;
    union audio_req r = {0};

    if (!os_mutex_valid(&__this->mutex)) {
        os_mutex_create(&__this->mutex);
    }
    os_mutex_pend(&__this->mutex, 100 * 30);
    bp->len = 0;
    r.dec.bp = bp;
    r.dec.cmd = AUDIO_DEC_GET_BREAKPOINT;

    if (bp->data) {
        free(bp->data);
        bp->data = NULL;
    }

    err = server_request(__this->dec_server, AUDIO_REQ_DEC, &r);
    if (err) {
        os_mutex_post(&__this->mutex);
        return err;
    }

    if (r.dec.status == AUDIO_DEC_STOP) {
        bp->len = 0;
        free(bp->data);
        bp->data = NULL;
        os_mutex_post(&__this->mutex);
        return -1;
    }
    /* put_buf(bp->data, bp->len); */
    os_mutex_post(&__this->mutex);
    return 0;
}

//设置音量大小
int music_play_set_volume(int volume)
{
    union audio_req req = {0};
    if (!__this->file) {
        return -1;
    }
    if (!os_mutex_valid(&__this->mutex)) {
        os_mutex_create(&__this->mutex);
    }
    os_mutex_pend(&__this->mutex, 100 * 30);
    __this->volume = sys_volume_chack(volume);

    printf("->set_volume: %d\n", __this->volume);
    sys_volume_write(&__this->volume);

    req.dec.cmd     = AUDIO_DEC_SET_VOLUME;
    req.dec.volume  = GET_SET_VOLUME(__this->volume);
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    os_mutex_post(&__this->mutex);
    return 0;
}

//设置音量大小
int music_play_set_volume_step(int step)
{
    union audio_req req = {0};
    if (!__this->file) {
        return -1;
    }
    if (!os_mutex_valid(&__this->mutex)) {
        os_mutex_create(&__this->mutex);
    }
    os_mutex_pend(&__this->mutex, 100 * 30);
    sys_volume_read(&__this->volume);
    __this->volume = sys_volume_chack(__this->volume + step);

    printf("->set_volume: %d\n", __this->volume);
    sys_volume_write(&__this->volume);

    req.dec.cmd     = AUDIO_DEC_SET_VOLUME;
    req.dec.volume  = GET_SET_VOLUME(__this->volume);
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    os_mutex_post(&__this->mutex);
    return 0;
}

//获取解码器状态
int music_play_get_status(void)
{
    union audio_req req = {0};

    if (!__this->dec_server) {
        return 0;
    }
    if (!os_mutex_valid(&__this->mutex)) {
        os_mutex_create(&__this->mutex);
    }
    os_mutex_pend(&__this->mutex, 100 * 30);
    req.dec.cmd     = AUDIO_DEC_GET_STATUS;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    os_mutex_post(&__this->mutex);
    return req.dec.status;//
}

//暂停/继续播放
int music_play_pause(void)
{
    union audio_req r = {0};
    if (!__this->dec_server) {
        return 0;
    }
    if (!os_mutex_valid(&__this->mutex)) {
        os_mutex_create(&__this->mutex);
    }
    os_mutex_pend(&__this->mutex, 100 * 30);
#ifdef CONFIG_DEC_ANALOG_VOLUME_ENABLE
    r.dec.attr = AUDIO_ATTR_FADE_INOUT;
#endif
    r.dec.cmd = AUDIO_DEC_PP;
    int ret = server_request(__this->dec_server, AUDIO_REQ_DEC, &r);
    os_mutex_post(&__this->mutex);
    return ret;
}
int music_play_stop_status(void)
{
    if (!__this->file || !__this->dec_server) {
        return 1;
    }
    return 0;
}

//停止播放
int music_play_stop(void *priv)
{
    int err = 0;
    union audio_req req = {0};

    if (!os_mutex_valid(&__this->mutex)) {
        os_mutex_create(&__this->mutex);
    }
    os_mutex_pend(&__this->mutex, 100 * 30);
    if (!__this->file || !__this->dec_server) {
        os_mutex_post(&__this->mutex);
        return 0;
    }
    log_i("music_play_stop\n");
    req.dec.cmd = AUDIO_DEC_STOP;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    int argv[2];
    argv[0] = AUDIO_SERVER_EVENT_END;
    argv[1] = (int)__this->file;
    server_event_handler_del(__this->dec_server, 2, argv);

    if (__this->file) {
        fclose(__this->file);
        __this->file = NULL;
    }
    os_sem_post(&__this->wait_sem);
    os_mutex_post(&__this->mutex);
    return 0;
}

//停止所有资源播放
int music_play_stop_all(void)
{
    if (__this->wait_completion_id) {
        sys_timer_del(__this->wait_completion_id);
        __this->wait_completion_id = 0;
        __this->wait_completion_time = 0;
    }
    if (!__this->file || !__this->dec_server) {
#if (defined CONFIG_USE_TTS_REPLY_ENABLE || defined CONFIG_MORE_NOTIC_REPLY_ENABLE)
        return http_tts_play_request_stop();
#endif
        return 0;
    }
    return music_play_stop(NULL);
}

int music_play_waite(void)
{
    if (!__this->file || !__this->dec_server) {
#if (defined CONFIG_USE_TTS_REPLY_ENABLE || defined CONFIG_MORE_NOTIC_REPLY_ENABLE)
        http_tts_play_wait();
#endif
        return 0;
    }
    if (os_sem_valid(&__this->wait_sem)) {
        os_sem_set(&__this->wait_sem, 0);
        if (__this->file) {
            return os_sem_pend(&__this->wait_sem, 100 * 30);
        }
    }
    printf("err os_sem_valid\n");
    return 0;
}

static int music_play_waite_scan(void)
{
    __this->wait_completion_time++;
    if ((__this->wait_completion_time * WAIT_COMPLETION_SCAN_TIM) > (1000 * 300)) { //不能有超过300秒的资源文件
        sys_timer_del(__this->wait_completion_id);
        music_play_stop(NULL);
        __this->wait_completion_id = 0;
        __this->wait_completion_time = 0;
    } else {
        if (!__this->dec_server) {
            sys_timer_del(__this->wait_completion_id);
            __this->wait_completion_id = 0;
            __this->wait_completion_time = 0;
        } else if (!__this->file) {
            sys_timer_del(__this->wait_completion_id);
            __this->wait_completion_id = 0;
            __this->wait_completion_time = 0;
            music_play_file(__this->res_file_path);
            __this->res_file_path[0] = 0;
        }
    }
}

//解码文件
int music_play_file(const char *path)
{
    int err;
    union audio_req req = {0};

    if (!path) {
        return -1;
    }
    music_play_init();
    os_mutex_pend(&__this->mutex, 100 * 30);
    if (__this->file && !__this->play_loop) {
        if (!strcmp(__this->res_file_path, path)) {
            os_mutex_post(&__this->mutex);
            return 0;
        }
        strcpy(__this->res_file_path, path);
        printf("-> warnning in more file play\n");
        if (!__this->wait_completion_id) {
            __this->wait_completion_time = 0;
            __this->wait_completion_id = sys_timer_add_to_task("sys_timer", NULL, music_play_waite_scan, WAIT_COMPLETION_SCAN_TIM);
        }
        os_mutex_post(&__this->mutex);
        return 0;
    }
    music_play_stop(NULL);
    if (qyai_chat_addr_version() <= 1) {
        sys_vad_clear(0);
    }
    __this->file = fopen(path, "r");
    if (!__this->file) {
        printf("music_play_file open err : \n", path);
        os_mutex_post(&__this->mutex);
        sys_vad_clear(1);
        return -1;
    }
//    int sample_rate = dac_get_sample_rate();
//    printf("-> sample_rate = %d \n",sample_rate);
    printf("music_play_file : %s\n", path);
    sys_volume_read(&__this->volume);

    req.dec.cmd             = AUDIO_DEC_OPEN;
    req.dec.volume          = GET_SET_VOLUME(__this->volume);
    req.dec.output_buf_len  = 6 * 1024;
    req.dec.file            = __this->file;
    req.dec.channel         = 0;
    req.dec.sample_rate     = 0;
    req.dec.priority        = 1;
    req.dec.sample_source   = "dac";
    req.dec.force_sr        = FORCE_DAC_SAMPLE_TRATE;//sample_rate > 0 ? sample_rate : FORCE_DAC_SAMPLE_TRATE;//强制使用采样率

#if 0   //变声变调功能
    req.dec.speedV = 80; // >80是变快，<80是变慢，建议范围：30到130
    req.dec.pitchV = 32768; // >32768是音调变高，<32768音调变低，建议范围20000到50000
    req.dec.attr = AUDIO_ATTR_PS_EN;
#endif

#if TCFG_EQ_ENABLE
#if defined EQ_CORE_V1
    req.dec.attr |= AUDIO_ATTR_EQ_EN;
#if TCFG_LIMITER_ENABLE
    req.dec.attr |= AUDIO_ATTR_EQ32BIT_EN;
#endif
#if TCFG_DRC_ENABLE
    req.dec.attr |= AUDIO_ATTR_DRC_EN;
#endif
#else
    struct eq_s_attr eq_attr = {0};
    extern void set_eq_req_attr_parm(struct eq_s_attr * eq_attr);
    extern void *get_eq_hdl(void);
    set_eq_req_attr_parm(&eq_attr);
    req.dec.eq_attr = &eq_attr;
    req.dec.eq_hdl = get_eq_hdl();
#endif
#endif

#if CONFIG_DEC_DECRYPT_ENABLE
    //播放加密文件
    extern const struct audio_vfs_ops *get_decrypt_vfs_ops(void);
    req.dec.vfs_ops = get_decrypt_vfs_ops();
    req.dec.attr |= AUDIO_ATTR_DECRYPT_DEC;
#endif

    err = server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    if (err) {
        printf("audio_dec_open: err = %d\n", err);
        fclose(__this->file);
        __this->file = NULL;
        os_mutex_post(&__this->mutex);
        sys_vad_clear(1);
        return err;
    }

    __this->play_time = req.dec.play_time;
    __this->total_time = req.dec.total_time; //获取播放总时长

    req.dec.cmd = AUDIO_DEC_START;

    err = server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    if (err) {
        printf("audio_dec_start: err = %d\n", err);
        fclose(__this->file);
        __this->file = NULL;
        os_mutex_post(&__this->mutex);
        sys_vad_clear(1);
        return err;
    }
    printf("music_play_file: ok\n");
    os_mutex_post(&__this->mutex);
    if (!__this->play_loop) {
        __this->res_file_path[0] = 0;
    }
    return 0;
}

//解码资源文件
int music_play_res_file(const char *name) //Backdown1.mp3
{
    char buf[64];
    struct application *app = get_current_app();

#ifdef CONFIG_AUX_MUSIC_MODE_ENABLE //AUX模式下不能使用播报
    int aux_mode_is_open(void);
    if (aux_mode_is_open() && app && !strcmp(app->name, "aux_music") && strcmp(name, "AuxMode.mp3")) {
        return 0;
    }
#endif
#ifdef CONFIG_USE_TTS_REPLY_ENABLE
    char *reply = NULL;
#ifdef CONFIG_KWS_ENGLISH
    if (!strcmp("MaxVolume.mp3", name)) {
        reply = "Max Volume";
    } else if (!strcmp("MinVolume.mp3", name)) {
        reply = "Min Volume";
    } else if (!strcmp("NoLight.mp3", name)) {
        reply = "No Light";
    }
#else
    if (!strcmp("MaxVolume.mp3", name)) {
        reply = "音量已最大";
    } else if (!strcmp("MinVolume.mp3", name)) {
        reply = "音量已最小";
    } else if (!strcmp("NoLight.mp3", name)) {
        reply = "没有打开灯光";
    }
#endif

#ifdef CONFIG_KWS_ENGLISH
    if (!strcmp("LowLight.mp3", name)) {
        reply = "Low brightness";
    } else if (!strcmp("HightLight.mp3", name)) {
        reply = "Hight brightness";
    } else if (!strcmp("PwrOff.mp3", name)) {
        reply = "Power off";
    }
#else
    if (!strcmp("LowLight.mp3", name)) {
        reply = "已最低亮度";
    } else if (!strcmp("HightLight.mp3", name)) {
        reply = "已最高亮度";
    } else if (!strcmp("PwrOff.mp3", name)) {
        reply = "关机~";
    }
#endif

    if (sys_connect_net_success() && reply) {
        return http_tts_request(reply, strlen(reply) + 1);
    }
#endif
    sprintf(buf, "%s%s", CONFIG_VOICE_PROMPT_FILE_PATH, name);
    return music_play_file(buf);
}
int music_play_backdown(void)//主人我退下了
{
    int ret = 0;
    printf("--->music_play_byebye\n");
#ifdef CONFIG_USE_TTS_REPLY_ENABLE
#ifdef CONFIG_KWS_ENGLISH
    char *byebye[] = {
        "I rested.",
        "I take my leave.",
        "Just call me if you need anything.",
        "Goodbye",
    };
#else
    char *byebye[] = {
        "我休息了",
        "我退下了",
        "有需要再叫我",
        "再见",
    };
#endif
    int index = rand() % 4;
    //ret = http_tts_reply(byebye[index], strlen(byebye[index]) + 1);
    ret = http_tts_reply_type(3);
#else
    char path[32];
    int id = (rand() % 50) % 5 + 1;
    sprintf(path, "byebye%d.mp3", id);
    ret = music_play_res_file(path);
#endif
    return ret;
}
int music_play_hello(char index)//主人我退下了
{
    int ret = 0;
    printf("--->music_play_hello : %d\n", index);
#ifdef CONFIG_USE_TTS_REPLY_ENABLE
#ifdef CONFIG_KWS_ENGLISH
    char *hello[] = {
        "Hey, I’m here!",
        "Hello! how can I help?",
        "Hello, I’m here!",
        "Hi there!",
        "Yep,I'm listening.",
    };
#else
    char *hello[] = {
        "来喽",
        "在呢",
        "你说",
        "我在听",
        "嗯？",
    };
#endif
    //ret = http_tts_reply(hello[index], strlen(hello[index]) + 1);
    ret = http_tts_reply_type(1);
#else
    char path[32];
    sprintf(path, "hello%d.mp3", index + 1);
    ret = music_play_res_file(path);
#endif
    return ret;
}
int aisp_timeout_first_exit(int noclealy)
{
#ifdef CONFIG_MORE_NOTIC_REPLY_ENABLE
#ifdef CONFIG_USE_TTS_REPLY_ENABLE
#ifdef CONFIG_KWS_ENGLISH
    char *req_nospeek[] = {
        "Are you still there?",
        "Are you still chatting with me?",
        "Are you still there?",
        "Are you still talking to me?",
    };
    char *req_noclealy[] = {
        "Are you chatting with me?",
        "Are you talking to me? ",
        "I didn't catch what you said. Could you repeat it?",
        "Um, what were you just saying?",
    };
#else
    char *req_nospeek[] = {
        "你还在不在呢？",
        "你还在和我聊天吗？",
        "你还在吗？",
        "你还在和我说话吗？",
    };
    char *req_noclealy[] = {
        "你是在和我聊天吗？",
        "你是在和我说话吗？",
        "没听清楚你再说什么？",
        "嗯，你刚刚在说什么？",
    };
#endif
    aisp_mic_gain_suspend();
    int index = rand() % 4;
//    if(noclealy){
//        http_tts_request(req_noclealy[index], strlen(req_noclealy[index]) + 1);
//    }else
    {
        //http_tts_request(req_nospeek[index], strlen(req_nospeek[index]) + 1);
        http_tts_reply_type(2);
    }
    http_tts_play_wait();
    aisp_mic_gain_resum();
#else
    aisp_mic_gain_suspend();
    music_play_res_file("WaitSpeek.mp3");
    music_play_waite();
    aisp_mic_gain_resum();
#endif
    return true;
#endif
    return 0;
}
int music_play_light_anser(void *p)//灯光回答
{
    int index = (int)p;
#ifdef CONFIG_USE_TTS_REPLY_ENABLE
    char *reply = NULL;
    switch (index) {
    case 0:
        reply = "好的";
#ifdef CONFIG_KWS_ENGLISH
        reply = "OK";
#else
        reply = "好的";
#endif
        break;
    case 1:
#ifdef CONFIG_KWS_ENGLISH
        reply = "Min brightness.";
#else
        reply = "已最低亮度";
#endif
        break;
    case 2:
#ifdef CONFIG_KWS_ENGLISH
        reply = "Max brightness.";
#else
        reply = "Max brightness.";
#endif
        break;
    case 3:
#ifdef CONFIG_KWS_ENGLISH
        reply = "NO light";
#else
        reply = "没有打开灯光";
#endif
        break;
    default:
        break;
    }
    if (sys_connect_net_success() && reply) {
        return http_tts_request(reply, strlen(reply) + 1);
    }
#endif
    switch (index) {
    case 0:
        music_play_res_file("OK.mp3");
        break;
    case 1:
        music_play_res_file("LowLight.mp3");
        break;
    case 2:
        music_play_res_file("HightLight.mp3");
        break;
    case 3:
        music_play_res_file("NoLight.mp3");
        break;
    default:
        break;
    }
    return 0;
}

int music_play_recording(void)//录音提示
{
    music_play_res_file("Recording.mp3");
    return 0;
}

int music_play_anser_OK(void)//回答"好的"
{
#ifdef CONFIG_USE_TTS_REPLY_ENABLE
    if (sys_connect_net_success()) {
#ifdef CONFIG_KWS_ENGLISH
        char *ok = "OK";
#else
        char *ok = "好的";
#endif
        //return http_tts_request(ok, strlen(ok) + 1);
        return http_tts_reply_type(5);
    }
#endif
    music_play_res_file("OK.mp3");
    return 0;
}

int music_play_anser_volume(int dec)//回答音量已加大已减小
{
#ifdef CONFIG_USE_TTS_REPLY_ENABLE
    if (sys_connect_net_success()) {
#ifdef CONFIG_KWS_ENGLISH
        char *volume = dec ? "Volume decreased" : "Volume increased";
#else
        char *volume = dec ? "音量已减小" : "音量已加大";
#endif

        return http_tts_request(volume, strlen(volume) + 1);
    }
#endif
    if (dec) {
        music_play_res_file("VolumeDec.mp3");
    } else {
        music_play_res_file("VolumeInc.mp3");
    }
    return 0;
}

int music_play_server_err(void)//服务器不回复音频
{
    music_play_stop_all();
    music_play_res_file("ServErr.mp3");//播放音乐文件
    return 0;
}

int music_play_instruc_word_err(void)//不支持指令
{
    int ret = 0;
#ifdef CONFIG_USE_TTS_REPLY_ENABLE
#ifdef CONFIG_KWS_ENGLISH
    char *word = "I haven't learned this function yet.";
#else
    char *word = "我还没学会这个功能呢";
#endif
    ret = http_tts_request(word, strlen(word) + 1);
#else
    ret = music_play_res_file("NoCMD.mp3");//播放音乐文件
#endif
    return ret;
}

int music_play_res_file_loop(void *name)
{
    int ret = 0;
    if (name) {
        __this->play_loop = true;
        sprintf(__this->res_file_path, "%s%s", CONFIG_VOICE_PROMPT_FILE_PATH, name);
        ret = music_play_file(__this->res_file_path);
        if (!ret) {
            aisp_mic_gain_suspend();
        }
    }
    return ret;
}

int music_play_is_loop(void)
{
    return __this->play_loop && __this->res_file_path[0];
}

int music_play_res_file_unloop(void)
{
    int ret = 0;
    __this->play_loop = 0;
    ret = music_play_stop_all();
    aisp_mic_gain_resum();
    return ret;
}

int music_sleep_play(void *priv)//睡眠音乐
{
    char path[64];
    u8 id = (rand() % 50) % 4;
    if ((int)priv == 1) {
        return music_play_stop_all();
    }
    sprintf(path, "%sSleep%d.mp3", CONFIG_VOICE_PROMPT_FILE_PATH, id);
    if (music_play_file(path)) {
        net_music_play_next("musicians_sleep");
    }
    __this->sleep_music_sec = timer_get_sec();
    return 0;
}

int music_sleep_play_auto(void)//睡眠音乐
{
    static char slee_auto = 0;
    int ret = music_sleep_play((void*)slee_auto);
    slee_auto = slee_auto == 0 ? 1 : 0;
    return ret;
}

