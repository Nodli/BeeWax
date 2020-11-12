#ifndef H_WINDOW_SDL_GL
#define H_WINDOW_SDL_GL

struct Window_SDL_GL3{
    void initialize(const Window_Settings& settings);
    void terminate();

    void swap_buffers();
    float aspect_ratio();

    // NOTE(hugo):
    // pixel coordinates : origin at the bottom left
    // screen coordinates : OpenGL convention
    vec2 pixel_to_screen_coordinates(ivec2 pixel);
    ivec2 screen_to_pixel_coordinates(vec2 screen);

    // ---- data

    s32 width;
    s32 height;
    SDL_Window* handle;

    SDL_GLContext context;
};

#endif
