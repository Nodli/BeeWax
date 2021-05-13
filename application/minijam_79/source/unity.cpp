using namespace bw;

static Engine* g_engine_ptr;

void set_engine_ptr(Engine* ptr){g_engine_ptr = ptr;};
Engine& get_engine(){return *g_engine_ptr;};

// ----

#include "imdrawer.h"
#include "imdrawer.cpp"

#include "physics_2D.h"

//#include "entity_manager.h"

// --

#if 0
struct Entity{
    Collider col;
    bool being_collided_with;
};

#define FOR_EACH_ENTITY_TYPE(FUNCTION)  \
FUNCTION(Entity)

struct Physics_Scene{
    Physics_Scene(){
        A.position = {0.f, 0.f};
        A.type = Collider::STATIC;
        A.collider_data = nullptr;
        A.collision_callback = nullptr;
        A.shape.hsize = {2.f, 2.f};

        B.position = {4.f, 0.f};
        B.type = Collider::DYNAMIC;
        B.collider_data = nullptr;
        B.collision_callback = nullptr;
        B.shape.hsize = {1.f, 1.f};

        colcontext.register_collider(A);
        colcontext.register_collider(B);

        scene_camera.position = {0.f, 0.f, 2.f};
        scene_camera.near_plane = 0.9f;
        scene_camera.far_plane = 2.1f;
        scene_camera.vertical_span = 12.f;

        get_engine().action_manager.register_action(0u, MOUSE_POSITION);
    }
    ~Physics_Scene(){
        colcontext.unregister_collider(A);
        colcontext.unregister_collider(B);
    }
    void update(){
        Action_Data action = get_engine().action_manager.get_action(0u);
        vec2 mouse_position_screen = get_engine().window.pixel_to_screen_coordinates({action.cursor.position_x, action.cursor.position_y});
        vec2 mouse_position = vec2({0.f, 0.f}) + mouse_position_screen * 6.f;

        B.position = mouse_position;

        colcontext.update();
    }
    void render(){
        scene_camera.aspect_ratio = get_engine().window.aspect_ratio();

        uniform_camera ucamera;
        mat4 camera_matrix = scene_camera.orthographic_matrix() * scene_camera.view_matrix();
        ucamera.matrix = to_std140(camera_matrix);
        get_engine().render_layer.update_uniform(camera, (void*)&ucamera);

        uniform_transform utransform;
        get_engine().render_layer.update_uniform(transform, (void*)&utransform);

        drawer.new_frame();

        debug_draw_collision_context(colcontext, drawer, 1.f, rgba32(1.f, 0.f, 0.f, 1.f));

        drawer.draw();
    }

    // ----

    Collider A;
    Collider B;

    bool collision;

    Camera_3D_FP scene_camera;

    ImDrawer drawer;
    Collision_Context colcontext;
};
#endif

#if 0
struct Fly{
    vec2 pos;
    float second_start;
};

enum Input_Action : u32{
    SHOOT_TONGUE,
    SHOOT_POSITION,
};

enum Game_State : u32{
    START_MENU,
    GAMEPLAY,
    RESTART_MENU,
};

constexpr vec2 tongue_origin = {6.f, 2.05f};
constexpr u32 tongue_reload = 30u;

constexpr float frogfly_scale = 7.f;
constexpr float start_scale = 2.f;
constexpr float ohno_scale = 7.f;
constexpr float restart_scale = 2.f;
constexpr float mouse_scale = 3.f;

constexpr u32 delay_frames = 30u;

struct Sample_Scene{
    Sample_Scene(){
        scene_camera.position = {6.f, 6.f, 2.f};
        scene_camera.near_plane = 0.9f;
        scene_camera.far_plane = 2.1f;
        scene_camera.vertical_span = 12.f;

        state = START_MENU;

        fly_spawn_timer_buffer = 60u;
        fly_spawn_timer_buffer_harder = 30u;
        fly_spawn_timer = 120u;

        fly_on_tongue_index = -1;
        tongue_reload_frames = 0u;

        menu_delay_frames = delay_frames;

        get_engine().action_manager.register_action(SHOOT_TONGUE, MOUSE_BUTTON_LEFT);
        get_engine().action_manager.register_action(SHOOT_POSITION, MOUSE_POSITION);

        const Audio_Asset* soundtrack = get_engine().audio_catalog.search_asset("soundtrack");
        soundtrack_channel = get_engine().audio.start(soundtrack, LOOP);
    }

    ~Sample_Scene(){
        get_engine().action_manager.remove_action(SHOOT_TONGUE);
        get_engine().action_manager.remove_action(SHOOT_POSITION);

        get_engine().audio.stop(soundtrack_channel);
    }

    void update(){
        tongue_position.tick();
        if(tongue_reload_frames < 5u){
            Action_Data shoot_tongue = get_engine().action_manager.get_action(SHOOT_TONGUE);
            if(shoot_tongue.button.nstart){

                {
                    const Audio_Asset* tA = get_engine().audio_catalog.search_asset("tongue_A");
                    const Audio_Asset* tB = get_engine().audio_catalog.search_asset("tongue_B");
                    const Audio_Asset* tC = get_engine().audio_catalog.search_asset("tongue_C");

                    const Audio_Asset* audio[3] = {tA, tB, tC};
                    u32 rand = random_u32_range_uniform(3u);
                    rand = min(2u, rand);
                    get_engine().audio.start(audio[rand], PLAY);
                }

                Action_Data shoot_position_action = get_engine().action_manager.get_action(SHOOT_TONGUE);
                vec2 shoot_position_screen = get_engine().window.pixel_to_screen_coordinates({shoot_position_action.cursor.position_x, shoot_position_action.cursor.position_y});
                vec2 shoot_position = vec2({6.f, 6.f}) + shoot_position_screen * 6.f;

                tongue_position.interpolate({shoot_position.x, shoot_position.y}, tongue_origin, 2u * tongue_reload, false);

                tongue_reload_frames = tongue_reload;

                if(flies.size() && fly_on_tongue_index != -1){
                    swap(flies[fly_on_tongue_index], flies[flies.size() - 1u]);
                    flies.pop_back();
                    fly_on_tongue_index = -1;

                    const Audio_Asset* eat = get_engine().audio_catalog.search_asset("eat");
                    get_engine().audio.start(eat, PLAY);
                }

                for(u32 ifly = 0; ifly != flies.size(); ++ifly){
                    if(length(flies[ifly].pos - shoot_position) < 1.5f){
                        fly_on_tongue_index = ifly;


                        break;
                    }
                }
            }
        }

        if(fly_on_tongue_index != -1 && tongue_reload_frames == 0u){
            swap(flies[fly_on_tongue_index], flies[flies.size() - 1u]);
            flies.pop_back();
            fly_on_tongue_index = -1;

            const Audio_Asset* eat = get_engine().audio_catalog.search_asset("eat");
            get_engine().audio.start(eat, PLAY);
        }
        if(tongue_reload_frames) --tongue_reload_frames;

        if(state == START_MENU && menu_delay_frames == 0u){
            Action_Data shoot_position_action = get_engine().action_manager.get_action(SHOOT_TONGUE);
            vec2 shoot_position_screen = get_engine().window.pixel_to_screen_coordinates({shoot_position_action.cursor.position_x, shoot_position_action.cursor.position_y});
            vec2 shoot_position = vec2({6.f, 6.f}) + shoot_position_screen * 6.f;

            float min_x = 6.f - 0.5f * 1.87f * start_scale;
            float max_x = 6.f + 0.5f * 1.87f * start_scale;

            float min_y = 4.f - 0.5f * start_scale;
            float max_y = 4.f + 0.5f * start_scale;

            if(shoot_position.x > min_x && shoot_position.x < max_x
            && shoot_position.y > min_y && shoot_position.y < max_y){
                state = GAMEPLAY;
            }

        }else if(state == GAMEPLAY){
            for(auto& fly : flies){
                if(fly.pos.x < -1.f || fly.pos.x > 13.f || fly.pos.y < -1.f || fly.pos.y > 13.f){
                    menu_delay_frames = delay_frames;
                    state = RESTART_MENU;
                    fly_on_tongue_index = -1;
                    flies.clear();
                    break;
                }
            }

            if(fly_spawn_timer-- == 0u){
                float random_x = 3.f + 6.f * random_float();
                float random_y = 4.f + 4.f * random_float();
                flies.push_back({{random_x, random_y}, (float)timer_seconds()});
                fly_spawn_timer = fly_spawn_timer_buffer_harder + fly_spawn_timer_buffer + random_u32_range_uniform(60u);

                if(fly_spawn_timer_buffer) fly_spawn_timer_buffer -= min(fly_spawn_timer_buffer, 5u);
                if(fly_spawn_timer_buffer == 0u && fly_spawn_timer_buffer_harder) fly_spawn_timer_buffer_harder -= min(fly_spawn_timer_buffer, 1u);
            }

            for(auto& fly : flies){
                float random_x = 0.4f * (2.f * random_float() - 1.f);
                float random_y = 0.4f * (2.f * random_float() - 1.f);
                fly.pos += {random_x, random_y};
            }

            if(fly_on_tongue_index != -1) flies[fly_on_tongue_index].pos = tongue_position;

        }else if(state == RESTART_MENU && menu_delay_frames == 0u){
            Action_Data shoot_position_action = get_engine().action_manager.get_action(SHOOT_TONGUE);
            vec2 shoot_position_screen = get_engine().window.pixel_to_screen_coordinates({shoot_position_action.cursor.position_x, shoot_position_action.cursor.position_y});
            vec2 shoot_position = vec2({6.f, 6.f}) + shoot_position_screen * 6.f;

            float min_x = 6.f - 0.5f * 2.33f * restart_scale;
            float max_x = 6.f + 0.5f * 2.33f * restart_scale;

            float min_y = 4.f - 0.5f * restart_scale;
            float max_y = 4.f + 0.5f * restart_scale;

            if(shoot_position.x > min_x && shoot_position.x < max_x
            && shoot_position.y > min_y && shoot_position.y < max_y){
                state = GAMEPLAY;
            }
        }

        if(menu_delay_frames) --menu_delay_frames;
    }

    void render(){
        scene_camera.aspect_ratio = get_engine().window.aspect_ratio();

        uniform_camera ucamera;
        mat4 camera_matrix = scene_camera.orthographic_matrix() * scene_camera.view_matrix();
        ucamera.matrix = to_std140(camera_matrix);
        get_engine().render_layer.update_uniform(camera, (void*)&ucamera);

        uniform_transform utransform;
        get_engine().render_layer.update_uniform(transform, (void*)&utransform);

        drawer.new_frame();

        const Texture_Asset* frog = get_engine().texture_catalog.search_asset("frog");
        drawer.command_sprite(frog, {6.f, 2.f}, {2., 2.f}, 0.6f);

        const Texture_Asset* lilypad = get_engine().texture_catalog.search_asset("lilypad");
        drawer.command_sprite(lilypad, {5.5f, 2.f}, {4., 4.f}, 0.5f);

        const Texture_Asset* background = get_engine().texture_catalog.search_asset("water");
        float offset_x = 2.f * fmodf(timer_seconds(), 10.f) / 10.f;
        float offset_y = 2.f * fmodf(timer_seconds(), 5.f) / 5.f;
        for(s32 x = -2; x != 14; x+= 2)
            for(s32 y = 0; y != 14; y += 2)
                drawer.command_sprite(background, {(float)x + 1.f + offset_x, (float)y + 1.f - offset_y}, {2., 2.f}, 0.25f);

        if(tongue_reload_frames){
            vec2 tongue_direction = (vec2)tongue_position - tongue_origin;
            vec2 tongue_ortho = normalized(vec2({tongue_direction.y, - tongue_direction.x}));

            vec2 tongue_end_L = (vec2)tongue_position - tongue_ortho * 0.15f;
            vec2 tongue_end_R = (vec2)tongue_position + tongue_ortho * 0.25f;

            drawer.command_quad({5.75f, 2.05f}, {6.25f, 2.05f}, {tongue_end_L.x, tongue_end_L.y}, {tongue_end_R.x, tongue_end_R.y}, 0.75f, rgba32({1.f, 0.f, 0.25f, 1.f}));
        }

        if(state == START_MENU){
            const Texture_Asset* frogfly = get_engine().texture_catalog.search_asset("frogfly");
            const Texture_Asset* start = get_engine().texture_catalog.search_asset("start");
            const Texture_Asset* mouse = get_engine().texture_catalog.search_asset("mouse");

            drawer.command_sprite(frogfly, {6.f, 8.f}, {1.5777f * frogfly_scale, frogfly_scale}, 1.f);
            drawer.command_sprite(start,   {6.f, 4.f}, {1.87f * start_scale, start_scale}, 1.f);
            drawer.command_sprite(mouse,   {9.f, 2.f}, {0.8837f * mouse_scale, mouse_scale}, 1.f);

        }else if(state == GAMEPLAY){
            const Texture_Asset* flyA = get_engine().texture_catalog.search_asset("fly");
            const Texture_Asset* flyB = get_engine().texture_catalog.search_asset("fly_wing");
            for(auto& fly : flies){
                if(fmodf((float)timer_seconds() - fly.second_start, 0.25f) > 0.125f){
                    drawer.command_sprite(flyA, {fly.pos.x, fly.pos.y}, {2., 2.f}, 0.9f);
                }else{
                    drawer.command_sprite(flyB, {fly.pos.x, fly.pos.y}, {2., 2.f}, 0.9f);
                }
            }

        }else if(state == RESTART_MENU){
            const Texture_Asset* ohno = get_engine().texture_catalog.search_asset("oh_no");
            const Texture_Asset* restart = get_engine().texture_catalog.search_asset("restart");

            drawer.command_sprite(ohno,    {6.f, 8.f}, {1.759f * ohno_scale, ohno_scale}, 1.f);
            drawer.command_sprite(restart, {6.f, 4.f}, {2.33f * restart_scale, restart_scale}, 1.f);
        }

        drawer.draw();
    }

    // ----

    Camera_3D_FP scene_camera;

    Audio_Channel soundtrack_channel;

    Game_State state;

    u32 fly_spawn_timer_buffer;
    u32 fly_spawn_timer_buffer_harder;
    u32 fly_spawn_timer;
    std::vector<Fly> flies;

    s32 fly_on_tongue_index;
    u32 tongue_reload_frames;
    Tween<vec2> tongue_position;

    u32 menu_delay_frames;

    ImDrawer drawer;
};
#endif

// ----

#if 0
int main(int argc, char* argv[]){
    Engine_Config configuration;
    configuration.window_name = "Engine";
    configuration.window_width = 1000u;
    configuration.window_height = 1000u;
    configuration.render_target_samples = 1u;
    configuration.asset_catalog_path = "./data/asset_catalog.json";

    Engine engine;
    set_engine_ptr(&engine);

    printf("-- starting\n");

    engine.create(configuration);
    //engine.scene_manager.push_scene<Sample_Scene>("Sample_Scene");
    engine.scene_manager.push_scene<Physics_Scene>("Physics_Scene");

    printf("-- mainloop\n");

    engine.run();

    printf("-- engine destroy \n");

    engine.destroy();

    printf("-- finished\n");

    DEV_Memtracker_Leakcheck();

    return 0;
}
#endif

#include "ecs.h"

int main(int argc, char* argv[]){
    Engine_Config configuration;
    configuration.window_name = "Engine";
    configuration.window_width = 1000u;
    configuration.window_height = 1000u;
    configuration.render_target_samples = 1u;
    configuration.asset_catalog_path = "./data/asset_catalog.json";

    Engine engine;
    set_engine_ptr(&engine);

    printf("-- starting\n");

    engine.create(configuration);

    printf("-- mainloop\n");

    engine.run();
    do_the_thing();

    printf("-- engine destroy \n");

    engine.destroy();

    printf("-- finished\n");

    DEV_Memtracker_Leakcheck();

    return 0;
}
