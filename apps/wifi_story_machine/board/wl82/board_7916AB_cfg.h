#ifndef CONFIG_BOARD_7916AB_CFG_H
#define CONFIG_BOARD_7916AB_CFG_H

#ifdef CONFIG_BOARD_7916AB

#include "board_7916A_demo_cfg.h"
#include "board_7916A_dev_kit_cfg.h"
#include "board_7916A_develop_cfg.h"

#ifndef __FLASH_SIZE__
#define __FLASH_SIZE__    (8 * 1024 * 1024)
#define __SDRAM_SIZE__    (8 * 1024 * 1024)
#endif

#ifdef CONFIG_BOARD_MUTEX
#error "board config can not enable at the same time, just select one!!!"
#else
#define CONFIG_BOARD_MUTEX
#endif

#endif

#endif
