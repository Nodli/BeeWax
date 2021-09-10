#include "archecs.h"
#include "archecs.cpp"

#include "imdrawer.h"
#include "imdrawer.cpp"

struct Transform{
    vec2 pos;
};
struct Shape_Disc{
    float radius;
    u32 color;
};
void system_draw_function(void* data, const archecs::System_Param& param);
void system_animate_function(void* data, const archecs::System_Param& param);

struct ArchECS_Scene{
    ArchECS_Scene(){
        nframe = 0u;
        nentities = 0u;

        cam.center = {5.f, 5.f};
        cam.height = 10.f;
        cam.aspect_ratio = 1.f;

        dpix = 0.1f;

        drawer.create();
        entity_manager.create();

        system_draw = entity_manager.create_system<Transform, Shape_Disc>();
        system_draw.data = (void*)this;
        system_draw.update = &system_draw_function;

        system_animate = entity_manager.create_system<Transform>();
        system_animate.data = (void*)this;
        system_animate.update = &system_animate_function;

        get_engine().action_manager.register_action(0u, KEYBOARD_SPACE);
    }

    ~ArchECS_Scene(){
        drawer.destroy();
        entity_manager.destroy();
    }

    void update(){
        //DEV_Timed_Block;

        Action_Data action = get_engine().action_manager.get_action(0u);
        if(action.button.nstart){
            archecs::Archetype entity_archetype = entity_manager.create_archetype<Transform, Shape_Disc>();

            u32 nnew = 256u + random_u32_range_uniform(257u);
            for(u32 inew = 0u; inew != nnew; ++inew){
                archecs::Entity_Handle entity = entity_manager.create_entity(entity_archetype);

                Transform* transform = entity_manager.get_data<Transform>(entity);
                Shape_Disc* shape = entity_manager.get_data<Shape_Disc>(entity);

                (*shape).radius = 0.5f + random_float();
                (*shape).color = rgba32(random_float(), random_float(), random_float(), 1.f);
                (*transform).pos = {5.f + (random_float() - 0.5f) * (10.f - (*shape).radius), 5.f + (random_float() - 0.5f) * (10.f - (*shape).radius)};
            }
            nentities = nentities + nnew;
        }

        ++nframe;
        if(nframe == 60u){
            LOG_TRACE("nentities: %d", nentities);
            nframe = 0u;
        }

        entity_manager.execute_system(system_animate);
    }

    void render(){
        //DEV_Timed_Block;

        uniform_camera u_camera;
        u_camera.matrix = to_std140(mat3D_from_mat2D(cam.projection_matrix() * cam.view_matrix()));
        get_engine().render_layer.update_uniform(camera, (void*)&u_camera);

        drawer.new_frame();

        entity_manager.execute_system(system_draw);

        drawer.command_disc({5.f, 5.f}, 1.f, 0.5f, rgba32(1.f, 1.f, 1.f, 1.f), dpix);
        drawer.draw();
    }

    // ----

    u32 nframe;
    u32 nentities;

    Camera_2D cam;
    ImDrawer drawer;
    float dpix;

    archecs::Entity_Manager<Transform, Shape_Disc> entity_manager;

    archecs::System system_draw;
    archecs::System system_animate;
};

void system_draw_function(void* data, const archecs::System_Param& param){
    ArchECS_Scene* scene = (ArchECS_Scene*)data;
    Transform* transform = (Transform*)((*param.chunk).data + param.type_offsets[0u]);
    Shape_Disc* shape = (Shape_Disc*)((*param.chunk).data + param.type_offsets[1u]);

    for(u32 ientity = 0u; ientity != param.nentities; ++ientity){
        (*scene).drawer.command_disc(transform[ientity].pos, shape[ientity].radius, 0.5f, shape[ientity].color, (*scene).dpix);
    }
}

void system_animate_function(void* data, const archecs::System_Param& param){
    ArchECS_Scene* scene = (ArchECS_Scene*)data;
    Transform* transform = (Transform*)((*param.chunk).data + param.type_offsets[0u]);

    for(u32 ientity = 0u; ientity != param.nentities; ++ientity){
        transform[ientity].pos += vec2({random_float(), random_float()}) * 0.01f;
    }
}
