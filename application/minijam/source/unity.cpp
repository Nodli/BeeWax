using namespace bw;

static Engine* g_engine_ptr;

void set_engine_ptr(Engine* ptr){g_engine_ptr = ptr;};
Engine& get_engine(){return *g_engine_ptr;};

// ----

#include "minijam.cpp"

// ----

int main(int argc, char* argv[]){
    Engine_Config configuration;
    configuration.window_name = "Engine";
    configuration.window_width = 1000u;
    configuration.window_height = 1000u;
    configuration.render_target_samples = 1u;
    configuration.asset_catalog_path = "./data/asset_catalog.json";

    Engine engine;
    set_engine_ptr(&engine);

    printf("-- starting\n");

    engine.create(configuration);
    engine.scene_manager.push_scene<Minijam_Scene>("Scene");

    printf("-- mainloop\n");

    engine.run();

    printf("-- engine destroy \n");

    engine.destroy();

    printf("-- finished\n");

    DEV_Memtracker_Leakcheck();

    return 0;
}
