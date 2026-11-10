#include "system/includes.h"
#include "app_config.h"
#include "update/update.h"
#include "fs/fs.h"
#include "update/update_loader_download.h"
#include "update/net_update.h"
#include "update/http_update.h"
#include "json_c/json.h"
#include "json_c/json_tokener.h"

#ifdef CONFIG_NET_ENABLE
#ifndef CONFIG_NO_SDRAM_ENABLE
#include "http/http_cli.h"
#include "http_chunck_stream_api.h"

/* 临时固定 HTTP OTA 地址：下载到 net_update 的系统固件升级通道 */
#define HTTP_OTA_FIXED_URL ""
#define HTTP_OTA_URL_MAX_LEN 128

//#define SD_FILE_OTA_ENABLE //sd卡升级

static char http_update_status = 0;
static char http_update_task_init = 0;
static char http_ota_url_valid = 0;
static char http_ota_url[HTTP_OTA_URL_MAX_LEN];
#define HTTP_OTA_RESPONSE_PROBING 0
#define HTTP_OTA_RESPONSE_JSON    1
#define HTTP_OTA_RESPONSE_BINARY  2
#define HTTP_OTA_RESPONSE_HTML    3
static char http_ota_response_type = HTTP_OTA_RESPONSE_PROBING;
static uint64_t http_download_percent = 0;
static void *update_fd = NULL;

#define HTTP_OTA_PROBE_BUF_SIZE 4096
static unsigned char http_ota_probe_buf[HTTP_OTA_PROBE_BUF_SIZE];
static int http_ota_probe_len = 0;
static int http_ota_probe_current_len = 0;
static u16 http_ota_expected_data_crc = 0;
static u32 http_ota_expected_data_len = 0;
static u16 http_ota_data_crc = 0;
static u64 http_ota_data_len = 0;
static char http_ota_crc_started = 0;
static struct json_tokener *http_ota_json_tokener = NULL;

typedef struct {
    u16 u16Crc;
    u16 u16DataCrc;
    u32 u32Address;
    u32 u32Length;
    u8 u8Attribute;
    u8 u8Res;
    u16 u16Index;
    char szFileName[16];
} HTTP_OTA_FILE_HEAD;

#define HTTP_OTA_FILE_HEAD_SIZE sizeof(HTTP_OTA_FILE_HEAD)

extern int jl_file_head_valid_check(void *head);
extern u16 calc_crc16_with_init_val(u16 init_crc, u8 *ptr, u16 len);

int http_chunck_stream_client_socket_request_thread(HTTP_CHUNCK_STREAM_INFO *HTTP,
        char *url,
        int (*buf_callbak)(void *http, char *buf, int len, int payloadlen, char fin),
        char *static_buf, int static_buf_size);

int http_is_update(void)
{
    return http_update_status;
}

static int http_ota_is_text_response(char *buf, int len, int payloadlen)
{
    int i;
    int copy_len;

    (void)payloadlen;
    http_ota_probe_current_len = 0;
    if (update_fd || http_ota_response_type != HTTP_OTA_RESPONSE_PROBING) {
        return 0;
    }

    copy_len = len;
    if (http_ota_probe_len + copy_len > HTTP_OTA_PROBE_BUF_SIZE) {
        copy_len = HTTP_OTA_PROBE_BUF_SIZE - http_ota_probe_len;
    }
    if (copy_len > 0) {
        memcpy(http_ota_probe_buf + http_ota_probe_len, buf, copy_len);
        http_ota_probe_len += copy_len;
        http_ota_probe_current_len = copy_len;
    }

    for (i = 0; i < http_ota_probe_len; i++) {
        if (http_ota_probe_buf[i] != ' ' && http_ota_probe_buf[i] != '\r' &&
            http_ota_probe_buf[i] != '\n' && http_ota_probe_buf[i] != '\t') {
            break;
        }
    }
    if (i == http_ota_probe_len) {
        return 0;
    }

    if (http_ota_probe_buf[i] == '{' || http_ota_probe_buf[i] == '[') {
        http_ota_response_type = HTTP_OTA_RESPONSE_JSON;
    } else if (http_ota_probe_buf[i] == '<') {
        http_ota_response_type = HTTP_OTA_RESPONSE_HTML;
    } else {
        http_ota_response_type = HTTP_OTA_RESPONSE_BINARY;
    }
    return http_ota_response_type == HTTP_OTA_RESPONSE_JSON ||
           http_ota_response_type == HTTP_OTA_RESPONSE_HTML;
}

static void http_ota_json_reset(void)
{
    if (http_ota_json_tokener) {
        json_tokener_free(http_ota_json_tokener);
        http_ota_json_tokener = NULL;
    }
}

static int http_ota_handle_text_response(char *buf, int len, char fin)
{
    struct json_object *new_obj;
    enum json_tokener_error parse_err;

    if (http_ota_response_type == HTTP_OTA_RESPONSE_HTML) {
        printf("OTA text error response: %.*s\n", len, buf);
        return HTTP_ERR;
    }
    if (!http_ota_json_tokener) {
        http_ota_json_tokener = json_tokener_new();
        if (!http_ota_json_tokener) {
            printf("OTA JSON parser allocation error\n");
            return HTTP_ERR;
        }
    }

    new_obj = json_tokener_parse_ex(http_ota_json_tokener, buf, len);
    if (new_obj) {
        struct json_object *message = json_object_object_get(new_obj, "message");
        if (message) {
            char *str = json_object_get_string(message);
            if (str) {
                printf("message : \n%s\n", str);
            }
        }
        json_object_put(new_obj);
        http_ota_json_reset();
        return HTTP_ERR;
    }

    parse_err = json_tokener_get_error(http_ota_json_tokener);
    if (parse_err == json_tokener_continue && !fin) {
        return HTTP_OK;
    }
    printf("OTA invalid or incomplete JSON error response: %.*s\n", len, buf);
    http_ota_json_reset();
    return HTTP_ERR;
}

static int http_ota_file_head_valid(void)
{
    HTTP_OTA_FILE_HEAD head;

    if (http_ota_probe_len < HTTP_OTA_FILE_HEAD_SIZE) {
        return -1;
    }
    memcpy(&head, http_ota_probe_buf, sizeof(head));
    if (jl_file_head_valid_check(&head) || head.u32Length <= HTTP_OTA_FILE_HEAD_SIZE ||
        strncmp(head.szFileName, "update_data", sizeof(head.szFileName))) {
        return -1;
    }
    http_ota_expected_data_crc = head.u16DataCrc;
    /* u32Length includes the header; CRC and data length exclude it. */
    http_ota_expected_data_len = head.u32Length - HTTP_OTA_FILE_HEAD_SIZE;
    return 0;
}

static void http_ota_collect_probe(char *buf, int len)
{
    int copy_len = 0;

    if (http_ota_probe_len < HTTP_OTA_PROBE_BUF_SIZE) {
        copy_len = HTTP_OTA_PROBE_BUF_SIZE - http_ota_probe_len;
        if (copy_len > len) {
            copy_len = len;
        }
        if (copy_len > 0) {
            memcpy(http_ota_probe_buf + http_ota_probe_len, buf, copy_len);
            http_ota_probe_len += copy_len;
        }
    }
    http_ota_probe_current_len = copy_len;
}

static void http_ota_update_crc(unsigned char *buf, int len)
{
    u16 part_len;

    if (len <= 0) {
        return;
    }
    while (len > 0) {
        part_len = len > 0xffff ? 0xffff : (u16)len;
        http_ota_data_crc = calc_crc16_with_init_val(http_ota_data_crc, buf, part_len);
        http_ota_data_len += part_len;
        buf += part_len;
        len -= part_len;
    }
}

static int http_data_callback(void *http, char *buf, int len, int payloadlen, char fin)
{
    int ret = 0;
    HTTP_CHUNCK_STREAM_INFO *phttp = http;
    if (!buf || len <= 0) {
        printf("OTA err : %d\n", len);
        goto exit;
    }
    http_download_percent += len;
    http_ota_probe_current_len = 0;
    if (http_ota_is_text_response(buf, len, payloadlen)) {
        printf("OTA recv %s %d : \n%.*s\n", (phttp && phttp->chunked) ? "Chunked" : "Content-Length", len, len, buf);
        if (http_ota_handle_text_response(buf, len, fin) == HTTP_OK) {
            return HTTP_OK;
        }
        goto exit;
    }
    if (http_ota_response_type == HTTP_OTA_RESPONSE_JSON) {
        if (http_ota_handle_text_response(buf, len, fin) == HTTP_OK) {
            return HTTP_OK;
        }
        goto exit;
    }
    if (http_ota_response_type == HTTP_OTA_RESPONSE_HTML) {
        printf("OTA text error response: %.*s\n", len, buf);
        goto exit;
    }
    if (http_ota_response_type == HTTP_OTA_RESPONSE_BINARY && !http_ota_crc_started && http_ota_probe_current_len == 0) {
        http_ota_collect_probe(buf, len);
    }
    if (http_ota_response_type == HTTP_OTA_RESPONSE_PROBING) {
        if (!fin) {
            return HTTP_OK;
        }
        http_ota_response_type = HTTP_OTA_RESPONSE_BINARY;
    }
    if (!update_fd) {
        if (http_ota_probe_len < HTTP_OTA_FILE_HEAD_SIZE) {
            if (!fin) {
                return HTTP_OK;
            }
            printf("OTA firmware header is incomplete\n");
            goto exit;
        }
        if (http_ota_file_head_valid()) {
            printf("OTA invalid firmware header or response body\n");
            printf("OTA response preview: %.*s\n", http_ota_probe_len > 256 ? 256 : http_ota_probe_len, http_ota_probe_buf);
            goto exit;
        }
#ifdef SD_FILE_OTA_ENABLE
        update_fd = fopen(CONFIG_ROOT_PATH CONFIG_UPGRADE_OTA_FILE_NAME, "w+");
#else
        update_fd = net_fopen(CONFIG_UPGRADE_OTA_FILE_NAME, "w+");
#endif
    }
    if (!update_fd) {
        printf("OTA open file err\n");
        goto exit;
    }
    http_update_status = true;
    if (!http_ota_crc_started) {
        http_ota_update_crc(http_ota_probe_buf + HTTP_OTA_FILE_HEAD_SIZE,
                            http_ota_probe_len - HTTP_OTA_FILE_HEAD_SIZE);
        http_ota_crc_started = 1;
    }
    if (http_ota_probe_len > 0) {
#ifdef SD_FILE_OTA_ENABLE
        ret = fwrite(http_ota_probe_buf, 1, http_ota_probe_len, update_fd);
#else
        ret = net_fwrite(update_fd, http_ota_probe_buf, http_ota_probe_len, 0);
#endif
        if (ret != http_ota_probe_len) {
            printf("OTA write error : 0x%x\n", ret);
            goto exit;
        }
        http_ota_probe_len = 0;
    }
    if (http_ota_probe_current_len < len) {
        http_ota_update_crc((unsigned char *)buf + http_ota_probe_current_len,
                            len - http_ota_probe_current_len);
#ifdef SD_FILE_OTA_ENABLE
        ret = fwrite(buf + http_ota_probe_current_len, 1, len - http_ota_probe_current_len, update_fd);
#else
        ret = net_fwrite(update_fd, buf + http_ota_probe_current_len, len - http_ota_probe_current_len, 0);
#endif
        if (ret != len - http_ota_probe_current_len) {
            printf("OTA write error : 0x%x\n", ret);
            goto exit;
        }
    }
    if (payloadlen > 0) {
        printf("OTA len:%d, upload : %llu, total:%d (%llu%%)\n", len, (unsigned long long)http_download_percent, payloadlen, (unsigned long long)(http_download_percent * 100 / payloadlen));
    } else {
        printf("OTA len:%d, upload : %llu\n", len, (unsigned long long)http_download_percent);
    }
    if (fin) {
        if (payloadlen > 0 && http_download_percent != (uint64_t)payloadlen) {
            printf("OTA size mismatch: received %llu, expected %d\n", (unsigned long long)http_download_percent, payloadlen);
#ifdef SD_FILE_OTA_ENABLE
            fclose(update_fd);
#else
            net_fclose(update_fd, (char)-1);
#endif
            update_fd = NULL;
            http_update_status = false;
            return HTTP_ERR;
        }
        if (http_ota_data_len != http_ota_expected_data_len ||
            http_ota_data_crc != http_ota_expected_data_crc) {
            printf("OTA firmware verify error: data_len=%llu/%u, crc=0x%x/0x%x\n",
                   (unsigned long long)http_ota_data_len, http_ota_expected_data_len,
                   http_ota_data_crc, http_ota_expected_data_crc);
#ifdef SD_FILE_OTA_ENABLE
            fclose(update_fd);
#else
            net_fclose(update_fd, (char)-1);
#endif
            update_fd = NULL;
            http_update_status = false;
            return HTTP_ERR;
        }
#ifdef SD_FILE_OTA_ENABLE
        ret = fclose(update_fd);
#else
        ret = net_fclose(update_fd, (char)0);
#endif
        update_fd = NULL;
        http_update_status = false;
        if (ret) {
            printf("OTA finalize/verify error: %d\n", ret);
            return HTTP_ERR;
        }
        printf("OTA write file OK\n");
    }
    return HTTP_OK;

exit:
    http_ota_json_reset();
    if (update_fd) {
#ifdef SD_FILE_OTA_ENABLE
        fclose(update_fd);
#else
        net_fclose(update_fd, (char) -1);
#endif
        update_fd = NULL;
    }
    http_update_status = false;
    return HTTP_ERR;
}

static int http_ota_task(void *priv)
{
    const char *source_url = http_ota_url_valid ? http_ota_url : HTTP_OTA_FIXED_URL;
    size_t url_len = strlen(source_url);
    char *url = malloc(url_len + 1);
    http_update_task_init = true;
    if (url) {
        extern int sys_connect_net_success(void);
        int to = 60 * 60;
        while (!sys_connect_net_success() && --to) {
            os_time_dly(100);
        }
#if (defined TCFG_VBAT_CHECK_EN && TCFG_VBAT_CHECK_EN == 1)
        extern int vbat_check_percent(void);
        int vbat_perc = vbat_check_percent();
        if (vbat_perc < 40) { //电压过低
            os_time_dly(10);
            vbat_perc = vbat_check_percent();
            if (vbat_perc < 40) { //电压过低不升级
                goto exit;
            }
        }
#endif
        if (priv == NULL) {
            os_time_dly(500);//非MQTT触发，5秒钟后升级
        }
        if (to) {
            memcpy(url, source_url, url_len + 1);
            printf("-> HTTP OTA : %s\n", url);
            http_download_percent = 0;
            http_ota_json_reset();
            http_ota_response_type = HTTP_OTA_RESPONSE_PROBING;
            http_ota_probe_len = 0;
            http_ota_probe_current_len = 0;
            http_ota_expected_data_crc = 0;
            http_ota_expected_data_len = 0;
            http_ota_data_crc = 0;
            http_ota_data_len = 0;
            http_ota_crc_started = 0;
            int request_ret = http_chunck_stream_client_socket_request_thread(NULL, url, http_data_callback, NULL, 0);
            if (request_ret < 0) {
                printf("HTTP OTA request error: %d\n", request_ret);
            }
        }
        free(url);
    } else {
        printf("HTTP OTA: malloc URL buffer failed\n");
        goto exit;
    }
exit:
    http_ota_url_valid = 0;
    http_update_task_init = 0;
    return 0;
}

int http_ota_init(void *priv)
{
    priv = priv == 1 ? 1 : NULL;
    if (production_io_is_enter() || is_production_test_enter(0) || system_is_alarm_wakeup() || http_update_task_init) {
        return -1;
    }
    http_ota_url_valid = 0;
    http_update_task_init = true;
    if (thread_fork("http_ota", 10, 2400, 0, 0, http_ota_task, priv)) {
        http_update_task_init = false;
        return -2;
    }
    return 0;
}

int http_ota_init_with_url(const char *url)
{
    size_t url_len;

    if (!url || !url[0]) {
        printf("HTTP OTA: empty URL\n");
        return -1;
    }
    url_len = strlen(url);
    if (url_len >= sizeof(http_ota_url)) {
        printf("HTTP OTA: URL too long (%u)\n", (unsigned int)url_len);
        return -1;
    }
    if (production_io_is_enter() || is_production_test_enter(0) || system_is_alarm_wakeup() || http_update_task_init) {
        printf("HTTP OTA: start rejected, task is busy or system is unavailable\n");
        return -1;
    }
    memcpy(http_ota_url, url, url_len + 1);
    http_ota_url_valid = 1;
    http_update_task_init = true;
    if (thread_fork("http_ota", 10, 2400, 0, 0, http_ota_task, (void *)1)) {
        http_ota_url_valid = 0;
        http_update_task_init = false;
        printf("HTTP OTA: create task failed\n");
        return -2;
    }
    return 0;
}

#endif
#endif
