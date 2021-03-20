#include "../engine/easy_setup.h"
#include "../engine/easy_setup.cpp"

#include "editor.h"

constexpr struct{
    s32 default_layer_increment = 1;
    s32 default_layer = 0u;
    s32 default_depth = 1 << 12u;
    vec4 default_color = {.5f, .5f, .5f, 1.f};

    u32 default_render_buffer_capacity = 128u;

    float default_zoom_ratio = 1.1f;
} Editor_Config;

// NOTE(hugo):
// * depth is an unsigned 32 bit value used as an unsigned 24 bit value in the depth buffer with (near)[0, 1u << 24u - 1u](far) as range
// * layer is a signed 32 bit value with values in the range (near)[1u << 12u, - 1 << 12u - 1u](far)
static u32 layer_to_depth(s32 layer){
    assert(layer > - Editor_Config.default_depth && layer < (Editor_Config.default_depth + 1u));
    return (u32)(- layer + Editor_Config.default_depth) << 8u;
}
static s32 depth_to_layer(u32 depth){
    depth = depth >> 8u;
    return - ((s32)depth - Editor_Config.default_depth);
}

enum Editor_Mode{
    MODE_SELECT,
    MODE_EDIT_POLYGON,
    MODE_CREATE_POLYGON,
};

struct Editor_Polygon{
    array<vec2> vertices;
    array<vec2> indices;
    u32 byte_color;
    u32 depth;
};

struct Editor{
    void initialize(){
        camera = {{0.f, 0.f}, 10.f, g_engine.window.aspect_ratio()};

        render_buffer = g_engine.renderer.get_transient_buffer(sizeof(vertex_xyzrgba) * Editor_Config.default_render_buffer_capacity);
        render_buffer_capacity = Editor_Config.default_render_buffer_capacity;
        g_engine.renderer.format(render_buffer, xyzrgba);

        vg_renderer.renderer = &g_engine.renderer;
    }
    void terminate(){
        vg_renderer.terminate();
        g_engine.renderer.free_transient_buffer(temp_buffer.buffer);
        for(auto& info : polygon_storage.storage){
            info.data.vertices.free();
            info.data.indices.free();
        }
        polygon_storage.free();
    }

    // ---- data

    struct{
        vec2 mouse_screen;
        vec2 mouse_world;
        Editor_Mode mode = MODE_SELECT;
        s32 layer_increment = Editor_Config.default_layer_increment;
        s32 layer = Editor_Config.default_layer;
        s32 color = Editor_Config.default_color;
    } state;
    void next_layer(){
        state.layer += state.layer_increment;
    }

    struct{
        array<vec2> vertices;
        array<vec2> indices;
    } create_polygon_state;
    void create_polygon_commit(){
        if(create_polygon_state.vertices.size > 2u){
            Component_Reference temp_ref;
            Editor_Polygon* polygon = polygon_storage.create(temp_ref);

            polygon->vertices = create_polygon_state.vertices;
            create_polygon_state.vertices = array<vec2>();

            polygon->indices = create_polygon_state.indices;
            create_polygon_state.indices = array<u32>();

            polygon->byte_color = rgba32(state.color);

            polygon->depth = layer_to_depth(state.layer);
            next_layer();

        }else{
            create_polygon_state.vertices.clear();
            create_polygon_state.indices.clear();
        }
    }

    // -- rendering

    Camera_2D camera;
    Vector_Graphics_Renderer vg_renderer;
    struct{
        u32 capacity;
        Transient_Buffer buffer;
    } temp_buffer;

    // -- storage

    Component_Storage<Editor_Polygon> polygon_storage;
};

struct Editor_Scene{
    void on_push(){ editor.initialize(); };
    void on_remove(){ editor.terminate(); };
    void update(){};
    void render(){};

    // ---- data

    Editor editor;
};

#if 0
struct Editor_Scene{
    void update(){
        // NOTE(hugo): update camera wrt. window
        camera.aspect_ratio = g_engine.window.aspect_ratio();

        // NOTE(hugo): update mouse position wrt. camera
        input_state.mouse_screen = g_engine.window.pixel_to_screen_coordinates(g_engine.mouse.motion.position);
        input_state.mouse_world = camera.screen_to_world_coordinates(input_state.mouse_screen);

        // NOTE(hugo): update the editing mode
        do{
#if 0
            if(editor_state.mode == MODE_SELECT && g_engine.mouse.button.right.npressed){
                transition_editor_mode(MODE_EDIT_POLYGON);
                break;
            }
#endif
            if(editor_state.mode == MODE_SELECT && g_engine.mouse.button.right.npressed){
                transition_editor_mode(MODE_NEW_POLYGON);
                break;
            }
            if(editor_state.mode == MODE_NEW_POLYGON && g_engine.mouse.button.right.npressed){
                transition_editor_mode(MODE_SELECT);
                break;
            }
        }while(false);

        // NOTE(hugo): update editor based on the current mode
        switch(editor_state.mode){
            case MODE_NEW_POLYGON:{
                // NOTE(hugo): add point to the current polygon ; create a new polygon if there is no current
                if(g_engine.mouse.button.left.npressed){
                    editor_state.polygon_vertices.push(input_state.mouse_world);

                    if(editor_state.polygon_vertices.size > 2u){
                        editor_state.polygon_indices.set_size(3u * (editor_state.polygon_vertices.size - 2u));
                        triangulation_2D(editor_state.polygon_vertices.size, editor_state.polygon_vertices.storage.data, editor_state.polygon_indices.storage.data);
                    }
                }
                break;
            }
        }

        // NOTE(hugo): drag
        if(g_engine.mouse.button.middle.is_down()){
            vec2 mouse_screen_prev = g_engine.window.pixel_to_screen_coordinates(g_engine.mouse.motion.previous_position);
            vec2 mouse_world_prev = camera.screen_to_world_coordinates(mouse_screen_prev);

            vec2 drag_vector = mouse_world_prev - input_state.mouse_world;
            camera.center += drag_vector;
        }

        // NOTE(hugo): zoom
        // * use unzoom = 1 / zoom so that the user can zoom + unzoom to the same view
        // * maintain the world position of the mouse while zooming
        if(g_engine.mouse.wheel.yticks != 0){
            float zoom_ratio = Editor_Config.default_zoom_ratio;
            float unzoom_ratio = 1.f / zoom_ratio;

            s32 mouse_zoom_tick = g_engine.mouse.wheel.yticks;
            for(s32 izoom = 0; izoom < mouse_zoom_tick; ++izoom){
                camera.height *= unzoom_ratio;
            }
            mouse_zoom_tick = - mouse_zoom_tick;
            for(s32 iunzoom = 0; iunzoom < mouse_zoom_tick; ++iunzoom){
                camera.height *= zoom_ratio;
            }

            vec2 new_mouse_world = camera.screen_to_world_coordinates(input_state.mouse_screen);
            vec2 snap_vector = input_state.mouse_world - new_mouse_world;
            camera.center += snap_vector;
        }

        // NOTE(hugo): set cursor depending on editor state
        if(!ImGui::GetIO().WantCaptureMouse){
            switch(editor_state.mode){
                case MODE_SELECT:{
                    g_engine.cursor.set_state(CURSOR_ARROW);
                    break;
                }
                case MODE_NEW_POLYGON:{
                    g_engine.cursor.set_state(CURSOR_CROSSHAIR);
                    break;
                }
            }
        }
    }
    void render(){
        // NOTE(hugo): update camera
        camera.aspect_ratio = g_engine.window.aspect_ratio();
        uniform_camera_2D camera_matrix = {to_std140(camera.camera_matrix())};
        g_engine.renderer.update_uniform(camera_2D, (void*)&camera_matrix);

        // NOTE(hugo): update pixel info
        uniform_pixel_info u_pixel_info = {(float)camera.height / (float)g_engine.window.height};
        g_engine.renderer.update_uniform(pixel_info, (void*)&u_pixel_info);

        // NOTE(hugo): background pattern
        uniform_pattern_info u_pattern_info = {{camera.center.x, camera.center.y, camera.height, camera.aspect_ratio}, 1.f};
        g_engine.renderer.update_uniform(pattern_info, (void*)&u_pattern_info);
        g_engine.renderer.use_shader(editor_pattern);
        g_engine.renderer.draw(PRIMITIVE_TRIANGLES, 0u, 3u);

        g_engine.renderer.checkout(render_buffer);
        vertex_xyzrgba* base_vptr = (vertex_xyzrgba*)render_buffer.ptr;
        vertex_xyzrgba* vptr = base_vptr;

        vg_renderer.new_frame();

        // NOTE(hugo): render the current polygon
        if(active_polygon()){
            u32 polygon_depth = layer_to_depth(editor_state.polygon_layer);
            u32 polygon_byte_color = rgba32(editor_state.polygon_color);

            if(editor_state.polygon_vertices.size > 2u){
                vg_renderer.simple_polygon(editor_state.polygon_vertices.size, editor_state.polygon_vertices.storage.data, editor_state.polygon_indices.storage.data, polygon_depth, polygon_byte_color, true);
            }

            u32 edit_depth = 0u;
            u32 edit_byte_color = rgba32(hsva_to_rgba(complementary_hue(rgba_to_hsva(editor_state.polygon_color))));

            for(u32 ivert = 0u; ivert != editor_state.polygon_vertices.size - 1u; ++ivert){
                const vec2& vert = editor_state.polygon_vertices[ivert];
                const vec2& next_vert = editor_state.polygon_vertices[ivert + 1u];
                *vptr++ = {{vert.x, vert.y}, edit_depth, edit_byte_color};
                *vptr++ = {{next_vert.x, next_vert.y}, edit_depth, edit_byte_color};
            }

            const vec2& vert_start = editor_state.polygon_vertices[0u];
            const vec2& vert_end = editor_state.polygon_vertices[editor_state.polygon_vertices.size - 1u];
            const vec2& mouse_vert = input_state.mouse_world;
            *vptr++ = {{vert_end.x, vert_end.y}, edit_depth, edit_byte_color};
            *vptr++ = {{mouse_vert.x, mouse_vert.y}, edit_depth, edit_byte_color};
            *vptr++ = {{mouse_vert.x, mouse_vert.y}, edit_depth, edit_byte_color};
            *vptr++ = {{vert_start.x, vert_start.y}, edit_depth, edit_byte_color};
        }

        // NOTE(hugo): render storage polygons
        for(auto& info : polygon_storage.storage){
            const Editor_Polygon& poly = info.data;
            assert(poly.vertices.size > 2u);
            assert(poly.indices.size > 2u);
            vg_renderer.simple_polygon(poly.vertices.size, poly.vertices.storage.data, poly.indices.storage.data, poly.depth, poly.byte_color, true);
        }

        g_engine.renderer.use_shader(polygon_2D);
        vg_renderer.draw();

        g_engine.renderer.use_shader(polygon_2D_norm);
        vg_renderer.draw_antialiasing();

        // TODO(hugo): DISABLE DEPTH WRITE ??

        g_engine.renderer.commit(render_buffer);
        g_engine.renderer.use_shader(polygon_2D);
        g_engine.renderer.draw(render_buffer, PRIMITIVE_LINES, 0u, vptr - base_vptr);

        // NOTE(hugo): render the GUI

        char temp_str[128];

        sprintf(temp_str, "Pointer: %f %f", input_state.mouse_world.x, input_state.mouse_world.y);
        ImGui::Begin("Editor");

        ImGui::Text(temp_str);
        ImGui::Separator();

        ImGui::ColorPicker4("ColorPicker", editor_state.polygon_color.data);
        ImGui::Separator();

        for(u32 ipoly = 0u; ipoly != polygon_storage.storage.size; ++ipoly){
            sprintf(temp_str, "Polygon: %d", ipoly);
            if(ImGui::TreeNode(temp_str)){
                const Editor_Polygon& polygon = polygon_storage.storage[ipoly].data;

                sprintf(temp_str, "Layer: %d", depth_to_layer(polygon.depth));
                ImGui::Text(temp_str);

                for(u32 ivert = 0u; ivert != polygon.vertices.size; ++ivert){
                    sprintf(temp_str, "%f %f", polygon.vertices[ivert].x, polygon.vertices[ivert].y);
                    ImGui::Text(temp_str);
                }

                ImGui::TreePop();
            }
        }

        ImGui::End();

        //DEV_ImGui(g_engine.window.width, g_engine.window.height);
    }

    // ---- data

    struct {
        Editor_Mode mode = MODE_SELECT;
        s32 layer_increment = Editor_Config.default_layer_increment;

        // -- EDITION_POLYGON

        array<vec2> polygon_vertices;
        array<u32> polygon_indices;
        s32 polygon_layer = Editor_Config.default_layer;
        vec4 polygon_color = Editor_Config.default_color;

    } editor_state;

    bool active_polygon() {
        return editor_state.polygon_vertices.size != 0u;
    }
    void next_layer() {
        editor_state.polygon_layer = editor_state.polygon_layer + editor_state.layer_increment;
    }
    void store_active_polygon(){
    }
    void transition_editor_mode(Editor_Mode new_mode){
        // NOTE(hugo): terminate current editor mode
        switch(editor_state.mode){
            case MODE_NEW_POLYGON:
                store_active_polygon();
                break;
        }

        // NOTE(hugo): setup new editor mode
        switch(new_mode){
        }

        editor_state.mode = new_mode;
    }
};
#endif

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
