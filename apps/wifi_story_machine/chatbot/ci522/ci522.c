/*-----------------------------------------------------------------------------
File Name       :   user_ci522.c
Author          :   momo
Created Time    :   2025.4.28
Description     :   CI522芯片应用
-----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
                             Dependencies
-----------------------------------------------------------------------------*/

#include "ci522.h"

#include "app_config.h"
#include "system/includes.h"
#include "device/device.h"
#include "asm/spi.h"
#include "device/gpio.h"

/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/
static const void *s_ci522_spi = NULL;              // CI522 SPI编号

#define CI522_HW_SPI_DEV_NAME                    "spi1"
#define CI522_RST_GPIO IO_PORTC_00
#define CI522_IRQ_GPIO IO_PORTC_01
#define CI522_SPI_CS   IO_PORTC_02


struct card_Info {
    int ai_index;
    char ai_report[40];
};

const struct card_Info CARD_DATA[98] = {
    {3, u8"你好,我是翻译助手"},
    {31, u8"你好,我是奶龙"},
    {47, u8"你好,我是热情美食制作家"},
    {65, u8"你好,我是贾维斯"},
    {69, u8"你好,我是宇航员DODO"},
    {70, u8"你好,我是懒羊羊"},
    {78, u8"你好,我是仙小美"},
    {93, u8"你好,我是清冷白狐仙"},
    {242, u8"你好,我是快乐小汤姆"},
    {244, u8"你好,我是文心一言"},
    {245, u8"你好,我是deepseek"},
    {247, u8"你好,我是拒绝内卷"},
    {249, u8"你好,我是启蒙之星"},
    {252, u8"你好,我是暴躁老王"},
    {253, u8"你好,我是法律顾问"},
    {254, u8"你好,我是中医"},
    {255, u8"你好,我是赛罗奥特"},
    {256, u8"你好,我是迪迦奥特曼"},
    {257, u8"你好,我是天才科学家"},
    {258, u8"你好,我是任达华"},
    {260, u8"你好,我是智启宝贝"},
    {264, u8"你好,我是周边一日游"},
    {265, u8"你好,我是健身助手"},
    {267, u8"你好,我是学龄前认知"},
    {269, u8"你好,我是老子"},
    {273, u8"你好,我是热情美食制作家"},
    {275, u8"你好,我是高考状元学习辅导师"},
    {276, u8"你好,我是刘华强"},
    {277, u8"你好,我是暴躁萌妹修蜜轰"},
    {278, u8"你好,我是千年狐妖妲己"},
    {284, u8"你好,我是体育赛事达人"},
    {286, u8"你好,我是仙小美"},
    {288, u8"你好,我是清白狐仙"},
    {289, u8"你好,我是修仙模拟器"},
    {290, u8"你好,我是大数据财经推荐师"},
    {292, u8"你好,我是小猪佩奇本奇"},
    {293, u8"你好,我是江户川柯南"},
    {294, u8"你好,我是诗仙李白"},
    {295, u8"你好,我是齐天大圣孙悟空"},
    {296, u8"你好,我是林黛玉"},
    {297, u8"你好,我是哈利波特"},
    {298, u8"你好,我是叮当猫"},
    {299, u8"你好,我是哪吒"},
    {300, u8"你好,我是小岳岳"},
    {301, u8"你好,我是拿破仑"},
    {302, u8"你好,我是路飞"},
    {303, u8"你好,我是圣诞老人"},
    {307, u8"你好,我是孤独情绪疗愈"},
    {308, u8"你好,我是江湖百晓生"},
    {309, u8"你好,我是高效工作小助理"},
    {310, u8"你好,我是快乐小猪美食家"},
    {336, u8"你好,我是菲菲老师"},
    {363, u8"你好,我是AI智能律师手"},
    {376, u8"你好,我是英语老师"},
    {670, u8"你好,我是布丁萌可"},
    {829, u8"你好,我是茶之道"},
    {844, u8"你好,我是幼师姐姐"},
    {1148, u8"你好,我是帕丁顿熊"},
    {1314, u8"你好,我是小安同学"},
    {1445, u8"你好,我是喜洋洋"},
    {1515, u8"你好,我是拉布拉多警长"},
    {1521, u8"你好,我是小树懒"},
    {1883, u8"你好,我是拉拉老师"},
    {2012, u8"你好,我是太乙真人"},
    {2015, u8"你好,我是你的台湾女友小语"},
    {2170, u8"你好,我是芙宁娜"},
    {2214, u8"你好,我是大熊猫花"},
    {2254, u8"你好,我是豆包"},
    {2410, u8"你好,我是优秀幼师"},
    {2574, u8"你好,我是AI大厨"},
    {2587, u8"你好,我是慧悦师姐"},
    {2876, u8"你好,我是夸猫哆哆"},
    {3768, u8"你好,我是贝奇"},
    {4750, u8"你好,我是敖丙"},
    {4804, u8"你好,我是卡达鸭"},
    {5053, u8"你好,我是AI历史家教"},
    {5054, u8"你好,我是AI生物家教"},
    {5055, u8"你好,我是AI美术家教"},
    {5056, u8"你好,我是AI化学家教"},
    {5057, u8"你好,我是AI体育家教"},
    {5058, u8"你好,我是AI英语家教"},
    {5059, u8"你好,我是AI地理家教"},
    {5060, u8"你好,我是AI科学家教"},
    {5061, u8"你好,我是AI语文家教"},
    {5062, u8"你好,我是AI数学家教"},
    {5397, u8"你好,我是暖暖姐姐"},
    {5592, u8"你好,我是小猪佩奇"},
    {5593, u8"你好,我是樱桃小丸子"},
    {5594, u8"你好,我是熊二"},
    {7472, u8"你好,我是擎天柱"},
    {7728, u8"你好,我是小猪猪"},
    {8487, u8"你好,我是星空兔"},
    {8530, u8"你好,我是超能爸爸"},
    {8566, u8"你好,我是米妮"},
    {8567, u8"你好,我是维尼小熊"},
    {8570, u8"你好,我是草莓熊"},
    {8874, u8"你好,我是蛋小蓝呀"},
};

#define CARD_NUM sizeof(CARD_DATA)/sizeof(CARD_DATA[0])



uint8_t nfc_uuid[4] = {0};
uint8_t last_uuid[4] = {0};
void osDelay(int time_tick)
{
    int time_tick1 = time_tick;
    os_time_dly(time_tick1);
}

int ci522_write(void* device, void* buf, u32 len)
{

    int ret;
    if (device == NULL) {
        printf("ERROR\n");
        return -1;
    }
    gpio_direction_output(CI522_SPI_CS, 0);

    ret = dev_write(device, buf, len);
    gpio_direction_output(CI522_SPI_CS, 1);

    return ret;
}
int ci522_read(void* device, void* buf, u32 len)
{

    int ret;

    if (device == NULL) {
        printf("ERROR\n");
        return -1;

    }
    gpio_direction_output(CI522_SPI_CS, 0);
    ret = dev_read(device, buf, len); //ret=0则已经读取到数据
    gpio_direction_output(CI522_SPI_CS, 1);

    return ret;
}

/*----------------------------------------------------------------------------*
**                             Local Func                                     *
**----------------------------------------------------------------------------*/
bool app_ci522_write(uint8_t reg_addr, uint8_t value);
uint8_t app_ci522_read(uint8_t reg_addr);

/**
 *  @brief 16进制转变为16进制字符串
 *
 *  @param [out] hexstr 16进制字符串数据缓冲区
 *  @param [in] hexstr_size 16进制字符串数据缓冲区大小
 *  @param [in] hex 16进制数据
 *  @param [in] hexlen 16进制数据长度
 *  @param [in] type 0:小写 1:大写
 *
 *  @return 转换后的16进制字符串大小
 *
 *  @details
 */
unsigned short app_get_hex2hexstr(unsigned char *hexstr, unsigned short hexstr_size, unsigned char *hex, unsigned short hexlen, unsigned char type)
{
    if (hex == NULL || hexstr == NULL || hexlen == 0 || hexstr_size < (2 * hexlen)) {
        return 0;
    }
    unsigned char temp[5] = {0};
    unsigned short i = 0;
    for (i = 0; i < hexlen; i++) {
        memset(temp, 0, sizeof(temp));
        if (type == 0) {
            sprintf((char *)temp, "%02x", *(hex + i));
        } else {
            sprintf((char *)temp, "%02X", *(hex + i));
        }
        strcat((char *)hexstr, (char *)temp);
        memcpy((hexstr + i * 2), temp, 2);
    }
    return (2 * hexlen);
}


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
void app_ci522_dev_init()
{
    char *buf = NULL;
    int ret;

    //1.CI522复位
    gpio_direction_output(CI522_RST_GPIO, 0);  //GPIO输出
    osDelay(100 / portTICK_PERIOD_MS);

    gpio_direction_output(CI522_RST_GPIO, 1);  //GPIO输出
    osDelay(100 / portTICK_PERIOD_MS);

    //2.打开spi设备
    s_ci522_spi = dev_open(CI522_HW_SPI_DEV_NAME, NULL);
    if (!s_ci522_spi) {
        printf("spi open err \n");
    }
//    dev_ioctl(CI522_HW_SPI_DEV_NAME, IOCTL_SPI_SET_IRQ_CPU_ID, (u32)1);   //可以指定中断到核1执行
//    dev_ioctl(CI522_HW_SPI_DEV_NAME, IOCTL_SPI_SET_USE_SEM, 0);   //等待数据时，用信号量等待，不用则为抢占查询硬件中断标记，建议应信号量等待
//    dev_ioctl(CI522_HW_SPI_DEV_NAME, IOCTL_SPI_SEND_BYTE, 0); //发送一个0字节，初始化SPI-DO为低电平


}
/**
 *  @brief 写ci522寄存器
 *
 *  @param [in] reg_addr 寄存器地址
 *  @param [in] value   需要写入的值
 *
 *  @return
 *
 *  @details
 */
bool app_ci522_write(uint8_t reg_addr, uint8_t value)
{
    uint8_t cmd[4] = {0};
    cmd[0] = (reg_addr & 0x3f) << 1;
    cmd[1] = value;
    if (s_ci522_spi == NULL) {
        s_ci522_spi = dev_open("spi1", NULL);
    }

    if (ci522_write(s_ci522_spi, cmd, 2) < 0) {
        return false;
    }
    return true;
}


/**
 *  @brief 读ci522寄存器
 *
 *  @param [in] reg_addr 寄存器地址
 *
 *  @return 寄存器地址的数值
 *
 *  @details
 */
uint8_t app_ci522_read(uint8_t reg_addr)
{
    uint8_t value[5] = {0}, cmd[5] = {0};
    cmd[0] = ((reg_addr & 0x3f) << 1) | 0x80;                 //code the first byte

    if (ci522_write(s_ci522_spi, cmd, 1) < 0) {
        puts("dev_write fail");
        return 0;
    }
    if (ci522_read(s_ci522_spi, value, 1) < 0) {
        puts("dev_read fail");
        return 0;
    }
    return value[0];
}

/**
 *  @brief ci522设置某个寄存器的bit位
 *
 *  @param [in] reg_addr 寄存器地址
 *  @param [in] mask   设置的bit位
 *
 *  @return
 *
 *  @details
 */
void app_ci522_set_bitmask(uint8_t reg_addr, uint8_t mask)
{
    uint8_t tmp = 0x00;
    tmp = app_ci522_read(reg_addr);
    app_ci522_write(reg_addr, tmp | mask); // set bit mask
}


/**
 *  @brief ci522清除某个寄存器的bit位
 *
 *  @param [in] reg_addr 寄存器地址
 *  @param [in] mask   清除的bit位
 *
 *  @return
 *
 *  @details
 */
void app_ci522_clear_bitmask(uint8_t reg_addr, uint8_t mask)
{
    uint8_t tmp = 0x00;
    tmp = app_ci522_read(reg_addr);

    app_ci522_write(reg_addr, tmp & ~mask);  // clear bit mask
}


/**
 *  @brief ci522复位
 *
 *  @param [in] mode 0:硬件引脚复位 1:操作寄存器复位
 *
 *  @return
 *
 *  @details
 */
void app_ci522_reset(uint8_t mode)
{
    //硬件复位
    if (mode == 0) {
        gpio_direction_output(CI522_RST_GPIO, 0);  //GPIO输出

        osDelay(100 / portTICK_PERIOD_MS);

        gpio_direction_output(CI522_RST_GPIO, 1);  //GPIO输出

        osDelay(100 / portTICK_PERIOD_MS);
    } else {
        //寄存器复位
        app_ci522_write(CommandReg, 0x0f);
        while (app_ci522_read(CommandReg) & 0x10);
        osDelay(100 / portTICK_PERIOD_MS);
    }
}

/**
 *  @brief 开启ci522天线
 *
 *  @param [in] type 1:打开TX1 2:打开TX2 3:打开TX1,TX2
 *
 *  @return
 *
 *  @details
 */
void app_ci522_antenna_open(uint8_t type)
{
    if (type == 0 || type > 3) {
        return;
    }
    uint8_t reg_value = 0;
    reg_value = app_ci522_read(TxControlReg);
    if (!(reg_value & type)) {
        app_ci522_set_bitmask(TxControlReg, type);
    }
}

/**
 *  @brief 关闭ci522天线
 *
 *  @param [in] type: 1:关闭TX1 2:关闭TX2 3:关闭TX1,TX2
 *
 *  @return
 *
 *  @details
 */
void app_ci522_antenna_close(uint8_t type)
{
    if (type == 0 || type > 3) {
        return;
    }
    app_ci522_clear_bitmask(TxControlReg, type);
}

/**
 *  @brief ci522功能初始化
 *
 *  @param [in]
 *
 *  @return
 *
 *  @details
 */
void app_ci522_func_init()
{
    //app_ci522_reset(1);
    app_ci522_clear_bitmask(Status2Reg, 0x08);
    // Reset baud rates
    app_ci522_write(TxModeReg, TxModeReg_Val);
    app_ci522_write(RxModeReg, RxModeReg_Val);
    // Reset ModWidthReg
    app_ci522_write(ModWidthReg, ModWidthReg_Val);

    // RxGain:110,43dB by default
    app_ci522_write(RFCfgReg, RFCfgReg_Val);
    // When communicating with a PICC we need a timeout if something goes wrong.
    // f_timer = 13.56 MHz / (2*TPreScaler+1) where TPreScaler = [TPrescaler_Hi:TPrescaler_Lo].
    // TPrescaler_Hi are the four low bits in TModeReg. TPrescaler_Lo is TPrescalerReg.
    app_ci522_write(TModeReg, TModeReg_Val);            // TAuto=1; timer starts automatically at the end of the transmission in all communication modes at all speeds

    app_ci522_write(TPrescalerReg, TPrescalerReg_Val);  // TPreScaler = TModeReg[3..0]:TPrescalerReg
    app_ci522_write(TReloadRegH, TReloadRegH_Val);      // Reload timer
    app_ci522_write(TReloadRegL, TReloadRegL_Val);
    app_ci522_write(TxASKReg, TxASKReg_Val);            // Default 0x00. Force a 100 % ASK modulation independent of the ModGsPReg register setting
    app_ci522_write(ModeReg, ModeReg_Val);              // Default 0x3F. Set the preset value for the CRC coprocessor for the CalcCRC command to 0x6363 (ISO 14443-3 part 6.2.4)
    // Turn on the analog part of receiver
    app_ci522_write(CommandReg, PCD_IDLE);
    //app_ci522_antenna_open(1);    //开启天线
}

/**
 *  @brief ci522计算CRC16校验值
 *
 *  @param [in] data 数据
 *  @param [in] datalen 数据长度
 *
 *  @return 校验值
 *
 *  @details
 */
uint16_t app_ci522_cal_crc16_value(uint8_t *data, uint16_t datalen)
{
    app_ci522_clear_bitmask(DivIrqReg, 0x04);
    app_ci522_write(CommandReg, PCD_IDLE);
    app_ci522_set_bitmask(FIFOLevelReg, 0x80);

    uint8_t i = 0, n = 0;
    for (i = 0; i < datalen; i++) {
        app_ci522_write(FIFODataReg, *(data + i));
    }
    app_ci522_write(CommandReg, PCD_CALCCRC);
    i = 20;
    do {
        n = app_ci522_read(DivIrqReg);
        i--;
        osDelay(1);
    } while ((i != 0) && !(n & 0x04));

    uint16_t crc_value = app_ci522_read(CRCResultRegH) * 256 + app_ci522_read(CRCResultRegL);
    return crc_value;
}

/**
 *  @brief ci522与card通讯
 *
 *  @param [out] out_buf 接收到卡片返回的数据
 *  @param [out] out_buflen 接收到卡片返回的数据长度
 *  @param [in] command CI522命令字
 *  @param [in] data 发送的数据
 *  @param [in] datalen 发送的数据长度
 *
 *  @return CI522错误码
 *
 *  @details
 */
int8_t app_ci522_comm_with_card(uint8_t *out_buf, uint16_t *out_buflen, uint8_t command, uint8_t *data, uint16_t datalen)
{
    uint8_t irqEn = 0x00, waitFor = 0x00, temp = 0, reg_value = 0, lastBits = 0;
    uint16_t timeout = 0;
    int8_t status = MI_ERR;
    switch (command) {
    /* 验证密钥 */
    case PCD_AUTHENT: {
            irqEn   = 0x12;
            waitFor = 0x10;
            break;
        }
    /* 发送并接收数据 */
    case PCD_TRANSCEIVE: {
            irqEn   = 0x77;
            waitFor = 0x30;
            break;
        }
    default:
        break;
    }
    app_ci522_write(ComIEnReg, irqEn | 0x80);
    app_ci522_clear_bitmask(ComIrqReg, 0x80);
    app_ci522_write(CommandReg, PCD_IDLE);
    app_ci522_set_bitmask(FIFOLevelReg, 0x80);

    for (uint8_t i = 0; i < datalen; i++) {
        app_ci522_write(FIFODataReg, data[i]);
    }

    app_ci522_write(CommandReg, command);

    if (command == PCD_TRANSCEIVE) {
        app_ci522_set_bitmask(BitFramingReg, 0x80);
    }

    do {
        timeout ++;
        if (timeout >= 1000) {
            break;
        }
        temp = app_ci522_read(ComIrqReg);
    } while (!(temp & 0x01) && !(temp & waitFor));

    app_ci522_clear_bitmask(BitFramingReg, 0x80);

    if (timeout < 1000) {
        uint8_t error_value =  app_ci522_read(ErrorReg);

        if (!(app_ci522_read(ErrorReg) & 0x1B)) {
            status = MI_OK;
            if (temp & irqEn & 0x01) {
                //printf("irqEn-------------%d\r\n", __LINE__);
                status = MI_NOTAGERR;
            }
            if (command == PCD_TRANSCEIVE) {
                temp = app_ci522_read(FIFOLevelReg);
                lastBits = app_ci522_read(ControlReg) & 0x07;
                if (lastBits) {
                    *out_buflen = (temp - 1) * 8 + lastBits;

                } else {
                    *out_buflen = temp * 8;
                }
                if (temp == 0) {
                    temp = 1;
                }
                if (temp > MAXRLEN) {
                    temp = MAXRLEN;
                }
                for (uint8_t i = 0; i < temp; i ++) {
                    *(out_buf + i) = app_ci522_read(FIFODataReg);
                }
            }
        } else {
            //printf("MI_ERR---(app_ci522_read(ErrorReg) & 0x1B)-------------%d\r\n", __LINE__);
            status = MI_ERR;
        }
    }
    app_ci522_set_bitmask(ControlReg, 0x80);
    app_ci522_write(CommandReg, PCD_IDLE);
    return status;
}

/**
 *  @brief ci522计算CRC16校验值
 *
 *  @param [in] type 哪个区域搜索   1:TX1区域搜索 2:TX2区域搜索 3:TX1、TX2区域搜索
 *  @param [in] req_code 寻卡方式   0x52:感应区内所有符合14443A标准的卡; 0x26:寻未进入休眠状态的卡;
 *  @param [out] card_type 卡片类型代码   0x4400:Mifare_UltraLight 0x0400:Mifare_One(S50) 0x0200:Mifare_One(S70) 0x0800:Mifare_Pro(X) 0x4403:Mifare_DESFire
 *
 *  @return CI522错误码
 *
 *  @details
 */
int8_t app_ci522_search_card(uint8_t type, uint8_t req_code, uint16_t *card_type)
{
    int8_t status = MI_ERR;
    uint8_t data[MAXRLEN] = {0};
    uint16_t datalen = 0;

    app_ci522_clear_bitmask(Status2Reg, 0x08);
    app_ci522_write(BitFramingReg, 0x07);


//    app_ci522_antenna_close(0x03);
//    osDelay(100);
//    app_ci522_antenna_open(type);

    app_ci522_set_bitmask(TxControlReg, 0x03);

    data[0] = req_code;

    status = app_ci522_comm_with_card(data, &datalen, PCD_TRANSCEIVE, &req_code, 1);
    if (status == MI_OK && datalen == 0x10) {
        *card_type = data[0] * 256 + data[1];   //高位在前
    } else {
        status = MI_ERR;
    }
    return status;
}

/**
 *  @brief 防冲撞
 *
 *  @param [out] out_buf 卡片序列号
 *  @param [in] anticollision_level 防冲撞等级(0x93 防冲撞等级1 0x95 防冲撞等级2  0x97 防冲撞等级3
 *
 *  @return CI522错误码
 *
 *  @details
 */
int8_t app_ci522_anticollision(uint8_t *out_buf, uint8_t anticollision_level)
{
    uint8_t data[MAXRLEN] = {0}, snr_check = 0;
    uint16_t datalen = 0;
    int8_t  status = MI_ERR;

    app_ci522_clear_bitmask(Status2Reg, 0x08);
    app_ci522_write(BitFramingReg, 0x00);
    app_ci522_set_bitmask(CollReg, 0x80);

    uint8_t cmd[4] = {0};
    cmd[0] = anticollision_level;
    cmd[1] = 0x20;

    status = app_ci522_comm_with_card(data, &datalen, PCD_TRANSCEIVE, cmd, 2);

    if (status == MI_OK) {
        uint8_t i = 0;
        for (i = 0; i < 4; i++) {
            *(out_buf + i) = data[i];
            snr_check ^= data[i];
        }
        if (snr_check != data[i]) {
            status = MI_ERR;
        }
    }
    app_ci522_set_bitmask(CollReg, 0x80);
    return status;
}

/**
 *  @brief 防冲撞
 *
 *  @param [out] uuid uuid码
 *  @param [out] sak sak标志
 *  @param [in] anticollision_level 防冲撞水平(0x93 防冲撞水平1 0x95 防冲撞水平2  0x97 防冲撞水平3
 *
 *  @return CI522错误码
 *
 *  @details
 */
int8_t app_ci522_select_card(uint8_t *uuid, uint8_t *sak, uint8_t anticollision_level)
{
    int8_t status = MI_ERR;
    uint8_t i = 0, data[MAXRLEN] = {0};
    uint16_t datalen = 0;

    data[0] = anticollision_level;
    data[1] = 0x70;
    data[6] = 0;
    for (i = 0; i < 4; i++) {
        data[i + 2] = *(uuid + i);
        data[6]  ^= *(uuid + i);
    }
    uint16_t crc_value = app_ci522_cal_crc16_value(data, 7);
    data[7] =  crc_value % 256;
    data[8] =  crc_value / 256;

    app_ci522_clear_bitmask(Status2Reg, 0x08);

    status = app_ci522_comm_with_card(data, &datalen, PCD_TRANSCEIVE, data, 9);

    if ((status == MI_OK) && (datalen == 0x18)) {
        *sak = data[0];
        status = MI_OK;
    } else {
        status = MI_ERR;
    }
    return status;
}


/**
 *  @brief 进入休眠状态
 *
 *  @param [in]
 *
 *  @return CI522错误码
 *
 *  @details
 */
int8_t app_ci522_halt()
{
    int8_t status = MI_ERR;
    uint8_t data[MAXRLEN] = {0};
    uint16_t datalen_bit = 0;

    data[0] = PICC_HALT;
    data[1] = 0;
    uint16_t crc_value =  app_ci522_cal_crc16_value(data, 2);
    data[2] =  crc_value % 256;
    data[3] =  crc_value / 256;

    status = app_ci522_comm_with_card(data, &datalen_bit, PCD_TRANSCEIVE, data, 4);
    return status;
}

/**
 *  @brief 验证卡片密码
 *
 *  @param [in] auth_mode 密码验证模式 0x60 = 验证A密钥 0x61 = 验证B密钥
 *  @param [in] addr 块地址(块号，M1-S50容量为8K 位EEPROM分为16个扇区，每个扇区为4块，每块16个字节,以块为存取单位，即地址范围0~63)
 *  @param [in] pKey 密码
 *  @param [in] pSnr 卡片序列号，4字节
 *
 *  @return CI522错误码
 *
 *  @details
 */
int8_t app_ci522_authstate(uint8_t auth_mode, uint8_t addr, uint8_t *pKey, uint8_t *pSnr)
{
    int8_t status = MI_ERR;
    uint8_t data[MAXRLEN] = {0};
    uint16_t datalen_bit = 0;

    data[0] = auth_mode;
    data[1] = addr;
    memcpy(&data[2], pKey, 6);
    memcpy(&data[8], pSnr, 6);

    status = app_ci522_comm_with_card(data, &datalen_bit, PCD_AUTHENT, data, 12);
//    if ((status != MI_OK) || (!(app_ci522_read(Status2Reg) & 0x08)))
//    {
//        printf("---> line = %d \n",__LINE__);
//      status = MI_ERR;
//  }
    if ((status != MI_OK)) {
        printf("---> line = %d \n", __LINE__);
        status = MI_ERR;
    }
    return status;
}

/**
 *  @brief 读取M1卡一块数据
 *
 *  @param [in] addr 块地址(块号，M1-S50容量为8K 位EEPROM分为16个扇区，每个扇区为4块，每块16个字节,以块为存取单位，即地址范围0~63)
 *  @param [in] pKey 密码
 *  @param [in] pSnr 卡片序列号，4字节
 *
 *  @return CI522错误码
 *
 *  @details
 */
int8_t app_ci522_block_read(uint8_t addr, uint8_t *pData)
{
    int8_t status = MI_ERR;
    uint8_t data[MAXRLEN] = {0};
    uint16_t datalen_bit = 0;

    data[0] = PICC_READ;
    data[1] = addr;
    uint16_t crc_value =  app_ci522_cal_crc16_value(data, 2);
    data[2] =  crc_value % 256;
    data[3] =  crc_value / 256;

    status = app_ci522_comm_with_card(data, &datalen_bit, PCD_TRANSCEIVE, data, 4);
    printf("---> status = %d \n", status);
    printf("---> line = %d \n", __LINE__);
    printf("datalen_bit %d-------\r\n", datalen_bit);
    if (status == MI_OK) {
        memcpy(pData, data, 16);
    }
//    if ((status == MI_OK))
//      {
//      memcpy(pData, data, 16);
//  }
    else {
        status = MI_ERR;
    }
    return status;
}

/**
 *  @brief 写数据到M1卡一块
 *
 *  @param [in] addr 块地址(块号，M1-S50容量为8K 位EEPROM分为16个扇区，每个扇区为4块，每块16个字节,以块为存取单位，即地址范围0~63)
 *  @param [in] pData 写入的数据，16字节
 *
 *  @return CI522错误码
 *
 *  @details
 */
int8_t app_ci522_block_write(uint8_t addr, uint8_t *pData)
{
    int8_t status = MI_ERR;
    uint8_t data[MAXRLEN] = {0};
    uint16_t datalen_bit = 0;

    data[0] = PICC_WRITE ;
    data[1] = addr;
    uint16_t crc_value =  app_ci522_cal_crc16_value(data, 2);
    data[2] =  crc_value % 256;
    data[3] =  crc_value / 256;
    status = app_ci522_comm_with_card(data, &datalen_bit, PCD_TRANSCEIVE, data, 4);

    if ((status != MI_OK) || (datalen_bit != 4) || ((data[0] & 0x0F) != 0x0A)) {
        status = MI_ERR;
    }

    if (status == MI_OK) {
        memcpy(data, pData, 16);
        crc_value = app_ci522_cal_crc16_value(data, 16);
        data[16] =  crc_value % 256;
        data[17] =  crc_value / 256;

        status = app_ci522_comm_with_card(data, &datalen_bit, PCD_TRANSCEIVE, data, 18);
        if ((status != MI_OK) || (datalen_bit != 4) || ((data[0] & 0x0F) != 0x0A)) {
            status = MI_ERR;
            printf("---> line = %d \n", __LINE__);
        }
    }
    return status;
}

/**
 *  @brief 获取卡的uuid
 *
 *  @param [in] uuid 卡的uuid(4 byte)
 *
 *  @return CI522错误码
 *
 *  @details
 */
int8_t app_ci522_read_uuid(uint8_t *uuid)
{
    uint16_t l_card_type = 0;
    //寻卡
    app_ci522_write(RFCfgReg, 0x68);             //复位接收增益
//    app_ci522_write(PICC_REQALL, 0x68);

    if (app_ci522_search_card(0x03, PICC_REQIDL, &l_card_type) != MI_OK) { //寻天线区内未进入休眠状态的卡，返回卡片类型 2字节
        app_ci522_write(RFCfgReg, 0x48);
        if (app_ci522_search_card(0x03, PICC_REQIDL, &l_card_type) != MI_OK) {
            app_ci522_write(RFCfgReg, 0x58);
            if (app_ci522_search_card(0x03, PICC_REQIDL, &l_card_type) != MI_OK) {
                return MI_NOTAGERR;
            }
        }
    }
    printf("search card success,card type is 0x%04x", l_card_type);

    //冲突检测 level1
    uint8_t sak = 0;
    if (app_ci522_anticollision(uuid, PICC_ANTICOLL1) != MI_OK) {
        puts("Anticoll1:fail");
        return MI_NOTAGERR;
    }

    if (app_ci522_select_card(uuid, &sak, PICC_ANTICOLL1) != MI_OK) {
        puts("Select1:fail");
        return 1;
    }
    printf("Select1:ok  SAK1:%02x", sak);

    if (sak & 0x04) { //判断UUID是否完整
        //Anticoll 冲突检测 level2
        if (app_ci522_anticollision(uuid, PICC_ANTICOLL2) != MI_OK) {
            puts("Anticoll2:fail");
            return MI_ERR;
        }
        if (app_ci522_select_card(uuid, &sak, PICC_ANTICOLL2) != MI_OK) {
            puts("Select2:fail");
            return MI_ERR;
        }
        printf("Select2:ok  SAK2:%02x", sak);

        if (sak & 0x04) { //判断UUID是否完整
            //Anticoll 冲突检测 level3
            if (app_ci522_anticollision(uuid, PICC_ANTICOLL3) != MI_OK) {
                puts("Anticoll3:fail");
                return MI_ERR;
            }
            if (app_ci522_select_card(uuid, &sak, PICC_ANTICOLL3) != MI_OK) {
                puts("Select3:fail");
                return MI_ERR;
            } else {
                printf("Select3:ok  SAK3:%02x", sak);
                if (sak & 0x04) {
                    return MI_ERR;
                }
            }
        }
    }
    hal_info_log("get uuid success,uuid is:%02x %02x %02x %02x", uuid[0], uuid[1], uuid[2], uuid[3]);
    return MI_OK;
}


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
int8_t app_ci522_cardid_read(uint8_t type, uint8_t *card_id, uint16_t card_id_size, uint8_t *auth_key, uint8_t auth_key_len)
{
    if (card_id_size < 32 || auth_key_len < 6) {
        //hal_info_log("ci522 buf is small");
        return MI_ERR;
    }
    uint8_t uuid[4] = {0}, sak = 0;
    uint16_t card_type = 0;
    //寻卡
    if (app_ci522_search_card(type, PICC_REQIDL, &card_type) != MI_OK) { //寻天线区内未进入休眠状态的卡，返回卡片类型 2字节
        return MI_NOTAGERR;
        printf("---> line = %d \n", __LINE__);
    }
    printf("search card success,card type is 0x%04x", card_type);

    //Anticoll 冲突检测
    if (app_ci522_anticollision(uuid, PICC_ANTICOLL1) != MI_OK) {
        puts("anticoll fail");
        return MI_ERR;
    }
    printf("anticoll success,uuid:%02x %02x %02x %02x", uuid[0], uuid[1], uuid[2], uuid[3]);

    // Select 选卡
    if (app_ci522_select_card(uuid, &sak, PICC_ANTICOLL1) != MI_OK) {
        puts("Select card fail");
        return MI_ERR;
    }
    printf("select card ok,SAK:0x%02x\r\n", sak);
//
    uint8_t keybuf[6] = {0};
    memcpy(keybuf, auth_key, 6);

    memcpy(nfc_uuid, uuid, 4);
    uint8_t snr = 1;//扇区号1
    if (app_ci522_authstate(PICC_AUTHENT1A, (snr * 4 + 3), keybuf, uuid) != MI_OK) { //校验1扇区密码，密码位于每一扇区第3块
        puts("authenticate card key fail");
        printf("---> line = %d \n", __LINE__);
        return MI_ERR;
    }
    puts("authenticate card key success");

    return MI_OK;
}


uint8_t *app_ci522_card_data_read(uint8_t *card_data, uint16_t card_data_size, uint8_t snr, uint8_t block)
{
    //读BLOCK原始数据
    uint8_t card_read_buf[16] = {0};
    // uint8_t card_write_buf[16] = {0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,
    //                             0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22};
    if (app_ci522_block_read((snr * 4 + block), card_read_buf) == MI_OK) { //读卡，读取1扇区0块数据到buf[0]-buf[16]
        //app_get_hex2hexstr(card_data,card_data_size,card_read_buf,16,1);
        memcpy(card_data, card_read_buf, card_data_size);
        printf("read card data success,data:%s\n", card_data);
        return card_data;
    } else {
        puts("read card data fail");
        printf("---> status = %d \n", app_ci522_block_read((snr * 4 + 2), card_read_buf));
        printf("---> line = %d \n", __LINE__);
        return NULL;
    }
}

int app_ci522_card_data_write(int card_AI_index, uint8_t snr)
{

    //BLOCK写数据
    //uint8_t card_write_buf[32] = {0x00};
    uint8_t card_write_buf[16] = {0xA5, 0x5A, 0xAA, 0xAA, 0xAA, 0xAA, 0x5A, 0xA5};
    memcpy(&card_write_buf[2], (uint8_t*)&card_AI_index, 4);
//     uint8_t card_write_buf[32] = {0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
//                                    0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33};
    if (app_ci522_block_write((snr * 4 + 0), card_write_buf) != MI_OK) { //写卡，读取1扇区0块数据到buf[0]-buf[16]
        puts("write card data fail");
        return -1;
    }
    //app_get_hex2hexstr(card_data,card_data_size,card_write_buf,16,1);
    printf("write card data1 success,data:%s\n", card_write_buf);
    return 0;
}

//void app_ci522_card_find(uint8_t *card_data1,uint8_t *card_data2){
uint8_t app_ci522_card_find(uint8_t *card_data)
{

    int card_AI_index = 0;
    //检查卡片信息是否正确
    if (card_data[0] == 0xA5 && card_data[1] == 0x5A && card_data[6] == 0x5A && card_data[7] == 0xA5) {
        memcpy(&card_AI_index, &card_data[2], sizeof(card_AI_index));
        for (uint8_t i = 0; i < CARD_NUM; i++) {
            if (CARD_DATA[i].ai_index == card_AI_index) {
                http_ai_intell_set_id(CARD_DATA[i].ai_index, NULL);
                http_tts_request(CARD_DATA[i].ai_report, strlen(CARD_DATA[i].ai_report));
            }
        }
        return 0;
    } else {
        return -1;
    }
}
void clear_last_uuid(void)
{
    memset(last_uuid, 0, sizeof(last_uuid));
}


/**
 *  @brief ci522读卡线程
 *
 *  @param [in]
 *
 *  @return
 *
 *  @details
 */
u16 clear_uuid_pid = 0;
void app_demo_ci522_task(void *p)
{
    uint8_t ic_card[64] = {0};      //IC卡号
    uint8_t ic_card_key[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};   //读取IC卡的密钥，6字节

    uint8_t card_data[16] = {0};
    uint8_t *card_data11 = NULL;
    uint8_t last_card_data[32];

    int8_t status = MI_ERR;
    uint16_t l_card_type = 0;

    app_ci522_dev_init();   //初始化外设引脚
    app_ci522_func_init();      //初始化ci522功能
    osDelay(100 / portTICK_PERIOD_MS);

    puts("app_ci522_demo_func ci522 init success!(2/2)");


    while (1) {
        if (app_ci522_cardid_read(3, ic_card, sizeof(ic_card), ic_card_key, 6) == MI_OK) { //1区域检测到IC卡

            memset(card_data, 0, sizeof(card_data));

            if (memcmp(nfc_uuid, last_uuid, 4) != 0) {
//                if(!app_ci522_card_data_write(2015,1)){
//                    printf("------------------printf------------ok---1173\r\n");
//                    memcpy(last_uuid, nfc_uuid, 4);
//                }

                //app_ci522_card_find(nfc_uuid);

                app_ci522_card_data_read(card_data, sizeof(card_data), 1, 0);
                if (!app_ci522_card_find(card_data)) {
                    memcpy(last_uuid, nfc_uuid, 4);
                } else {
                    printf("card_data_error\n");
                    continue;//跳过本次循环，继续下一次检测
                }
            } else if (memcmp(nfc_uuid, last_uuid, 4) == 0) {
                if (clear_uuid_pid != 0) {
                    sys_timer_del(clear_uuid_pid);
                    clear_uuid_pid = 0;
                }
                delay(2);
                if (clear_uuid_pid == 0) {
                    clear_uuid_pid = sys_timer_add_to_task("sys_timer", NULL, clear_last_uuid, 3000);
                }
            }

        }
        os_time_dly(5);

    }

}



/**
 *  @brief ci522 刷ID卡使用示例
 *
 *  @param [in]
 *
 *  @return
 *
 *  @details
 */
void app_ci522_demo_func(void)
{
    osThreadAttr_t app_task_attr = {0};
    app_task_attr.name = "ci522_task";
    app_task_attr.stack_size = 1024 * 4;
    app_task_attr.priority = osPriorityNormal;

    os_task_create(app_demo_ci522_task, NULL, 12, 1000, 0, "app_demo_ci522_task");
    printf("---> line = %d \n", __LINE__);

    puts("app_ci522_demo_func create ci522 task success!(1/2)");
}

//late_initcall(app_ci522_demo_func);

