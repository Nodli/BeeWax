#include "imdrawer.h"
#include "imdrawer.cpp"

struct Physics_Scene{
    Physics_Scene(){
        scene_camera.position = {0.f, 0.f, 2.f};
        scene_camera.near_plane = 0.9f;
        scene_camera.far_plane = 2.1f;
        scene_camera.vertical_span = 10.f;

        render_target = get_engine().render_layer.get_render_target(1000u, 1000u);

        drawer.create();
    }
    ~Physics_Scene(){
        get_engine().render_layer.free_render_target(render_target);
        drawer.destroy();
    }
    void update(){
    }
    void render(){
        scene_camera.aspect_ratio = get_engine().window.aspect_ratio();

        uniform_camera ucamera;
        ucamera.matrix = to_std140(scene_camera.orthographic_matrix() * scene_camera.view_matrix());
        get_engine().render_layer.update_uniform(camera, (void*)&ucamera);

        uniform_transform utransform;
        get_engine().render_layer.update_uniform(transform, (void*)&utransform);

        drawer.new_frame();

        // --

        float dpix = scene_camera.vertical_span / (float)render_target.height * 0.3f;
        drawer.command_disc({0.f, 0.f}, 1.f, 0.5f, rgba32(1.f, 1.f, 0.f, 1.f), dpix);
        drawer.command_disc_arc({2.f, 2.f}, {1.f, 1.f}, 0.25f * PI, 0.5f, rgba32(0.f, 1.f, 1.f, 1.f), dpix);
        drawer.command_capsule({-2.f, 2.f}, {0.f, 2.f}, 0.25f, 0.5f, rgba32(1.f, 0.f, 1.f, 1.f), dpix);
        drawer.command_circle({-2.f, -3.f}, 0.5f, 0.25f, 0.5f, rgba32(1.f, 1.f, 1.f, 1.f), dpix);
        drawer.command_circle_arc({ 2.f, -3.f}, {1.f, 1.f}, 0.25f, 0.75f * PI, 0.5f, rgba32(0.f, 0.f, 0.f, 1.f), dpix);

        // --

        get_engine().render_layer.use_render_target(render_target);
        get_engine().render_layer.clear_render_target();
        drawer.draw();
        get_engine().render_layer.copy_render_target(render_target, get_engine().render_target);
    }

    // ----

    Camera_3D_FP scene_camera;
    Render_Target render_target;
    ImDrawer drawer;
};
