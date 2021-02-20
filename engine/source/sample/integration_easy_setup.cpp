#include "../engine/easy_setup.h"
#include "../engine/easy_setup.cpp"

struct Integration_Scene{
    void on_push(){
        camera = {{0.f, 0.f}, 2.f, g_engine.window.aspect_ratio()};

        buffer = g_engine.renderer.get_transient_buffer(sizeof(vertex_xyzrgba) * 3u);
        g_engine.renderer.format(buffer, xyzrgba);

        indexed_buffer = g_engine.renderer.get_transient_buffer_indexed(sizeof(vertex_xyzrgba) * 4u, sizeof(u16) * 6u);
        g_engine.renderer.format(indexed_buffer, xyzrgba);

        song = g_engine.audio_catalog.search("song");

        vg_renderer.renderer = &g_engine.renderer;
    }
    void update(){
        if(g_engine.keyboard.space.npressed){
            if(!g_engine.audio.is_valid(song_ref)) song_ref = g_engine.audio.start(song);
            else g_engine.audio.stop(song_ref);
        }
    }
    void render(){
        vg_renderer.next_frame();

        {
            uniform_camera_2D camera_matrix = {to_std140(camera.camera_matrix())};
            g_engine.renderer.update_uniform(camera_2D, (void*)&camera_matrix);
        }

        {
            g_engine.renderer.checkout(buffer);
            vertex_xyzrgba* vert = (vertex_xyzrgba*)buffer.ptr;
            vert[0u] = {{-0.5f, -0.5f, 0.f}, {1.f, 1.f, 0.f, 1.f}};
            vert[1u] = {{ 0.5f, -0.5f, 0.f}, {1.f, 0.f, 1.f, 1.f}};
            vert[2u] = {{ 0.0f, +0.5f, 0.f}, {0.f, 1.f, 1.f, 1.f}};
            g_engine.renderer.commit(buffer);
        }

        {
            g_engine.renderer.checkout(indexed_buffer);
            vertex_xyzrgba* vert = (vertex_xyzrgba*)indexed_buffer.vptr;
            vert[0u] = {{-1.f, -0.5f, 0.1f}, {1.f, 1.f, 0.f, 1.f}};
            vert[1u] = {{-0.5f, -0.5f, 0.1f}, {1.f, 0.f, 1.f, 1.f}};
            vert[2u] = {{-0.5f, +0.5f, 0.1f}, {0.f, 1.f, 1.f, 1.f}};
            vert[3u] = {{-1.f, +0.5f, 0.1f}, {1.f, 1.f, 1.f, 1.f}};
            u16* idx = (u16*)indexed_buffer.iptr;
            idx[0u] = 0u;
            idx[1u] = 1u;
            idx[2u] = 3u;
            idx[3u] = 3u;
            idx[4u] = 1u;
            idx[5u] = 2u;
            g_engine.renderer.commit(indexed_buffer);
        }

        float dpix = (float)camera.height / (float)g_engine.window.height;
        vg_renderer.segment_round({0.5f, 0.f}, {0.5f, 0.75f}, 0.1f, 0.1f, {1.f, 1.f, 0.f, 1.f}, dpix);
        vg_renderer.segment_round({0.75f, 0.f}, {0.75f, 0.75f}, 0.1f, 0.1f, {1.f, 1.f, 0.f, 1.f}, dpix);

        float oscillator = 0.5f + 0.5f * bw::cos(2.f * PI * (float)timer_seconds() / 10.f);
        vec2 segment_end = rotated({0.5f, 0.f}, oscillator * 2.f * PI);
        vec4 segment_color = vec4({0.5f, 0.f, 0.f, 1.f}) * oscillator + vec4({0.f, 0.5f, 0.f, 1.f}) * (1.f - oscillator);
        vg_renderer.segment_round({0.f, 0.f}, segment_end, 0.1f + oscillator * 0.2f, 0.01f, segment_color, dpix);

        g_engine.renderer.use_shader(polygon_2D);
        g_engine.renderer.draw(buffer, PRIMITIVE_TRIANGLES, 0u, 3u);
        g_engine.renderer.draw(indexed_buffer, PRIMITIVE_TRIANGLES, TYPE_USHORT, 6u, 0u);
        vg_renderer.draw();
    }
    void on_remove(){
        g_engine.renderer.free_transient_buffer(buffer);
        vg_renderer.terminate();
    }

    Camera_2D camera;
    Transient_Buffer buffer = {};
    Transient_Buffer_Indexed indexed_buffer = {};

    Audio_Asset* song = nullptr;
    Audio_Reference song_ref = unknown_audio_reference;

    Vector_Graphics_Renderer vg_renderer;
};

void easy_config(){
    g_config::window_name = "integration_easy_setup";
    g_config::asset_catalog_path = "./data/asset_catalog.json";
}
void* easy_setup(){
    g_engine.scene.push_scene<Integration_Scene>("Integration_Scene");
    return nullptr;
}
void easy_terminate(void* user_data){}
