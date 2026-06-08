#include "server/audio_server.h"
#include "server/server_core.h"
#include "app_config.h"
#include "storage_device.h"
#include "event/key_event.h"
#include "event/device_event.h"
#include "event/net_event.h"
#include "syscfg/syscfg_id.h"
#include "system/wait.h"
#include "system/app_core.h"
#include "volume.h"
#include "lwip.h"
#include "ai_uart_ctrol.h"
#include "server/ai_server.h"
#include "network_download/net_download.h"
#include "http_chunck_stream_api.h"

#if (defined PRODUCTION_TEST_ENABLE || defined PRODUCTION_ALL_TEST_ENABLE)
#define MUSIC_PLAY_NUM  50  //没有打断情况下：自动连续播放音乐数量
#else
#define MUSIC_PLAY_NUM  30000  //没有打断情况下：自动连续播放音乐数量
#endif

#if (__SDRAM_SIZE__ <= 0)
#define NET_MUSIC_MEM_SMALL
#endif

#ifdef CONFIG_NET_MUSIC_MODE_ENABLE

#if (__SDRAM_SIZE__ > 0)
#define NET_MUSIC_MAX_NUM   6
#else
#define NET_MUSIC_MAX_NUM   2
#endif

struct net_music_list {
    struct list_head entry;
    char music_url[1024];
    char music_name[256];
};
struct net_music_hdl {
    char volume;
    char play_status;
    char request_status;
    char play_loop;
    char play_one;
    u16 wait_download;
    u16 timeout_id;
    int download_ready;
    int play_time;
    int total_time;
    int total_num;
    int suspend_time;
    void *net_file;
    const char *ai_name;
    struct server *dec_server;
    struct server *ai_server;
    void *spectrum_fft_hdl;
    char *url; //保存断点歌曲的链接
    char *play_next_mode;
    struct audio_dec_breakpoint dec_bp;
    OS_MUTEX mutex;
    struct list_head head;
    unsigned char list_num;
    struct net_music_list *new_list;
    struct net_music_list *last_list;
};
static char music_name[128];
static char music_url[1024];
static struct net_music_hdl net_music_handler;
#define __this  (&net_music_handler)

int net_music_dec_file(const char *url);
int websocket_client_thread_create(void *priv);
int websocket_client_next_music_play(void *priv);
int http_music_play_set_puase(char pause, int timeout_sec);
int http_music_play_request_stop(void);
int http_music_play_request(char *url, int buf_size);
int http_music_play_get_puase(void);
int http_music_play_get_stop(void);
void http_music_play_end_callback(int (*cb)(int event, int time));

int music_buf_play_set_volume(int volume);
int music_buf_play_set_volume_step(int step);
void music_buf_play_set_stop(void);

static int net_music_dec_end(int is_force_stop);
static int net_music_mode_init(void);
static int net_music_mode_init(void);
void net_music_play_set_stop(void);
void net_music_name_save(char *name);

#ifndef NET_MUSIC_MEM_SMALL
//网络解码需要的数据访问句柄
static const struct audio_vfs_ops net_audio_dec_vfs_ops = {
    .fread = net_download_read,
    .fseek = net_download_seek,
    .flen  = net_download_get_file_len,
};

//int get_app_music_volume(void)
//{
//    return __this->volume;
//}
//
//int get_app_music_playtime(void)
//{
//    return __this->play_time;
//}
//
//int get_app_music_total_time(void)
//{
//    return __this->total_time;
//}

//检查音频需要的格式检查数据头部是否下载缓冲完成
static int __net_download_ready(void *p)
{
    __this->download_ready = net_download_check_ready(__this->net_file);
    if (__this->download_ready) {
        printf("---> __net_download_ready \n");
        return 1;
    }
    return 0;
}
static void net_music_dec_play_pause_tolong(void*p)
{
    if (net_music_play_pause_status()) {
        net_music_dec_stop();
    }
    __this->suspend_time = 0;
}
int net_music_set_dec_volume(int volume);
static int net_music_dec_play_pause(u8 notify)
{
    union audio_req r = {0};
    union ai_req req  = {0};
    int err = 0;
    __this->request_status = false;
    if (!__this->net_file || !__this->dec_server) {
        __this->play_status = 0;
        return -1;
    }
    int volume = 0;
    sys_volume_read(&volume);
    if (__this->volume != volume) {
        net_music_set_dec_volume(volume);
    }

#ifdef NET_MUSIC_CONFIG_DEC_ANALOG_VOLUME_ENABLE
    r.dec.attr = AUDIO_ATTR_FADE_INOUT;
#endif
    r.dec.cmd = AUDIO_DEC_PP;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &r);
    __this->play_status = r.dec.status;

    if (r.dec.status == AUDIO_DEC_START) {//播放状态
        net_download_set_pp(__this->net_file, 0);//播放状态
        if (__this->suspend_time) {
            sys_timer_del(__this->suspend_time);
            __this->suspend_time = 0;
        }
        printf("======== AUDIO_DEC_STAR\n");
#if (defined CONFIG_UI_ENABLE && defined CONFIG_LVGL_UI_ENABLE)
        lv_demo_music_play_pause_button(0);
#endif
    } else if (r.dec.status == AUDIO_DEC_PAUSE) {//暂停状态
        net_download_set_pp(__this->net_file, 1);//暂停播放
        if (!__this->suspend_time) {
            __this->suspend_time = sys_timeout_add(NULL, net_music_dec_play_pause_tolong, 20 * 60 * 1000); //20分钟暂停则关闭
        } else {
            sys_timer_modify(__this->suspend_time, 20 * 60 * 1000); //20分钟暂停则关闭
        }
        err = 1;
        printf("======== AUDIO_DEC_PAUSE\n");
#if (defined CONFIG_UI_ENABLE && defined CONFIG_LVGL_UI_ENABLE)
        lv_demo_music_play_pause_button(1);
#endif
    } else {
        if (__this->suspend_time) {
            sys_timer_del(__this->suspend_time);
            __this->suspend_time = 0;
        }
        printf("======== AUDIO_DEC err = %d\n", r.dec.status);
    }
    if (notify && __this->ai_server && r.dec.status != AUDIO_DEC_STOP) {
        //notify : 是否需要通知云端暂停
        req.evt.event   = AI_EVENT_PLAY_PAUSE;
        req.evt.ai_name = __this->ai_name;
        if (r.dec.status == AUDIO_DEC_PAUSE) {
            req.evt.arg = 1;
        }
        ai_server_request(__this->ai_server, AI_REQ_EVENT, &req);
    }
    return err;
}
#endif // NET_MUSIC_NO_SURPPOT_PAUSE

int net_music_play_stop_status(void)
{
#ifdef NET_MUSIC_MEM_SMALL
    return http_music_play_get_stop();
#else
    if (!__this->net_file || !__this->dec_server || __this->play_status == AUDIO_DEC_STOP || __this->play_status == AUDIO_DEC_PAUSE || __this->play_status == 0) {
        return true;
    }
#endif
    return false;
}

int net_music_play_pause_status(void)
{
#ifdef NET_MUSIC_MEM_SMALL
    return http_music_play_get_puase();
#else
    union audio_req req = {0};
    if (!__this->net_file || !__this->dec_server) {
        return 0;
    }
    if (__this->play_status == AUDIO_DEC_PAUSE) {
        return true;
    }
    req.dec.cmd = AUDIO_DEC_GET_STATUS;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    return req.dec.status == AUDIO_DEC_PAUSE;//AUDIO_DEC_OPEN  AUDIO_DEC_START  AUDIO_DEC_PAUSE
#endif
}
int net_music_play_start_status(void)
{
#ifdef NET_MUSIC_MEM_SMALL
    return !http_music_play_get_stop();
#else
    union audio_req req = {0};
    if (!__this->net_file || !__this->dec_server) {
        return 0;
    }
    if (__this->play_status == AUDIO_DEC_START) {
        return true;
    }
    req.dec.cmd = AUDIO_DEC_GET_STATUS;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    return req.dec.status == AUDIO_DEC_START;//AUDIO_DEC_OPEN  AUDIO_DEC_START  AUDIO_DEC_PAUSE
#endif
}
//暂停/继续播放
int net_music_play_pause(u8 stop)
{
    union audio_req req = {0};
    int err = 0;
    __this->request_status = false;

#ifdef NET_MUSIC_MEM_SMALL
    if (http_music_play_get_stop() > 0) {
#ifdef AI_UART_CMD_CTROL_ENABLE
        ai_uart_cmd_data_push(stop ? AI_UART_CMD_NET_MUSIC_STOP : AI_UART_CMD_NET_MUSIC_PLAY, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
        at_uart_cmd_send(stop ? AI_UART_CMD_NET_MUSIC_STOP : AI_UART_CMD_NET_MUSIC_PLAY, NULL);
#endif
    }
    return http_music_play_set_puase(stop, 20 * 60);//暂停20分钟
#else
    if (!__this->net_file || !__this->dec_server || __this->play_status == AUDIO_DEC_STOP  || __this->play_status == 0) {
        __this->play_status = 0;
        return -1;
    }
    req.dec.cmd = AUDIO_DEC_GET_STATUS;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    if (req.dec.status == AUDIO_DEC_STOP && __this->wait_download) {
        printf("__this->play_status = AUDIO_DEC_PP\n");
        __this->play_status = AUDIO_DEC_PP;
        return stop;
    }
    if (__this->play_status == AUDIO_DEC_START && stop == 0) {
        return 0;
    }
    if (__this->play_status == AUDIO_DEC_PAUSE && stop == 1) {
        return 1;
    }
#ifdef AI_UART_CMD_CTROL_ENABLE
    ai_uart_cmd_data_push(stop ? AI_UART_CMD_NET_MUSIC_STOP : AI_UART_CMD_NET_MUSIC_PLAY, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
    at_uart_cmd_send(stop ? AI_UART_CMD_NET_MUSIC_STOP : AI_UART_CMD_NET_MUSIC_PLAY, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
    play_face_emoji(stop ? AI_UART_CMD_NET_MUSIC_STOP : AI_UART_CMD_NET_MUSIC_PLAY);
#endif
    printf("__this->play_status = %d, %d \n", __this->play_status, stop);
    if (!stop) {
        __this->play_status = AUDIO_DEC_START;
        sys_timeout_add_to_task("sys_timer", NULL, net_music_dec_play_pause, 1000);
    } else {
        __this->play_status = AUDIO_DEC_PAUSE;
        net_music_dec_play_pause(0);
        err = 1;
    }
#endif
    return err;
}
//停止播放
int net_music_dec_stop(void)
{
    union audio_req r = {0};
    union ai_req req = {0};
    __this->request_status = false;
    if (__this->timeout_id) {
        sys_timeout_del(__this->timeout_id);
        __this->timeout_id = 0;
    }
#ifdef NET_MUSIC_MEM_SMALL
    if (!http_music_play_get_stop()) {
#ifdef AI_UART_CMD_CTROL_ENABLE
        ai_uart_cmd_data_push(AI_UART_CMD_NET_MUSIC_STOP, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
        at_uart_cmd_send(AI_UART_CMD_NET_MUSIC_STOP, NULL);
#endif
    }

#ifdef CONFIG_UI_PLAY_EMOJI
    if (!http_music_play_get_stop()) {
        play_face_emoji(AI_UART_CMD_NET_MUSIC_STOP);
    }
#endif
    return http_music_play_request_stop();
#else
    if (!__this->net_file) {
        return 0;
    }
#ifdef AI_UART_CMD_CTROL_ENABLE
    ai_uart_cmd_data_push(AI_UART_CMD_NET_MUSIC_STOP, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
    at_uart_cmd_send(AI_UART_CMD_NET_MUSIC_STOP, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
    play_face_emoji(AI_UART_CMD_NET_MUSIC_STOP);
#endif
    os_mutex_pend(&__this->mutex, 6000);
    __this->spectrum_fft_hdl = NULL;
    __this->play_status = AUDIO_DEC_STOP;

    log_i("net_music_dec_stop\n");

    net_download_buf_inactive(__this->net_file);
#ifdef NET_MUSIC_CONFIG_DEC_ANALOG_VOLUME_ENABLE
    r.dec.attr |= AUDIO_ATTR_FADE_INOUT;
#endif
    if (__this->wait_download) {
        /*
         * 歌曲还未开始播放，删除wait
         */
        /* sys_timer_del(__this->wait_download); */
        wait_completion_del(__this->wait_download);
        __this->wait_download = 0;
    } else {
        r.dec.cmd = AUDIO_DEC_STOP;
        server_request(__this->dec_server, AUDIO_REQ_DEC, &r);

        int argv[2];
        argv[0] = AUDIO_SERVER_EVENT_END;
        argv[1] = (int)__this->net_file;
        server_event_handler_del(__this->dec_server, 2, argv);
    }

    extern int lwip_canceladdrinfo(void);
    lwip_canceladdrinfo();
    //释放网络下载资源
    net_download_close(__this->net_file);
    if (__this->suspend_time) {
        sys_timer_del(__this->suspend_time);
        __this->suspend_time = 0;
    }
    __this->net_file = NULL;
    __this->play_status = 0;
    __this->play_one = 0;
    os_mutex_post(&__this->mutex);
#endif
    return 0;
}
static void net_music_request_status_clear(void)
{
    __this->request_status = false;
}
int net_music_play_next(char *priv)
{
    if (__this->timeout_id) {
        sys_timeout_del(__this->timeout_id);
        __this->timeout_id = 0;
    }
    if (websockets_nobind_check() || !sys_connect_net_success() || __this->request_status) {
        return -1;
    }
#ifndef CONFIG_UI_ENABLE
    __this->play_loop = 0;
#endif
    music_buf_play_accept(0);
    websockets_free_lbuf_buf();
    websockets_close_request(1);
    websockets_dialogue_timeout_del();

    net_music_dec_stop();
    net_music_play_set_stop();

    music_play_stop(NULL);
    music_buf_play_stop_all();
    music_buf_play_set_stop();
    music_buf_play_free_lbuf();

    if (priv && priv != __this->play_next_mode) {
        __this->play_next_mode = priv;
    }
    __this->request_status = true;
    sys_timeout_add_to_task("sys_timer", NULL, net_music_request_status_clear, 5000);
    sys_timeout_add_to_task("sys_timer", __this->play_next_mode, websocket_client_next_music_play, 1000);//1秒后再创建，否则会在发断开消息到下次音乐请求
    music_buf_play_accept(1);
    return 0;//websocket_client_thread_create(2);//继续播放下一首
}

void net_music_play_url_type(void *priv)
{
    if (priv) {
        __this->play_next_mode = priv;
        printf("net_music type : %s\n", __this->play_next_mode);
    }
}
void *net_music_play_type_get(void)
{
    return __this->play_next_mode;
}
void net_music_name_save(char *name)
{
    strncpy(music_name, name, sizeof(music_name));
#if (defined CONFIG_UI_ENABLE && defined CONFIG_LVGL_UI_ENABLE)
    if (!lv_demo_is_ring_page()) {
        lv_demo_music_update_title(name);// 使用解析出的歌曲名
    }
#endif
}
int net_music_num_clear(void)
{
    int ret = __this->total_num;
    __this->total_num = 0;
    __this->play_one = 0;
    return ret;
}

#ifdef NET_MUSIC_MEM_SMALL
//播放结束
static int net_music_dec_end(int is_force_stop)
{
    union ai_req req = {0};
    __this->request_status = false;
    int timeout = (__this->total_time - __this->play_time) * 1000;
    timeout = timeout > 0 ? timeout : 0;
    __this->total_num++;
    printf("->net_music play num = %d \n", __this->total_num);
    if (is_force_stop || __this->play_one) {
        __this->total_num = 0;
        __this->play_one = 0;
        __this->play_next_mode = NULL;
        return 0;
    }

    if (__this->total_num <= MUSIC_PLAY_NUM) { //完整播放完成，自动播放30首关闭
        if (__this->new_list && __this->play_loop) {
            if (!__this->last_list) {
                __this->last_list = __this->new_list;
            }
            struct net_music_list *last_list = (struct net_music_list *)__this->last_list;
            __this->timeout_id = sys_timeout_add_to_task("sys_timer", last_list->music_url, net_music_dec_file, timeout + 500);//1秒后再创建，否则会在发断开消息到下次音乐请求
            net_music_name_save(last_list->music_name);
        } else if (__this->play_loop && music_url[0] != 0) {
            //net_music_dec_file(music_url);
            __this->timeout_id = sys_timeout_add_to_task("sys_timer", music_url, net_music_dec_file, timeout + 500);
            net_music_name_save(music_name);
        } else if (!system_is_alarm_wakeup()) {
            printf("->play next music: %d\n", __this->play_loop);
            __this->timeout_id = sys_timeout_add_to_task("sys_timer", __this->play_next_mode, net_music_play_next, timeout + 500);
        } else {
            goto _stop;
        }
        __this->total_num++;
    } else {
_stop:
        __this->total_num = 0;
        __this->play_one = 0;
        __this->play_next_mode = NULL;
        music_buf_play_set_stop();
    }
    return 0;
}
#else
//播放结束
static int net_music_dec_end(int is_force_stop)
{
    union ai_req req = {0};
    __this->request_status = false;
    int timeout = (__this->total_time - __this->play_time) * 1000;
    timeout = timeout > 0 ? timeout : 0;

    printf("->net_music play num = %d \n", __this->total_num);
    os_mutex_pend(&__this->mutex, 6000);
    if (is_force_stop || __this->play_one) {
        goto _stop;
    }
    __this->spectrum_fft_hdl = NULL;
    printf("->total_time = %d,play_time = %d\n", __this->total_time, __this->play_time);
    if ((__this->total_time - __this->play_time) <= 2 && __this->total_num <= MUSIC_PLAY_NUM) { //完整播放完成，自动播放50首关闭
        __this->play_time = 0;
        if (__this->new_list && __this->play_loop) {
            if (!__this->last_list) {
                __this->last_list = __this->new_list;
            }
            struct net_music_list *last_list = (struct net_music_list *)__this->last_list;
            __this->timeout_id = sys_timeout_add_to_task("sys_timer", last_list->music_url, net_music_dec_file, timeout + 500);//1秒后再创建，否则会在发断开消息到下次音乐请求
            net_music_name_save(last_list->music_name);
        } else if (__this->play_loop && music_url[0] != 0) {
            //net_music_dec_file(music_url);
            __this->timeout_id = sys_timeout_add_to_task("sys_timer", music_url, net_music_dec_file, timeout + 500);
            net_music_name_save(music_name);
        } else if (!system_is_alarm_wakeup()) {
            printf("->play next music: %d\n", __this->play_loop);
            __this->timeout_id = sys_timeout_add_to_task("sys_timer", __this->play_next_mode, net_music_play_next, timeout + 500);
        } else {
            goto _stop;
        }
        __this->total_num++;
    } else if ((__this->total_time - __this->play_time) > 2 || __this->total_num > MUSIC_PLAY_NUM) {
_stop:
        __this->total_num = 0;
        __this->play_time = 0;
        __this->play_one = 0;
        __this->play_next_mode = NULL;
        music_buf_play_set_stop();
        net_music_dec_stop();
        net_music_play_set_stop();
    }
    os_mutex_post(&__this->mutex);
    if (!__this->ai_server) {
        return 0;
    }

    /* 歌曲播放完成，发送此命令后ai平台会发送新的URL */
    req.evt.event   = AI_EVENT_MEDIA_END;
    req.evt.ai_name     = __this->ai_name;
    return ai_server_request(__this->ai_server, AI_REQ_EVENT, &req);
}

//数据缓冲已完成，开始解码
static int __net_music_dec_file(void *file)
{
    int err;
    union ai_req r = {0};
    union audio_req req = {0};

    extern int sys_connect_net_success(void);
    if (!sys_connect_net_success()) {
        return -1;
    }
    __this->request_status = false;
    __this->wait_download = 0;

    if (__this->download_ready < 0) {
        /* 网络下载失败 */
        goto __err;
    }

    //获取网络资源的格式
    req.dec.dec_type = net_download_get_media_type(file);
    if (req.dec.dec_type == NULL) {
        goto __err;
    }
    log_i("url_file_type: %s\n", req.dec.dec_type);

    net_download_set_read_timeout(file, 5000);

    sys_volume_read(&__this->volume);

    req.dec.cmd             = AUDIO_DEC_OPEN;
    req.dec.volume          = GET_SET_VOLUME(__this->volume);
    req.dec.output_buf_len  = 6 * 1024;
    req.dec.file            = (FILE *)file;
    req.dec.channel         = 0;
    req.dec.sample_rate     = 0;
    req.dec.priority        = 1;
    req.dec.vfs_ops         = &net_audio_dec_vfs_ops;
    req.dec.sample_source   = "dac";
    req.dec.force_sr        = FORCE_DAC_SAMPLE_TRATE;//强制使用采样率
    /* req.dec.bp              = &__this->dec_bp; //恢复断点 */
#if 0   //变声变调功能
    req.dec.speedV = 80; // >80是变快，<80是变慢，建议范围：30到130
    req.dec.pitchV = 32768; // >32768是音调变高，<32768音调变低，建议范围20000到50000
    req.dec.attr = AUDIO_ATTR_PS_EN;
#endif

#ifdef NET_MUSIC_CONFIG_DEC_ANALOG_VOLUME_ENABLE
    req.dec.attr |= AUDIO_ATTR_FADE_INOUT;
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
#ifdef CONFIG_SPECTRUM_FFT_EFFECT_ENABLE
    req.dec.effect |= AUDIO_EFFECT_SPECTRUM_FFT;
#endif

    err = server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    if (err) {
        goto __err;
    }

    __this->play_time = req.dec.play_time;
    __this->total_time = req.dec.total_time;

    net_download_set_read_timeout(file, 0);

    req.dec.cmd = AUDIO_DEC_START;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

#ifdef CONFIG_SPECTRUM_FFT_EFFECT_ENABLE
    memset(&req, 0, sizeof(req));
    req.dec.cmd = AUDIO_DEC_GET_EFFECT_HANDLE;
    req.dec.effect = AUDIO_EFFECT_SPECTRUM_FFT;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    __this->spectrum_fft_hdl = req.dec.get_hdl;
#endif

    if (__this->play_status == AUDIO_DEC_PP) {
        req.dec.cmd = AUDIO_DEC_PAUSE;
        server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
        __this->play_status = AUDIO_DEC_PAUSE;
        net_download_set_pp(file, 1);
        printf("-> play_status = AUDIO_DEC_PP\n");
    } else {
        net_download_set_pp(file, 0);
    }
#ifdef AI_UART_CMD_CTROL_ENABLE
    ai_uart_cmd_data_push(AI_UART_CMD_NET_MUSIC_PLAY, NULL, 0);
#endif
#ifdef AT_UART_CMD_ENABLE
    at_uart_cmd_send(AI_UART_CMD_NET_MUSIC_PLAY, NULL);
#endif
#ifdef CONFIG_UI_PLAY_EMOJI
    play_face_emoji(AI_UART_CMD_NET_MUSIC_PLAY);
#endif
#if (defined CONFIG_UI_ENABLE && defined CONFIG_LVGL_UI_ENABLE)
    // 更新UI界面的总时长显示
    if (__this->total_time > 0) {
        lv_demo_music_update_total_time(__this->total_time);
    }
    if (!lv_demo_is_ring_page()) {
        void *websockets_music_url_type(char clear);
        lv_demo_switch_to_music_page(__this->play_next_mode);
    }
#endif
    return 0;

__err:
    printf("play_net_music_faild\n");

    net_download_buf_inactive(file);

    req.dec.cmd = AUDIO_DEC_STOP;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    net_download_close(file);
    __this->net_file = NULL;

    if (__this->ai_server) {
        r.evt.event   = AI_EVENT_MEDIA_END;
        r.evt.ai_name   = __this->ai_name;
        ai_server_request(__this->ai_server, AI_REQ_EVENT, &r);
    }
    __this->play_status = AUDIO_DEC_STOP;
    return -1;
}
#endif

int net_music_dec_file(const char *url)
{
    int err;
    struct net_download_parm parm = {0};

    if (__this->timeout_id) {
        sys_timeout_del(__this->timeout_id);
        __this->timeout_id = 0;
    }

    if (!url || !sys_connect_net_success() || __this->wait_download) {
        return -1;
    }
    if (!strcmp(music_url, url) && net_music_play_start_status() && !__this->play_loop) {
        return 0;
    }
    strcpy(music_url, url);
    __this->total_time = __this->play_time = 0;
    __this->request_status = true;
    sys_timeout_add_to_task("sys_timer", NULL, net_music_request_status_clear, 5000);

#ifndef NET_MUSIC_MEM_SMALL
    net_music_mode_init();
    net_music_dec_stop();
#endif

    if (!__this->head.next) {//初始化
        INIT_LIST_HEAD(&__this->head);
        __this->list_num = 0;
    }
    if (__this->head.next) {
        struct net_music_list *p;
        char find = 0;
        list_for_each_entry(p, &__this->head, entry) {//遍历所有链表
            if (p && p->music_url[0] && !strcmp(music_url, p->music_url)) {//当前存在则不添加到链表
                find = true;
                break;
            }
        }
        if (!find) {
            struct net_music_list *new_list = NULL;
            if (__this->list_num >= NET_MUSIC_MAX_NUM) {//超过NUM
                __this->list_num--;
                struct net_music_list *find_list = list_first_entry(&__this->head, struct net_music_list, entry);
                if (find_list) {
                    if (__this->new_list == find_list) {
                        __this->new_list = NULL;
                    }
                    if (__this->last_list == find_list) {
                        __this->last_list = NULL;
                    }
                    list_del(&find_list->entry);
                    //free(find_list);
                    new_list = find_list;//使用就内存不释放
                }
            }
            if (!new_list) {
                new_list = malloc(sizeof(struct net_music_list));
            }
            if (new_list) {
                memset(new_list, 0, sizeof(struct net_music_list));
                strncpy(new_list->music_url, music_url, sizeof(new_list->music_url));
                strncpy(new_list->music_name, music_name, sizeof(new_list->music_name));
                list_add_tail(&new_list->entry, &__this->head);//list_add_tail先进先出，list_add:先进后出
                __this->new_list = new_list;
                __this->list_num++;
                printf("play : %s\n", new_list->music_name);
            }
        } else {
            printf("play : %s\n", p->music_name);
        }
    }

#ifdef NET_MUSIC_MEM_SMALL
    void dec_server_event_handler(int event, int time);
    //printf("http_music_open url = %s\n", url);
    http_music_play_end_callback(dec_server_event_handler);
#ifdef CONFIG_NO_SDRAM_ENABLE
    int http_buf_size = 8 * 1024;
#else
#if __SDRAM_SIZE__ == (8 * 1024 * 1024)
    int http_buf_size          = 128 * 1024;
#else
    int http_buf_size          = 32 * 1024;
#endif
#endif // CONFIG_NO_SDRAM_ENABLE
    return http_music_play_request(url, http_buf_size);
#else
    //printf("net_download_open url = %s\n", url);
    __this->play_status = AUDIO_DEC_START;
    parm.url                = url;
    //网络缓冲buf大小
#ifdef CONFIG_NO_SDRAM_ENABLE
    parm.cbuf_size          = 32 * 1024;
#else
#if __SDRAM_SIZE__ == (8 * 1024 * 1024)
    parm.cbuf_size          = 500 * 1024;
#else
    parm.cbuf_size          = 200 * 1024;
#endif
#endif
    //设置网络下载超时
    parm.timeout_millsec    = 10000;
#ifdef CONFIG_DOWNLOAD_SAVE_FILE
    if (storage_device_ready()) {
        parm.save_file          = 1;
        parm.file_dir           = NULL;
    }
#endif
    parm.seek_threshold     = 1024 * 200;   //用户可适当调整
    /* parm.seek_low_range     = __this->dec_bp.fptr;    //恢复断点时设置网络的开始下载地址 */

    err = net_download_open(&__this->net_file, &parm);
    if (err) {
        printf("net_download_open: err = %d\n", err);
        net_music_play_set_stop();
        __this->play_status = AUDIO_DEC_STOP;
        return err;
    }
    /*异步等待网络下载ready，防止网络阻塞导致app_core卡住 */
    __this->wait_download = wait_completion(__net_download_ready, (int (*)(void *))__net_music_dec_file, __this->net_file, NULL);
#endif
    return 0;
}
int net_music_play_last(void)
{
    int ret = -1;
    if (__this->request_status) {
        return 0;
    }
    __this->request_status = true;
    sys_timeout_add_to_task("sys_timer", NULL, net_music_request_status_clear, 5000);
    if (!__this->new_list) {
        ret = net_music_play_next(net_music_play_type_get());
    } else {
        if (__this->head.next && __this->new_list) {
            if (!__this->last_list) {//两首以上
                __this->last_list = __this->new_list;
            }
            if (__this->last_list) {
                if (__this->list_num == 1) {//只有1首
                    __this->last_list = NULL;
                    ret = net_music_play_next(NULL);
                    goto exit;
                }
                struct list_head *list = (struct list_head *)__this->last_list;
                if (list && list->prev) {
                    __this->last_list = list->prev;
                }
                if (__this->last_list == &__this->head || list == NULL) {//循环到头
                    __this->last_list = NULL;
                    ret = net_music_play_next(net_music_play_type_get());
                    goto exit;
                }
            }
            struct net_music_list *last_list = (struct net_music_list *)__this->last_list;
            sys_timeout_add_to_task("sys_timer", last_list->music_url, net_music_dec_file, 1000);//1秒后再创建，否则会在发断开消息到下次音乐请求
            net_music_name_save(last_list->music_name);
        }
    }
exit:
    if (ret) {
        __this->request_status = false;
    }
    return 0;
}
int net_music_play_last_check(void)
{
#ifndef CONFIG_UI_ENABLE
    __this->play_loop = 0;
#endif
    if (__this->list_num > 1) {
        return 1;
    }
    return 0;
}
int net_music_play_loop(void)
{
    if (!__this->play_loop) {
        __this->play_loop = 1;
    }
    return 0;
}
int net_music_play_one_url(char *url)
{
    int ret = 0;
    if (url) {
        ret = net_music_dec_file(url);
        __this->play_one = ret ? false : true;
    }
    return ret;
}

int net_music_play_loop_fore_set(char loop)//UI触摸控制使用的API
{
    __this->play_loop = loop ? 2 : 0;
    return 0;
}
int net_music_play_loop_clear(void)
{
    if (__this->play_loop == 1) {
        __this->play_loop = 0;
    }
    return 0;
}
#ifndef NET_MUSIC_MEM_SMALL
//切换上一首或下一首
static int net_music_dec_switch_file(int fsel_mode)
{
    union ai_req req = {0};

    if (!__this->ai_server) {
        return 0;
    }

    log_i("net_music_dec_switch_file\n");

    if (!strcmp(__this->ai_name, "dlna")) {
        return 0;
    }

    net_music_dec_stop();

    if (fsel_mode == FSEL_NEXT_FILE) {
        req.evt.event = AI_EVENT_NEXT_SONG;
    } else if (fsel_mode == FSEL_PREV_FILE) {
        req.evt.event = AI_EVENT_PREVIOUS_SONG;
    } else {
        return 0;
    }

    req.evt.ai_name = __this->ai_name;
    return ai_server_request(__this->ai_server, AI_REQ_EVENT, &req);
}

//获取断点数据
static int net_music_get_dec_breakpoint(struct audio_dec_breakpoint *bp)
{
    int err;
    union audio_req r = {0};

    bp->len = 0;
    r.dec.bp = bp;
    r.dec.cmd = AUDIO_DEC_GET_BREAKPOINT;

    if (bp->data) {
        free(bp->data);
        bp->data = NULL;
    }

    err = server_request(__this->dec_server, AUDIO_REQ_DEC, &r);
    if (err) {
        return err;
    }

    if (r.dec.status == AUDIO_DEC_STOP) {
        bp->len = 0;
        free(bp->data);
        bp->data = NULL;
        return -1;
    }
    /* put_buf(bp->data, bp->len); */

    return 0;
}
#endif

//设置音量大小
int net_music_set_dec_volume_step(int step)
{
    union audio_req req = {0};
    union ai_req ai = {0};
#ifdef NET_MUSIC_MEM_SMALL
    return http_music_play_set_volume_step(step);
    //return music_buf_play_set_volume_step(step);
#else
    if (!__this->net_file) {
        return -1;
    }

    sys_volume_read(&__this->volume);
    __this->volume = sys_volume_chack(__this->volume + step);

    printf("->set_dec_volume: %d\n", __this->volume);

    req.dec.cmd     = AUDIO_DEC_SET_VOLUME;
    req.dec.volume  = GET_SET_VOLUME(__this->volume);
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    if (__this->ai_server) {
        ai.evt.event       = AI_EVENT_VOLUME_CHANGE;
        ai.evt.arg         = __this->volume;
        ai.evt.ai_name     = __this->ai_name;
        ai_server_request(__this->ai_server, AI_REQ_EVENT, &ai);
    }
    sys_volume_write(&__this->volume);
#endif
    return 0;
}
//设置音量大小
int net_music_set_dec_volume(int volume)
{
    union audio_req req = {0};
    union ai_req ai = {0};
#ifdef NET_MUSIC_MEM_SMALL
    return http_music_play_set_volume(volume);
    //return music_buf_play_set_volume(volume);
#else
    if (!__this->net_file) {
        return -1;
    }

    __this->volume = sys_volume_chack(volume);

    printf("->set_dec_volume: %d\n", __this->volume);

    sys_volume_write(&__this->volume);

    req.dec.cmd     = AUDIO_DEC_SET_VOLUME;
    req.dec.volume  = GET_SET_VOLUME(__this->volume);
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    if (__this->ai_server) {
        ai.evt.event       = AI_EVENT_VOLUME_CHANGE;
        ai.evt.arg         = __this->volume;
        ai.evt.ai_name     = __this->ai_name;
        ai_server_request(__this->ai_server, AI_REQ_EVENT, &ai);
    }
#endif
    return 0;
}
#ifndef NET_MUSIC_MEM_SMALL
//获取解码器状态
static int net_music_get_dec_status(void)
{
    union audio_req req = {0};

    req.dec.cmd     = AUDIO_DEC_GET_STATUS;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    return req.dec.status;
}
#endif
short *net_music_get_audio_fft(int *len)
{
#ifdef NET_MUSIC_MEM_SMALL
    return NULL;
#else
#ifdef CONFIG_SPECTRUM_FFT_EFFECT_ENABLE
    if (!os_mutex_valid(&__this->mutex) || !__this->spectrum_fft_hdl || __this->play_status == AUDIO_DEC_STOP ||
        __this->play_status == AUDIO_DEC_PAUSE) {
        return NULL;
    }
    os_mutex_pend(&__this->mutex, 6000);
    if (!__this->spectrum_fft_hdl || __this->play_status == AUDIO_DEC_STOP ||
        __this->play_status == AUDIO_DEC_PAUSE) {
        os_mutex_post(&__this->mutex);
        return NULL;
    }
    if (__this->spectrum_fft_hdl) {
        short *db_data = audio_spectrum_fft_get_val(__this->spectrum_fft_hdl);//获取存储频谱值得地址
        if (db_data) {
            int n = audio_spectrum_fft_get_num(__this->spectrum_fft_hdl);
            *len = n;
            os_mutex_post(&__this->mutex);
            return db_data;
//                put_buf(db_data, n * 2);
//                for (int i = 0; i < audio_spectrum_fft_get_num(__this->spectrum_fft_hdl); i++) {
//                    //输出db_num个 db值
////                    printf("db_data db[%d] %d\n", i, db_data[i]); */
//                }
        }
    }
    os_mutex_post(&__this->mutex);
#endif
#endif
    return NULL;
}
#ifdef NET_MUSIC_MEM_SMALL
static void dec_server_event_handler(int event, int time)
{
    switch (event) {
    case HTTP_REQ_NO_FILE:
        printf("net_music: HTTP_REQ_NO_FILE ERR\n");
    case AUDIO_SERVER_EVENT_ERR:
        printf("net_music: AUDIO_SERVER_EVENT_ERR\n");
        net_music_play_set_stop();
    case AUDIO_SERVER_EVENT_END:
        printf("net_music: AUDIO_SERVER_EVENT_END\n");
        net_music_dec_end(0);
#if (defined CONFIG_UI_ENABLE && defined CONFIG_LVGL_UI_ENABLE)
        // 网络音乐播放结束，隐藏音乐页面
        lv_demo_music_play_pause_button(1);//暂停图标显示
#endif
        //net_music_play_set_stop();
        break;
    case AUDIO_SERVER_EVENT_CURR_TIME:
        ;
        char buf[32];
        sprintf(buf, "%02d:%02d ", (int)time / 60, (int)time % 60);
        __this->play_time = time;
        printf("play_time : %s\n", buf);
#if (defined CONFIG_UI_ENABLE && defined CONFIG_LVGL_UI_ENABLE)
        // 更新当前播放时间到UI界面
        if (__this->play_time > 0) {
            lv_demo_music_update_current_time(__this->play_time);
        }
        lv_demo_music_play_pause_button(0);
#endif
        break;
    }
}

#else
//快进快退,单位是秒,暂时只支持MP3格式
static int net_music_dec_seek(int seek_step)
{
    int err;
    union audio_req r = {0};

    if (0 == seek_step) {
        return 0;
    }

    if (__this->total_time != 0 && __this->total_time != -1) {
        if (__this->play_time + seek_step <= 0 || __this->play_time + seek_step >= __this->total_time) {
            printf("local music seek out of range\n");
            return -1;
        }
    }

    if (seek_step > 0) {
        r.dec.cmd = AUDIO_DEC_FF;
        r.dec.ff_fr_step = seek_step;
    } else {
        r.dec.cmd = AUDIO_DEC_FR;
        r.dec.ff_fr_step = -seek_step;
    }

    log_i("net music seek step : %d\n", seek_step);

    return server_request(__this->dec_server, AUDIO_REQ_DEC, &r);
}

static void dec_server_event_handler(void *priv, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
        printf("net_music: AUDIO_SERVER_EVENT_ERR\n");
        net_music_play_set_stop();
    case AUDIO_SERVER_EVENT_END:
        printf("net_music: AUDIO_SERVER_EVENT_END\n");
        net_music_dec_end(0);
#if (defined CONFIG_UI_ENABLE && defined CONFIG_LVGL_UI_ENABLE)
        // 网络音乐播放结束，隐藏音乐页面
        lv_demo_music_play_pause_button(1);//暂停图标显示
#endif
        //net_music_play_set_stop();
        break;
    case AUDIO_SERVER_EVENT_CURR_TIME:
        ;
        char buf[32];
        sprintf(buf, "%02d:%02d ", (int)argv[1] / 60, (int)argv[1] % 60);
        //puts(buf);
        printf("play_time: %d\n", argv[1]);
        __this->play_time = argv[1];
#if (defined CONFIG_UI_ENABLE && defined CONFIG_LVGL_UI_ENABLE)
        // 更新当前播放时间到UI界面
        if (__this->play_time > 0) {
            lv_demo_music_update_current_time(__this->play_time);
        }
        lv_demo_music_play_pause_button(0);
#endif
        break;
    }
}

//第三方平台的事件通知回调
static void ai_server_event_handler(void *priv, int argc, int *argv)
{
    if (!__this->ai_server) {
        switch (argv[0]) {
        case AI_SERVER_EVENT_URL:
            free((void *)argv[1]);
            break;
        }
        return;
    }

    switch (argv[0]) {
    case AI_SERVER_EVENT_CONNECTED:
        break;
    case AI_SERVER_EVENT_DISCONNECTED:
        break;
    case AI_SERVER_EVENT_URL:
    case AI_SERVER_EVENT_URL_TTS:
    case AI_SERVER_EVENT_URL_MEDIA:
        __this->total_time = 0; //清空上一首歌的信息
        __this->ai_name = (const char *)argv[2];
        const char *url = (const char *)argv[1];
        free(__this->url);
        __this->url = malloc(strlen(url) + 1);
        if (__this->url) {
            strcpy(__this->url, url);
        }
        net_music_dec_file(url);
        break;
    case AI_SERVER_EVENT_CONTINUE:
        if (AUDIO_DEC_PAUSE == net_music_get_dec_status()) {
            net_music_dec_play_pause(0);
        }
        break;
    case AI_SERVER_EVENT_PAUSE:
        if (AUDIO_DEC_START == net_music_get_dec_status()) {
            net_music_dec_play_pause(0);
        }
        break;
    case AI_SERVER_EVENT_STOP:
        net_music_dec_stop();
        break;
    case AI_SERVER_EVENT_RESUME_PLAY:
        break;
    case AI_SERVER_EVENT_SEEK:
        net_music_dec_seek(argv[1] - __this->play_time);
        break;
    case AI_SERVER_EVENT_VOLUME_CHANGE:
        net_music_set_dec_volume(argv[1] - __this->volume);
        break;
    case AI_SERVER_EVENT_SET_PLAY_TIME:
        __this->play_time = argv[1];
        break;
    default:
        break;
    }
}
#endif
static int net_music_mode_init(void)
{
#ifndef NET_MUSIC_MEM_SMALL
    log_i("net_music_play_main\n");
    if (__this->dec_server) {
        return 0;
    }
    memset(__this, 0, sizeof(struct net_music_hdl));

    os_mutex_create(&__this->mutex);

    sys_volume_read(&__this->volume);

    __this->ai_name = "unknown";

    __this->dec_server = server_open("audio_server", "dec");
    if (!__this->dec_server) {
        return -1;
    }
    server_register_event_handler_to_task(__this->dec_server, NULL, dec_server_event_handler, "app_core");

//    if (lwip_dhcp_bound()) {
//        __this->ai_server = server_open("ai_server", NULL);
//        if (__this->ai_server) {
//            union ai_req req = {0};
//            server_register_event_handler_to_task(__this->ai_server, NULL, ai_server_event_handler, "app_core");
//            ai_server_request(__this->ai_server, AI_REQ_CONNECT, &req);
//        }
//    }
#else
    memset(__this, 0, sizeof(struct net_music_hdl));
#endif
    return 0;
}

static void net_music_mode_exit(void)
{
#ifndef NET_MUSIC_MEM_SMALL
    if (__this->dec_server) {
        net_music_dec_stop();
        server_close(__this->dec_server);
        __this->dec_server = NULL;
        if (__this->ai_server) {
            server_close(__this->ai_server);
            __this->ai_server = NULL;
        }
        free(__this->url);
        __this->url = NULL;
    }
    //os_mutex_del(&__this->mutex, 0);
#endif
    memset(__this, 0, sizeof(struct net_music_hdl));
}

static int net_music_key_click(struct key_event *key)
{
    int ret = true;

    switch (key->value) {
    case KEY_OK:
        net_music_play_pause(1);
        break;
    case KEY_VOLUME_DEC:
        net_music_set_dec_volume(-VOLUME_STEP);
        break;
    case KEY_VOLUME_INC:
        net_music_set_dec_volume(VOLUME_STEP);
        break;
    default:
        ret = false;
        break;
    }

    return ret;
}

static int net_music_key_long(struct key_event *key)
{
    switch (key->value) {
    case KEY_OK:
        /* net_music_dec_file(""); */
        break;
    case KEY_VOLUME_DEC:
//        net_music_dec_switch_file(FSEL_PREV_FILE);
        break;
    case KEY_VOLUME_INC:
//        net_music_dec_switch_file(FSEL_NEXT_FILE);
        break;
    case KEY_MODE:
//        net_music_get_dec_breakpoint(&__this->dec_bp);
        break;
    default:
        break;
    }

    return true;
}

static int net_music_key_event_handler(struct key_event *key)
{
    switch (key->action) {
    case KEY_EVENT_CLICK:
        return net_music_key_click(key);
    case KEY_EVENT_LONG:
        return net_music_key_long(key);
    default:
        break;
    }

    return true;
}

static int net_music_net_event_handler(struct net_event *event)
{
    switch (event->event) {
    case NET_EVENT_CONNECTED:
//        if (!__this->ai_server) {
//            __this->ai_server = server_open("ai_server", NULL);
//            if (__this->ai_server) {
//                union ai_req req = {0};
//                server_register_event_handler_to_task(__this->ai_server, NULL, ai_server_event_handler, "app_core");
//                ai_server_request(__this->ai_server, AI_REQ_CONNECT, &req);
//            }
//        }
        break;
    default:
        break;
    }

    return false;
}

static int net_music_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        return net_music_key_event_handler((struct key_event *)event->payload);
    case SYS_NET_EVENT:
        return net_music_net_event_handler((struct net_event *)event->payload);
    default:
        return false;
    }
}

static int net_music_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;
    case APP_STA_START:
        net_music_mode_init();
        break;
    case APP_STA_PAUSE:
        break;
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
        net_music_mode_exit();
        break;
    case APP_STA_DESTROY:
        break;
    }

    return 0;
}

static const struct application_operation net_music_ops = {
    .state_machine  = net_music_state_machine,
    .event_handler  = net_music_event_handler,
};

REGISTER_APPLICATION(net_music) = {
    .name   = "net_music",
    .ops    = &net_music_ops,
    .state  = APP_STA_DESTROY,
};

#endif
