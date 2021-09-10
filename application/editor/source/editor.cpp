Editor::Editor(){
    scene_camera.center = {0.f, 0.f};
    scene_camera.height = 25.f;
    scene_camera.aspect_ratio = get_engine().window.aspect_ratio();

    get_engine().action_manager.register_action(OPEN_UI, KEYBOARD_TAB);
    get_engine().action_manager.register_action(CAMERA_TRIGGER, MOUSE_BUTTON_LEFT);
    get_engine().action_manager.register_action(CAMERA_CURSOR, MOUSE_POSITION);
    get_engine().action_manager.register_action(CAMERA_ZOOM, MOUSE_WHEEL_VERTICAL);

    Action_Data camera_cursor = get_engine().action_manager.get_action(CAMERA_CURSOR);
    vec2 cursor_screen = get_engine().window.pixel_to_screen_coordinates({camera_cursor.cursor.position_x, camera_cursor.cursor.position_y});
    vec2 cursor_canvas = scene_camera.screen_to_world_coordinates(cursor_screen);

    editor_state.previous_cursor_screen = cursor_screen;
    editor_state.previous_cursor_canvas = cursor_canvas;
    editor_state.cursor_screen = cursor_screen;
    editor_state.cursor_canvas = cursor_canvas;

    module_registry.create();

    ui_state.active = true;
    ui_state.imodule = -1;

    module = nullptr;

}

Editor::~Editor(){
    if(module) bw_delete(module);
    module_registry.destroy();
}

void Editor::update(){
    // NOTE(hugo): advance editor_state
    editor_state.previous_cursor_screen = editor_state.cursor_screen;
    editor_state.previous_cursor_canvas = editor_state.cursor_canvas;

    // NOTE(hugo): input
    {
        Action_Data open_ui = get_engine().action_manager.get_action(OPEN_UI);
        if(open_ui.button.nstart) ui_state.active = !ui_state.active;

        Action_Data camera_trigger = get_engine().action_manager.get_action(CAMERA_TRIGGER);
        Action_Data camera_cursor = get_engine().action_manager.get_action(CAMERA_CURSOR);
        Action_Data camera_zoom = get_engine().action_manager.get_action(CAMERA_ZOOM);

        // NOTE(hugo): update camera position
        editor_state.cursor_screen = get_engine().window.pixel_to_screen_coordinates({camera_cursor.cursor.position_x, camera_cursor.cursor.position_y});
        editor_state.cursor_canvas = scene_camera.screen_to_world_coordinates(editor_state.cursor_screen);

        vec2 previous_cursor_canvas_reprojection = scene_camera.screen_to_world_coordinates(editor_state.previous_cursor_screen);

        if(camera_trigger.button.active) scene_camera.center -= editor_state.cursor_canvas - previous_cursor_canvas_reprojection;

        // NOTE(hugo): update camera zoom while maintining the position of the pointer on the canvas
        if(camera_zoom.axis.value != 0.f){
            constexpr float unzoom_ratio = 1.1f;
            constexpr float zoom_ratio = 1.f / unzoom_ratio;

            s32 zoom_tick = (s32)camera_zoom.axis.value;
            for(s32 itick = 0; itick < zoom_tick; ++itick){
                scene_camera.height *= zoom_ratio;
            }

            zoom_tick = - zoom_tick;
            for(s32 itick = 0; itick < zoom_tick; ++itick){
                scene_camera.height *= unzoom_ratio;
            }

            vec2 new_cursor_canvas = scene_camera.screen_to_world_coordinates(editor_state.cursor_screen);
            vec2 snap = new_cursor_canvas - editor_state.cursor_canvas;
            scene_camera.center -= snap;
        }
    }

    // NOTE(hugo): ui
    {
        if(ui_state.active){
            auto window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
            ImGui::SetNextWindowPos({0.f, 0.f});
            if(ImGui::Begin("##Module", nullptr, window_flags)){
                const char* module_name = "";
                if(ui_state.imodule != -1) module_name = module_registry[(u32)ui_state.imodule].name;

                if(ImGui::BeginCombo("##ModuleSelector", module_name)){
                    for(u32 imodule = 0u; imodule != module_registry.size; ++imodule){
                        if(ImGui::Selectable(module_registry[imodule].name, ui_state.imodule == (s32)imodule)){
                            ui_state.imodule = (s32)imodule;
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::SameLine();

                if(ImGui::Button("Switch") && ui_state.imodule != -1){
                    if(module) bw_delete(module);
                    module = module_registry[(u32)ui_state.imodule].create_module();
                }

                ImGui::Separator();
            }
            ImGui::End();
        }
    }

    // NOTE(hugo): module
    if(module) (*module).update(this);
}

void Editor::render(){
    // NOTE(hugo): background pattern
    {
        uniform_pattern_info u_pattern_info = {{scene_camera.center.x, scene_camera.center.y, scene_camera.height, scene_camera.aspect_ratio}, 1.f};
        get_engine().render_layer.update_uniform(pattern_info, (void*)&u_pattern_info);

        get_engine().render_layer.use_shader(editor_pattern);
        get_engine().render_layer.draw(PRIMITIVE_TRIANGLES, 0u, 3u);
    }

    // NOTE(hugo): camera
    {
        uniform_camera u_camera;
        u_camera.matrix = to_std140(mat3D_from_mat2D(scene_camera.projection_matrix() * scene_camera.view_matrix()));

        get_engine().render_layer.update_uniform(camera, (void*)&u_camera);
    }

    // NOTE(hugo): module
    if(module) (*module).render(this);
}
