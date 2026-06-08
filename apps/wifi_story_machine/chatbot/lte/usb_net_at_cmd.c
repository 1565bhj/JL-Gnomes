#include "usb_host.h"
#include "usbnet.h"
#include "event/device_event.h"
#include "event/net_event.h"
#include "os/pthread.h"
#include "app_config.h"
#include "qyai_config.h"
#include "event/key_event.h"

#ifdef CONFIG_LTE_PHY_ENABLE
static char **LTE_MUDULE_AT_CMD = NULL;
static unsigned char LTE_MUDULE_AT_VENDOR = 0;

//中移4G-ML307R/C-AT指令
static const char *ML307R_AT_TX_CMD[] = {
    //注意：前4个命令顺序固定
    "AT\r\n",               //0:检查模块运行是否正常，正常则返回"OK"
    "AT+CPIN?\r\n",         //1:检查SIM卡问题，成功则返回: "+CPIN: READY", "OK"
    "AT+CGATT?\r\n",        //2:检查网络问题，联网则返回: "+CGATT: 1", "OK"
    "AT+MREBOOT=0\r\n",     //3:模块复位AT指令
    "AT+MCCID\r\n",         //4:读取ICCID

    //组合：mode,0 + auto,1 ;设置过一次就可以(设置后重启生效)
    "AT+MDIALUPCFG=mode\r\n",//读当前模式
    "AT+MDIALUPCFG=mode,0\r\n",//不是"mode,0"则设置mode=0->配置模组自动拨号(配置模组自动拨号后，模组重启生效)
    "AT+MDIALUPCFG=auto\r\n",//读当前auto
    "AT+MDIALUPCFG=auto,1\r\n",//不是"auto,1"则设置auto=1(配置模组自动拨号后，模组重启生效)
    "NULL",
};

//合宙4G-AIR780E模组-AT指令
static const char *AIR780E_AT_TX_CMD[] = {
    //注意：前4个命令顺序固定
    "AT\r\n",               //0:检查模块运行是否正常，正常则返回"OK"
    "AT+CPIN?\r\n",         //1:检查SIM卡问题，成功则返回: "+CPIN: READY", "OK"
    "AT+CGATT?\r\n",        //2:检查网络问题，联网则返回: "+CGATT: 1", "OK"
    "AT+RESET\r\n",         //3:模块复位AT指令
    "AT+ICCID\r\n",         //4:读取ICCID
    "NULL",
};

//域格4G-AT指令(GPS)
static const char *YUGEYM310_AT_TX_CMD[] = {
    //注意：前4个命令顺序固定
    "AT\r\n",               //0:检查模块运行是否正常，正常则返回"OK"
    "AT+CPIN?\r\n",         //1:检查SIM卡问题，成功则返回: "+CPIN: READY", "OK"
    "AT+CGATT?\r\n",        //2:检查网络问题，联网则返回: "+CGATT: 1", "OK"
    "AT+CFUN=1,1\r\n",      //3:模块复位AT指令
    "AT+ICCID\r\n",         //4:读取ICCID

    "AT+CGPS=1\r\n",        //启动GPS
    "AT+CGPS=0\r\n",        //停止GPS
    "AT+CGPSINFO\r\n",      //返回格式化后的定位数据
    "AT+CGPSPOS\r\n",       //直接输出NMEA原始数据
    "NULL",
};

//骐俊ML120H-AT指令
static const char *ML120H_AT_TX_CMD[] = {
    //注意：前4个命令顺序固定
    "AT\r\n",               //0:检查模块运行是否正常，正常则返回"OK"
    "AT+CPIN?\r\n",         //1:检查SIM卡问题，成功则返回: "+CPIN: READY", "OK"
    "AT+CGATT?\r\n",        //2:检查网络问题，联网则返回: "+CGATT: 1", "OK"
    "AT+RESET\r\n",         //3:模块复位AT指令
    "AT+QCCID\r\n",         //4:读取ICCID

    "AT+NCONFIG=AUTOCONNECT,NDIS\r\n",  //配置模组自动拨号(配置模组自动拨号后，模组重启生效)
    "AT+NCONFIG=AUTOCONNECT,APP\r\n",   //取消模组自动拨号(手动拨号模式下)
    "AT+CFUN=0\r\n",    //MCU重启前发下AT+CFUN=0，然后给模组掉电
    "AT+CGDCONT=1,\"IPV4V6\"\r\n", //手动拨号，连接网络
    "AT+NDISDUP=1,1\r\n", //手动拨号，连接网络
    "AT+NDISDUP=1,0\r\n", //手动拨号，断开网络
    "NULL",
};

//定义在应用层的可用AT接口信息，所有4G模块全加在这里，一个固件可以全部使用
static struct wireless_device_serial_set_AT_CMD app_AT_CMD_dev[] ALIGNED(4) = {
    //合宙：接口两个接口均可用（一个AT，一个拨号？MCU获取到的描述符信息和PC不一样）
    {
        //AC79
        .idVendor = 0x19D1,
        .idProduct = 0x0001,
        .InterfaceNumber_at = 7, //该接口下的可用端点是0x04/0x88
        .prive_vendor = VENDOR_AIR780E,
        .AT_CMD = AIR780E_AT_TX_CMD,
    }, { //WINDOWS
        .idVendor = 0x0417,
        .idProduct = 0x4a88,
        .InterfaceNumber_at = 3, //该接口下的可用端点是0x02/0x84
        .prive_vendor = VENDOR_AIR780E,
        .AT_CMD = AIR780E_AT_TX_CMD,
    },

    //中移ML307R，接口下的可用端点是0x81/0x0a
    {
        .idVendor = 0x2ECC,
        .idProduct = 0x3012,
        .InterfaceNumber_at = 2,
        .prive_vendor = VENDOR_ML307R,
        .AT_CMD = ML307R_AT_TX_CMD,
    },

    //域格YM310，接口下的可用端点是0x86/0x0f
    {
        .idVendor = 0x1286,
        .idProduct = 0x4E3C,
        .InterfaceNumber_at = 3,
        .prive_vendor = VENDOR_YUGEYM310,
        .AT_CMD = YUGEYM310_AT_TX_CMD,
    },

    //骐俊ML120H，接口下的可用端点是0x86/0x03
    {
        .idVendor = 0x3361,
        .idProduct = 0x7B6E,
        .InterfaceNumber_at = 5,
        .prive_vendor = VENDOR_ML120H,
        .AT_CMD = ML120H_AT_TX_CMD,
    },
};

//Note: 不同AT指令的响应时间，有可能超时时间不够，会导致代码逻辑处理出错
//eg: 切卡耗时就比较长
#define SEM_WAIT_TIME_OUT (300)
#define SIM_CHECK_MAX   (2)

//长度定义
#define AT_RX_BUF_SIZE     512
//#define MAX_ICCID_LEN      20
//#define WIFI_MAC_LEN       12
#define IMEI_LEN           15

static u8 usb_driver_ready = 0;
static u8 usb_id = 0;
static u8 card_idx = 2;
static unsigned char iccid[33] = {0};

//同步USB传输
static OS_SEM at_cmd_sem;
//同步Flash读写(存储某些id值)
static OS_SEM io_opt_sem;

static unsigned int check_sim_card_err = 0;
static unsigned int check_net_err = 0;

//按键事件线程同步
static OS_SEM key_event_sem;
static u8 key_press_flag = 0;

//上锁，避免资源竞争
static pthread_mutex_t mutex_start;
static pthread_mutex_t mutex_end;

typedef enum {
    ML_STATE_CHECK_IDEL = 0,//空闲状态
    ML_STATE_CHECK_SIM_CARD,//检查sim卡
    ML_STATE_READ_ICCID,    //读取ICCID
    ML_STATE_CHECK_NET,     //检查是否连接网络
    ML_STATE_CHECK_MODE,    //检查mode值
    ML_STATE_CHECK_AUTO,    //检查auto值
    ML_STATE_CHECK_WAIT_OK,
    ML_STATE_CHECK_SET_MODE,    //设置mode,0
    ML_STATE_CHECK_SET_AUTO,    //设置auto,1
    ML_STATE_CHECK_COMPLETE,    //完成状态
    ML_STATE_CHECK_ERROR        //错误状态
} ml_state_machine;

#ifdef CONFIG_LTE_PHY_ENABLE
//SIM卡状态
enum sim_card_status {
    SIM_CARD_OK = 0,            //插入+激活
    SIM_CARD_NOT_INSERT = 1,    //SIM卡未插入或MCU检测不到SIM卡的存在
    SIM_CARD_NOT_ACTIVATE = 2,  //已经插入但未激活
};
u8 sim_status = SIM_CARD_OK;
#endif

static ml_state_machine ml_state_code = ML_STATE_CHECK_IDEL;

void *qyai_usb_h_device_serial_at_cmd_str_get(void);
int qyai_usb_h_device_serial_at_id_prive_vendor_get(void);
extern int usb_host_reset_ep(u8 usb_id);
extern int lte_net_auto_rndis_up(u8 opt, u8 * auto_rndis);

int app_usb_get_controller_id(void)
{
    return usb_id;
}

int app_usb_get_driver_status(void)
{
    return usb_driver_ready;
}

//Note:这里相当于是中断下文，不要做耗时或阻塞的动作，尽快将数据拷走
static void app_usb_at_port_rx_cb(u8 *buf, u32 len)
{
    static char tmp_buf[AT_RX_BUF_SIZE];

    //拷贝数据
    len = (len >= sizeof(tmp_buf)) ? sizeof(tmp_buf) -1 : len;
    memset(tmp_buf, 0, sizeof(tmp_buf));
    memcpy(tmp_buf, buf, len);
    tmp_buf[len] = '\0';

    printf("************************app_usb_at_port_rx_cb******************************************");
    printf("rx len=%d, %s", len, tmp_buf);

    static int mode_value = 0;
    static int auto_value = 0;
    char *str = (char *)buf;
    //最先检查是不是等OK
    if (ml_state_code == ML_STATE_CHECK_WAIT_OK) {
        char *ok_str = strstr(str, "OK");
        if (ok_str != NULL) {
            if (mode_value == 1) { //重新设置为0
                ml_state_code = ML_STATE_CHECK_SET_MODE;
                goto done;
            } else if (auto_value == 0) { //重新设置为1
                ml_state_code = ML_STATE_CHECK_SET_AUTO;
                goto done;
            } else {
                ml_state_code = ML_STATE_CHECK_COMPLETE;
                goto done;
            }
        }
    } else if (ml_state_code == ML_STATE_CHECK_SIM_CARD) {
        if (strstr(buf, "+CPIN: READY") || strstr(buf, "+CPIN:READY")) {
            if (LTE_MUDULE_AT_VENDOR == VENDOR_AIR780E) {
                ml_state_code = ML_STATE_CHECK_COMPLETE;
                goto done;
            } else if (LTE_MUDULE_AT_VENDOR == VENDOR_ML307R) { //wait "OK"
                ml_state_code = ML_STATE_CHECK_WAIT_OK;
                return;
            }
            if (strstr(buf, "OK")) {
                ml_state_code = ML_STATE_CHECK_COMPLETE;
                goto done;
            }
        } else {
            return;
        }
    } else if (ml_state_code == ML_STATE_READ_ICCID) {
        if (strstr(buf, "CCID:")) {
            char *data = strstr(buf, "CCID:");
            data += strlen("CCID:");
            int i = 0;
            int max = len - ((int)data - (int)buf);
            while (*data && !isxdigit(*data)) {
                *data++;
            }
            while (*data && isxdigit(*data) && i < sizeof(iccid) && i < max) {
                iccid[i++] = *data++;
            }
            if (strstr(buf, "OK")) {
                ml_state_code = ML_STATE_CHECK_COMPLETE;
                goto done;
            } else {
                ml_state_code = ML_STATE_CHECK_WAIT_OK;
            }
            return;
        } else {
            return;
        }
    } else if (ml_state_code == ML_STATE_CHECK_NET) {
        if (strstr(buf, "+CGATT: 1") || strstr(buf, "+CGATT:1")) {
            if (strstr(buf, "OK")) {
                ml_state_code = ML_STATE_CHECK_COMPLETE;
                goto done;
            }
            ml_state_code = ML_STATE_CHECK_WAIT_OK;
            return;
        } else {
            return;
        }
    }

    //是否含有子串"+MDIALUPCFG"
    char *prefix = strstr(str, "+MDIALUPCFG:");
    if (prefix == NULL) {
        goto done;
    }

    if (ml_state_code == ML_STATE_CHECK_MODE || ml_state_code == ML_STATE_CHECK_SET_MODE) {
        //是否含有子串"mode"
        char *mode_ptr = strstr(prefix, "mode");
        if (mode_ptr == NULL) {
            goto done;
        }

        //跳过特殊字符
        char *num_ptr = mode_ptr + strlen("mode");
        while (*num_ptr && !isdigit(*num_ptr) && *num_ptr != '-') {
            num_ptr++;
        }

        //提取数字
        if (*num_ptr) {
            mode_value = atoi(num_ptr);
            printf("mode: %d\n", mode_value);
            //进入下一阶段,等OK
            ml_state_code = ML_STATE_CHECK_WAIT_OK;
            //不释放信号量
            return;
        }
    } else if (ml_state_code == ML_STATE_CHECK_AUTO || ml_state_code == ML_STATE_CHECK_SET_AUTO) {
        //是否含有子串"auto"
        char *mode_ptr = strstr(prefix, "auto");
        if (mode_ptr == NULL) {
            goto done;
        }

        //跳过特殊字符
        char *num_ptr = mode_ptr + strlen("auto");
        while (*num_ptr && !isdigit(*num_ptr) && *num_ptr != '-') {
            num_ptr++;
        }

        //提取数字
        if (*num_ptr) {
            auto_value = atoi(num_ptr);
            printf("auto: %d\n", auto_value);
            //进入下一阶段,等OK
            ml_state_code = ML_STATE_CHECK_WAIT_OK;
            //不释放信号量
            return;
        }
    }
done:
    os_sem_post(&at_cmd_sem);
}

//AT模块重启指令
int app_usb_at_cmd_send_reset(void)
{
    int res = 0;
    LTE_MUDULE_AT_CMD = qyai_usb_h_device_serial_at_cmd_str_get();
    if (!usb_driver_ready) {
        printf("usb not ready!!!");
        return -2;
    }
    //不允许4G网络通信，只允许AT指令通信
    LTE_MUDULE_AT_CMD = qyai_usb_h_device_serial_at_cmd_str_get();
    if (!LTE_MUDULE_AT_CMD || !strcmp(LTE_MUDULE_AT_CMD[3], "NULL")) {
        printf("warning no CMD\n");
        return -1;
    }

    usb_net_set_module_reset_status(1);
    os_sem_set(&at_cmd_sem, 0);
    //获取对应4G设备的对应AT指令地址
    //CATA4全网通模组的复位AT指令:前4个固定顺序
    res = usbnet_host_at_data_send(usb_id, LTE_MUDULE_AT_CMD[3], strlen(LTE_MUDULE_AT_CMD[3]));
    if (res < 0) {
        return res;
    }
    if (OS_TIMEOUT == os_sem_pend(&at_cmd_sem, SEM_WAIT_TIME_OUT)) {
        printf("WAIT AT CMD Time OUT, line:%d!!!", __LINE__);
        res = -1;
    }

    //复位端点
    usb_host_reset_ep(usb_id);
    lte_net_close();
    return res;
}

//4G模组设置自动拨号上网(设置一次即可)
int app_usb_net_auto_dialup(u8 on_off)
{
    int res = 0;
    int reboot_flag = 0;
    if (!usb_driver_ready) {
        printf("usb not ready!!!");
        return -2;
    }

    //获取对应4G设备的对应AT指令地址
    LTE_MUDULE_AT_CMD = qyai_usb_h_device_serial_at_cmd_str_get();
    if (!LTE_MUDULE_AT_CMD) {
        printf("warning no AT CMD\n");
        return -1;
    }
    LTE_MUDULE_AT_VENDOR = qyai_usb_h_device_serial_at_id_prive_vendor_get();

#if (defined PRODUCTION_TEST_ENABLE || defined PRODUCTION_ALL_TEST_ENABLE)
    u8 auto_rndis = 0;
    lte_net_auto_rndis_up(1, &auto_rndis);//从flash读取自动上网设置，已设置则不需要设置
    if (auto_rndis) {
        return 0;
    }
#endif

    os_sem_set(&at_cmd_sem, 0);
    if (LTE_MUDULE_AT_VENDOR == VENDOR_ML307R) {//中移：ML307R
        ml_state_code = ML_STATE_CHECK_MODE;
        res = usbnet_host_at_data_send(usb_id, LTE_MUDULE_AT_CMD[5], strlen(LTE_MUDULE_AT_CMD[5]));
        if (res < 0) {
            return res;
        }
        if (OS_TIMEOUT == os_sem_pend(&at_cmd_sem, SEM_WAIT_TIME_OUT)) {
            printf("WAIT AT CMD Time OUT, line:%d!!!", __LINE__);
            res = -1;
        }
        //需要重新设置mode为0
        if (ml_state_code == ML_STATE_CHECK_SET_MODE) {
            os_sem_set(&at_cmd_sem, 0);
            res = usbnet_host_at_data_send(usb_id, LTE_MUDULE_AT_CMD[6], strlen(LTE_MUDULE_AT_CMD[6]));
            if (res < 0) {
                return res;
            }
            if (OS_TIMEOUT == os_sem_pend(&at_cmd_sem, SEM_WAIT_TIME_OUT)) {
                printf("WAIT AT CMD Time OUT, line:%d!!!", __LINE__);
                res = -1;
            }
            reboot_flag++;
        }

        ml_state_code = ML_STATE_CHECK_AUTO;
        os_sem_set(&at_cmd_sem, 0);
        res = usbnet_host_at_data_send(usb_id, LTE_MUDULE_AT_CMD[7], strlen(LTE_MUDULE_AT_CMD[7]));
        if (res < 0) {
            return res;
        }
        if (OS_TIMEOUT == os_sem_pend(&at_cmd_sem, SEM_WAIT_TIME_OUT)) {
            printf("WAIT AT CMD Time OUT, line:%d!!!", __LINE__);
            res = -1;
        }
        //需要重新设置auto为1
        if (ml_state_code == ML_STATE_CHECK_SET_AUTO) {
            os_sem_set(&at_cmd_sem, 0);
            res = usbnet_host_at_data_send(usb_id, LTE_MUDULE_AT_CMD[8], strlen(LTE_MUDULE_AT_CMD[8]));
            if (res < 0) {
                return res;
            }
            if (OS_TIMEOUT == os_sem_pend(&at_cmd_sem, SEM_WAIT_TIME_OUT)) {
                printf("WAIT AT CMD Time OUT, line:%d!!!", __LINE__);
                res = -1;
            }
            reboot_flag++;
        }
        if (ml_state_code == ML_STATE_CHECK_COMPLETE) {
            ml_state_code = ML_STATE_CHECK_IDEL;
        }

        if (reboot_flag) {
            printf("reboot make at command effetct!!!");
            app_usb_at_cmd_send_reset();
            os_time_dly(10);

#if (defined PRODUCTION_TEST_ENABLE || defined PRODUCTION_ALL_TEST_ENABLE)
            auto_rndis = true;
            lte_net_auto_rndis_up(0, &auto_rndis);//保存自动上网设置到flash
#endif
        }
    } else if (LTE_MUDULE_AT_VENDOR == VENDOR_ML120H) {//海思骐骏：ML120H
        //LTE_MUDULE_AT_CMD = ML120H_AT_TX_CMD;
        //后续需要优化：开机先读取是否是自动上网，不是才重新设置，是则跳过
        os_sem_set(&at_cmd_sem, 0);
        if (on_off) { //设置自动上网
            res = usbnet_host_at_data_send(usb_id, LTE_MUDULE_AT_CMD[5], strlen(LTE_MUDULE_AT_CMD[5]));
        } else { //取消自动上网
            res = usbnet_host_at_data_send(usb_id, LTE_MUDULE_AT_CMD[6], strlen(LTE_MUDULE_AT_CMD[6]));
        }
        if (res < 0) {
            return res;
        }

        if (OS_TIMEOUT == os_sem_pend(&at_cmd_sem, SEM_WAIT_TIME_OUT)) {
            printf("WAIT AT CMD Time OUT, line:%d!!!", __LINE__);
            res = -1;
        }

        //重启生效
        app_usb_at_cmd_send_reset();
        os_time_dly(10);
#if (defined PRODUCTION_TEST_ENABLE || defined PRODUCTION_ALL_TEST_ENABLE)
        auto_rndis = true;
        lte_net_auto_rndis_up(0, &auto_rndis);//保存自动上网设置到flash
#endif
    }
    return res;
}

//发AT指令，同步的过程，阻塞式API
int app_usb_at_cmd_send(u8 trig_type, u8 opt_code, u8 param)
{
    int res = 0;
    if (!usb_driver_ready) {
        printf("usb not ready!!!");
        return -2;
    }
    return res;
}

//俊骐4G模组手动拨号上网(manual dial-up)，每次开机都设置一次
int app_usb_net_manual_dialup(u8 net_on)
{
    int ret = 0;
    if (!usb_driver_ready) {
        printf("usb not ready!!!");
        return -2;
    }
    LTE_MUDULE_AT_VENDOR = qyai_usb_h_device_serial_at_id_prive_vendor_get();
    if (LTE_MUDULE_AT_VENDOR == VENDOR_ML120H) {//海思骐骏：ML120H
        if (net_on) {
            //获取对应4G设备的对应AT指令地址
            LTE_MUDULE_AT_CMD = qyai_usb_h_device_serial_at_cmd_str_get();
            if (!LTE_MUDULE_AT_CMD) {
                printf("warning no AT CMD\n");
                return -1;
            }
            os_sem_set(&at_cmd_sem, 0);
            ret = usbnet_host_at_data_send(usb_id, LTE_MUDULE_AT_CMD[8], strlen(LTE_MUDULE_AT_CMD[8]));
            if (ret < 0) {
                return ret;
            }
            if (OS_TIMEOUT == os_sem_pend(&at_cmd_sem, SEM_WAIT_TIME_OUT)) {
                printf("WAIT AT CMD Time OUT, line:%d!!!", __LINE__);
                ret = -1;
            }

            os_sem_set(&at_cmd_sem, 0);
            ret = usbnet_host_at_data_send(usb_id, LTE_MUDULE_AT_CMD[9], strlen(LTE_MUDULE_AT_CMD[9]));
            if (ret < 0) {
                return ret;
            }
            if (OS_TIMEOUT == os_sem_pend(&at_cmd_sem, SEM_WAIT_TIME_OUT)) {
                printf("WAIT AT CMD Time OUT, line:%d!!!", __LINE__);
                ret = -1;
            }
        } else {
            os_sem_set(&at_cmd_sem, 0);
            ret = usbnet_host_at_data_send(usb_id, LTE_MUDULE_AT_CMD[9], strlen(LTE_MUDULE_AT_CMD[9]));
            if (ret < 0) {
                return ret;
            }
            if (OS_TIMEOUT == os_sem_pend(&at_cmd_sem, SEM_WAIT_TIME_OUT)) {
                printf("WAIT AT CMD Time OUT, line:%d!!!", __LINE__);
                ret = -1;
            }
        }
    }
    return ret;
}

//切换SIM卡
int app_usb_at_change_sim_card(struct key_event *key)
{
    return 0;
}

#ifdef CONFIG_LTE_PHY_ENABLE
//判断SIM卡是否OK
int app_usb_at_if_sim_card_is_ok(void)
{
    int res = 0;
    lte_sim_card_status_opt(0, &sim_status);
    printf("sim_card_status_opt：%d,%d", res,   sim_status);
    switch (sim_status) {
    case SIM_CARD_OK:
        printf("SIM card ok\n");
        res = 1;
        break;
    case SIM_CARD_NOT_INSERT:
        printf("SIM card not insert\n");
        //SIM卡不OK->OK
        sys_net_channel_write(NET_CH_SELECT_LTE);
        break;
    case SIM_CARD_NOT_ACTIVATE:
        printf("SIM card not activate\n");
        //SIM卡不OK->OK
        sys_net_channel_write(NET_CH_SELECT_LTE);
        break;
    default:
        printf("SIM card unknow status\n");
        break;
    }
    return res;
}
#endif

//NOTE: 卡不支持热插拔，上电前确保卡插入
static int app_usb_module_sim_card_det(void)
{
    if (!usb_driver_ready) {
        printf("usb not ready!!!");
        return -2;
    }

    //获取对应4G设备的对应AT指令地址
    LTE_MUDULE_AT_CMD = qyai_usb_h_device_serial_at_cmd_str_get();
    if (!LTE_MUDULE_AT_CMD || !strcmp(LTE_MUDULE_AT_CMD[1], "NULL")) {
        printf("warning no AT CMD\n");
        return -1;
    }

    os_sem_set(&at_cmd_sem, 0);
    ml_state_code = ML_STATE_CHECK_SIM_CARD;

    //检测SIM卡是否存在
    int ret = usbnet_host_at_data_send(usb_id, LTE_MUDULE_AT_CMD[1], strlen(LTE_MUDULE_AT_CMD[1]));
    if (ret < 0) {
        return ret;
    }

    if (OS_TIMEOUT == os_sem_pend(&at_cmd_sem, SEM_WAIT_TIME_OUT)) {
        printf("err SIM CARD Detect Time OUT, line:%d!!!", __LINE__);
        sim_status = SIM_CARD_NOT_INSERT;
        lte_sim_card_status_opt(1, &sim_status);
        check_sim_card_err++;
        if (check_sim_card_err % SIM_CHECK_MAX == 0) {
            check_sim_card_err = 0;
            music_play_res_file("NetCardErr.mp3");
        }
        return -1;
    }

    int lte_iccid_dev_info_opt(u8 opt_code, char *iccid, int iccid_len);
    lte_iccid_dev_info_opt(1, iccid, sizeof(iccid));
    if (!iccid[0]) {
        os_sem_set(&at_cmd_sem, 0);
        ml_state_code = ML_STATE_READ_ICCID;

        //检测SIM卡的ICCID
        ret = usbnet_host_at_data_send(usb_id, LTE_MUDULE_AT_CMD[4], strlen(LTE_MUDULE_AT_CMD[4]));
        if (ret < 0) {
            return ret;
        }
        if (OS_TIMEOUT == os_sem_pend(&at_cmd_sem, SEM_WAIT_TIME_OUT)) {
            printf("err SIM CARD read ICCID Time OUT, line:%d!!!", __LINE__);
        }
        if (iccid[0]) {
            printf("get CCID : %s \n", iccid);
            lte_iccid_dev_info_opt(0, iccid, sizeof(iccid));
        }
    } else {
        printf("get flash CCID : %s \n", iccid);
    }
    return 0;
}

//NOTE: 联网失败的原因->(1)流量卡未激活 (2)板载天线有问题
static int app_usb_module_sim_card_net_check(void)
{
    if (!usb_driver_ready) {
        printf("usb not ready!!!");
        return -2;
    }

    //获取对应4G设备的对应AT指令地址
    LTE_MUDULE_AT_CMD = qyai_usb_h_device_serial_at_cmd_str_get();
    if (!LTE_MUDULE_AT_CMD || !strcmp(LTE_MUDULE_AT_CMD[2], "NULL")) {
        printf("warning no AT CMD\n");
        return -1;
    }

    os_sem_set(&at_cmd_sem, 0);
    ml_state_code = ML_STATE_CHECK_NET;

    //检测SIM卡是否能联网
    int ret = usbnet_host_at_data_send(usb_id, LTE_MUDULE_AT_CMD[2], strlen(LTE_MUDULE_AT_CMD[2]));
    if (ret < 0) {
        return ret;
    }

    if (OS_TIMEOUT == os_sem_pend(&at_cmd_sem, SEM_WAIT_TIME_OUT)) {
        printf("err SIM CARD Check Net Time OUT, line:%d!!!", __LINE__);
        sim_status = SIM_CARD_NOT_ACTIVATE;
        lte_sim_card_status_opt(1, &sim_status);
        check_sim_card_err++;
        if (check_sim_card_err % SIM_CHECK_MAX == 0) {
            check_sim_card_err = 0;
            music_play_res_file("NetCheckErr.mp3");
        }
        return -1;
    }

    //插入+联网
    sim_status = SIM_CARD_OK;
    lte_sim_card_status_opt(1, &sim_status);
    return 0;
}

//一直等到连接服务器  超时(60s)
static void app_usb_card_check_thread(void)
{
    int ret = 1;
    //设置自动拨号上网
    app_usb_net_auto_dialup(1);

    //SIM卡检测
    ml_state_code = ML_STATE_CHECK_SIM_CARD;
    while (ret && usb_driver_ready) {
        //检查SIM卡是否插入
        ret = app_usb_module_sim_card_det();
        os_time_dly(100);
    }

    ret = 1;
    while (ret && usb_driver_ready) {
        //检查SIM卡是否联网
        ret = app_usb_module_sim_card_net_check();
        os_time_dly(100);
    }

    ml_state_code = ML_STATE_CHECK_IDEL;
    printf("===========card check is ok!===============\r\n");
}

int app_usb_at_cmd_init(u8 usb_id_num)
{
    int ret;
    //注册at数据回调
    usbnet_at_port_rx_handler_register(app_usb_at_port_rx_cb);
    //允许4G-LTE发送数据
    usb_net_set_module_reset_status(0);

    usb_driver_ready = 1;
    usb_id = usb_id_num;
    check_sim_card_err = 0;

    //初始化信号量(无信号状态)
    os_sem_create(&at_cmd_sem, 0);
    os_sem_create(&io_opt_sem, 0);
    os_sem_create(&key_event_sem, 0);
    pthread_mutex_init(&mutex_start, NULL);
    pthread_mutex_init(&mutex_end, NULL);

    thread_fork("card_check", 10, 1024, 0, NULL, app_usb_card_check_thread, NULL);
    return 0;
}

//检测usb的plug in事件
static void app_usb_hardware_check_thread(void)
{
#define MAX_CNT (15)
    extern u8 IPV4_ADDR_CONFLICT_DETECT;
    u8 cnt = 0;
    int net_mode = sys_net_mode_read();
    do {
        if (app_usb_is_plugin_state()) {
            goto done;
        }
        os_time_dly(200);
        printf("=====plugin_state_check:%d", cnt);
        if (cnt++ == MAX_CNT) {
            break;
        }
    } while (1);

    //USB有硬件故障，用WIFI模式
    printf("=====err: usb hardware error=====:%d", cnt);
    sys_net_channel_write(NET_CH_SELECT_WIFI);
    sys_net_mode_write(NET_MODE_MANUAL_SET);
    system_reset();
done:
    return;
}

int app_usb_hardware_check(void)
{
    thread_fork("lte_hardware_check", 10, 4096, 0, NULL, app_usb_hardware_check_thread, NULL);
    return 0;
}

int app_usb_at_cmd_deinit(void)
{
    //不允许4G-LTE发送数据
    //usb_net_set_module_reset_status(1);
    usbnet_at_port_rx_handler_unregister();

    usb_driver_ready = 0;
    usb_id = 0;
    ml_state_code = ML_STATE_CHECK_IDEL;
    check_sim_card_err = 0;
    check_net_err = 0;
    if (!os_sem_query(&at_cmd_sem)) {
        os_sem_post(&at_cmd_sem);
    }
    if (!os_sem_query(&io_opt_sem)) {
        os_sem_post(&io_opt_sem);
    }
    if (!os_sem_query(&key_event_sem)) {
        os_sem_post(&key_event_sem);
    }

    return 0;
}

//注册AT接口列表
int app_usb_at_register_inter(void)
{
    return qyai_usb_h_device_serial_at_cmd_interface_register(app_AT_CMD_dev, ARRAY_SIZE(app_AT_CMD_dev));
}

int app_usb_at_unregister_inter(void)
{
    return qyai_usb_h_device_serial_at_cmd_interface_unregister();
}

#endif

