#ifndef __HTTP_UPDATE_H__
#define __HTTP_UPDATE_H__

int get_update_data(const char *url);

/* Start an asynchronous HTTP OTA using a URL copied from the caller. */
int http_ota_init_with_url(const char *url);

#endif

