#include "system/includes.h"
#include "server/audio_server.h"
#include "server/server_core.h"
#include "app_config.h"
#include "syscfg/syscfg_id.h"

static struct lte_wifi_info {
    unsigned char channel;
    unsigned char change_mode;
    unsigned char produc_test;
    unsigned char resv2;
    unsigned char ssid[64];
    unsigned char pwd[64];
#ifdef CONFIG_LTE_PHY_ENABLE
    unsigned char auto_rndis;
    unsigned char sim_card_status_flag; //SIM卡状态
    unsigned char iccid[21];
    unsigned char imsi[21];
    unsigned char imei[21];
#endif
    unsigned char net_mode;//0:wifi/4g、1:auto mode
} LTE_WIFI_INFO;

int sys_production_test(char test)
{
    u8 value = 0;
    if (syscfg_read(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO)) < 0) {
        return -1;
    }
    if (LTE_WIFI_INFO.produc_test != test) {
        LTE_WIFI_INFO.produc_test = test;
        syscfg_write(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO));
    }
    return LTE_WIFI_INFO.produc_test;
}

int sys_net_channel_read(void)
{
#ifdef CONFIG_LTE_PHY_ENABLE
    u8 value = 0;
    if (syscfg_read(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO)) < 0) {
        if (DEFAULT_NET_CHANNEL) {
            memset(&LTE_WIFI_INFO, 0, sizeof(LTE_WIFI_INFO));
            LTE_WIFI_INFO.channel = DEFAULT_NET_CHANNEL;
            syscfg_write(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO));
            return LTE_WIFI_INFO.channel;
        }
        return -1;
    }
    //printf("-> read LTE_WIFI_INFO.channel = %d \n",LTE_WIFI_INFO.channel);
    return LTE_WIFI_INFO.channel;
#endif
    return 0;
}

//0为wifi,1为4G
int sys_net_channel_write(u8 value)
{
#ifdef CONFIG_LTE_PHY_ENABLE
    char write_data = 0;
    if (syscfg_read(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO)) < 0) {
        memset(&LTE_WIFI_INFO, 0, sizeof(LTE_WIFI_INFO));
        write_data = 1;
    }
    if (LTE_WIFI_INFO.channel != value || write_data) {
        LTE_WIFI_INFO.channel = value;
        //printf("-> write LTE_WIFI_INFO.channel = %d \n",LTE_WIFI_INFO.channel);
        if (syscfg_write(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO)) < 0) {
            return -1;
        }
    }
#endif
    return 0;
}

int sys_net_channel_ssid_read(char **ssid, char **pwd)
{
    if (syscfg_read(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO)) < 0) {
        return -1;
    }
    if (!LTE_WIFI_INFO.change_mode) {
        return -1;
    }
    if (LTE_WIFI_INFO.ssid[0] != 0 && ssid) {
        //memcpy(ssid, strlen(LTE_WIFI_INFO.ssid) + 1);
        *ssid = LTE_WIFI_INFO.ssid;
        if (pwd) {
            //memcpy(pwd, strlen(LTE_WIFI_INFO.pwd) + 1);
            *pwd = LTE_WIFI_INFO.pwd;
        }
        return 0;
    }
    return -1;
}

int sys_net_channel_ssid_reset(void)
{
    if (syscfg_read(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO)) < 0) {
        return -1;
    }
    if (!LTE_WIFI_INFO.change_mode) {
        return -1;
    }
    memset(LTE_WIFI_INFO.ssid, 0, sizeof(LTE_WIFI_INFO.ssid));
    memset(LTE_WIFI_INFO.pwd, 0, sizeof(LTE_WIFI_INFO.pwd));
    LTE_WIFI_INFO.change_mode = 0;
    syscfg_write(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO));
    return 0;
}

int sys_net_channel_ssid_write(char *ssid, char *pwd)
{
    if (ssid) {
        memset(LTE_WIFI_INFO.ssid, 0, sizeof(LTE_WIFI_INFO.ssid));
        memset(LTE_WIFI_INFO.pwd, 0, sizeof(LTE_WIFI_INFO.pwd));
        memcpy(LTE_WIFI_INFO.ssid, ssid, strlen(ssid) + 1);
        LTE_WIFI_INFO.change_mode = true;
        if (pwd && *pwd != 0) {
            memcpy(LTE_WIFI_INFO.pwd, pwd, strlen(pwd) + 1);
        }
        if (syscfg_write(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO)) < 0) {
            return -1;
        }
        return 0;
    }
    return -1;
}

#ifdef CONFIG_LTE_PHY_ENABLE
int lte_net_auto_rndis_up(u8 opt, u8 *auto_rndis)
{
#ifdef CONFIG_LTE_PHY_ENABLE
    if (syscfg_read(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO)) < 0) {
        memset(&LTE_WIFI_INFO, 0, sizeof(LTE_WIFI_INFO));
    }
    if (opt == 1) {
        if (auto_rndis) {
            *auto_rndis = LTE_WIFI_INFO.auto_rndis == true;
        }
        return LTE_WIFI_INFO.auto_rndis == true;
    } else if (auto_rndis && LTE_WIFI_INFO.auto_rndis != (u8) * auto_rndis) {
        LTE_WIFI_INFO.auto_rndis = (u8) * auto_rndis;
        syscfg_write(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO));
    }
    return LTE_WIFI_INFO.auto_rndis;
#endif
    return 0;
}

int lte_iccid_dev_info_opt(u8 opt_code, char *iccid, int iccid_len)
{
    if (syscfg_read(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO)) < 0) {
        memset(&LTE_WIFI_INFO, 0, sizeof(LTE_WIFI_INFO));
    }
    switch (opt_code) {
    case 1:
        if (LTE_WIFI_INFO.iccid[0] != 0 && LTE_WIFI_INFO.iccid[0] != 0xFF) {
            strncpy(iccid, LTE_WIFI_INFO.iccid, MIN(iccid_len, sizeof(LTE_WIFI_INFO.iccid)));
        }
        break;
    default :
        if (iccid && strlen(iccid) < sizeof(LTE_WIFI_INFO.iccid)) {
            memset(LTE_WIFI_INFO.iccid, 0, sizeof(LTE_WIFI_INFO.iccid));
            strncpy(LTE_WIFI_INFO.iccid, iccid, MIN(iccid_len, sizeof(LTE_WIFI_INFO.iccid)));
            syscfg_write(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO));
        }
        break;
    }
    return 0;
}

int lte_imsi_dev_info_opt(u8 opt_code, char *imsi, int imsi_len)
{
    if (syscfg_read(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO)) < 0) {
        memset(&LTE_WIFI_INFO, 0, sizeof(LTE_WIFI_INFO));
    }
    switch (opt_code) {
    case 1:
        if (LTE_WIFI_INFO.imsi[0] != 0 && LTE_WIFI_INFO.imsi[0] != 0xFF) {
            strncpy(imsi, LTE_WIFI_INFO.imsi, MIN(imsi_len, sizeof(LTE_WIFI_INFO.imsi)));
        }
        break;
    default :
        if (imsi && strlen(imsi) < sizeof(LTE_WIFI_INFO.imsi)) {
            memset(LTE_WIFI_INFO.imsi, 0, sizeof(LTE_WIFI_INFO.imsi));
            strncpy(LTE_WIFI_INFO.imsi, imsi, MIN(imsi_len, sizeof(LTE_WIFI_INFO.imsi)));
            syscfg_write(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO));
        }
        break;
    }
    return 0;
}

int lte_sim_card_status_opt(u8 opt_code, u8 *dat)
{
    if (dat == NULL) {
        printf("Error: dat is NULL!!!");
        return -1;
    }
    //read
    if (opt_code == 0) {
        if (syscfg_read(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO)) < 0) {
            return -1;
        }
        *dat = LTE_WIFI_INFO.sim_card_status_flag;
        return 0;
        //write
    } else if (opt_code == 1) {
        if (syscfg_read(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO)) < 0) {
            memset(&LTE_WIFI_INFO, 0, sizeof(LTE_WIFI_INFO));
        }
        LTE_WIFI_INFO.sim_card_status_flag = *dat;
        syscfg_write(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO));
        return 0;
    }
}

#if (CONFIG_LTE_VENDOR == VENDOR_GT108)
int lte_dev_info_opt(u8 opt_code, char *iccid_buf_0, char *iccid_buf_1, char *wifi_mac_buf, char *imei_buf, char *card_id)
{
    if (syscfg_read(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO)) < 0) {
        memset(&LTE_WIFI_INFO, 0, sizeof(LTE_WIFI_INFO));
    }
    //note: 读/写都已经包含有结束符的处理
    if (opt_code == 0) {
        //覆盖数据
#if 1
        if (iccid_buf_0) {
            memcpy(LTE_WIFI_INFO.iccid_buffer_0, iccid_buf_0, sizeof(LTE_WIFI_INFO.iccid_buffer_0));
        }
        if (iccid_buf_1) {
            memcpy(LTE_WIFI_INFO.iccid_buffer_1, iccid_buf_1, sizeof(LTE_WIFI_INFO.iccid_buffer_1));
        }
        //memcpy(LTE_WIFI_INFO.wifi_mac, wifi_mac_buf, sizeof(LTE_WIFI_INFO.wifi_mac));
#endif
        if (imei_buf != NULL) {
            memcpy(LTE_WIFI_INFO.imei, imei_buf, sizeof(LTE_WIFI_INFO.imei));
        }
        if (card_id != NULL) {
            LTE_WIFI_INFO.card_id = *card_id;
        }
        //写入flash
        syscfg_write(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO));
    } else if (opt_code == 1) {
#if 1
        if (iccid_buf_0 && LTE_WIFI_INFO.iccid_buffer_0[0] && LTE_WIFI_INFO.iccid_buffer_0[0] != 0xFF) {
            memcpy(iccid_buf_0, LTE_WIFI_INFO.iccid_buffer_0, sizeof(LTE_WIFI_INFO.iccid_buffer_0));
        }
        if (iccid_buf_1 && LTE_WIFI_INFO.iccid_buffer_1[0] && LTE_WIFI_INFO.iccid_buffer_1[0] != 0xFF) {
            memcpy(iccid_buf_1, LTE_WIFI_INFO.iccid_buffer_1, sizeof(LTE_WIFI_INFO.iccid_buffer_1));
        }
        //memcpy(wifi_mac_buf, LTE_WIFI_INFO.wifi_mac, sizeof(LTE_WIFI_INFO.wifi_mac));
#endif
        if (imei_buf != NULL && LTE_WIFI_INFO.imei[0] && LTE_WIFI_INFO.imei[0] != 0xFF) {
            memcpy(imei_buf, LTE_WIFI_INFO.imei, sizeof(LTE_WIFI_INFO.imei));
        }
        if (card_id != NULL && LTE_WIFI_INFO.card_id != 0xFF) {
            *card_id = LTE_WIFI_INFO.card_id;
        }
    } else {
        printf("unvalid opt code:%d", opt_code);
    }

    return 0;
}
#endif
#endif // CONFIG_LTE_PHY_ENABLE

int sys_net_mode_read(void)
{
    u8 value = 0;
    if (syscfg_read(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO)) < 0) {
        printf("=====sys_net_mode_read error!=====");
        return -1;
    }
    //printf("-> read LTE_WIFI_INFO.net_mode = %d \n",LTE_WIFI_INFO.net_mode);
    return LTE_WIFI_INFO.net_mode;
}

int sys_net_mode_write(u8 value)
{

    char write_data = 0;
    if (syscfg_read(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO)) < 0) {
        memset(&LTE_WIFI_INFO, 0, sizeof(LTE_WIFI_INFO));
    }

    LTE_WIFI_INFO.net_mode = value;
    //printf("-> write LTE_WIFI_INFO.channel = %d \n",LTE_WIFI_INFO.channel);
    if (syscfg_write(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO)) < 0) {
        printf("=====sys_net_mode_write error!=====");
        return -1;
    }

    return 0;
}

int sys_net_channel_info_clear(void)
{
    if (syscfg_read(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO)) < 0) {
        return -1;
    }
    memset(&LTE_WIFI_INFO, 0, sizeof(LTE_WIFI_INFO));
#ifdef CONFIG_LTE_PHY_ENABLE
    LTE_WIFI_INFO.channel = DEFAULT_NET_CHANNEL;
#endif
    return syscfg_write(CFG_USER_LET_WIFI_SELECT, &LTE_WIFI_INFO, sizeof(LTE_WIFI_INFO));
}

