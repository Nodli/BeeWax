void Engine::setup(){

    // ---- externals

    if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
        printf("Failed SDL_Init() %s\n", SDL_GetError());
    }

    stbi_set_flip_vertically_on_load(true);

    // ---- input

    keyboard.initialize();
    mouse.initialize();

    // ---- window

    const char* window_name = "default_main";
    Window_Settings window_settings;
    window_settings.name = window_name;
    window_settings.width = 1280;
    window_settings.height = 720;
    window_settings.mode = Window_Settings::mode_windowed;
    window_settings.synchronization = Window_Settings::synchronize_vertical;
    window_settings.buffering = Window_Settings::buffering_double;

    window.initialize(window_settings);

    // ---- renderer

    gl3wInit();
    glClearColor(0.f, 0.f, 0.f, 1.f);

    //DEV_Debug_Renderer;

    renderer.setup_resources();

    // ---- font

    font.width = window.width;
    font.height = window.height;
    font.renderer = &renderer;

    // ---- texture animation

    // ---- audio

    audio.setup();

    // ---- asset

    asset.audio_player = &audio;
    asset.renderer = &renderer;

    // ---- dev tools

    DEV_setup();
}

void Engine::terminate(){
    // ---- dev tools

    DEV_terminate();

    // ---- engine

    asset.terminate();
    audio.terminate();
    texture_animation.terminate();
    renderer.free_resources();
    window.terminate();

    // ---- external

    SDL_Quit();
}

u32 Engine::update_start(){
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
    keyboard.next_frame();
    mouse.next_frame();
    audio.mix_next_frame();
    texture_animation.next_frame();
}

void Engine::render_start(){
    renderer.start_frame();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Engine::render_end(){
    renderer.end_frame();
    window.swap_buffers();

    DEV_LOG_timing_entries();
    DEV_next_frame();
}

int main(int argc, char* argv[]){

    Frame_Timing frame_timing;
    frame_timing.initialize(60u);

    g_engine.setup();

    // ---- easy setup
    void* user_data = easy_setup();
    // ----

    // NOTE(hugo): signals to close the application
    // scene : scene_stack.size == 0u
    // engine : update_start() != 0u
    while(g_engine.scene.scene_stack.size != 0u){
        //DEV_LOG_frame_duration;

        u32 nupdates = frame_timing.nupdates_before_render();
        for(u32 iupdate = 0; iupdate != nupdates; ++iupdate){
            if(g_engine.update_start() != 0u){
                goto exit_gameloop;
            }
            g_engine.scene.update();
            g_engine.update_end();
        }
        g_engine.render_start();
        g_engine.scene.render();
        g_engine.render_end();
    }

    exit_gameloop:

    // ---- scene termination
    easy_terminate(user_data);
    // ----

    g_engine.scene.terminate();
    g_engine.terminate();

    return 0;
}

