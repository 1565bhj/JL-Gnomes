#ifndef __LCD_CONFIG_H__
#define __LCD_CONFIG_H__
#include "app_config.h"

#if HORIZONTAL_SCREEN
#ifndef LCD_W
#define LCD_W     320
#define LCD_H     240
#endif
#else
#ifndef LCD_W
#define LCD_W     240
#define LCD_H     320
#endif
#endif

#define LCD_YUV420_DATA_SIZE    LCD_W*LCD_H*3/2
#define LCD_RGB565_DATA_SIZE    LCD_W*LCD_H*2
#define LCD_RGB888_DATA_SIZE    LCD_W*LCD_H*3

#endif

