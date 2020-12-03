void Scene_Manager::terminate(){
    for(u32 iscene = 0u; iscene != scene_stack.size; ++iscene){
        Scene& scene = scene_stack[iscene];
        scene.terminate();
        ::free(scene.data);
    }
    scene_stack.free();
}

template<typename T>
T* Scene_Manager::push_scene(const char* scene_name){
    T* scene = new_struct<T>();
    T::setup((void*)scene);

    Scene to_push;
    to_push.name = scene_name;
    to_push.data = (void*)scene;
    to_push.setup_func = &T::setup;
    to_push.update_func = &T::update;
    to_push.render_func = &T::render;
    to_push.terminate_func = &T::terminate;
    scene_stack.push(to_push);

    return scene;
}

void Scene_Manager::remove_scene(const char* scene_name){
    u32 remove_index = UINT_MAX;
    for(u32 iscene = 0u; iscene != scene_stack.size; ++iscene){
        if(strcmp(scene_stack[scene_stack.size - 1u - iscene].name, scene_name) == 0u){
            remove_index = scene_stack.size - 1u - iscene;
            break;
        }
    }

    if(remove_index == UINT_MAX){
        LOG_ERROR("no scene with name %s", scene_name);
    }

    Scene& scene = scene_stack[remove_index];
    scene.terminate();
    ::free(scene.data);
    scene_stack.remove(remove_index);
}

Scene* Scene_Manager::search_scene(const char* scene_name){
    for(u32 iscene = 0u; iscene != scene_stack.size; ++iscene){
        if(strcmp(scene_stack[scene_stack.size - 1u - iscene].name, scene_name) == 0u){
            return &scene_stack[scene_stack.size - 1u - iscene];
        }
    }
    return nullptr;
}

void Scene_Manager::update(){
    scene_stack[scene_stack.size - 1u].update();
}

void Scene_Manager::render(){
    scene_stack[scene_stack.size - 1u].render();
}
