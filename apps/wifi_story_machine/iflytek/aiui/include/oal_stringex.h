/*****************************************************************************
* Project:
* All project
* Copyright (C) 2008-2018, ASC Ltd.
* All Rights Reserved.
* Description:
* All code is genrated by ASC-yangyang
* Author:
* yangyang
****************************************************************************/
#ifndef _OAL_STRINGEX_H_
#define _OAL_STRINGEX_H_

#include "oal_type.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// String
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char *OAL_strdup(const char *str);
char *OAL_strndup(const char *str, int n);
OAL_S32 OAL_strcpyLine(char *str1, char *str2);
OAL_S32 OAL_strlenLine(char *str);
char *OAL_strnstr(const char * src, long src_len, const char * substr, long substr_len);
int OAL_natoi(const char *str, int len);
char *OAL_itoa(int value, char *buffer, int radix);
int OAL_randrange(int range);
char *OAL_strsep(char **stringp, const char *delim);
char *OAL_pathExtName(char *path, int *extnamelen);
OAL_BOOL OAL_strmatch(char* str1, char* pattern);
int OAL_hex2dec(char *hex);
char *OAL_strcasestr(const char* haystack, const char* needle);
char *OAL_ptrtoa(void *ptr, char *buffer);
char *OAL_random_uuid(char buf[37]);
OAL_S32 OAL_strncasecmp(char* str, char* str1, int len);
OAL_S32 OAL_strcasecmp(char* str, char* str1);

// for debug print_to_screen
void OAL_printfBuffer(OAL_U8 *pData, OAL_U32 len);
void OAL_printfBuffer16(unsigned char *pData, int len);
void OAL_printfBuffer32(unsigned char *pData, int len);

// tools
char OAL_ToLowerCase(char ch);
OAL_BOOL OAL_IsAlpha(char ch);
OAL_U32 OAL_StrTo4CC(const char *str, const int strlen);
void OAL_printf4CC(int v4int);
int OAL_GetLines(char *txt_buffer, int txt_size, char **lines);
int OAL_base64_decode(const char * base64, unsigned char * bindata);

#endif


