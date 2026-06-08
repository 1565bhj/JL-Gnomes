#include "app_config.h"

#ifdef CONFIG_UI_ENABLE
#if (defined CONFIG_UI_AVI_EYE && defined CONFIG_VIDEO_DEC_EN)

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
#include "ai_uart_ctrol.h"

#ifndef USE_LCD_TE_ROTATE_ENABLE
#define USE_LCD_TE_ROTATE_ENABLE    0 //是否开启TE使用时播放视频翻转视频数据
#endif

#define UI_RES_SD_ENABLE    0   //1：UI资源放在SD卡,0：UI资源放在flash

#define UI_AVI_PIX_WIDTH    LCD_W   //240 320

#if UI_RES_SD_ENABLE
#define  GIF_FILE_PATH   CONFIG_ROOT_PATH"emoji-avi/%d.avi" //SD卡读取
#else
#define  GIF_FILE_PATH   CONFIG_UI_RES_FILE_PATH"%d.avi" //SD卡读取
#endif

#define JPEG_DEC_MUC_FIX    0 //1:修复JPEG解码问题

#define JPEG_FRAM_SIZE_DEFALUT_SIZE (30*1024)

static SEC_USED(.sram) char JPEG_BUF[JPEG_FRAM_SIZE_DEFALUT_SIZE] ALIGNE(32);

#if JPEG_DEC_MUC_FIX
#define JPEG_WIDTH      UI_AVI_PIX_WIDTH //分辨率宽,用户根据实际应用更改，和解码图片分辨率相关！！！！
#define JPEG_LINE       16  //行：yuv420/422->16, yuv444->8
#define JPEG_YUV_TTPE   4   //444:1, 422:2, 420:4
#define JPEG_YUV_SIZE   ((JPEG_WIDTH * JPEG_LINE + JPEG_WIDTH * JPEG_LINE / JPEG_YUV_TTPE * 2 + 64) * 2)
static u8 yuv_buf[JPEG_YUV_SIZE] sec(.sram) ALIGNE(32);//内部使用
#endif

#ifdef CONFIG_SFC_ENABLE
static u8 yuv_buf[LCD_W * LCD_H * 3 / 2] sec(.sram) ALIGNE(32);//外部使用
static u8 rgb_buf[LCD_W * LCD_H * 2] /*sec(.sram)*/ ALIGNE(32);//外部使用
#else
#define yuv_buf NULL
#define rgb_buf NULL
#endif

#if (USE_LCD_TE_ROTATE_ENABLE && USE_LCD_TE && (__SDRAM_SIZE__ >= (8*1024*1024)))
static u8 yuv_rev_buf[LCD_W * LCD_H * 3 / 2] /*sec(.sram)*/ ALIGNE(32);//外部使用
#endif

#ifdef CONFIG_UI_MIRROR_EYE
//打开镜像时，需要设置镜像ID
static int avi_mirror_index[] = {
    AI_UART_CMD_EMOJI_HAPPY,
    AI_UART_CMD_EMOJI_EXCITE,
    AI_UART_CMD_EMOJI_QUIET,
    AI_UART_CMD_EMOJI_THINK,
    AI_UART_CMD_EMOJI_AMAZE,
    //添加自己的镜像ID
};
#endif

static const char *happy_str[] = {
    "😀", "😃", "😄", "😁", "😆", "🤣", "😂",
    //"快乐", "欢喜", "开心", "欢乐", "喜","笑哭","高兴","有意思","有趣","好玩","开心","笑疯了","嘻嘻","哈哈",
    NULL,
};
static const char *smile_str[] = {
    "🙂", "☺️", "🙃", "🫠", "🤗", "🤭", "👌🏻", "👈🏻",
    //"微笑", "友好", "冷笑", "呵呵", "放松",
    NULL,
};
static const char *excited_str[] = {
    "🤩", "😁", "😉", "👋", "✨", "✌🏻",
    //"兴奋","期待",
    NULL,
};
static const char *quite_str[] = {
    "🤫", "😶", "😯", "🙂", "😌", "😐", "😑", "😐", "🤐", "🔕",
    //"静静","安静","无聊","发呆","在干什么","太慢了","等待",
    NULL,
};
static const char *think_str[] = {
    "🤔", "😔", "🤔", "🧐", "🫤", "😕", "🤓", "😖",
    //"思考","专注",
    NULL,
};
static const char *sad_str[] = {
    "😭", "😢", "😣", "😢", "🥹", "😧", "🥺", "😞", "😵", "🥴", "😫", "😩", "💔",
    //"伤心","哭","泪","呜呜","哭了","哭啦","哭泣","哭哭","呜呜","难过",
    NULL,
};
static const char *grievance_str[] = {
    "☹️", "😢", "🥹", "😢", "🫥", "🥹", "😖", "🙁", "😟", "😕", "😔", "😞", "❤️‍🩹",
    //"委屈","悲伤","憋屈","凶我","真凶","说你","你的错","都怪你","冤枉",
    NULL,
};
static const char *angry_str[] = {
    "😡", "🤬", "😤", "😡", "👿", "😈", "😒", "❤️‍🔥", "🤛🏻", "👊🏻",
    //"生气","暴躁","发火","不爽","诅咒","生气","气到了","气炸了","气坏","气疯","火大","发火","越想越气","憋火","窝火","揍人","揍他",
    NULL,
};
static const char *whiny_str[] = {
    "😒", "😑", "😖", "🙄", "😶‍🌫️", "😬",
    //"懊恼","烦躁",
    NULL,
};
static const char *surprise_str[] = {
    "😮", "😠", "😲", "😯", "😧",
    //"惊讶","诧异","我去","好家伙","我的天","妈呀","哇塞","不会吧","不是吧",
    NULL,
};
static const char *confused_str[] = {
    "🧐", "🫤", "😟", "🤓", "🧐", "⁉️", "❓", "❔",
    //"疑惑","不解","为什么","疑惑不解","疑问","没听懂","啥意思","没明白","啊？","啥？","啥呀？","为啥？","啥情况？",
    NULL,
};
static const char *shy_str[] = {
    "🫣", "😊", "😳", "🪭",
    //"害羞","腼腆","害羞","有点害羞","好害羞","怪不好意思","羞得慌",
    NULL,
};
static const char *sleep_str[] = {
    "😴", "😪", "😫", "🥱", "😫", "😩",
    //"休息","睡觉","疲倦","困了","困倦","困了","好困","困死","贼困","困得不行","有点困","想睡觉","累了","好累",
    NULL,
};
static const char *enjoy_str[] = {
    "🤩", "🥰", "😍", "😎", "😘", "🤤",
    //"陶醉","享受","可爱",
    NULL,
};
static const char *naughty_str[] = {
    "😜", "🤪", "😝", "😋", "😛", "😝", "🤨", "🤓", "🥵", "🤑", "🙏🏻", "🥚",
    //"调皮","捣蛋","淘气","捣乱","欠揍","恶作剧","小丑","魔术",
    NULL,
};
static const char *fear_str[] = {
    "😧", "😟", "😦", "😅", "🫢", "🫣", "😧", "😯", "😱", "😨", "😰", "😥", "😖",
    //"恐惧","害怕","担心","紧张","怕怕","有点怕","好怕","贼怕","怕不行","我怕","别吓我","哆嗦","害怕",
    NULL,
};
static const char *proud_str[] = {
    "🥲", "🥹", "😤", "😎",
    //"骄傲","自豪","把握","淡定","不慌","踏实","你能行","你可以的","牛啊","你真牛","稳了","没问题","小意思","靠你了","交给你了","你可以","放心",
    NULL,
};
static const char *depressed_str[] = {
    "😞", "😦", "😥", "😣", "😟", "😧", "😨", "🙁",
    //"沮丧","失落",
    NULL,
};
static const char *desire_str[] = {
    "😍", "🥺", "🤤",
    //"急切","渴望",
    NULL,
};
static const char *gentle_str[] = {
    "🥰", "😘", "😚", "😙", "😗", "😉", "😌", "🥳", "🤩", "😽", "🤗",
    //"温柔","亲切",
    NULL,
};
static const char *curiosity_str[] = {
    "🤔", "🧐‌", "👀", "💡", "🦝", "🤩",
    //"好奇","探索",
    NULL,
};
static const char *cute_str[] = {
    "😍", "🥰", "🙂", "🤠", "😈", "💖", "❤️", "🧡", "💛", "💚", "💙", "💜", "🤍", "🤎", "❣", "️", "💕", "💞", "💓", "💗", "💖", "👄",
    //"可爱", "活剥",
    NULL,
};
static const char *cool_str[] = {
    "😎", "🥸", "🤓", "🧐", "👍🏻",
    //"耍帅","装酷","耍酷","真帅","真美","真漂亮","真好看","耍个帅","装个酷","好酷","我爱你","爱你","你真好","真棒","真不错",
    NULL,
};
static const char *embarrassed_str[] = {
    "🥲", "🥵", "😰", "😨", "😥", "😓", "😅",
    //"尬得慌","有点尬","尬住","太尬","好尬","尴尬","尴死",
    NULL,
};
static const char *fantasy_str[] = {
    "🤔", "🧐", "🤨", "🤓", "😎", "🥸",
    //"发呆","幻想","走思","走神儿","瞎想","白日梦","发会儿呆",
    NULL,
};
static const char *dizzy_str[] = {
    "😵‍💫", "🥴", "🤢", "🤮", "🤤", "🤕", "😇",
    //"晕","我晕","我醉了","我懵了","服了","绕晕我了","整晕了","说晕我了","这操作",
    NULL,
};

static struct gif_eye {
    unsigned int avi_index;
    unsigned int lock_index;
    unsigned char task_init;
    unsigned char play_speed;
    unsigned char task_suspend;
    unsigned char loop_disable;
    unsigned char play_audio;
    FILE *gif_fd;
    OS_SEM sem;
} AVI_FACE = {0};

enum {
    TASK_INIT = 0,
    TASK_CREATE,
    TASK_KILL,
};

extern void lcd_show_frame_to_dev(u8 *buf, u32 len);
extern void lcd_show_mirror1_set(char enable);
extern void *jpg_dec_open(struct video_format *f);
extern int jpg_dec_input_data(void *_fh, void *data, u32 len);
extern int jpg_dec_set_output_handler(void *_fh, void *priv, int (*handler)(void *, struct YUV_frame_data *));
extern int jpg_dec_get_s_attr(void *_fh, struct jpg_dec_s_attr *attr);
extern int jpg_dec_set_s_attr(void *_fh, struct jpg_dec_s_attr *attr);
extern int jpg_dec_close(void *_fh);

int storage_device_ready(void);
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
static int avi_unpkg_decoder_to_play(void *priv)
{
    int ret, cnt;
    char name[64];
    char *fbuf = NULL;
    char *yuv = NULL;
    char *cy, *cb, *cr;
    int fbuflen = JPEG_FRAM_SIZE_DEFALUT_SIZE;
    FILE *fd = NULL;
    FILE *fd1 = NULL;
    FILE *yuv_fd = NULL;
    int num = 0;
    int pix;
    char ytype;
    int yuv_len;
    int last, now;
    void *fh = NULL;
    int last_index = -1;
    char err_cnt = 0;
    char *rgb565 = NULL;
    char *audio_buf = NULL;
    int audio_buf_len = 0;
    int index = (int)priv;
    int video_chunk = 0;
    int audio_chunk = 0;
    int per_video_time = 0;
    int per_audio_time = 0;
    int all_video_time = 0;
    int all_audio_time = 0;

    if (is_production_test_enter(0)) {
        goto exit;
    }

    AVI_FACE.task_init = TASK_CREATE;

#if UI_RES_SD_ENABLE
    while (!storage_device_ready()) {//等待sd文件系统挂载完成
        os_time_dly(20);
    }
#endif

    yuv = yuv_buf;
    rgb565 = rgb_buf;

    if (JPEG_BUF) {
        fbuf = JPEG_BUF;
    } else {
        fbuf = malloc(fbuflen);
        if (!fbuf) {
            printf("no men fbuf err!!!\n");
            goto exit;
        }
    }
redo:
    AVI_FACE.avi_index = index;

    sprintf(name, GIF_FILE_PATH, index);

    fd = fopen(name, "r");
    if (!fd) {
        printf("avi file open err : %s !!!\n", name);
        if (index != AI_UART_CMD_EMOJI_DATA) {
            index = last_index != -1 ? last_index : AI_UART_CMD_EMOJI_DATA;
        }
        goto redo;
    }
    err_cnt = 0;
    last_index = index;

    ret = avi_net_playback_unpkg_init(fd, 1); //解码初始化,最多10分钟视频
    if (ret) {
        printf("avi_net_playback_unpkg_init err!!!\n");
        goto exit;
    }
    video_chunk = avi_get_video_chunk_num(fd, 1);
    audio_chunk = avi_get_audio_chunk_num(fd, 1);
    per_audio_time = avi_get_audio_per_frame_time(1);
    per_video_time = avi_get_video_per_frame_time(1);
    int new_audio_buf_len = audio_chunk * avi_get_audio_per_frame_size(1) * 1.2;
    int ad_num = 0;
    int au_size = 0;
    if (audio_chunk) {
        mp3_buf_play_stop(NULL);
        if (new_audio_buf_len > audio_buf_len) {
            if (audio_buf) {
                free(audio_buf);
                audio_buf = NULL;
            }
            audio_buf_len = audio_chunk * avi_get_audio_per_frame_size(1) * 1.2;//解码音频是MP3的文件的所有音频大小
            audio_buf = malloc(audio_buf_len);
        }
        if (!audio_buf) {
            printf("audio_buf malloc err!!!\n");
            goto exit;
        }
        memset(audio_buf, 0, audio_buf_len);
        while (ad_num < audio_chunk) {
            ret = avi_audio_get_frame(fd, ++ad_num, audio_buf + au_size, audio_buf_len - au_size, 1); //全回放功能获取帧
            if (ret > 0) {
                au_size += ret;
            }
        }
        if (au_size > 0) {
            mp3_buf_play_file(audio_buf, au_size);
            AVI_FACE.play_audio = true;
        }
    }
#ifdef CONFIG_LVGL_UI_ENABLE
    lcd_lvgl_able(0);//禁用LVGL接口
#endif

#ifdef CONFIG_UI_MIRROR_EYE
    char mirror_enbale = 0;
    for (int i = 0; i < ARRAY_SIZE(avi_mirror_index); i++) { //查找需要镜像的index
        if (avi_mirror_index[i] == index) {
            mirror_enbale = true;
            break;
        }
    }
    lcd_show_mirror1_set(mirror_enbale);
#endif
    printf("avi file open : %s !!!\n", name);
    num = cnt = 0;
    while (AVI_FACE.task_init == TASK_CREATE) {
        ret = avi_video_get_frame(fd, ++num, fbuf, fbuflen, 1); //全回放功能获取帧
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

#if (USE_LCD_TE_ROTATE_ENABLE && USE_LCD_TE && (__SDRAM_SIZE__ >= (8*1024*1024))) //使用TE信号解决播放视频撕裂感
                int w, h;
                YUV420p_REVERSAL(yuv, yuv_rev_buf, info.width, info.height, &w, &h, 270);//视频缓存需要翻转的角度
                yuv420p_quto_rgb565(yuv_rev_buf, rgb565, info.height, info.width, 0);//YUV转RGB
                lcd_show_frame_to_dev_flip(rgb565, info.width * info.height * 2, 0);//视频缓存数据翻转后推屏，并设置屏幕不需要翻，即屏幕翻转0度
#else
                yuv420p_quto_rgb565(yuv, rgb565, info.width, info.height, 0);
                lcd_show_frame_to_dev(rgb565, info.width * info.height * 2);
#endif
                if (AVI_FACE.task_suspend) {
                    goto uninit_waite;
                }
                if (index != AVI_FACE.avi_index) {
                    sprintf(name, GIF_FILE_PATH, AVI_FACE.avi_index);
                    fd1 = fopen(name, "r");
                    if (fd1) {
                        fclose(fd1);
                        goto uninit_redo;
                    }
                    if (AVI_FACE.avi_index == AI_UART_CMD_DIALUOGE_PLAY_END) {
                        AVI_FACE.avi_index = index = AI_UART_CMD_EMOJI_DATA;
                    } else {
                        AVI_FACE.avi_index = index;
                    }
                }
                if (AVI_FACE.lock_index > 0 && AVI_FACE.avi_index != (AVI_FACE.lock_index - 1)) {
                    AVI_FACE.avi_index  = AVI_FACE.lock_index - 1;
                }
                os_time_dly(AVI_FACE.play_speed);
                if (AVI_FACE.task_init == TASK_KILL) {
                    goto exit;
                }
            }
        } else {
            if (num < video_chunk) {
                printf("avi get video err, num = %d\n", num);
                continue;
            }
            if (AVI_FACE.lock_index > 0 && index == AVI_FACE.avi_index && index == (AVI_FACE.lock_index - 1)) {
                num = 0;
                if (au_size > 0) {
                    mp3_buf_play_file(audio_buf, au_size);
                    AVI_FACE.play_audio = true;
                }
                printf("avi play loop\n");
                continue;
                /*
                if (os_sem_pend(&AVI_FACE.sem, 100 * 60 * 10)) {//10分钟延时
                    AVI_FACE.lock_index = 0;
                }*/
            }
            printf("avi play end\n");
uninit_redo:
            num = 0;
            avi_net_unpkg_exit(fd, 1);
            if (fd) {
                fclose(fd);
                fd = NULL;
            }
            if (AVI_FACE.task_suspend) {
                goto uninit_waite;
            }
            if (AVI_FACE.lock_index > 0 && AVI_FACE.avi_index != (AVI_FACE.lock_index - 1)) {
                AVI_FACE.avi_index  = AVI_FACE.lock_index - 1;
            }
            if (index != AVI_FACE.avi_index) {
                sprintf(name, GIF_FILE_PATH, AVI_FACE.avi_index);
                fd1 = fopen(name, "r");
                if (fd1) {
                    fclose(fd1);
                    index = AVI_FACE.avi_index;
                }
                if (AVI_FACE.avi_index == AI_UART_CMD_DIALUOGE_PLAY_END) {
                    AVI_FACE.avi_index = index = AI_UART_CMD_EMOJI_DATA;
                } else {
                    AVI_FACE.avi_index = index;
                }
            } else {
                if (AVI_FACE.loop_disable) {
                    goto exit;
                }
                cnt++;
                if (index > AI_UART_CMD_EMOJI_REST && index < AI_UART_CMD_DIALUOGE_START) {
                    index = AI_UART_CMD_EMOJI_DATA;
                    goto redo;
                } else if (index != AI_UART_CMD_EMOJI_REST && index != AI_UART_CMD_WIFI_CONFIG && index != AI_UART_CMD_MUSIC_START && index != AI_UART_CMD_NET_MUSIC_PLAY) {
                    if (net_music_play_start_status()) {
                        index = AI_UART_CMD_NET_MUSIC_PLAY;
                    } else {
                        index = AI_UART_CMD_EMOJI_DATA;
                    }
                }
            }
            if (last_index == index && index == AI_UART_CMD_EMOJI_DATA) {
                if (AVI_FACE.task_init == TASK_KILL) {
                    goto exit;
                }
                os_sem_pend(&AVI_FACE.sem, 300);//3秒延时
                if (AVI_FACE.task_init == TASK_KILL) {
                    goto exit;
                }
                if (index != AVI_FACE.avi_index) {
                    sprintf(name, GIF_FILE_PATH, AVI_FACE.avi_index);
                    fd1 = fopen(name, "r");
                    if (fd1) {
                        fclose(fd1);
                        index = AVI_FACE.avi_index;
                    } else if (AVI_FACE.avi_index == AI_UART_CMD_DIALUOGE_PLAY_END) {
                        AVI_FACE.avi_index = index = AI_UART_CMD_EMOJI_DATA;
                    } else {
                        AVI_FACE.avi_index = index;
                        if (net_music_play_start_status()) {
                            index = AI_UART_CMD_NET_MUSIC_PLAY;
                        } else if (AVI_FACE.avi_index == AI_UART_CMD_NET_MUSIC_PLAY) {
                            AVI_FACE.avi_index = AI_UART_CMD_EMOJI_DATA;
                        }
                    }
                    sprintf(name, GIF_FILE_PATH, AVI_FACE.avi_index);
                    fd1 = fopen(name, "r");
                    if (fd1) {
                        fclose(fd1);
                        index = AVI_FACE.avi_index;
                    } else {
                        AVI_FACE.avi_index = AI_UART_CMD_EMOJI_DATA;
                        index = AVI_FACE.avi_index;
                    }
                }
            }
            goto redo;
uninit_waite:
            num = 0;
            avi_net_unpkg_exit(fd, 1);
            if (fd) {
                fclose(fd);
                fd = NULL;
            }
            AVI_FACE.task_suspend = 2;
            if (AVI_FACE.play_audio == true) {
                mp3_buf_play_stop(NULL);
                AVI_FACE.play_audio = 0;
            }
            if (AVI_FACE.task_init == TASK_KILL) {
                goto exit;
            }
            os_sem_pend(&AVI_FACE.sem, 0);//延时
            if (AVI_FACE.task_init == TASK_KILL) {
                goto exit;
            }
            AVI_FACE.task_suspend = 0;
            AVI_FACE.loop_disable = 0;
            goto redo;
        }
    }
exit:
    avi_net_unpkg_exit(fd, 1);
    if (fh) {
        jpg_dec_close(fh);
    }
    if (fd) {
        fclose(fd);
    }
    if (yuv && yuv != yuv_buf) {
        free(yuv);
    }
    if (fbuf && fbuf != JPEG_BUF) {
        free(fbuf);
    }
    if (rgb565 && rgb565 != rgb_buf) {
        free(rgb565);
    }
    if (audio_buf) {
        mp3_buf_play_stop(NULL);
        free(audio_buf);
        AVI_FACE.play_audio = 0;
    }
    AVI_FACE.task_init = TASK_INIT;
    AVI_FACE.loop_disable = 0;
#ifdef CONFIG_LVGL_UI_ENABLE
    lvgl_flush_enable();
#endif
    return 0;
}

int play_face_emoji_suspend(void)//挂起AVI文件播放
{
    int to = 100;
    os_sem_set(&AVI_FACE.sem, 0);//
    if (AVI_FACE.task_suspend == 2 || AVI_FACE.task_suspend == 0) {
        return 0;
    }
    AVI_FACE.task_suspend = 1;
    while (AVI_FACE.task_suspend == 1 && --to) {
        os_time_dly(2);
    }
    ASSERT(to, "waite face_suspend err");
#ifdef CONFIG_LVGL_UI_ENABLE
    lvgl_flush_enable();
#endif
    return 0;
}
int play_face_emoji_resum(void)//恢复AVI文件播放
{
    int to = 100;
    if (AVI_FACE.task_suspend == 2) {
        os_sem_post(&AVI_FACE.sem);//
        while (AVI_FACE.task_suspend == 2 && --to) {
            os_time_dly(2);
        }
    }//恢复AVI文件播放，会在播放任务自动禁止LVGL
    return 0;
}
int play_face_emoji_kill(void)//删除AVI文件播放
{
    if (AVI_FACE.play_audio == true) {
        mp3_buf_play_stop(NULL);
        AVI_FACE.play_audio = 0;
    }
    if (AVI_FACE.task_init == TASK_CREATE) {
        AVI_FACE.task_init = TASK_KILL;
        if (os_sem_valid(&AVI_FACE.sem)) {
            os_sem_post(&AVI_FACE.sem);
        }
        int to = 100;
        while (AVI_FACE.task_init != TASK_KILL && --to) {
            os_time_dly(2);
        }
        //删除任务后，AVI文件会在播放任务退出前恢复LVGL显示
        if (AVI_FACE.task_init != TASK_KILL) {
            printf("kill err\n");
            return -1;
        }
    }
    return 0;
}
void play_face_emoji_loop(int avi_index)//循环指定index AVI文件播放
{
    AVI_FACE.lock_index = avi_index >= 0 ? (avi_index + 1) : 0;
}
void play_face_emoji_loop_enable(int enable)//单个循环AVI文件播放，在播放后调用
{
    AVI_FACE.loop_disable = !enable;
}
void play_face_emoji_lock(int lock)//锁存AVI文件播放
{
    if (lock) {
        play_face_emoji_suspend();
        AVI_FACE.lock_index = 1;
    } else {
        AVI_FACE.lock_index = 0;
        play_face_emoji_resum();
    }
}
int play_face_emoji(int avi_index)//启动AVI文件播放
{
    char name[32];
    if (AVI_FACE.lock_index > 0) {
        return 0;
    }
    printf("-->avi_index = %d\n", avi_index);
    user_lcd_init();
    avi_index = avi_index == AI_UART_CMD_REC_END ? AI_UART_CMD_EMOJI_DATA : avi_index;
    if (AVI_FACE.task_init == TASK_INIT) {
        AVI_FACE.task_init = TASK_CREATE;
        AVI_FACE.play_speed = 5;
        os_sem_create(&AVI_FACE.sem, 0);
        sprintf(name, "avi_play_%d", rand() % 256);
        if (thread_fork(name, 10, 1024, 0, 0, avi_unpkg_decoder_to_play, (void *)avi_index)) {
            AVI_FACE.task_init = TASK_INIT;
            puts("err in avi_play");
            return -1;
        }
    } else {
        AVI_FACE.avi_index = avi_index;
        AVI_FACE.play_speed = 5;
        play_face_emoji_resum();
        if (os_sem_valid(&AVI_FACE.sem)) {
            os_sem_post(&AVI_FACE.sem);
        }
    }
    return 0;
}

int emoji_tts_callback(char *emoji)
{
    char *pdata = NULL;

    // 定义表情符号映射表
    struct EmojiMap {
        const char **strings;
        int index;
    };

    static const struct EmojiMap emoji_maps[] = {
        {happy_str, AI_UART_CMD_EMOJI_HAPPY},
        {smile_str, AI_UART_CMD_EMOJI_SMILE},
        {excited_str, AI_UART_CMD_EMOJI_EXCITE},
        {quite_str, AI_UART_CMD_EMOJI_QUIET},
        {think_str, AI_UART_CMD_EMOJI_THINK},
        {sad_str, AI_UART_CMD_EMOJI_SAD},
        {grievance_str, AI_UART_CMD_EMOJI_GIEVAN},
        {angry_str, AI_UART_CMD_EMOJI_ANGRY},
        {whiny_str, AI_UART_CMD_EMOJI_FRET},
        {surprise_str, AI_UART_CMD_EMOJI_AMAZE},
        {confused_str, AI_UART_CMD_EMOJI_DOUBT},
        {shy_str, AI_UART_CMD_EMOJI_SHY},
        {sleep_str, AI_UART_CMD_EMOJI_SLEEP},
        {enjoy_str, AI_UART_CMD_EMOJI_REVEL},
        {naughty_str, AI_UART_CMD_EMOJI_NAUGHTY},
        {fear_str, AI_UART_CMD_EMOJI_FEAR},
        {proud_str, AI_UART_CMD_EMOJI_PROUD},
        {depressed_str, AI_UART_CMD_EMOJI_DEPPRESS,},
        {desire_str, AI_UART_CMD_EMOJI_DESIRE},
        {gentle_str, AI_UART_CMD_EMOJI_GENTLE},
        {curiosity_str, AI_UART_CMD_EMOJI_INQUIST},
        {cute_str, AI_UART_CMD_EMOJI_CUTE},
        {cool_str, AI_UART_CMD_EMOJI_COLL},
        {embarrassed_str, AI_UART_CMD_EMOJI_EMBARRASSED},
        {fantasy_str, AI_UART_CMD_EMOJI_FANTASY},
        {dizzy_str, AI_UART_CMD_EMOJI_DIZZY},
        {NULL, 0} // 结束标记
    };

    int emoji_index = 0;
    int i, j;

    // 遍历表情符号映射表
    for (i = 0; emoji_maps[i].strings != NULL; i++) {
        const char **str_list = emoji_maps[i].strings;
        // 遍历当前表情符号列表
        for (j = 0; str_list[j] != NULL; j++) {
            if (strstr(emoji, str_list[j])) {
                emoji_index = emoji_maps[i].index;
                goto exit;
            }
        }
    }
exit:
    if (emoji_index) {
        printf("-->emoji_index = %d\n", emoji_index);
        play_face_emoji(emoji_index);
        //play_face_emoji_loop_enable(0);
    }
    return 0;
}

#endif
#endif

