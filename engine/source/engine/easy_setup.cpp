void Engine::setup(){

    // ---- externals

    SDL_CHECK(SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) == 0);
    setup_vmemory();
    setup_timer();
    setup_LOG();
    stbi_set_flip_vertically_on_load(true);

    // ---- input

    keyboard.initialize();
    mouse.initialize();

    // ---- frame timing

    frame_timing.initialize(timer_ticks(), timer_frequency(), 60u, 0.05f, 2u, 60u);

    // ---- window

    Window_Settings window_settings;
    window_settings.width = g_config::window_width;
    window_settings.height = g_config::window_height;
    window_settings.name = g_config::window_name;
    window_settings.mode = Window_Settings::mode_windowed;
    window_settings.synchronization = Window_Settings::synchronize;
    window_settings.OpenGL_major = 3u;
#if defined(OPENGL_DESKTOP)
    window_settings.OpenGL_minor = 3u;
#else
    window_settings.OpenGL_minor = 0u;
#endif

    window.initialize(window_settings);

    // ---- renderer

#if defined(OPENGL_DESKTOP)
    gl3wInit();
#endif

    DEV_Debug_Renderer;

    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

    glEnable(GL_FRAMEBUFFER_SRGB);
    glClearColor(0.5f, 0.5f, 0.5f, 1.f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glCullFace(GL_BACK);
    //glEnable(GL_CULL_FACE);

    renderer.setup();

    offscreen_target = renderer.get_render_target(window.width, window.height);

    // ---- imgui

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();
    ImGui::SetColorEditOptions(
            ImGuiColorEditFlags_None
            | ImGuiColorEditFlags_NoLabel
            | ImGuiColorEditFlags_AlphaBar
            | ImGuiColorEditFlags_AlphaPreviewHalf
            | ImGuiColorEditFlags_DisplayHSV
            | ImGuiColorEditFlags_PickerHueWheel
    );

    ImGui_ImplSDL2_InitForOpenGL(window.handle, window.context);
    ImGui_ImplOpenGL3_Init(GLSL_version);

    // ---- audio

    audio.setup();

    // ---- asset

    import_asset_catalog_from_json(g_config::asset_catalog_path,
        &audio_catalog,
        &texture_catalog,
        &audio,
        &renderer);

    // ---- dev tools

    //DEV_setup();
}

void Engine::terminate(){
    // ---- dev tools

    DEV_terminate();

    // ---- engine

    scene.terminate();

    {
        Asset_Catalog_Bucket* ptr = audio_catalog.head;
        while(ptr){
            free_audio_asset((Audio_Asset*)asset_from_bucket(ptr));
            ptr = ptr->next;
        }
    }
    {
        Asset_Catalog_Bucket* ptr = texture_catalog.head;
        while(ptr){
            free_texture_asset((Texture_Asset*)asset_from_bucket(ptr), &renderer);
            ptr = ptr->next;
        }
    }
    audio_catalog.terminate();

    ImGui::DestroyContext();

    audio.terminate();
    renderer.free_render_target(offscreen_target);
    renderer.terminate();

    window.terminate();

    // ---- external

    SDL_Quit();
}

Engine_Code Engine::process_event(){
    keyboard.new_frame();
    mouse.new_frame();

    SDL_Event event;
    ImGuiIO& imgui_io = ImGui::GetIO();
    while(SDL_PollEvent(&event)){
        switch(event.type){
            case SDL_QUIT:
                return Engine_Code::Window_Quit;
            default:
                break;
        }

        // NOTE(hugo): short-circuit input to ImGui
        ImGui_ImplSDL2_ProcessEvent(&event);
        if(imgui_io.WantCaptureMouse || imgui_io.WantCaptureKeyboard) continue;

        keyboard.register_event(event);
        mouse.register_event(event);
        window.register_event(event);
    }

    return Engine_Code::Nothing;
}

Engine_Code Engine::update_start(){
    if(g_engine.scene.scene_stack.size == 0u) return Engine_Code::No_Scene;

    return Engine_Code::Nothing;
}

void Engine::update_end(){
}

Engine_Code Engine::render_start(){
    if(g_engine.scene.scene_stack.size == 0u) return Engine_Code::No_Scene;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(g_engine.window.handle);
    ImGui::NewFrame();

    // NOTE(hugo): window resizing
    if(offscreen_target.height != window.height || offscreen_target.width != window.width){
        renderer.free_render_target(offscreen_target);
        offscreen_target = renderer.get_render_target(window.width, window.height);
    }

    renderer.use_render_target(offscreen_target);
    renderer.clear_render_target(offscreen_target);

    return Engine_Code::Nothing;
}

void Engine::render_end(){
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    renderer.copy_render_target(offscreen_target, window.render_target());
    window.swap_buffers();

    DEV_next_frame();
}

Engine_Code frame(){
    if(g_engine.scene.scene_stack.size == 0u) return Engine_Code::No_Scene;
    Engine_Code error_code = Engine_Code::Nothing;

    error_code = g_engine.process_event();
    if(error_code != Engine_Code::Nothing) return error_code;

    u64 timer = timer_ticks();
    u32 nupdates = g_engine.frame_timing.nupdates_before_render(timer);

    for(u32 iupdate = 0; iupdate != nupdates; ++iupdate){
        error_code = g_engine.update_start();
        if(error_code != Engine_Code::Nothing) return error_code;
        g_engine.scene.update();
        g_engine.update_end();
    }

    error_code = g_engine.render_start();
    if(error_code != Engine_Code::Nothing) return error_code;
    g_engine.scene.render();
    g_engine.render_end();

    return error_code;
}

#if defined(PLATFORM_EMSCRIPTEN)
void emscripten_frame(){
    if(frame() != Engine_Code::Nothing) emscripten_cancel_main_loop();
}
#endif

int main(int argc, char* argv[]){

    printf("-- started using easy_setup\n");
    printf("-- user configuration\n");

    // ---- easy config
    easy_config();
    // ----

    printf("-- engine setup\n");

    g_engine.setup();

    printf("-- user setup\n");

    // ---- easy setup
    void* user_data = easy_setup();
    // ----

    printf("-- mainloop\n");

    // NOTE(hugo): signals to close the application
    // scene : scene_stack.size == 0u
    // engine : update_start() != 0u

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
    while(frame() == Engine_Code::Nothing){};
#elif defined(PLATFORM_EMSCRIPTEN)
    emscripten_set_main_loop(emscripten_frame, 0, 1);
#else
    static_assert(false, "no frame function was specified");
#endif

    printf("-- user termination\n");

    // ---- scene termination
    easy_terminate(user_data);
    // ----

    printf("-- engine termination\n");

    g_engine.terminate();

    printf("-- finished\n");

    return 0;
}
