#include "app_config.h"

#ifdef CONFIG_BOARD_7916A
#include "app_power_manage.h"
#include "system/includes.h"
#include "device/includes.h"
#include "device/iic.h"
#include "server/audio_dev.h"
#include "asm/includes.h"
#include "gSensor_manage.h"

#ifdef CONFIG_EXT_WIFI_ENABLE
#include "net/wireless_ext/wifi_dev.h"
#include "device/sdio/sdio_host_init.h"
#endif
#if TCFG_USB_SLAVE_ENABLE || TCFG_USB_HOST_ENABLE
#include "otg.h"
#include "usb_host.h"
#include "usb_storage.h"
#endif
#ifdef CONFIG_UI_ENABLE
#include "ui/include/lcd_drive.h"
#include "ui/include/ui_api.h"
#endif
#ifdef CONFIG_LTE_PHY_ENABLE
#include "lte_module/lte_module.h"
#endif

// *INDENT-OFF*
#ifndef TCFG_DEBUG_PORT
#define TCFG_DEBUG_PORT -1
#endif
#ifndef UART0_BUAD
#define UART0_BUAD  19200
#endif
#ifndef UART2_BUAD
#define UART2_BUAD  9600
#endif
#ifndef UART2_PORT
#define UART2_PORT 0
#endif

UART0_PLATFORM_DATA_BEGIN(uart0_data)
    .baudrate = UART0_BUAD,
#ifdef TIMER_UART_TX_PORT
    .port = PORT_REMAP,
    .tx_pin = TIMER_UART_TX_PORT,//IO_PORT_USB_DPA,
    .rx_pin = -1,//IO_PORT_USB_DMA,
    .output_channel = OUTPUT_CHANNEL0,
#elif defined AI_UART_CMD_TX_PORT
    .port = PORT_REMAP,
    .tx_pin = AI_UART_CMD_TX_PORT,//IO_PORT_USB_DPA,
    .rx_pin = -1,//IO_PORT_USB_DMA,
    .output_channel = OUTPUT_CHANNEL0,
#else
    .port = PORTA_5_6,
    .tx_pin = IO_PORTA_05,//IO_PORT_USB_DPA,
    .rx_pin = IO_PORTA_06,//IO_PORT_USB_DMA,
#endif
    .max_continue_recv_cnt = 1024,
    .idle_sys_clk_cnt = 500000,
    .clk_src = PLL_48M,
    .flags = UART_DEBUG,
UART0_PLATFORM_DATA_END();

#ifdef RF_FCC_TEST_ENABLE
UART1_PLATFORM_DATA_BEGIN(uart1_data)
    .baudrate = 115200,
    .port = PORTUSB_A,
#if CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_PLNK0
    .output_channel = OUTPUT_CHANNEL3,
#else
    .output_channel = OUTPUT_CHANNEL0,
#endif
    .tx_pin = IO_PORT_USB_DPA,
    .rx_pin = IO_PORT_USB_DMA,
    .max_continue_recv_cnt = 1024,
    .idle_sys_clk_cnt = 500000,
    .clk_src = PLL_48M,
    .flags = UART_DEBUG,
UART1_PLATFORM_DATA_END();

UART2_PLATFORM_DATA_BEGIN(uart2_data)
    .baudrate = 1000000,
    .port = PORT_REMAP,
#if CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_PLNK0
    .output_channel = OUTPUT_CHANNEL3,
#else
    .output_channel = OUTPUT_CHANNEL0,
#endif
    .tx_pin = TCFG_DEBUG_PORT,
    .rx_pin = -1,
    .max_continue_recv_cnt = 1024,
    .idle_sys_clk_cnt = 500000,
    .clk_src = PLL_48M,
    .flags = UART_DEBUG,
UART2_PLATFORM_DATA_END();

#else
UART1_PLATFORM_DATA_BEGIN(uart1_data)
    .baudrate = 1000000,
    .port = PORT_REMAP,//PORTUSB_A
#if CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_PLNK0
    .output_channel = OUTPUT_CHANNEL3,
#else
    .output_channel = OUTPUT_CHANNEL0,
#endif
    .tx_pin = TCFG_DEBUG_PORT,//IO_PORT_USB_DPA
    //.tx_pin = IO_PORT_USB_DPA,//IO_PORT_USB_DPA
    .rx_pin = -1,//IO_PORT_USB_DMA
    .max_continue_recv_cnt = 1024,
    .idle_sys_clk_cnt = 500000,
    .clk_src = PLL_48M,
    .flags = UART_DEBUG,
UART1_PLATFORM_DATA_END();

UART2_PLATFORM_DATA_BEGIN(uart2_data)
    .baudrate = UART2_BUAD,
//    .port = PORT_REMAP,//PORTC_9_10
//    .tx_pin = IO_PORTC_10,//IO_PORT_USB_DPA,
//    .rx_pin = IO_PORTC_09,//IO_PORTC_09,//IO_PORT_USB_DMA,

#if UART2_PORT == PORTH_2_3
    .port = PORTH_2_3,
    .tx_pin = IO_PORTH_02,//IO_PORT_USB_DPA,
    .rx_pin = IO_PORTH_03,//IO_PORT_USB_DMA,
#else
    .port = PORTC_9_10,
    .tx_pin = IO_PORTC_09,//IO_PORT_USB_DPA,
    .rx_pin = IO_PORTC_10,//IO_PORT_USB_DMA,
#endif // UART2_PORT
    .output_channel = OUTPUT_CHANNEL3,
//    .input_channel = INPUT_CHANNEL1,
    .max_continue_recv_cnt = 1024,
    .idle_sys_clk_cnt = 500000,
    .clk_src = PLL_48M,
    .flags = UART_DEBUG,
UART2_PLATFORM_DATA_END();
#endif

/**********************************SD模块****************************************
 * 暂只支持1个SD外设，如需要多个SD外设，自行去掉下面SD0/1_ENABLE控制，另外定义即可
********************************************************************************/
#if TCFG_SD0_ENABLE
//sd0
#define SD_PLATFORM_DATA_BEGIN() 	SD0_PLATFORM_DATA_BEGIN(sd0_data)
#define SD_PLATFORM_DATA_END() 		SD0_PLATFORM_DATA_END()
#define SD_CLK_DETECT_FUNC			sdmmc_0_clk_detect
#elif TCFG_SD1_ENABLE
//sd1
#define SD_PLATFORM_DATA_BEGIN() 	SD1_PLATFORM_DATA_BEGIN(sd1_data)
#define SD_PLATFORM_DATA_END() 		SD1_PLATFORM_DATA_END()
#define SD_CLK_DETECT_FUNC			sdmmc_1_clk_detect
#endif

#if TCFG_SD0_ENABLE || TCFG_SD1_ENABLE

SD_PLATFORM_DATA_BEGIN()
    .port 					= TCFG_SD_PORTS,
    .priority 				= 3,
    .data_width 			= TCFG_SD_DAT_WIDTH,
	.speed 					= TCFG_SD_CLK,
    .detect_mode 			= TCFG_SD_DET_MODE,
#if (TCFG_SD_DET_MODE == SD_CLK_DECT)
    .detect_func 			= SD_CLK_DETECT_FUNC,
#elif (TCFG_SD_DET_MODE == SD_IO_DECT)
    .detect_func 			= sdmmc_0_io_detect,
    .detect_io              = TCFG_SD_DET_IO,
    .detect_io_level        = TCFG_SD_DET_IO_LEVEL,
#else
    .detect_func 			= NULL,
#endif
#if TCFG_SD_POWER_ENABLE
    .power                  = sd_set_power,
#endif
SD_PLATFORM_DATA_END()

#endif
/****************SD模块*********************/


HW_IIC0_PLATFORM_DATA_BEGIN(hw_iic1_data)
    .clk_pin = IO_PORTH_00,
    .dat_pin = IO_PORTH_01,
    .baudrate = 0x3f,
HW_IIC0_PLATFORM_DATA_END()

SW_IIC_PLATFORM_DATA_BEGIN(sw_iic0_data)
    .clk_pin = TCFG_SW_IIC_CLK_PORT,
    .dat_pin = TCFG_SW_IIC_DAT_PORT,
    .sw_iic_delay = 50,
SW_IIC_PLATFORM_DATA_END()

SW_IIC_PLATFORM_DATA_BEGIN(sw_iic1_data)
    .clk_pin = TCFG_SW_IIC1_CLK_PORT,
    .dat_pin = TCFG_SW_IIC1_DAT_PORT,
    .sw_iic_delay = 50,
SW_IIC_PLATFORM_DATA_END()

#if TCFG_IRKEY_ENABLE
const struct irkey_platform_data irkey_data = {
    .enable = 1,
    .port = IO_PORTA_09,
};
#endif

#ifdef TCFG_GPCNT_ENABLE
#include "gpcnt.h"
const struct gpcnt_platform_data gpcnt_data = {
    .gpcnt_gpio = IO_PORTB_01,
    .gss_clk_source = GPCNT_PLL_CLK,//480M
    .ch_source  = GPCNT_INPUT_CHANNEL1,
    .cycle      = CYCLE_15,
};
#endif

#if TCFG_RDEC_KEY_ENABLE
const struct rdec_device rdeckey_list[] = {
    {
        .index = RDEC0 ,
        .sin_port0 = IO_PORTB_06,
        .sin_port1 = IO_PORTB_07,
        .key_value0 = KEY_VOLUME_DEC | BIT(7),
        .key_value1 = KEY_VOLUME_INC | BIT(7),
    },
};
const struct rdec_platform_data rdec_key_data = {
    .enable = 1,
    .num = ARRAY_SIZE(rdeckey_list),            //RDEC按键的个数
    .rdec = rdeckey_list,                       //RDEC按键参数表
};
#endif

#if TCFG_IOKEY_ENABLE
static const struct iokey_port iokey_list[] = {
    {
        .connect_way = ONE_PORT_TO_LOW,         //IO按键的连接方式
        .key_type.one_io.port = TCFG_IOKEY_PORT,    //IO按键对应的引脚
        .key_value = KEY_OK,            //按键值
    },
};
const struct iokey_platform_data iokey_data = {
    .enable = 1,                              //是否使能IO按键
    .num = ARRAY_SIZE(iokey_list),            //IO按键的个数
    .port = iokey_list,                       //IO按键参数表
};
#endif

#if TCFG_TOUCH_KEY_ENABLE
static const struct touch_key_port plcnt_port[] = {
    {
        .port = IO_PORTH_02,
        .key_value = KEY_VOLUME_DEC,
    },
//    {
//        .port = IO_PORTC_10,
//        .key_value = KEY_VOLUME_INC,
//    },
};
const struct touch_key_platform_data touch_key_data = {
    .num            = ARRAY_SIZE(plcnt_port),
    .clock          = TOUCH_KEY_PLL_240M_CLK,
    .change_gain 	= 4,		//变化放大倍数, 一般固定
    .press_cfg		= -100,		//触摸按下灵敏度, 类型:s16, 数值越大, 灵敏度越高
    .release_cfg0 	= -50,		//触摸释放灵敏度0, 类型:s16, 数值越大, 灵敏度越高
    .release_cfg1 	= -80,		//触摸释放灵敏度1, 类型:s16, 数值越大, 灵敏度越高
    .port_list      = plcnt_port,
};
#endif

#if TCFG_CTMU_TOUCH_KEY_ENABLE
static const struct touch_key_port ctmu_port[] = {
    {
        .port = IO_PORTH_02,
        .key_value = KEY_POWER,
    },
//    {
//        .port = IO_PORTC_10,
//        .key_value = KEY_VOLUME_INC,
//    },
};
const struct touch_key_platform_data ctmkey_data = {
    .num            = ARRAY_SIZE(ctmu_port),
    .press_cfg		= -10,
    .release_cfg0 	= -50,
    .release_cfg1 	= -80,
    .port_list      = ctmu_port,
};
#endif

#if TCFG_ADKEY_ENABLE
/*
使用两个按键AD值中间值作为判断，如按键1的ad值为100，按键2的ad值为200，按键3的ad值为300
则：判断值从小值开始判断
ad值小于150是按键1，
ad值小于250是按键2，
ad值小于350是按键3，
*/
#define ADKEY_UPLOAD_R      22  //上拉电阻22K，单位K
#define ADKEY0_DOWNLOAD_R   0   //按键0下拉电阻0K，单位K
#define ADKEY1_DOWNLOAD_R   33  //按键1下拉电阻10K，单位K
#define ADKEY2_DOWNLOAD_R   51  //按键2下拉电阻22K，单位K
#define ADKEY3_DOWNLOAD_R   100 //按键3下拉电阻100K，单位K

#define ADKEY_NONE_V 	(0x3FF)//1023
#define ADKEY0_V 		(0x3FF * ADKEY0_DOWNLOAD_R / (ADKEY_UPLOAD_R + ADKEY0_DOWNLOAD_R))
#define ADKEY1_V 		(0x3FF * ADKEY1_DOWNLOAD_R / (ADKEY_UPLOAD_R + ADKEY1_DOWNLOAD_R))
#define ADKEY2_V 		(0x3FF * ADKEY2_DOWNLOAD_R / (ADKEY_UPLOAD_R + ADKEY2_DOWNLOAD_R))
#define ADKEY3_V 		(0x3FF * ADKEY3_DOWNLOAD_R / (ADKEY_UPLOAD_R + ADKEY3_DOWNLOAD_R))

#define ADKEY0_VAL      (ADKEY0_V + ADKEY1_V) / 2   //从小值开始放，两个按键AD值中间值作为判断
#define ADKEY1_VAL      (ADKEY1_V + ADKEY2_V) / 2   //两个按键AD值中间值作为判断
#define ADKEY2_VAL      (ADKEY2_V + ADKEY3_V) / 2   //两个按键AD值中间值作为判断
#define ADKEY2_END      (ADKEY2_V + ADKEY_NONE_V) / 2   //两个按键AD值中间值作为判断，最高ad加上峰值进行除2
#define ADKEY3_VAL      (ADKEY3_V + ADKEY_NONE_V) / 2   //两个按键AD值中间值作为判断，最高ad加上峰值进行除2

const struct adkey_platform_data adkey_data = {
    .enable     = 1,
	.adkey_pin  = TCFG_ADKEY_PORT,
    .extern_up_en = 1,
	.ad_channel = 3,
    .ad_value = {
        ADKEY0_VAL,
        ADKEY1_VAL,
        ADKEY2_VAL,
        ADKEY2_END,//3个按键，ADKEY3_VAL是4个按键
        ADKEY_NONE_V,
        ADKEY_NONE_V,
        ADKEY_NONE_V,
        ADKEY_NONE_V,
        ADKEY_NONE_V,
        ADKEY_NONE_V,
    },
    .key_value = {
        KEY_OK,
        KEY_UP,//KEY_MODE,
        KEY_DOWN,
        NO_KEY,//KEY_UP,
        NO_KEY,
        NO_KEY,
        NO_KEY,
        NO_KEY,
        NO_KEY,
        NO_KEY,
    },
};
#endif

/*
 * spi0接falsh
 */
SPI0_PLATFORM_DATA_BEGIN(spi0_data)
    .clk    = 40000000,
    .mode   = SPI_DUAL_MODE,
    .port   = 'A',
    .attr   = SPI_SCLK_L_UPL_SMPH | SPI_UPDATE_SAMPLE_SAME,
SPI0_PLATFORM_DATA_END()

SPI1_PLATFORM_DATA_BEGIN(spi1_data)
    .clk    = 40000000,
    .mode   = SPI_STD_MODE,
    .port   = 'B',
//    .attr	= SPI_SCLK_L_UPH_SMPH | SPI_BIDIR_MODE,
//    .attr	= SPI_SCLK_L_UPH_SMPL | SPI_BIDIR_MODE,
    .attr	= SPI_SCLK_L_UPL_SMPH | SPI_BIDIR_MODE,
//    .attr	= SPI_SCLK_L_UPL_SMPL | SPI_BIDIR_MODE,
//    .attr	= SPI_SCLK_L_UPH_SMPH | SPI_BIDIR_MODE,//WS2818 RGB灯
SPI1_PLATFORM_DATA_END()

#ifndef SPI2_CLOCK_HZ
#define SPI2_CLOCK_HZ   500000
#endif
#ifndef SPI2_ATTR_SET
#define SPI2_ATTR_SET   SPI_SCLK_H_UPL_SMPH | SPI_UNIDIR_MODE //单向单线
#endif
SPI2_PLATFORM_DATA_BEGIN(spi2_data)
    .clk    = SPI2_CLOCK_HZ,
    .mode   = SPI_STD_MODE,
    .port   = 'A',
    .attr	= SPI2_ATTR_SET,
SPI2_PLATFORM_DATA_END()


#ifdef CONFIG_UI_ENABLE
#if TCFG_LCD_ILI9341_ENABLE
#define EMI_BAUD_DIV	EMI_BAUD_DIV6
#else
#define EMI_BAUD_DIV	EMI_BAUD_DIV8
#endif
static const struct emi_platform_data emi_data = {
    .bits_mode      = EMI_8BITS_MODE,
    .baudrate       = EMI_BAUD_DIV,			//clock = HSB_CLK / (baudrate + 1) , HSB分频
    .colection      = EMI_FALLING_COLT,		//EMI_FALLING_COLT / EMI_RISING_COLT : 下降沿 上升沿 采集数据
    .time_out       = 1*1000,				//最大写超时时间ms
    .th             = EMI_TWIDTH_NO_HALF,
    .ts             = 0,
#if TCFG_LCD_ILI9481_ENABLE
    .tw             = EMI_BAUD_DIV5,
#else
    .tw             = (EMI_BAUD_DIV > 1) ? EMI_BAUD_DIV / 2 : 1,
#endif
    .data_bit_en    = 0,					//0默认根据bits_mode数据位来配置
};

static const struct pap_info pap_data = {
    .datawidth 		= PAP_PORT_8BITS,
    .endian    		= PAP_BE,				//数据输出大小端
    .cycle     		= PAP_CYCLE_ONE,		//1/2字节发送次数
    .pre			= PAP_READ_LOW,			//读取rd有效电平
    .pwe			= PAP_WRITE_LOW,		//写wr有效电平
    .use_sem		= TRUE,					//使用信号等待
    .rd_en			= FALSE,				//不使用rd读信号
    .port_sel		= PAP_PORT_A,			//PAP_PORT_A PAP_PORT_B
    .timing_setup 	= 1,					//具体看pap.h
    .timing_hold  	= 1,					//具体看pap.h
    .timing_width 	= 2,					//具体看pap.h
};

#if TCFG_LCD_BL_PWM_PORT
#define LCD_BL_PORT TCFG_LCD_BL_PWM_PORT
#else
#define LCD_BL_PORT IO_PORTH_03
#endif

static const struct ui_lcd_platform_data pdata = {
#if TCFG_LCD_ST7735S_ENABLE || TCFG_LCD_ST7789P3_ENABLE || TCFG_LCD_ST77912_ENABLE || TCFG_LCD_GC9D01_ENABLE || TCFG_LCD_ST7735P3_ENABLE || TCFG_LCD_JD9853_ENABLE
    .spi_id  = "spi1",
    .rs_pin  = IO_PORTC_00,
    .te_pin  = IO_PORTC_03,//IO_PORTH_07,
    .rst_pin = -1,//IO_PORTC_05,
    .cs_pin  = IO_PORTH_07,
#ifdef CONFIG_UI_TOW_EYE
    .cs1_pin = IO_PORTC_01,
#endif
    .bl_pin  = LCD_BL_PORT,
    .lcd_if  = LCD_SPI,//屏幕接口类型还有 PAP , SPI
#endif
#if TCFG_LCD_ILI9341_ENABLE || TCFG_LCD_ILI9481_ENABLE
    .rs_pin  = IO_PORTC_09,
    .te_pin  = IO_PORTC_00,
    .bl_pin  = IO_PORTB_08,
    .touch_int_pin = IO_PORTB_04,
    .touch_reset_pin = IO_PORTB_00,
    .cs_pin  = -1,
    .rst_pin = -1,
    .lcd_if  = LCD_EMI,
#endif
#if TCFG_LCD_480x272_8BITS
     .lcd_if  = LCD_IMD,
#endif
#if TCFG_LCD_ST7789V_ENABLE
    .rs_pin  = IO_PORTC_00,
    .te_pin  = -1,//IO_PORTC_00,
    .rst_pin = -1,
    .cs_pin  = IO_PORTH_07,
    .bl_pin  = LCD_BL_PORT,
    .touch_int_pin = IO_PORTH_04,
    .touch_reset_pin = IO_PORTH_00,
    .check_pin_D6 = IO_PORTH_02,
    .check_pin_D7 = IO_PORTA_00,
    .lcd_if  = LCD_EMI,//屏幕接口类型还有 PAP , SPI
#endif
#if TCFG_LCD_GC3907_ENABLE || TCFG_LCD_NV3031A_ENABLE || TCFG_LCD_AXS15252_ENABLE
    .spi_id  = "spi1",
    .rs_pin  = IO_PORTC_00,
    .te_pin  = IO_PORTC_03,//IO_PORTH_07,
    .rst_pin = IO_PORTC_02,//IO_PORTC_05,
    .cs_pin  = IO_PORTH_07,
#ifdef CONFIG_UI_TOW_EYE
    .cs1_pin = IO_PORTC_01,
#endif
    .bl_pin  = LCD_BL_PORT,

    .touch_int_pin = IO_PORTH_04,
    .touch_reset_pin = IO_PORTH_00,
    .check_pin_D6 = IO_PORTH_02,
    .check_pin_D7 = IO_PORTA_00,


    .lcd_if  = LCD_SPI,//屏幕接口类型还有 PAP , SPI
#endif

};
const struct ui_devices_cfg ui_cfg_data = {
    .type = TFT_LCD,
    .private_data = (void *)&pdata,
};
#endif

static const struct dac_platform_data dac_data = {
    .sw_differ = 0,
    .pa_auto_mute = TCFG_DAC_AUTO_MUTE_ENABLE,
    .pa_mute_port = TCFG_DAC_MUTE_PORT,
    .pa_mute_value = TCFG_DAC_MUTE_VALUE,
    .differ_output = TCFG_DAC_DIFF_OUT_ENABLE,//选择是否采用差分输出模式
    .hw_channel = TCFG_DAC_HW_CHANNEL_BIT,//BIT(0):DACFL、 BIT(1):DACFR、 BIT(2):DACRL、 BIT(3):DACRR
    .ch_num = TCFG_DAC_SOFT_CHANNEL_NUM,//差分只需开一个通道
    .vcm_init_delay_ms = 1000,
#if TCFG_DAC_AUTO_MUTE_ENABLE
    .mute_delay_ms = 200,
#endif
#if (defined CONFIG_DEC_ANALOG_VOLUME_ENABLE || defined NET_MUSIC_CONFIG_DEC_ANALOG_VOLUME_ENABLE)
    .fade_enable = 1,
    .fade_delay_ms = 50,
#endif
};

static const struct adc_platform_data adc_data = {
#ifdef CONFIG_ALL_ADC_CHANNEL_OPEN_ENABLE
    .mic_channel = TCFG_MIC_CHANNEL_MAP,
    .linein_channel = TCFG_LINEIN_CHANNEL_MAP,
    .mic_ch_num = TCFG_MIC_CHANNEL_NUM,
    .linein_ch_num = TCFG_LINEIN_CHANNEL_NUM,
    .all_channel_open = 1,
#else
    .mic_channel = LADC_CH_MIC1_P_N,
    .mic_ch_num = 1,
#endif
    .isel = 2,
    .dump_num = 480,
};

static const struct iis_platform_data iis0_data = {
    .channel_in = BIT(1),
    .channel_out = BIT(0),
    .port_sel = IIS_PORTA,
    .data_width = 0,
    .mclk_output = 0,
    .slave_mode = 0,
    .dump_points_num = 320,
};

static const struct iis_platform_data iis1_data = {
    .channel_in = BIT(0),
    .channel_out = BIT(1),
    .port_sel = IIS_PORTA,
    .data_width = 0,
    .mclk_output = 0,
    .slave_mode = 0,
    .dump_points_num = 320,
};

static void plnk0_port_remap_cb(void)
{
    //重映射PDM DAT-PH2   PDM CLK-PH3
    extern int gpio_plnk_rx_input(u32 gpio, u8 index, u8 data_sel);
    gpio_plnk_rx_input(IO_PORTH_02, 0, 0);
    gpio_output_channle(IO_PORTH_03, CH0_PLNK0_SCLK_OUT);	//SCLK0使用outputchannel0
    JL_IOMAP->CON3 |= BIT(18);
}

static void plnk0_port_unremap_cb(void)
{
    JL_IOMAP->CON3 &= ~BIT(18);
    gpio_clear_output_channle(IO_PORTH_03, CH0_PLNK0_SCLK_OUT);
    gpio_set_die(IO_PORTH_02, 0);
}

//plnk的时钟和数据引脚都采用重映射的使用例子
static const struct plnk_platform_data plnk0_data = {
    .hw_channel = PLNK_CH_MIC_L,
    .clk_out = 1,
    .port_remap_cb = plnk0_port_remap_cb,
    .port_unremap_cb = plnk0_port_unremap_cb,
    .sample_edge = 0,   //在CLK的下降沿采样左MIC
    .share_data_io = 1, //两个数字MIC共用一个DAT脚
    .high_gain = 1,
    .dc_cancelling_filter = 14,
    .dump_points_num = 640, //丢弃刚打开硬件时的数据点数
};

static const struct plnk_platform_data plnk1_data = {
    .hw_channel = PLNK_CH_MIC_DOUBLE,
    .clk_out = 1,
    .dc_cancelling_filter = 14,
    .dump_points_num = 640, //丢弃刚打开硬件时的数据点数
};

static const struct audio_pf_data audio_pf_d = {
    .adc_pf_data = &adc_data,
    .dac_pf_data = &dac_data,
    .iis0_pf_data = &iis0_data,
    .iis1_pf_data = &iis1_data,
    .plnk0_pf_data = &plnk0_data,
    .plnk1_pf_data = &plnk1_data,
};

static const struct audio_platform_data audio_data = {
    .private_data = (void *) &audio_pf_d,
};

#ifdef CONFIG_VIDEO_ENABLE
#define CAMERA_GROUP_PORT	ISC_GROUPA
/* #define CAMERA_GROUP_PORT	ISC_GROUPC */
static const struct camera_platform_data camera0_data = {
    .xclk_gpio      = TCFG_CAMERA_XCLK_PORT,//注意： 如果硬件xclk接到芯片IO，则会占用OUTPUT_CHANNEL1
    .reset_gpio     = TCFG_CAMERA_RESET_PORT,
    .online_detect  = NULL,
    .pwdn_gpio      = -1,
    .power_value    = 0,
    .interface      = SEN_INTERFACE0,//SEN_INTERFACE_CSI2,
    .dvp={
#if (CAMERA_GROUP_PORT == ISC_GROUPA)
        .pclk_gpio   = IO_PORTA_08,
        .hsync_gpio  = IO_PORTA_09,
        .vsync_gpio  = IO_PORTA_10,
#else
        .pclk_gpio   = IO_PORTC_08,
        .hsync_gpio  = IO_PORTC_09,
        .vsync_gpio  = IO_PORTC_10,
#endif
        .group_port  = CAMERA_GROUP_PORT,
        .data_gpio   = {
#if (CAMERA_GROUP_PORT == ISC_GROUPA)
                IO_PORTA_07,
                IO_PORTA_06,
                IO_PORTA_05,
                IO_PORTA_04,
                IO_PORTA_03,
                IO_PORTA_02,
                IO_PORTA_01,
                IO_PORTA_00,
                -1,
                -1,
#else
                IO_PORTC_07,
                IO_PORTC_06,
                IO_PORTC_05,
                IO_PORTC_04,
                IO_PORTC_03,
                IO_PORTC_02,
                IO_PORTC_01,
                IO_PORTC_00,
                -1,
                -1,
#endif
        },
    }
};
static const struct video_subdevice_data video0_subdev_data[] = {
    { VIDEO_TAG_CAMERA, (void *)&camera0_data },
};
static const struct video_platform_data video0_data = {
    .data = video0_subdev_data,
    .num = ARRAY_SIZE(video0_subdev_data),
};
#endif

LED_PLATFORM_DATA_BEGIN(pwm_led_data)
    .led_pin                = IO_PORTB_00, //固定IO,不能更改
    .clk_src                = PWM_LED_CLK_RC32K,
    .led_cfg.led0_bright    = 4,   //1 ~ 4, value越大, (红灯)亮度越高
    .led_cfg.led1_bright    = 4,   //1 ~ 4, value越大, (蓝灯)亮度越高
    .led_cfg.single_led_slow_freq   = 8,   //1 ~ 8, value越大, LED单独慢闪速度越慢, value * 0.5s闪烁一次
    .led_cfg.single_led_fast_freq   = 2,   //1 ~ 4, value越大, LED单独快闪速度越慢, value * 100ms闪烁一次
    .led_cfg.double_led_slow_freq   = 3,   //1 ~ 8, value越大, LED交替慢闪速度越慢, value * 0.5s闪烁一次
    .led_cfg.double_led_fast_freq   = 3,   //1 ~ 4, value越大, LED交替快闪速度越慢, value * 100ms闪烁一次
LED_PLATFORM_DATA_END()

#ifdef TCFG_LED_PWM0_PORT
#ifndef TCFG_LED_PWMCH_PORT
#define TCFG_LED_PWMCH_PORT 0
#endif
PWM_PLATFORM_DATA_BEGIN(pwm_data0)
    .port	= TCFG_LED_PWM0_PORT,
    .pwm_ch = PWM_TIMER3_OPCH3 | TCFG_LED_PWMCH_PORT,//初始化可选多通道,如:PWMCH0_H | PWMCH0_L | PWMCH1_H ... | PWMCH7_H | PWMCH7_L | PWM_TIMER2_OPCH2 | PWM_TIMER3_OPCH3 ,
    .freq   = 10000,//频率
    .duty   = 100,//占空比
    .point_bit = 0,//根据point_bit值调节占空比小数点精度位: 0<freq<=4K,point_bit=2;4K<freq<=40K,point_bit=1; freq>40K,point_bit=0;
PWM_PLATFORM_DATA_END()
#endif
#ifdef TCFG_PWM1_PORT
PWM_PLATFORM_DATA_BEGIN(pwm_data1)
    .port   = TCFG_PWM1_PORT,//选择定时器的TIMER PWM任意IO，pwm_ch加上PWM_TIMER3_OPCH3或PWM_TIMER2_OPCH2有效,只支持2个PWM,占用output_channel2/3，其他外设使用output_channel需留意
    .pwm_ch = PWM_TIMER2_OPCH2,//初始化可选多通道,如:PWMCH0_H | PWMCH0_L | PWMCH1_H ... | PWMCH7_H | PWMCH7_L | PWM_TIMER2_OPCH2 | PWM_TIMER3_OPCH3 ,
    .freq   = TCFG_PWM1_FREQ,//频率
    .duty   = TCFG_PWM1_FREQ_DUTY,//占空比
    .point_bit = 0,//根据point_bit值调节占空比小数点精度位: 0<freq<=4K,point_bit=2;4K<freq<=40K,point_bit=1; freq>40K,point_bit=0;
PWM_PLATFORM_DATA_END()
#elif defined TCFG_LCD_BL_PWM_PORT
PWM_PLATFORM_DATA_BEGIN(pwm_data1)
    .port   = TCFG_LCD_BL_PWM_PORT,//选择定时器的TIMER PWM任意IO，pwm_ch加上PWM_TIMER3_OPCH3或PWM_TIMER2_OPCH2有效,只支持2个PWM,占用output_channel2/3，其他外设使用output_channel需留意
    .pwm_ch = PWM_TIMER2_OPCH2,//初始化可选多通道,如:PWMCH0_H | PWMCH0_L | PWMCH1_H ... | PWMCH7_H | PWMCH7_L | PWM_TIMER2_OPCH2 | PWM_TIMER3_OPCH3 ,
    .freq   = 1000,//频率
    .duty   = 80,//占空比
    .point_bit = 0,//根据point_bit值调节占空比小数点精度位: 0<freq<=4K,point_bit=2;4K<freq<=40K,point_bit=1; freq>40K,point_bit=0;
PWM_PLATFORM_DATA_END()
#endif

#if TCFG_USB_SLAVE_ENABLE || TCFG_USB_HOST_ENABLE
static const struct otg_dev_data otg_data = {
#if defined CONFIG_MP_TEST_ENABLE
    .usb_dev_en = 0x02,
#else
#ifdef CONFIG_LTE_PHY_ENABLE
	//4G情况下不能全开，只开USB1，不占用另一个USB[用作串口打印或者产测调试相关]
    .usb_dev_en = CONFIG_USB_OTG_EN_VAL,
#else
    .usb_dev_en = 0x03,
#endif
#endif

#if TCFG_USB_SLAVE_ENABLE
    .slave_online_cnt = 10,
    .slave_offline_cnt = 10,
#endif
#if TCFG_USB_HOST_ENABLE
    .host_online_cnt = 10,
    .host_offline_cnt = 10,
#endif
    .detect_mode = OTG_HOST_MODE | OTG_SLAVE_MODE | OTG_CHARGE_MODE,
    .detect_time_interval = 50,
};

#if TCFG_VIR_UDISK_ENABLE
extern const struct device_operations ram_disk_dev_ops;
#endif
#endif

#if defined CONFIG_LTE_PHY_ENABLE
extern const struct device_operations lte_module_dev_ops;
LTE_MODULE_DATA_BEGIN(lte_module_data)
    .name = (u8 *)"usb_rndis",
LTE_MODULE_DATA_END()
#endif

#if defined CONFIG_BT_ENABLE || defined CONFIG_WIFI_ENABLE
#include "wifi/wifi_connect.h"
const struct wifi_calibration_param wifi_calibration_param = {
    .xosc_l     = 0x4,// 调节左晶振电容
    .xosc_r     = 0x3,// 调节右晶振电容
    .pa_trim_data = {1, 7, 4, 7, 11, 1, 7},// 根据MP测试生成PA TRIM值
    .mcs_dgain    = {
#ifdef YIERD_WIFI_MODULE
        50,//11B_1M
        50,//11B_2.2M
        50,//11B_5.5M
        50,//11B_11M
        39,//11G_6M
        39,//11G_9M
        54,//11G_12M
        54,//11G_18M
        54,//11G_24M
        53,//11G_36M
        52,//11G_48M
        46,//11G_54M
        33,//11N_MCS0
        47,//11N_MCS1
        46,//11N_MCS2
        45,//11N_MCS3
        45,//11N_MCS4
        44,//11N_MCS5
        44,//11N_MCS6
        38,//11N_MCS7
#else
#define GAIN_ADD    (-6)
        0x32,//11B_1M
        0x32,//11B_2.2M
        0x32,//11B_5.5M
        0x32,//11B_11M
        0x48 + GAIN_ADD,//11G_6M
        0x48 + GAIN_ADD,//11G_9M
        0x55 + GAIN_ADD,//11G_12M
        0x50 + GAIN_ADD,//11G_18M
        0x40 + GAIN_ADD,//11G_24M
        0x3D + GAIN_ADD,//11G_36M
        0x35 + GAIN_ADD,//11G_48M
        0x34 + GAIN_ADD,//11G_54M
        0x48 + GAIN_ADD,//11N_MCS0
        0x5A + GAIN_ADD,//11N_MCS1
        0x50 + GAIN_ADD,//11N_MCS2
        0x40 + GAIN_ADD,//11N_MCS3
        0x40 + GAIN_ADD,//11N_MCS4
        0x3D + GAIN_ADD,//11N_MCS5
        0x32 + GAIN_ADD,//11N_MCS6
        0x2B + GAIN_ADD,//11N_MCS7
#endif
    }
};
#endif

#ifdef CONFIG_EXT_WIFI_ENABLE
WIFI_PLATFORM_DATA_BEGIN(wifi_data)
#ifdef CONFIG_RTL8189E_ENABLE
	.module = RTL8189E,
#elif defined(CONFIG_RTL8822ES_10M_ENABLE)
	.module = RTL8822E,
#elif defined(CONFIG_RTL8822CS_ENABLE)
	.module = RTL8822C,
#elif defined(CONFIG_AIC8800D40_ENABLE)
	.module = AIC8800M40,
#endif
	.sdio_parm = SDIO_GRP_0 | SDIO_PORT_1 | SDIO_4_BIT_DATA | SDIO_DATA1_IRQ_MASK | SDIO_CLOCK_26M,//AC7916的使用PA5、6、7、8、9、10，和开发板默认SD卡使用同一组
	.wakeup_port = -1,
	.cs_port = -1,
	.power_port = -1,
WIFI_PLATFORM_DATA_END()
#endif


#ifdef TCFG_EXTFLASH_ENABLE
extern const struct device_operations extflash_dev_ops;
#endif

#ifdef CONFIG_RTC_ENABLE
const struct rtc_init_config rtc_init_data = {
#if 1//使用内部RTC时钟
    .rtc_clk_sel = RTC_CLK_SEL_INSIDE,
#else//使用外部32.768k时钟
    .rtc_clk_sel = RTC_CLK_SEL_OUTSIDE,
#endif
    .rtc_power_sel = RTCVDD_SUPPLY_OUTSIDE,
};
#endif

#ifdef CONFIG_DMSDX_ENABLE
static struct dmsdx_platform_data dmsdx_data0 = {
    .block_offset_start = CONFIG_SDNAND_HFS_LEN / 512,
    .block_offset_end = (CONFIG_SDNAND_HFS_LEN + CONFIG_SDNAND_HFAT_LEN) / 512,
};
static struct dmsdx_platform_data dmsdx_data1 = {
    .block_offset_start = (CONFIG_SDNAND_HFS_LEN + CONFIG_SDNAND_HFAT_LEN) / 512,
    .block_offset_end = (CONFIG_SDNAND_HFS_LEN + CONFIG_SDNAND_HFAT_LEN + CONFIG_SDNAND_FAT1_LEN) / 512,
};
#endif
#ifdef CONFIG_GSENSOR_ENABLE
const struct gsensor_platform_data gsensor_data = {
	.iic = "iic1",
};
#endif

REGISTER_DEVICES(device_table) = {
#ifdef CONFIG_UI_ENABLE
    { "pap",   &pap_dev_ops, (void *)&pap_data },
    { "emi",   &emi_dev_ops, (void *)&emi_data },
    { "imd",   &imd_dev_ops, NULL },
#endif
#ifdef TCFG_LED_PWM0_PORT
    { "pwm0",   &pwm_dev_ops,  (void *)&pwm_data0},
#endif
#if (defined TCFG_PWM1_PORT || defined TCFG_LCD_BL_PWM_PORT)
    { "pwm1",   &pwm_dev_ops,  (void *)&pwm_data1},
#endif
#if !TCFG_LCD_ILI9341_ENABLE && !TCFG_LCD_ILI9481_ENABLE
    //{ "pwm_led",&pwm_led_ops,  (void *)&pwm_led_data},
#endif

    { "iic0",  &iic_dev_ops, (void *)&sw_iic0_data },
    { "iic1",  &iic_dev_ops, (void *)&sw_iic1_data },

#ifdef CONFIG_GSENSOR_ENABLE
	{"gsensor", &gsensor_dev_ops, (void *)&gsensor_data},
#endif

    { "audio", &audio_dev_ops, (void *)&audio_data },

#if TCFG_SD0_ENABLE
    { "sd0",  &sd_dev_ops, (void *)&sd0_data },
#ifdef CONFIG_DMSDX_ENABLE
    { "sd0.0",  &dmsd_dev_ops, (void *)&dmsdx_data0 },
    { "sd0.1",  &dmsd_dev_ops, (void *)&dmsdx_data1 },
#endif
#endif

#if TCFG_SD1_ENABLE
    { "sd1",  &sd_dev_ops, (void *)&sd1_data },
#ifdef CONFIG_DMSDX_ENABLE
    { "sd1.0",  &dmsd_dev_ops, (void *)&dmsdx_data0 },
    { "sd1.1",  &dmsd_dev_ops, (void *)&dmsdx_data1 },
#endif
#endif

#ifdef CONFIG_VIDEO_ENABLE
    { "video0.*",  &video_dev_ops, (void *)&video0_data },
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    { "video1.*",  &video_dev_ops, (void *)&video1_data },//vdieo1为spi摄像头
    { "video3.*",  &video_dev_ops, (void *)&video1_data },//video3为双路(jpeg+yuv)dvp摄像头
#endif

#ifdef CONFIG_UVC_VIDEO2_ENABLE
    { "uvc", &uvc_dev_ops, NULL },
    { "video2.*",  &video_dev_ops, NULL },
#endif

#ifdef CONFIG_VIDEO_DEC_ENABLE
    { "video_dec",  &video_dev_ops, NULL },
#endif

    { "spi1", &spi_dev_ops, (void *)&spi1_data },
    { "spi2", &spi_dev_ops, (void *)&spi2_data },

    {"rtc", &rtc_dev_ops, NULL},

    {"uart0", &uart_dev_ops, (void *)&uart0_data },
    {"uart1", &uart_dev_ops, (void *)&uart1_data },
    {"uart2", &uart_dev_ops, (void *)&uart2_data },

#if TCFG_USB_SLAVE_ENABLE || TCFG_USB_HOST_ENABLE
    { "otg", &usb_dev_ops, (void *)&otg_data},
#endif
#if TCFG_UDISK_ENABLE
    { "udisk0", &mass_storage_ops, NULL },
    { "udisk1", &mass_storage_ops, NULL },
#endif
#if TCFG_VIR_UDISK_ENABLE
    {"vir_udisk",  &ram_disk_dev_ops, NULL},
#endif
#ifdef CONFIG_EXT_WIFI_ENABLE
    { "wifi",  &wifi_dev_ops, (void *) &wifi_data},
#endif
#ifdef CONFIG_LTE_PHY_ENABLE
    { "lte",  &lte_module_dev_ops, (void *)&lte_module_data},
#endif
#ifdef TCFG_EXTFLASH_ENABLE
    { "extflash",  &extflash_dev_ops, NULL},
#endif
#ifdef TCFG_GPCNT_ENABLE
    {"gpcnt", &gpcnt_dev_ops, (void *) &gpcnt_data },
#endif
};


/************************** LOW POWER config ****************************/
static const struct low_power_param power_param = {
    .config         = TCFG_LOWPOWER_LOWPOWER_SEL,
    .btosc_disable  = TCFG_LOWPOWER_BTOSC_DISABLE,         //进入低功耗时BTOSC是否保持
    .vddiom_lev     = TCFG_LOWPOWER_VDDIOM_LEVEL,          //强VDDIO等级,可选：2.2V  2.4V  2.6V  2.8V  3.0V  3.2V  3.4V  3.6V
    .vddiow_lev     = TCFG_LOWPOWER_VDDIOW_LEVEL,          //弱VDDIO等级,可选：2.1V  2.4V  2.8V  3.2V
    .vdc14_dcdc 	= TRUE,	   							   //打开内部1.4VDCDC，关闭则用外部
    .vdc14_lev		= VDC14_VOL_SEL_LEVEL, 				   //VDD1.4V配置
    .sysvdd_lev		= SYSVDD_VOL_SEL_LEVEL,				   //内核、sdram电压配置
    .vlvd_enable	= TRUE,                                //TRUE电压复位使能
    .vlvd_value		= VLVD_SEL_25V,                        //低电压复位电压值
};

/************************** PWR config ****************************/
//#define PORT_VCC33_CTRL_IO		IO_PORTA_03					//VCC33 DCDC控制引脚,该引脚控制DCDC器件输出的3.3V连接芯片HPVDD、VDDIO、VDD33
#ifdef TCFG_ADKEY_PORT
#define PORT_WAKEUP_IO			TCFG_ADKEY_PORT
#elif (defined TCFG_IOKEY_PORT)
#define PORT_WAKEUP_IO			TCFG_IOKEY_PORT
#else
#define PORT_WAKEUP_IO			IO_PORTB_01					//软关机和休眠唤醒引脚
#endif
#define PORT_WAKEUP_NUM			(PORT_WAKEUP_IO/IO_GROUP_NUM)//默认:0-7:GPIOA-GPIOH, 可以指定0-7组

static const struct port_wakeup port0 = {
    .edge       = FALLING_EDGE,                            //唤醒方式选择,可选：上升沿\下降沿
    .attribute  = BLUETOOTH_RESUME,                        //保留参数
    .iomap      = PORT_WAKEUP_IO,                          //唤醒口选择
    .low_power	= POWER_SLEEP_WAKEUP|POWER_OFF_WAKEUP,    //低功耗IO唤醒,不需要写0
};
#ifdef CONFIG_PRESS_LONG_KEY_POWERON
#define LONG_RESET_EN       TRUE
#else
#define LONG_RESET_EN       FALSE
#endif
static const struct long_press lpres_port = {
    .enable 	= LONG_RESET_EN,
    .use_sec4 	= FALSE,										//enable = TRUE , use_sec4: TRUE --> 4 sec , FALSE --> 8 sec
    .edge		= FALLING_EDGE,								//长按方式,可选：FALLING_EDGE /  RISING_EDGE --> 低电平/高电平
    .iomap 		= PORT_WAKEUP_IO,							//长按复位IO和IO唤醒共用一个IO
};

static const struct sub_wakeup sub_wkup = {
    .attribute  = BLUETOOTH_RESUME,
};

static const struct charge_wakeup charge_wkup = {
    .attribute  = BLUETOOTH_RESUME,
};

static const struct wakeup_param wk_param = {
    .port[PORT_WAKEUP_NUM] = &port0,
    .sub = &sub_wkup,
    .charge = &charge_wkup,
    .lpres = &lpres_port,
};

//ad按键和开关机键复用
unsigned int read_power_key_press(int dly)
{
#if TCFG_ADKEY_ENABLE
    gpio_latch_en(adkey_data.adkey_pin, 0);
    gpio_direction_input(adkey_data.adkey_pin);
    gpio_set_die(adkey_data.adkey_pin, 0);
    gpio_set_pull_up(adkey_data.adkey_pin, 0);
    gpio_set_pull_down(adkey_data.adkey_pin, 0);

    if (dly) {
        delay_us(3000);
    }

    JL_ADC->CON = (0xf << 12) | (adkey_data.ad_channel << 8) | (1 << 6) | (1 << 4) | (1 << 3) | (6 << 0);
    while (!(JL_ADC->CON & BIT(7)));
    return (JL_ADC->RES < 100);
#endif
#if TCFG_IOKEY_ENABLE
    char pu = iokey_list[0].connect_way == ONE_PORT_TO_LOW;
    gpio_latch_en(iokey_list[0].key_type.one_io.port, 0);
    gpio_direction_input(iokey_list[0].key_type.one_io.port);
    gpio_set_die(iokey_list[0].key_type.one_io.port, 1);
    gpio_set_pull_up(iokey_list[0].key_type.one_io.port, pu);
    gpio_set_pull_down(iokey_list[0].key_type.one_io.port, !pu);
    if (dly) {
        delay_us(3000);
    }
    if (iokey_list[0].connect_way == ONE_PORT_TO_LOW) {
        return (!gpio_read(iokey_list[0].key_type.one_io.port));
    } else {
        return (gpio_read(iokey_list[0].key_type.one_io.port));
    }
#endif
}

void sys_power_poweroff_wait_powerkey_up(void)
{
    JL_TIMER1->CON = 0;
    JL_TIMER1->CON = 0;
    JL_TIMER1->CON = 0;
    JL_ADC->CON = 0;
    JL_ADC->CON = 0;
    JL_ADC->CON = 0;
    int to = 1000;//1000*3m，最多等待3秒
    while (read_power_key_press(1) && --to);
}

void dac_mute_control(char enable, char force)
{
    if(enable){
        gpio_latch_en(TCFG_DAC_MUTE_PORT, 0);
        gpio_direction_output(TCFG_DAC_MUTE_PORT, TCFG_DAC_MUTE_VALUE);
        gpio_set_hd(TCFG_DAC_MUTE_PORT, 0);//务必开启强驱
        if (force) {
            gpio_latch_en(TCFG_DAC_MUTE_PORT, 1);
        }
    } else if (!TCFG_DAC_AUTO_MUTE_ENABLE || force) {
        if (force) {
            gpio_latch_en(TCFG_DAC_MUTE_PORT, 0);
        }
        gpio_direction_output(TCFG_DAC_MUTE_PORT, !TCFG_DAC_MUTE_VALUE);
        gpio_set_hd(TCFG_DAC_MUTE_PORT, 1);//务必开启强驱
		//gpio_latch_en(TCFG_DAC_MUTE_PORT, 0);
    }
}

void dac_mute_control_init(void)
{
    if (!TCFG_DAC_AUTO_MUTE_ENABLE) {
        gpio_latch_en(TCFG_DAC_MUTE_PORT, 0);
        gpio_direction_input(TCFG_DAC_MUTE_PORT);
    }
    //gpio_direction_output(TCFG_DAC_MUTE_PORT, !TCFG_DAC_MUTE_VALUE);
    //gpio_set_hd(TCFG_DAC_MUTE_PORT, 1);//务必开启强驱
}

void dac_mute_control_sleep(void)
{
    gpio_latch_en(TCFG_DAC_MUTE_PORT, 0);
    gpio_direction_output(TCFG_DAC_MUTE_PORT, TCFG_DAC_MUTE_VALUE);
    gpio_set_pull_up(TCFG_DAC_MUTE_PORT, TCFG_DAC_MUTE_VALUE);
    gpio_set_pull_down(TCFG_DAC_MUTE_PORT, !TCFG_DAC_MUTE_VALUE);
    gpio_set_direction(TCFG_DAC_MUTE_PORT, 1);
    gpio_set_hd(TCFG_DAC_MUTE_PORT, 0);
}

/*进软关机之前默认将IO口都设置成高阻状态，需要保留原来状态的请修改该函数*/
static void board_set_soft_poweroff(void)
{
    u32 IO_PORT;
    JL_PORT_FLASH_TypeDef *gpio[] = {JL_PORTA, JL_PORTB, JL_PORTC, JL_PORTD, JL_PORTE, JL_PORTF, JL_PORTG, JL_PORTH};

#ifdef TCFG_IOVDD_CTRL_PORT
    gpio_latch_en(TCFG_IOVDD_CTRL_PORT, 0);
    gpio_direction_output(TCFG_IOVDD_CTRL_PORT, 0);
    gpio_set_pull_up(TCFG_IOVDD_CTRL_PORT, 0);
    gpio_set_pull_down(TCFG_IOVDD_CTRL_PORT, 1);
    gpio_set_direction(TCFG_IOVDD_CTRL_PORT, 1);
    gpio_set_die(TCFG_IOVDD_CTRL_PORT, 0);
    gpio_set_dieh(TCFG_IOVDD_CTRL_PORT, 0);
    gpio_set_dieh(TCFG_IOVDD_CTRL_PORT, 0);
    gpio_set_hd(TCFG_IOVDD_CTRL_PORT, 0);
    gpio_set_hd1(TCFG_IOVDD_CTRL_PORT, 0);
    gpio_latch_en(TCFG_IOVDD_CTRL_PORT, 1);
#endif
#ifdef TCFG_VCC33_CTRL_PORT
    gpio_latch_en(TCFG_VCC33_CTRL_PORT, 0);
    gpio_direction_output(TCFG_VCC33_CTRL_PORT, 0);
    gpio_set_pull_up(TCFG_VCC33_CTRL_PORT, 0);
    gpio_set_pull_down(TCFG_VCC33_CTRL_PORT, 1);
    gpio_set_direction(TCFG_VCC33_CTRL_PORT, 1);
    gpio_set_die(TCFG_VCC33_CTRL_PORT, 0);
    gpio_set_dieh(TCFG_VCC33_CTRL_PORT, 0);
    gpio_set_hd(TCFG_VCC33_CTRL_PORT, 0);
    gpio_set_hd1(TCFG_VCC33_CTRL_PORT, 0);
    gpio_latch_en(TCFG_VCC33_CTRL_PORT, 1);
#endif


    dac_mute_control_sleep();

    memset(JL_PWM, 0, sizeof(JL_MCPWM_TypeDef));//PWM无法完全关闭问题

    for (u8 p = 0; p < 8; ++p) {
        //flash sdram PD PE PF PG口不能进行配置,由内部完成控制
        if (gpio[p] == JL_PORTD || gpio[p] == JL_PORTE || gpio[p] == JL_PORTF || gpio[p] == JL_PORTG) {
            continue;
        }
        for (u8 i = 0; i < 16; ++i) {
            IO_PORT = IO_PORTA_00 + p * 16 + i;
#ifdef TCFG_VCC33_CTRL_PORT
            if(TCFG_VCC33_CTRL_PORT == IO_PORT){
                continue;
            }
#endif
#ifdef TCFG_IOVDD_CTRL_PORT
            if(TCFG_IOVDD_CTRL_PORT == IO_PORT){
                continue;
            }
#endif
            gpio_latch_en(IO_PORT, 0);
            gpio_set_pull_up(IO_PORT, 0);
            gpio_set_pull_down(IO_PORT, 0);
            gpio_set_direction(IO_PORT, 1);
            gpio_set_die(IO_PORT, 0);
            gpio_set_dieh(IO_PORT, 0);
            gpio_set_hd(IO_PORT, 0);
            gpio_set_hd1(IO_PORT, 0);
            gpio_latch_en(IO_PORT, 1);
        }
    }
    for (u8 p = IO_PORT_USB_DPA; p <= IO_PORT_USB_DMB; ++p) {
        IO_PORT = p;
        gpio_latch_en(IO_PORT, 0);
        gpio_set_pull_up(IO_PORT, 0);
        gpio_set_pull_down(IO_PORT, 0);
        gpio_set_direction(IO_PORT, 1);
        gpio_set_die(IO_PORT, 0);
        gpio_set_dieh(IO_PORT, 0);
        gpio_set_hd(IO_PORT, 0);
        gpio_set_hd1(IO_PORT, 0);
        gpio_latch_en(IO_PORT, 1);
    }
    for (u8 p = IO_PORT_PR_00; p <= IO_PORT_PR_03; ++p) {
        IO_PORT = p;
        if (rtc_init_data.rtc_clk_sel == RTC_CLK_SEL_OUTSIDE && (IO_PORT == IO_PORT_PR_00 || IO_PORT == IO_PORT_PR_01)) {
            continue;
        }
        gpio_latch_en(IO_PORT, 0);
        gpio_set_pull_up(IO_PORT, 0);
        gpio_set_pull_down(IO_PORT, 1);
        gpio_set_direction(IO_PORT, 1);
        gpio_set_die(IO_PORT, 0);
        gpio_set_dieh(IO_PORT, 0);
        gpio_set_hd(IO_PORT, 0);
        gpio_set_hd1(IO_PORT, 0);
        gpio_latch_en(IO_PORT, 1);
    }
}

static void sleep_exit_callback(u32 usec)
{
#ifdef TCFG_VCC33_CTRL_PORT
	gpio_direction_output(TCFG_VCC33_CTRL_PORT, 1);
	gpio_set_pull_up(TCFG_VCC33_CTRL_PORT, 1);
	gpio_set_pull_down(TCFG_VCC33_CTRL_PORT,0);
#endif
#ifdef TCFG_IOVDD_CTRL_PORT
	gpio_direction_output(TCFG_IOVDD_CTRL_PORT, 1);
	gpio_set_pull_up(TCFG_IOVDD_CTRL_PORT, 1);
	gpio_set_pull_down(TCFG_IOVDD_CTRL_PORT,0);
#endif
    /* putchar('-'); */
}

static void sleep_enter_callback(u8  step)
{
    /* 此函数禁止添加打印 */
    if (step == 1) {
        dac_power_off();
    } else {
#ifdef TCFG_VCC33_CTRL_PORT
		gpio_direction_output(TCFG_VCC33_CTRL_PORT, 0);
		gpio_set_pull_up(TCFG_VCC33_CTRL_PORT, 0);
		gpio_set_pull_down(TCFG_VCC33_CTRL_PORT, 1);
		gpio_set_direction(TCFG_VCC33_CTRL_PORT, 1);
		gpio_set_die(TCFG_VCC33_CTRL_PORT, 0);
#endif
#ifdef TCFG_IOVDD_CTRL_PORT
		gpio_direction_output(TCFG_IOVDD_CTRL_PORT, 0);
		gpio_set_pull_up(TCFG_IOVDD_CTRL_PORT, 0);
		gpio_set_pull_down(TCFG_IOVDD_CTRL_PORT, 1);
		gpio_set_direction(TCFG_IOVDD_CTRL_PORT, 1);
		gpio_set_die(TCFG_IOVDD_CTRL_PORT, 0);
#endif
	}
}

static void board_power_init(void)
{
    gpio_direction_input(IO_PORTB_08);
    gpio_set_pull_up(IO_PORTB_08, 0);
	gpio_set_pull_down(IO_PORTB_08,0);

	gpio_direction_input(IO_PORT_USB_DPB);
    gpio_set_pull_up(IO_PORT_USB_DPB, 0);
	gpio_set_pull_down(IO_PORT_USB_DPB,0);

    gpio_direction_input(IO_PORT_USB_DMB);
    gpio_set_pull_up(IO_PORT_USB_DMB, 0);
	gpio_set_pull_down(IO_PORT_USB_DMB,0);

	gpio_direction_input(IO_PORT_PR_00);
    gpio_set_pull_up(IO_PORT_PR_00, 0);
	gpio_set_pull_down(IO_PORT_PR_00,0);

	gpio_direction_input(IO_PORTB_00);
    gpio_set_pull_up(IO_PORTB_00, 0);
	gpio_set_pull_down(IO_PORTB_00,0);

    gpio_direction_input(IO_PORT_PR_01);
    gpio_set_pull_up(IO_PORT_PR_01, 0);
	gpio_set_pull_down(IO_PORT_PR_01,0);

#ifdef TCFG_VCC33_CTRL_PORT
    gpio_latch_en(TCFG_VCC33_CTRL_PORT, 0);
	gpio_direction_output(TCFG_VCC33_CTRL_PORT, 1);
    gpio_set_pull_up(TCFG_VCC33_CTRL_PORT, 1);
	gpio_set_pull_down(TCFG_VCC33_CTRL_PORT,0);
#endif

    power_init(&power_param);

    power_set_callback(TCFG_LOWPOWER_LOWPOWER_SEL, sleep_enter_callback, sleep_exit_callback, board_set_soft_poweroff);

#ifdef CONFIG_AUDIO_ENABLE
    power_keep_state(POWER_KEEP_RESET | POWER_KEEP_DACVDD);//0, POWER_KEEP_DACVDD | POWER_KEEP_RTC | POWER_KEEP_RESET
#else
    power_keep_state(POWER_KEEP_RESET);//0, POWER_KEEP_DACVDD | POWER_KEEP_RTC | POWER_KEEP_RESET
#endif

#ifdef CONFIG_RTC_ENABLE
    power_keep_state(POWER_KEEP_RTC);
#endif
    /* power_keep_state(POWER_KEEP_PWM_LED); */

    power_wakeup_init(&wk_param);

#ifdef CONFIG_PRESS_LONG_KEY_POWERON
#ifdef CONFIG_POWEROFF_NO_VBAT
    {
#else
    if (system_reset_reason_get() == SYS_RST_PORT_WKUP) {
#endif
        for (int i = 0; i < 500; i++) {//500ms * 3
            if (!read_power_key_press(1)) {
                sys_power_poweroff();
                break;
            }
        }
    }
#endif

	//VDDIO电源放在V33电源使能之后，并且正常开机后
#ifdef TCFG_IOVDD_CTRL_PORT
    gpio_latch_en(TCFG_IOVDD_CTRL_PORT, 0);
	gpio_direction_output(TCFG_IOVDD_CTRL_PORT, 1);
    gpio_set_pull_up(TCFG_IOVDD_CTRL_PORT, 1);
	gpio_set_pull_down(TCFG_IOVDD_CTRL_PORT,0);
#endif

    extern void clean_wakeup_source_port(void);
    clean_wakeup_source_port();
    dac_mute_control_init();
}

#ifdef CONFIG_DEBUG_ENABLE
void debug_uart_init()
{
#ifdef RF_FCC_TEST_ENABLE
	uart_init(&uart2_data);
#else
    uart_init(&uart1_data);
#endif
}
#endif

void board_early_init()
{
#ifdef CONFIG_AUDIO_ENABLE
    dac_early_init(0, dac_data.differ_output ? (dac_data.ch_num > 1 ? 0xf : 0x3): dac_data.hw_channel, 1000);
#endif
    devices_init();
}

void board_init()
{
    board_power_init();
#ifdef CONFIG_RTC_ENABLE
    rtc_early_init();
#endif
#if TCFG_ADKEY_ENABLE || (defined CONFIG_BT_ENABLE || defined CONFIG_WIFI_ENABLE)
    adc_init();
#endif
    key_driver_init();
#ifdef CONFIG_AUTO_SHUTDOWN_ENABLE
    sys_power_init();
#endif
#ifdef CONFIG_BT_ENABLE
    void cfg_file_parse(void);
    cfg_file_parse();
#endif

}

#endif
