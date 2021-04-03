// ---- testing / samples defines

//#define RENDERER_GL3_USER_SETTINGS "../sample/integration_GL3_setup.h"

// ---- precompiler
// REF(hugo): https://blog.kowalczyk.info/article/j/guide-to-predefined-macros-in-c-compilers-gcc-clang-msvc-etc..html

#if defined(_WIN32)
    #define PLATFORM_WINDOWS
    #define AVAILABLE_MULTITHREADING
#elif defined(__linux__)
    #define PLATFORM_LINUX
    #define AVAILABLE_MULTITHREADING
#elif defined(__EMSCRIPTEN__)
    #define PLATFORM_EMSCRIPTEN
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

#include "macro.h"

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
    #elif defined(PLATFORM_EMSCRIPTEN)
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

#if defined(PLATFORM_EMSCRIPTEN)
    #include <emscripten.h>
#endif

#if defined(PLATFORM_LAYER_SDL)
    #include "SDL.h"
    #if defined(RENDERER_VULKAN)
        #include "SDL_vulkan.h"
    #endif
#else
    static_assert(false, "no platform layer was specified");
#endif

#if defined(RENDERER_OPENGL3)
    #if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
        #define OPENGL_DESKTOP
        #include "gl3w.h"
    #elif defined(PLATFORM_EMSCRIPTEN)
        #define OPENGL_ES
        #include <GLES3/gl3.h>
    #endif
#elif defined(RENDERER_VULKAN) && defined(PLATFORM_LAYER_SDL)
    #include "vulkan.h"
    #include "vulkan_core.h"
#else
    static_assert(false, "no rendering API was specified");
#endif

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

// ----

namespace bw{
    #include "typedef.h"

    #include "logprint.h"
    #include "logprint.cpp"

    #include "debug_tools.h"
    #include "error_handling.h"

    #include "type.h"
    #include "type.cpp"

    #include "intrinsics.h"
    #include "intrinsics.cpp"

    #include "utils.h"
    #include "utils.cpp"

    #include "hash.h"
    #include "hash.cpp"

    #include "vmemory.h"
    #include "vmemory.cpp"

    #include "sstring.h"

    #include "data_structure.h"
    #include "component_storage.h"
    #include "sort_search.h"

    #include "filepath.h"
    #include "filepath.cpp"

    #include "file.h"
    #include "file.cpp"

    // ---- debug

    #include "developper_tools.h"

    // ----

    #include "time.h"

    #include "frame_timing.h"
    #include "frame_timing.cpp"

    #include "vec.h"
    #include "vec.cpp"
    #include "mat.h"
    #include "rot.h"
    #include "quat.h"

    #include "color.h"
    #include "color.cpp"

    #include "random_custom.h"
    #include "random_custom.cpp"

    #include "dense_grid.h"

    #include "collision.h"
    #include "collision.cpp"

    #include "geometry.h"
    #include "geometry.cpp"

    #include "camera_math.h"
    #include "camera_math.cpp"

    #include "camera_2D.h"
    #include "camera_2D.cpp"

    #include "camera_3D.h"
    #include "camera_3D.cpp"

    #include "noise.h"
    #include "noise.cpp"

    #include "tween.h"

    // ---- platform layer & rendering

    #if defined(PLATFORM_LAYER_SDL)
        #include "time_SDL.cpp"

        #include "input_SDL.h"
        #include "keyboard_SDL.h"
        #include "mouse_SDL.h"
        #include "cursor_SDL.h"
        typedef Device_Button_SDL Device_Button;
        typedef Keyboard_State_SDL Keyboard_State;
        typedef Mouse_State_SDL Mouse_State;
        typedef Cursor_SDL Cursor;

        #include "audio_SDL.h"
        #include "audio_SDL.cpp"
        typedef Audio_Player_SDL Audio_Player;

        #if defined(RENDERER_OPENGL3)
            #include "GL.h"
            #include "GL.cpp"

            #include "renderer_GL3_settings.h"
            #include "renderer_GL3.h"
            #include "renderer_GL3.cpp"
            typedef Renderer_GL3 Renderer;
            typedef Transient_Buffer_GL3 Transient_Buffer;
            typedef Transient_Buffer_Indexed_GL3 Transient_Buffer_Indexed;
            typedef Texture_GL3 Texture;
            typedef Render_Target_GL3 Render_Target;

            #include "window_SDL_GL.h"
            #include "window_SDL_GL.cpp"
            typedef Window_Settings_SDL_GL Window_Settings;
            typedef Window_SDL_GL Window;

        #elif defined(RENDERER_VULKAN)
            #include "VK.h"
            #include "VK.cpp"

            #include "window_SDL_VK.h"
            #include "window_SDL_VK.cpp"
            typedef Window_Settings_SDL_VK Window_Settings;
            typedef Window_SDL_VK Window;
        #endif
    #endif

    // ---- additional structures

    #include "texture.h"
    #include "texture.cpp"

    #include "vector_graphics.h"
    #include "vector_graphics.cpp"

    //#include "texture_animation.h"
    //#include "texture_animation.cpp"

    //#include "particle.h"
    //#include "particle.cpp"

    #include "scene_manager.h"
    #include "scene_manager.cpp"

    #include "asset_catalog.h"
    #include "asset_catalog.cpp"
    #include "asset_import.h"
    #include "asset_import.cpp"
}

// ---- sample

//#include "../sample/unit.cpp"
//#include "../sample/integration_easy_setup.cpp"

// ---- application

//#include "../application/GL4_3D.cpp"

#include "../application/editor/main.cpp"

//#include "../application/diffusion.cpp"
//#include "../application/reaction_diffusion.cpp"
//#include "../application/physarum.cpp"

//#include "../application/VK.cpp"

//#include "../application/minijam67_void.cpp"

// ----
