#ifndef H_EASY_SETUP
#define H_EASY_SETUP

using namespace bw;

struct Engine{
    void setup();
    void terminate();

    u32 update_start();
    void update_end();
    void render_start();
    void render_end();

    // ---- data

    Keyboard_State keyboard;
    Mouse_State mouse;

    Window window;
    Renderer renderer;

    Font_Renderer font;
    Texture_Animation_Player texture_animation;
    Audio_Player audio;
    Asset_Manager asset;

    Scene_Manager scene;
};

static Engine g_engine;

// NOTE(hugo): to be defined by the user to inject setup / termination functions
void* easy_setup();
void easy_terminate(void* user_data);

#endif
