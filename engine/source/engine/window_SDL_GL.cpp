void Window_SDL_GL3::initialize(const Window_Settings& settings){
    SDL_DisplayMode display_mode;

    if(SDL_GetDesktopDisplayMode(0, &display_mode)){
        LOG_ERROR("Failed SDL_GetDesktopDisplayMode() %s", SDL_GetError());
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

    if(settings.sync == Window_Settings::SINGLE_BUFFER_NOSYNC){
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);
    }else if(settings.sync == Window_Settings::DOUBLE_BUFFER_NOSYNC
    || settings.sync == Window_Settings::DOUBLE_BUFFER_VSYNC){
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    }

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
        LOG_ERROR("Failed SDL_CreateWindow() %s", SDL_GetError());
        return;
    }

    // ---- OpenGL after creation

    context = SDL_GL_CreateContext(handle);
    if(!context){
        LOG_ERROR("Failed SDL_GL_CreateContext() %s", SDL_GetError());
        return;
    }

    // NOTE(hugo): synchronize the buffer swap with the monitor refresh rate by
    // waiting in the main thread
    if(settings.sync == Window_Settings::SINGLE_BUFFER_NOSYNC
    || settings.sync == Window_Settings::DOUBLE_BUFFER_NOSYNC){
        SDL_GL_SetSwapInterval(0);
    }else if(settings.sync == Window_Settings::DOUBLE_BUFFER_VSYNC){
        SDL_GL_SetSwapInterval(1);
    }

    // ----

    // NOTE(hugo): ---- settings that do not require to recreate a window
    // TODO(hugo): properly handle the window resize / settings change

    Window_Settings::Mode screen_mode_to_use = settings.mode;

    if(screen_mode_to_use == Window_Settings::WINDOWED){
        SDL_SetWindowResizable(handle, SDL_TRUE);
    }else if(screen_mode_to_use == Window_Settings::BORDERLESS){
        SDL_SetWindowResizable(handle, SDL_FALSE);
        SDL_SetWindowBordered(handle, SDL_FALSE);
    }else if(screen_mode_to_use == Window_Settings::FULLSCREEN){
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
    return {(float)pixel.x / (float)width, (float)pixel.y / (float)height};
}

ivec2 Window_SDL_GL3::screen_to_pixel_coordinates(vec2 screen){
    return {(int)(screen.x * (float)width), (int)(screen.y * (float)height)};
}
