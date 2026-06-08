#ifndef _IFLY_AIUI_WEBSOCKET_H
#define _IFLY_AIUI_WEBSOCKET_H

#include "ifly_aiui_net.h"

int ifly_aiui_websockets_connect(t_ifly_websockt_handle *pp_ifly_ws_handle, char *pp_url, u8 *port, char *pp_orgin_str, t_p_fun_ws_rcv_cb *p_cb);
int ifly_aiui_websockets_client_send(t_ifly_websockt_handle *pp_ifly_ws_handle, u8 *buf, int len, char type);
int ifly_aiui_websockets_client_send_text(t_ifly_websockt_handle *pp_ifly_ws_handle, const char *pp_text);
int ifly_aiui_websockets_client_send_bin(t_ifly_websockt_handle *pp_ifly_ws_handle, const char *pp_dat, int vp_len);
void ifly_aiui_websockets_disconnect(t_ifly_websockt_handle *pp_ifly_ws_handle);



#endif
