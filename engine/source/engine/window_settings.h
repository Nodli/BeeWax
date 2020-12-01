#ifndef H_WINDOW_SETTINGS
#define H_WINDOW_SETTINGS

struct Window_Settings{
    enum : s32 {
        mode_windowed = 0,
        mode_borderless = 1,
        mode_fullscreen = 2,

        synchronize_none = 0,
        synchronize_vertical = 1,
        synchronize_adaptive = -1,

        buffering_single = 0,
        buffering_double = 1
    };

    s32 width = 0;
    s32 height = 0;
    const char* name = nullptr;
    s32 mode = mode_windowed;
    s32 synchronization = synchronize_adaptive;
    s32 buffering = buffering_double;
};

#endif
