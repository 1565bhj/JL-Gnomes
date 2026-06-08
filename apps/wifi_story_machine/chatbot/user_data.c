#include "system/includes.h"
#include "server/audio_server.h"
#include "server/server_core.h"
#include "app_config.h"
#include "syscfg/syscfg_id.h"

static struct user_data_info {
#ifdef USED_TM1629_SHOWN
    unsigned char bright_level;
#endif
    unsigned char user_account[48];
#ifdef UART_MOTOR_ENABLE
    unsigned char motor_dir;
#endif
#ifdef CONFIG_UI_ENABLE
    unsigned char lcd_light;
#endif
    unsigned char resv;
    unsigned short last_power_off; //上次关机计数
    unsigned short power_off; //本次关机计数
    unsigned short connnect_wifi_cnt;
#ifdef CONFIG_SD_MUSIC_MODE_ENABLE
    unsigned short last_dir_num; //上次播放文件夹
    unsigned short last_file_num; //上次播放文件夹下文件数量
#endif
#ifdef CONFIG_USB_DISK_MUSIC_MODE_ENABLE
    unsigned short udisk_last_dir_num; //上次播放文件夹
    unsigned short udisk_last_file_num; //上次播放文件夹下文件数量
#endif
    unsigned char alarm_index;
} USER_DATA = {0};

int user_connect_server_cnt_read(void)
{
    if (syscfg_read(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA)) < 0) {
        return 0;
    }
    return USER_DATA.connnect_wifi_cnt;
}

int user_connect_server_cnt_write(void)
{
    if (syscfg_read(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA)) < 0) {
        memset(&USER_DATA, 0, sizeof(USER_DATA));
    }
    USER_DATA.connnect_wifi_cnt++;
    syscfg_write(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA));
    return 0;
}

#ifdef CONFIG_UI_ENABLE
int user_lcd_light_read(void)
{
    if (syscfg_read(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA)) < 0) {
        return 0;
    }
    return USER_DATA.lcd_light;
}

int user_lcd_light_write(u8 light_value)
{
    if (syscfg_read(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA)) < 0) {
        memset(&USER_DATA, 0, sizeof(USER_DATA));
    }
    USER_DATA.lcd_light = light_value;
    printf("USER_DATA.lcd_light =%d\n", USER_DATA.lcd_light);
    syscfg_write(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA));
    return 0;
}
#endif

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
    extern int alarm_time_index(void);
    USER_DATA.alarm_index = alarm_time_index();
    USER_DATA.last_power_off = USER_DATA.power_off;
    USER_DATA.power_off++;
    syscfg_write(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA));
    return 0;
}

int user_read_alarm_index(void)
{
    if (syscfg_read(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA)) < 0) {
        return 0;
    }
    if (USER_DATA.alarm_index == 0xFF) {
        return 0;
    }
    return USER_DATA.alarm_index;
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

int user_data_account_info_opt(u8 opt_code, char *account, int len)
{
    if (syscfg_read(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA)) < 0) {
        memset(&USER_DATA, 0, sizeof(USER_DATA));
    }

    //写入Flash
    if (opt_code == 0 && account && len > 0) {
        if (memcmp(USER_DATA.user_account, account, MIN(sizeof(USER_DATA.user_account), len))) {
            memset(USER_DATA.user_account, 0, sizeof(USER_DATA.user_account));
            memcpy(USER_DATA.user_account, account,  MIN(sizeof(USER_DATA.user_account), len));
            syscfg_write(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA));
        }
    } else if (opt_code == 1 && USER_DATA.user_account[0] != 0 && USER_DATA.user_account[0] != 0XFF && len > 0) { // 从Flash读取
        memcpy(account, USER_DATA.user_account, MIN(sizeof(USER_DATA.user_account), len));
    } else {
        printf("invalid opt code:%d", opt_code);
        return -1;
    }
    return 0;
}

int user_data_info_clear(void)
{
    if (syscfg_read(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA)) < 0) {
        return 0;
    }
    memset(&USER_DATA, 0, sizeof(USER_DATA));
    syscfg_write(CFG_USER_CONFIG_DATA, &USER_DATA, sizeof(USER_DATA));
    return 0;
}

