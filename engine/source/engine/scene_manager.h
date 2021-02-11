#ifndef H_SCENE_MANAGER
#define H_SCENE_MANAGER

struct Scene_Info{
    const char* name;
    void* data;
    void (*on_push)(void* data);
    void (*update)(void* data);
    void (*render)(void* data);
    void (*on_remove)(void* data);
};

struct Scene_Manager{
    void terminate();

    template<typename T,
    void(T::*on_push_ptr)() = &T::on_push,
    void(T::*update_ptr)() = &T::update,
    void(T::*render_ptr)() = &T::render,
    void(T::*on_remove_ptr)() = &T::on_remove>
    void push_scene(const char* scene_name);

    void remove_scene(const char* scene_name);
    Scene_Info* search_scene(const char* scene_name);

    void update();
    void render();

    // ---- data

    array<Scene_Info> scene_stack;
};

#endif
