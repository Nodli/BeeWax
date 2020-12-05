// ---- user defines

#define RENDERER_SETUP_USER "../application/default_main_renderer_setup.h"

// ---- precompiler
// REF(hugo): https://blog.kowalczyk.info/article/j/guide-to-predefined-macros-in-c-compilers-gcc-clang-msvc-etc..html

#if defined(_WIN32)
    #define PLATFORM_WINDOWS
#elif defined(__linux__)
    #define PLATFORM_LINUX
#else
    static_assert(false, "no platform was specified");
#endif

#if defined(_MSC_VER)
    #define COMPILER_MSVC
#elif defined(__GNUG__)
    #define COMPILER_GCC
#else
    static_assert(false, "no compiler was identified");
#endif

#include "macro.h"

// ---- C standard library

#if defined(PLATFORM_WINDOWS)
    #define _CRT_SECURE_NO_WARNINGS
#endif

#include <cassert>

#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <cstddef>
#include <cstdarg>

#include <ctime>

#include <cmath>
#include <climits>

// ---- C++ standard library

// NOTE(hugo): required for placement new...
#include <new>

// ---- system includes

#if defined(PLATFORM_WINDOWS)
    // NOTE(hugo): https://aras-p.info/blog/2018/01/12/Minimizing-windows.h/
    #define NOMINMAX
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #undef NOMINMAX
    #include <sysinfoapi.h>

    // NOTE(hugo): suggests laptop drivers to use a dedicated gpu
    // REF(hugo): https://stackoverflow.com/questions/16823372/forcing-machine-to-use-dedicated-graphics-card
    extern "C" {
        __declspec(dllexport) unsigned int NvOptimusEnablement = 1;
        __declspec(dllexport) unsigned int AmdPowerXpressRequestHighPerformance = 1;
    }

#elif defined(PLATFORM_LINUX)
    #include <sys/mman.h>   // NOTE(hugo): os.h / os.cpp
    #include <unistd.h>     // NOTE(hugo): intrinsics.h
    #include <signal.h>     // NOTE(hugo): debug_break.h
#endif

// ---- force dedicated gpu


// ---- compiler includes

//#include <xmmintrin.h>  // SSE & previous
//#include <emmintrin.h>  // SSE2 & previous
//#include <pmmintrin.h>  // SSE3 & previous
//#include <tmmintrin.h>  // SSSE3 & previous
//#include <smmintrin.h>  // SSE4.1 & previous
//#include <nmmintrin.h>  // SSE4.2 & previous

#if defined(COMPILER_MSVC)
    #include <intrin.h>
    #pragma intrinsic(_BitScanReverse)
    #pragma intrinsic(_BitScanForward)
#elif defined(COMPILER_GCC)
    #include <cpuid.h>
    #include <x86intrin.h>
#endif

// ---- external libraries

#if defined(LIB_STB)
    #include "stb_image.h"
    #include "stb_image_write.h"
    #include "stb_truetype.h"
#endif

#if defined(PLATFORM_LAYER_SDL)
    #include "SDL.h"
#else
    static_assert(false, "no platform layer was specified");
#endif

#if defined(RENDERER_OPENGL3)
    #include "gl3w.h"
#elif defined(RENDERER_VULKAN) && defined(PLATFORM_LAYER_SDL)
    #include "SDL_vulkan.h"
    #include "vulkan.h"
    #include "vulkan_core.h"
#else
    static_assert(false, "no rendering API was specified");
#endif

// ----

namespace bw{
    #include "typedef.h"

    #include "type.h"
    #include "type.cpp"

    #include "intrinsics.h"
    #include "intrinsics.cpp"

    #include "constexpr.h"
    #include "utils.h"

    #include "byteoperation.h"
    #include "byteoperation.cpp"

    #include "hash.h"
    #include "hash.cpp"

    #if defined(PLATFORM_LINUX)
        #define LOG_COLORED
    #endif
    #include "logprint.h"
    #include "logprint.cpp"
    #include "logprint_typemacro.h"

    #include "sstring.h"

    #include "sort_search.h"
    #include "data_structure.h"

    #include "filepath.h"
    #include "filepath.cpp"

    #include "file.h"
    #include "file.cpp"

    // ---- debug

    #define DEVELOPPER_MODE
    #include "debug_break.h"
    #include "developper_tools.h"

    // ---- os layer

    #include "os.h"
    #include "os.cpp"

    // ---- core

    #include "array_indexing.h"
    #include "array_indexing.cpp"

    #include "time.h"

    #include "frame_timing.h"
    #include "frame_timing.cpp"

    #include "vec.h"
    #include "vec.cpp"
    #include "mat.h"
    #include "rot.h"
    #include "quat.h"

    #include "random_custom.h"
    #include "random_custom.cpp"

    #include "dense_grid.h"

    #include "shape_2D.h"
    #include "shape_2D.cpp"

    #include "collision_2D.h"
    #include "collision_2D.cpp"

    #include "camera_2D.h"
    #include "camera_2D.cpp"

    #include "shape_3D.h"
    #include "shape_3D.cpp"

    #include "camera_math.h"
    #include "camera_math.cpp"

    #include "camera_3D.h"
    #include "camera_3D.cpp"

    #include "colormap.h"
    #include "colormap.cpp"

    #include "noise.h"
    #include "noise.cpp"

    #include "tweening.h"

    // ---- platform layer & rendering

    #include "window_settings.h"

    #if defined(PLATFORM_LAYER_SDL)
        #include "time_SDL.cpp"

        #include "input_SDL.h"
        #include "keyboard_SDL.h"
        #include "mouse_SDL.h"

        #include "audio_SDL.h"
        #include "audio_SDL.cpp"

        #if defined(RENDERER_OPENGL3)
            #include "GL.h"
            #include "GL.cpp"

            #include "window_SDL_GL.h"
            #include "window_SDL_GL.cpp"
            typedef Window_SDL_GL3 Window;

            #include "renderer_setup.h"
            #include "renderer_GL3.h"
            #include "renderer_GL3.cpp"
            typedef Renderer_GL3 Renderer;

        #elif defined(RENDERER_VULKAN)
            #include "VK.h"

            #include "window_SDL_VK.h"
            #include "window_SDL_VK.cpp"
            typedef Window_SDL_VK Window;
        #endif
    #endif

    // ---- additional structures

    #include "font.h"
    #include "font.cpp"

    #include "texture_animation.h"
    #include "texture_animation.cpp"

    #include "particle.h"
    #include "particle.cpp"

    #include "asset_manager.h"
    #include "asset_manager.cpp"

    #include "scene_manager.h"
    #include "scene_manager.cpp"
}

// ---- testing

//#include "test.cpp"
#include "../application/default_main.cpp"

// ---- easy setup

//#include "easy_setup.h"
//#include "easy_setup.cpp"
//#include "../application/minijam67_void.cpp"
