void polygon_bytesize(u32 nvertices, u32& nindices, size_t& vbytesize, size_t& ibytesize){
    assert(nvertices > 2u);

    nindices = 3u * (nvertices - 2u);
    vbytesize = sizeof(vertex_xydrgba) * nvertices;
    ibytesize = sizeof(u32) * nindices;
}

void polygon_generate(u32 nvertices, vec2* vertices, u32 color_rgba, u32 base_index, vertex_xydrgba* out_vertices, u32* out_indices, Virtual_Arena& arena){
    assert(nvertices > 2u);

    u32 nindices = 3u * (nvertices - 2u);
    Virtual_Arena_Memory vmem = arena.allocate<u32>(nindices);
    DEFER{ arena.free(vmem); };
    u32* iptr = (u32*)vmem.ptr;

    triangulation_2D(nvertices, vertices, iptr);

    for(u32 ivert = 0u; ivert != nvertices; ++ivert){
        out_vertices[ivert] = {vertices[ivert], 0.f, color_rgba};
    }

    memcpy(out_indices, iptr, nindices * sizeof(u32));
}

void Editor::initialize(){
    arena.initialize(GIGABYTES(16u));
}

void Editor::terminate(){

    // ---- state

    terminate_mode(state.mode);

    // ---- storage

    for(auto& info : entity_storage.storage){
        for(auto& polygon : info.data.polygons){
            polygon.vertices.free();
        }
        info.data.polygons.free();
    }
    entity_storage.free();

    arena.terminate();
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
            CREATE_POLYGON_state.vertices[0u] = state.mouse_world;
            if(g_engine.mouse.button.left.npressed){
                CREATE_POLYGON_state.vertices.push(state.mouse_world);
            }
            break;
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
    uniform_transform_2D default_transform;
    g_engine.renderer.update_uniform(transform_2D, (void*)&default_transform);

    for(auto& component : entity_storage.storage){
        for(auto& polygon : component.data.polygons){
            u32 nindices = polygon.buffer.ibytesize / sizeof(u32);

            g_engine.renderer.use_shader(polygon_2D);
            g_engine.renderer.draw(polygon.buffer, PRIMITIVE_TRIANGLES, TYPE_UINT, nindices, 0u);
        }
    }
}

void Editor::render_mode(){
    switch(state.mode){
        case MODE_CREATE_POLYGON:
            auto buffer_reserve = [&](size_t vbytesize, size_t ibytesize){
                if(vbytesize > CREATE_POLYGON_state.buffer.vbytesize
                || ibytesize > CREATE_POLYGON_state.buffer.ibytesize){
                    g_engine.renderer.free_buffer(CREATE_POLYGON_state.buffer);
                    CREATE_POLYGON_state.buffer = g_engine.renderer.get_transient_buffer_indexed(vbytesize, ibytesize);
                    g_engine.renderer.format(CREATE_POLYGON_state.buffer, xydrgba);
                }
            };

            // NOTE(hugo): draw a line
            if(CREATE_POLYGON_state.vertices.size == 2u){

                // TODO(hugo):

            // NOTE(hugo): draw the polygon
            }else if(CREATE_POLYGON_state.vertices.size > 3u){

                u32 byte_color = rgba32(state.color);

                u32 nindices;
                size_t vbytesize;
                size_t ibytesize;
                polygon_bytesize(CREATE_POLYGON_state.vertices.size, nindices, vbytesize, ibytesize);

                buffer_reserve(vbytesize, ibytesize);

                g_engine.renderer.checkout(CREATE_POLYGON_state.buffer);
                vertex_xydrgba* vptr = (vertex_xydrgba*)CREATE_POLYGON_state.buffer.vptr;
                u32* iptr = (u32*)CREATE_POLYGON_state.buffer.iptr;
                polygon_generate(CREATE_POLYGON_state.vertices.size, CREATE_POLYGON_state.vertices.storage.data, byte_color, 0u, vptr, iptr, arena);
                g_engine.renderer.commit(CREATE_POLYGON_state.buffer);

                uniform_transform_2D default_transform;
                g_engine.renderer.update_uniform(transform_2D, (void*)&default_transform);

                g_engine.renderer.use_shader(polygon_2D);
                g_engine.renderer.draw(CREATE_POLYGON_state.buffer, PRIMITIVE_TRIANGLES, TYPE_UINT, nindices, 0u);
            }
        break;
    }
}

void Editor::render_ui(){
    char scratch[128];

    auto window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;

    ImGui::SetNextWindowPos({0.f, 0.f});
    if(ImGui::Begin("Editor", nullptr, window_flags)){
        ImGui::ColorPicker4("Color Picker", state.color.data);

        ImGui::Separator();

        ImGui::LabelText("Bullet");

        ImGui::Button("New");
        ImGui::SameLine();
        ImGui::Button("New Entity");

        static s32 ibutton = -1;

        ImGui::RadioButton("", &ibutton, 0);
    }
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

void Editor::initialize_mode(Editor_Mode mode){
    switch(mode){
        case MODE_CREATE_POLYGON:
            assert(CREATE_POLYGON_state.vertices.size == 0u);

            CREATE_POLYGON_state.vertices.set_size(1u);

            CREATE_POLYGON_state.buffer = g_engine.renderer.get_transient_buffer_indexed(1024u * sizeof(vertex_xydrgba), 3u * 1024u * sizeof(u32));
            g_engine.renderer.format(CREATE_POLYGON_state.buffer, xydrgba);

            break;
    }
}

void Editor::terminate_mode(Editor_Mode mode){
    switch(mode){
        case MODE_CREATE_POLYGON:
            if(CREATE_POLYGON_state.vertices.size > 3u){
                u32 nvertices = CREATE_POLYGON_state.vertices.size - 1u;

                Storage_Polygon polygon;
                polygon.vertices.set_size(nvertices);
                memcpy(polygon.vertices.storage.data, &CREATE_POLYGON_state.vertices[1u], nvertices * sizeof(vec2));

                u32 byte_color = rgba32(state.color);
                polygon.byte_color = byte_color;

                u32 nindices;
                size_t vbytesize;
                size_t ibytesize;
                polygon_bytesize(nvertices, nindices, vbytesize, ibytesize);

                polygon.buffer = g_engine.renderer.get_static_buffer_indexed(vbytesize, ibytesize);
                g_engine.renderer.format(polygon.buffer, xydrgba);
                g_engine.renderer.checkout(polygon.buffer);

                vertex_xydrgba* vptr = (vertex_xydrgba*)polygon.buffer.vptr;
                u32* iptr = (u32*)polygon.buffer.iptr;

                polygon_generate(nvertices, polygon.vertices.storage.data, byte_color, 0u, vptr, iptr, arena);

                Component_Reference temp_ref;
                Entity* entity = entity_storage.create(temp_ref);

                entity->polygons.push(polygon);
            }

            CREATE_POLYGON_state.vertices.free();
            g_engine.renderer.free_buffer(CREATE_POLYGON_state.buffer);

            break;
    }
}

void Editor::transition_to_mode(Editor_Mode new_mode){
    terminate_mode(state.mode);
    initialize_mode(new_mode);
    state.mode = new_mode;
}
