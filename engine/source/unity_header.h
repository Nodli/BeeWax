#ifndef H_ENGINE_HEADER_EXTERNALS
#define H_ENGINE_HEADER_EXTERNALS

// ---- precompiler
// REF(hugo): https://blog.kowalczyk.info/article/j/guide-to-predefined-macros-in-c-compilers-gcc-clang-msvc-etc..html

#if defined(_WIN32)
    #define PLATFORM_WINDOWS
    #define AVAILABLE_MULTITHREADING
#elif defined(__linux__)
    #define PLATFORM_LINUX
    #define AVAILABLE_MULTITHREADING
#else
    #error "no platform was specified"
#endif

#if defined(_MSC_VER)
    #define COMPILER_MSVC
#elif defined(__GNUG__)
    #define COMPILER_GCC
#else
    #error "no compiler was identified"
#endif

#if defined(NDEBUG)
    #define RELEASE_BUILD
#else
    #define DEBUG_BUILD
#endif

// ---- C standard library

#if defined(PLATFORM_WINDOWS)
    #define _CRT_SECURE_NO_WARNINGS
#endif

#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <cstddef>
#include <cstdarg>

#include <ctime>

#include <cmath>
#include <climits>

// ---- C++ standard library

// NOTE(hugo): required for placement new ...
#include <new>

// ---- system includes

#if defined(PLATFORM_WINDOWS)
    // NOTE(hugo): https://aras-p.info/blog/2018/01/12/Minimizing-windows.h/
    #define NOMINMAX
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #undef NOMINMAX
    #include <sysinfoapi.h>

    // NOTE(hugo): request dedicated GPU on laptops
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

// ---- compiler includes

//#include <xmmintrin.h>  // SSE & previous
//#include <emmintrin.h>  // SSE2 & previous
//#include <pmmintrin.h>  // SSE3 & previous
//#include <tmmintrin.h>  // SSSE3 & previous
//#include <smmintrin.h>  // SSE4.1 & previous
//#include <nmmintrin.h>  // SSE4.2 & previous

#if defined(COMPILER_MSVC)
    #define AVAILABLE_RDTSC
    #define AVAILABLE_CPUID
    #define AVAILABLE_VECTORIZATION

    #include <intrin.h>
    #pragma intrinsic(_BitScanReverse)
    #pragma intrinsic(_BitScanForward)

#elif defined(COMPILER_GCC)
    #if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
        #define AVAILABLE_RDTSC
        #define AVAILABLE_CPUID
        #define AVAILABLE_VECTORIZATION

        #include <cpuid.h>
        #include <x86intrin.h>
    #endif
#endif

// ---- external libraries

#if defined(LIB_STB)
    #include "stb_image.h"
    #include "stb_image_write.h"
    #include "stb_truetype.h"
    #include "stb_rect_pack.h"
#endif

#if defined(LIB_CJSON)
    #include "cJSON.h"
#endif

#if defined(LIB_FAST_OBJ)
    #include "fast_obj.h"
#endif

#include "SDL.h"
#if defined(RENDERER_VULKAN)
#include "SDL_vulkan.h"
#endif

#if defined(RENDERER_OPENGL3)
    #if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
        #include "gl3w.h"
    #endif
#else
    static_assert(false, "no rendering API was specified");
#endif

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

// ---- beewax

#include "macro.h"
#include "typedef.h"

namespace bw{
    struct Engine;

    #include "tracked_memory.h"
    #include "logprint.h"
    #include "debug_tools.h"
    #include "error_handling.h"

    #include "type.h"
    #include "intrinsics.h"
    #include "utils.h"
    #include "hash.h"
    #include "vmemory.h"

    #include "data_structure.h"
    #include "dense_grid.h"
    #include "algorithm.h"

    #include "sstring.h"
    #include "filepath.h"
    #include "file.h"

    // TODO(hugo):
    //#include "developper_tools.h"

    #include "time_SDL.h"
    #include "frame_timing.h"

    #include "vec.h"
    #include "mat.h"
    #include "rot.h"
    #include "quat.h"
    #include "color.h"

    #include "random_custom.h"
    #include "noise.h"

    #include "geometry.h"

    #include "tween.h"
    #include "indexmap.h"

    #include "camera_math.h"
    #include "camera_2D.h"
    #include "camera_3D.h"

    // --

    #include "input_SDL.h"
    #include "cursor_SDL.h"
    typedef Cursor_SDL Cursor;

    #include "audio_SDL.h"
    typedef Audio_Player_SDL Audio_Player;

    #if defined(RENDERER_OPENGL3)
        #include "GL.h"

        // TODO(hugo): need to cleaup that because of the static definitions

        #include "render_layer_GL3_settings.h"
        #include "render_layer_GL3.h"
        typedef Render_Layer_GL3 Render_Layer;
        typedef Buffer_GL3 Buffer;
        typedef Transient_Buffer_GL3 Transient_Buffer;
        typedef Buffer_Indexed_GL3 Buffer_Indexed;
        typedef Transient_Buffer_Indexed_GL3 Transient_Buffer_Indexed;
        typedef Texture_GL3 Texture;
        typedef Render_Target_GL3 Render_Target;

        #include "window_SDL_GL.h"
        typedef Window_Settings_SDL_GL Window_Settings;
        typedef Window_SDL_GL Window;

    #endif

    // --

    #include "texture.h"
    #include "font.h"

    #include "scene_manager.h"

    //#include "asset_catalog.h"
    //#include "asset_import.h"

    #include "asset_management.h"

    #include "engine.h"
}

#endif
