#include "app_config.h"

#ifdef CONFIG_UI_ENABLE
#if (defined CONFIG_UI_AVI_LOGO && defined CONFIG_VIDEO_DEC_EN)

#include "GUI_GIF_Private.h"
#include "lcd_drive.h"
#include "lcd_config.h"
#include "server/video_dec_server.h"
#include "sys_common.h"
#include "generic/typedef.h"
#include "fs/fs.h"
#include "simple_avi_unpkg.h"
#include "os/os_api.h"
#include "app_config.h"
#include "video_ioctl.h"
#include "video.h"
#include "yuv_to_rgb.h"
#include "yuv_soft_scalling.h"
#include "asm/jpeg_codec.h"
#include "system/includes.h"
#include "simple_avi_unpkg.h"

#define UI_RES_SD_ENABLE    0   //1：UI资源放在SD卡,0：UI资源放在flash
#define UI_CYC_TEST         0   //1：循环显示测试

#define UI_AVI_PIX_WIDTH    LCD_W   //240 320

#if UI_RES_SD_ENABLE
#define  GIF_FILE_PATH   CONFIG_ROOT_PATH //SD卡读取
#else
#define  GIF_FILE_PATH   CONFIG_UI_RES_FILE_PATH //SD卡读取
#endif

#define JPEG_DEC_MUC_FIX    0 //1:修复JPEG解码问题

#define JPEG_FRAM_SIZE_DEFALUT_SIZE (30*1024)

#if JPEG_DEC_MUC_FIX
#define JPEG_WIDTH      UI_AVI_PIX_WIDTH //分辨率宽,用户根据实际应用更改，和解码图片分辨率相关！！！！
#define JPEG_LINE       16  //行：yuv420/422->16, yuv444->8
#define JPEG_YUV_TTPE   4   //444:1, 422:2, 420:4
#define JPEG_YUV_SIZE   ((JPEG_WIDTH * JPEG_LINE + JPEG_WIDTH * JPEG_LINE / JPEG_YUV_TTPE * 2 + 64) * 2)
static u8 yuv_buf[JPEG_YUV_SIZE] sec(.sram) ALIGNE(32);//内部使用
#endif

//#ifdef CONFIG_SFC_ENABLE
static u8 yuv_buf[LCD_W * LCD_H * 3 / 2] /*sec(.sram)*/ ALIGNE(32);//外部使用
static u8 rgb_buf[LCD_W * LCD_H * 2] /*sec(.sram)*/ ALIGNE(32);//外部使用
#if USE_LCD_TE
static u8 yuv_rev_buf[LCD_W * LCD_H * 3 / 2] /*sec(.sram)*/ ALIGNE(32);//外部使用
#endif
//#else
//#define yuv_buf NULL
//#define rgb_buf NULL
//#endif

static char *logo_avi_path = NULL;
static char *logo_jpeg_path = NULL;
static char logo_play_start = 0;

extern void lcd_show_frame_to_dev(u8 *buf, u32 len);
extern void lcd_show_mirror1_set(char enable);
extern void *jpg_dec_open(struct video_format *f);
extern int jpg_dec_input_data(void *_fh, void *data, u32 len);
extern int jpg_dec_set_output_handler(void *_fh, void *priv, int (*handler)(void *, struct YUV_frame_data *));
extern int jpg_dec_get_s_attr(void *_fh, struct jpg_dec_s_attr *attr);
extern int jpg_dec_set_s_attr(void *_fh, struct jpg_dec_s_attr *attr);
extern int jpg_dec_close(void *_fh);
extern void lcd_lvgl_able(char enable);
extern void ui_jump_to_main_page_flush(void);
extern int ui_is_main_page(void);

int storage_device_ready(void);
static int logo_avi_file_play_task_create(char *avi_path);
static void lcd_lvgl_flush_enable(void);

struct yuv_recv {
    volatile unsigned char *y;
    volatile unsigned char *u;
    volatile unsigned char *v;
    volatile int y_size;
    volatile int u_size;
    volatile int v_size;
    volatile int recv_size;
    volatile int size;
    volatile char complit;
    volatile int yuv_cb_cnt;
};
static int yuv_out_cb(void *priv, struct YUV_frame_data *p)
{
    struct yuv_recv *rec = (struct yuv_recv *)priv;
    u8 type = (p->pixformat == VIDEO_PIX_FMT_YUV444) ? 1 : ((p->pixformat == VIDEO_PIX_FMT_YUV422) ? 2 : 4);
    ++rec->yuv_cb_cnt;
    if (!rec->complit) {
        memcpy((void *)(rec->y + rec->y_size), (void *)p->y, p->width * p->data_height);
        memcpy((void *)(rec->u + rec->u_size), (void *)p->u, p->width * p->data_height / type);
        memcpy((void *)(rec->v + rec->v_size), (void *)p->v, p->width * p->data_height / type);
        rec->y_size += p->width * p->data_height;
        rec->u_size += p->width * p->data_height / type;
        rec->v_size += p->width * p->data_height / type;
        rec->recv_size +=  p->width * p->data_height + (p->width * p->data_height / type) * 2;
        if (rec->recv_size >= rec->size) {
            rec->complit = 1;
        }
    } else {
        printf("err in complit  = %d , type = %s \n", rec->complit, (type == 1) ? "YUV444" : (type == 2) ? "yuv422" : "YUV420");
    }
    return 0;
}
static const char *avi_path[] = {
    "Sleep.avi", "Wakeup.avi", "Ye.avi", NULL,
};
char *rand_path(void)
{
    int max_index = 0;
    int index = 0;
    for (int i = 0; avi_path[i] != NULL; i++) { //删除所有闹钟
        max_index++;
    }
    if (max_index) {
        return avi_path[rand() % max_index];
    }
    return NULL;
}
static int avi_unpkg_decoder_to_play(void *priv)
{
    int ret;
    char name[64];
    char *fbuf = NULL;
    char *yuv = NULL;
    char *cy, *cb, *cr;
    int fbuflen = JPEG_FRAM_SIZE_DEFALUT_SIZE;
    FILE *fd = NULL;
    FILE *yuv_fd = NULL;
    int num = 0;
    int pix;
    char ytype;
    int yuv_len;
    int last, now;
    void *fh = NULL;
    char *rgb565 = NULL;
    int video_chunk = 0;
    int fps = 0;
    char err_cnt = 0;
    char show_jpeg = 0;

    logo_avi_path = priv;
    if (!logo_avi_path || !logo_play_start) {
        return 0;
    }
#if UI_RES_SD_ENABLE
    while (!storage_device_ready()) {//等待sd文件系统挂载完成
        os_time_dly(20);
    }
#endif

    yuv = yuv_buf;
    rgb565 = rgb_buf;

    fbuf = malloc(fbuflen);
    if (!fbuf) {
        printf("no men fbuf err!!!\n");
        goto exit;
    }
    sprintf(name, GIF_FILE_PATH"%s", logo_avi_path);

    fd = fopen(name, "r");
    if (!fd) {
        printf("avi file open err : %s !!!\n", name);
        goto exit;
    }
    ret = avi_net_playback_unpkg_init(fd, 1); //解码初始化,最多10分钟视频
    if (ret) {
        printf("avi_net_playback_unpkg_init err!!!\n");
        goto exit;
    }
    fps = avi_get_fps(fd, 1);
    video_chunk = avi_get_video_chunk_num(fd, 1);

    lcd_lvgl_able(0);

    printf("avi file open : %s !!!\n", name);
    num = 0;
    while (num < video_chunk && logo_play_start) {
        ret = avi_video_get_frame(fd, ++num, fbuf, fbuflen, 1); //全回放功能获取帧
__jpeg_dec:
        if (ret > 0) {
            struct jpeg_image_info info = {0};
            struct jpeg_decode_req req = {0};
            u32 *head = (u32 *)fbuf;
            u8 *dec_buf = fbuf;
            u32 fblen = ret;
            if (*head == IDX_00DC || *head == IDX_01WB || *head == IDX_00WB) {
                fblen -= 8;
                dec_buf += 8;
            }
            info.input.data.buf = dec_buf;
            info.input.data.len = fblen;
            if (jpeg_decode_image_info(&info)) {//获取JPEG图片信息
                printf("jpeg_decode_image_info err, num = %d\n", num);
                //put_buf(dec_buf, fblen);
                continue;
                //break;
            } else {
                switch (info.sample_fmt) {
                case JPG_SAMP_FMT_YUV444:
                    ytype = 1;
                    break;//444
                case JPG_SAMP_FMT_YUV420:
                    ytype = 4;
                    break;//420
                default:
                    ytype = 2;
                }
#if JPEG_DEC_MUC_FIX
                ASSERT(info.width <= JPEG_WIDTH, "err in %s line : %d", __func__, __LINE__)
#endif
                pix = info.width * info.height;
                yuv_len = pix + pix / ytype * 2;
                if (!yuv) {
                    yuv = malloc(yuv_len);
                    if (!yuv) {
                        printf("yuv malloc err len : %d , width : %d , height : %d \n", yuv_len, info.width, info.height);
                        break;
                    }
                }
                if (!rgb565) {
                    rgb565 = malloc(info.width * info.height * 2);
                    if (!rgb565) {
                        printf("rgb565 malloc err len : %d , width : %d , height : %d \n", info.width * info.height * 2, info.width, info.height);
                        break;
                    }
                }
#if JPEG_DEC_MUC_FIX == 0
                cy = yuv;
                cb = cy + pix;
                cr = cb + pix / ytype;

                req.input_type = JPEG_INPUT_TYPE_DATA;
                req.input.data.buf = info.input.data.buf;
                req.input.data.len = info.input.data.len;
                req.buf_y = cy;
                req.buf_u = cb;
                req.buf_v = cr;
                req.buf_width = info.width;
                req.buf_height = info.height;
                req.out_width = info.width;
                req.out_height = info.height;
                req.output_type = JPEG_DECODE_TYPE_DEFAULT;
                req.bits_mode = BITS_MODE_UNCACHE;
                req.dec_query_mode = TRUE;

                ret = jpeg_decode_one_image(&req);//JPEG转YUV解码
                if (ret) {
                    printf("jpeg decode err !!\n");
                    break;
                }
#else
                struct yuv_recv yuv_rec_data = {0};
                if (!fh) {
                    fh = jpg_dec_open(NULL);
                    if (!fh) {
                        printf("err in jpg_dec_open \n\n");
                        return 0;
                    }
                    struct jpg_dec_s_attr jpg_attr;
                    jpg_dec_get_s_attr(fh, &jpg_attr);
                    jpg_attr.max_o_width  = 1920;
                    jpg_attr.max_o_height = 1088;
                    jpg_dec_set_s_attr(fh, &jpg_attr);
                    jpg_dec_set_output_handler(fh, (void *)&yuv_rec_data, yuv_out_cb);
                    jpg_dec_set_yuv(fh, (void *)yuv_buf, sizeof(yuv_buf));
                }
                yuv_rec_data.y = yuv;
                yuv_rec_data.u = yuv_rec_data.y + pix;
                yuv_rec_data.v = yuv_rec_data.u + pix / ytype;
                yuv_rec_data.size = yuv_len;
                yuv_rec_data.recv_size = 0;
                yuv_rec_data.y_size = 0;
                yuv_rec_data.u_size = 0;
                yuv_rec_data.v_size = 0;
                yuv_rec_data.complit = 0;
                yuv_rec_data.yuv_cb_cnt = 0;
                ret = jpg_dec_input_data(fh, info.input.data.buf, info.input.data.len);
                if (ret) {
                    printf("jpg_dec_input_data waite err \n\n");
                    continue;
                }

                if (!yuv_rec_data.complit) {
                    printf("yuv_cb_cnt=%d, num = %d\n", yuv_rec_data.yuv_cb_cnt, num);
                    printf("err yuv_rec_data.complit , size=%d, recv_size=%d\n\n", yuv_rec_data.size, yuv_rec_data.recv_size);
                    goto uninit_redo;
                }

#endif
#if USE_LCD_TE //使用TE信号解决播放视频撕裂感
                int w, h;
                YUV420p_REVERSAL(yuv, yuv_rev_buf, info.width, info.height, &w, &h, 270);
                yuv420p_quto_rgb565(yuv_rev_buf, rgb565, info.height, info.width, 0);
                lcd_show_frame_to_dev_flip(rgb565, info.width * info.height * 2, 0);
#else
                yuv420p_quto_rgb565(yuv, rgb565, info.width, info.height, 0);
                lcd_show_frame_to_dev(rgb565, info.width * info.height * 2);
#endif
                if (!logo_play_start) {
                    break;
                }
                os_time_dly(1000 / fps / 10 / 2);
            }
        }
    }
exit:
    if (fd) {
        avi_net_unpkg_exit(fd, 1);
    }
    if (fd) {
        fclose(fd);
        fd = NULL;
    }
    if (logo_jpeg_path && !show_jpeg) {
        show_jpeg = true;
        sprintf(name, GIF_FILE_PATH"%s", logo_jpeg_path);
        fd = fopen(name, "r");
        if (fd) {
            ret = flen(fd);
            if (ret > fbuflen) {
                fclose(fd);
                fd = NULL;
                ASSERT(0, "fbuflen err in logo file");
            } else {
                ret = fread(fbuf, ret, 1, fd);
                fclose(fd);
                fd = NULL;
                if (ret > 0) {
                    goto __jpeg_dec;
                }
            }
        } else {
            printf("avi file open err : %s !!!\n", name);
        }
    }
    if (fh) {
        jpg_dec_close(fh);
    }
    if (yuv && yuv != yuv_buf) {
        free(yuv);
    }
    if (fbuf) {
        free(fbuf);
    }
    if (rgb565 && rgb565 != rgb_buf) {
        free(rgb565);
    }
    sys_timeout_add_to_task("sys_timer", NULL, lcd_lvgl_flush_enable, 1000);
    if (logo_play_start) {
        sys_timeout_add_to_task("sys_timer", rand_path(), logo_avi_file_play_task_create, TCFG_AUTO_DEV_LOW_PWER_TIME_SEC * 1000);
//        sys_timeout_add_to_task("sys_timer" , rand_path(), logo_avi_file_play_task_create, 2 * 1000);
    }
    puts("--> avi_unpkg_decoder_to_play\n\n");
    return 0;
}
int logo_avi_file_play_kill(void)
{
    if (!logo_play_start) {
        return 0;
    }
    puts("logo_avi_file_play_kill\n");
    logo_play_start = 0;
    os_time_dly(5);
    return logo_play_start;
}
int logo_avi_file_play(char *avi_path, char *jpeg_path)
{
    if (!ui_is_main_page()) {
        return NULL;
    }
    logo_avi_file_play_kill();
    logo_avi_path = avi_path;
    logo_jpeg_path = jpeg_path;
    logo_play_start = true;
    user_lcd_init();
    puts("logo_avi_file_play\n");
    if (thread_fork("logo_task", 10, 1500 * 3, 0, 0, avi_unpkg_decoder_to_play, (void *)avi_path)) {
        puts("err in avi_play");
        return -1;
    }
    return 0;
}
static int logo_avi_file_play_task_create(char *avi_path)
{
    if (!ui_is_main_page()) {
        return NULL;
    }
    user_lcd_init();
    puts("logo_avi_file_play\n");
    if (thread_fork("logo_task", 10, 1500 * 3, 0, 0, avi_unpkg_decoder_to_play, (void *)avi_path)) {
        puts("err in avi_play");
        return -1;
    }
    return 0;
}
static void lcd_lvgl_flush_enable(void)
{
#ifdef CONFIG_LVGL_UI_ENABLE
    lvgl_flush_enable();
#endif
}

#endif
#endif

