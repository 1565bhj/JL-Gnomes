#include "app_config.h"

#include "system/includes.h"
#include "device/includes.h"
#include "asm/includes.h"

// *INDENT-OFF*

UART1_PLATFORM_DATA_BEGIN(uart1_data)
    .baudrate = 115200,
    .port = PORTUSB_A,
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
	.output_channel = OUTPUT_CHANNEL0,
	//.tx_pin = IO_PORTC_00,
	.tx_pin = IO_PORTA_10,
	.rx_pin = -1,
	.max_continue_recv_cnt = 1024,
	.idle_sys_clk_cnt = 500000,
	.clk_src = PLL_48M,
	.flags = UART_DEBUG,
UART2_PLATFORM_DATA_END();

/****************SD模块**********************************************************
* 暂只支持1个SD外设，如需要多个SD外设，自行去掉下面SD0/1_ENABLE控制，另外定义即可
********************************************************************************/
#if (TCFG_SD0_ENABLE || TCFG_SD1_ENABLE)
#if TCFG_SD0_ENABLE
  //sd0
  #define SD_PLATFORM_DATA_BEGIN() SD0_PLATFORM_DATA_BEGIN(sd0_data)
  #define SD_PLATFORM_DATA_END() SD0_PLATFORM_DATA_END()
  #define SD_CLK_DETECT_FUNC          sdmmc_0_clk_detect
#elif TCFG_SD1_ENABLE
  //sd1
  #define SD_PLATFORM_DATA_BEGIN() SD1_PLATFORM_DATA_BEGIN(sd1_data)
  #define SD_PLATFORM_DATA_END() SD1_PLATFORM_DATA_END()
  #define SD_CLK_DETECT_FUNC          sdmmc_1_clk_detect
#endif

SD_PLATFORM_DATA_BEGIN()
      .port                               = TCFG_SD_PORTS,
      .priority                           = 3,
      .data_width                         = TCFG_SD_DAT_WIDTH,
      .speed                                      = TCFG_SD_CLK,
      .detect_mode                        = TCFG_SD_DET_MODE,
#if (TCFG_SD_DET_MODE == SD_CLK_DECT)
      .detect_func                        = SD_CLK_DETECT_FUNC,
#elif (TCFG_SD_DET_MODE == SD_IO_DECT)
      .detect_func                        = sdmmc_0_io_detect,
#else
      .detect_func                        = NULL,
      .power                  			  = NULL,	//sd_set_power, //是否需要PB8提供SD电源
#endif

SD_PLATFORM_DATA_END()
#endif


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

#if defined CONFIG_BT_ENABLE || defined CONFIG_WIFI_ENABLE
#include "wifi/wifi_connect.h"
const struct wifi_calibration_param wifi_calibration_param = {
    .xosc_l     = 0xb,// 调节左晶振电容
    .xosc_r     = 0xb,// 调节右晶振电容
    .pa_trim_data  ={1, 7, 4, 7, 11, 1, 7},// 根据MP测试生成PA TRIM值
	.mcs_dgain     ={
		50,//11B_1M
	  	50,//11B_2.2M
	  	50,//11B_5.5M
		50,//11B_11M

		72,//11G_6M
		72,//11G_9M
		85,//11G_12M
		80,//11G_18M
		64,//11G_24M
		64,//11G_36M
		62,//11G_48M
		52,//11G_54M

		72,//11N_MCS0
		90,//11N_MCS1
		80,//11N_MCS2
		64,//11N_MCS3
		64,//11N_MCS4
		64,//11N_MCS5
		50,//11N_MCS6
		43,//11N_MCS7
	}
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
        ADKEY3_VAL,
        ADKEY_NONE_V,
        ADKEY_NONE_V,
        ADKEY_NONE_V,
        ADKEY_NONE_V,
        ADKEY_NONE_V,
        ADKEY_NONE_V,
    },
    .key_value = {
        KEY_OK,
        KEY_MODE,
        KEY_DOWN,
        KEY_UP,
        NO_KEY,
        NO_KEY,
        NO_KEY,
        NO_KEY,
        NO_KEY,
        NO_KEY,
    },
};
#endif

REGISTER_DEVICES(device_table) = {
    {"uart1", &uart_dev_ops, (void *)&uart1_data },
	{"uart2", &uart_dev_ops, (void *)&uart2_data },
	{"rtc", &rtc_dev_ops, NULL},
#if TCFG_SD0_ENABLE
	{ "sd0",  &sd_dev_ops, (void *)&sd0_data },
#endif
#if TCFG_SD1_ENABLE
	{ "sd1",  &sd_dev_ops, (void *)&sd1_data },
#endif
};

#ifdef CONFIG_DEBUG_ENABLE
void debug_uart_init()
{
    uart_init(&uart2_data);
}
#endif

void board_early_init()
{
    devices_init();
}

static void board_power_init(void)
{
#ifdef TCFG_VCC33_CTRL_PORT
    gpio_latch_en(TCFG_VCC33_CTRL_PORT, 0);
	gpio_direction_output(TCFG_VCC33_CTRL_PORT, 1);
    gpio_set_pull_up(TCFG_VCC33_CTRL_PORT, 1);
	gpio_set_pull_down(TCFG_VCC33_CTRL_PORT,0);
#endif

    power_init(&power_param);

	//VDDIO电源放在V33电源使能之后，并且正常开机后
#ifdef TCFG_IOVDD_CTRL_PORT
	gpio_latch_en(TCFG_IOVDD_CTRL_PORT, 0);
	gpio_direction_output(TCFG_IOVDD_CTRL_PORT, 1);
	gpio_set_pull_up(TCFG_IOVDD_CTRL_PORT, 1);
	gpio_set_pull_down(TCFG_IOVDD_CTRL_PORT, 0);
#endif
}

void board_init()
{
    board_power_init();
#if TCFG_ADKEY_ENABLE || (defined CONFIG_BT_ENABLE || defined CONFIG_WIFI_ENABLE)
	adc_init();
#endif

#if TCFG_ADKEY_ENABLE
	key_driver_init();
#endif // TCFG_ADKEY_ENABLE
}

