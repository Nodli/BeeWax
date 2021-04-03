u32 Editor::polygon_geometry_request(u32 nvertices){
    assert(nvertices > 2u);
    return 3u * (nvertices - 2u);

}
void Editor::polygon_geometry_generate(u32 nvertices, u32* vertices, xydrgba* geometry){
    assert(nvertices > 2u);

    Virtual_Arena_Memory vmem = arena.allocate<u32>(polygon_geometry_request);


}

void Editor::initialize(){
    arena.initialize(GIGABYTES(16u));
}

void Editor::terminate(){

    // ---- state
    // -- MODE_CREATE_POLYGON

    CREATE_POLYGON_state.vertices.free();

    // ---- storage

    for(auto& info : entity_storage.storage){
        for(auto& polygon : info.data.polygons){
            polygon.vertices.free();
            polygon.indices.free();
        }
        info.data.polygons.free();
    }
    entity_storage.free();

    arena.terminate();
}

void Editor::transition_to_mode(Editor_Mode new_mode){
    switch(state.mode){
        case MODE_CREATE_POLYGON:
            if(CREATE_POLYGON_state.vertices.size > 2u){
                Component_Reference temp_ref;
                Editor_Polygon* polygon = polygon_storage.create(temp_ref);

                polygon->vertices = CREATE_POLYGON_state.vertices;
                CREATE_POLYGON_state.vertices = array<vec2>();

                polygon->byte_color = rgba32(state.color);

                u32 cache_size = request_geometry();
                //polygon->cache =

            }else{
                CREATE_POLYGON_state.vertices.clear();

            }
            break;
    }

    switch(new_mode){
    }
    state.mode = new_mode;
}

void Editor::update_state(){
    // NOTE(hugo): update camera wrt. window
    camera.aspect_ratio = g_engine.window.aspect_ratio();

    // NOTE(hugo): update mouse position wrt. camera
    vec2 mouse_screen = g_engine.window.pixel_to_screen_coordinates(g_engine.mouse.motion.position);
    state.mouse_screen = mouse_screen;
    state.mouse_world = camera.screen_to_world_coordinates(mouse_screen);
}

void Editor::update_mode(){
    // NOTE(hugo): register mode transition
    do{
        if(state.mode == MODE_DEFAULT && g_engine.mouse.button.right.npressed){
            transition_to_mode(MODE_CREATE_POLYGON);
            break;
        }
        if(state.mode == MODE_CREATE_POLYGON && g_engine.mouse.button.right.npressed){
            transition_to_mode(MODE_DEFAULT);
            break;
        }
    }while(false);

    // NOTE(hugo): update current mode
    switch(state.mode){
        case MODE_CREATE_POLYGON:
#if 0
            if(g_engine.mouse.button.left.npressed){
                CREATE_POLYGON_state.vertices.push(state.mouse_world);

                if(CREATE_POLYGON_state.vertices.size > 2u){
                    Virtual_Arena_Memory vmem = arena.allocate<u32>(3u * (CREATE_POLYGON_state.vertices.size - 2u));
                    DEFER{ arena.free(vmem); };

                    u32* indices = (u32*)vmem.ptr;
                    triangulation_2D(CREATE_POLYGON_state.vertices.size,
                            CREATE_POLYGON_state.vertices.storage.data,
                            indices);
                    CREATE_POLYGON_state.cache = vbatcher.cache_polygon(CREATE_POLYGON_state.vertices.size,
                            CREATE_POLYGON_state.vertices.storage.data,
                            indices,
                            layer_to_depth(state.layer),
                            rgba32(state.color));
                }
            }
            break;
#endif
    }
}

void Editor::update_common(){
    // NOTE(hugo): drag
    if(g_engine.mouse.button.middle.is_down()){
        vec2 mouse_screen_prev = g_engine.window.pixel_to_screen_coordinates(g_engine.mouse.motion.previous_position);
        vec2 mouse_world_prev = camera.screen_to_world_coordinates(mouse_screen_prev);

        vec2 drag_vector = mouse_world_prev - state.mouse_world;
        camera.center += drag_vector;
    }

    // NOTE(hugo): zoom
    // * use unzoom = 1 / zoom so that the user can zoom + unzoom to the same view
    // * maintain the world position of the mouse while zooming
    if(g_engine.mouse.wheel.yticks != 0){
        float zoom_ratio = 1.1f;
        float unzoom_ratio = 1.f / zoom_ratio;

        s32 mouse_zoom_tick = g_engine.mouse.wheel.yticks;
        for(s32 izoom = 0; izoom < mouse_zoom_tick; ++izoom){
            camera.height *= unzoom_ratio;
        }
        mouse_zoom_tick = - mouse_zoom_tick;
        for(s32 iunzoom = 0; iunzoom < mouse_zoom_tick; ++iunzoom){
            camera.height *= zoom_ratio;
        }

        vec2 new_mouse_world = camera.screen_to_world_coordinates(state.mouse_screen);
        vec2 snap_vector = state.mouse_world - new_mouse_world;
        camera.center += snap_vector;
    }

    // NOTE(hugo): set cursor depending on editor state
    if(!ImGui::GetIO().WantCaptureMouse){
        switch(state.mode){
            case MODE_DEFAULT:
                g_engine.cursor.set_state(CURSOR_ARROW);
                break;
            case MODE_CREATE_POLYGON:
                g_engine.cursor.set_state(CURSOR_CROSSHAIR);
                break;
        }
    }
}

void Editor::update(){
    update_state();
    update_mode();
    update_common();
}

void Editor::render_storage(){
}

void Editor::render_mode(){
}

void Editor::render_ui(){
    char scratch[128];

    ImGui::SetNextWindowPos({0.f, 0.f});
    ImGui::Begin("Editor_State_UI", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::ColorPicker4("Color Picker", state.color.data);
    ImGui::Separator();

    ImGui::End();
}

void Editor::render(){
    {
        // NOTE(hugo): update camera unifom
        camera.aspect_ratio = g_engine.window.aspect_ratio();
        uniform_camera_2D camera_matrix = {to_std140(camera.camera_matrix())};
        g_engine.renderer.update_uniform(camera_2D, (void*)&camera_matrix);
    }

    // NOTE(hugo): render background
    {
        // NOTE(hugo): update pattern uniform
        uniform_pattern_info u_pattern_info = {{camera.center.x, camera.center.y, camera.height, camera.aspect_ratio}, 1.f};
        g_engine.renderer.update_uniform(pattern_info, (void*)&u_pattern_info);

        g_engine.renderer.use_shader(editor_pattern);
        g_engine.renderer.draw(PRIMITIVE_TRIANGLES, 0u, 3u);
    }

    render_storage();
    render_mode();
    render_ui();
    //DEV_ImGui(g_engine.window.width, g_engine.window.height);
}
