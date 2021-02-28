void Window_SDL_GL::initialize(const Window_Settings_SDL_GL& settings){
    SDL_DisplayMode display_mode;

    SDL_CHECK(SDL_GetDesktopDisplayMode(0, &display_mode) == 0);

    assert(settings.width <= display_mode.w);
    assert(settings.height <= display_mode.h);

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

    ENGINE_CHECK(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, settings.OpenGL_major) == 0, "FAILED SDL_GL_CONTEXT_MAJOR_VERSION");
    ENGINE_CHECK(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, settings.OpenGL_minor) == 0, "FAILED SDL_GL_CONTEXT_MINOR_VERSION");

#if defined(OPENGL_DESKTOP)
    ENGINE_CHECK(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) == 0, "FAILED SDL_GL_CONTEXT_PROFILE_MASK");
#else
    ENGINE_CHECK(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES) == 0, "FAILED SDL_GL_CONTEXT_PROFILE_MASK");
;
#endif

#if defined(DEBUG_BUILD)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

    ENGINE_CHECK(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) == 0, "FAILED SDL_GL_DOUBLEBUFFER");
    ENGINE_CHECK(SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1) == 0, "FAILED SDL_GL_FRAMEBUFFER_SRGB_CAPABLE");

    // ----

    const char* window_name = "window_SDL_GL";
    if(settings.name){
        window_name = settings.name;
    }

    handle = SDL_CreateWindow(window_name,
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            width, height,
            SDL_WINDOW_OPENGL);
    SDL_CHECK(handle);

    ID = SDL_GetWindowID(handle);

    // ---- OpenGL after creation

    context = SDL_GL_CreateContext(handle);
    SDL_CHECK(context);

    // NOTE(hugo): synchronize the buffer swap with the monitor refresh rate by
    // waiting in the main thread
    s32 sync_success = SDL_GL_SetSwapInterval(settings.synchronization);

    if(sync_success && settings.synchronization == Window_Settings_SDL_GL::synchronize_adaptive){
        sync_success = SDL_GL_SetSwapInterval(Window_Settings_SDL_GL::synchronize);
    }
    if(sync_success
    && (settings.synchronization == Window_Settings_SDL_GL::synchronize_adaptive || settings.synchronization == Window_Settings_SDL_GL::synchronize)){
        sync_success = SDL_GL_SetSwapInterval(Window_Settings_SDL_GL::synchronize_none);
    }
    if(sync_success){
        LOG_ERROR("SDL_GL_SetSwapInterval FAILED - %s", SDL_GetError());
    }

    // ----

    // NOTE(hugo): ---- settings that do not require to recreate a window

    SDL_SetWindowResizable(handle, SDL_TRUE);
    if(settings.mode == Window_Settings_SDL_GL::mode_borderless){
        SDL_SetWindowBordered(handle, SDL_FALSE);
    }else if(settings.mode == Window_Settings_SDL_GL::mode_fullscreen){
        SDL_SetWindowFullscreen(handle, SDL_TRUE);
    }

    SDL_RaiseWindow(handle);
}

void Window_SDL_GL::terminate(){
    if(context){
        SDL_GL_DeleteContext(context);
    }

    if(handle){
        SDL_DestroyWindow(handle);
    }
}

float Window_SDL_GL::aspect_ratio(){
    return (float)width / (float)height;
}

Render_Target_GL3 Window_SDL_GL::render_target(){
    return {(u32)width, (u32)height, 0u, 0u, 0u};
}

vec2 Window_SDL_GL::pixel_to_screen_coordinates(ivec2 pixel){
    assert(width > 0u && height > 0u);
    return {
        ((pixel.x + 0.5f) / (float)(width)) * 2.f - 1.f,
        ((height - (pixel.y + 0.5f)) / (float)(height)) * 2.f - 1.f
    };
}

ivec2 Window_SDL_GL::screen_to_pixel_coordinates(vec2 screen){
    return {
        (int)floor((screen.x + 1.f) * 0.5f * (float)width - 0.5f),
        (int)floor((screen.y + 1.f) * 0.5f * (float)height * - 1.f - 0.5f + height)
    };
}

void Window_SDL_GL::register_event(SDL_Event& event){
    if(event.type == SDL_WINDOWEVENT && event.window.windowID == ID){
        switch(event.window.event){
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                width = event.window.data1;
                height = event.window.data2;
                break;
            case SDL_WINDOWEVENT_CLOSE:
                event.type = SDL_QUIT;
                SDL_PushEvent(&event);
                break;
            default:
                break;
        }
    }
}

void Window_SDL_GL::swap_buffers(){
    SDL_GL_SwapWindow(handle);
}
