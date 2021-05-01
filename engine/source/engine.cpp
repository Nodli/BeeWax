void Engine::create(){

    // ---- externals

    SDL_CHECK(SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) == 0);
    setup_vmemory();
    setup_timer();
    setup_LOG();
    stbi_set_flip_vertically_on_load(true);

    // ---- engine

    action_manager.create();
    cursor.create();

    // --

    frame_timing.create(timer_ticks(), timer_frequency(), 60u, 0.05f, 2u, 60u);

    // --

    Window_Settings window_settings;
    window_settings.width = g_engine_config.window_width;
    window_settings.height = g_engine_config.window_height;
    window_settings.name = g_engine_config.window_name;
    window_settings.mode = Window_Settings::mode_windowed;
    window_settings.synchronization = Window_Settings::synchronize;
    window_settings.OpenGL_major = 3u;
    window_settings.OpenGL_minor = 3u;

    window.create(window_settings);

    // --

    gl3wInit();

    DEV_Debug_Render_Layer();

    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

    glEnable(GL_MULTISAMPLE);

    glEnable(GL_FRAMEBUFFER_SRGB);
    glClearColor(0.5f, 0.5f, 0.5f, 1.f);
    glClearDepth(1.f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glCullFace(GL_BACK);
    //glEnable(GL_CULL_FACE);

    render_layer.create();

    u32 target_width = g_engine_config.target_width ? g_engine_config.target_width : window.width;
    u32 target_height = g_engine_config.target_height ? g_engine_config.target_height : window.height;
    render_target = render_layer.get_render_target_multisample(target_width, target_height, g_engine_config.target_samples);

    // --

    audio.create();

    // --

    scene_manager.create();

    // --

    audio_catalog.create();
    texture_catalog.create();
    texture_animation_catalog.create();

    Asset_Catalog_Description asset_catalog_desc[] = {
        {"audio",             &audio_catalog.map,                &create_Audio_Asset_from_json,              &destroy_Audio_Asset},
        {"texture",           &texture_catalog.map,              &create_Texture_Asset_from_json,            &destroy_Texture_Asset},
        {"texture_animation", &texture_animation_catalog.map,    &create_Texture_Animation_Asset_from_json,  &destroy_Texture_Animation_Asset}
    };
    import_asset_from_json(g_engine_config.asset_catalog_path, this, asset_catalog_desc, carray_size(asset_catalog_desc));

    // ---- dev tools

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

    DEV_create();
}

void Engine::destroy(){
    // ---- dev tools

    DEV_destroy();

    ImGui::DestroyContext();

    // ---- engine

    Asset_Catalog_Description asset_catalog_desc[] = {
        {"audio",             &audio_catalog.map,                &create_Audio_Asset_from_json,              &destroy_Audio_Asset},
        {"texture",           &texture_catalog.map,              &create_Texture_Asset_from_json,            &destroy_Texture_Asset},
        {"texture_animation", &texture_animation_catalog.map,    &create_Texture_Animation_Asset_from_json,  &destroy_Texture_Animation_Asset}
    };
    remove_asset_from_json(g_engine_config.asset_catalog_path, this, asset_catalog_desc, carray_size(asset_catalog_desc));

    texture_animation_catalog.destroy();
    texture_catalog.destroy();
    audio_catalog.destroy();

    scene_manager.destroy();

    audio.destroy();
    render_layer.free_render_target(render_target);
    render_layer.destroy();

    window.destroy();

    frame_timing.destroy();

    cursor.destroy();

    action_manager.destroy();

    // ---- external

    SDL_Quit();
}

Engine_Code Engine::process_event(){
    action_manager.new_frame();

    SDL_Event event;
    ImGuiIO& imgui_io = ImGui::GetIO();
    while(SDL_PollEvent(&event)){
        switch(event.type){
            case SDL_QUIT:
                return Engine_Code::Window_Quit;
            default:
                break;
        }

        if(window.register_event(event))            continue;

        ImGui_ImplSDL2_ProcessEvent(&event);
        if(imgui_io.WantCaptureMouse || imgui_io.WantCaptureKeyboard) continue;

        if(action_manager.register_event(event))    continue;
    }

    return Engine_Code::Nothing;
}

Engine_Code Engine::update_start(){
    if(!g_engine.scene_manager.has_scene()) return Engine_Code::No_Scene;

    return Engine_Code::Nothing;
}

void Engine::update_end(){
}

Engine_Code Engine::render_start(){
    if(!g_engine.scene_manager.has_scene()) return Engine_Code::No_Scene;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(g_engine.window.handle);
    ImGui::NewFrame();

    // NOTE(hugo): window resizing
    u32 target_width = g_engine_config.target_width ? g_engine_config.target_width : window.width;
    u32 target_height = g_engine_config.target_height ? g_engine_config.target_height : window.height;
    if(render_target.width != target_width || render_target.height != target_height){
        render_layer.free_render_target(render_target);
        render_target = render_layer.get_render_target_multisample(target_width, target_height, g_engine_config.target_samples);
    }

    render_layer.use_render_target(render_target);
    render_layer.clear_render_target(render_target);

    return Engine_Code::Nothing;
}

void Engine::render_end(){
    render_layer.copy_render_target(render_target, window.render_target());

    render_layer.use_render_target(window.render_target());
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    window.swap_buffers();
}
