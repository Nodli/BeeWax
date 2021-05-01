void Scene_Manager::create(){
    stack.create();
}

void Scene_Manager::destroy(){
    for(auto& scene : stack){
        (*scene.on_remove)(scene.data);
    }
    stack.destroy();
}

namespace BEEWAX_INTERNAL{
    template<typename T>
    void generic_scene_on_push(void*& data){
        data = bw_malloc(sizeof(T));
        new (data) T();
    }

    template<typename T>
    void generic_scene_update(void*& data){
        (*(T*)data).update();
    }

    template<typename T>
    void generic_scene_render(void*& data){
        (*(T*)data).render();
    }

    template<typename T>
    void generic_scene_on_remove(void*& data){
        T* typed_data = (T*)data;
        (*typed_data).~T();
        bw_free(data);
    }
}

template<typename T>
Scene_Description& Scene_Manager::push_scene(const char* name){
    Scene_Description desc;
    desc.name = name;
    desc.on_push = &BEEWAX_INTERNAL::generic_scene_on_push<T>;
    desc.update = &BEEWAX_INTERNAL::generic_scene_update<T>;
    desc.render = &BEEWAX_INTERNAL::generic_scene_render<T>;
    desc.on_remove = &BEEWAX_INTERNAL::generic_scene_on_remove<T>;

    if(desc.on_push) (*desc.on_push)(desc.data);
    return stack.push(desc);
}

Scene_Description& Scene_Manager::push_scene(const Scene_Description& desc){
    Scene_Description& stack_desc = stack.push(desc);
    if(stack_desc.on_push) (*stack_desc.on_push)(stack_desc.data);
    return stack_desc;
}

void Scene_Manager::remove_scene_by_ptr(void* data_ptr){
    u32 remove_index = UINT32_MAX;
    for(u32 iscene = 0u; iscene != stack.size; ++iscene){
        if(stack[stack.size - 1u - iscene].data == data_ptr){
            remove_index = stack.size - 1u - iscene;
            break;
        }
    }
    assert(remove_index != UINT32_MAX);

    Scene_Description& desc = stack[remove_index];
    if(desc.on_remove) (*desc.on_remove)(desc.data);
    stack.remove(remove_index);
}

void Scene_Manager::remove_scene_by_name(const char* scene_name){
    u32 remove_index = UINT32_MAX;
    for(u32 iscene = 0u; iscene != stack.size; ++iscene){
        if(strcmp(stack[stack.size - 1u - iscene].name, scene_name) == 0u){
            remove_index = stack.size - 1u - iscene;
            break;
        }
    }
    assert(remove_index != UINT32_MAX);

    Scene_Description& desc = stack[remove_index];
    if(desc.on_remove) (*desc.on_remove)(desc.data);
    stack.remove(remove_index);
}

Scene_Description* Scene_Manager::search_scene(const char* scene_name){
    Scene_Description* ptr = nullptr;
    for(u32 iscene = 0u; iscene != stack.size; ++iscene){
        if(strcmp(stack[stack.size - 1u - iscene].name, scene_name) == 0u){
            ptr = &stack[stack.size - 1u - iscene];
            break;
        }
    }
    return ptr;
}

bool Scene_Manager::has_scene(){
    return stack.size != 0u;
}

void Scene_Manager::update(){
    assert(stack.size);

    Scene_Description& desc = stack[stack.size - 1u];
    if(desc.update) (*desc.update)(desc.data);
}

void Scene_Manager::render(){
    assert(stack.size);

    Scene_Description& desc = stack[stack.size - 1u];
    if(desc.render) (*desc.render)(desc.data);
}
