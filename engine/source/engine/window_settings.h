#ifndef H_WINDOW_SETTINGS
#define H_WINDOW_SETTINGS

struct Window_Settings{
    enum Mode {WINDOWED, FULLSCREEN, BORDERLESS};
    enum Synchronization {SINGLE_BUFFER_NOSYNC, DOUBLE_BUFFER_NOSYNC, DOUBLE_BUFFER_VSYNC};

    s32 width = 0;
    s32 height = 0;
    const char* name = nullptr;
    Mode mode = WINDOWED;
    Synchronization sync = DOUBLE_BUFFER_VSYNC;
};

#endif
