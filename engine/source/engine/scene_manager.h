#ifndef H_SCENE_MANAGER
#define H_SCENE_MANAGER

// NOTE(hugo): template for a Scene structure ; search and replace struct_name to something
/*
struct struct_name{
    static void setup(void* data){
        struct_name* self = (struct_name*)data;
    }
    static void update(void* data){
        struct_name* self = (struct_name*)data;
        return action_stop;
    }
    static void render(void* data){
        struct_name* self = (struct_name*)data;
        return action_stop;
    }
    static void terminate(void* data){
        struct_name* self = (struct_name*)data;
    }

    // ---- data
};
*/

struct Scene{
    void setup()    { setup_func(data);     };
    void update()   { update_func(data);    };
    void render()   { render_func(data);    };
    void terminate(){ terminate_func(data); };

    const char* name;
    void* data;
    void (*setup_func)(void* data);
    void (*update_func)(void* data);
    void (*render_func)(void* data);
    void (*terminate_func)(void* data);
};

struct Scene_Manager{
    void terminate();

    template<typename T>
    T* push_scene(const char* scene_name);
    void remove_scene(const char* scene_name);

    Scene* search_scene(const char* scene_name);

    void update();
    void render();

    // ---- data

    darray<Scene> scene_stack;
};

#endif
