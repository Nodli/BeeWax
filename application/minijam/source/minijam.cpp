#include "imdrawer.h"
#include "imdrawer.cpp"

#include "rect_packer.h"
#include "rect_packer.cpp"

#include "imtext.h"
#include "imtext.cpp"

#include "virtentity.h"
#include "virtentity.cpp"

#include "box2d.h"

constexpr float arena_height = 10.f;
constexpr float arena_width = 10.f;
constexpr float wall_width = 0.5f;

struct Bubble_Entity : Entity{
    virtual void create(void* context);
    virtual void update(void* context);
    virtual void render(void* context);
    virtual void destroy(void* context);

    // ----

    b2Body* body;
};

struct Minijam_Scene{
    Minijam_Scene() : physics_world({-0.9f, 0.f}) {
        scene_camera.center = {0.f, arena_height * 0.5f};
        scene_camera.height = arena_height;
        scene_camera.aspect_ratio = 1.f;

        dpix = arena_height / get_engine().window.height;

        // ---- arena

        b2BodyDef wall_left_def;
        wall_left_def.position.Set(- arena_width * 0.5f, arena_height * 0.5f);

        b2PolygonShape wall_left_shape;
        wall_left_shape.SetAsBox(wall_width * 0.5f, arena_height);

        wall_left = physics_world.CreateBody(&wall_left_def);
        wall_left->CreateFixture(&wall_left_shape, 0.f);


        b2BodyDef wall_right_def;
        wall_right_def.position.Set(- arena_width * 0.5f, arena_height * 0.5f);

        b2PolygonShape wall_right_shape;
        wall_right_shape.SetAsBox(wall_width * 0.5f, arena_height);

        wall_right = physics_world.CreateBody(&wall_right_def);
        wall_right->CreateFixture(&wall_right_shape, 0.f);


        b2BodyDef wall_bottom_def;
        wall_bottom_def.position.Set(0.f, 0.f);

        b2PolygonShape wall_bottom_shape;
        wall_bottom_shape.SetAsBox(arena_width, wall_width * 0.5f);

        wall_bottom = physics_world.CreateBody(&wall_bottom_def);
        wall_bottom->CreateFixture(&wall_bottom_shape, 0.f);

        // ---- entities

        entity_manager.create();
        entity_manager.context = (void*)this;

        entity_manager.root.local_transform.depth = 0.5f;
        entity_manager.root.update_transform();

        Bubble_Entity* bubble = entity_manager.create_entity<Bubble_Entity>();
        bubble->local_transform.position = {0.f, arena.height * 0.5f};
        bubble->set_parent(&entity_manager.root);

        vec2 bubble_pos = bubble->scene_transform.position();

        b2BodyDef bubble_def;
        bubble_def.position.Set(bubble_pos.x, bubble_pos.y);

        b2CircleShape bubble_shape;
        bubble_shape.m_radius = 0.5f;

        bubble_phy = physics_world.CreateBody(&bubble_def);
        bubble_phy->CreateFixture(&bubble_shape, 0.f);
    }

    ~Minijam_Scene(){
    }

    void update(){
        bubble->local_transform.position

        entity_manager.update();
    }

    void render(){
        drawer.new_frame();

        // ---- arena

        vec2 wall_left_start = {- arena_width * 0.5f, 0.f};
        vec2 wall_left_end = {- arena_width * 0.5f, arena_height};
        drawer.command_capsule(wall_left_start, wall_left_end, wall_width * 0.5f, 1.f, rgba32(0.f, 0.f, 0.f, 1.f), dpix);

        vec2 wall_right_start = {arena_width * 0.5f, 0.f};
        vec2 wall_right_end = {arena_width * 0.5f, arena_height};
        drawer.command_capsule(wall_right_start, wall_right_end, wall_width * 0.5f, 1.f, rgba32(0.f, 0.f, 0.f, 1.f), dpix);

        // ---- entities

        entity_manager.render();

        // ---- execution

        uniform_camera u_camera;
        u_camera.matrix = to_std140(mat3D_from_mat2D(scene_camera.projection_matrix() * scene_camera.view_matrix()));
        get_engine().render_layer.update_uniform(camera, (void*)&u_camera);

        drawer.draw();
    }

    // ----

    float dpix;
    ImDrawer drawer;

    Camera_2D scene_camera;

    b2World physics_world;
    b2Body* wall_left;
    b2Body* wall_right;
    b2Body* wall_bottom;

    Bubble_Entity* bubble;
    b2Body* bubble_phy;

    Entity_Manager entity_manager;
};

void Bubble_Entity::create(void* context){
}
void Bubble_Entity::update(void* context){
    if(child)   local_transform.position += {0.f, 0.01f};
    else        local_transform.orientation += PI * 0.01f;
    update_transform();
}
void Bubble_Entity::render(void* context){
    Minijam_Scene* scene = (Minijam_Scene*)context;

    vec2 pos = scene_transform.position();
    scene->drawer.command_disc(pos, 0.5f, 0.5f, rgba32(0.f, 0.f, 1.f, 1.f), 0.01f);
}
void Bubble_Entity::destroy(void* context){
}
