#include "server/audio_server.h"
#include "server/server_core.h"
#include "app_config.h"
#include "storage_device.h"
#include "reverb_deal.h"
#include "event/key_event.h"
#include "event/bt_event.h"
#include "btstack/avctp_user.h"
#include "event/device_event.h"
#include "system/wait.h"
#include "system/app_core.h"
#include "action.h"
#include "volume.h"
#include "ai_uart_ctrol.h"

#define MAX_FILENAME_LEN                128

#define UDISK_MUSIC_FILE_PLAY_EN           1  //SD卡音乐按文件夹播放使能

#if UDISK_MUSIC_FILE_PLAY_EN
#define UDISK_STORY_STEP_FILE_PLAY_EN      1   //SD卡跳文件夹使能
#define UDISK_STORY_FILE_PLAY_RAND_EN      0   //SD卡音乐随机播放
#define UDISK_STORY_FILE_PLAY_STEP_NUM     10  //SD卡音乐下十个故事
#endif

#ifdef CONFIG_LTE_PHY_ENABLE
extern u8 IPV4_ADDR_CONFLICT_DETECT;
#endif

// 播放暂停控制函数
int usbdisk_music_play_pause(char pause);
int usbdisk_music_play_prev(void);  // 新增上一曲函数声明
int usbdisk_music_play_next(void);  // 新增下一曲函数声明
int usbdisk_music_set_volume(int volume);  // 新增直接设置音量值函数声明

static int local_music_dec_play_set(char pause, char force_set);
static int local_music_play_file_add(int next_file);
#ifdef  USED_TM1629_SHOWN
extern void tm_1629_shown_set_dir_file_num(int fnum, char is_file);
#endif

#ifdef CONFIG_USB_DISK_MUSIC_MODE_ENABLE
extern void key_vad_pcm_send_set_status(char start, char noice_not);

struct local_music_hdl {
    u8 local_play_all;  //1:全盘播放 0:播放目录
    char volume;
    char puase;
    char force_set;
    char event_out;
    u8 reverb_enable;
    u8 play_loop;
    u16 wait_sd;
    u16 wait_udisk;
    u16 last_key;
    int play_time;
    int total_time;
    FILE *file;
    struct vfscan *fscan;
    struct vfscan *dir_list;
    struct server *dec_server;
    struct audio_dec_breakpoint local_bp;
    const char *local_path;
    void *eq_hdl;
    OS_MUTEX mutex;
    void *spectrum_fft_hdl;
    int last_dir_num;
    int last_file_num;
    unsigned char dir_path[MAX_FILENAME_LEN];
    unsigned char fname[MAX_FILENAME_LEN];

#if UDISK_MUSIC_FILE_PLAY_EN
    const char *local_dir_path;
    int last_rand;
    unsigned char dir_path_len;
    int dir_fnum_start;
    char next_10_file;
    char next_10_dir;
    int fnum;
    int dir_fnum;
#endif
};

static struct local_music_hdl local_music_handler;

#define __this  (&local_music_handler)

extern int user_sd_music_last_dir_num_read(void);
extern int user_sd_music_last_file_num_read(void);
extern int user_sd_music_dir_file_num_write(int dir_num, int file_num);

//获取断点数据
static int local_music_get_dec_breakpoint(struct audio_dec_breakpoint *bp)
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

//设置音量大小
static int local_music_set_dec_volume(int step, char force)
{
    union audio_req req = {0};
    if (!__this->file && !force) {
        return -1;
    }
    sys_volume_read(&__this->volume);
    __this->volume = sys_volume_chack(__this->volume + step);

    printf("->usidk set_dec_volume: %d\n", __this->volume);

    req.dec.cmd     = AUDIO_DEC_SET_VOLUME;
    req.dec.volume  = GET_SET_VOLUME(__this->volume);
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    sys_volume_write(&__this->volume);
    return 0;
}

//设置音量大小
int usbdisk_music_set_dec_volume(int volume)
{
    union audio_req req = {0};
    if (!__this->file || !__this->dec_server) {
        return -1;
    }
    __this->volume = sys_volume_chack(volume);

    printf("->set_dec_volume: %d\n", __this->volume);

    req.dec.cmd     = AUDIO_DEC_SET_VOLUME;
    req.dec.volume  = GET_SET_VOLUME(__this->volume);
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    sys_volume_write(&__this->volume);
    return 0;
}

int usbdisk_music_play_pause(char pause)// 播放暂停控制函数
{
    // 直接调用内部的local_music_dec_play_set函数，并设置force_set为1表示强制设置
    return local_music_dec_play_set(pause, 1);
}

int usbdisk_music_play_prev(void)// 上一曲播放
{
#if UDISK_MUSIC_FILE_PLAY_EN
    return local_music_play_file_add(-1);
#else
    // 调用内部的local_music_dec_switch_file函数，传入FSEL_PREV_FILE参数
    return local_music_dec_switch_file(FSEL_PREV_FILE);
#endif
}

int usbdisk_music_play_next(void)// 下一曲播放
{
    // 调用内部的local_music_dec_switch_file函数，传入FSEL_NEXT_FILE参数
#if UDISK_MUSIC_FILE_PLAY_EN
    return local_music_play_file_add(1);
#else
    return local_music_dec_switch_file(FSEL_NEXT_FILE);
#endif
}

static int local_music_get_dec_status(void)//获取解码器状态
{
    union audio_req req = {0};

    if (__this->dec_server) {
        req.dec.cmd     = AUDIO_DEC_GET_STATUS;
        server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    }
    return req.dec.status;
}

//暂停/继续播放
static int local_music_dec_play_set(char pause, char force_set)
{
    union audio_req req = {0};
    char status = 0;
    if (__this->dec_server) {
        status = local_music_get_dec_status();
        if (status == AUDIO_DEC_START) { //需要：暂停播放
            if (pause) {
                __this->force_set = force_set;
                __this->puase = true;
#ifdef CONFIG_DEC_ANALOG_VOLUME_ENABLE
                req.dec.attr = AUDIO_ATTR_FADE_INOUT;
#endif
                req.dec.cmd = AUDIO_DEC_PP;
                return server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
            }
        } else if (status == AUDIO_DEC_PAUSE) { //需要：继续播放
            if (!pause && (!__this->force_set || force_set)) {
                __this->puase = 0;
                __this->force_set = 0;
#ifdef CONFIG_DEC_ANALOG_VOLUME_ENABLE
                req.dec.attr = AUDIO_ATTR_FADE_INOUT;
#endif
                req.dec.cmd = AUDIO_DEC_PP;
                return server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
            } else if (force_set) {
                __this->force_set = force_set;
            }
        }
    }
    return 0;
}

//暂停/继续播放
static int local_music_dec_play_pause(void)
{
    union audio_req r = {0};

    if (__this->dec_server) {
        char status = local_music_get_dec_status();
        if (status == AUDIO_DEC_START) {
            __this->puase = true;
        } else {
            __this->puase = 0;
        }
#ifdef CONFIG_DEC_ANALOG_VOLUME_ENABLE
        r.dec.attr = AUDIO_ATTR_FADE_INOUT;
#endif
        r.dec.cmd = AUDIO_DEC_PP;
        return server_request(__this->dec_server, AUDIO_REQ_DEC, &r);
    }
    return 0;
}

//停止播放
static int local_music_dec_stop(void)
{
    int err = 0;
    union audio_req req = {0};

    if (!__this->file) {
        return 0;
    }
    os_mutex_pend(&__this->mutex, 6000);
    if (!__this->file) {
        os_mutex_post(&__this->mutex);
        return 0;
    }
    __this->spectrum_fft_hdl = NULL;
    log_i("local_music_dec_stop\n");

    req.dec.cmd = AUDIO_DEC_STOP;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);

    int argv[2];
    argv[0] = AUDIO_SERVER_EVENT_END;
    argv[1] = (int)__this->file;
    server_event_handler_del(__this->dec_server, 2, argv);

    if (__this->play_loop) {
        if (__this->event_out) {
            fclose(__this->file);
            __this->file = NULL;
        } else if (__this->file) {
            fseek(__this->file, 0, SEEK_SET);
        }
    } else {
        fclose(__this->file);
        __this->file = NULL;
    }
    os_mutex_post(&__this->mutex);
    return 0;
}

short *usbdisk_music_get_audio_fft(int *len)
{
#ifdef CONFIG_SPECTRUM_FFT_EFFECT_ENABLE
    if (!os_mutex_valid(&__this->mutex) || __this->puase || !__this->spectrum_fft_hdl) {
        return NULL;
    }
    os_mutex_pend(&__this->mutex, 6000);
    if (__this->puase || !__this->spectrum_fft_hdl) {
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

int usbdisk_music_play_loop_set(char enable)
{
    __this->play_loop = enable;
    return __this->play_loop;
}

int usbdisk_music_stop_status(void)
{
    if (__this->puase || !__this->file || !__this->dec_server) {
        return 1;
    }
    return 0;
}

int usbdisk_music_pause_status(void)
{
    if (__this->puase) {
        return 1;
    }
    return 0;
}

//解码文件
static int local_music_dec_file(FILE *file)
{
    int err;
    union audio_req req = {0};

    log_i("local_music_dec_local_file\n");

    if (!file) {
        return -1;
    }

    local_music_dec_stop();
    os_mutex_pend(&__this->mutex, 6000);
    sys_volume_read(&__this->volume);

    req.dec.cmd             = AUDIO_DEC_OPEN;
    req.dec.volume          = GET_SET_VOLUME(__this->volume);
    req.dec.output_buf_len  = 6 * 1024;
    req.dec.file            = file;
    req.dec.channel         = 0;
    req.dec.sample_rate     = 0;
    req.dec.priority        = 1;
    req.dec.sample_source   = "dac";
    req.dec.force_sr        = FORCE_DAC_SAMPLE_TRATE;//强制使用采样率
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
#ifdef CONFIG_SPECTRUM_FFT_EFFECT_ENABLE
    req.dec.effect |= AUDIO_EFFECT_SPECTRUM_FFT;
#endif
    err = server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    if (err) {
        printf("audio_dec_open: err = %d\n", err);
        fclose(file);
        os_mutex_post(&__this->mutex);
        return err;
    }

    __this->play_time = req.dec.play_time;
    __this->total_time = req.dec.total_time; //获取播放总时长

#if (defined CONFIG_UI_ENABLE && defined CONFIG_LVGL_UI_ENABLE)
    // 更新UI界面的总时长显示
    if (__this->total_time > 0) {
        lv_demo_music_update_total_time(__this->total_time);
    }
#endif

    req.dec.cmd = AUDIO_DEC_START;

    err = server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    if (err) {
        printf("audio_dec_start: err = %d\n", err);
        fclose(file);
        os_mutex_post(&__this->mutex);
        return err;
    }

#ifdef CONFIG_SPECTRUM_FFT_EFFECT_ENABLE
    memset(&req, 0, sizeof(req));
    req.dec.cmd = AUDIO_DEC_GET_EFFECT_HANDLE;
    req.dec.effect = AUDIO_EFFECT_SPECTRUM_FFT;
    server_request(__this->dec_server, AUDIO_REQ_DEC, &req);
    __this->spectrum_fft_hdl = req.dec.get_hdl;
#endif
    __this->file = file;
    os_mutex_post(&__this->mutex);
    log_i("play_music_file: suss\n");

    return 0;
}

//切换上一首或下一首
static int local_music_dec_switch_file(int fsel_mode)
{
    int i = 0;
    FILE *file = NULL;
    int len = 0;
    char name[MAX_FILENAME_LEN] = {0};

    log_i("local_music_dec_switch_file\n");
#ifndef CONFIG_UI_ENABLE
    __this->play_loop = 0;
#endif
    if (!__this->fscan || !__this->fscan->file_number) {
        return -1;
    }

__retry:
    do {
        file = fselect(__this->fscan, fsel_mode, fsel_mode == FSEL_BY_NUMBER ? (CPU_RAND() % __this->fscan->file_number) + 1 : 0);
        if (file) {
            memset(name, 0, sizeof(name));
            int fname_len = fget_name(file, (u8 *)name, sizeof(name) - 1);
            //put_buf(name, fname_len);
            memset(__this->fname, 0, sizeof(__this->fname));
            memcpy(__this->fname, name, fname_len);
            __this->fname[fname_len] = 0;
#ifdef CONFIG_JLFAT_ENABLE
            if (fname_len > 2 && name[0] == '\\' && name[1] == 'U') {
                fname_len = unicode2utf8(&name[2], strlen(&name[2]), __this->fname, sizeof(__this->fname));
                __this->fname[fname_len] = 0;
                //fname_len = unicode_2_utf8((u8 *)name, sizeof(name), (u8 *)&name[2], fname_len - 2);
                //name[fname_len] = 0;
            }
#endif
            printf("---> play file name : %s\n", __this->fname);
#if (defined CONFIG_UI_ENABLE && defined CONFIG_LVGL_UI_ENABLE)
            lv_demo_music_update_lyric(__this->fname);
            lv_demo_music_update_artist("");
#endif
            ++i;
            if (fsel_mode == FSEL_NEXT_FILE) {
                __this->last_file_num++;
                if (__this->last_file_num > __this->fscan->file_number) {
                    __this->last_file_num = 1;
                }
            } else {
                if (__this->last_file_num <= 0) {
                    __this->last_file_num = __this->fscan->file_number;
                }
                __this->last_file_num--;
            }
            break;
        }
        if (fsel_mode == FSEL_NEXT_FILE) {
            fsel_mode = FSEL_FIRST_FILE;
            __this->last_file_num = 1;
        } else if (fsel_mode == FSEL_PREV_FILE) {
            fsel_mode = FSEL_LAST_FILE;
            __this->last_file_num = __this->fscan->file_number;
        } else {
            break;
        }
    } while (i++ < __this->fscan->file_number);

    if (!file) {
        return -1;
    }

    if (0 != local_music_dec_file(file)) {
        if (fsel_mode == FSEL_FIRST_FILE) {
            fsel_mode = FSEL_NEXT_FILE;
        } else if (fsel_mode == FSEL_LAST_FILE) {
            fsel_mode = FSEL_PREV_FILE;
        }
        if (i < __this->fscan->file_number) {
            goto __retry;
        }
    }

    return 0;
}

//切换文件夹
static int local_music_dec_switch_dir(int fsel_mode)
{
    int len = 0;
    int i = 0;
    char name[16] = {0};
    char path[MAX_FILENAME_LEN] = {0};
    FILE *dir = NULL;
    FILE *file = NULL;

    log_i("local_music_dec_switch_dir\n");

    if (!__this->local_path) {
        return -1;
    }

    if (__this->local_play_all && __this->local_path != CONFIG_MUSIC_PATH_FLASH) {
        //全盘搜索
        if (__this->fscan) {
            fscan_release(__this->fscan);
        }
#if CONFIG_DEC_DECRYPT_ENABLE
        __this->fscan = fscan(__this->local_path, "-r -tMP3WMAWAVM4AAMRAPEFLAAACSPXOPUDTSADPSMPOGG -sn", 2);
#else
        __this->fscan = fscan(__this->local_path, "-r -tMP3WMAWAVM4AAMRAPEFLAAACSPXOPUDTSADPOGG -sn", 2);
#endif
        if (!__this->fscan) {
            return -1;
        }
        return local_music_dec_switch_file(FSEL_FIRST_FILE);
    }

    //搜索文件夹
    if (!__this->dir_list) {
        __this->dir_list = fscan(__this->local_path, "-d -sn", 2);
        if (!__this->dir_list || __this->dir_list->file_number == 0) {
            log_w("no_music_dir_find\n");
            return -1;
        }
    }
    if (__this->last_dir_num > __this->dir_list->file_number) {
        __this->last_dir_num = 0;
    }

    //选择文件夹
__again:
    do {
        dir = fselect(__this->dir_list, fsel_mode, 0);
        if (dir) {
            i++;
        }
        if (__this->last_dir_num && dir) {
            if (__this->last_dir_num == i) {
                break;
            }
        } else if (dir) {
            __this->last_dir_num = i;
            break;
        }
        if (fsel_mode == FSEL_NEXT_FILE) {
            fsel_mode = FSEL_FIRST_FILE;
        } else if (fsel_mode == FSEL_PREV_FILE) {
            fsel_mode = FSEL_LAST_FILE;
        } else {
            log_w("fselect_dir_faild, create dir\n");
            return -1;
        }
    } while (i++ < __this->dir_list->file_number);

    if (!dir) {
        return -1;
    }

    len = fget_name(dir, (u8 *)name, sizeof(name) - 1);
    if (0 == len) {
        fclose(dir);
        if (fsel_mode == FSEL_FIRST_FILE) {
            fsel_mode = FSEL_NEXT_FILE;
        } else if (fsel_mode == FSEL_LAST_FILE) {
            fsel_mode = FSEL_PREV_FILE;
        }
        goto __again;
    }

    fclose(dir);

    if (__this->fscan) {
        fscan_release(__this->fscan);
    }

    fname_to_path(path, __this->local_path, name, len, 1, 0);

#if 0   //此处播放指定目录，用户填写的目录路径要注意中文编码问题，看不懂就直接用16进制把路径打印出来
    const char *user_dir = "";
    if (!file && strcmp(path, user_dir)) {
        log_i("dir name : %s\n", path);
        if (fsel_mode == FSEL_FIRST_FILE) {
            fsel_mode = FSEL_NEXT_FILE;
        } else if (fsel_mode == FSEL_LAST_FILE) {
            fsel_mode = FSEL_PREV_FILE;
        }
        goto __again;
    }
#endif

    log_i("fscan path : %s\n", path);

    /*搜索文件夹下的音频文件，按序号排序*/
#if CONFIG_DEC_DECRYPT_ENABLE
    __this->fscan = fscan(path, "-tMP3WMAWAVM4AAMRAPEFLAAACSPXOPUDTSADPSMPOGG -sn", 2);
#else
    __this->fscan = fscan(path, "-tMP3WMAWAVM4AAMRAPEFLAAACSPXOPUDTSADPOGG -sn", 2);
#endif

    if (!file) {
        if (!__this->fscan || !__this->fscan->file_number) {
            if (fsel_mode == FSEL_FIRST_FILE) {
                fsel_mode = FSEL_NEXT_FILE;
            } else if (fsel_mode == FSEL_LAST_FILE) {
                fsel_mode = FSEL_PREV_FILE;
            }
            goto __again;
        }
        local_music_dec_switch_file(FSEL_FIRST_FILE);
    }

    return 0;
}

//快进快退,单位是秒,暂时只支持MP3格式
static int local_music_dec_seek(int seek_step)
{
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

    log_i("local music seek step : %d\n", seek_step);

    return server_request(__this->dec_server, AUDIO_REQ_DEC, &r);
}

#if UDISK_MUSIC_FILE_PLAY_EN
static int local_music_scan_dir(char *path, int next_dir)
{
    FILE *file = NULL;
    char name[MAX_FILENAME_LEN];
    int len = 0, i = 0;

    if (storage_device_ready() == 0) {
        printf("storage_device_ready err\n");
        return -1;
    }
    __this->local_dir_path = path;
    __this->dir_list = fscan(__this->local_dir_path, "-r -d -sn", 2);
    if (!__this->dir_list) {
        printf("dir fscan err\n");
        return -1;
    }
    __this->dir_fnum = __this->dir_list->file_number;
    printf("-> fscan->dir_number : %d\n", __this->dir_fnum);
    if (__this->dir_fnum <= 0) {
        if (__this->dir_list) {
            fscan_release(__this->dir_list);
            __this->dir_list = NULL;
        }
        __this->local_dir_path = NULL;
        return -2;
    }
    if (__this->next_10_dir != 0) {
        if (__this->next_10_dir > 0) {
            if (__this->next_10_dir > __this->dir_fnum) {
                __this->dir_fnum_start = rand32() % __this->dir_fnum + 1;
                __this->next_10_dir = 0;
                __this->last_rand = 0;
                goto _fselet;
            }
        } else if (__this->next_10_dir < 0) {
            if ((-1 * __this->next_10_dir) > __this->dir_fnum) {
                __this->dir_fnum_start = rand32() % __this->dir_fnum + 1;
                __this->next_10_dir = 0;
                __this->last_rand = 0;
                goto _fselet;
            }
        }
        //printf("-> 0 dir_fnum_start : %d, %d\n", __this->dir_fnum_start, __this->next_10_dir);
        if ((int)(__this->dir_fnum_start + __this->next_10_dir) > (int)__this->dir_fnum) {
            __this->dir_fnum_start = (__this->dir_fnum_start + __this->next_10_dir) % __this->dir_fnum;
            __this->next_10_dir = 0;
            __this->last_rand = 0;
        } else if ((int)((int)__this->dir_fnum_start + (int)__this->next_10_dir) <= 0) {
            __this->dir_fnum_start = __this->dir_fnum + (__this->dir_fnum_start + __this->next_10_dir);
            //printf("-> 111 dir_fnum_start : %d, %d, %d\n", __this->dir_fnum_start, __this->dir_fnum, __this->dir_fnum_start + __this->next_10_dir);
            __this->last_rand = 0;
            __this->next_10_dir = 0;
        } else {
            __this->dir_fnum_start += __this->next_10_dir;
            //printf("-> 22 dir_fnum_start : %d, %d\n", __this->dir_fnum_start, __this->next_10_dir);
            __this->next_10_dir = 0;
            __this->last_rand = 0;
        }
        //printf("-> 3 dir_fnum_start : %d, %d\n", __this->dir_fnum_start, __this->next_10_dir);
    }
_fselet:
    //printf("-> last dir_fnum = %d, %d \n",__this->dir_fnum_start,next_dir);
    if (next_dir) {
        __this->dir_fnum_start += next_dir;
    }
    if (__this->dir_fnum_start > __this->dir_fnum) {
        __this->dir_fnum_start = __this->dir_fnum_start % __this->dir_fnum + 1;
    } else if (__this->dir_fnum_start <= 0) {
        __this->dir_fnum_start = (-1 * __this->dir_fnum_start) % __this->dir_fnum + 1;
    }
    //printf("-> dir_fnum = %d \n",__this->dir_fnum_start);
    int fsel_mode = FSEL_FIRST_FILE;
    do {
        file = fselect(__this->dir_list, fsel_mode, fsel_mode == FSEL_BY_NUMBER ? (CPU_RAND() % __this->dir_list->file_number) + 1 : 0);
        if (file) {
            memset(name, 0, sizeof(name));
            int fname_len = fget_name(file, (u8 *)name, sizeof(name) - 1);
            memcpy(__this->dir_path, name, fname_len);
#ifdef CONFIG_JLFAT_ENABLE
            if (fname_len > 2 && name[0] == '\\' && name[1] == 'U') {
                fname_len = unicode2utf8(&name[2], strlen(&name[2]), __this->dir_path, sizeof(__this->dir_path));
                __this->dir_path[fname_len] = 0;
                //fname_len = unicode_2_utf8((u8 *)name, sizeof(name), (u8 *)&name[2], fname_len - 2);
                //name[fname_len] = 0;
            }
#endif
            //printf("-> scan dir name : %s\n", name);
            ++i;
            fclose(file);
            if (__this->dir_fnum_start == i) {
                __this->last_dir_num = __this->dir_fnum_start;
                printf("-> scan dir_%d name : %s\n", i, __this->dir_path);
                goto exit;
            }
        }
        if (fsel_mode == FSEL_FIRST_FILE) {
            fsel_mode = FSEL_NEXT_FILE;
        } else if (fsel_mode == FSEL_PREV_FILE) {
            fsel_mode = FSEL_LAST_FILE;
        }
    } while (file);
    __this->last_rand = 0;
    __this->last_dir_num = __this->dir_fnum_start = 1;
exit:
    if (__this->dir_list) {
        fscan_release(__this->dir_list);
        __this->dir_list = NULL;
    }
    return 0;
}

static int local_music_scan_file(char *path, int next_file)
{
    FILE *file = NULL;
    char name[MAX_FILENAME_LEN];
    int i = 0;

    if (storage_device_ready() == 0) {
        printf("storage_device_ready err\n");
        __this->last_rand = 1;
        __this->dir_fnum_start = 1;
        return -1;
    }
    __this->local_path = path;
//    printf("---> scan path : %s\n", path);
#if CONFIG_DEC_DECRYPT_ENABLE
    __this->fscan = fscan(__this->local_path, "-r -tMP3WMAWAVM4AAMRAPEFLAAACSPXOPUDTSADPSMPOGG -sn", 2);
#else
    __this->fscan = fscan(__this->local_path, "-r -tMP3WMAWAVM4AAMRAPEFLAAACSPXOPUDTSADPOGG -sn", 2);
#endif
    if (!__this->fscan) {
        printf("file fscan err\n");
        return -1;
    }
    __this->fnum = __this->fscan->file_number;
    printf("-> fscan->file_number : %d\n", __this->fnum);
    if (__this->fnum <= 0) {
        goto exit;
    }
    if (__this->next_10_file != 0) {
        //printf("-> next_10_file last_rand : %d, %d\n", __this->last_rand, __this->next_10_file);
        if ((int)(__this->last_rand + __this->next_10_file) > __this->fnum) {
            __this->next_10_file = (__this->last_rand + __this->next_10_file) - __this->fnum;
            __this->dir_fnum_start++;
            __this->last_rand = 1;
            //printf("-> next_dir last_rand : %d, %d\n", __this->last_rand, __this->next_10_file);
            return -2;
        } else if ((int)((int)__this->last_rand + (int)__this->next_10_file) <= 0) {
            __this->next_10_file = (__this->last_rand + __this->next_10_file);
            __this->dir_fnum_start--;
            __this->last_rand = 1;
            //printf("-> prev_dir last_rand : %d, %d\n", __this->last_rand, __this->next_10_file);
            return -3;
        }
        __this->last_rand += __this->next_10_file - 1;
        __this->next_10_file = 0;
        if (__this->last_rand < 1) {
            __this->last_rand = __this->fnum - 1;
        }
        //printf("-> last_rand : %d, %d\n", __this->last_rand, __this->next_10_file);
    }
    //printf("-> last file_num : %d\n", __this->last_rand);
#if UDISK_STORY_FILE_PLAY_RAND_EN
    if (__this->fnum > 1) {
        while (1) {
            int r = rand() % __this->fnum + 1;
            if (r != __this->last_rand) {
                __this->last_rand = r;
                break;
            }
        }
    } else {
        __this->last_rand = __this->fnum;
    }
#else
    if (__this->last_rand <= 0) {
        __this->last_rand = 0;
    }
    if (next_file) {
        __this->last_rand += next_file;
    } else {
        __this->last_rand++;
    }
    //printf("-> __this->last_rand : %d , %d \n",__this->last_rand,__this->fnum);
    if (__this->fnum <= 1 && __this->dir_fnum <= 1) {
        __this->last_rand = __this->fnum;
    } else {
        if (__this->last_rand > __this->fnum) {
            __this->last_rand = 0;
            __this->dir_fnum_start++;
            return -2;
        } else if (__this->last_rand <= 0) {
            __this->last_rand = 0;
            __this->dir_fnum_start--;
            return -3;
        }
    }
#endif
    //printf("-> file_num : %d , %d\n", __this->fnum, next_file);
    int fsel_mode = FSEL_FIRST_FILE;
    do {
        file = fselect(__this->fscan, fsel_mode, fsel_mode == FSEL_BY_NUMBER ? (CPU_RAND() % __this->fscan->file_number) + 1 : 0);
        if (file) {
            memset(name, 0, sizeof(name));
            int fname_len = fget_name(file, (u8 *)name, sizeof(name) - 1);
            //put_buf(name, fname_len);
            memset(__this->fname, 0, sizeof(__this->fname));
            memcpy(__this->fname, name, fname_len);
            __this->fname[fname_len] = 0;
#ifdef CONFIG_JLFAT_ENABLE
            if (fname_len > 2 && name[0] == '\\' && name[1] == 'U') {
                fname_len = unicode2utf8(&name[2], strlen(&name[2]), __this->fname, sizeof(__this->fname));
                __this->fname[fname_len] = 0;
                //fname_len = unicode_2_utf8((u8 *)name, sizeof(name), (u8 *)&name[2], fname_len - 2);
                //name[fname_len] = 0;
            }
#endif
            printf("-> scan file name  %d %d : %s\n", __this->last_rand, i, __this->fname);
#if (defined CONFIG_UI_ENABLE && defined CONFIG_LVGL_UI_ENABLE)
            lv_demo_music_update_lyric(__this->fname);
            lv_demo_music_update_artist("");
#endif
            ++i;
            if (__this->last_rand == i) {
                printf("-> scan file_%d name : %s\n", i, __this->fname);
                goto exit;
            }
            fclose(file);
        }
        if (fsel_mode == FSEL_FIRST_FILE) {
            fsel_mode = FSEL_NEXT_FILE;
        } else if (fsel_mode == FSEL_PREV_FILE) {
            fsel_mode = FSEL_LAST_FILE;
        }
    } while (file);
exit:
    //printf("-> num %d : %s \n",__this->last_rand, __this->fname);
    if (file) {
        __this->last_file_num = __this->last_rand;
        return local_music_dec_file(file);
    } else {
        if (__this->fscan) {
            fscan_release(__this->fscan);
            __this->fscan = NULL;
        }
    }
    return 0;
}

static int local_music_dir_file_scan_play(int dir_fnum_start, int next_dir, int next_file)
{
    int len_offset = 0;
    char buf[MAX_FILENAME_LEN];
    if (storage_device_ready() == 0) {
        printf("storage_device_ready err\n");
        __this->last_rand = 0;
        __this->dir_fnum_start = 0;
        return -1;
    }
    local_music_dec_stop();
    if (__this->dir_list) {
        fscan_release(__this->dir_list);
        __this->dir_list = NULL;
    }
    if (__this->fscan) {
        fscan_release(__this->fscan);
        __this->fscan = NULL;
    }
    int ret = local_music_scan_dir(CONFIG_MUSIC_PATH_SD, next_dir);
    if (ret == -1) {
        printf("local_music_scan_dir err\n");
        return -1;
    }
    if (dir_fnum_start != 0) {
        __this->dir_fnum_start = dir_fnum_start;
    }
    memset(buf, 0, sizeof(buf));
    memcpy(buf + len_offset, CONFIG_MUSIC_PATH_SD, strlen(CONFIG_MUSIC_PATH_SD));
    len_offset += strlen(CONFIG_MUSIC_PATH_SD);
    if (ret == 0) {
        memcpy(buf + len_offset, __this->dir_path, __this->dir_path_len);
        len_offset += __this->dir_path_len;
    }
    put_buf(buf, len_offset);
    return local_music_scan_file(buf, next_file);
}
static int local_music_play_dir_add(int next_dir)
{
#if (UDISK_STORY_FILE_PLAY_RAND_EN == 0)
    __this->last_rand = 0;
#endif
    int next = 0;
    if (next_dir) {
        next = next_dir > 0 ? next_dir : -next_dir;
        if (next == UDISK_STORY_FILE_PLAY_STEP_NUM) {
            __this->next_10_dir = next_dir;
            next = 0;
        } else {
            next = next_dir;
        }
    }
    return local_music_dir_file_scan_play(0, next, 0);
}

static int local_music_play_file_add(int next_file)
{
    int cnt = next_file > 0 ? next_file : (-next_file);
    int next = 0;
#ifndef CONFIG_UI_ENABLE
    __this->play_loop = 0;
#endif
    if (next_file) {
        next = cnt;
        if (next == UDISK_STORY_FILE_PLAY_STEP_NUM) {
            __this->next_10_file = next_file;
            next = 0;
        } else {
            next = next_file;
        }
    }
    int ret = local_music_dir_file_scan_play(0, 0, next);
    while ((ret == -2 || ret == -3) && cnt--) {
        ret = local_music_dir_file_scan_play(0, 0, 0);
    }
    return ret;
}
#endif // UDISK_MUSIC_FILE_PLAY_EN

int user_udisk_music_dir_file_get(int *dir, int *filen)
{
    if (dir) {
        *dir = __this->last_dir_num;
    }
    if (filen) {
        *filen = __this->last_file_num;
    }
    return 0;
}

//释放资源，切换播放源设备
static int local_music_switch_local_device(const char *path)
{
    printf("local_music_switch_local_device\n");
    local_music_dec_stop();
    if (__this->dir_list) {
        fscan_release(__this->dir_list);
        __this->dir_list = NULL;
    }
    if (__this->fscan) {
        fscan_release(__this->fscan);
        __this->fscan = NULL;
    }
    if (__this->wait_sd) {
        wait_completion_del(__this->wait_sd);
        __this->wait_sd = 0;
    }
    if (__this->wait_udisk) {
        wait_completion_del(__this->wait_udisk);
        __this->wait_udisk = 0;
    }
    if (path == NULL) {
        return -1;
    }
    __this->local_path = path;

    if (!__this->last_dir_num) {
        __this->last_dir_num = user_sd_music_last_dir_num_read();
    }
    if (!__this->last_file_num) {
        __this->last_file_num = user_sd_music_last_file_num_read();
    }
    printf("-> last_dir_num = %d, last_file_num = %d\n", __this->last_dir_num, __this->last_file_num);
#if UDISK_MUSIC_FILE_PLAY_EN
    if (__this->last_file_num > 1) {
        __this->last_file_num--;
    }
    if (__this->last_dir_num || __this->last_file_num) {
        __this->last_rand = __this->last_file_num;
        __this->dir_fnum_start = __this->last_dir_num;
    } else {
        __this->last_rand = 0;
        __this->dir_fnum_start = 1;
    }
    local_music_dir_file_scan_play(0, 0, 0);
#else
    local_music_dec_switch_dir(FSEL_FIRST_FILE);
#endif

    return 0;
}

static void udisk_music_dir_file_num_write_to_flash(void)
{
    if (__this->last_dir_num && __this->last_file_num) {
        user_sd_music_dir_file_num_write(__this->last_dir_num, __this->last_file_num);
        printf("-> save last_dir_num = %d, last_file_num = %d\n", __this->last_dir_num, __this->last_file_num);
    }
}

static void dec_server_event_handler(void *priv, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
        log_i("local_music: AUDIO_SERVER_EVENT_ERR\n");
    case AUDIO_SERVER_EVENT_END:
        log_i("local_music: AUDIO_SERVER_EVENT_END\n");
        local_music_dec_stop();
        if (__this->event_out) {
            break;
        }
        if (__this->play_loop && __this->play_time >= (__this->total_time - 2) && __this->file) {
            sys_timeout_add(__this->file, local_music_dec_file, 100);
        } else {
#if UDISK_MUSIC_FILE_PLAY_EN
            local_music_play_file_add(1);
#else
            local_music_dec_switch_file(FSEL_NEXT_FILE);
#endif
        }
        break;
    case AUDIO_SERVER_EVENT_CURR_TIME:
        log_d("play_time: %d\n", argv[1]);
        __this->play_time = argv[1];
#if (defined CONFIG_UI_ENABLE && defined CONFIG_LVGL_UI_ENABLE)
        // 更新当前播放时间到UI界面
        if (__this->play_time > 0) {
            lv_demo_music_update_current_time(__this->play_time);
        }
#endif
        break;
    }
}

static int local_music_mode_init(void)
{
    puts("local_music_play_main\n");

    memset(__this, 0, sizeof(struct local_music_hdl));

    os_mutex_create(&__this->mutex);

    sys_volume_read(&__this->volume);

    __this->local_play_all = 1;
    __this->dec_server = server_open("audio_server", "dec");
    if (!__this->dec_server) {
        return -1;
    }
    server_register_event_handler_to_task(__this->dec_server, NULL, dec_server_event_handler, "sys_timer");
    if (udisk_storage_device_ready(0)) {
        return local_music_switch_local_device(CONFIG_MUSIC_PATH_UDISK);
    }
    return 0;
}

static void local_music_mode_exit(void)
{
#if defined CONFIG_REVERB_MODE_ENABLE && defined CONFIG_AUDIO_MIX_ENABLE
    if (__this->reverb_enable) {
        echo_reverb_uninit();
    }
#endif
    local_music_switch_local_device(NULL);
    if (__this->dec_server) {
        server_close(__this->dec_server);
        __this->dec_server = NULL;
    }
    //os_mutex_del(&__this->mutex, 0);
}

static int local_music_key_click(struct key_event *key)
{
    int ret = true;
    u8 volume = 0;
    switch (key->value) {
    case KEY_OK:
        if (system_keyworld_start()) {
            aisp_clear(1);
        }
#if (defined SXY_LL_YHH_BOARD)
        local_music_mode_exit();
        extern int audio_app_mode_switch(char *name);
        audio_app_mode_switch(NULL);
#else
        local_music_dec_play_pause();
#endif
        break;
    case KEY_UP:
#if UDISK_MUSIC_FILE_PLAY_EN
        local_music_play_file_add(-1);
#else
        local_music_dec_switch_file(FSEL_PREV_FILE);
#endif
#ifdef  USED_TM1629_SHOWN
        tm_1629_shown_set_dir_file_num(__this->last_file_num, 1);
#endif
        break;
    case KEY_DOWN:
#if UDISK_MUSIC_FILE_PLAY_EN
        local_music_play_file_add(1);
#else
        local_music_dec_switch_file(FSEL_NEXT_FILE);
#endif
#ifdef  USED_TM1629_SHOWN
        tm_1629_shown_set_dir_file_num(__this->last_file_num, 1);
#endif
        break;
    case KEY_VOLUME_DEC:
        local_music_set_dec_volume(-VOLUME_STEP, 0);
#if (SXY_LL_YHH_BOARD && defined USED_TM1629_SHOWN)
        extern void tm_1629_shown_set_volume(int vol);
        sys_volume_read(&volume);
        tm_1629_shown_set_volume((int)volume);
#endif
        break;
    case KEY_VOLUME_INC:
        local_music_set_dec_volume(VOLUME_STEP, 0);
#if (SXY_LL_YHH_BOARD && defined USED_TM1629_SHOWN)
        extern void tm_1629_shown_set_volume(int vol);
        sys_volume_read(&volume);
        tm_1629_shown_set_volume((int)volume);
#endif
        break;
    case KEY_MODE:
//#if defined CONFIG_REVERB_MODE_ENABLE && defined CONFIG_AUDIO_MIX_ENABLE
//        if (__this->reverb_enable) {
//            //关闭混响
//            echo_reverb_uninit();
//            __this->reverb_enable = 0;
//        } else {
//            //配置混响参数
//            const struct __HOWLING_PARM_ howling_parm = {13, 20, 20, 300, 5, -50000/*-25000*/, 0, 16000, 1};
//#if CONFIG_AUDIO_ENC_SAMPLE_SOURCE != AUDIO_ENC_SAMPLE_SOURCE_MIC
//            echo_reverb_init(48000, 16000, BIT(CONFIG_AUDIO_ENC_SAMPLE_SOURCE + 3), 100, __this->volume, NULL, NULL, (void *)&howling_parm, NULL);
//#else
//            echo_reverb_init(48000, 16000, BIT(CONFIG_REVERB_ADC_CHANNEL), 100, __this->volume, NULL, NULL, (void *)&howling_parm, NULL);
//#endif
//            __this->reverb_enable = 1;
//        }
//#else
//        if (udisk_storage_device_ready(0)) {
//            //local_music_switch_local_device(CONFIG_MUSIC_PATH_SD);
//            local_music_switch_local_device(NULL);
//        }
//#endif
#if (defined SXY_LL_YHH_BOARD)
        local_music_dec_play_pause();
#else
        local_music_mode_exit();
        extern int audio_app_mode_switch(char *name);
        audio_app_mode_switch(NULL);
#endif
        break;
    case KEY_POWER:;//唤醒
        extern void aisp_wake(char index);//0:小飞小飞，8：配网模式
        aisp_wake(0);
        break;
    case KEY_SUPSPEND:
        local_music_dec_play_set(1, 1);
        break;
    case KEY_RESUM:
        local_music_dec_play_set(0, 1);
        break;
    case KEY_CPU_RESET:
        local_music_mode_exit();
        ret = false;
        break;
    case KEY_LOOP:
        usbdisk_music_play_loop_set(1);
        break;
    case KEY_EXIT_LOOP:
        usbdisk_music_play_loop_set(0);
        break;
    default:
        ret = false;
        break;
    }

    return ret;
}

static int local_music_key_long(struct key_event *key)
{
    u8 volume = 0;
    switch (key->value) {
    case KEY_MODE:
        ;
#if (defined SXY_LL_YHH_BOARD)
        if (((__this->last_key >> 8) & 0xFF) != KEY_EVENT_LONG) {
            key_vad_pcm_send_set_status(1, 1);
        }
#else
        extern void aisp_wake(char index);//0:小飞小飞，8：配网模式
        aisp_wake(8);//按键进入配网
#endif
        break;
    case KEY_VOLUME_DEC:
    case KEY_UP:
        local_music_set_dec_volume(-VOLUME_STEP, 1);
#if (SXY_LL_YHH_BOARD && defined USED_TM1629_SHOWN)
        extern void tm_1629_shown_set_volume(int vol);
        sys_volume_read(&volume);
        tm_1629_shown_set_volume((int)volume);
#endif
        break;
    case KEY_VOLUME_INC:
    case KEY_DOWN:
        local_music_set_dec_volume(VOLUME_STEP, 1);
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
        ai_speaker_mode_exit();
        local_music_mode_exit();
#ifdef CONFIG_UI_PLAY_EMOJI
        play_face_emoji(AI_UART_CMD_PWR_OFF);
#endif
        music_play_stop_all();
        music_play_res_file("PwrOff.mp3");
        music_play_waite();
        sys_power_poweroff();
        break;
    case KEY_SUPSPEND:
        local_music_dec_play_set(1, 0);
        break;
    case KEY_RESUM:
        local_music_dec_play_set(0, 0);
        break;
    default:
        break;
    }

    return true;
}
static int local_music_key_hand_up(struct key_event *key)
{
    int ret = true;
    int play = 1;
    u8 volume = 0;
    printf("-->sd_key_hand_up  = %d \n", key->value);
    switch (key->value) {
    case KEY_MODE:
        ;
        if (((__this->last_key >> 8) & 0xFF) == KEY_EVENT_LONG) {
            printf("-->KEY_EVENT_LONG hand_up\n");
#if (defined SXY_LL_YHH_BOARD)
            key_vad_pcm_send_set_status(0, 1);
#endif
        }
        break;
    case KEY_SUPSPEND:
        local_music_dec_play_set(1, 0);
        break;
    case KEY_RESUM:
        local_music_dec_play_set(0, 0);
        break;
    }
    return true;
}

static int local_music_key_event_handler(struct key_event *key)
{
    auto_sleep_check_clear();
    if (!alarm_music_play_stop() && (key->value != KEY_SUPSPEND && key->value != KEY_RESUM && key->value != KEY_POWER)) {
        return true;//闹钟在响，按键失效
    }
    switch (key->action) {
    case KEY_EVENT_UP:
        local_music_key_hand_up(key);
        break;
    case KEY_EVENT_CLICK:
        local_music_key_click(key);
        break;
    case KEY_EVENT_LONG:
        local_music_key_long(key);
        break;
    case KEY_EVENT_HOLD:
        break;
    case KEY_EVENT_DOUBLE_CLICK:
        if (key->value == KEY_OK) { //模式切换
#if (SXY_LL_YHH_BOARD)
            extern int tm_light_pwm_auto(void);
            tm_light_pwm_auto();
#else // (!defined TCFG_ADKEY_ENABLE || TCFG_ADKEY_ENABLE == 0)
            local_music_mode_exit();
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
#if UDISK_STORY_STEP_FILE_PLAY_EN
        if (key->value == KEY_DOWN) { //V+
            local_music_play_dir_add(1);//文件夹
        } else if (key->value == KEY_UP) { //V-
            local_music_play_dir_add(-1);//文件夹
        }
#ifdef  USED_TM1629_SHOWN
        if (key->value == KEY_DOWN || key->value == KEY_UP) { //V+ V-
            tm_1629_shown_set_dir_file_num(__this->last_dir_num, 0);
        }
#endif
#endif
        break;
    case KEY_EVENT_TRIPLE_CLICK:
        if (key->value == KEY_OK) { //配网
            extern void aisp_wake(char index);//0:小飞小飞，8：配网模式
            aisp_wake(8);//按键进入配网
        }
#if UDISK_STORY_STEP_FILE_PLAY_EN
        if (key->value == KEY_DOWN) { //V+
            local_music_play_dir_add(UDISK_STORY_FILE_PLAY_STEP_NUM);//文件夹
        } else if (key->value == KEY_UP) { //V-
            local_music_play_dir_add(-UDISK_STORY_FILE_PLAY_STEP_NUM);//文件夹
        } else if (key->value == KEY_MODE) { //mode
            local_music_play_file_add(UDISK_STORY_FILE_PLAY_STEP_NUM);//文件
        }
#ifdef  USED_TM1629_SHOWN
        if (key->value == KEY_DOWN || key->value == KEY_UP) { //V+ V-
            tm_1629_shown_set_dir_file_num(__this->last_dir_num, 0);
        } else if (key->value == KEY_MODE) { //mode
            tm_1629_shown_set_dir_file_num(__this->last_file_num, 1);
        }
#endif
#endif
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

/*
 *设备响应函数
 */
static int local_music_device_event_handler(struct sys_event *sys_eve)
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
            audio_app_mode_switch("sd_music");
            ret = true;
#endif
            break;
        case DEVICE_EVENT_OUT:
            break;
        }
#if TCFG_UDISK_ENABLE
        /* U盘插拔处理 */
    } else if (sys_eve->from == DEVICE_EVENT_FROM_USB_HOST && !strncmp((const char *)device_eve->value, "udisk", 5)) {
        switch (device_eve->event) {
        case DEVICE_EVENT_IN:
            __this->event_out = false;
            if (!__this->wait_udisk) {
                __this->wait_udisk = wait_completion(udisk_storage_device_ready,
                                                     (int (*)(void *))local_music_switch_local_device,
                                                     CONFIG_MUSIC_PATH_UDISK, ((const char *)device_eve->value)[5] - '0');
            }
            break;
        case DEVICE_EVENT_OUT://U盘拔出，释放资源
            __this->event_out = true;
            udisk_music_dir_file_num_write_to_flash();
            local_music_switch_local_device(NULL);
#if (defined CONFIG_USB_DISK_MUSIC_MODE_ENABLE && defined CONFIG_USB_DISK_AUTO_ENABLE)
            extern int audio_app_mode_switch_set(char *name, char note);
            audio_app_mode_switch_set("ai_speaker", 1);
#endif
#if (defined CONFIG_UI_ENABLE && defined CONFIG_LVGL_UI_ENABLE)
            lv_demo_music_clean();//清除音乐接口
            lv_demo_switch_to_main_page();
#endif
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
#if (defined CONFIG_AUX_AUTO_ENABLE)
            audio_app_mode_switch_set("aux_music", 1);
#endif
            break;
        case DEVICE_EVENT_OUT:
            puts("->AUX OUT\n");
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

static int local_music_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        return local_music_key_event_handler((struct key_event *)event->payload);
    case SYS_DEVICE_EVENT:
        return local_music_device_event_handler(event);
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

static int local_music_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;
    case APP_STA_START:
        switch (it->action) {
        case ACTION_HOME_MAIN:
            local_music_mode_init();
            if (!__this->wait_udisk) {
#if (defined CONFIG_UI_ENABLE && defined CONFIG_LVGL_UI_ENABLE)
                lv_demo_music_clean();//清除音乐接口
                lv_demo_switch_to_music_page("usbdisk_music");
#endif
                __this->wait_udisk = wait_completion(udisk_storage_device_ready,
                                                     (int (*)(void *))local_music_switch_local_device,
                                                     CONFIG_MUSIC_PATH_UDISK, 0);//usb0
            }
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
        udisk_music_dir_file_num_write_to_flash();
        local_music_mode_exit();
#if (defined CONFIG_UI_ENABLE && defined CONFIG_LVGL_UI_ENABLE)
        lv_demo_music_clean();//清除音乐接口
        lv_demo_switch_to_main_page();
#endif
        break;
    case APP_STA_DESTROY:
        break;
    }

    return 0;
}

static const struct application_operation local_music_ops = {
    .state_machine  = local_music_state_machine,
    .event_handler  = local_music_event_handler,
};

REGISTER_APPLICATION(local_music) = {
    .name   = "usbdisk_music",
    .ops    = &local_music_ops,
    .state  = APP_STA_DESTROY,
};

#endif
