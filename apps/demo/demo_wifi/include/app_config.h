#ifndef APP_CONFIG_H
#define APP_CONFIG_H
//AC7916N
#define __FLASH_SIZE__    (8 * 1024 * 1024)
#define __SDRAM_SIZE__    (8 * 1024 * 1024)

#define CONFIG_WIFI_ENABLE  /* 无线WIFI */
#ifdef CONFIG_NO_SDRAM_ENABLE
#define CONFIG_RF_TRIM_CODE_MOVABLE //把RF TRIM 的运行代码动态加载到ram运行(节省4K ram内存), 防止RF TRIM 期间500ms大电流访问flash造成flash挂掉持续大电流
#else
#define CONFIG_RF_TRIM_CODE_AT_RAM  //把RF TRIM 的运行代码定死到ram运行(浪费4K ram内存,否则若动态加载到sdram需清cache), 防止RF TRIM 期间500ms大电流访问flash造成flash挂掉持续大电流
#endif
//#define CONFIG_ASSIGN_MACADDR_ENABLE  //第一次开机连上外网后，使用杰理服务器分配WIFI模块的MAC地址, 关闭则使用<蓝牙地址更新工具*.exe>或者随机数作为MAC地址
#define CONFIG_IPERF_ENABLE         // iperf测试
#define  CONFIG_AIRKISS_NET_CFG     //AIRKISS配网
#define RTOS_STACK_CHECK_ENABLE
#define CONFIG_STATIC_IPADDR_ENABLE //记忆路由器分配的IP,下次直接使用记忆IP节省DHCP时间

//*********************************************************************************//
//                                    电源配置                                     //
//*********************************************************************************//
//#define CONFIG_LOW_POWER_ENABLE              //WIFI节能模式开关
#define TCFG_LOWPOWER_BTOSC_DISABLE         0

#ifdef CONFIG_LOW_POWER_ENABLE
#define TCFG_LOWPOWER_LOWPOWER_SEL          RF_SLEEP_EN //配置仅WIFI RF休眠
#else
#define TCFG_LOWPOWER_LOWPOWER_SEL          0
#endif
#define TCFG_LOWPOWER_VDDIOM_LEVEL          VDDIOM_VOL_32V       //强VDDIO电压档位，不要高于外部DCDC的电压

#ifdef CONFIG_RTC_ENABLE
#define TCFG_LOWPOWER_VDDIOW_LEVEL          VDDIOW_VOL_32V       //弱VDDIO电压档位。RTCVDD电压低于3.2V可能不走时，因此RTCVDD由IOVDD供电时，VDDIOW应设置为VDDIOW_VOL_32V档
#else
#define TCFG_LOWPOWER_VDDIOW_LEVEL          VDDIOW_VOL_21V       //弱VDDIO电压档位
#endif

#define VDC14_VOL_SEL_LEVEL                 VDC14_VOL_SEL_140V   //RF1.4V电压档位
#define SYSVDD_VOL_SEL_LEVEL                SYSVDD_VOL_SEL_126V  //内核电压档位值

//*********************************************************************************//
//                                  FCC测试相关配置                                //
//*********************************************************************************//
//#define RF_FCC_TEST_ENABLE//使能RF_FCC测试，详细配置见"apps/common/rf_fcc_tool/include/rf_fcc_main.h"


#define CONFIG_DEBUG_ENABLE                     /* 打印开关 */
//#define SDTAP_DEBUG
#if !defined CONFIG_DEBUG_ENABLE || defined CONFIG_LIB_DEBUG_DISABLE
#define LIB_DEBUG    0
#else
#define LIB_DEBUG    1
#endif
#define CONFIG_DEBUG_LIB(x)         (x & LIB_DEBUG)

//*********************************************************************************//
//                                  BT_BLE配置                                     //
//*********************************************************************************//
#ifdef CONFIG_BT_ENABLE

#ifdef CONFIG_WFT_ENBALE
#define BLE_DEV_NAME                "XWAI_AC_"
#define BT_DEV_NAME                 "XWAI_"
#define BT_DEV_NAME_LONE_ENABLE            //升级设备名称长名称
#else
#define BLE_DEV_NAME                "QYAI_AC_"
#define BT_DEV_NAME                 "QYAI_"
#endif
#endif


//*********************************************************************************//
//                            app适配qyai-开发板                                        //
//*********************************************************************************//
//qyai-开发板用SD0
#define TCFG_SD0_ENABLE    1   //使用SD0 IO
//#define TCFG_SD1_ENABLE    1 //使用SD1 IO

//qyai-开发板：B组，SD_CMD_DECT检测插拔事件
#if (TCFG_SD0_ENABLE || TCFG_SD1_ENABLE)
#define TCFG_SD_PORTS                      'B'  //SD0/SD1的ABCD组, 注意:IO占用问题
#define TCFG_SD_DAT_WIDTH                  1    //1:单线模式, 4:四线模式
#define TCFG_SD_DET_MODE                   SD_CMD_DECT      //检测模式:命令检测，时钟检测，IO检测
#define TCFG_SD_DET_IO                     IO_PORTA_01      //SD_DET_MODE为SD_IO_DECT时有效
#define TCFG_SD_DET_IO_LEVEL               0                //IO检卡上线的电平(0/1),SD_DET_MODE为SD_IO_DECT时有效
#define TCFG_SD_CLK                        24000000         //SD时钟
#endif

#if TCFG_SD0_ENABLE
#define CONFIG_STORAGE_PATH         "storage/sd0"  //定义对应SD0的路径
#define SDX_DEV                     "sd0"
#endif

#if TCFG_SD1_ENABLE
#define CONFIG_STORAGE_PATH         "storage/sd1" //定义对应SD1的路径
#define SDX_DEV                     "sd1"
#endif

#ifndef CONFIG_STORAGE_PATH
#define CONFIG_STORAGE_PATH         "no_sd_card" //不使用SD定义对应别的路径，防止编译出错
#define SDX_DEV                     "no_sd"
#endif

//指定根目录
#define CONFIG_ROOT_PATH            CONFIG_STORAGE_PATH"/C/" //定义对应SD文件系统的根目录路径

//AD按键使能
#define TCFG_ADKEY_ENABLE           1
#define TCFG_ADKEY_PORT             IO_PORTB_01 //ADC按键IO

//PMU相关
#define TCFG_VCC33_CTRL_PORT            IO_PORTB_03 //3.3V电原控制IO
#define TCFG_IOVDD_CTRL_PORT            IO_PORTB_00 //VDDIO电原控制IO

//*********************************************************************************//
//                        SD 配置（暂只支持打开一个SD外设）                        //
//*********************************************************************************//
//SD0   cmd,  clk,  data0, data1, data2, data3
//A     PB6   PB7   PB5    PB5    PB3    PB2
//B     PA7   PA8   PA9    PA10   PA5    PA6
//C     PH1   PH2   PH0    PH3    PH4    PH5
//D     PC9   PC10  PC8    PC7    PC6    PC5

//SD1   cmd,  clk,  data0, data1, data2, data3
//A     PH6   PH7   PH5    PH4    PH3    PH2
//B     PC0   PC1   PC2    PC3    PC4    PC5

#endif

