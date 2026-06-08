#include "app_config.h"
#include "system/includes.h"
#include "asm/clock.h"
#include "asm/irq.h"
#include "asm/gpio.h"
#include "asm/spi.h"
#include "asm/jpeg_codec.h"
#include "fs/fs.h"
#include "websocket_sxy_mutilmodal.h"
#include "printer_stepper_motor.h"
#include "lcd_config.h"

#ifdef CONFIG_QYAI_MUTILMODAL_ENABLE

int mutil_modal_stop(void);
void mutil_modal_img_callback_reg(void (*cb)(char *buf, int len));
int mutil_modal_request(enum Opt_mode_t Optmode, enum Render_mode_t Rendermode,
                        void *img_buf, int img_buf_size,
                        void *out_img_buf, int out_img_buf_size,
                        void *txt, int width, int height);
int printer_jpg_print(unsigned char *jpg, unsigned int jpg_len, unsigned char *raster_out, unsigned int raster_out_size);


enum {
    IMG_MODE_INIT = 0,
    IMG_MODE_LINE,      //线条
    IMG_MODE_ANIME,     //动漫
    IMG_MODE_REAL,      //真实
    IMG_MODE_ENTITY,    //实体

    IMG_MODE_MAX,
};

static char img_mode = 0;
static char fore_mode = 0;
static char printf_buf[LCD_W * PRINTER_ONELINE_POINT_DATA_BYTES * 8 / LCD_H * PRINTER_ONELINE_POINT_DATA_BYTES];
static int printf_len = 0;

//对话没听清回调
void  qyai_ai_dialogue_no_asr_callback(char *buf, int len)
{
    //screen_main_set_bg_img(4);
    //screen_main_set_textual_lab("不好意思，没听清，请重新说一遍");//lvgl显示文本
}

//打印图片
void qyai_print_img(void)
{
    // 这里执行打印机打印数据
    if (printf_len > 0) {
        printer_test_print_image(printf_buf, printf_len);
    }
}

//图片生成结果打印
void qyai_text2img_callback(unsigned char *buf, int len)
{
    printf_len = printer_jpg_print(buf, len, printf_buf, sizeof(printf_buf));//图片缩放、转换、打印在该函数完成
    //qyai_print_img();
}

//设定图片生成的模式
void qyai_text2img_mode(char mode, char force)
{
    img_mode = mode;
    fore_mode = force;
}

//设定图片生成的循环模式
void qyai_text2img_cyc_mode_auto(char force)
{
    ++img_mode;
    img_mode = img_mode >= IMG_MODE_MAX ? IMG_MODE_LINE : img_mode;
}

//文本生成图片
int qyai_text2img(char *text)
{
    if (!fore_mode && !strstr(text, "打印") && !strstr(text, "生成一张")) {
        return 0;
    }
    //screen_main_set_textual_lab(text);//lvgl显示文本

#define IMAGE_SIZE  (50 * 1024)
#define TEXT_SIZE   (4 * 1024)
    static char *imag_buf = NULL;
    static char *text_buf = NULL;
    if (!imag_buf) {
        imag_buf = malloc(IMAGE_SIZE);
    }
    if (!text_buf) {
        text_buf = malloc(TEXT_SIZE);
    }
    switch (img_mode) {
    case IMG_MODE_LINE:
        snprintf(text_buf, TEXT_SIZE, "生成一张%s的图片，风格为简洁、纯线条、白色背景", text);
        break;
    case IMG_MODE_ANIME:
        snprintf(text_buf, TEXT_SIZE, "生成一张%s的图片，风格为简洁、纯动漫、白色背景", text);
        break;
    case IMG_MODE_REAL:
        snprintf(text_buf, TEXT_SIZE, "生成一张%s的图片，风格为真实场景", text);
        break;
    case IMG_MODE_ENTITY:
        snprintf(text_buf, TEXT_SIZE, "生成一张%s的图片，风格为简洁、纯实体、白色背景", text);
        break;
    default:
        snprintf(text_buf, TEXT_SIZE, "生成一张%s的图片，风格为简洁、纯线条、白色背景", text);
        break;
    }
    if (text && imag_buf && text_buf) {
        mutil_modal_img_callback_reg(qyai_text2img_callback);
        mutil_modal_request(OPT_MODE_TEXT2IMG, RENDER_MODE_REAL, NULL, NULL, imag_buf, IMAGE_SIZE, text_buf, 320, 256);
    }
    return 1;
}

#endif // CONFIG_QYAI_MUTILMODAL_ENABLE



