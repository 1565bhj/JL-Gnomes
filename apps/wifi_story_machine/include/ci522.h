/*********************************************************
*  @file    app_demo_socket.h
*  @brief   ML307R OpenCPU app_demo_socket header file
*  Copyright (c) 2019 China Mobile IOT.
*  All rights reserved.
*  created by Yangqiang
********************************************************/
#ifndef __APP_DEMO_CI522_H__
#define __APP_DEMO_CI522_H__

#include "device/gpio.h"
#include "asm/spi.h"
//#include "hal_ml307r.h"

/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
**                             CI522 寄存器地址                                *
**----------------------------------------------------------------------------*/
//Page 0: Command and status
#define    RFU00                0x00
#define    CommandReg           0x01
#define    ComIEnReg            0x02
#define    DivIEnReg            0x03
#define    ComIrqReg            0x04
#define    DivIrqReg            0x05
#define    ErrorReg             0x06
#define    Status1Reg           0x07
#define    Status2Reg           0x08
#define    FIFODataReg          0x09
#define    FIFOLevelReg         0x0A
#define    WaterLevelReg        0x0B
#define    ControlReg           0x0C
#define    BitFramingReg        0x0D
#define    CollReg              0x0E
#define    ACDConfigReg         0x0F

//Page 1: Command
#define    RFU10                0x10
#define    ModeReg              0x11
#define    TxModeReg            0x12
#define    RxModeReg            0x13
#define    TxControlReg         0x14
#define    TxASKReg             0x15
#define    TxSelReg             0x16
#define    RxSelReg             0x17
#define    RxThresholdReg       0x18
#define    DemodReg             0x19
#define    RFU1A                0x1A
#define    RFU1B                0x1B
#define    MfTxReg              0x1C
#define    MfRxReg              0x1D
#define    RFU1E                0x1E
#define    SerialSpeedReg       0x1F

// Page 2: Configuration
#define    ACDConfigSelReg      0x20
#define    CRCResultRegH        0x21
#define    CRCResultRegL        0x22
#define    RFU23                0x23
#define    ModWidthReg          0x24
#define    RFU25                0x25
#define    RFCfgReg             0x26
#define    GsNReg               0x27
#define    CWGsPReg             0x28
#define    ModGsPReg            0x29
#define    TModeReg             0x2A
#define    TPrescalerReg        0x2B
#define    TReloadRegH          0x2C
#define    TReloadRegL          0x2D
#define    TCounterValueRegH    0x2E
#define    TCounterValueRegL    0x2F

//Page 3: Test Register
#define    RFU30                0x30
#define    TestSel1Reg          0x31
#define    TestSel2Reg          0x32
#define    TestPinEnReg         0x33
#define    TestPinValueReg      0x34
#define    TestBusReg           0x35
#define    AutoTestReg          0x36
#define    VersionReg           0x37
#define    AnalogTestReg        0x38
#define    TestDAC1Reg          0x39
#define    TestDAC2Reg          0x3A
#define    TestADCReg           0x3B
#define    RFU3C                0x3C
#define    RFU3D                0x3D
#define    RFU3E                0x3E
#define    RFU3F                0x3F

//CI522命令字
#define PCD_IDLE              0x00               //取消当前命令
#define PCD_AUTHENT           0x0E               //验证密钥
#define PCD_RECEIVE           0x08               //接收数据
#define PCD_TRANSMIT          0x04               //发送数据
#define PCD_TRANSCEIVE        0x0C               //发送并接收数据
#define PCD_RESETPHASE        0x0F               //复位
#define PCD_CALCCRC           0x03               //CRC计算
//
//Mifare_One卡片命令字
#define PICC_REQIDL           0x26               //寻天线区内未进入休眠状态
#define PICC_REQALL           0x52               //寻天线区内全部卡
#define PICC_ANTICOLL1        0x93               //防冲撞
#define PICC_ANTICOLL2        0x95               //防冲撞
#define PICC_ANTICOLL3        0x97               //防冲撞

#define PICC_AUTHENT1A        0x60               //验证A密钥
#define PICC_AUTHENT1B        0x61               //验证B密钥
#define PICC_READ             0x30               //读块
#define PICC_WRITE            0xA0               //写块
#define PICC_DECREMENT        0xC0               //扣款
#define PICC_INCREMENT        0xC1               //充值
#define PICC_RESTORE          0xC2               //调块数据到缓冲区
#define PICC_TRANSFER         0xB0               //保存缓冲区中数据
#define PICC_HALT             0x50               //休眠


/*----------------------------------------------------------------------------*
**                             CI522 返回错误码                               *
**----------------------------------------------------------------------------*/
#define     MI_OK                 0
#define     MI_NOTAGERR           (-1)
#define     MI_ERR                (-2)


#define DEF_FIFO_LENGTH       64                 //FIFO size=64byte
#define MAXRLEN  18

///*----------------------------------------------------------------------------*
//**                             CI522 寄存器设置值                               *
//**----------------------------------------------------------------------------*/
#define TxModeReg_Val 0x00
#define RxModeReg_Val 0x00
#define ModWidthReg_Val 0x26
#define RFCfgReg_Val 0x58
#define TModeReg_Val 0x80
#define TPrescalerReg_Val 0xA9
#define TReloadRegH_Val 0x03
#define TReloadRegL_Val 0xE8
#define TxASKReg_Val 0x40
#define ModeReg_Val 0x3D
#define ACDConfigRegA_Val 0x02
#define N_ACDConfigRegA_Val 0x00
#define ACDConfigRegB_Val 0x04
#define ACDConfigRegC_Val 0x40
#define ACDConfigRegD_Val 0x03
#define ACDConfigRegE_Val 0x07
#define ACDConfigRegF_Val 0x00
#define ACDConfigRegG_Val 0x00
#define ACDConfigRegH_Val 0x14
#define ComIEnReg_Val 0x80
#define DivIEnReg_Val 0x00
#define CommandReg_Val 0x30
#define EX_CommandReg_Val 0x30
#define TestSel2Reg_Val 0x00
#define TestPinEnReg_Val 0x08



typedef enum {
    osPriorityIdle          = -3,  ///< priority: idle (lowest)
    osPriorityLow           = -2,  ///< priority: low
    osPriorityBelowNormal   = -1,  ///< priority: below normal
    osPriorityNormal        =  0,  ///< priority: normal
    osPriorityAboveNormal   = +1,  ///< priority: above normal
    osPriorityHigh          = +2,  ///< priority: high
    osPriorityRealtime      = +3,  ///< priority: realtime (highest)
    osPriorityError         =  0x84,///< system cannot determine priority or thread has illegal priority
    // ... 可能还有其他特定于实现的优先级定义
} osPriority_t;


typedef struct osThreadAttr {
    const char                   *name;           ///< name of the thread
    uint32_t                      attr_bits;      ///< thread attributes
    void                         *cb_mem;         ///< memory for object control block
    uint32_t                      cb_size;        ///< size of the memory for object control block
    void                         *stack_mem;      ///< memory for stack
    uint32_t                      stack_size;     ///< size of the stack memory
    osPriority_t                  priority;       ///< initial thread priority
    uint32_t                      reserved;       ///< reserved for future use
} osThreadAttr_t;

typedef void (*osThreadFunc_t)(void *argument);



/**
 *  @brief ci522 外设初始化
 *
 *  @param [in] spi_id  spi序号
 *  @param [in] rst_gpio  ci522复位引脚
 *  @param [in] irq_pgio  ci522中断引脚
 *
 *  @return
 *
 *  @details
 */
void app_ci522_dev_init();

/**
 *  @brief ci522功能初始化
 *
 *  @param [in]
 *
 *  @return
 *
 *  @details
 */
void app_ci522_func_init();

/**
 *  @brief 获取卡的uuid
 *
 *  @param [in] uuid 卡的uuid(4 byte)
 *
 *  @return CI522错误码
 *
 *  @details
 */
int8_t app_ci522_read_uuid(uint8_t *uuid);

/**
 *  @brief 读取卡ID号
 *
 *  @param [in] type 哪个区域搜索0x01:TX1区域搜索 0x02:TX2区域搜索 0x03:TX1、TX2区域搜索
 *  @param [out] card_id  卡号缓冲区(32 byte卡号)
 *  @param [out] card_id_size  缓冲区大小
 *  @param [in] auth_key  鉴权码(固定为6字节)
 *  @param [in] auth_key_len  鉴权码长度
 *
 *  @return CI522错误码
 *
 *  @details
 */
int8_t app_ci522_cardid_read(uint8_t type, uint8_t *card_id, uint16_t card_id_size, uint8_t *auth_key, uint8_t auth_key_len);

/**
 *  @brief ci522 刷ID卡使用示例
 *
 *  @param [in]
 *
 *  @return
 *
 *  @details
 */
void app_ci522_demo_func(void);


void app_demo_ci522_task(void *p);

#endif
