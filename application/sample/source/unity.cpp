using namespace bw;

static Engine* g_engine_ptr;

void set_engine_ptr(Engine* ptr){g_engine_ptr = ptr;};
Engine& get_engine(){return *g_engine_ptr;};

// ----

//#include "sample_ImDrawer.cpp"
//#include "sample_Rect_Packer.cpp"
//#include "sample_Physics.cpp"
//#include "sample_ArchECS.cpp"
#include "sample_Theremin.cpp"


// ----

int main(int argc, char* argv[]){
    Engine_Config configuration;
    configuration.window_name = "Engine";
    configuration.window_width = 1000u;
    configuration.window_height = 1000u;
    configuration.render_target_samples = 1u;

    Engine engine;
    set_engine_ptr(&engine);

    printf("-- starting\n");

    engine.create(configuration);
    //engine.scene_manager.push_scene<ImDrawer_Scene>("Scene");
    //engine.scene_manager.push_scene<Rect_Packer_Scene>("Scene");
    //engine.scene_manager.push_scene<Physics_Scene>("Scene");
    //engine.scene_manager.push_scene<ArchECS_Scene>("Scene");
    engine.scene_manager.push_scene<Theremin_Scene>("Scene");

    printf("-- mainloop\n");

    engine.run();

    printf("-- engine destroy \n");

    engine.destroy();

    printf("-- finished\n");

    DEV_Memtracker_Leakcheck();

    return 0;
}
