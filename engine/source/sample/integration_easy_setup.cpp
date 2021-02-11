#include "../engine/easy_setup.h"
#include "../engine/easy_setup.cpp"

struct Integration_Scene{
    void on_push(){
        camera = {{0.f, 0.f}, 2.f, g_engine.window.aspect_ratio()};
        buffer = g_engine.renderer.get_transient_buffer(sizeof(vertex_xyzrgba) * 3u);
        g_engine.renderer.format_transient_buffer(buffer, xyzrgba);
    }
    void update(){
    }
    void render(){
        uniform_camera_2D camera_matrix = {to_std140(camera.camera_matrix())};
        g_engine.renderer.update_uniform(camera_2D, (void*)&camera_matrix);

        g_engine.renderer.checkout_transient_buffer(buffer);
        vertex_xyzrgba* vert = (vertex_xyzrgba*)buffer.ptr;
        vert[0] = {{-0.5f, -0.5f, 0.f}, {1.f, 1.f, 0.f, 1.f}};
        vert[1] = {{ 0.5f, -0.5f, 0.f}, {1.f, 0.f, 1.f, 1.f}};
        vert[2] = {{ 0.0f, +0.5f, 0.f}, {0.f, 1.f, 1.f, 1.f}};
        g_engine.renderer.commit_transient_buffer(buffer);

        g_engine.renderer.use_shader(polygon_2D);
        g_engine.renderer.draw(buffer, PRIMITIVE_TRIANGLES, 0u, 3u);
    }
    void on_remove(){
        g_engine.renderer.free_transient_buffer(buffer);
    }

    Camera_2D camera;
    Transient_Buffer buffer;
};

void easy_config(){}
void* easy_setup(){
    g_engine.scene.push_scene<Integration_Scene>("Integration_Scene");
    return nullptr;
}
void easy_terminate(void* user_data){}
