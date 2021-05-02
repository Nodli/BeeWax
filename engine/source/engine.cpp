void Engine::create(const Engine_Config& iconfig){

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
    window_settings.width = iconfig.window_width;
    window_settings.height = iconfig.window_height;
    window_settings.name = iconfig.window_name;
    window_settings.mode = Window_Settings::mode_windowed;
    window_settings.synchronization = Window_Settings::synchronize;
    window_settings.size_control = Window_Settings::size_control_none;
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

    render_target = render_layer.get_render_target_multisample(iconfig.window_width, iconfig.window_height, iconfig.render_target_samples);

    // --

    audio.create();

    // --

    scene_manager.create();

    // --

    audio_catalog.create();
    texture_catalog.create();
    texture_animation_catalog.create();

    asset_catalog_path = iconfig.asset_catalog_path;

    if(asset_catalog_path != ""){
        Asset_Catalog_Description asset_catalog_desc[] = {
            {"audio",             &audio_catalog.map,                &create_Audio_Asset_from_json,              &destroy_Audio_Asset},
            {"texture",           &texture_catalog.map,              &create_Texture_Asset_from_json,            &destroy_Texture_Asset},
            {"texture_animation", &texture_animation_catalog.map,    &create_Texture_Animation_Asset_from_json,  &destroy_Texture_Animation_Asset}
        };
        import_asset_from_json(asset_catalog_path, this, asset_catalog_desc, carray_size(asset_catalog_desc));
    }

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

    if(asset_catalog_path != ""){
        Asset_Catalog_Description asset_catalog_desc[] = {
            {"audio",             &audio_catalog.map,                &create_Audio_Asset_from_json,              &destroy_Audio_Asset},
            {"texture",           &texture_catalog.map,              &create_Texture_Asset_from_json,            &destroy_Texture_Asset},
            {"texture_animation", &texture_animation_catalog.map,    &create_Texture_Animation_Asset_from_json,  &destroy_Texture_Animation_Asset}
        };
        remove_asset_from_json(asset_catalog_path, this, asset_catalog_desc, carray_size(asset_catalog_desc));
    }

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

namespace BEEWAX_INTERNAL{
    enum struct Engine_Code : u32{
        Nothing = 0u,
        No_Scene,
        Window_Quit,
    };

    static Engine_Code  Engine_process_event(Engine& engine);
    static Engine_Code  Engine_update_start(Engine& engine);
    static void         Engine_update_end(Engine& engine);
    static Engine_Code  Engine_render_start(Engine& engine);
    static void         Engine_render_end(Engine& engine);
    static Engine_Code  Engine_main_frame(Engine& engine);

    Engine_Code Engine_process_event(Engine& engine){
        engine.action_manager.new_frame();

        SDL_Event event;
        ImGuiIO& imgui_io = ImGui::GetIO();
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    return Engine_Code::Window_Quit;
                default:
                    break;
            }

            if(engine.window.register_event(event)) continue;

            ImGui_ImplSDL2_ProcessEvent(&event);
            if(imgui_io.WantCaptureMouse || imgui_io.WantCaptureKeyboard) continue;

            if(engine.action_manager.register_event(event)) continue;
        }

        return Engine_Code::Nothing;
    }

    Engine_Code Engine_update_start(Engine& engine){
        if(!engine.scene_manager.has_scene()) return Engine_Code::No_Scene;

        return Engine_Code::Nothing;
    }

    void Engine_update_end(Engine& engine){
    }

    Engine_Code Engine_render_start(Engine& engine){
        if(!engine.scene_manager.has_scene()) return Engine_Code::No_Scene;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(engine.window.handle);
        ImGui::NewFrame();

        // NOTE(hugo): window resizing
        if(engine.render_target.width != engine.window.width || engine.render_target.height != engine.window.height){
            engine.render_layer.free_render_target(engine.render_target);
            engine.render_target = engine.render_layer.get_render_target_multisample(engine.window.width, engine.window.height, engine.render_target.samples);
        }

        engine.render_layer.use_render_target(engine.render_target);
        engine.render_layer.clear_render_target();

        return Engine_Code::Nothing;
    }

    void Engine_render_end(Engine& engine){
        engine.render_layer.copy_render_target(engine.render_target, engine.window.render_target());

        engine.render_layer.use_render_target(engine.window.render_target());
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        engine.window.swap_buffers();
    }

    Engine_Code Engine_main_frame(Engine& engine){
        if(!engine.scene_manager.has_scene()) return Engine_Code::No_Scene;
        Engine_Code error_code = Engine_Code::Nothing;

        error_code = Engine_process_event(engine);
        if(error_code != Engine_Code::Nothing) return error_code;

        u64 timer = timer_ticks();
        u32 nupdates = engine.frame_timing.nupdates_before_render(timer);

        for(u32 iupdate = 0; iupdate != nupdates; ++iupdate){
            error_code = Engine_update_start(engine);
            if(error_code != Engine_Code::Nothing) return error_code;
            engine.scene_manager.update();
            Engine_update_end(engine);
        }

        error_code = Engine_render_start(engine);
        if(error_code != Engine_Code::Nothing) return error_code;
        engine.scene_manager.render();
        Engine_render_end(engine);

        return error_code;
    }
}

void Engine::run(){
    while(BEEWAX_INTERNAL::Engine_main_frame(*this) == BEEWAX_INTERNAL::Engine_Code::Nothing){};
}
