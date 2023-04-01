#ifndef __PULL_H__
#define __PULL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(MINGW32) && defined(BUILD_EXPORT)
    #ifdef ALG_SDK_EXPORT
        #define ALG_SDK_API __declspec(dllexport)
    #else
        #define ALG_SDK_API __declspec(dllimport)
    #endif // ALG_SDK_EXPORT
#else
    #define ALG_SDK_API extern
#endif // MINGW32

typedef void(*alg_sdk_callback_func)(void* p);

ALG_SDK_API int  alg_sdk_notify(alg_sdk_callback_func consumer);
ALG_SDK_API void alg_sdk_notify_spin_on(void);
ALG_SDK_API int  alg_sdk_stop_notify(void);

#ifdef __cplusplus
}
#endif
#endif
