void Window_SDL_GL3::initialize(const Window_Settings& settings){
    SDL_DisplayMode display_mode;

    if(SDL_GetDesktopDisplayMode(0, &display_mode)){
        LOG_ERROR("SDL_GetDesktopDisplayMode() FAILED - %s", SDL_GetError());
    }

    assert(!(settings.width > display_mode.w));
    assert(!(settings.height > display_mode.h));

    if(settings.width){
        width = settings.width;
    }else{
        width = display_mode.w;
    }

    if(settings.height){
        height = settings.height;
    }else{
        height = display_mode.h;
    }

    // ---- OpenGL before creation

    constexpr u32 version_OpenGL_major = 3u;
    constexpr u32 version_OpenGL_minor = 3u;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, version_OpenGL_major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, version_OpenGL_minor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, settings.buffering);

    // ----

    const char* window_name = "window_SDL_GL";
    if(settings.name){
        window_name = settings.name;
    }

    handle = SDL_CreateWindow(window_name,
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            width, height,
            SDL_WINDOW_OPENGL);
    if(!handle){
        LOG_ERROR("SDL_CreateWindow() FAILED - %s", SDL_GetError());
        return;
    }

    // ---- OpenGL after creation

    context = SDL_GL_CreateContext(handle);
    if(!context){
        LOG_ERROR("SDL_GL_CreateContext() FAILED - %s", SDL_GetError());
        return;
    }

    // NOTE(hugo): synchronize the buffer swap with the monitor refresh rate by
    // waiting in the main thread
    s32 sync_success = SDL_GL_SetSwapInterval(settings.synchronization);
    if(sync_success && settings.synchronization == Window_Settings::synchronize_adaptive){
        sync_success = SDL_GL_SetSwapInterval(Window_Settings::synchronize_vertical);
    }
    if(sync_success
    && (settings.synchronization == Window_Settings::synchronize_adaptive || settings.synchronization == Window_Settings::synchronize_vertical)){
        sync_success = SDL_GL_SetSwapInterval(Window_Settings::synchronize_none);
    }
    if(sync_success){
        LOG_ERROR("SDL_GL_SetSwapInterval FAILED - %s", SDL_GetError());
    }

    // ----

    // NOTE(hugo): ---- settings that do not require to recreate a window

    if(settings.mode == Window_Settings::mode_windowed){
        SDL_SetWindowResizable(handle, SDL_FALSE);
    }else if(settings.mode == Window_Settings::mode_borderless){
        SDL_SetWindowResizable(handle, SDL_FALSE);
        SDL_SetWindowBordered(handle, SDL_FALSE);
    }else if(settings.mode == Window_Settings::mode_fullscreen){
        SDL_SetWindowFullscreen(handle, SDL_TRUE);
    }

    SDL_RaiseWindow(handle);
}

void Window_SDL_GL3::terminate(){
    if(context){
        SDL_GL_DeleteContext(context);
    }

    if(handle){
        SDL_DestroyWindow(handle);
    }
}

void Window_SDL_GL3::swap_buffers(){
    SDL_GL_SwapWindow(handle);
}

float Window_SDL_GL3::aspect_ratio(){
    return (float)width / (float)height;
}

vec2 Window_SDL_GL3::pixel_to_screen_coordinates(ivec2 pixel){
    return {
        ((float)pixel.x / (float)width) * 2.f - 1.f,
        ((float)pixel.y / (float)height) * 2.f - 1.f
    };
}

ivec2 Window_SDL_GL3::screen_to_pixel_coordinates(vec2 screen){
    return {
        (int)((screen.x + 1.f) * 0.5f * (float)width),
        (int)((screen.y + 1.f) * 0.5f * (float)height)
    };
}
