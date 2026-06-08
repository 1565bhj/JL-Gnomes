#ifndef ASR_FN_ASR_H_
#define ASR_FN_ASR_H_

#ifdef __cplusplus
extern "C" {
#endif

// Return value   : 0 - OK, -1 - Error
int  Wanson_ASR_Init();
int Wanson_ASR_Reset(int domain);
int Wanson_ASR_Recog(short* buf, int buf_len, const char** text, int* score);
void Wanson_ASR_Release();

#ifdef __cplusplus
}
#endif

#endif
