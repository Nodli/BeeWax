static struct {
    const char* window_name = "";
    s32 window_width = 800;
    s32 window_height = 600;

    s32 target_width = 0;
    s32 target_height = 0;
    u32 target_samples = 1u;

    File_Path asset_catalog_path = "./data/asset_catalog.json";
} g_engine_config;

static Engine g_engine;

// NOTE(hugo): to be defined by the user to inject setup / termination functions
void user_config();
void* user_create();
void user_destroy(void* user_data);

Engine_Code main_frame(){
    if(!g_engine.scene_manager.has_scene()) return Engine_Code::No_Scene;
    Engine_Code error_code = Engine_Code::Nothing;

    error_code = g_engine.process_event();
    if(error_code != Engine_Code::Nothing) return error_code;

    u64 timer = timer_ticks();
    u32 nupdates = g_engine.frame_timing.nupdates_before_render(timer);

    for(u32 iupdate = 0; iupdate != nupdates; ++iupdate){
        error_code = g_engine.update_start();
        if(error_code != Engine_Code::Nothing) return error_code;
        g_engine.scene_manager.update();
        g_engine.update_end();
    }

    error_code = g_engine.render_start();
    if(error_code != Engine_Code::Nothing) return error_code;
    g_engine.scene_manager.render();
    g_engine.render_end();

    return error_code;
}

int main(int argc, char* argv[]){

    printf("-- starting\n");
    printf("-- user configuration\n");

    // ---- easy config
    user_config();
    // ----

    printf("-- engine create\n");

    g_engine.create();

    printf("-- user create\n");

    // ---- easy setup
    void* user_data = user_create();
    // ----

    printf("-- mainloop\n");

    // NOTE(hugo): signals to close the application
    // scene : scene_stack.size == 0u
    // engine : update_start() != 0u

    while(main_frame() == Engine_Code::Nothing){};

    printf("-- user destroy\n");

    // ---- scene termination
    user_destroy(user_data);
    // ----

    printf("-- engine destroy \n");

    g_engine.destroy();

    DEV_Memtracker_Leakcheck();

    printf("-- finished\n");

    return 0;
}
