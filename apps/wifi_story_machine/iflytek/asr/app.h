#ifndef __CAE_1_APP_H__
#define __CAE_1_APP_H__

//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
#include "MiddleErrorCode.h"

#define VTN_INIT     (1)
#define VTN_UNINIT   (0)

// �׳����ѽ���Ļص�
typedef int(*ivw_res_cb)(const char*, int, void*);
// ���ʶ����Ƶ�Ļص�
typedef int(*iat_audio_cb)(const char*, int);

#endif