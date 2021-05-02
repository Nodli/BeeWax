#ifndef H_WINDOW_SDL_GL
#define H_WINDOW_SDL_GL

struct Window_Settings_SDL_GL{
    enum Settings : s32 {
        mode_windowed = 0,
        mode_borderless = 1,
        mode_fullscreen = 2,

        synchronize_none = 0,
        synchronize = 1,
        synchronize_adaptive = -1,

        size_control_none = 0,
        size_control_allowed = 1,
    };

    s32 width = 0;
    s32 height = 0;

    const char* name = nullptr;
    s32 mode = mode_windowed;
    s32 synchronization = synchronize_adaptive;
    s32 size_control = size_control_none;

    u32 OpenGL_major = 0u;
    u32 OpenGL_minor = 0u;
};

struct Window_SDL_GL{
    void create(const Window_Settings_SDL_GL& settings);
    void destroy();

    float aspect_ratio();
    Render_Target_GL3 render_target();

    // NOTE(hugo):
    // * pixel coordinates  (SDL)    : origin at the top left    ; ([0; window_width], [0; window_height])
    // * screen coordinates (OpenGL) : origin at the bottom left ; ([-1.; 1.], [-1.; 1.])
    vec2 pixel_to_screen_coordinates(ivec2 pixel);
    ivec2 screen_to_pixel_coordinates(vec2 screen);

    bool register_event(SDL_Event& event);

    void swap_buffers();

    // ---- data

    s32 width;
    s32 height;
    SDL_Window* handle;
    u32 ID;
    SDL_GLContext context;
};

#endif
