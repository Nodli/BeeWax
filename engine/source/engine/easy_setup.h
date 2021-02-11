#ifndef H_EASY_SETUP
#define H_EASY_SETUP

using namespace bw;

enum struct Engine_Code : u32{
    Nothing = 0u,
    No_Scene,
    Window_Quit,
};

struct Engine{
    void setup();
    void terminate();

    Engine_Code update_start();
    void update_end();
    Engine_Code render_start();
    void render_end();

    // ---- data

    Keyboard_State keyboard;
    Mouse_State mouse;

    Frame_Timing frame_timing;

    Window window;
    Renderer renderer;
    Audio_Player audio;

    Asset_Catalog<Audio_Asset> audio_catalog;
    Asset_Catalog<Texture_Asset> texture_catalog;

    Scene_Manager scene;
};

namespace g_config {
    static const char* window_name = "easy_setup";
    static File_Path asset_catalog_path = "./data/asset_catalog.json";
};

static Engine g_engine;

// NOTE(hugo): to be defined by the user to inject setup / termination functions
void easy_config();
void* easy_setup();
void easy_terminate(void* user_data);

#endif
