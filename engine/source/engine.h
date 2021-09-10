#ifndef H_ENGINE
#define H_ENGINE

struct Engine_Config{
    const char* window_name;
    u32 window_width;
    u32 window_height;
    u32 render_target_samples;
};

struct Engine{
    void create(const Engine_Config& iconfig);
    void destroy();

    void run();

    // ---- data

    Action_Manager action_manager;
    Cursor cursor;

    Frame_Timing frame_timing;

    Window window;

    Render_Layer render_layer;
    Render_Target render_target;

    Audio_Player audio;

    Scene_Manager scene_manager;
};

void set_noot_icon(Window& window);

#endif
