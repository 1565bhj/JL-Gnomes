#ifndef __W_IVW_H__
#define __W_IVW_H__

#include "param_module.h"
//#include "basic_defines.h"
#include "ivw_type.h"
#include "ivw_version.h"


typedef void *WIVW_INST;

/*
typedef struct WakeUpResult
{
    ivInt iFrameStart_;
    ivInt nFrameDuration_;
    ivInt nFillerScore_;
    ivInt nKeyWordScore_;

    ivInt nCM_Thresh_;
    ivInt nCM_;
    ivInt iResID_;
    char  pSzLabel_[64];
}TWakeUpResult;
*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define USE_EXT_MEM
//void  ivw_init(WIVW_INST p_ivw_instance);
void  ivw_init(WIVW_INST p_ivw_instance, const char* mlp_resource, const char* keywords_resource);
ivInt wIvwDestroy(WIVW_INST wIvwInst);
ivInt wIvwSetParameter(WIVW_INST wIvwInst, PARAM param, ivInt value);
ivInt wIvwGetParameter(WIVW_INST wIvwInst, PARAM param, ivInt *value);
ivInt wIvwRegisterCallBacks(WIVW_INST wIvwInst, CallBackType FuncType, const PIVWCallBack pFunc, void *pUserParam);
ivInt wIvwUnRegisterCallBacks(WIVW_INST wIvwInst, CallBackType FuncType);
ivInt wIvwStart(WIVW_INST wIvwInst);
ivInt wIvwStop(WIVW_INST wIvwInst);
ivInt wIvwWrite(WIVW_INST wIvwInst, const short *samples, ivInt len);
ivInt wIvwGetLastWakeupRst(WIVW_INST wIvwInst, void *rst);
ivChar *wIvwGetVersion(void);
ivInt wIvwFlush(WIVW_INST wIvwInst);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__W_IVW_H__

