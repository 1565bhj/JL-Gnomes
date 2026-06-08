#ifndef __AIUI_STDAPI_H__
#define __AIUI_STDAPI_H__

void *aiui_malloc(int size);

void *aiui_calloc(int num, int size);

void *aiui_realloc(void *ptr, int size);

void aiui_free(void *ptr);

#endif // __AIUI_STDAPI_H__
