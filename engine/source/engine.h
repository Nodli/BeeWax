#ifndef H_ENGINE
#define H_ENGINE

using namespace bw;

enum struct Engine_Code : u32{
    Nothing = 0u,
    No_Scene,
    Window_Quit,
};

struct Engine{
    void create();
    void destroy();

    Engine_Code process_event();
    Engine_Code update_start();
    void update_end();
    Engine_Code render_start();
    void render_end();

    // ---- data

    Action_Manager action_manager;
    Cursor cursor;

    Frame_Timing frame_timing;

    Window window;

    Render_Layer render_layer;
    Render_Target render_target;

    Audio_Player audio;

    Scene_Manager scene_manager;

    Asset_Catalog<Audio_Asset> audio_catalog;
    Asset_Catalog<Texture_Asset> texture_catalog;
    Asset_Catalog<Texture_Animation_Asset> texture_animation_catalog;
};

#endif
