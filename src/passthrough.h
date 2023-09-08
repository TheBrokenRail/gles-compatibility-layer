#ifdef GLES_COMPATIBILITY_LAYER_USE_SDL
#include <SDL.h>
#else
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define SDL_GL_GetProcAddress glfwGetProcAddress
#endif

#include "log.h"

// Testing
#ifdef GLES_COMPATIBILITY_LAYER_TESTING
typedef void (*test_t)();
extern void add_test(const char *function_name);
#define ADD_TEST(test) \
    __attribute__((constructor)) static void add_##test##_test() { \
        add_test(#test); \
    }
#else
#define ADD_TEST(test)
#endif

// Load GL Function
#if defined(_WIN32) && !defined(_WIN32_WCE) && !defined(__SCITECH_SNAP__)
#define GL_APIENTRY __stdcall
#else
#define GL_APIENTRY
#endif
#define GL_FUNC(name, return_type, args) \
    typedef return_type (GL_APIENTRY *real_##name##_t)args; \
    \
    real_##name##_t real_##name() { \
        static real_##name##_t func = NULL; \
        if (!func) { \
            func = (real_##name##_t) SDL_GL_GetProcAddress(#name); \
            if (!func) { \
                ERR("Error Resolving GL Symbol: " #name); \
            } \
        } \
        return func; \
    } \
    ADD_TEST(name)
