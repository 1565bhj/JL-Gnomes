#ifndef IFLY_ALGENGINE_H
#define IFLY_ALGENGINE_H

#define IV_PTR_PREFIX
#define ivPtr       IV_PTR_PREFIX*
//#include "ivAEC.h"
//#include "ivAECParam.h"
//extern "C"
//{

typedef void   *ivPointer;
typedef ivPointer ivHandle;
typedef signed int ivInt32;
typedef short ivData;
/*
typedef struct tagAECParam
{
    void    *pBuffer;
    ivInt32  nSize;
}TAECParam, ivPtr PAECParam;
*/

typedef struct tagAlgParm {
    TAECParam *g_tAECParam;
    ivPointer g_maeHandle;
    unsigned int BlockLen;

    unsigned int filterLen2;
    unsigned int NumMic;
    float *init;
    float *initw;
    float *W;
    float *curDatVec;
    float *Wp;
} St_tagAlgParm;

/*
typedef struct tagdData
{
    ivData l;
    ivData r;
}dData;
*/

int NCInit(St_tagAlgParm* AlgEngine, int MicNumber, int len);
int Ifly_AlgProcess(St_tagAlgParm* AlgEngine, const void* pdata, short* OutPcm, int len);
int NCUnit(St_tagAlgParm* AlgEngine);
void NCGetVersion(char* Version);
//}
#endif
