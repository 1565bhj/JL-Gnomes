#include "sock_api/sock_api.h"
#include "os/os_api.h"
#include "server/audio_server.h"
#include "update/net_update.h"
#include "app_config.h"

#define TEST_RUN_TIME_SEC   60
#ifdef CONFIG_UI_ENABLE
#define TEST_PCM_BUFF_SIZE  (64*1024)
#else
#define TEST_PCM_BUFF_SIZE  (512*1024)
#endif

#define PCM_STREAM_TCP_PORT     5002
#define PCM_STREAM_UDP_PORT     5003

enum {
    NO_FILE = 0,
    LOCAL_TEST,
    LOCAL_SIN_TEST,
    LOCAL_1TO24K_TEST,
    LOCAL_CYC_TEST,
};

static void *udp_socket;
static void *server_socket;
static void *client_socket;
static struct sockaddr_in udp_clientaddr;
static u32 run_sec = 0;
static char *mic_buf;
static cbuffer_t pcm_cbuf;
static OS_SEM pcm_sem;
static char socket_err = 0;
static char play_local_file = 0;

int music_play_stop(void *priv);
int aisp_mic_gain_set(unsigned char volume);
int ai_speaker_volume_set(int volume, char update_gain);
int aisp_all_mic_gain_set(int volume, int aec_gian, int mic0_gian, int mic1_gian);
void go_mask_usb_updata(void);

static int wifi_pcm_stream_socket_create(void)
{
    struct sockaddr_in local_ipaddr;
    struct sockaddr_in clientaddr;
    struct sockaddr_in udp_local_ipaddr;

    u32 len = sizeof(clientaddr);
    char ipaddr[20];
    char *msg = "wifi_pcm_connet";
    char *msg_start = "wifi_pcm_start";
    char *msg_local_file = "local_file";
    char *sin_test_file = "sin_test";
    char *test_1to24Khz_file = "1-24Khz_test";
    char *test_cyc = "test_cyc";
    char *vol = "-vol:";
    char *aec = "-aec:";
    char *mic0 = "-mic0:";
    char *mic1 = "-mic1:";
    char *reset = "-reset:";
    char buf[84];
    int send_to_millsec = 500;
    int recv_to_millsec = 400;

    if (!server_socket) {
        server_socket = sock_reg(AF_INET, SOCK_STREAM, 0, NULL, NULL);
        if (server_socket == NULL) {
            goto __err;
        }

        if (0 != sock_set_reuseaddr(server_socket)) {
            goto __err;
        }

        local_ipaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        local_ipaddr.sin_port = htons(PCM_STREAM_TCP_PORT);
        local_ipaddr.sin_family = AF_INET;

        if (0 != sock_bind(server_socket, (struct sockaddr *)&local_ipaddr, sizeof(local_ipaddr))) {
            goto __err;
        }

        if (0 != sock_listen(server_socket, 1)) {
            goto __err;
        }
    }

    if (!udp_socket) {
        udp_socket = sock_reg(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, NULL);
        if (udp_socket == NULL) {
            goto __err;
        }

        udp_local_ipaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        udp_local_ipaddr.sin_port = htonl(INADDR_BROADCAST);//htons(PCM_STREAM_UDP_PORT);
        udp_local_ipaddr.sin_family = AF_INET;

        sk_setsockopt(udp_socket, SOL_SOCKET, SO_RCVTIMEO, (const void *)&recv_to_millsec, sizeof(recv_to_millsec));
        sk_setsockopt(udp_socket, SOL_SOCKET, SO_SNDTIMEO, (const void *)&send_to_millsec, sizeof(send_to_millsec));

        int optval = 1;
        sk_setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR, (const void *)&optval, sizeof(optval));

        sock_set_recv_timeout(udp_socket, recv_to_millsec);

        udp_clientaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);//inet_addr("255.255.255.255");//INADDR_ANY ¡À?¦Ì?IP;
        udp_clientaddr.sin_port = htons(PCM_STREAM_UDP_PORT);
        udp_clientaddr.sin_family = AF_INET;
    }
    u8 volume = 0;
    u8 mac[6];
    int recv_volume = 0;
    int recv_aec = 0;
    int recv_mic0 = 0;
    int recv_mic1 = 0;
    char *uuid = app_user_product_uuid();
    if (!uuid) {
        printf("attention: use default product uuid!!!\r\n");
        //use product uuid of qy-ai
        uuid = user_product_uuid_default();
    }

    char *pdnum = user_product_batch_num();
    char *pdname = app_user_product_name();
    int rst_reson = system_reset_reason_get();
    bt_ble_get_mac(mac);//使用蓝牙mac
    while (1) {
        sys_volume_read(&volume);
        sprintf(buf, "%s%s%d", msg, vol, volume);
//        if(uuid){
//            sprintf(buf,"%s-mac:%02X%02X%02X%02X%02X%02X-%s-%s-%s",buf,mac[5], mac[4],mac[3],mac[2],mac[1],mac[0],uuid,pdnum,OTA_VERSON);
//        }else{
//            sprintf(buf,"%s-mac:%02X%02X%02X%02X%02X%02X-%s-%s",buf,mac[5], mac[4],mac[3],mac[2],mac[1],mac[0],pdnum,OTA_VERSON);
//        }
        sprintf(buf, "%s-mac:%02X%02X%02X%02X%02X%02X-0x%x-%s", buf, mac[5], mac[4], mac[3], mac[2], mac[1], mac[0], rst_reson, pdname);
        sock_sendto(udp_socket, buf, strlen(buf) + 1, 0, (const struct sockaddr *)&udp_clientaddr, (socklen_t)sizeof(struct sockaddr_in)); //?¨®¨º?o¡¥¨ºyrecvfrom()
//        putchar('U');
        len = sizeof(clientaddr);
        memset(buf, 0, sizeof(buf));
        if (sock_recvfrom(udp_socket, buf, sizeof(buf), 0, (struct sockaddr *)&clientaddr, (socklen_t *)&len) > 0) {
            printf("->sock_recvfrom : %s \n", buf);
            if (strstr(buf, msg_start)) {
                char *p = strstr(buf, vol);
                if (p) {
                    recv_volume = atoi(p + strlen(vol));
                }
                p = strstr(buf, aec);
                if (p) {
                    recv_aec = atoi(p + strlen(aec));
                }
                p = strstr(buf, mic0);
                if (p) {
                    recv_mic0 = atoi(p + strlen(mic0));
                }
                p = strstr(buf, mic1);
                if (p) {
                    recv_mic1 = atoi(p + strlen(mic1));
                }
                p = strstr(buf, msg_local_file);
                if (p) {
                    play_local_file = LOCAL_TEST;
                }
                p = strstr(buf, sin_test_file);
                if (p) {
                    play_local_file = LOCAL_SIN_TEST;
                }
                p = strstr(buf, test_1to24Khz_file);
                if (p) {
                    play_local_file = LOCAL_1TO24K_TEST;
                }
                p = strstr(buf, test_cyc);
                if (p) {
                    play_local_file = LOCAL_CYC_TEST;
                }
#if (!defined PRODUCTION_TEST_ENABLE && !defined PRODUCTION_ALL_TEST_ENABLE)
                p = strstr(buf, reset);
                if (p) {
                    p += strlen(reset);
                    if (*p) {
                        char *end = NULL;
                        int hex1 = (int)strtoul(p, &end, 16);
                        int hex2 = 0;
                        if (end && *end != 0) {
                            end += 1;
                            hex2 = (int)strtoul(end, NULL, 16);
                            if (((hex1 ^ 0xD8C7B6A5) | 0xA1B2C3D4) == hex2) {
                                printf("->reset go to mask usb updata\n");
                                go_mask_usb_updata();
                            }
                        }
                    }
                }
#endif
                sock_sendto(udp_socket, msg_start, strlen(msg_start) + 1, 0, (const struct sockaddr *)&clientaddr, (socklen_t)sizeof(struct sockaddr_in));
                os_time_dly(100);
                sock_sendto(udp_socket, msg_start, strlen(msg_start) + 1, 0, (const struct sockaddr *)&clientaddr, (socklen_t)sizeof(struct sockaddr_in));
                printf("->sock_recvfrom :OK\n");
                break;
            }
        }
    }
    if (recv_volume >= 10 && recv_volume <= 100) {
        ai_speaker_volume_set(recv_volume, 0);
        if (recv_aec) {
            aisp_all_mic_gain_set(recv_volume, recv_aec, recv_mic0, recv_mic1);
        } else if (play_local_file) {
            aisp_mic_gain_set(recv_volume);
        }
    }
    len = sizeof(clientaddr);
    client_socket = sock_accept(server_socket, (struct sockaddr *)&clientaddr, (socklen_t *)&len, NULL, NULL);
    if (!client_socket) {
        goto __err;
    }

    sock_set_send_timeout(client_socket, 3000);
    run_sec = 0;
    return 0;

__err:
    if (server_socket) {
        sock_unreg(server_socket);
        server_socket = NULL;
    }
    if (udp_socket) {
        sock_unreg(udp_socket);
        udp_socket = NULL;
    }
    return -1;
}

void wifi_pcm_stream_socket_send(u8 *buf, u32 len)
{
//    if (client_socket) {
//        sock_send(client_socket, buf, len, 0);
//    }
    if (mic_buf && !socket_err && client_socket) {
        u32 vl_writed_len = cbuf_write(&pcm_cbuf, buf, len);
        if (vl_writed_len != len) {
            puts("err wifi_pcm_ cbuf_write\n");
        }
        os_sem_set(&pcm_sem, 0);
        os_sem_post(&pcm_sem);
    }
}
int wifi_pcm_stream_socket_valid(void)
{
    if (client_socket) {
        return 1;
    }
    return 0;
}

void wifi_pcm_stream_socket_close(void)
{
    if (client_socket) {
        sock_unreg(client_socket);
        client_socket = NULL;
    }
#if 0
    if (server_socket) {
        sock_unreg(server_socket);
        server_socket = NULL;
    }
#endif
    run_sec = TEST_RUN_TIME_SEC;
}
int music_play_res_file(const char *name);
static void wifi_pcm_stream_task(void *priv)
{
    int start_time = 0;
    int start_play = 0;
    int waite_play_end = 0;
    int start_cnt = 0;

    mic_buf = malloc(TEST_PCM_BUFF_SIZE);
    if (mic_buf) {
        cbuf_init(&pcm_cbuf, mic_buf, TEST_PCM_BUFF_SIZE);
    }
#define TMPP_BUF_SIZE   16*1024
    char *tmp_buf = malloc(TMPP_BUF_SIZE);
    os_sem_create(&pcm_sem, 0);
    while (0 == wifi_pcm_stream_socket_create()) {
        printf("wifi_pcm_stream_socket_accept ok \n");
        start_time = timer_get_ms();
        start_play = 0;
        waite_play_end = 0;
        while (!sock_get_error(client_socket)) {
            if (mic_buf) {
                if (cbuf_get_data_size(&pcm_cbuf) < TMPP_BUF_SIZE) {
                    os_sem_pend(&pcm_sem, 3);
                    continue;
                }
                if (client_socket && tmp_buf) {
                    if (cbuf_read(&pcm_cbuf, tmp_buf, TMPP_BUF_SIZE) == TMPP_BUF_SIZE) {
                        if (sock_send(client_socket, tmp_buf, TMPP_BUF_SIZE, 0) != TMPP_BUF_SIZE) {
                            goto exit;
                        }
                        putchar('T');
                    }
                }
            } else {
                puts("err in buf\n");
                break;
            }
            if ((timer_get_ms() - start_time) > 3000 && !start_play && !waite_play_end) {
                start_play = 1;
            }
            if (play_local_file == LOCAL_TEST) {
                if (music_play_stop_status() && start_play) {
                    music_play_res_file("Test.mp3");
                    waite_play_end = 1;
                    start_play = 0;
                }
            } else if (play_local_file == LOCAL_SIN_TEST) {
                if (music_play_stop_status() && start_play) {
                    music_play_res_file("Sin.mp3");
                    waite_play_end = 1;
                    start_play = 0;
                }
            } else if (play_local_file == LOCAL_1TO24K_TEST) {
                if (music_play_stop_status() && start_play) {
                    music_play_res_file("1-24KHz.mp3");
                    waite_play_end = 1;
                    start_play = 0;
                }
            } else if (play_local_file == LOCAL_CYC_TEST) {
                if (music_play_stop_status() && start_play) {
                    char m = start_cnt++ % 5;
                    switch (m) {
                    case 0:
                        music_play_res_file("PdTestLR.mp3");
                        break;
                    case 1:
                        music_play_res_file("Test.mp3");
                        break;
                    case 2:
                        music_play_res_file("Sin.mp3");
                        break;
                    case 3:
                        music_play_res_file("1-24KHz.mp3");
                        break;
                    case 4:
                        music_play_res_file("WhiteNoise.mp3");
                        break;
                    }
                    waite_play_end = 1;
                    start_play = 0;
                }
            }

            if (waite_play_end && music_play_stop_status()) {
                waite_play_end = 0;
                start_play = 1;
            }
        }
exit:
        socket_err = 1;
        sock_unreg(client_socket);
        client_socket = NULL;
        cbuf_clear(&pcm_cbuf);
        socket_err = 0;
        play_local_file = 0;
        printf("client_socket err \n");
        music_play_stop(NULL);
    }
}

void wifi_pcm_stream_task_init(void)
{
    if (!server_socket) {
        thread_fork("wifi_pcm_stream", 12, 1024, 0, 0, wifi_pcm_stream_task, NULL);
    }
}
