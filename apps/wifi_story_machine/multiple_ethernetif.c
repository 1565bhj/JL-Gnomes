#include "init.h"
#include "wifi/wifi_connect.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/ip.h"
#include "os/os_api.h"
#include "asm/gpio.h"
#include "lwip.h"
#include "app_config.h"
#include "system/timer.h"
#include "lte_module/lte_module.h"
#include "bt_profile_cfg.h"
#include "server/net_server.h"
#include "event/net_event.h"
#include "event/device_event.h"

#ifdef CONFIG_NET_ENABLE
static struct {
    u16 check_timer;
    //DHCP服务分配IP
    u8 lte_phy_dhcp_succ;
    //Ping通万维网
    u8 net_connect_succ;
    u32 lte_phy_timeout;
    u8 hwaddr[6];
    //网络检测线程
    u8 net_check_th_ready;
    //USB总线异常检测线程
    u8 bus_check_th_ready;

    u8 lte_init_flag;
} multiple_ethernetif_hdl;
#define __this  (&multiple_ethernetif_hdl)

//连接服务器超时(DHCP分配失败 or 无法连接外网)
#define TIME_OUT_SECOND (60)

static void *dev = NULL;
extern void wifi_and_network_on(void);
extern void wifi_and_network_off(void);
extern int app_usb_get_driver_status(void);
extern int net_connect_check(void);
extern int qyai_net_interface_set(char index);

int wifi_connect_net_success(void)
{
    return (wifi_get_sta_connect_state() == WIFI_STA_NETWORK_STACK_DHCP_SUCC);
}

int lte_connect_net_success(void)
{
    return (__this->net_connect_succ == NET_CONNECT_SERVER_OK);
}

int sys_connect_net_success(void)
{
#ifdef CONFIG_NO_SDRAM_ENABLE
    return (wifi_get_sta_connect_state() == WIFI_STA_NETWORK_STACK_DHCP_SUCC);
#else
    return __this->net_connect_succ == NET_CONNECT_SERVER_OK;
#endif
}

#if IP_NAPT_EXT
static void check_dhcps_client_ipaddr_timer(void *p)
{
    struct ip4_addr src_ipaddr, dest_ipaddr;
    char gw_addr[32];

    extern int dhcps_get_ipaddr(u8 hwaddr[6], struct ip4_addr * ipaddr);
    if (0 == dhcps_get_ipaddr(__this->hwaddr, &dest_ipaddr)) {
        sys_timer_del(__this->check_timer);
        __this->check_timer = 0;
        get_gateway(LTE_NETIF, gw_addr);
        inet_aton(gw_addr, &src_ipaddr);
        extern void ip_napt_ext_set_forward_addr(u32_t src_addr, u32_t dest_addr);
        ip_napt_ext_set_forward_addr(src_ipaddr.addr, dest_ipaddr.addr);
    }
}

void check_dhcps_client_ipaddr(u8 *hwaddr)
{
    memcpy(__this->hwaddr, hwaddr, 6);
    if (!__this->check_timer) {
        __this->check_timer = sys_timer_add_to_task("sys_timer", NULL, check_dhcps_client_ipaddr_timer, 500);
    }
}
#endif

//只用一个GPIO控制供电，去掉硬件复位引脚
int lte_power_control(void *on)
{
    int pwr_on = (int)on;
#ifdef LTE_USB_SWITCH_PORT //USB模拟开关器
    gpio_direction_output(LTE_USB_SWITCH_PORT, pwr_on);
    gpio_set_pull_down(LTE_USB_SWITCH_PORT, 0);
    gpio_set_pull_up(LTE_USB_SWITCH_PORT, 0);
#endif

#ifdef LTE_POWER_ONOFF_PORT  //USB模组供电IO
    printf("################lte_power_control:%d\r\n", pwr_on);
    gpio_direction_output(LTE_POWER_ONOFF_PORT, pwr_on);
    gpio_set_pull_down(LTE_POWER_ONOFF_PORT, 0);
    gpio_set_pull_up(LTE_POWER_ONOFF_PORT, 0);
#endif

#ifdef CONFIG_LTE_PHY_ENABLE
    if (pwr_on == 0) {
        usb_net_set_module_reset_status(1);
    } else {
        usb_net_set_module_reset_status(0);
    }
#endif
    return 0;
}

static void lte_net_params_clear(void)
{
    __this->net_check_th_ready = 0;
    __this->bus_check_th_ready = 0;
    __this->lte_phy_timeout = 0;
    __this->lte_phy_dhcp_succ = 0;
    __this->net_connect_succ = 0;
}

static void net_reset_lte_module(void)
{
    printf("=============net_reset_lte_module=============");
    //模组断电，协议栈有执行unmount
#if 0
    //unmount
    struct device_event event = {0};
    event.arg = (void *)"otg";
    int usb_id = app_usb_get_controller_id();
    event.event = DEVICE_EVENT_OUT;
    event.value = usb_id ? (int)"h:1:0" : (int)"h:0:0";
    device_event_notify(DEVICE_EVENT_FROM_OTG, &event);
#endif
    //reset module power
    lte_power_control(0);
    os_time_dly(200);
    lte_power_control(1);
    __this->lte_phy_timeout = timer_get_sec();
}

//重置超时时间[eg: 连不上网，切卡]
void net_connected_check_reset_timeout(void)
{
    if (__this->net_connect_succ != NET_CONNECT_SERVER_OK) {
        __this->lte_phy_timeout = timer_get_sec();
    }
}

static void net_connected_check_thread(void)
{
    int net_ch = 0;
    while (__this->net_check_th_ready) {
#if (defined CONFIG_WIFI_ENABLE && defined CONFIG_LTE_PHY_ENABLE)
        net_ch = sys_net_channel_read();
#endif

#if (defined CONFIG_LTE_PHY_ENABLE)
        if (net_ch) {
            //单次：20s超时
#define SINGAL_TIMEOUT_CNT (20)
            int not_ready_cnt = 0;
            //设置最大超时次数
#define MAX_NOT_READY_CNT (2)
            u8 max_not_ready_cnt = 0;

            //检查USB驱动: 第一次上电，后续的热拔插
            while (!app_usb_get_driver_status()) {
                //如果是USB总线硬件异常
                if (usb_net_get_module_reset_status() == 2) {
                    printf("error cause!, line:%d", __LINE__);
                    usb_net_set_module_reset_status(0);
                    net_reset_lte_module();
                }

                //硬件DPDM信号没有出错，仅连接超时(note: 模组没通电)：复位4G模组
                if (usb_net_get_module_reset_status() == 0) {
                    if (not_ready_cnt % 10 == 0 && not_ready_cnt != 0) {
                        printf("usb not ready!!!:%d\r\n", not_ready_cnt);
                    }

                    if (not_ready_cnt == SINGAL_TIMEOUT_CNT) {
                        not_ready_cnt = 0;
                        printf("usb wait timeout, reset module!!!\r\n");
                        max_not_ready_cnt++;
                        //总的超时重启时间：MAX_NOT_READY_CNT * SINGAL_TIMEOUT_CNT
                        if (max_not_ready_cnt >= MAX_NOT_READY_CNT) { //2x20=40s
                            printf("Max retry count reached, switch to WiFi\r\n");
                            max_not_ready_cnt = 0;
                            sys_net_channel_write(NET_CH_SELECT_WIFI);
                            system_reset();
                            break;
                        } else {
                            net_reset_lte_module();
                        }
                    } else {
                        not_ready_cnt++;
                    }
                }
                os_time_dly(100);
                __this->lte_phy_timeout = timer_get_sec();
            }
        }
#endif
        struct net_event net = {0};
        if (__this->lte_phy_dhcp_succ == NET_DHCP_SUCCESS && net_connect_check()) {
            __this->lte_phy_timeout = 0;
            __this->net_connect_succ = NET_CONNECT_SERVER_OK;
            net.arg = "net";
            net.event = NET_EVENT_CONNECTED;
            net_event_notify(NET_EVENT_FROM_USER, &net);
#ifdef CONFIG_MQTT_IOT_ENABLE
            void mqtt_example(void);
            mqtt_example();
#endif
            printf("net_connected_check ok, __this->net_connect_succ:%d\n", __this->net_connect_succ);
            break;
        } else if (__this->net_connect_succ == NET_DISCONNECT) {//断开则退出检测
            __this->net_connect_succ = NET_DHCP_INIT;
            __this->lte_phy_dhcp_succ = NET_DHCP_INIT;
            break;
        } else {
            //TIME_OUT_SECOND秒无法连上服务器则发配网失败事件[DHCP分配失败 or 无法连接外网]
            if (timer_get_sec() - __this->lte_phy_timeout >= TIME_OUT_SECOND) {
                printf("net_connected_check err\n");
                __this->lte_phy_timeout = timer_get_sec();
                net.arg = "net";
                net.event = NET_EVENT_SMP_CFG_TIMEOUT;
                net_event_notify(NET_EVENT_FROM_USER, &net);

#if (defined CONFIG_LTE_PHY_ENABLE)
                //复位4G模组
                if (net_ch == 1) {
                    if (__this->lte_phy_dhcp_succ != NET_DHCP_SUCCESS) {
                        printf("DHCP Server fail!!!");
                        //切到WIFI网络进行尝试
                        sys_net_channel_write(NET_CH_SELECT_WIFI);
                        system_reset();
                    } else {
                        printf("4G-Module connect to server timeout(>60s)!!!");
                        //net_reset_lte_module();
                    }
                }
#endif
            }
#if (defined CONFIG_LTE_PHY_ENABLE)
            //规避USB总线硬件异常
            if (usb_net_get_module_reset_status() == 2) {
                printf("error cause!, line:%d", __LINE__);
                usb_net_set_module_reset_status(0);
                net_reset_lte_module();
            }
#endif
        }
        os_time_dly(10);
    }
    __this->net_check_th_ready = 0;
}

void net_dhcp_status_set(int status)
{
    if (!__this->net_check_th_ready && status == NET_DISCONNECT) {
        status = NET_DHCP_INIT;
    }
    __this->lte_phy_dhcp_succ = status;
    __this->net_connect_succ = status;
    //非低内存版本才使用连接服务器检测
#ifndef CONFIG_NO_SDRAM_ENABLE
    if (__this->net_connect_succ == NET_DHCP_SUCCESS) {
        if (__this->net_check_th_ready) {
            return ;
        }
        __this->net_check_th_ready = 1;
        if (thread_fork("net_check", 10, 2048, 0, NULL, net_connected_check_thread, NULL) != OS_NO_ERR) {
            __this->net_check_th_ready = 0;
        }
    }
#endif
}

int lte_lwip_event_cb(void *lwip_ctx, enum LWIP_EVENT event)
{
    char ip_addr[32];
    char gw_addr[32];

    printf("lte_lwip_event_cb:%d", event);
    switch (event) {
    case LWIP_LTE_DHCP_BOUND_TIMEOUT:
        printf("lwip lte dhcp bound timeout!!!");
        __this->lte_phy_dhcp_succ = 0;
        break;

    case LWIP_LTE_DHCP_BOUND_SUCC:
        Get_IPAddress(LTE_NETIF, ip_addr);
        get_gateway(LTE_NETIF, gw_addr);
        printf("***LTE DHCP SUCC, IP:[%s] , GW:[%s], Line:%d", ip_addr, gw_addr, __LINE__);
        //此处4G网络已连通
        qyai_net_interface_set(1);   //设置4G网卡为默认模块
#if IP_NAPT_EXT
        wifi_and_network_on();
#endif
#if IP_NAPT
        struct ip4_addr ipaddr;
        struct lan_setting *lan_setting_info = net_get_lan_info(WIFI_NETIF);
        IP4_ADDR(&ipaddr, lan_setting_info->WIRELESS_IP_ADDR0, lan_setting_info->WIRELESS_IP_ADDR1, lan_setting_info->WIRELESS_IP_ADDR2, lan_setting_info->WIRELESS_IP_ADDR3);
        extern void ip_napt_enable(u32_t addr, int enable);
        ip_napt_enable(ipaddr.addr, 1);
#endif
        //wifi模式下，记录基准时间
        int net_ch = sys_net_channel_read();
        if (!net_ch) {
            __this->lte_phy_timeout = timer_get_sec();
        }
        __this->lte_phy_dhcp_succ = NET_DHCP_SUCCESS;
        break;

    default:
        break;
    }

    return 0;
}

int lte_net_restart(void)
{
    dev_ioctl(dev, LTE_NETWORK_START, 0);
    return 0;
}

static int lte_state_cb(void *priv, int on)
{
    //printf("lte_state_cb__LINE:%d-%s\r\n", __LINE__,  (const char *)priv);
    struct net_event net = {0};
    if (priv) { /*&& !strncmp((const char *)priv, "at_port1", strlen("at_port1"))) {*/
        if (on) {
            printf("lte on\n");
            if (wifi_connect_net_success()) {
                __this->lte_phy_dhcp_succ = 0;
                __this->net_connect_succ = 0;
                dev_ioctl(dev, LTE_NETWORK_STOP, 0);
                printf("=====LTE_NETWORK_STOP=====");
            } else {
                dev_ioctl(dev, LTE_NETWORK_START, 0);
                printf("=====LTE_NETWORK_START=====");
            }
        } else {
            printf("lte off\n");
            __this->lte_phy_dhcp_succ = 0;
            __this->net_connect_succ = 0;
            dev_ioctl(dev, LTE_NETWORK_STOP, 0);
#if IP_NAPT_EXT
            wifi_and_network_off();
#endif
//            net.arg = "net";
//            net.event = NET_EVENT_DISCONNECTED;
//            net_event_notify(NET_EVENT_FROM_USER, &net);
        }
    }

    return 0;
}

//网络检测线程
static int lte_create_net_conn_check_thread(void)
{
    if (__this->net_check_th_ready) {
        printf("lte_create_net_conn_check_thread already!!!");
        return 0;
    }

    __this->net_check_th_ready = 1;
    if (thread_fork("net_conn_check", 10, 2048, 0, NULL, net_connected_check_thread, NULL) != OS_NO_ERR) {
        puts("net_connected_check_thread create err\n");
        __this->net_check_th_ready = 0;
        return -1;
    }
    return 0;
}

#if 1
//没有IN/OUT事件，总线一直出错的规避方案
//notes: 如果有IN/OUT事件，设备栈已经umount过一次
#define USB_BUS_ERR_CAUSE_CNT (200)
extern int usb_net_get_err_cnt(void);
extern int usb_net_get_at_err_cnt(void);

static void usb_bus_check_thread(void)
{
    while (__this->bus_check_th_ready) {
        if (usb_net_get_err_cnt() >= USB_BUS_ERR_CAUSE_CNT) {
            printf("data transfer err cause!, cnt:%d", usb_net_get_err_cnt());
            //system_reset();
            usb_net_set_err_cnt(0);
            usb_net_set_at_err_cnt(0);
            net_reset_lte_module();
        }

        if (usb_net_get_at_err_cnt() >= USB_BUS_ERR_CAUSE_CNT) {
            printf("at transfer err cause!, cnt:%d", usb_net_get_at_err_cnt());
            //system_reset();
            usb_net_set_err_cnt(0);
            usb_net_set_at_err_cnt(0);
            net_reset_lte_module();
        }
        os_time_dly(50);
    }
}

//USB总线异常检测线程
static int lte_create_usb_bus_check_thread(void)
{
    if (__this->bus_check_th_ready) {
        printf("lte_create_usb_bus_check_thread already!!!");
        return 0;
    }

    __this->bus_check_th_ready = 1;
    if (thread_fork("usb_bus_check", 9, 1024, 0, NULL, usb_bus_check_thread, NULL) != OS_NO_ERR) {
        puts("usb_bus_check_thread create err\n");
        __this->bus_check_th_ready = 0;
        return -1;
    }
    return 0;
}
#endif

int lte_net_init(void)
{
    printf("****************lte_net_init************");
    if (__this->lte_init_flag) {
        printf("lte net already init!!!");
        return -1;
    }
    __this->lte_init_flag = 1;

#ifdef LTE_POWER_ONOFF_PORT
    //1.turn on power
    //lte_power_control(1);
    //sys_timeout_add_to_task("sys_timer", 1, lte_power_control, 1200);
#endif

    //2.Enable LTE module
    dev = dev_open("lte", NULL);
    dev_ioctl(dev, LTE_DEV_SET_CB, (u32)lte_state_cb);

    qyai_net_interface_set(1);
#if 0
    if (thread_fork("net_check", 10, 2048, 0, NULL, net_connected_check_thread, NULL) != OS_NO_ERR) {
        puts("net_connected_check_thread create err\n");
    }
#endif

    lte_net_params_clear();

    //3.send msg to front app
    struct net_event net = {0};
    net.arg = "net";
    net.event = NET_NTP_GET_TIME_SUCC + 32;
    net_event_notify(NET_EVENT_FROM_USER, &net);

    //4.记录超时基准时间(首次)
    __this->lte_phy_timeout = timer_get_sec();

    //创建两个检测线程
    lte_create_net_conn_check_thread();
    lte_create_usb_bus_check_thread();

    return 0;
}
//late_initcall(lte_net_init);

int lte_net_close(void)
{
    if (!__this->lte_init_flag) {
        printf("lte net already deinit!!!");
        return -1;
    }
    __this->lte_init_flag = 0;

    if (dev) {
        dev_close(dev);
    }

    printf("lte_net_close: %d", __this->net_connect_succ);
//    if (__this->net_connect_succ == NET_CONNECT_SERVER_OK) {
    //send msg: 网络已断开
    struct net_event net = {0};
    net.arg = "net";
    net.event = NET_EVENT_DISCONNECTED;
    net_event_notify(NET_EVENT_FROM_USER, &net);
//    }

    lte_net_params_clear();
    //fix me
    //net_reset_lte_module();
    return 0;
}

#if defined CONFIG_BT_ENABLE && USER_SUPPORT_PROFILE_PAN
int bt_lwip_event_cb(void *lwip_ctx, enum LWIP_EVENT event)
{
    char ip_addr[32];
    char gw_addr[32];

    switch (event) {
    case LWIP_BT_DHCP_BOUND_TIMEOUT:
        break;

    case LWIP_BT_DHCP_BOUND_SUCC:
        Get_IPAddress(BT_NETIF, ip_addr);
        get_gateway(BT_NETIF, gw_addr);
        printf("BT DHCP SUCC, IP:[%s] , GW:[%s]", ip_addr, gw_addr);
        lwip_set_default_netif(BT_NETIF);   //设置蓝牙网卡为默认模块
#if IP_NAPT_EXT
        wifi_and_network_on();
#endif
#if IP_NAPT
        struct ip4_addr ipaddr;
        struct lan_setting *lan_setting_info = net_get_lan_info(WIFI_NETIF);
        IP4_ADDR(&ipaddr, lan_setting_info->WIRELESS_IP_ADDR0, lan_setting_info->WIRELESS_IP_ADDR1, lan_setting_info->WIRELESS_IP_ADDR2, lan_setting_info->WIRELESS_IP_ADDR3);
        extern void ip_napt_enable(u32_t addr, int enable);
        ip_napt_enable(ipaddr.addr, 1);
#endif
        break;

    default:
        break;
    }

    return 0;
}
#endif
#endif

