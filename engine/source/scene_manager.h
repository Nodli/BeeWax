#ifndef H_SCENE_MANAGER
#define H_SCENE_MANAGER

// TODO(hugo): scenes should be allowed to come from another DLL
// ie declare a Scene vtable in each DLL and match with names
// ie Scene_Manager should store a pointer to the Scene vtable

struct Scene_Description{
    const char* name;
    void (*on_push)(void*& data);
    void (*update)(void*& data);
    void (*render)(void*& data);
    void (*on_remove)(void*& data);
    void* data;
};

struct Scene_Manager{
    void create();
    void destroy();

    template<typename T>
    Scene_Description& push_scene(const char* name);
    Scene_Description& push_scene(const Scene_Description& desc);

    void remove_scene_by_ptr(void* data_ptr);
    void remove_scene_by_name(const char* name);

    Scene_Description* search_scene(const char* name);

    bool has_scene();

    void update();
    void render();

    // ---- data

    array<Scene_Description> stack;
};

// ----

#endif
