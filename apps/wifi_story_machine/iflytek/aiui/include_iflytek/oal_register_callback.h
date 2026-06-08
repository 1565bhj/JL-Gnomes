#ifndef _OAL_REGISTER_CALLBACK_H_
#define _OAL_REGISTER_CALLBACK_H_

#include "oal_type.h"

OAL_HANDLE OAL_register_callbacks_create();
void OAL_register_callback(OAL_HANDLE inst, int id, OAL_CALL_FUNCTION pfunc);
void OAL_register_callbacks_do(OAL_HANDLE inst, int id, int wParam, int lParam, void *data, int data_len);
void OAL_register_callbacks_destroy(OAL_HANDLE inst);

#endif


