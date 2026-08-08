#ifndef HTTP_CHUNCK_STREAM_API_H
#define HTTP_CHUNCK_STREAM_API_H

#include "string.h"

#include "mbedtls/mbedtls_config.h"
//#include "mbedtls/platform.h"
#include "mbedtls/net.h"
//#include "mbedtls/debug.h"
//#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "generic/typedef.h"
#include "mbedtls/certs.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"


#define http_chunck_stream_sleep  msleep

#ifdef HTTP_CHUNCK_MEM_SMALL
#define ALL_FRAME_MAX           (8*1024)
#else
#define ALL_FRAME_MAX           (150*1024)
#endif

#ifdef MBEDTLS_SSL_MEM_SMALL
#define HTTPS_MBTLE_FRAME_MAX   (MBEDTLS_SSL_MAX_FRAG_LEN_4096)
#else
#define HTTPS_MBTLE_FRAME_MAX   (16*1024)
#endif

//#define HTTP_POST_BODY_URLENCODE //POST的body采用x-www-form-urlencoded类型，格式：id=124&name=%23%ED%EA

enum {
    HTTP_MODE = 0,
    HTTPS_MODE,
};
enum {
    HTTP_OK = 0,
    HTTP_ERR = -1,
    HTTP_ERR_NO_MEM = -2,
    HTTP_RECV_ERR = -3,
    HTTP_TIMEOUT = -4,

    HTTP_RANGE = -5,
    HTTP_STOP = -6,
    HTTP_REQ_NO_FILE = -6,
};
typedef struct http_chunck_stream_mbedtls {
    /*client*/
    mbedtls_net_context server_fd;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;
    char ssl_fd;

    /*server*/
    u8 sll_ip_addr[16];
    mbedtls_net_context client_fd;//add client fd
    mbedtls_x509_crt srvcert;
    mbedtls_pk_context pkey;

    /* CA */
    char *mbedtls_ca_buf;
    int mbedtls_ca_size;
} HTTP_CHUNCK_STREAM_MBTLS_INFO;

struct http_chunck_stream_req_head {
    u8 medthod[4];
    u8 file[1024];
    u8 host[48];
    u8 version[8];
};

typedef struct http_chunck_stream_struct {
    u8 openning;
    u8 static_buf;
    u8 chunked;
    u8 encoding;
    u8 *ip_or_url;
    u8 *send_buf;
    u32 send_buf_size;
    u32 send_len;
    u32 recv_len;
    u32 recv_all_len;
    u32 content_length;
    u32 range_start;
    u32 range_end;
    u32 start_offset;
    int sk_fd;
    u8 http_mode;
    u8 use_socket_api;
    u8 close_doing;
    u8 request_close;
    u8 range_surrport;
    u16 port;
    struct sockaddr_in clientaddr;
    const char *origin_str;
    const char *user_agent_str;
    const char *request_content;
    int request_content_len;
    u8 ip_addr[16];
    u32 recv_time_out;
    u32 send_time_out;
    struct http_chunck_stream_req_head req_head;
    struct http_chunck_stream_mbedtls http_chunck_stream_mbtls_info;
    int (*buf_cb)(void *http, char *buf, int len, int payloadlen,  char fin);
    u16 http_chunck_stream_valid;
} HTTP_CHUNCK_STREAM_INFO;


typedef struct content_details {
    int id;
    int page;
    void *buf;
    int buf_size;
    void (*set_callback)(void *http, char *buf, int len, int payloadlen, char fin);
    void *keyword;
} QYAI_CONTENT_DATA;

typedef struct user_info {
    void *user_nickname;
    void *user_account;
    void *user_phone;
    int  user_id;
    void *register_ip;
    void *esim_active_url;
    int msg_code;
    int works_id;
    int timbre_id;
    void *works_name;
    void *callback_priv;
    void (*set_callback)(void *http, char *buf, int len, int payloadlen, char fin);
} QYAI_USER_INFO;


extern int http_chunck_stream_client_socket_post_body_thread(HTTP_CHUNCK_STREAM_INFO *HTTP,
        char *url,  char *body_dat,
        int (*buf_callback)(void *http, char *buf, int len, int payloadlen, char fin),
        char *static_buf, int static_buf_size);
extern int http_chunck_stream_client_socket_request_thread(HTTP_CHUNCK_STREAM_INFO *HTTP,
        char *url,
        int (*buf_callback)(void *http, char *buf, int len, int payloadlen, char fin),
        char *static_buf, int static_buf_size);
#endif

