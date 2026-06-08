#include "system/includes.h"
#include "server/audio_server.h"
#include "server/server_core.h"
#include "app_config.h"
#include "syscfg/syscfg_id.h"

static struct user_data_info {
#ifdef USED_TM1629_SHOWN
    unsigned char bright_level;
#endif
#ifdef CONFIG_LTE_PHY_ENABLE
    unsigned char auto_connect_net;
#if (CONFIG_LTE_VENDOR==VENDOR_GT108)
    //char iccid_buffer_0[20+1]; //长度根据手册来定，多一个字节存储结束符
    //char iccid_buffer_1[20+1];
    //char wifi_mac[12+1];
    char imei[15 + 1];
    char card_id;
#endif
#endif
    //char user_account[32]; //设备鉴权，用户的账户信息
#ifdef UART_MOTOR_ENABLE
    unsigned char motor_dir;
#endif
    unsigned char resv;
    unsigned short last_power_off; //上次关机计数
    unsigned short power_off; //本次关机计数
#ifdef CONFIG_SD_MUSIC_MODE_ENABLE
    unsigned short last_dir_num; //上次播放文件夹
    unsigned short last_file_num; //上次播放文件夹下文件数量
#endif
#ifdef CONFIG_USB_DISK_MUSIC_MODE_ENABLE
    unsigned short udisk_last_dir_num; //上次播放文件夹
    unsigned short udisk_last_file_num; //上次播放文件夹下文件数量
#endif
} USER_DATA = {0};

int user_is_soft_power_off_read(void)
{
    int ret = 0;
    if (syscfg_read(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA)) < 0) {
#if TCFG_FIRST_POWER_ON_FORCE_SLEEP
        return true;
#endif
        return ret;
    }
    ret = (USER_DATA.last_power_off != USER_DATA.power_off) ? true : 0;
    if (USER_DATA.last_power_off != USER_DATA.power_off) {
        USER_DATA.last_power_off = USER_DATA.power_off;
        syscfg_write(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA));
    }
    return ret;
}

int user_soft_power_off_read_cnt(void)
{
    if (syscfg_read(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA)) < 0) {
        return 0;
    }
    return USER_DATA.power_off;
}

int user_soft_power_off_write(void)
{
    if (syscfg_read(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA)) < 0) {
        memset(&USER_DATA, 0, sizeof(USER_DATA));
    }
#ifdef CONFIG_SD_MUSIC_MODE_ENABLE
    extern int user_sd_music_dir_file_get(int *dir, int *filen);
    int dir = 0;
    int filen = 0;
    user_sd_music_dir_file_get(&dir, &filen);
    USER_DATA.last_dir_num = (unsigned short)dir;
    USER_DATA.last_file_num = (unsigned short)filen;
#endif
#ifdef CONFIG_USB_DISK_MUSIC_MODE_ENABLE
    extern int user_udisk_music_dir_file_get(int *dir, int *filen);
    int udir = 0;
    int ufilen = 0;
    user_udisk_music_dir_file_get(&udir, &ufilen);
    USER_DATA.udisk_last_dir_num = (unsigned short)udir;
    USER_DATA.udisk_last_file_num = (unsigned short)ufilen;
#endif
    USER_DATA.last_power_off = USER_DATA.power_off;
    USER_DATA.power_off++;
    syscfg_write(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA));
    return 0;
}

int user_sd_music_last_dir_num_read(void)
{
    int ret = 0;
#ifdef CONFIG_SD_MUSIC_MODE_ENABLE
    if (syscfg_read(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA)) < 0) {
        return ret;
    }
    ret = USER_DATA.last_dir_num;
#endif
    return ret;
}

int user_sd_music_last_file_num_read(void)
{
    int ret = 0;
#ifdef CONFIG_SD_MUSIC_MODE_ENABLE
    if (syscfg_read(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA)) < 0) {
        return ret;
    }
    ret = USER_DATA.last_file_num;
#endif
    return ret;
}

int user_sd_music_dir_file_num_write(int dir_num, int file_num)
{
#ifdef CONFIG_SD_MUSIC_MODE_ENABLE
    if (syscfg_read(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA)) < 0) {
        memset(&USER_DATA, 0, sizeof(USER_DATA));
    }
    USER_DATA.last_dir_num = (unsigned short)dir_num;
    USER_DATA.last_file_num = (unsigned short)file_num;
    syscfg_write(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA));
#endif
    return 0;
}

int user_data_bright_level_read(void)
{
    if (syscfg_read(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA)) < 0) {
        return -1;
    }
#ifdef  USED_TM1629_SHOWN
    return USER_DATA.bright_level;
#endif
    return 0;
}

int user_data_bright_level_write(u8 level)
{
#ifdef  USED_TM1629_SHOWN
    if (syscfg_read(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA)) < 0) {
        memset(&USER_DATA, 0, sizeof(USER_DATA));
    }
    USER_DATA.bright_level = level;
    syscfg_write(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA));
#endif
    return 0;
}

int user_motor_dir_read(void)
{
    if (syscfg_read(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA)) < 0) {
        return -1;
    }
#ifdef  UART_MOTOR_ENABLE
    return USER_DATA.motor_dir;
#endif
    return 0;
}

int user_motor_dir_write(u8 level)
{
#ifdef  UART_MOTOR_ENABLE
    if (syscfg_read(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA)) < 0) {
        memset(&USER_DATA, 0, sizeof(USER_DATA));
    }
    USER_DATA.motor_dir = level;
    syscfg_write(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA));
#endif
    return 0;
}

#if 0
int user_data_account_info_opt(u8 opt_code, char *account, int len)
{
    if (syscfg_read(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA)) < 0) {
        memset(&USER_DATA, 0, sizeof(USER_DATA));
    }

    if (account == NULL || len <= 0) {
        printf("invalid params: account=%p, len=%d\n", account, len);
        return -1;
    }

    //写入Flash
    if (opt_code == 0) {
        int copy_len = len < sizeof(USER_DATA.user_account) ? len : sizeof(USER_DATA.user_account);
        memset(USER_DATA.user_account, 0, sizeof(USER_DATA.user_account));
        memcpy(USER_DATA.user_account, account, copy_len);
        syscfg_write(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA));
    } else if (opt_code == 1) { // 从Flash读取
        int copy_len = sizeof(USER_DATA.user_account) < len ? sizeof(USER_DATA.user_account) : len;
        memcpy(account, USER_DATA.user_account, copy_len);
        if (len > copy_len) { //超出部分填0
            memset(account + copy_len, 0, len - copy_len);
        }
    } else {
        printf("invalid opt code:%d", opt_code);
        return -1;
    }

    return 0;
}
#endif

#ifdef  CONFIG_LTE_PHY_ENABLE
int lte_rndis_auto_connet_net(void *auto_set_save)
{
    if (syscfg_read(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA)) < 0) {
        memset(&USER_DATA, 0, sizeof(USER_DATA));
    }
    if (auto_set_save) {
        USER_DATA.auto_connect_net = true;
        syscfg_write(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA));
    }
    return USER_DATA.auto_connect_net != true ? 0 : USER_DATA.auto_connect_net;
}

#if (CONFIG_LTE_VENDOR==VENDOR_GT108)
int lte_dev_info_opt(u8 opt_code, char *iccid_buf_0, char *iccid_buf_1, char *wifi_mac_buf, char *imei_buf, char *card_id)
{
#if 0
    if (opt_code == 0) {
        ;
    } else {
        if (imei_buf != NULL) {
            memcpy(USER_DATA.imei, "ABCDEFGHIJK", sizeof("ABCDEFGHIJK"));
            memcpy(imei_buf, USER_DATA.imei, sizeof(USER_DATA.imei));
        }
        if (card_id != NULL) {
            USER_DATA.card_id = 1;
            *card_id = USER_DATA.card_id;
        }
    }
    return 0;
#endif

    if (syscfg_read(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA)) < 0) {
        memset(&USER_DATA, 0, sizeof(USER_DATA));
    }

    //note: 读/写都已经包含有结束符的处理
    if (opt_code == 0) {
        //覆盖数据
#if 0
        memcpy(USER_DATA.iccid_buffer_0, iccid_buf_0, sizeof(USER_DATA.iccid_buffer_0));
        memcpy(USER_DATA.iccid_buffer_1, iccid_buf_1, sizeof(USER_DATA.iccid_buffer_1));
        memcpy(USER_DATA.wifi_mac, wifi_mac_buf, sizeof(USER_DATA.wifi_mac));
#endif
        if (imei_buf != NULL) {
            memcpy(USER_DATA.imei, imei_buf, sizeof(USER_DATA.imei));
        }
        if (card_id != NULL) {
            USER_DATA.card_id = *card_id;
        }
        //写入flash
        syscfg_write(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA));
    } else if (opt_code == 1) {
#if 0
        memcpy(iccid_buf_0, USER_DATA.iccid_buffer_0, sizeof(USER_DATA.iccid_buffer_0));
        memcpy(iccid_buf_1, USER_DATA.iccid_buffer_1, sizeof(USER_DATA.iccid_buffer_1));
        memcpy(wifi_mac_buf, USER_DATA.wifi_mac, sizeof(USER_DATA.wifi_mac));
#endif
        if (imei_buf != NULL) {
            memcpy(imei_buf, USER_DATA.imei, sizeof(USER_DATA.imei));
        }
        if (card_id != NULL) {
            *card_id = USER_DATA.card_id;
        }
    } else {
        printf("unvalid opt code:%d", opt_code);
    }

    return 0;
}
#endif
#endif // CONFIG_LTE_PHY_ENABLE

int user_data_info_clear(void)
{
    if (syscfg_read(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA)) < 0) {
        return 0;
    }
    memset(&USER_DATA, 0, sizeof(USER_DATA));
    syscfg_write(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA));
    return 0;
}

