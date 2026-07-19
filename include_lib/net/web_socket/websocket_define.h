
#ifndef WEBSOCKET_DEF_H
#define WEBSOCKET_DEF_H

#include "string.h"

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;

#define     BIT(n)              (1UL << (n))

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif


#define ARRAY_SIZE(array)  (sizeof(array)/sizeof(array[0]))

/**********************************************/

#define WEBSOCKET_MODE      0   //不加密
#define WEBSOCKETS_MODE     1   //SSL加密

/*************************************/

#define CLIENNT_CHECK_INDEX         0
#define SERVER_CHECK_INDEX          1

#define SERVER_MASK_DISEN           1
#define CLIENNT_MASK_EN             1

#define SOCKET_SEND_RETRY_CNT       5
#define SOCKET_RECV_RETRY_CNT       5

#define HAND_SHAKE_BUF_MAX          (2048)
#define REQUEST_LEN_MAX             (1460)
#define FRAME_LEN_MAX               (1460)
#define DEFEULT_SERVER_PORT         (8000)
#define WEB_SOCKET_KEY_LEN_MAX      (1024)
#define RESPONSE_HEADER_LEN_MAX     (512)
#define WEB_SOCKET_TYPE_USE_SEQ_MAX (32*1024)
#define WEB_SOCKET_BEST_PACKET_SIZE (TCP_MSS)

#ifdef WEBSOCKET_STATIC_MEM_SMALL
#define ALL_FRAME_MAX                (FRAME_LEN_MAX*8)
#define WEB_SOCKETS_TYPE_USE_SEQ_MAX (4096 - 512) //wss一次最多收发：MBEDTLS_SSL_MAX_FRAG_LEN_4096 - 512，即 4092 - 512
#else
#define ALL_FRAME_MAX                (FRAME_LEN_MAX*16)
#define WEB_SOCKETS_TYPE_USE_SEQ_MAX (15*1024+32) //wss一次最多收发15K（多模态加上头部32），优化：MBEDTLS_SSL_MAX_CONTENT_LEN - 32

#endif

#endif



