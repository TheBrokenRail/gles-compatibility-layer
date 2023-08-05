#pragma once

#include <stdlib.h>

#ifdef ANDROID
#include <android/log.h>

#define DEBUG(...) { ((void) __android_log_print(ANDROID_LOG_VERBOSE, "GL DEBUG", __VA_ARGS__)); }
#define ERR(...) { ((void) __android_log_print(ANDROID_LOG_ERROR, "GL ERROR", __VA_ARGS__)); exit(EXIT_FAILURE); }
#else
#include <stdio.h>

#define DEBUG(format, ...) { printf("GL DEBUG: " format "\n", ##__VA_ARGS__); }
#define ERR(format, ...) { printf("GL ERROR: (%s:%i): " format "\n", __FILE__, __LINE__, ##__VA_ARGS__); exit(EXIT_FAILURE); }
#endif

#define ALLOC_CHECK(obj) \
    { \
        if (obj == NULL) { \
            ERR("Memory Allocation Failed"); \
        } \
    }
