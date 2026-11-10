#include "system/includes.h"
#include "asm/system_reset_reason.h"
#include "action.h"
#include "app_config.h"
#include "storage_device.h"
#include "generic/log.h"
#include "os/os_api.h"
#include "event/key_event.h"
#include "event/bt_event.h"
#include "event/device_event.h"
#include "event/net_event.h"
#include "wifi/wifi_connect.h"
#include "fs/fs.h"
#include "asm/p33.h"
#include "asm/gpio.h"
#include "fs/fs.h"
#include "ai_uart_ctrol.h"
#include "qyai_config.h"

extern int sys_net_channel_read(void);
extern int wireless_net_init(void);
extern int lte_net_init(void);
extern int flash_protect_check_match(void);
extern int production_test_io_get(void);
extern void product_function_app_start(void);
extern int play_face_emoji(int gif_index);
extern void lvgl_test_demo(void);
extern int vbat_check_percent(void);
extern int user_is_soft_power_off_read(void);
extern int audio_app_mode_switch(char *name);


/* 中断列表 */
const struct irq_info irq_info_table[] = {
    //中断号   //优先级0-7   //注册的cpu(0或1)
#ifdef CONFIG_IPMASK_ENABLE
    //不可屏蔽中断方法：支持写flash，但中断函数和调用函数和const要全部放在内部ram
    { IRQ_SOFT5_IDX,      6,   0    }, //此中断强制注册到cpu0
    { IRQ_SOFT4_IDX,      6,   1    }, //此中断强制注册到cpu1
#if 0 //如下，SPI1使用不可屏蔽中断设置
    { IRQ_SPI1_IDX,      7,   1    },//中断强制注册到cpu0/1
#endif
#if 0 //如下，CTMU触摸大于3个IO则使用不可屏蔽中断设置
    { IRQ_CTM_IDX,      7,   1    },//中断强制注册到cpu0/1
#endif
#endif
#if CPU_CORE_NUM == 1
    { IRQ_SOFT5_IDX,      7,   0    }, //此中断强制注册到cpu0
    { IRQ_SOFT4_IDX,      7,   1    }, //此中断强制注册到cpu1
    { -2,               -2,   -2   }, //如果加入了该行, 那么只有该行之前的中断注册到对应核, 其他所有中断强制注册到CPU0
#endif

    { -1,     -1,   -1    },
};

#if defined CONFIG_NO_SDRAM_ENABLE
#define SEC_IN_CACHRAM  SEC_USED(.cache_ram)
#define SEC_IN_SRAM
#elif defined CONFIG_SFC_ENABLE
#define SEC_IN_CACHRAM  SEC_USED(.sram)
#define SEC_IN_SRAM  SEC_USED(.sram)
#else
#define SEC_IN_CACHRAM
#define SEC_IN_SRAM
#endif

/* 创建使用 os_task_create_static 或者task_create 接口的 静态任务堆栈 */
#ifdef CONFIG_NO_SDRAM_ENABLE
#define SYS_TIMER_STK_SIZE 800
#else
#define SYS_TIMER_STK_SIZE 3000
#endif
#define SYS_TIMER_Q_SIZE 128
static SEC_IN_SRAM u8 sys_timer_tcb_stk_q[sizeof(StaticTask_t) + SYS_TIMER_STK_SIZE * 4 + sizeof(struct task_queue) + SYS_TIMER_Q_SIZE] ALIGNE(4);

#define SYSTIMER_STK_SIZE 256
static SEC_IN_SRAM u8 systimer_tcb_stk_q[sizeof(StaticTask_t) + SYSTIMER_STK_SIZE * 4] ALIGNE(4);

#define SYS_EVENT_STK_SIZE 512
static SEC_IN_SRAM u8 sys_event_tcb_stk_q[sizeof(StaticTask_t) + SYS_EVENT_STK_SIZE * 4] ALIGNE(4);

#ifdef CONFIG_NO_SDRAM_ENABLE
#define APP_CORE_STK_SIZE 1000
#else
#define APP_CORE_STK_SIZE 2600
#endif
#define APP_CORE_Q_SIZE 1024
static u8 app_core_tcb_stk_q[sizeof(StaticTask_t) + APP_CORE_STK_SIZE * 4 + sizeof(struct task_queue) + APP_CORE_Q_SIZE] ALIGNE(4);

/* 创建使用  thread_fork 接口的 静态任务堆栈 */
#ifdef CONFIG_NO_SDRAM_ENABLE
#define WIFI_TASKLET_STK_SIZE 800
#else
#define WIFI_TASKLET_STK_SIZE 1400
#endif
static SEC_IN_SRAM u8 wifi_tasklet_tcb_stk_q[sizeof(struct thread_parm) + WIFI_TASKLET_STK_SIZE * 4] ALIGNE(4);

#define WIFI_CMDQ_STK_SIZE 300
static SEC_IN_CACHRAM u8 wifi_cmdq_tcb_stk_q[sizeof(struct thread_parm) + WIFI_CMDQ_STK_SIZE * 4] ALIGNE(4);

#define WIFI_MLME_STK_SIZE 800
static u8 wifi_mlme_tcb_stk_q[sizeof(struct thread_parm) + WIFI_MLME_STK_SIZE * 4] ALIGNE(4);

#define WIFI_RX_STK_SIZE 256
static SEC_IN_SRAM u8 wifi_rx_tcb_stk_q[sizeof(struct thread_parm) + WIFI_RX_STK_SIZE * 4] ALIGNE(4);

extern int app_usb_at_register_inter(void);

/* 任务列表 */
const struct task_info task_info_table[] = {
    {"thread_fork_kill",    25,      256,   0     },
    {"led_ui_server",       30,      256,   64    },
    {"app_core",            15,     APP_CORE_STK_SIZE,    APP_CORE_Q_SIZE,       app_core_tcb_stk_q },
    {"sys_event",           29,     SYS_EVENT_STK_SIZE,    0,                    sys_event_tcb_stk_q },
    {"systimer",            14,     SYSTIMER_STK_SIZE,     0,                    systimer_tcb_stk_q },
    {"sys_timer",            9,     SYS_TIMER_STK_SIZE,   SYS_TIMER_Q_SIZE,      sys_timer_tcb_stk_q },
    {"audio_server",        16,      512,   64    },
    {"audio_mix",           28,      512,   0     },
    {"audio_encoder",       25,      384,   64    },
    {"speex_encoder",       13,      512,   0     },
    {"mp3_encoder",         13,      768,   0     },
    {"opus_encoder",        13,     1536,   0     },
    {"vir_dev_task",        14,      256,   0     },
    {"amr_encoder",         13,     1024,   0     },
    {"cvsd_encoder",        13,      512,   0     },
    {"vad_encoder",         14,      768,   0     },
    {"aec_encoder",         13,     1024,   0     },
    {"dns_encoder",         13,      512,   0     },
    {"msbc_encoder",        13,      256,   0     },
    {"sbc_encoder",         13,      512,   0     },
    {"adpcm_encoder",       13,      512,   0     },
    {"echo_deal",           11,     1024,   32    },
    {"uac_sync",            20,      512,   0     },
    {"uac_play0",           26,      512,   32    },
    {"uac_play1",           26,      512,   32    },
    {"uac_record0",         26,      512,   0     },
    {"uac_record1",         26,      512,   0     },
#if (RCSP_MODE)
    {"rcsp",                4,       768,   128   },
    {"dev_mg",              3,       512,   512   },
#endif//RCSP_MODE
#if (TCFG_DEV_MANAGER_ENABLE)
    {"file_bs",              1,       768,   0    },
    {"ftran_back",           1,       512,   0    },
#endif
#if CPU_CORE_NUM > 1
    {"#C0usb_msd0",          1,      512,   128   },
#else
    {"usb_msd0",             1,      512,   128   },
#endif
    {"usb_msd1",             1,      512,   128   },
    {"uda_main",             2,     7000,   0     },

    {"update",              21,      512,   32    },
    {"dw_update",           21,      512,   32    },
    {"iperf_test",          15,     1024,   0     },
    //4G-LTE必要线程
#ifdef CONFIG_LTE_PHY_ENABLE
    {"#C0tcpip_thread",        16,      800,    0    },
#else
    {"tcpip_thread",        16,      800,    0    },
#endif

#ifdef CONFIG_WIFI_ENABLE
    //通过调节任务优先级平衡WIFI收发占据总CPU的比重
    {"tasklet",             15,     WIFI_TASKLET_STK_SIZE,   0,      wifi_tasklet_tcb_stk_q  },
    {"RtmpMlmeTask",        17,     WIFI_MLME_STK_SIZE,      0,      wifi_mlme_tcb_stk_q     },
    {"RtmpCmdQTask",        17,     WIFI_CMDQ_STK_SIZE,      0,      wifi_cmdq_tcb_stk_q     },
    {"wl_rx_irq_thread",    16,     WIFI_RX_STK_SIZE,        0,      wifi_rx_tcb_stk_q       },
#endif
    //4G-LTE必要线程
#ifdef CONFIG_LTE_PHY_ENABLE
    {"#C0lte_rx_task",         16,      1500,   256  },
#endif

#ifdef CONFIG_BT_ENABLE
#if CPU_CORE_NUM > 1
    {"#C0btencry",          14,      512,   128   },
#else
    {"btencry",             14,      512,   128   },
#endif
#if CPU_CORE_NUM > 1
    {"#C0btctrler",         19,      512,   384   },
#ifdef CONFIG_NO_SDRAM_ENABLE
    {"#C0btstack",          18,      800,  384   },
#else
    {"#C0btstack",          18,      1024,  384   },
#endif
#else
    {"btctrler",            19,      512,   384   },
    {"btstack",             18,      768,   384   },
#endif
#endif
    {"wl80_test",            1,     2048,   64    },

    {"video_server",        16,      768,   128   },
    {"vpkg_server",         16,      512,   128   },
    {"video0_rec0",         20,      256,   128   },
    {"video0_rec1",         20,      256,   128   },
    {"video2_rec0",         20,      512,   128   },
    {"video2_rec1",         20,      512,   128   },
    {"net_video_server",    16,      256,   64    },

    {"net_avi0",            18,      512,   0     },
    {"net_avi1",            18,      512,   0     },

    {"avi0",                11,      320,   64    },
    {"jpg_dec",             10,     1024,   32    },
    {"video_dec_server",    16,     1024,   256   },
    {"vunpkg_server",       16,     1024,   128   },

#ifdef CONFIG_UI_ENABLE
    {"ui",                  21,      768,   256   },
    {"lcd_task_0",          8,      1024,    32   },
    {"lcd_task_1",          8,      1024,    32   },
    {"te_task",             9,      1024,    32   },
#endif
    //讯飞aiui任务栈比加大，指定减小
    {"aiui_net_task",       11,     1600,   256   },

    {0, 0},
};

#if 0
char task_designated_cpu(const char *task_name)//返回0指定CPU0运行,返回1指定CPU1运行,返回-1 CPU01都可以运行
{
    if (!strcmp(task_name, "init")) {
        return -1;//操作系统第一个任务init任务必须安装在核0,运行完会自动删除
    }
    if (!strcmp(task_name, "thread_fork_kill")) {
        return -1;
    }

    //用户在此增加指定任务

    return -1;
}
#endif

#if IP_NAPT
int dns_set_network_detect_redirect(void)
{
    return 1;
}
#endif


/*
 * 默认的系统事件处理函数
 * 当所有活动的app的事件处理函数都返回false时此函数会被调用
 */
void app_default_event_handler(struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        break;
    case SYS_TOUCH_EVENT:
        break;
    case SYS_DEVICE_EVENT:
        break;
    case SYS_NET_EVENT:
        break;
    case SYS_BT_EVENT:
#if (RCSP_MODE)
        if (event->from == BT_EVENT_FROM_BLE_RCSP_UPDATE) {
            struct bt_event *e = (struct device_event *)event->payload;
            JL_rcsp_update_msg_deal(NULL, e->event, e->args);
        }
#endif
        break;
    default:
        ASSERT(0, "unknow event type: %s\n", __func__);
        break;
    }
}

//用户项目产品名称
char *app_user_product_name(void)
{
#ifdef PRODUCT_NAME
    return PRODUCT_NAME;
#endif
    QYAI_CFG_RETURN_STR(CFG_IDX_PRODUCT_NAME);
#ifdef CONFIG_KWS_ENGLISH
    return "OverseasTESTBOX";
#else
    return "TESTBOX"; //英文名称
#endif
}

//用户UUID
char *app_user_product_uuid(void)
{
#ifdef PRODUCT_UUID
    return PRODUCT_UUID;
#endif
    QYAI_CFG_RETURN_STR(CFG_IDX_PRODUCT_UUID);
    return NULL; //客户ID
}

//产品批次号
int app_user_product_batch(void)
{
#ifdef PRODUCT_BATCH
    return PRODUCT_BATCH; //批次
#endif
    QYAI_CFG_RETURN_INT(CFG_IDX_PRODUCT_BATCH);
    return 1; //批次
}

//产品批次的数量，单位K
int app_user_product_batch_num(void)
{
#ifdef PRODUCT_UUID
    return PRODUCT_NUM; //批次号的数量，单位K
#endif
    QYAI_CFG_RETURN_INT(CFG_IDX_PRODUCT_NUM);
    return 1; //批次号的数量，单位K
}

/*
 * 应用程序主函数
 */
void app_main()
{
    printf("------------- app main version: %s %s-------------\n", __DATE__, __TIME__);

    //校验，防止复制flash放在另外flash可以启动
    while (!flash_protect_check_match()) {
        os_time_dly(50);
    }

#if (defined PRODUCTION_TEST_ENABLE || defined PRODUCTION_ALL_TEST_ENABLE)
    if (production_io_is_enter() && system_reset_reason_get() == SYS_RST_VDDIO_PWR_ON) { //只有上电才能进行厂测
        product_test_task_init(2);
        return;
    }
#endif

#if (defined TCFG_VBAT_CHECK_EN && TCFG_VBAT_CHECK_EN == 1)

#if (!defined TCFG_SOFT_POWER_OFF_ALL_ELECTRY || TCFG_SOFT_POWER_OFF_ALL_ELECTRY == 0) //没有彻底断电才需要检查
    //带电池：正常软关机后重启
    if (
#if (defined TCFG_FIRST_POWER_OFF_EN)
        (system_reset_reason_get() == SYS_RST_VDDIO_PWR_ON) ||
#endif
        (user_is_soft_power_off_read() &&
         system_reset_reason_get() != SYS_RST_ALM_WKUP && //非闹钟
         system_reset_reason_get() != SYS_RST_PORT_WKUP && //非按键
         system_reset_reason_get() != SYS_RST_LONG_PRESS)) { //非长按
        sys_power_poweroff(0); //异常重启或者开启第一次插电关机的进入关机
    }
#endif

    int vbat_perc = vbat_check_percent();
    if (vbat_perc <= 20) { //电压过低
        os_time_dly(10);
        vbat_perc = vbat_check_percent();
        if (vbat_perc <= 20) { //电压过低
            os_time_dly(50);
            sys_power_poweroff(0); //电压过低强制关机
        }
    }
#endif

    /************************************************************
    * 应用层所有功能在下面才能开始初始化：
    * 1、非模组产测、
    * 2、非第一次上电关机、
    * 3、非电量低、
    *************************************************************/

    //QYAI配置项读取
    qyai_config_res_init(0, NULL, NULL);

    //关闭功放静音
    dac_mute_control(0, 0);

#ifdef CONFIG_MQTT_IOT_ENABLE
    extern void bt_ble_mac_set(char *mac);
    extern int mqtt_dev_mac_read(char *mac);
    char mac[6] = {0};
    if (!mqtt_dev_mac_read(mac)) {
        bt_ble_mac_set(mac);
    }
#endif

#ifdef CONFIG_LTE_PHY_ENABLE
    //注册AT接口列表到USB协议栈
    app_usb_at_register_inter();
#endif

    //闹钟唤醒
    if (system_reset_reason_get() == SYS_RST_ALM_WKUP) {
#ifdef CONFIG_LVGL_UI_ENABLE
        int lv_demo_switch_to_ring_page(void);
        sys_timeout_add(NULL, lv_demo_switch_to_ring_page, 1000);//需要延时执行，否则闹钟唤醒无法切换页面
#endif
        extern int music_play_alarm(int index);
        sys_timeout_add(user_read_alarm_index(), music_play_alarm, 500);
    }

#ifdef  USED_TM1629_SHOWN
    extern int tm_time_init(void);
    tm_time_init();
#endif
#ifdef  USED_WS2812B_SHOWN
    extern int ws2812_shown_init(void);
    ws2812_shown_init();
#endif

    //todo
    sys_net_mode_write(NET_MODE_MANUAL_SET);
    //sys_net_channel_write(NET_CH_SELECT_LTE);
#if IP_NAPT
    sys_net_channel_write(NET_CH_SELECT_LTE);
#endif
    int net_ch = sys_net_channel_read();
    int net_mode = sys_net_mode_read();
    if (net_mode < 0) {
        sys_net_mode_write(NET_MODE_MANUAL_SET);
        net_mode = sys_net_mode_read();
    }

    if (net_ch < 0) {
        sys_net_channel_write(NET_CH_SELECT_WIFI);
        net_ch = sys_net_channel_read();
    }

#if ((defined CONFIG_WIFI_ENABLE || defined CONFIG_EXT_WIFI_ENABLE) && defined CONFIG_LTE_PHY_ENABLE)
#if 0 //废弃这种方式
    net_ch = sys_net_channel_read();
    if (net_ch < 0 || (net_ch != 0 && net_ch != 1)) {
        sys_net_channel_write(0);
        net_ch = sys_net_channel_read();
    }
#else
    if (net_mode == NET_MODE_AUTO) {
        //AUTO模式，wifi网络优先
        sys_net_channel_write(NET_CH_SELECT_WIFI);
        net_ch = NET_CH_SELECT_WIFI;
        //todo: 加提示音或者LED指示灯
        printf("自动网络模式");
    } else {
        //todo: 加提示音或者LED指示灯
        printf("手动网络模式");
    }
#endif
#elif (defined CONFIG_LTE_PHY_ENABLE)
    net_ch = 1;
#endif


    if (net_ch == 0) {
#if (IP_NAPT==0)
#ifdef CONFIG_WIFI_ENABLE
#if (defined CONFIG_WIFI_ENABLE && defined CONFIG_LTE_PHY_ENABLE)
        extern u8 IPV4_ADDR_CONFLICT_DETECT;
        //WiFi开启静态ip冲突检测
        IPV4_ADDR_CONFLICT_DETECT = 1;
#endif
        if (net_mode == NET_MODE_AUTO) {
            lte_power_control(1);
        }
        wireless_net_init();
#endif

#ifdef CONFIG_EXT_WIFI_ENABLE
        ext_wireless_net_init();
#endif
#endif
    }

    printf("====net_mode:%d, ====net_ch:%d", net_mode, net_ch);
#ifdef CONFIG_LTE_PHY_ENABLE
    if (net_ch == 1) {
#if IP_NAPT
        //enter ap mode
        //NOTE：放在4G联网之前
        wifi_ap_mode_init();
#endif
        char mac[6];
        //第一次读取蓝牙mac，如没有mac则生成mac给后面流程使用
        bt_ble_get_mac(&mac);
#if (defined CONFIG_WIFI_ENABLE && defined CONFIG_LTE_PHY_ENABLE)
        extern u8 IPV4_ADDR_CONFLICT_DETECT;
        //4G关闭静态ip冲突检测
        //IPV4_ADDR_CONFLICT_DETECT = 0;
#endif
        //lte_net_init();
        lte_power_control(1);
        app_usb_hardware_check();
    } else {
        //printf("wifi mode, lte power off");
        //lte_power_control(0);
    }
#endif

#if TCFG_EQ_ENABLE && !defined EQ_CORE_V1
    extern void app_eq_init(void);
    app_eq_init();
#endif // TCFG_EQ_ENABLE

#if (defined TCFG_LED_STATUES_PORT && TCFG_LED_STATUES_PORT != -1)
    led_init();
#endif

#if defined CONFIG_BT_ENABLE && !defined CONFIG_WIFI_ENABLE
    extern void bt_ble_module_init(void);
    bt_ble_module_init();
#else
#if defined CONFIG_BT_ENABLE
    if (net_ch == 1) {
        extern void bt_ble_module_init(void);
        bt_ble_module_init();
    }
#endif
#endif
#ifdef CONFIG_GSENSOR_ENABLE
    sc7a20h_init();
#endif

#ifdef CONFIG_UI_ENABLE
#ifdef CONFIG_UI_PLAY_EMOJI
    play_face_emoji(AI_UART_CMD_PWR_ON);
#endif
#ifdef CONFIG_LVGL_UI_ENABLE
    lvgl_test_demo();
#endif
#endif

    audio_app_mode_switch(NULL);

    gpio_direction_input(TCFG_IOKEY_PORT);
    gpio_set_pull_up(TCFG_IOKEY_PORT, 1);
    gpio_set_pull_down(TCFG_IOKEY_PORT, 0);

    touch_adc_init();
    void userinit();
    userinit();
    puts("-------------app main end-------------\n");
}

static void green_led_toggle(void *priv)
{
    static u8 level = 1;

    level = !level;
    gpio_direction_output(IO_PORTB_00, level);
}

void userinit()
{
    //绿灯
    gpio_direction_output(IO_PORTB_00, 1);
    sys_timer_add_to_task("sys_timer", NULL, green_led_toggle, 500);
}
// late_initcall(userinit);