#include "init.h"
#include "server/audio_server.h"
#include "server/server_core.h"
#include "system/app_core.h"
#include "generic/circular_buf.h"
#include "os/os_api.h"
#include "event/bt_event.h"
#include "event/device_event.h"
#include "btstack/avctp_user.h"
#include "asm/ladc.h"
#include "asm/gpio.h"
#include "app_config.h"
#include "syscfg/syscfg_id.h"
#include "event/key_event.h"
#include "storage_device.h"
#include "action.h"
#include "fs/fs.h"
#include <time.h>
#include "system/timer.h"
#include "media/spectrum/SpectrumShow_api.h"
#include "volume.h"
#include "ai_uart_ctrol.h"

#ifdef CONFIG_AUX_MUSIC_MODE_ENABLE
extern void key_vad_pcm_send_set_status(char start, char noice_not);
#ifdef CONFIG_LTE_PHY_ENABLE
extern u8 IPV4_ADDR_CONFLICT_DETECT;
#endif

struct aux_hdl {
    FILE *fp;
    struct server *enc_server;
    struct server *dec_server;
    void *cache_buf;
    cbuffer_t save_cbuf;
    OS_SEM w_sem;
    OS_SEM r_sem;
    volatile u8 run_flag;
    u8 volume;
    u8 gain;
    u8 channel;
    u8 direct;
    u8 init;
    u8 aux_det;
    u8 aux_det_last;
    u8 aux_det_ok;
    u16 last_key;
    char pause;
    const char *sample_source;
    int sample_rate;
    int aux_time_1s_drop;
    OS_MUTEX mutex;
    void *spectrum_fft_hdl;
};

static struct aux_hdl aux_handler;

#define __this (&aux_handler)

//AUDIO ADC支持的采样率
static const u16 sample_rate_table[] = {
    8000,
    11025,
    12000,
    16000,
    22050,
    24000,
    32000,
    44100,
    48000,
};
#define AUX_SAMPLE_RATE                 48000 //AUX采样率最高48K

#if (defined TCFG_AUX_DET_PORT && defined CONFIG_AUX_AUTO_ENABLE)
#if (TCFG_AUX_DET_PORT != 0xff)
static int aux_det_scan(void)
{
    struct device_event event = {0};
    int aux_det_val = gpio_read(TCFG_AUX_DET_PORT);

    if (aux_det_val && !__this->aux_det_last) {
        if (++__this->aux_det > 5) { //拔出
            puts("-> AUX EVENT OUT\n");
            __this->aux_det_ok = 0;
            __this->aux_det_last = 1;
            __this->aux_det = 0;
            event.event = DEVICE_EVENT_OUT;
            device_event_notify(DEVICE_EVENT_FROM_LINEIN, &event);
        }
    } else if (!aux_det_val && __this->aux_det_last) {
        if (++__this->aux_det >= 10) { //插入
            puts("-> AUX EVENT IN\n");
            __this->aux_det_ok = true;
            __this->aux_det_last = 0;
            __this->aux_det = 0;
            event.event = DEVICE_EVENT_IN;
            device_event_notify(DEVICE_EVENT_FROM_LINEIN, &event);
        }
    } else if (__this->aux_det) {
        __this->aux_det = 0;
    }
}

int aux_det_is_ok(void)
{
    return __this->aux_det_ok;
}

static int aux_det_scan_init(void)
{
    __this->aux_det = 0;
    __this->aux_det_last = 1;//默认上拉高电平1
    gpio_direction_input(TCFG_AUX_DET_PORT);
    gpio_set_pull_down(TCFG_AUX_DET_PORT, 0);
    gpio_set_pull_up(TCFG_AUX_DET_PORT, 1);
    gpio_set_die(TCFG_AUX_DET_PORT, 1);
    sys_timer_add_to_task("sys_timer", NULL, aux_det_scan, 50);
    return 0;
}
late_initcall(aux_det_scan_init);
#endif
#endif

//#ifdef CONFIG_SPECTRUM_FFT_EFFECT_ENABLE
//static void aux_spectrum_fft_show(void *p)
//{
//    if (__this->work_buf) {
//        short *db_data = getSpectrumValue(__this->work_buf);
//        int num = getSpectrumNum(__this->work_buf);
//        if (db_data && num > 0) {
//            for (int i = 0; i < num; i++) {
//                //输出db_num个 db值
////                printf("db_data db[%d] %d\n", i, db_data[i]);
//            }
//        }
//    }
//}
//#endif

//编码器输出PCM数据
static int aux_vfs_fwrite(void *file, void *data, u32 len)
{
    if (__this->aux_time_1s_drop) {
        if (timer_get_ms() - __this->aux_time_1s_drop < 1000) {
            return len;
        }
    }
    cbuffer_t *cbuf = (cbuffer_t *)file;
    if (0 == cbuf_write(cbuf, data, len)) {
        //上层buf写不进去时清空一下，避免出现声音滞后的情况
        cbuf_clear(cbuf);
        putchar('#');
    }
//    os_sem_set(&__this->r_sem, 0);
    os_sem_post(&__this->r_sem);

//#ifdef CONFIG_SPECTRUM_FFT_EFFECT_ENABLE
//    u32 in_remain = len, tlen = 0;
//
//    while (in_remain) {
//        if (__this->offset < __this->out_buf_size) {
//            tlen = __this->out_buf_size - __this->offset;
//            if (tlen > in_remain) {
//                tlen = in_remain;
//            }
//            memcpy((u8 *)__this->out_buf + __this->offset, (u8 *)data + (len - in_remain), tlen);
//            __this->offset += tlen;
//            in_remain -= tlen;
//            if (in_remain && (__this->offset != __this->out_buf_size)) {
//                continue;
//            }
//        }
//        if (__this->offset == __this->out_buf_size) {
//            __this->offset = 0;
//            SpectrumShowRun(__this->work_buf, __this->out_buf, 512);
//        }
//    }
//#endif

    //此回调返回0录音就会自动停止
    return len;
}

//解码器读取PCM数据
static int aux_vfs_fread(void *file, void *data, u32 len)
{
    cbuffer_t *cbuf = (cbuffer_t *)file;
    u32 rlen;

    do {
        rlen = cbuf_get_data_size(cbuf);
        rlen = rlen > len ? len : rlen;
        if (cbuf_read(cbuf, data, rlen) > 0) {
            len = rlen;
            break;
        }
        //此处等待信号量是为了防止解码器因为读不到数而一直空转
        os_sem_pend(&__this->r_sem, 0);
        if (!__this->run_flag) {
            return 0;
        }
    } while (__this->run_flag);

    //返回成功读取的字节数
    return len;
}

static int aux_vfs_fclose(void *file)
{
    return 0;
}

static int aux_vfs_flen(void *file)
{
    return 0;
}

static const struct audio_vfs_ops aux_vfs_ops = {
    .fwrite = aux_vfs_fwrite,
    .fread  = aux_vfs_fread,
    .fclose = aux_vfs_fclose,
    .flen   = aux_vfs_flen,
};

static int aux_close(void)
{
    union audio_req req = {0};

    if (!__this->run_flag) {
        return 0;
    }
    os_mutex_pend(&__this->mutex, 6000);
    __this->spectrum_fft_hdl = NULL;
    printf("----------recorder close----------\n");

    __this->run_flag = 0;
    __this->aux_time_1s_drop = 0;

    os_sem_post(&__this->w_sem);
    os_sem_post(&__this->r_sem);

    if (__this->enc_server) {
        req.enc.cmd = AUDIO_ENC_CLOSE;
        server_request(__this->enc_server, AUDIO_REQ_ENC, &req);
    }

    if (__this->dec_server) {
        req.dec.cmd = AUDIO_DEC_STOP;
        server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    }

    if (__this->cache_buf) {
        free(__this->cache_buf);
        __this->cache_buf = NULL;
    }

    if (__this->fp) {
        fclose(__this->fp);
        __this->fp = NULL;
    }
    os_mutex_post(&__this->mutex);
//#ifdef CONFIG_SPECTRUM_FFT_EFFECT_ENABLE
//    if (__this->work_buf) {
//        free(__this->work_buf);
//        __this->work_buf = NULL;
//    }
//    if (__this->out_buf) {
//        free(__this->out_buf);
//        __this->out_buf = NULL;
//    }
//    if (__this->show_timer_id) {
//        sys_timeout_del(__this->show_timer_id);
//        __this->show_timer_id = 0;
//    }
//#endif

    return 0;
}

static void enc_server_event_handler(void *priv, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
    case AUDIO_SERVER_EVENT_END:
        aux_close();
        break;
    case AUDIO_SERVER_EVENT_SPEAK_START:
        log_i("speak start ! \n");
        break;
    case AUDIO_SERVER_EVENT_SPEAK_STOP:
        log_i("speak stop ! \n");
        break;
    default:
        break;
    }
}

//将MIC的数字信号采集后推到DAC播放
//注意：如果需要播放两路MIC，DAC分别对应的是DACL和DACR，要留意芯片封装是否有DACR引脚出来，
//      而且要使能DAC的双通道输出，DAC如果采用差分输出方式也只会听到第一路MIC的声音
static int aux_play_to_dac(int sample_rate, u8 channel)
{
    int err;
    union audio_req req = {0};

    printf("----------aux_play_to_dac----------\n");

    if (channel > 2) {
        channel = 2;
    }
    __this->cache_buf = malloc(sample_rate * channel * 10); //上层缓冲buf缓冲0.5秒的数据，缓冲太大听感上会有延迟
    if (__this->cache_buf == NULL) {
        return -1;
    }
    cbuf_init(&__this->save_cbuf, __this->cache_buf, sample_rate * channel * 10);

    os_sem_create(&__this->w_sem, 0);
    os_sem_create(&__this->r_sem, 0);
    sys_volume_read(&__this->volume);

    __this->run_flag = 1;

    /****************打开解码DAC器*******************/
    req.dec.cmd             = AUDIO_DEC_OPEN;
    req.dec.volume          = GET_SET_VOLUME(__this->volume);
    req.dec.output_buf_len  = 8 * 1024;
    req.dec.channel         = channel;
    req.dec.sample_rate     = sample_rate;
    req.dec.priority        = 1;
    req.dec.vfs_ops         = &aux_vfs_ops;
    req.dec.dec_type        = "pcm";
    req.dec.sample_source   = "dac";
    req.dec.file            = (FILE *)&__this->save_cbuf;
    req.dec.attr            = AUDIO_ATTR_LR_ADD;          //左右声道数据合在一起,封装只有DACL但需要测试两个MIC时可以打开此功能
    req.dec.force_sr        = sample_rate;//强制使用采样率

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
#ifdef CONFIG_SPECTRUM_FFT_EFFECT_ENABLE
    req.dec.effect |= AUDIO_EFFECT_SPECTRUM_FFT;
#endif
    err = server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    if (err) {
        goto __err;
    }

    req.dec.cmd = AUDIO_DEC_START;
    req.dec.attr = AUDIO_ATTR_NO_WAIT_READY;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

#ifdef CONFIG_SPECTRUM_FFT_EFFECT_ENABLE
    memset(&req, 0, sizeof(req));
    req.dec.cmd = AUDIO_DEC_GET_EFFECT_HANDLE;
    req.dec.effect = AUDIO_EFFECT_SPECTRUM_FFT;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    __this->spectrum_fft_hdl = req.dec.get_hdl;
#endif

//#ifdef CONFIG_SPECTRUM_FFT_EFFECT_ENABLE
//    __this->work_buf = zalloc(getSpectrumShowBuf());
//    if (!__this->work_buf) {
//        goto __err1;
//    }
//    __this->offset = 0;
//    __this->out_buf_size = 512 * 2 * channel;
//    __this->out_buf = zalloc(__this->out_buf_size);
//    if (!__this->out_buf) {
//        free(__this->work_buf);
//        __this->work_buf = NULL;
//        goto __err1;
//    }
//
//    SpectrumShowInit(__this->work_buf, 0.9, 0.9,
//                     sample_rate, channel, channel > 1 ? 2 : 0, JL_FFT_BASE);
//
//    __this->show_timer_id = sys_timer_add(NULL, aux_spectrum_fft_show, 1000);
//#endif

    /****************打开编码器*******************/
    memset(&req, 0, sizeof(union audio_req));

    //BIT(x)用来区分上层需要获取哪个通道的数据
    if (channel == 2) {
        req.enc.channel_bit_map = TCFG_LINEIN_CHANNEL_MAP;
    } else {
        req.enc.channel_bit_map = TCFG_LINEIN_CHANNEL_MAP;
    }
    req.enc.frame_size = sample_rate / 100 * 4 * channel;   //收集够多少字节PCM数据就回调一次fwrite
    req.enc.output_buf_len = req.enc.frame_size * 6; //底层缓冲buf至少设成3倍frame_size
    req.enc.cmd = AUDIO_ENC_OPEN;
    req.enc.channel = channel;
    req.enc.volume = __this->gain;
    req.enc.sample_rate = sample_rate;
    req.enc.format = "pcm";
    req.enc.sample_source = __this->sample_source;
    req.enc.vfs_ops = &aux_vfs_ops;
    req.enc.file = (FILE *)&__this->save_cbuf;
//    if (channel == 1 && !strcmp(__this->sample_source, "mic") && (sample_rate == 8000 || sample_rate == 16000)) {
//        req.enc.use_vad = 1; //打开VAD断句功能
//        req.enc.dns_enable = 1; //打开降噪功能
//        req.enc.vad_auto_refresh = 1; //VAD自动刷新
//    }

    err = server_request(__this->enc_server, AUDIO_REQ_ENC, &req);
    if (err) {
        goto __err1;
    }
    __this->aux_time_1s_drop = timer_get_ms();
    return 0;

__err1:
    req.dec.cmd = AUDIO_DEC_STOP;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

__err:
    if (__this->cache_buf) {
        free(__this->cache_buf);
        __this->cache_buf = NULL;
    }

    __this->run_flag = 0;

    return -1;
}

//MIC或者LINEIN模拟直通到DAC，不需要软件参与
static int audio_adc_anaprintfirect_to_dac(int sample_rate, u8 channel)
{
    union audio_req req = {0};
    if (!__this->enc_server) {
        return -1;
    }

//    printf("----------audio_adc_anaprintfirect_to_dac----------\n");

    __this->run_flag = 1;

    req.enc.cmd = AUDIO_ENC_OPEN;
    req.enc.channel = channel;
    req.enc.volume = __this->gain;
    req.enc.format = "pcm";
    req.enc.sample_source = __this->sample_source;
    req.enc.sample_rate = sample_rate;
    req.enc.direct2dac = 1;
    req.enc.high_gain = 1;
    if (channel == 4) {
        req.enc.channel_bit_map = 0x0f;
    } else if (channel == 2) {
        req.enc.channel_bit_map = TCFG_LINEIN_CHANNEL_MAP;
    } else {
        req.enc.channel_bit_map = TCFG_LINEIN_CHANNEL_MAP;
    }
    __this->aux_time_1s_drop = timer_get_ms();
    return server_request(__this->enc_server, AUDIO_REQ_ENC, &req);
}

static void aux_play_pause(void)
{
    union audio_req req = {0};
    if (!__this->dec_server || !__this->enc_server) {
        return;
    }
    req.dec.cmd = AUDIO_DEC_PP;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    req.enc.cmd = AUDIO_ENC_PP;
    server_request(__this->enc_server, AUDIO_REQ_ENC, &req);

    if (__this->cache_buf) {
        cbuf_clear(&__this->save_cbuf);
    }

    if (__this->dec_server) {
        memset(&req, 0, sizeof(union audio_req));
        req.dec.cmd     = AUDIO_DEC_GET_STATUS;
        server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
        if (req.dec.status == AUDIO_ENC_PAUSE) {
            __this->pause = true;
        } else {
            __this->pause = 0;
        }
    }
}

//调整ADC的模拟增益
static int aux_enc_gain_change(int step)
{
    union audio_req req = {0};

    int gain = __this->gain + step;
    if (gain < 0) {
        gain = 0;
    } else if (gain > 100) {
        gain = 100;
    }
    if (gain == __this->gain) {
        return -1;
    }
    __this->gain = gain;

    if (!__this->enc_server) {
        return -1;
    }

    printf("--->set_enc_gain: %d\n", gain);

    req.enc.cmd     = AUDIO_ENC_SET_VOLUME;
    req.enc.volume  = gain;
    return server_request(__this->enc_server, AUDIO_REQ_ENC, &req);
}

//调整DAC的数字音量和模拟音量
static int aux_dec_volume_change(int step)
{
    union audio_req req = {0};
    if (!__this->dec_server) {
        return -1;
    }

    sys_volume_read(&__this->volume);
    __this->volume = sys_volume_chack(__this->volume + step);

    printf("->aux set_dec_volume: %d\n", __this->volume);
    req.dec.cmd     = AUDIO_DEC_SET_VOLUME;
    req.dec.volume  = GET_SET_VOLUME(__this->volume);
    int err = server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    sys_volume_write(&__this->volume);
    return err;
}

//设置音量大小
int aux_dec_volume_set_volume(int volume)
{
    union audio_req req = {0};
    if (!__this->dec_server) {
        return -1;
    }
    __this->volume = sys_volume_chack(volume);

    printf("->aux set_dec_volume: %d\n", __this->volume);

    req.dec.cmd     = AUDIO_DEC_SET_VOLUME;
    req.dec.volume  = GET_SET_VOLUME(__this->volume);
    int err = server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    sys_volume_write(&__this->volume);
    return err;
}
static int aux_mode_init(void)
{
    if (__this->init) {
        puts("-> aux_play_init already\n");
        return 0;
    }
    u8 aux_det_last = __this->aux_det_last;
    memset(__this, 0, sizeof(struct aux_hdl));
    __this->aux_det_last = aux_det_last;
    __this->init = true;
#ifdef CONFIG_AUX_AUTO_ENABLE
#if (TCFG_AUX_DET_PORT != 0xff)
    int aux_det_val = gpio_read(TCFG_AUX_DET_PORT);
    if (!aux_det_val) {
        __this->aux_det_ok = true;
    }
#endif
#endif
    os_mutex_create(&__this->mutex);

    sys_volume_read(&__this->volume);
    __this->sample_source = "linein";
    __this->channel = 1;
    __this->gain = 90;
    __this->sample_rate = AUX_SAMPLE_RATE;//44100
    __this->aux_time_1s_drop = 0;
    __this->enc_server = server_open("audio_server", "enc");
    server_register_event_handler_to_task(__this->enc_server, NULL, enc_server_event_handler, "app_core");

    __this->dec_server = server_open("audio_server", "dec");

    aux_play_to_dac(__this->sample_rate, __this->channel);//不开语音识别可以使用软件转DAC播放
//    audio_adc_anaprintfirect_to_dac(__this->sample_rate, __this->channel);//单通道DAC：开语音识别只能直通DAC
#if (defined CONFIG_UI_ENABLE && defined CONFIG_LVGL_UI_ENABLE)
    lv_demo_music_clean();//清除音乐接口
    lv_demo_switch_to_music_page("aux_music");
#endif
    puts("aux_play_init\n");
    return 0;
}

static void aux_mode_exit(void)
{
    aux_close();
    if (__this->dec_server) {
        server_close(__this->dec_server);
        __this->dec_server = NULL;
    }
    if (__this->enc_server) {
        server_close(__this->enc_server);
        __this->enc_server = NULL;
    }
    __this->init = 0;
    //os_mutex_del(&__this->mutex, 0);

//#ifdef CONFIG_SPECTRUM_FFT_EFFECT_ENABLE
//    if(__this->show_timer_id){
//        sys_timer_del(__this->show_timer_id);
//        __this->show_timer_id = 0;
//    }
//#endif
#if (defined CONFIG_UI_ENABLE && defined CONFIG_LVGL_UI_ENABLE)
    lv_demo_music_clean();//清除音乐接口
    lv_demo_switch_to_main_page();
#endif
}

int aux_mode_is_open(void)
{
    return __this->init;
}

short *aux_music_get_audio_fft(int *len)
{
#ifdef CONFIG_SPECTRUM_FFT_EFFECT_ENABLE
    if (!os_mutex_valid(&__this->mutex) || !__this->dec_server || __this->pause || !__this->spectrum_fft_hdl) {
        return NULL;
    }
    os_mutex_pend(&__this->mutex, 6000);
    if (!__this->dec_server || __this->pause || !__this->spectrum_fft_hdl) {
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
    return NULL;
}

int aux_music_stop_status(void)
{
    int det = 1;
#if (defined TCFG_AUX_DET_PORT && defined CONFIG_AUX_AUTO_ENABLE)
#if (TCFG_AUX_DET_PORT != 0xff)
    det = aux_det_is_ok();
#endif
#endif
    if (__this->dec_server && !__this->pause && det) {
        return 0;
    }
    return 1;
}

static int aux_key_click(struct key_event *key)
{
    int ret = true;
    u8 volume = 0;
    switch (key->value) {
    case KEY_OK:
        printf("--->KEY_OK aux_play_pause \n");
        if (__this->direct) {
            if (__this->run_flag) {
                aux_close();
            } else {
                audio_adc_anaprintfirect_to_dac(__this->sample_rate, __this->channel);
            }
        } else {
            aux_play_pause();
        }
        break;
    case KEY_VOLUME_DEC:
        aux_enc_gain_change(-VOLUME_STEP);
        aux_dec_volume_change(-VOLUME_STEP);
#if (SXY_LL_YHH_BOARD && defined USED_TM1629_SHOWN)
        extern void tm_1629_shown_set_volume(int vol);
        sys_volume_read(&volume);
        tm_1629_shown_set_volume((int)volume);
#endif
        break;
    case KEY_VOLUME_INC:
        aux_enc_gain_change(VOLUME_STEP);
        aux_dec_volume_change(VOLUME_STEP);
#if (SXY_LL_YHH_BOARD && defined USED_TM1629_SHOWN)
        extern void tm_1629_shown_set_volume(int vol);
        sys_volume_read(&volume);
        tm_1629_shown_set_volume((int)volume);
#endif
        break;
    case KEY_MODE:
        aux_mode_exit();
        extern int audio_app_mode_switch(char *name);
        audio_app_mode_switch(NULL);
        break;
    case KEY_POWER:;//唤醒
        extern void aisp_wake(char index);//0:小飞小飞，8：配网模式
        aisp_wake(0);
        break;
    case KEY_SUPSPEND:
        if (__this->direct) {
            if (__this->run_flag) {
                aux_close();
            } else {
                audio_adc_anaprintfirect_to_dac(__this->sample_rate, __this->channel);
            }
        } else {
            //aux_play_pause();
            aux_mode_exit();
        }
        break;
    case KEY_RESUM:
        if (__this->direct) {
            if (__this->run_flag) {
                aux_close();
            } else {
                audio_adc_anaprintfirect_to_dac(__this->sample_rate, __this->channel);
            }
        } else {
            //aux_play_pause();
            aux_mode_init();
        }
        break;
    case KEY_CPU_RESET:
        aux_mode_exit();
        ret = false;
        break;
    default:
        ret = false;
        break;
    }

    return ret;
}

static int aux_key_long(struct key_event *key)
{
    u8 volume = 0;
    switch (key->value) {
    case KEY_MODE:
        ;
        extern void aisp_wake(char index);//0:小飞小飞，8：配网模式
        aisp_wake(8);//按键进入配网
        break;
    case KEY_VOLUME_DEC:
    case KEY_UP:
        aux_enc_gain_change(-VOLUME_STEP);
        aux_dec_volume_change(-GAIN_STEP);
#if (SXY_LL_YHH_BOARD && defined USED_TM1629_SHOWN)
        extern void tm_1629_shown_set_volume(int vol);
        sys_volume_read(&volume);
        tm_1629_shown_set_volume((int)volume);
#endif
        break;
    case KEY_VOLUME_INC:
    case KEY_DOWN:
        aux_enc_gain_change(VOLUME_STEP);
        aux_dec_volume_change(GAIN_STEP);
#if (SXY_LL_YHH_BOARD && defined USED_TM1629_SHOWN)
        extern void tm_1629_shown_set_volume(int vol);
        sys_volume_read(&volume);
        tm_1629_shown_set_volume((int)volume);
#endif
        break;
    case KEY_OK:
    case KEY_POWER:
        ;
        extern void sys_power_poweroff(void);
        extern int music_play_waite(void);
        aux_mode_exit();
        ai_speaker_mode_exit();
#ifdef CONFIG_UI_PLAY_EMOJI
        play_face_emoji(AI_UART_CMD_PWR_OFF);
#endif
        music_play_stop_all();
        music_play_res_file("PwrOff.mp3");
        music_play_waite();
        sys_power_poweroff();
        break;
    case KEY_SUPSPEND:
        if (__this->direct) {
            if (__this->run_flag) {
                aux_close();
            } else {
                audio_adc_anaprintfirect_to_dac(__this->sample_rate, __this->channel);
            }
        } else {
            //aux_play_pause();
            aux_mode_exit();
        }
        break;
    case KEY_RESUM:
        if (__this->direct) {
            if (__this->run_flag) {
                aux_close();
            } else {
                audio_adc_anaprintfirect_to_dac(__this->sample_rate, __this->channel);
            }
        } else {
            //aux_play_pause();
            aux_mode_init();
        }
        break;
    default:
        break;
    }

    return true;
}

static int aux_key_hand_up(struct key_event *key)
{
    int ret = true;
    int play = 1;
    u8 volume = 0;
    printf("-->aux_key_hand_up  = %d \n", key->value);

    switch (key->value) {
    case KEY_MODE:
        ;
        if (((__this->last_key >> 8) & 0xFF) == KEY_EVENT_LONG) {
            printf("-->KEY_EVENT_LONG hand_up\n");
        }
        break;
    case KEY_SUPSPEND:
        if (__this->direct) {
            if (__this->run_flag) {
                aux_close();
            } else {
                audio_adc_anaprintfirect_to_dac(__this->sample_rate, __this->channel);
            }
        } else {
            //aux_play_pause();
            aux_mode_exit();
        }
        break;
    case KEY_RESUM:
        if (__this->direct) {
            if (__this->run_flag) {
                aux_close();
            } else {
                audio_adc_anaprintfirect_to_dac(__this->sample_rate, __this->channel);
            }
        } else {
            //aux_play_pause();
            aux_mode_init();
        }
        break;
    }
    return true;
}

static int aux_key_event_handler(struct key_event *key)
{
    auto_sleep_check_clear();
    if (!alarm_music_play_stop() && (key->value != KEY_SUPSPEND && key->value != KEY_RESUM && key->value != KEY_POWER)) {
        return true;//闹钟在响，按键失效
    }
    switch (key->action) {
    case KEY_EVENT_UP:
        aux_key_hand_up(key);
        break;
    case KEY_EVENT_CLICK:
        aux_key_click(key);
        break;
    case KEY_EVENT_LONG:
        aux_key_long(key);
        break;
    case KEY_EVENT_HOLD:
        if (key->value != KEY_CANCLE) { //闹钟唤醒事件
            void music_play_alarm_url_clear(void);
            //music_play_alarm_url_clear();
            music_play_res_file("AlarmRing1.mp3");
            music_play_alarm(key->value);
#ifdef CONFIG_LVGL_UI_ENABLE
            int lv_demo_switch_to_ring_page(void);
            lv_demo_switch_to_ring_page();
#endif
        }
        break;
    case KEY_EVENT_DOUBLE_CLICK:
        if (key->value == KEY_OK) { //模式切换
#if (SXY_LL_YHH_BOARD)
            extern int tm_light_pwm_auto(void);
            tm_light_pwm_auto();
#else //(!defined TCFG_ADKEY_ENABLE || TCFG_ADKEY_ENABLE == 0)
            aux_mode_exit();
            extern int audio_app_mode_switch(char *name);
            audio_app_mode_switch(NULL);
#endif
        }
#ifdef  USED_TM1629_SHOWN
        else if (key->value == KEY_MODE) { //显示亮度切换
            int tm_1629_led_shown_bright_level_auto(void);
            tm_1629_led_shown_bright_level_auto();
        }
#endif
        break;
    case KEY_EVENT_TRIPLE_CLICK:
        if (key->value == KEY_OK) { //配网
            extern void aisp_wake(char index);//0:小飞小飞，8：配网模式
            aisp_wake(8);//按键进入配网
        }
        break;
    case KEY_EVENT_FOURTH_CLICK:
#if (defined CONFIG_WIFI_ENABLE && defined CONFIG_LTE_PHY_ENABLE)
        //4G和WiFi切换
        if (key->value == KEY_OK) {
            int net_ch = sys_net_channel_read();
            if (net_ch < 0) {
                sys_net_channel_write(NET_CH_SELECT_WIFI);
                net_ch = sys_net_channel_read();
            } else {
                net_ch = net_ch ? 0 : 1;
                sys_net_channel_write(net_ch);
                if (sys_net_channel_read() == net_ch) {
                    aisp_all_pause(1);
                    usb_net_set_module_reset_status(1);
                    music_play_res_file("SysReset.mp3");
                    music_play_waite();
                    if (sys_net_channel_read() == 0 && app_usb_get_driver_status()) {
                        app_usb_at_cmd_send_reset();
                        lte_power_control(0);
                    }
                    system_reset();
                }
            }
        }
#endif
        break;
    default:
        break;
    }
    __this->last_key = ((key->action & 0xFF) << 8) | (key->value & 0xFF);
    return true;
}

static int aux_device_event_handler(struct sys_event *sys_eve)
{
    struct device_event *device_eve = (struct device_event *)sys_eve->payload;
    int ret = true;
    u8 usb_id = 0;
    extern int audio_app_mode_switch_set(char *name, char note);
    /* SD卡插拔处理 */
    if (sys_eve->from == DEVICE_EVENT_FROM_SD) {
        switch (device_eve->event) {
        case DEVICE_EVENT_IN:
#if (defined CONFIG_SD_MUSIC_MODE_ENABLE && defined CONFIG_SD_AUTO_ENABLE)
            audio_app_mode_switch_set("sd_music", 1);
#endif
            break;
        case DEVICE_EVENT_OUT:
            ret = false;
            break;
        default :
            ret = false;
            break;
        }
#if TCFG_UDISK_ENABLE
        /* U盘插拔处理 */
    } else if (sys_eve->from == DEVICE_EVENT_FROM_USB_HOST && !strncmp((const char *)device_eve->value, "udisk", 5)) {
        switch (device_eve->event) {
        case DEVICE_EVENT_IN:
            //printf("->device_eve->arg : %s\n",device_eve->arg);
#if (defined CONFIG_USB_DISK_MUSIC_MODE_ENABLE && defined CONFIG_USB_DISK_AUTO_ENABLE)
            audio_app_mode_switch_set("usbdisk_music", 1);
#endif
            break;
        case DEVICE_EVENT_OUT:
            ret = false;
            break;
        default :
            ret = false;
            break;
        }
#endif

#ifdef CONFIG_AUX_MUSIC_MODE_ENABLE
        /* AUX插拔处理 */
    } else if (sys_eve->from == DEVICE_EVENT_FROM_LINEIN) {
        switch (device_eve->event) {
        case DEVICE_EVENT_IN:
            puts("->AUX IN\n");
            aux_mode_init();
            break;
        case DEVICE_EVENT_OUT:
            puts("->AUX OUT\n");
#if (defined CONFIG_AUX_AUTO_ENABLE)
            audio_app_mode_switch_set("ai_speaker", 1);
#endif
            break;
        default :
            ret = false;
            break;
        }
#endif
    } else if (sys_eve->from == DEVICE_EVENT_FROM_ALM) {
        switch (device_eve->event) {
        case DEVICE_EVENT_IN://闹钟事件
            music_play_alarm(device_eve->value);
#ifdef CONFIG_LVGL_UI_ENABLE
            int lv_demo_switch_to_ring_page(void);
            lv_demo_switch_to_ring_page();
#endif
            break;
        case DEVICE_EVENT_OUT:
            break;
        }
    } else if (sys_eve->from == DEVICE_EVENT_FROM_USB_HOST && !strncmp((const char *)device_eve->value, "wireless", 8)) {
#ifdef CONFIG_LTE_PHY_ENABLE
        usb_id = ((const char *)device_eve->value)[8] - '0';
        int net_ch = sys_net_channel_read();
        //走4G网络
        if (net_ch == 1) {
            switch (device_eve->event) {
            case DEVICE_EVENT_IN:
                printf("usb wireless%d device IN", usb_id);
                //4G关闭静态ip冲突检测
                IPV4_ADDR_CONFLICT_DETECT = 0;
                lte_net_init();
                app_usb_at_cmd_init(usb_id);
                break;
            case DEVICE_EVENT_OUT:
                printf("usb wireless%d device OUT", usb_id);
                app_usb_at_cmd_deinit();
                lte_net_close();
                break;
            }
        }
#endif
    }

    return ret;
}

static int aux_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        return aux_key_event_handler((struct key_event *)event->payload);
    case SYS_DEVICE_EVENT:
        return aux_device_event_handler(event);
    case SYS_NET_EVENT:
        ;
        extern int ai_net_event_handler(struct net_event * event);
        return ai_net_event_handler((struct net_event *)event->payload);
#ifdef CONFIG_BT_ENABLE
    case SYS_BT_EVENT:
        ;
        return app_music_bt_event_handler(event);
#endif
    default:
        return false;
    }
}

static int aux_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;
    case APP_STA_START:
        switch (it->action) {
        case ACTION_HOME_MAIN:
            ;
            extern void ai_speaker_mode_exit(void);
            ai_speaker_mode_exit();
            aux_mode_init();
            break;
        case ACTION_MUSIC_PLAY_MAIN:
            break;
        case ACTION_MUSIC_PLAY_VOICE_PROMPT:
            if (!strcmp((const char *)it->data, "BtSucceed.mp3")) {
                music_play_res_file("BtSucceed.mp3");
            } else if (!strcmp((const char *)it->data, "BtDisc.mp3")) {
#ifdef BT_DISCON_MUSIC_NOTICE_ENABLE
                music_play_res_file("BtDisc.mp3");
#endif
            } else if (!strcmp((const char *)it->data, "ring.mp3")) {
                music_play_res_file("ring.mp3");
            }
            break;
        default:
            break;
        }
        break;
    case APP_STA_PAUSE:
        break;
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
        aux_mode_exit();
        break;
    case APP_STA_DESTROY:
        break;
    }

    return 0;
}

static const struct application_operation aux_ops = {
    .state_machine  = aux_state_machine,
    .event_handler  = aux_event_handler,
};

REGISTER_APPLICATION(recorder) = {
    .name   = "aux_music",
    .ops    = &aux_ops,
    .state  = APP_STA_DESTROY,
};

#endif
