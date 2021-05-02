using namespace bw;

static Engine* g_engine_ptr;

void set_engine_ptr(Engine* ptr){g_engine_ptr = ptr;};
Engine& get_engine(){return *g_engine_ptr;};

// ----

#include <vector>

struct ImDrawer{
    struct ImDrawTextureCommand{
        Texture texture;
        u32 buffer_index;
        u32 vertex_index;
        u32 vertex_count;
    };

    struct ImDrawVectorCommand{
        u32 buffer_index;
        u32 vertex_index;
        u32 vertex_count;
    };

    struct ImDrawBuffer{
        Transient_Buffer buffer = {};
        size_t vertex_count = 0u;
    };

    ImDrawer(){
    }

    ~ImDrawer(){
        for(auto& imbuffer : texture_buffers){
            get_engine().render_layer.free_buffer(imbuffer.buffer);
        }
    }

    void command_texture(const Texture_Asset* texture_asset, vec3 position, vec2 size){
        // NOTE(hugo): find an available buffer
        s32 buffer_index = -1;
        for(s32 ibuffer = 0u; ibuffer != texture_buffers.size(); ++ibuffer){
            u32 free_vertices = (texture_buffers[ibuffer].buffer.bytesize / sizeof(vertex_xyzuv) - texture_buffers[ibuffer].vertex_count);
            if(free_vertices > 5u){
                buffer_index = ibuffer;
                break;
            }
        }

        // NOTE(hugo): create a new one when necessary
        if(buffer_index == -1){
            ImDrawBuffer new_buffer;

            new_buffer.buffer = get_engine().render_layer.get_transient_buffer(6u * 512u * sizeof(vertex_xyzuv));
            get_engine().render_layer.format(new_buffer.buffer, xyzuv);
            get_engine().render_layer.checkout(new_buffer.buffer);

            new_buffer.vertex_count = 0u;
            buffer_index = texture_buffers.size();
            texture_buffers.push_back(new_buffer);
        }

        // NOTE(hugo): emit vertices
        ImDrawBuffer& imbuffer = texture_buffers[buffer_index];
        u32 vertex_index = imbuffer.vertex_count;
        vertex_xyzuv* vptr = (vertex_xyzuv*)imbuffer.buffer.ptr + vertex_index;

        vec2 hsize = size * 0.5f;

        *vptr++ = {{position.x - hsize.x, position.y - hsize.y, position.z}, uv32(0.f, 0.f)};
        *vptr++ = {{position.x + hsize.x, position.y - hsize.y, position.z}, uv32(1.f, 0.f)};
        *vptr++ = {{position.x + hsize.x, position.y + hsize.y, position.z}, uv32(1.f, 1.f)};

        *vptr++ = {{position.x - hsize.x, position.y - hsize.y, position.z}, uv32(0.f, 0.f)};
        *vptr++ = {{position.x + hsize.x, position.y + hsize.y, position.z}, uv32(1.f, 1.f)};
        *vptr++ = {{position.x - hsize.x, position.y + hsize.y, position.z}, uv32(0.f, 1.f)};

        imbuffer.vertex_count += 6u;

        // NOTE(hugo): queue draw command
        ImDrawTextureCommand command;
        command.texture = texture_asset->texture;
        command.buffer_index = buffer_index;
        command.vertex_index = vertex_index;
        command.vertex_count = 6u;
        texture_commands.push_back(command);
    }

    void command_quad(vec3 A, vec3 B, vec3 C, vec3 D, u32 rgba){
        // NOTE(hugo): find an available buffer
        s32 buffer_index = -1;
        for(s32 ibuffer = 0u; ibuffer != vector_buffers.size(); ++ibuffer){
            u32 free_vertices = (vector_buffers[ibuffer].buffer.bytesize / sizeof(vertex_xyzrgba) - vector_buffers[ibuffer].vertex_count);
            if(free_vertices > 5u){
                buffer_index = ibuffer;
                break;
            }
        }

        // NOTE(hugo): create a new one when necessary
        if(buffer_index == -1){
            ImDrawBuffer new_buffer;

            new_buffer.buffer = get_engine().render_layer.get_transient_buffer(6u * 1024u * sizeof(vertex_xyzrgba));
            get_engine().render_layer.format(new_buffer.buffer, xyzrgba);
            get_engine().render_layer.checkout(new_buffer.buffer);

            new_buffer.vertex_count = 0u;
            buffer_index = vector_buffers.size();
            vector_buffers.push_back(new_buffer);
        }

        // NOTE(hugo): emit vertices
        ImDrawBuffer& imbuffer = vector_buffers[buffer_index];
        u32 vertex_index = imbuffer.vertex_count;
        vertex_xyzrgba* vptr = (vertex_xyzrgba*)imbuffer.buffer.ptr + vertex_index;

        *vptr++ = {A, rgba};
        *vptr++ = {B, rgba};
        *vptr++ = {C, rgba};

        *vptr++ = {A, rgba};
        *vptr++ = {C, rgba};
        *vptr++ = {D, rgba};

        imbuffer.vertex_count += 6u;

        // NOTE(hugo): queue draw command
        ImDrawVectorCommand command;
        command.buffer_index = buffer_index;
        command.vertex_index = vertex_index;
        command.vertex_count = 6u;
        vector_commands.push_back(command);

    }

    void new_frame(){
        texture_commands.clear();
        for(auto& imbuffer : texture_buffers){
            imbuffer.vertex_count = 0u;
            get_engine().render_layer.checkout(imbuffer.buffer);
        }

        vector_commands.clear();
        for(auto& imbuffer : vector_buffers){
            imbuffer.vertex_count = 0u;
            get_engine().render_layer.checkout(imbuffer.buffer);
        }
    }

    void draw(){
        for(auto& imbuffer : texture_buffers){
            get_engine().render_layer.commit(imbuffer.buffer);
        }
        for(auto& imbuffer : vector_buffers){
            get_engine().render_layer.commit(imbuffer.buffer);
        }

        get_engine().render_layer.use_shader(polygon_tex);
        for(auto& command : texture_commands){
            get_engine().render_layer.setup_texture_unit(0u, command.texture, nearest_clamp);
            get_engine().render_layer.draw(texture_buffers[command.buffer_index].buffer, PRIMITIVE_TRIANGLES, command.vertex_index, command.vertex_count);
        }

        get_engine().render_layer.use_shader(polygon);
        for(auto& command : vector_commands){
            get_engine().render_layer.draw(vector_buffers[command.buffer_index].buffer, PRIMITIVE_TRIANGLES, command.vertex_index, command.vertex_count);
        }
    }

    // ---- data

    std::vector<ImDrawTextureCommand> texture_commands;
    std::vector<ImDrawBuffer> texture_buffers;

    std::vector<ImDrawVectorCommand> vector_commands;
    std::vector<ImDrawBuffer> vector_buffers;
};

// --

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
        get_engine().audio.start(soundtrack, true);
    }

    ~Sample_Scene(){
        get_engine().action_manager.remove_action(SHOOT_TONGUE);
        get_engine().action_manager.remove_action(SHOOT_POSITION);
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
                    get_engine().audio.start(audio[rand]);
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
                    get_engine().audio.start(eat);
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
            get_engine().audio.start(eat);
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
        drawer.command_texture(frog, {6.f, 2.f, 0.6f}, {2., 2.f});

        const Texture_Asset* lilypad = get_engine().texture_catalog.search_asset("lilypad");
        drawer.command_texture(lilypad, {5.5f, 2.f, 0.5f}, {4., 4.f});

        const Texture_Asset* background = get_engine().texture_catalog.search_asset("water");
        float offset_x = 2.f * fmodf(timer_seconds(), 10.f) / 10.f;
        float offset_y = 2.f * fmodf(timer_seconds(), 5.f) / 5.f;
        for(s32 x = -2; x != 14; x+= 2)
            for(s32 y = 0; y != 14; y += 2)
                drawer.command_texture(background, {(float)x + 1.f + offset_x, (float)y + 1.f - offset_y, 0.25f}, {2., 2.f});

        if(tongue_reload_frames){
            vec2 tongue_direction = (vec2)tongue_position - tongue_origin;
            vec2 tongue_ortho = normalized(vec2({tongue_direction.y, - tongue_direction.x}));

            vec2 tongue_end_L = (vec2)tongue_position - tongue_ortho * 0.15f;
            vec2 tongue_end_R = (vec2)tongue_position + tongue_ortho * 0.25f;

            drawer.command_quad({5.75f, 2.05f, 0.75f}, {6.25f, 2.05f, 0.75f}, {tongue_end_L.x, tongue_end_L.y, 0.75f}, {tongue_end_R.x, tongue_end_R.y, 0.75f}, rgba32({1.f, 0.f, 0.25f, 1.f}));
        }

        if(state == START_MENU){
            const Texture_Asset* frogfly = get_engine().texture_catalog.search_asset("frogfly");
            const Texture_Asset* start = get_engine().texture_catalog.search_asset("start");
            const Texture_Asset* mouse = get_engine().texture_catalog.search_asset("mouse");

            drawer.command_texture(frogfly, {6.f, 8.f, 1.f}, {1.5777f * frogfly_scale, frogfly_scale});
            drawer.command_texture(start,   {6.f, 4.f, 1.f}, {1.87f * start_scale, start_scale});
            drawer.command_texture(mouse,   {9.f, 2.f, 1.f}, {0.8837f * mouse_scale, mouse_scale});

        }else if(state == GAMEPLAY){
            const Texture_Asset* flyA = get_engine().texture_catalog.search_asset("fly");
            const Texture_Asset* flyB = get_engine().texture_catalog.search_asset("fly_wing");
            for(auto& fly : flies){
                if(fmodf((float)timer_seconds() - fly.second_start, 0.25f) > 0.125f){
                    drawer.command_texture(flyA, {fly.pos.x, fly.pos.y, 0.9f}, {2., 2.f});
                }else{
                    drawer.command_texture(flyB, {fly.pos.x, fly.pos.y, 0.9f}, {2., 2.f});
                }
            }

        }else if(state == RESTART_MENU){
            const Texture_Asset* ohno = get_engine().texture_catalog.search_asset("oh_no");
            const Texture_Asset* restart = get_engine().texture_catalog.search_asset("restart");

            drawer.command_texture(ohno,    {6.f, 8.f, 1.f}, {1.759f * ohno_scale, ohno_scale});
            drawer.command_texture(restart, {6.f, 4.f, 1.f}, {2.33f * restart_scale, restart_scale});
        }

        drawer.draw();
    }

    // ----

    Camera_3D_FP scene_camera;

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

// ----

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
    engine.scene_manager.push_scene<Sample_Scene>("Sample_Scene");

    printf("-- mainloop\n");

    engine.run();

    printf("-- engine destroy \n");

    engine.destroy();

    printf("-- memory leak detection\n");

    DEV_Memtracker_Leakcheck();

    printf("-- finished\n");

    return 0;
}
