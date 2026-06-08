#include "app_config.h"
#include "system/includes.h"
#include "asm/clock.h"
#include "asm/irq.h"
#include "asm/gpio.h"
#include "asm/spi.h"
#include "asm/jpeg_codec.h"
#include "fs/fs.h"
#include "yuv_soft_scalling.h"
#include "yuv_to_rgb.h"
#include "printer_stepper_motor.h"
#include "lcd_config.h"

/* LCD 显示目标分辨率 */
#define LCD_DST_W       LCD_W
#define LCD_DST_H       LCD_H

/* 数据缓冲区长度（字节），与 PRINTER_CLOCK_CYCLE_COUNT/8 一致 */
#define PRINTER_TIMER_DATA_BYTES  48
#define PRINTER_HEAD_WIDTH_PX  (PRINTER_TIMER_DATA_BYTES * 8)  /* 384 = 48*8，打印头宽度(点) */
#define JPG_BINARIZE_THRESHOLD  150   /* 二值化阈值：Y < 此值视为黑(打点)，可调 80~180 */

/* 纵向分辨率倍率 = NUM/DENOM，直接增加内容行数（双线性插值生成唯一行，非简单重复）
 * 1/1=原始比例  2/1=2倍细腻  3/1=3倍  值越大图片越高越细腻，但打印时间和内存成正比增加 */
#define PRINT_VSCALE_NUM    3
#define PRINT_VSCALE_DENOM  3


/* 缓冲区最大容量（首次 malloc，后续 memset 清零复用，永不释放） */
#define YUV_DEC_MAX     (LCD_DST_W * LCD_DST_W * 3)            /* JPEG 解码输出 YUV，按 YUV444 最大情况预留 */
#define YUV_SCALED_MAX  (LCD_DST_W * LCD_DST_H * 3 / 2)  /* LCD 缩放后的 YUV420 */
#define Y_SCALED_MAX    (PRINTER_HEAD_WIDTH_PX * 512)    /* 打印用 Y 平面缩放缓冲区 */

static u8 *s_yuv_dec    = NULL;   /* JPEG 解码输出 YUV 缓冲区 */
static u8 *s_yuv_scaled = NULL;   /* LCD 缩放 YUV420 缓冲区 */
static u8 *s_y_scaled   = NULL;   /* 打印 Y 平面缩放缓冲区 */
static u8 ALIGNED(32) s_rgb565_tmp[LCD_DST_W * LCD_DST_H * 2];   /* RGB565 中间缓冲区 */
static u8 ALIGNED(32) s_rgb565a_buf[LCD_DST_W * LCD_DST_H * 3];  /* RGB565+Alpha 最终输出给 LVGL */

//送数据到LVGL进行推屏，数据格式RGB565A带透明度格式
void lv_demo_screen_main_show_print_img(char *buf, int len)
{
    //screen_main_set_textual_lab("");//清空文本显示
    //screen_main_show_print_img(buf);//LVGL空间显示图片
}

/**
 * @brief  将 JPG 图片解码并转换为热敏打印点阵数据，同时生成 RGB565A 用于 LCD 显示
 *
 * @param  jpg         JPG 原始数据指针
 * @param  jpg_len     JPG 数据长度（字节）
 * @param  raster_out  输出：二值化点阵缓冲区（每行 48 字节，1bit/像素，MSB 在左）
 * @param  raster_out_size  raster_out 缓冲区最大容量（字节）
 *
 * @return 0 成功，负值失败（-1 参数错误 -2 解析头失败 -3 尺寸异常
 *         -4 内存分配失败 -5 解码失败 -6 Y缩放缓冲区不足）
 */
int printer_jpg_print(unsigned char *jpg, unsigned int jpg_len, unsigned char *raster_out_buf, unsigned int raster_out_size)
{
    struct jpeg_image_info info = {0};
    struct jpeg_decode_req req = {0};
    unsigned char *raster_out = raster_out_buf;
    u8 *cy;
    u8 ytype;       /* YUV 采样因子：YUV444=1, YUV422=2, YUV420=4 */
    u32 pix, dec_size;
    u16 sw, sh;     /* 源图宽高 */
    u16 dw, dh;     /* 打印目标宽高 */
    u16 max_lines;
    u32 x;

    /* ① 参数校验 */
    if (!jpg || jpg_len < 32) {
        printf("[printer_jpg] err jpg param\n");
        return 0;
    }

    /* ② 首次调用分配缓冲区，后续直接复用 */
    if (!s_yuv_dec) {
        s_yuv_dec    = (u8 *)malloc(YUV_DEC_MAX);
    }
    if (!s_yuv_scaled) {
        s_yuv_scaled = (u8 *)malloc(YUV_SCALED_MAX);
    }
    if (!s_y_scaled) {
        s_y_scaled   = (u8 *)malloc(Y_SCALED_MAX);
    }
    if (!s_yuv_dec || !s_yuv_scaled || !s_y_scaled) {
        printf("[printer_jpg] malloc fail\n");
        return 0;
    }

    /* ③ 解析 JPEG 头信息，获取图片宽高和 YUV 采样格式 */
    info.input.data.buf = (u8 *)jpg;
    info.input.data.len = jpg_len;
    if (jpeg_decode_image_info(&info)) {
        printf("[printer_jpg] decode_info fail\n");
        return -2;
    }
    sw = info.width;
    sh = info.height;
    if (sw == 0 || sh == 0) {
        printf("[printer_jpg] width err\n");
        return 0;
    }
    printf("[printer_jpg] %dx%d\n", sw, sh);
    /* 根据采样格式确定 U/V 平面大小比例 */
    switch (info.sample_fmt) {
    case JPG_SAMP_FMT_YUV444:
        ytype = 1;
        break;
    case JPG_SAMP_FMT_YUV420:
        ytype = 4;
        break;
    default:
        ytype = 2;
        break;
    }
    pix = (u32)sw * sh;
    dec_size = pix + pix / ytype * 2;  /* Y + U + V 总大小 */
    if (dec_size > YUV_DEC_MAX) {
        printf("[printer_jpg] image too large: %u > %u\n", dec_size, (u32)YUV_DEC_MAX);
        return 0;
    }

    /* ④ 清零 YUV 解码缓冲区，硬件 JPEG 解码 → YUV */
    memset(s_yuv_dec, 0, dec_size);

    cy = s_yuv_dec;
    req.input_type     = JPEG_INPUT_TYPE_DATA;
    req.input.data.buf = (u8 *)jpg;
    req.input.data.len = jpg_len;
    req.buf_y          = cy;
    req.buf_u          = cy + pix;
    req.buf_v          = req.buf_u + pix / ytype;
    req.buf_width      = sw;
    req.buf_height     = sh;
    req.out_width      = sw;
    req.out_height     = sh;
    req.output_type    = JPEG_DECODE_TYPE_DEFAULT;
    req.bits_mode      = BITS_MODE_UNCACHE;
    req.dec_query_mode = 1;

    if (jpeg_decode_one_image(&req)) {
        printf("[printer_jpg] decode_one_image fail\n");
        return 0;
    }

    /* ⑤ 统一转换为 YUV420p（如果源格式不是 YUV420） */
    if (info.sample_fmt == JPG_SAMP_FMT_YUV444) {
        YUV444pToYUV420p(s_yuv_dec, s_yuv_dec, sw, sh);
    } else if (info.sample_fmt == JPG_SAMP_FMT_YUV422) {
        YUV422pToYUV420p(s_yuv_dec, s_yuv_dec, sw, sh);
    }

    /* ⑥ LCD 显示：YUV420 缩放到 320x240 → RGB565 Swap → 插入 Alpha → 送 LVGL 显示 */
    {
        memset(s_yuv_scaled, 0, YUV_SCALED_MAX);
        /* YUV420 缩放到 LCD 目标分辨率 */
        YUV420p_Soft_Scaling(s_yuv_dec, s_yuv_scaled, sw, sh, LCD_DST_W, LCD_DST_H);
        /* YUV420 → RGB565（大端/Swap，匹配 LV_COLOR_16_SWAP=1） */
        yuv420p_quto_rgb565(s_yuv_scaled, s_rgb565_tmp, LCD_DST_W, LCD_DST_H, 1);

        /* RGB565 → RGB565 + Alpha（CF_TRUE_COLOR_ALPHA 格式：每像素 3 字节） */
        u32 px_count = LCD_DST_W * LCD_DST_H;
        u8 *src = s_rgb565_tmp;
        u8 *dst = s_rgb565a_buf;
        for (u32 i = 0; i < px_count; i++) {
            *dst++ = *src++;   /* RGB565 高字节 */
            *dst++ = *src++;   /* RGB565 低字节 */
            *dst++ = 0xFF;     /* Alpha = 不透明 */
        }

        /* 送到 LVGL 显示（投递到 sys_timer 线程执行，LVGL 非线程安全） */
        lv_demo_screen_main_show_print_img(s_rgb565a_buf, px_count * 2);

        printf("[printer_jpg] RGB565A %dx%d ready, %d bytes\n",
               LCD_DST_W, LCD_DST_H, LCD_DST_W * LCD_DST_H * 3);
    }

    /* ⑦ 打印：顺时针旋转90°后目标尺寸
     *   旋转后：原始高度(sh) → 打印宽度(384)，原始宽度(sw) → 打印高度
     *   缩放比 = 384/sh，打印高度 = sw × (384/sh) × vscale */
    dw = PRINTER_HEAD_WIDTH_PX;
    dh = (u16)(((u32)sw * dw + sh / 2) / sh);
    dh = (u16)((u32)dh * PRINT_VSCALE_NUM / PRINT_VSCALE_DENOM);
    if (dh < 1) {
        dh = 1;
    }
    max_lines = (u16)(sw * PRINTER_ONELINE_POINT_DATA_BYTES * 8 / sh);

    if (!raster_out) {
        raster_out_size = PRINTER_TIMER_DATA_BYTES * max_lines;
        raster_out = malloc(raster_out_size);
        if (!raster_out) {
            printf("[printer_jpg] malloc %d err \n", raster_out_size);
            return 0;
        }
    } else {
        max_lines = (u16)(raster_out_size / PRINTER_ONELINE_POINT_DATA_BYTES);
        if (max_lines > (sw * PRINTER_ONELINE_POINT_DATA_BYTES * 8 / sh)) {
            max_lines = (u16)(sw * PRINTER_ONELINE_POINT_DATA_BYTES * 8 / sh);
        }
    }

    if (dh > max_lines) {
        dh = max_lines;
    }

    /* ⑧ 顺时针旋转90° + 双线性插值：
     *   print(y, x) ← original(sh-1 - x*(sh-1)/(dw-1),  y*(sw-1)/(dh-1))
     *   即：打印行(y) 对应原图列，打印列(x) 对应原图行（反向） */
    if ((u32)dw * dh > Y_SCALED_MAX) {
        printf("[printer_jpg] Y buf overflow: %u > %u\n", (u32)dw * dh, (u32)Y_SCALED_MAX);
        return 0;
    }
    {
        const u8 *y_plane = s_yuv_dec;
        u16 y, x;
        for (y = 0; y < dh; y++) {
            /* 打印行 y → 原图列 sx：0..dh-1 映射到 0..sw-1 */
            u32 sx_fp = (sw > 1 && dh > 1)
                        ? (u32)y * ((u32)(sw - 1) << 8) / (dh - 1) : 0;
            u16 sx0 = (u16)(sx_fp >> 8);
            u16 sx1 = sx0 + 1;
            if (sx1 >= sw) {
                sx1 = sw - 1;
            }
            u16 fx = (u16)(sx_fp & 0xFF);

            for (x = 0; x < dw; x++) {
                /* 打印列 x → 原图行 sy：0..dw-1 映射到 sh-1..0（顺时针翻转） */
                u32 sy_fp = (sh > 1 && dw > 1)
                            ? (u32)(dw - 1 - x) * ((u32)(sh - 1) << 8) / (dw - 1) : 0;
                u16 sy0 = (u16)(sy_fp >> 8);
                u16 sy1 = sy0 + 1;
                if (sy1 >= sh) {
                    sy1 = sh - 1;
                }
                u16 fy = (u16)(sy_fp & 0xFF);

                u8 p00 = y_plane[(u32)sy0 * sw + sx0];
                u8 p10 = y_plane[(u32)sy0 * sw + sx1];
                u8 p01 = y_plane[(u32)sy1 * sw + sx0];
                u8 p11 = y_plane[(u32)sy1 * sw + sx1];

                u32 top = ((u32)(256 - fx) * p00 + (u32)fx * p10 + 128) >> 8;
                u32 bot = ((u32)(256 - fx) * p01 + (u32)fx * p11 + 128) >> 8;
                s_y_scaled[(u32)y * dw + x] =
                    (u8)(((u32)(256 - fy) * top + (u32)fy * bot + 128) >> 8);
            }
        }
    }

    /* ⑧½ 对比度增强：按 1% 分位裁剪后拉伸灰度至 [0, 255] 全范围 */
    {
        u32 total_px = (u32)dw * dh;
        u32 hist[256];
        u32 i, sum;
        u8 lo = 0, hi = 255;

        memset(hist, 0, sizeof(hist));
        for (i = 0; i < total_px; i++) {
            hist[s_y_scaled[i]]++;
        }

        u32 clip = total_px / 100;
        sum = 0;
        for (i = 0; i < 256; i++) {
            sum += hist[i];
            if (sum > clip) {
                lo = (u8)i;
                break;
            }
        }
        sum = 0;
        for (i = 0; i < 256; i++) {
            sum += hist[255 - i];
            if (sum > clip) {
                hi = (u8)(255 - i);
                break;
            }
        }

        if (hi > lo + 4) {
            u16 range = hi - lo;
            for (i = 0; i < total_px; i++) {
                s16 v = (s16)s_y_scaled[i] - lo;
                if (v < 0) {
                    v = 0;
                }
                v = (s16)((u32)v * 255 / range);
                if (v > 255) {
                    v = 255;
                }
                s_y_scaled[i] = (u8)v;
            }
        }
    }

    /* ⑧¾ 白底/黑底净化：消除 JPEG 压缩噪点，避免抖动在纯白区域产生零星黑点 */
    {
        u32 total_px = (u32)dw * dh;
        u32 i;
        for (i = 0; i < total_px; i++) {
            if (s_y_scaled[i] >= 240) {
                s_y_scaled[i] = 255;
            } else if (s_y_scaled[i] <= 15) {
                s_y_scaled[i] = 0;
            }
        }
    }

    /* ⑨ Floyd-Steinberg 抖动 + 打包：灰度打印，每行 48 字节，MSB 为左
     *    dh 已含纵向倍率，每行都是双线性插值生成的唯一内容 */
    cy = s_y_scaled;
    {
        s16 err_cur[PRINTER_HEAD_WIDTH_PX + 2];
        s16 err_nxt[PRINTER_HEAD_WIDTH_PX + 2];
        u8  line_buf[PRINTER_ONELINE_POINT_DATA_BYTES];
        u16 y;
        s16 val, err;

        memset(err_cur, 0, sizeof(err_cur));

        for (y = 0; y < dh; y++) {
            const u8 *row = cy + (u32)y * dw;
            memset(err_nxt, 0, sizeof(err_nxt));
            memset(line_buf, 0, sizeof(line_buf));

            for (x = 0; x < (u32)dw; x++) {
                val = (s16)row[x] + err_cur[x + 1];
                if (val < 0) {
                    val = 0;
                }
                if (val > 255) {
                    val = 255;
                }

                if (val < 128) {
                    line_buf[x >> 3] |= (u8)(0x80 >> (x & 7));
                    err = val;
                } else {
                    err = val - 255;
                }
                s16 e7 = err * 7 / 16;
                s16 e3 = err * 3 / 16;
                s16 e5 = err * 5 / 16;
                s16 e1 = err - e7 - e3 - e5;
                err_cur[x + 2] += e7;
                err_nxt[x]     += e3;
                err_nxt[x + 1] += e5;
                err_nxt[x + 2] += e1;
            }

            memcpy(err_cur, err_nxt, sizeof(err_cur));
            memcpy(raster_out + (u32)y * PRINTER_ONELINE_POINT_DATA_BYTES,
                   line_buf, PRINTER_ONELINE_POINT_DATA_BYTES);
        }
    }
    printf("[printer_jpg] ok %ux%u ->旋转90° 灰度抖动 %ux%u, (vscale x%d/%d)\n",
           sw, sh, dw, dh, PRINT_VSCALE_NUM, PRINT_VSCALE_DENOM);

    if (raster_out && !raster_out_buf) {
        free(raster_out);
    }
    return dh * PRINTER_ONELINE_POINT_DATA_BYTES;
}




