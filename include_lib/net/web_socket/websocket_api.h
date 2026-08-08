
#ifndef WEBSOCKET_API_H
#define WEBSOCKET_API_H


#include "websocket_define.h"
#include "websocket_base64.h"
#include "websocket_sha_1.h"
#include "websocket_intlib.h"
#include "websocket_api.h"
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


#define websockets_sleep  msleep


enum {
    NO_MSG = 0,
    ERCV_DATA_MSG,
    CLIENT_SEND_DATA_MSG,
    CLIENT_RECV_DATA_MSG,
    SERVER_SEND_DATA_MSG,
    SERVER_ERCV_DATA_MSG,
    CLIENT_PING_MSG,
    CLIENT_PONG_MSG,
    SERVER_PING_MSG,
    SERVER_PONG_MSG,

    RECV_TIME_OUT_MSG,
    DISCONNECT_MSG,
    CONNECT_RST_MSG,

    MAX_MSG = 32,
};

enum {
    ERR_CODE_NONE = 0,
    ERR_CODE_NORMAL_DISCON,//正常收到断开包
    ERR_CODE_EXCEPT_DISCON,//异常断开包
};

// websocket根据data[0]判别数据包类型    比如0x81 = 0x80 | 0x1 为一个txt类型数据包
typedef enum {//opcose：接收数据的第一个字节:0-3bit
    WS_TYPE_SEQ     = 0x00,     // 分片帧
    WS_TYPE_TXTDATA = 0x01,     // 0x1：标识一个txt类型数据帧
    WS_TYPE_BINDATA = 0x02,     // 0x2：标识一个bin类型数据帧
    WS_TYPE_DISCONN = 0x08,     // 0x8：标识一个断开连接类型数据帧
    WS_TYPE_PING    = 0x09,     // 0x9：表示一个ping类型数据包
    WS_TYPE_PONG    = 0x0a,     // 0xA：表示一个pong类型数据包
    WS_TYPE_FIN     = 0x80,     // fin: 结束帧
    WS_TYPE_RSV1    = 0x40,
    WS_TYPE_RSV2    = 0x20,
    WS_TYPE_RSV3    = 0x10,
    WS_TYPE_END     = 0x10,


    WS_TYPE_CLOSE_OK   = 0xaa,
    WS_TYPE_INIT    = 0xff,
} WS_CMD_Type;

typedef struct websockets_mbedtls {
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
} WEBSOCKETS_MBTLS_INFO;

struct websocket_req_head {
    u8 medthod[4];
    u8 file[1024];
    u8 host[32];
    u8 version[8];
};

typedef struct websocket_struct {
    void *sk_fd;
    void *lst_fd;
    int ping_thread_id;
    int recv_thread_id;
    char websocket_mode;
    char websocket_recvsub;
    u8 err_code;
    u8 websocket_data_type;
    u8 send_data_use_seq;
    u8 use_socket_api;
    u8 close_doing;
    u8 send_doing;
    u8 request_close;
    u8 recv_type_status;
    u8 no_alloc_buf;
    u16 port;
    struct sockaddr_in servaddr;
    struct sockaddr_in clientaddr;
    u8 *ip_or_url;
    const char *origin_str;
    const char *user_agent_str;
    const char *request_content;
    const char *user_data;
    int request_content_len;
    u8 ip_addr[16];
    u8 key[32];
    u8 key_len;
    u8 msg[MAX_MSG];
    u8 msg_write;
    u8 msg_read;
    volatile u8 msg_lock;
    u8 *send_buf;
    u8 *recv_buf;
    u32 recv_buff_size;
    u32 send_buff_size;
    u64 send_len;
    u64 recv_len;
    u32 recv_time_out;
    u32 send_time_out;
    u32 connect_time_out;
    u32 payload_data_len;
    u32 payload_data_continue;
    struct websocket_req_head req_head;
    struct websockets_mbedtls websockets_mbtls_info;
    u16 websocket_valid;
    int (*_init)(struct websocket_struct *websocket_info);
    void (*_exit)(struct websocket_struct *websocket_info);
    int (*_handshack)(struct websocket_struct *websocket_info);
    void (*_heart_thread)(void *param);
    void (*_recv_thread)(void *param);
    int (*_recv)(struct websocket_struct *websocket_info);
    int (*_send)(struct websocket_struct *websocket_info, u8 *buf, int len, char type);
    void (*_recv_cb)(u8 *buf, u32 len, u32 payloadlen, u8 type, u16 seq_enable, u8 end_seq);
    int (*_exit_notify)(struct websocket_struct *websocket_info);
} WEBSOCKET_INFO;

typedef enum {
    NOT_ESTABLISHED = 0x00,
    ESTABLISHED = 0x01,
    INVALID_ESTABLISHED = 0x02,
} WS_STATUS;


enum {
    WBSK_INI = 0,
    WBSK_INIING,
    WBSK_ONLINE,
    WBSK_OFFLINE,
    WBSK_REQ_OFFLINE,

    WBSK_SEND_BIN,
    WBSK_SEND_TEXT,
    WBSK_SEND_START,
    WBSK_SEND_END,
    WBSK_SEND_ERR,
    WBSK_SEND_DISCON,

    WBSK_RECV_BIN,
    WBSK_RECV_TEXT,
    WBSK_RECV_END,
    WBSK_RECV_MUSIC,
    WBSK_RECV_MP3_URL,

    WBSK_DIALOGUE_SEND_MEM_SIZE,
    WBSK_DIALOGUE_TIMEOUT_START,
    WBSK_DIALOGUE_TIMEOUT_RESET,
    WBSK_DIALOGUE_CLOSE,

    WBSK_REQUEST_MUSIC_NEXT,

    WBSK_TASK_KILL,
    WBSK_FREE_ALL_LBUF,

};

enum wbsk_status {
    WBSK_INI_STATUS = 0,
    WBSK_CONNECTING_STATUS,
    WBSK_CONNECTED_STATUS,
    WBSK_RECOED_STATUS,
    WBSK_WAITE_AUDIO_STATUS,
    WBSK_RECVED_AUDIO_STATUS,
    WBSK_RECVED_COMPLETE_STATUS,
    WBSK_RECVED_DISCON_ERR_STATUS,
};

struct send_buf {
    unsigned char num: 8;
    unsigned int len: 24;
    unsigned char data[0];
};

typedef struct chatgtp_info {
    void *lbuf_buff;
    void *lbuf;
    int lbuf_buff_size;
    int time_add_hdl;
    int time_send_hdl;
    int wbsk_light_pwm;
    int wbsk_light_pwm_last;
    int dns_update_time;

    unsigned int license_accpet;//授权次数
    unsigned int license_req_cnt;//请求次数
    unsigned int round_id;
    unsigned int best_one_packet;

    unsigned char alarm_del_all;
    unsigned char alarm_del;
    unsigned char alarm_cyc;
    unsigned char *alarm_url;
    int64_t alarm_int_val;
    unsigned int timeout_ms;

    unsigned char music_play_next;
    unsigned char *music_play_next_mode;
    unsigned short recv_music;
    unsigned char sending_pcm;
    unsigned char send_frame_cnt;
    unsigned char wbsk_online;
    unsigned char wbsk_next_dialogue;
    unsigned char key_word_recv;
    unsigned char online_vad_end;
    unsigned char key_dialogue;//按键对话
    unsigned char nodialuoge_count;
    unsigned char no_count_down;
    unsigned char no_play_audio;
    unsigned char fast_ip_index;
    unsigned char process_status;
    unsigned char text_dialogue;//text文本对话模式
    unsigned char audio_format;//0 pcm , 1 opus，2 text文本对话

    unsigned char wbsk_dialogue_play_recv_music;
    unsigned char wbsk_dialogue_close;//1立马退出，2超时退出
    unsigned char wbsk_dialogue_noneed_music;//1立马退出，2超时退出
    unsigned char wbsk_dialogue_tolong;
    unsigned char wbsk_dialogue_nothing;
    unsigned char wbsk_url_music_play_pause;
    unsigned char wbsk_nobind_account_message;
    unsigned char asr_nothing;

    unsigned char is_end_index;
    unsigned char is_net_music;
    unsigned char alarm_set;
    unsigned char new_asr;
    unsigned short index;
    unsigned short end_index;
    unsigned char alarm_content[240];
    unsigned char mac[6];
    unsigned char url[1024];
    unsigned char mp3_url[1024];
    unsigned char url_type[48];
    unsigned char message[512];
    unsigned char message_gbk[512];
    unsigned char music_info_type[4];
    unsigned char host_ip[16];//"255.255.255.255"
    unsigned char host_ip2[16 * 5];//"255.255.255.255"
    //本轮对话id
    unsigned char trace_id[32 + 4 + 1]; //加1个结束符

    unsigned int time_connect;
    unsigned int time_recoder;
    unsigned int time_vad_end_to_reply;
    //OS_MUTEX lbuf_mutex;
    char *wbsk_pcm_buf;
    int wbsk_pcm_buf_size;
    struct websocket_struct wbsk_info;
} GTP_INFO;

#define WEBSOCKET_DIALOGUE_CONTINIU         0//ASR_CONTINIU_ENABLE                  //连续对话使能
#define WEBSOCKET_DIALOGUE_TIMEOUT          (10*1000)           //10S
#define WEBSOCKET_DIALOGUE_TOLONG_TIMEOUT   (30*1000)           //30S只能一次对话传30秒音频
#define WEBSOCKET_SEND_LBUF_MAX             (15*(16*1000*2))    //PCM数据缓存大小,15秒
#define WEBSOCKET_NOTHING_ALLOW_CNT         3                   //"不好意思没听清"允许次数
//#define WEBSOCKET_DIALOGUE_ONECE_ENABLE                     //1次对话，每次都要唤醒

//#define WEBSOCKET_STATIC_MEM_SMALL
#define AUDIO_MEM_NOTICE_SIZE       (150*1024)
#define WEBSOCKET_V2_SEND_SIZE      (4*1024)
#define WEBSOCKET_OPUS_SEND_SIZE    (2*1024)

extern int atoi(const char *__nptr);

void websocket_msg_fifo(struct websocket_struct *websockets_info, u8 msg_value);
u8 websocket_msg_get(struct websocket_struct *websockets_info);
void websocket_msg_clear(struct websocket_struct *websockets_info);
void websockets_send_data_set_seq_packet(struct websocket_struct *websockets_info, char seq_en);
int websockets_socket_send(struct websocket_struct *websockets_info, u8 *buf, int len, char type);//通用数据发送，type自定义指定类型
int websockets_socket_set_timeout(struct websocket_struct *websockets_info, int send_timeout, int recv_timeout);
int websockets_copy_host_ip_addr(struct websocket_struct *websockets_info, char *ip_addr, int size);

/******************websockets**************************************/
int websockets_struct_check(int sizeof_struct);
int  websockets_pong_heart_beat(struct websocket_struct *websockets_info, u8 *buf, char index);
int  websockets_ping_heart_beat(struct websocket_struct *websockets_info, u8 *buf, char index);
void websockets_client_socket_heart_thread(void *param);
void websockets_client_socket_recv_thread(void *param);
void websockets_client_socket_exit(struct websocket_struct *websocket_info);
int  websockets_client_socket_send(struct websocket_struct *websocket_info, u8 *buf, int len, char type);
int  websockets_client_socket_recv(struct websocket_struct *websocket_info);
int  webcockets_client_socket_handshack(struct websocket_struct *websocket_info);
int  websockets_client_socket_init(struct websocket_struct *websocket_info);
int  websockets1_client_socket_init(struct websocket_struct *websocket_info);
int websockets_client_notify_disconnet_to_server(struct websocket_struct *websockets_info);//客户端往服务器发送disconnect消息
int webcockets_client_socket_connect_check(struct websocket_struct *websockets_info);
int websockets_request_head_user_data(struct websocket_struct *websockets_info, char *user_data);
int websockets_request_close(struct websocket_struct *websockets_info, char enable);
int websockets_recv_discon_type(struct websocket_struct *websockets_info);
int websockets_request_close_get(struct websocket_struct *websockets_info);

int websockets_get_max_packet_size(struct websocket_struct *websockets_info);
void websockets_serv_socket_heart_thread(void *param);
int  websockets_serv_socket_recv(struct websocket_struct *websockets_info);
int  websockets_serv_socket_send(struct websocket_struct *websockets_info, u8 *buf, int len, char type);
void websockets_serv_socket_exit(struct websocket_struct *websockets_info);
int  websockets_serv_socket_hanshack(struct websocket_struct *websockets_info);
int  websockets_serv_socket_init(struct websocket_struct *websockets_info);
int websockets_struct_check(int sizeof_struct);
int websockets_hanshake_set_content(struct websocket_struct *websockets_info, char *content, int len);

int websockets_socket_valid(struct websocket_struct *websockets_info);

#endif

