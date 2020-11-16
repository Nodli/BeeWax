using namespace bw;

struct Engine{
    void setup();
    void terminate();
    u32 update_start();
    void update_end();
    void render_start();
    void render_end();

    // ---- data

    Window window;
    Renderer renderer;
    Font_Renderer font;
    Audio_Manager audio;
    Keyboard_State keyboard;
    Mouse_State mouse;
};

enum Scene_Action : u32{
    action_continue,
    action_stop
};
struct Scene{
    const char* name;
    void* data;
    u32 (*update)(void* data);
    u32 (*render)(void* data);
    void (*terminate)(void* data);
};

// NOTE(hugo): template for a Scene structure ; search and replace struct_name to something
/*
struct struct_name{
    static void setup(void* data){
        struct_name* self = (struct_name*)data;
    }
    static u32 update(void* data){
        struct_name* self = (struct_name*)data;
        return action_stop;
    }
    static u32 render(void* data){
        struct_name* self = (struct_name*)data;
        return action_stop;
    }
    static void terminate(void* data){
        struct_name* self = (struct_name*)data;
    }

    // ---- data
};
*/

struct Scene_Manager{
    template<typename T>
    T* push_scene(const char* scene_name);
    void pop_scene();

    void interpret_scene_action(u32 action, u32& index);
    void update();
    void render();

    // ---- data

    darray<Scene> scene_stack;
};

static struct {
    Engine engine;
    Scene_Manager manager;
} g_context;

void easy_entry();
void easy_exit();

int main(int argc, char* argv[]){

    Frame_Timing frame_timing;
    frame_timing.initialize(60u);

    g_context.engine.setup();

    // ---- easy setup
    easy_entry();
    // ----

    // NOTE(hugo):
    // scene_stack.size == 0u is the game's signal to shut down
    // engine->update_start() == 1u is the engine's signal to shut down
    while(g_context.manager.scene_stack.size != 0u){
        u32 nupdates = frame_timing.nupdates_before_render();
        for(u32 iupdate = 0; iupdate != nupdates; ++iupdate){
            if(g_context.engine.update_start() == 1u){
                goto exit_gameloop;
            }
            g_context.manager.update();
            g_context.engine.update_end();
        }
        g_context.engine.render_start();
        g_context.manager.render();
        g_context.engine.render_end();
    }

    exit_gameloop:

    // ---- scene termination
    easy_exit();
    // ----

    g_context.manager.scene_stack.free();
    g_context.engine.terminate();

    return 0;
}
