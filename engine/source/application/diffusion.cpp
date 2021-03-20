#include "../engine/easy_setup.h"
#include "../engine/easy_setup.cpp"

void easy_config(){
    g_config::window_name = "Diffusion";
    g_config::window_width = 1280u;
    g_config::window_width = 720u;
    g_config::asset_catalog_path = "./data/asset_catalog_path.json"
}
void* easy_setup(){
    g_engine.scene.push_scene<Diffusion_Scene>("Diffusion_Scene");
    return nullptr;
}
void easy_terminate(void* user_data){
}
