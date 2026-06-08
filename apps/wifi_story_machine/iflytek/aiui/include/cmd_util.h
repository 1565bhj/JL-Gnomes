#ifndef _CMD_UTIL_H_
#define _CMD_UTIL_H_

#include <stdlib.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif

enum cmd_status {
    CMD_STATUS_ACKED        = 100,  /* already acked, no need to send respond */

    /* success status */
    CMD_STATUS_SUCCESS_MIN  = 200,
    CMD_STATUS_OK           = 200,  /* command exec success */
    CMD_STATUS_SUCCESS_MAX  = 200,

    /* error status */
    CMD_STATUS_ERROR_MIN    = 400,
    CMD_STATUS_UNKNOWN_CMD  = 400,  /* unknown command */
    CMD_STATUS_INVALID_ARG  = 401,  /* invalid argument */
    CMD_STATUS_FAIL         = 402,  /* command exec failed */
    CMD_STATUS_ERROR_MAX    = 402,
};

/* command format: <command-name> <arg>... */
typedef struct tag_t_cmd_data {
    char *name;
    int (*exec)(int argc, char **argv);
} t_cmd_data;

#ifdef __cplusplus
}
#endif

#endif /* _CMD_UTIL_H_ */

