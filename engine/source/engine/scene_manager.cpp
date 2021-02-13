void Scene_Manager::terminate(){
    for(u32 iscene = 0u; iscene != scene_stack.size; ++iscene){
        Scene_Info& scene_info = scene_stack[iscene];
        (*scene_info.on_remove)(scene_info.data);
        ::free(scene_info.data);
    }
    scene_stack.free();
}

template<typename T, void(T::*on_push_ptr)(), void(T::*update_ptr)(), void(T::*render_ptr)(), void(T::*on_remove_ptr)()>
struct Scene_Static_Proxy{
    static void on_push(void* ptr){
        T* type = (T*)ptr;
        (*type.*on_push_ptr)();
    };
    static void update(void* ptr){
        T* type = (T*)ptr;
        (*type.*update_ptr)();
    };
    static void render(void* ptr){
        T* type = (T*)ptr;
        (*type.*render_ptr)();
    };
    static void on_remove(void* ptr){
        T* type = (T*)ptr;
        (*type.*on_remove_ptr)();
    };
};

template<typename T,
void(T::*on_push_ptr)(),
void(T::*update_ptr)(),
void(T::*render_ptr)(),
void(T::*on_remove_ptr)()>
void Scene_Manager::push_scene(const char* scene_name){
    typedef Scene_Static_Proxy<T, on_push_ptr, update_ptr, render_ptr, on_remove_ptr> Proxy;

    void* ptr = malloc(sizeof(T));
    new((void*) ptr) T{};

    Proxy::on_push(ptr);

    Scene_Info scene_info;
    scene_info.name = scene_name;
    scene_info.data = ptr;
    scene_info.on_push = &Proxy::on_push;
    scene_info.update = &Proxy::update;
    scene_info.render = &Proxy::render;
    scene_info.on_remove = &Proxy::on_remove;
    scene_stack.push(scene_info);
}

void Scene_Manager::remove_scene_by_ptr(void* data_ptr){
    u32 remove_index = UINT32_MAX;
    for(u32 iscene = 0u; iscene != scene_stack.size; ++iscene){
        if(scene_stack[scene_stack.size - 1u - iscene].data == data_ptr){
            remove_index = scene_stack.size - 1u - iscene;
            break;
        }
    }
    ENGINE_CHECK(remove_index != UINT32_MAX, "no scene with data pointer %p", data_ptr);

    Scene_Info& scene_info = scene_stack[remove_index];
    (*scene_info.on_remove)(scene_info.data);
    ::free(scene_info.data);
    scene_stack.remove(remove_index);
}

void Scene_Manager::remove_scene_by_name(const char* scene_name){
    u32 remove_index = UINT32_MAX;
    for(u32 iscene = 0u; iscene != scene_stack.size; ++iscene){
        if(strcmp(scene_stack[scene_stack.size - 1u - iscene].name, scene_name) == 0u){
            remove_index = scene_stack.size - 1u - iscene;
            break;
        }
    }
    ENGINE_CHECK(remove_index != UINT32_MAX, "no scene with name %s", scene_name);

    Scene_Info& scene_info = scene_stack[remove_index];
    (*scene_info.on_remove)(scene_info.data);
    ::free(scene_info.data);
    scene_stack.remove(remove_index);
}

Scene_Info* Scene_Manager::search_scene(const char* scene_name){
    Scene_Info* ptr = nullptr;
    for(u32 iscene = 0u; iscene != scene_stack.size; ++iscene){
        if(strcmp(scene_stack[scene_stack.size - 1u - iscene].name, scene_name) == 0u){
            ptr = &scene_stack[scene_stack.size - 1u - iscene];
            break;
        }
    }
    return ptr;
}

void Scene_Manager::update(){
    assert(scene_stack.size);

    Scene_Info& scene_info = scene_stack[scene_stack.size - 1u];
    (*scene_info.update)(scene_info.data);
}

void Scene_Manager::render(){
    assert(scene_stack.size);

    Scene_Info& scene_info = scene_stack[scene_stack.size - 1u];
    (*scene_info.render)(scene_info.data);
}
