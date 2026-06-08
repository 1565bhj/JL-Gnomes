#include "app_config.h"
#include "ai_uart_ctrol.h"

#ifdef CONFIG_UI_ENABLE
#ifdef CONFIG_UI_GIF_EYE

#include "GUI_GIF_Private.h"
#include "lcd_drive.h"
#include "lcd_config.h"
#include "server/video_dec_server.h"//fopen
#include "sys_common.h" //flen()

static struct gif_eye {
    char *fdata;
    char *rgb_data;
    unsigned int gif_index;
    unsigned int lock_index;
    unsigned char task_init;
    unsigned char task_suspend;
    unsigned char play_speed;
    FILE *gif_fd;
    OS_SEM sem;
} GIF_EYE[2] = {0};
enum {
    TASK_INIT = 0,
    TASK_CREATE,
    TASK_KILL,
};
#define GIF_FILE_SIZE_MAX (500*1024) //整个gif大小不能超过500KB

#ifndef USE_LCD_TE_ROTATE_ENABLE
#define USE_LCD_TE_ROTATE_ENABLE    0 //是否开启TE使用时播放视频翻转视频数据
#endif

#define UI_RES_SD_ENABLE        0   //1：UI资源放在SD卡,0：UI资源放在flash
#define UI_RES_USE_ANIME        0   //1：使用动漫两个眼睛
#define UI_EMOJI_ANIME          0   //1：使用动漫有背景的表情

#if UI_RES_USE_ANIME
#define GIF_FILE_MAX_NUM    8
#if UI_RES_SD_ENABLE
#define  GIF_FILE_PATH   CONFIG_ROOT_PATH"emoji-2eye/L/%d.gif" //SD卡读取
//#define  GIF_FILE_PATH   CONFIG_ROOT_PATH"emoji-2eye/R/%d.gif" //SD卡读取
#else
#define  GIF_FILE_PATH   CONFIG_UI_RES_FILE_PATH"L%d.gif" //SD卡读取
//#define  GIF_FILE_PATH   CONFIG_UI_RES_FILE_PATH"R%d.gif" //SD卡读取
#endif
#elif UI_EMOJI_ANIME
#define GIF_FILE_MAX_NUM    8
#if UI_RES_SD_ENABLE
#define  GIF_FILE_PATH   CONFIG_ROOT_PATH"emoji-anim/%d.gif" //SD卡读取
#else
#define  GIF_FILE_PATH   CONFIG_UI_RES_FILE_PATH"%d.gif" //SD卡读取
#endif

#else

#define GIF_FILE_MAX_NUM    13
#if UI_RES_SD_ENABLE
#define  GIF_FILE_PATH   CONFIG_ROOT_PATH"emoji-gif/%d.gif" //SD卡读取
#else
#define  GIF_FILE_PATH   CONFIG_UI_RES_FILE_PATH"%d.gif" //SD卡读取
#endif
#endif

#if (USE_LCD_TE_ROTATE_ENABLE && USE_LCD_TE && (__SDRAM_SIZE__ >= (8*1024*1024)))
static u8 rgb_rev_buf[LCD_W * LCD_H * 2] /*sec(.sram)*/ ALIGNE(32);//外部使用
#endif

#ifdef CONFIG_UI_MIRROR_EYE
//打开镜像时，需要设置镜像ID
static int gif_mirror_index[] = {
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


#if UI_RES_USE_ANIME
static const char mirror_id_table[] = { 1, 2, 3, 4, 5, 6, 7, 8};
#else
static const char mirror_id_table[] = { 5, 7, 8, 11};
#endif

extern void lcd_show_frame_to_dev(u8 *buf, u32 len);
extern void lcd_show_mirror1_set(char enable);
extern int GUI_GIF_GetInfoFileClose(GUI_GIF_IMAGE_INFO *pInfo);
extern int GUI_GIF_GetInfoFileOpen(const char *path, GUI_GIF_IMAGE_INFO *pInfo);
extern int Gif_to_PictureFile(GUI_GIF_IMAGE_INFO *Info, char *inbuf, int type, int Index);

static int gif_play_to_lcd_task(void *priv)
{
    GUI_GIF_IMAGE_INFO info = {0};
    char gif_path[64];
    char *buf = NULL;
    FILE *gif_fd = NULL;
    u32 gif_len;
    int index = (char)priv;
    int gif_index;
    char play_speed;
    int mirror_id;
    int mirror_last_id;

    if (is_production_test_enter(0)) {
        goto _exit;
    }

#if UI_RES_SD_ENABLE
    while (!storage_device_ready()) {
        os_time_dly(50);
    }
#endif

_init:
    play_speed = GIF_EYE[index].play_speed;
    gif_index = GIF_EYE[index].gif_index;
    gif_index = gif_index > GIF_FILE_MAX_NUM ? 0 : gif_index;
    gif_index = gif_index < 0 ? GIF_FILE_MAX_NUM : gif_index;
    gif_index = gif_index == 0 ? 6 : gif_index;
    GIF_EYE[index].gif_index = gif_index;
    index = index ? 1 : 0;

    sprintf(gif_path, GIF_FILE_PATH, gif_index);
    printf("-->fopen file : %s\n", gif_path);

    GIF_EYE[index].gif_fd = fopen(gif_path, "r");
    if (!GIF_EYE[index].gif_fd) {
        printf("-->fopen file err : %s\n", gif_path);
        goto _exit;
    }

    gif_fd = GIF_EYE[index].gif_fd;
    gif_len = flen(gif_fd);
    if (gif_len > GIF_FILE_SIZE_MAX) {
        printf("err gif flen too large\n");
        goto _exit;
    }
    if (!GIF_EYE[index].fdata) {
        GIF_EYE[index].fdata = malloc(GIF_FILE_SIZE_MAX);
    }
    if (!GIF_EYE[index].fdata) {
        printf("err gif flen too large\n");
        goto _exit;
    }
    buf = GIF_EYE[index].fdata;

    fread(buf, gif_len, 1, gif_fd);
    fclose(gif_fd);

    GIF_EYE[index].gif_fd = NULL;

    GUI_GIF_GetInfo(buf, gif_len, &info);   //获取图片信息

//    printf("--> line = %d\n",__LINE__);
    if (!GIF_EYE[index].rgb_data) {
        GIF_EYE[index].rgb_data = malloc(info.xSize * info.ySize * 2);
    }
    if (!GIF_EYE[index].rgb_data) {
        printf("err malloc gif rgb buf\n");
        goto _exit;
    }
    u8 *rgb565_buf = GIF_EYE[index].rgb_data;
    if (info.xSize <= lcd_w && info.ySize <= lcd_h) {
        lcd_set_draw_area(0, info.xSize - 1, 0, info.ySize - 1);
    } else { //比屏大的GIF需要自行缩放 我们提供了工具进行缩放GiF_Cat
        printf("err in gif size %d > %d, %d > %d", info.xSize, lcd_w, info.ySize, lcd_h);
        goto _exit;
    }
    printf("-->gif wsize = %d  , hsize = %d NumImages = %d  delay = %d\n", info.xSize, info.ySize, info.NumImages, info.Delay);
#ifdef CONFIG_UI_MIRROR_EYE
    char mirror_enbale = 0;
    for (int i = 0; i < ARRAY_SIZE(gif_mirror_index); i++) { //查找需要镜像的index
        if (gif_mirror_index[i] == gif_index) {
            mirror_enbale = true;
            break;
        }
    }
    lcd_show_mirror1_set(mirror_enbale);
#endif
    while (1) {
        for (int num = 0; num < info.NumImages; num++) {
            if (Gif_to_Picture(buf, gif_len, &info, rgb565_buf, 2, num)) {
                printf("err in Gif_to_Picture\n");
                goto _exit;
            }
#if (USE_LCD_TE_ROTATE_ENABLE && USE_LCD_TE && (__SDRAM_SIZE__ >= (8*1024*1024))) //使用TE信号解决播放视频撕裂感
            unsigned short *r_datao = rgb_rev_buf;
            int angle = 270;//数据翻转270度

            switch (angle) { //rgb565翻转
            case 90:
                for (int i = 0; i < info.width; i++) {//90
                    for (int j = info.height - 1; j >= 0; j--) {
                        *r_datao = *((unsigned short *)((unsigned int)rgb565 + ((unsigned int)info.width * j + (unsigned int)i) * 2);
                    }
                }
                break;
            case 180:
                for (int i = 0; i < info.height ; i++) {//180
                    for (int j = 0; j < info.width; j++) {
                        *r_datao = *((unsigned short *)((unsigned int)rgb565 + ((unsigned int)info.width * i + (unsigned int)j) * 2);
                    }
                }
                break;
            case 270:
                for (int i = info.width - 1; i >= 0; i--) {//270
                    for (int j = 0; j < info.height; j++) {
                        *r_datao = *((unsigned short *)((unsigned int)rgb565 + ((unsigned int)info.width * j + (unsigned int)i) * 2);
                    }
                }
                break;
            }
            lcd_show_frame_to_dev_flip(rgb_rev_buf, info.width * info.height * 2, 0);//视频缓存数据翻转后推屏，并设置屏幕不需要翻，即屏幕翻转0度
#else
            lcd_show_frame_to_dev(rgb565_buf, info.xSize * info.ySize * 2);
#endif
            if (GIF_EYE[index].task_suspend || GIF_EYE[index].task_init == TASK_KILL) {
                goto _exit;
            }

            if (play_speed == 0) {
                os_time_dly(info.Delay);
            } else {
                os_time_dly(play_speed);
            }
            if (GIF_EYE[index].lock_index > 0 && GIF_EYE[index].gif_index != (GIF_EYE[index].lock_index - 1)) {
                GIF_EYE[index].gif_index  = GIF_EYE[index].lock_index - 1;
            }
            if (gif_index != GIF_EYE[index].gif_index) {
                gif_index = GIF_EYE[index].gif_index;
                goto _init;
            }
        }
        if (GIF_EYE[index].lock_index > 0 && gif_index == GIF_EYE[index].gif_index && gif_index == (GIF_EYE[index].lock_index - 1)) {
            printf("gif play loop\n");
            continue;
        }
//        GIF_EYE[index].gif_index++;
//        GIF_EYE[index].gif_index = rand() % GIF_FILE_MAX_NUM + 1;
        printf("--> next file\n");
        goto _init;
    }
_exit:
    os_time_dly(5);//等待屏幕数据发送完再释放内存
    if (GIF_EYE[index].fdata) {
        free(GIF_EYE[index].fdata);
        GIF_EYE[index].fdata = NULL;
    }
    if (GIF_EYE[index].rgb_data) {
        free(GIF_EYE[index].rgb_data);
        GIF_EYE[index].rgb_data = NULL;
    }
    if (GIF_EYE[index].gif_fd) {
        fclose(GIF_EYE[index].gif_fd);
        GIF_EYE[index].gif_fd = NULL;
    }
    if (GIF_EYE[index].task_suspend) {
        if (GIF_EYE[index].task_init == TASK_KILL) {
            goto _uninit;
        }
        os_sem_pend(&GIF_EYE[index].sem, 0);//延时
        if (GIF_EYE[index].task_init == TASK_KILL) {
            goto _uninit;
        }
        GIF_EYE[index].task_suspend = 0;
        goto _init;
    }
_uninit:
    GIF_EYE[index].task_init = TASK_INIT;
    return 0;
}
int play_face_emoji_suspend(void)//挂起AVI文件播放
{
    int to = 100;
    int index = 0;
    os_sem_set(&GIF_EYE[index].sem, 0);//
    if (GIF_EYE[index].task_suspend == 2 || GIF_EYE[index].task_suspend == 0) {
        return 0;
    }
    GIF_EYE[index].task_suspend = 1;
    while (GIF_EYE[index].task_suspend == 1 && --to) {
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
    int index = 0;
    if (GIF_EYE[index].task_suspend == 2) {
        os_sem_post(&GIF_EYE[index].sem);//
        while (GIF_EYE[index].task_suspend == 2 && --to) {
            os_time_dly(2);
        }
    }//恢复AVI文件播放，会在播放任务自动禁止LVGL
    return 0;
}
void play_face_emoji_loop(int gif_index)//锁存AVI文件播放
{
    int index = 0;
    GIF_EYE[index].lock_index = gif_index >= 0 ? (gif_index + 1) : 0;
}
void play_face_emoji_lock(int lock)//锁存AVI文件播放
{
    int index = 0;
    if (lock) {
        play_face_emoji_suspend();
        GIF_EYE[index].lock_index = 1;
    } else {
        GIF_EYE[index].lock_index = 0;
        play_face_emoji_resum();
    }
}
int play_face_emoji_kill(void)//删除AVI文件播放
{
    int index = 0;
    if (GIF_EYE[index].task_init == TASK_CREATE) {
        GIF_EYE[index].task_init = TASK_KILL;
        if (os_sem_valid(&GIF_EYE[index].sem)) {
            os_sem_post(&GIF_EYE[index].sem);
        }
        int to = 100;
        while (GIF_EYE[index].task_init != TASK_KILL && --to) {
            os_time_dly(2);
        }
        //删除任务后，AVI文件会在播放任务退出前恢复LVGL显示
        if (GIF_EYE[index].task_init != TASK_KILL) {
            printf("kill err\n");
            return -1;
        }
    }
    return 0;
}
int play_face_emoji(int gif_index)
{
    char name[32];
    int index = 0;
    if (GIF_EYE[index].lock_index > 0) {
        return 0;
    }
    user_lcd_init();
    if (!GIF_EYE[index].task_init) {
        GIF_EYE[index].task_init = TASK_CREATE;
        GIF_EYE[index].play_speed = 5;
        sprintf(name, "gif_play%d", index);
        if (thread_fork(name, 10, 1024, 0, 0, gif_play_to_lcd_task, (void *)index)) {
            GIF_EYE[index].task_init = TASK_INIT;
            puts("err in gif_play");
            return -1;
        }
    } else {
        GIF_EYE[index].gif_index = gif_index;
        GIF_EYE[index].play_speed = 5;
        play_face_emoji_resum();
        if (os_sem_valid(&GIF_EYE[index].sem)) {
            os_sem_post(&GIF_EYE[index].sem);
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

