#ifndef __AIUI_SOCKET_API_H__
#define __AIUI_SOCKET_API_H__

#define AIUI_SOCKET int
#define AIUI_INVALID_SOCKET (-1)

AIUI_SOCKET aiui_socket_connect(const char *address, int port);
void aiui_socket_close(AIUI_SOCKET sock);

int aiui_socket_send(AIUI_SOCKET sock, const char *buf, const size_t len);
int aiui_socket_recv(AIUI_SOCKET sock, char *buf, const size_t len);
int aiui_connect_is_ok(AIUI_SOCKET sock);

#endif // __AIUI_SOCKET_API_H__