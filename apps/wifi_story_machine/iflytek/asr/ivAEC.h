/*----------------------------------------------+
 |                                              |
 |  ivAEC.h - AEC 1.0 API                           |
 |                                              |
 |      Copyright (c) 1999-2013, iFLYTEK Ltd.   |
 |      All rights reserved.                    |
 |                                              |
 +----------------------------------------------*/

#if !defined(AEC_TEAM__2013_5_22__AEC__H)
#define AEC_TEAM__2013_5_22__AEC__H

#include "ivErrorCode.h"

/* Definition of AEC parameters and parameter value */

/* AEC out write mode parameters and parameter value */
#define AEC_AES_OUT_WRITE_MODE              (32)
#define OUT_WRITE_MODE_MONO             (0)     // just get 128 samples of AEC output.( AECOut0 AECOut1   AECout127 )
#define OUT_WRITE_MODE_STEREO_ECHO      (1)     // get 128 samples of AEC output and 128 samples.( AECOut0 Mic0 AECOut1 Mic1   AECout127 Mic127 )
#define OUT_WRITE_MODE_STEREO_REF       (2)     // get 128 samples of AEC output and 128 samples.( AECOut0 Ref0 AECOut1 Ref1   AECout127 Ref127 )

/* AEC work mode parameters and parameter value */
#define AEC_ECHO_CANCEALLATION_MODEL        (33)
#define WORK_MODE_TELEPHONE             (0)     // Telephone Model
#define WORK_MODE_MUSIC                 (1)     // Music Model

/*

*   Interface
*/

typedef struct tagAECParam {
    void    *pBuffer;
    ivInt32  nSize;
} TAECParam, ivPtr PAECParam;

typedef short ivData;
typedef struct tagdData {
    ivData l;
    ivData r;
} dData;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Create MAE object */
ivStatus                                /* Returned Error Info */
ivCall AECCreate(
    ivHandle ivPtr phAecObj,            /* To Receive the AEC object handle */
    PAECParam pAECParam
);

/* Append Audio data to the AEC object,In general, Call this function in record thread */
ivStatus                                /* Returned Error Info */
ivCall AECAppendAudioData(
    ivHandle    hAecObj,            /* The AEC object handle */
    ivCPointer  pAudioDataL,            /* [In] Input Audio data buffer of Mic1 */
    ivCPointer  pAudioDataR,            /* [In] Input Audio data buffer of Mic2*/
    ivInt16     nSamples            /* Specifies the length, in samples, of Audio data */
);

/* Start process */
ivStatus                                /* Returned Error Info */
ivCall AECRunStep(
    ivHandle        hAecObj,        /* The AEC object handle */
    ivPointer       pOutData,       /* [Out]Out Audio data buffer of Enhance */
    ivPInt16        pnSamples);     /* Specifies the length, in samples, of Out Audio data */

ivStatus
ivCall AECReset(ivHandle hAecObj);

ivStatus                                /* Returned Error Info */
ivCall AECSetParam(
    ivHandle    hAecObj,            /* The AEC object handle */
    ivUInt32    nParamID,           /* Parameter ID */
    ivCPointer  nParamValue         /* Parameter Value */
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !defined(AEC_TEAM__2013_5_22__AEC__H) */
