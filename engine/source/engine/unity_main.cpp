// ---- precompiler

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS
#elif defined(unix) || defined(__unix__) || defined(__unix)
    #define PLATFORM_LINUX
#else
    static_assert(false, "no platform was specified")
#endif

#if defined(_MSC_VER)
    #define COMPILER_MSVC
#elif defined(__GNUG__)
    #define COMPILER_GCC
#else
    static_assert(false, "no compiler was identified")
#endif

#include "macro.h"

// ---- C standard library

#include <cassert>

#include <cstdint>
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
    #include <intrin.h>
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
    #include "byteoperation.h"
    #include "byteoperation.cpp"

    #include "hash.h"
    #include "hash.cpp"

    #include "logprint.h"
    #include "logprint.cpp"
    #include "logprint_typemacro.h"

    #include "sort_search.h"
    #include "data_structure.h"

    // ---- debug

    #include "debug_break.h"

    #define USE_DEVELOPPER_MODE
    #include "developper_tools.h"

    // ---- os layer

    #include "os.h"
    #include "os.cpp"

    // ---- core

    #include "random_custom.h"
    #include "random_custom.cpp"

    #include "time.h"
    #include "frame_timing.h"
    #include "frame_timing.cpp"

    #include "file.h"
    #include "file.cpp"

    #include "vec.h"
    #include "vec.cpp"
    #include "mat.h"
    #include "rot.h"
    #include "quat.h"

    #include "shape.h"
    #include "shape.cpp"

    #include "array_indexing.h"
    #include "array_indexing.cpp"

    #include "camera_math.h"
    #include "camera_math.cpp"
    #include "camera_2D.h"
    #include "camera_2D.cpp"
    #include "camera_3D.h"
    #include "camera_3D.cpp"

    #include "collision_2D.h"
    #include "collision_2D.cpp"

    #include "colormap.h"
    #include "colormap.cpp"

    #include "noise.h"
    #include "noise.cpp"

    // ---- platform layer & rendering

    #include "window_settings.h"

    #if defined(PLATFORM_LAYER_SDL)
        #include "time_SDL.cpp"

        #include "input_SDL.h"
        #include "keyboard_SDL.h"
        #include "mouse_SDL.h"

        #include "sound_SDL.h"

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
}

// ---- unit testing

#include "test.cpp"

// ---- application

//#include "../application/default_main.cpp"

// NOTE(hugo): WIP audio
#if 0
int main(){
    SDL_Init(SDL_INIT_AUDIO);

    // ---- detect

    detect_audio_devices();
    detect_audio_drivers();
    detect_current_audio_driver();

    // ---- initialize

    SDL_AudioSpec expected_spec;
    expected_spec.freq = 22050;
    expected_spec.format = AUDIO_S16;
    expected_spec.channels = 1;
    expected_spec.samples = 4096;
    expected_spec.callback = nullptr;
    //expected_spec.callback = audio_callback;
    expected_spec.userdata = nullptr;

    SDL_AudioSpec device_spec;
    SDL_AudioDeviceID device = SDL_OpenAudioDevice(nullptr, 0, &expected_spec, &device_spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
    if(!device){
        LOG_ERROR("Failed SDL_OpenAudioDevice() %s", SDL_GetError());
    }

    LOG_INFO("Device Audio Specification");
    LOG_INFO("- frequency : %i", device_spec.freq);
    LOG_INFO("- format: %s", (device_spec.format == AUDIO_S16) ? "AUDIO_S16" : "UNKNOWN");
    LOG_INFO("- channels: %i", device_spec.channels);

    // ---- sound loading

    SDL_AudioSpec wav_spec;
    u32 wav_size;
    u8* wav_buffer;
    if(!SDL_LoadWAV("./data/audio/ImperialMarch60.wav", &wav_spec, &wav_buffer, &wav_size)){
        LOG_ERROR("Failed SDL_LoadWAV() %s", SDL_GetError());
    }

    LOG_INFO("Device Audio Specification");
    LOG_INFO("- frequency : %i", wav_spec.freq);
    LOG_INFO("- format: %s", (wav_spec.format == AUDIO_S16) ? "AUDIO_S16" : "UNKNOWN");
    LOG_INFO("- channels: %i", wav_spec.channels);

    LOG_TRACE("%d", SDL_QueueAudio(device, wav_buffer, wav_size));
    //SDL_LockAudioDevice(device);
    //SDL_UnlockAudioDevice(device);

    // ---- start playing

    SDL_PauseAudioDevice(device, 0);

    // ---- update wav file

    //SDL_LockAudioDevice(device);
    //SDL_UnlockAudioDevice(device);

    // ---- termination

    SDL_Delay(10000);

    SDL_FreeWAV(wav_buffer);
    SDL_CloseAudioDevice(device);
    SDL_Quit();

    return 0;
}
#endif
