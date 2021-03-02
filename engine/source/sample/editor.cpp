#include "../engine/easy_setup.h"
#include "../engine/easy_setup.cpp"

struct Editor_Scene{
    void on_push(){
        camera = {{0.f, 0.f}, 10.f, g_engine.window.aspect_ratio()};
        vg_renderer.renderer = &g_engine.renderer;

        DEV_Tweak(float, 1.1f, "zoom ratio");
        DEV_Tweak(float, 1.f, "ruler interval");
    }
    void update(){
        // NOTE(hugo): synchronize camera with window
        camera.aspect_ratio = g_engine.window.aspect_ratio();

        // NOTE(hugo): mouse interactions
        {
            vec2 mouse_screen = g_engine.window.pixel_to_screen_coordinates(g_engine.mouse.motion.position);
            vec2 mouse_screen_prev = g_engine.window.pixel_to_screen_coordinates(g_engine.mouse.motion.previous_position);
            mouse_world = camera.screen_to_world_coordinates(mouse_screen);
            mouse_world_prev = camera.screen_to_world_coordinates(mouse_screen_prev);

            // NOTE(hugo): mouse drag
            if(g_engine.mouse.button.left.is_down()){
                vec2 drag_vector = mouse_world_prev - mouse_world;
                camera.center += drag_vector;
            }

            // NOTE(hugo): mouse zoom
            // * use unzoom = 1 / zoom so that the user can zoom + unzoom to the same view
            // * maintain the world position of the mouse while zooming
            if(g_engine.mouse.wheel.yticks != 0){
                float zoom_ratio = DEV_Tweak(float, 1.1f, "zoom ratio");
                float unzoom_ratio = 1.f / zoom_ratio;

                s32 mouse_zoom_tick = g_engine.mouse.wheel.yticks;
                for(s32 izoom = 0; izoom < mouse_zoom_tick; ++izoom){
                    camera.height *= unzoom_ratio;
                }
                mouse_zoom_tick = - mouse_zoom_tick;
                for(s32 iunzoom = 0; iunzoom < mouse_zoom_tick; ++iunzoom){
                    camera.height *= zoom_ratio;
                }

                vec2 new_mouse_world = camera.screen_to_world_coordinates(mouse_screen);
                vec2 snap_vector = mouse_world - new_mouse_world;
                camera.center += snap_vector;
            }
        }
    }
    void render(){
        // NOTE(hugo): update camera
        camera.aspect_ratio = g_engine.window.aspect_ratio();
        uniform_camera_2D camera_matrix = {to_std140(camera.camera_matrix())};
        g_engine.renderer.update_uniform(camera_2D, (void*)&camera_matrix);

        uniform_pattern_info u_pattern_info = {{camera.center.x, camera.center.y, camera.height, camera.aspect_ratio}, DEV_Tweak(float, 1.f, "ruler interval")};
        g_engine.renderer.update_uniform(pattern_info, (void*)&u_pattern_info);
        g_engine.renderer.use_shader(editor_pattern);
        g_engine.renderer.draw(PRIMITIVE_TRIANGLES, 0u, 3u);

        vg_renderer.new_frame();
        float dpix = (float)camera.height / (float)g_engine.offscreen_target.height;
        g_engine.renderer.use_shader(polygon_2D);
        vg_renderer.draw();

        char mouse_position_str[128];
        sprintf(mouse_position_str, "Pointer: %f %f", mouse_world.x, mouse_world.y);
        ImGui::Begin("Info");
        ImGui::Text(mouse_position_str);
        ImGui::Separator();
        ImGui::End();

        DEV_ImGui(g_engine.window.width, g_engine.window.height);
    }
    void on_remove(){
    }

    // ---- data

    Vector_Graphics_Renderer vg_renderer;

    // -- input

    vec2 mouse_world;
    vec2 mouse_world_prev;

    // -- GUI

    Camera_2D camera;

};

void easy_config(){
    g_config::window_name = "Editor";
    g_config::window_width = 1280;
    g_config::window_height = 720;
    g_config::asset_catalog_path = "./data/asset_catalog.json";
}
void* easy_setup(){
    g_engine.scene.push_scene<Editor_Scene>("Editor_Scene");
    return nullptr;
}
void easy_terminate(void* user_data){}
