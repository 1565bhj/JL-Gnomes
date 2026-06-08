#include "app_config.h"
#include "json_c/json.h"
#include "json_c/json_tokener.h"
#include "fs/fs.h"
#include "asm/sfc_norflash_api.h"

#ifdef CONFIG_MQTT_IOT_ENABLE
#include "os/os_api.h"
#include "system/includes.h"
#include "net_update.h"
#include "cJSON.h"
#include "mbedtls/md.h"

//MQTT相关信息存储在flashD的资源预留区，不能存VM区域，防止被乱改

#define MQTT_DEV_INFO_ADDR  0x76f000 //请根据isd_config_rule.c的mqtt.bin_ADR值填写，务必保持一致

#define DEC_NUM 99 //<127

//static struct mqtt_user_data_info {
//    unsigned int  mqtt_server_port;
//    unsigned char mqtt_client_id[84];
//    unsigned char mqtt_address[48];
//    unsigned char mqtt_username[48];
//    unsigned char mqtt_password[33];
//} MQTT_USER_DATA = {0};

static void mqtt_dev_info_dec(unsigned char *buf, int len)
{
    for (int i = 0; i < len; i++) {
        if (buf[i] > 0 && buf[i] != 0xFF) {
            buf[i] -= DEC_NUM;
        }
    }
}
static void mqtt_dev_info_enc(unsigned char *buf, int len)
{
    for (int i = 0; i < len; i++) {
        if (buf[i] > 0 && buf[i] < 0x80) {
            buf[i] += DEC_NUM;
        }
    }
}
static int mqtt_dev_info_file_read(char *buf, int buf_len)
{
    int ret = 0;
    FILE *f = fopen("mnt/sdfile/EXT_RESERVED/mqtt.bin", "r");
    if (!f) {
        printf("fopen err!\n");
    } else {
        char name[32];
        fget_name(f, name, sizeof(name));
        printf("file_name: %s\n", name);
        int len = fread(buf, 1, buf_len, f);
        if (len > 0) {
            ret = strlen(buf);
            ret = ret > buf_len ? len : ret;
            mqtt_dev_info_dec(buf, ret);
            //put_buf(buf, len);
            return ret;
        }
    }
    return 0;
}
int mqtt_dev_info_flash_read(int *server_port,
                             char *client_ID, int client_ID_len,
                             char *address, int address_len,
                             char *username, int username_len,
                             char *password, int password_len,
                             char *product_key, int product_key_len,
                             char *device_name, int device_name_len,
                             char *device_secret, int device_secret_len)
{
    int ret = -1;
    char *buf = malloc(4096);
    if (buf) {
        memset(buf, 0, 4096);
        //1.打开flash设备
        norflash_open(NULL, NULL, NULL);

        if (mqtt_dev_info_file_read(buf, 4096)) {
            /*
            {
            "server_port": 1833,
            "client_ID": "id_test",
            "address": "home.test.com",
            "username": "hometest",
            "password": "ABcd123456",
            "ProductKey": "a1jgj408xm2",
            "DeviceName": "8E658415F05A-LLKJS5Ky-B1N1-XHZ",
            "DeviceSecret": "5fb1b6f462661cc6eed1ea23d286419a",
            "QRCode_URL": "https://test.12356.com"
            }
            */
            json_object *new_obj = NULL;
            json_object *parm = NULL;
            char *Serverport = "server_port";
            int Serverport_val = 0;
            char *Client_ID = "client_ID";
            char *Address = "address";
            char *Username = "username";
            char *Password = "password";
            char *Client_ID_val = NULL;
            char *Address_val = NULL;
            char *Username_val = NULL;
            char *Password_val = NULL;

            char *ProductKey = "ProductKey";
            char *DeviceName = "DeviceName";
            char *DeviceSecret = "DeviceSecret";
            char *QRCode_URL = "QRCode_URL";
            char *ProductKey_val = NULL;
            char *DeviceName_val = NULL;
            char *DeviceSecret_val = NULL;
            new_obj = json_tokener_parse(buf);
            if (!new_obj) {
                free(buf);
                return -1;
            }
            printf("->mqtt read info : \n%s\n", buf);
            parm = json_object_object_get(new_obj, Serverport);
            if (parm) {
                Serverport_val = json_object_get_string(parm);
            }
            parm = json_object_object_get(new_obj, Client_ID);
            if (parm) {
                Client_ID_val = json_object_get_string(parm);
            }
            parm = json_object_object_get(new_obj, Address);
            if (parm) {
                Address_val = json_object_get_string(parm);
            }
            parm = json_object_object_get(new_obj, Username);
            if (parm) {
                Username_val = json_object_get_string(parm);
            }
            parm = json_object_object_get(new_obj, Password);
            if (parm) {
                Password_val = json_object_get_string(parm);
            }
            if (Address_val && Username_val && Password_val) {
                if (server_port && Serverport_val) {
                    *server_port = Serverport_val;
                }
                if (client_ID && client_ID_len > strlen(Client_ID_val)) {
                    strcpy(client_ID, Client_ID_val);
                }
                if (address && address_len > strlen(Address_val)) {
                    strcpy(address, Address_val);
                }
                if (username && username_len > strlen(Username_val)) {
                    strcpy(username, Username_val);
                }
                if (password && password_len > strlen(Password_val)) {
                    strcpy(password, Password_val);
                }
            }

            //获取"voice_text"
            parm = json_object_object_get(new_obj, ProductKey);
            if (parm) {
                ProductKey_val = json_object_get_string(parm);
            }
            parm = json_object_object_get(new_obj, DeviceName);
            if (parm) {
                DeviceName_val = json_object_get_string(parm);
            }
            parm = json_object_object_get(new_obj, DeviceSecret);
            if (parm) {
                DeviceSecret_val = json_object_get_string(parm);
            }
            if (ProductKey_val && DeviceName_val && DeviceSecret_val) {
                if (product_key && product_key_len > strlen(ProductKey_val)) {
                    strcpy(product_key, ProductKey_val);
                }
                if (device_name && device_name_len > strlen(DeviceName_val)) {
                    strcpy(device_name, DeviceName_val);
                }
                if (device_secret && device_secret_len > strlen(DeviceSecret_val)) {
                    strcpy(device_secret, DeviceSecret_val);
                }
            }
            json_object_put(new_obj);
            ret = 0;
        }
        free(buf);
    }
    return ret;
}

int mqtt_dev_mac_read(char *mac)
{
    unsigned char device_name[36] = {0};
    unsigned char tmp;

//    mqtt_dev_info_flash_read(&MQTT_USER_DATA.mqtt_server_port,
//                             MQTT_USER_DATA.mqtt_client_id, sizeof(MQTT_USER_DATA.mqtt_client_id),
//                             MQTT_USER_DATA.mqtt_address, sizeof(MQTT_USER_DATA.mqtt_address),
//                             MQTT_USER_DATA.mqtt_username, sizeof(MQTT_USER_DATA.mqtt_username),
//                             MQTT_USER_DATA.mqtt_password, sizeof(MQTT_USER_DATA.mqtt_password),
//                             NULL, 0,
//                             device_name, sizeof(device_name),
//                             NULL, 0);
    mqtt_dev_info_flash_read(NULL,
                             NULL, 0,
                             NULL, 0,
                             NULL, 0,
                             NULL, 0,
                             NULL, 0,
                             device_name, sizeof(device_name),
                             NULL, 0);
    if (device_name[0] != 0) {
        int i = 0, j = 0;
        while (device_name[i] != '-' && i < sizeof(device_name)) {
            i++;
        }
        if (i == 12) { //MAC固定12个字符串
            device_name[i] = 0;
            j = 0, tmp = 0;
            while (j < i) {
                for (char n = 0; n < 2; n++) {
                    tmp <<= 4;
                    if (device_name[j + n] >= '0' && device_name[j + n] <= '9') {
                        tmp |= (device_name[j + n] - '0');
                    } else if (device_name[j + n] >= 'a' && device_name[j + n] <= 'f') {
                        tmp |= (10 + device_name[j + n] - 'a');
                    } else if (device_name[j + n] >= 'A' && device_name[j + n] <= 'F') {
                        tmp |= (10 + device_name[j + n] - 'A');
                    }
                }
                mac[5 - (j / 2)] = tmp;
                j += 2;
            }
            return 0;
        }
    }
    return -1;
}
/* buf数据例如：
    {
    "server_port": 1833,
    "client_ID": "id_test",
    "address": "home.test.com",
    "username": "hometest",
    "password": "ABcd123456",
    "ProductKey": "a1jgj408xm2",
    "DeviceName": "8E658415F05A-LLKJS5Ky-B1N1-XHZ",
    "DeviceSecret": "5fb1b6f462661cc6eed1ea23d286419a",
    "QRCode_URL": "https://test.12356.com"
    }
*/
int mqtt_dev_info_flash_write(unsigned char *buf, int buf_len)
{
    unsigned int write_sum = 0;
    unsigned int read_sum = 0;

    if (!buf || !buf_len) {
        return -1;
    }
    for (int i = 0; i < buf_len; i++) {
        write_sum += buf[i];
    }

    mqtt_dev_info_enc(buf, buf_len);

    //擦除一个扇区
    puts("USER_FLASH_EXIF ERASE_SECTOR...\r\n");
    //put_buf(buf,buf_len);

    norflash_protect_suspend();//解除flash写保护

    norflash_ioctl(NULL, IOCTL_ERASE_SECTOR, MQTT_DEV_INFO_ADDR);

    //写入数据
    norflash_write(NULL, buf, buf_len, MQTT_DEV_INFO_ADDR);

    norflash_protect_resume();

    memset(buf, 0, buf_len);
    //再次读出来，查看数据是否写入成功
    norflash_read(NULL, buf, buf_len, MQTT_DEV_INFO_ADDR);
    //put_buf(buf,buf_len);

    mqtt_dev_info_dec(buf, buf_len);

    for (int i = 0; i < buf_len; i++) {
        read_sum += buf[i];
    }

    if (read_sum != write_sum || read_sum == 0 || write_sum == 0) {
        puts("mqtt_dev_info_write err !\r\n");
        return -1;
    }
    puts("mqtt_dev_info_write ok\r\n");
    return 0;
}


#endif

