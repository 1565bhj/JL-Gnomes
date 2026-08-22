#ifndef IFLY_ALGENGINE_H
#define IFLY_ALGENGINE_H

#include "ivDefine.h"

#define MIC_NUM 2
#define FILTER_LEN 10

enum WorkMode {
    WORKMODE_TELEPHONE = 0,
    WORKMODE_MUSIC
};

enum ParamID {
    PARAM_WORK_MODE = 0,
    PARAM_FIX_GAIN
};

//extern "C"
//{
const char *NCGetVersion();
unsigned int NCGetInstSize();
int NCInit(ivHandle handle, int MicNumber, int len);
int Ifly_AlgProcess(ivHandle handle, const void* pdata, short* OutPcm, int len);
int NCSetParam(ivHandle handle, ivUInt32 nParamID, ivCPointer nParamValue);
int NCUnit(ivHandle handle);
//}
#endif
