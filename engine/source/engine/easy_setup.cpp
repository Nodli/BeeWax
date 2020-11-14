// ---- engine

void Engine::setup(){
    // ---- developper tools

    DEV_INITIALIZE;
    DEV_DISPLAY_TWEAKABLE_ENTRIES;

    // ---- initialization ---- //

    if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
        printf("Failed SDL_Init() %s\n", SDL_GetError());
    }

    // ---- window

    const char* window_name = "default_main";
    Window_Settings window_settings;
    window_settings.name = window_name;
    window_settings.width = 1280;
    window_settings.height = 720;
    //window_settings.sync = Window_Settings::SINGLE_BUFFER_NOSYNC;

    window.initialize(window_settings);

    // ---- renderer

    gl3wInit();
    glClearColor(0.f, 0.f, 0.f, 1.f);

    renderer.setup_resources();

    //DEV_DEBUG_RENDERER;

    // ---- font

    font.window = &window;
    font.renderer = &renderer;
    font.make_bitmap_from_file("./data/Roboto_Font/Roboto-Black.ttf", ASCII_PRINTABLE, window_settings.height / 10.f, 5, 180);

    // ---- audio

    audio.setup();
}

void Engine::terminate(){
    audio.terminate();
    font.free();
    renderer.free_resources();
    window.terminate();

    SDL_Quit();

    DEV_TERMINATE;
}

u32 Engine::update_start(){
    //DEV_LOG_FRAME_TIME;

    // ---- event handling

    SDL_Event event;
    while(SDL_PollEvent(&event)){
        switch(event.type){
            case SDL_QUIT:
                return 1u;
            default:
                break;
        }

        keyboard.register_event(event);
        mouse.register_event(event);
    }
    return 0u;
}

void Engine::update_end(){
    keyboard.reset();
    mouse.reset();
    audio.next_frame();
}

void Engine::render_start(){
    renderer.start_frame();
    font.start_frame();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Engine::render_end(){
    font.end_frame();
    renderer.end_frame();
    window.swap_buffers();

    DEV_DISPLAY_TIMING_ENTRIES;
    DEV_NEXT_FRAME;
}

// ---- scene manager

template<typename T>
T* Scene_Manager::push_scene(const char* scene_name){
    T* scene = new_struct<T>();
    T::setup((void*)scene);

    Scene to_push;
    to_push.name = scene_name;
    to_push.data = (void*)scene;
    to_push.update = &T::update;
    to_push.render = &T::render;
    to_push.terminate = &T::terminate;
    scene_stack.push(to_push);

    return scene;
}

void Scene_Manager::pop_scene(){
    assert(scene_stack.size);
    Scene& scene = scene_stack[scene_stack.size - 1u];
    scene.terminate(scene.data);
    free(scene.data);
    scene_stack.pop();
}

void Scene_Manager::interpret_scene_action(u32 action, u32& index){
    switch(action){
        case action_continue:
            ++index;
            break;
        default:
        case action_stop:
            index = scene_stack.size;
    }
}

void Scene_Manager::update(){
    u32 scene_index = 0u;
    while(scene_index < scene_stack.size){
        Scene& scene = scene_stack[scene_stack.size - 1u - scene_index];
        u32 scene_action = scene.update(scene.data);
        interpret_scene_action(scene_action, scene_index);
    }
}

void Scene_Manager::render(){
    u32 scene_index = 0u;
    while(scene_index < scene_stack.size){
        Scene& scene = scene_stack[scene_stack.size - 1u - scene_index];
        u32 scene_action = scene.render(scene.data);
        interpret_scene_action(scene_action, scene_index);
    }
}
