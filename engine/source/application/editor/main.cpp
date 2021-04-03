#include "../engine/easy_setup.h"
#include "../engine/easy_setup.cpp"

#include "editor.h"
#include "editor.cpp"

struct Editor_Scene{
    void on_push(){ editor.initialize(); };
    void on_remove(){ editor.terminate(); };
    void update(){ editor.update(); };
    void render(){ editor.render(); };

    // ---- data

    Editor editor;
};

void easy_config(){
    g_config.window_name = "Editor";
    g_config.window_width = 0;
    g_config.window_height = 0;
    g_config.asset_catalog_path = "./data/asset_catalog.json";
}
void* easy_setup(){
    g_engine.scene.push_scene<Editor_Scene>("Editor_Scene");
    return nullptr;
}
void easy_terminate(void* user_data){}
