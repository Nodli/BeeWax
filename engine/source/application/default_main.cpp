using namespace bw;

int main(int argc, char* argv[]){

	// ---- initialization ---- //

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
		printf("Failed SDL_Init() %s\n", SDL_GetError());
	}

    stbi_set_flip_vertically_on_load(true);

    // ---- window

    const char* window_name = "default_main";
	Window_Settings window_settings;
	window_settings.name = window_name;
	window_settings.width = 1280;
	window_settings.height = 720;
	window_settings.mode = Window_Settings::mode_windowed;
	//window_settings.synchronization = Window_Settings::synchronize_adaptive;
	//window_settings.buffering = Window_Settings::buffering_double;

	Window window;
	window.initialize(window_settings);

    Frame_Timing timing;
    timing.initialize(60u);

    // ---- rendering

	gl3wInit();
	glClearColor(0.f, 0.f, 0.f, 1.f);

    Renderer renderer;
    renderer.setup_resources();

    //DEV_Debug_Renderer;

    Font_Renderer font_renderer;
    font_renderer.width = window.width;
    font_renderer.height = window.height;
    font_renderer.renderer = &renderer;

    // ---- audio

    Audio_Player audio;
    audio.setup();

    // ---- texture animation

    Texture_Animation_Player texture_animation;

    // ---- particles

    Particle_Emitter particle_emitter;

    particle_emitter.shape = Particle_Emitter::Disc;
    particle_emitter.desc.circle.center = {0.f, 0.f};
    particle_emitter.desc.circle.radius = 0.1f;
    particle_emitter.duration_min_range[0] = 60u;
    particle_emitter.duration_min_range[1] = 1u;
    particle_emitter.theta_min_range[0] = 0.f;
    particle_emitter.theta_min_range[1] = TWO_PI<float>;
    particle_emitter.velocity_min_range[0] = 0.005f;
    particle_emitter.velocity_min_range[1] = 0.f;
    particle_emitter.angular_velocity_min_range[0] = 0.f;
    particle_emitter.angular_velocity_min_range[1] = 0.f;
    particle_emitter.size_min_range[0] = 0.01f;
    particle_emitter.size_min_range[1] = 0.01f;
    particle_emitter.particles_per_frame = -10;

    // ---- assets

    Asset_Manager asset;
    asset.audio_player = &audio;
    asset.renderer = &renderer;
    asset.from_asset_file(asset_path() / "asset.txt");

    // ---- input

    Keyboard_State keyboard = {};
    keyboard.initialize();

    Mouse_State mouse;

    // ---- state

	bool running = true;

    // ---- developper tools

    DEV_setup();

    // ---- custom setup

    u8* perlin_value;
    u8* perlin_dx;
    u8* perlin_dy;
    void* perlin_memory = generate_noise_derivatives_textures<perlin_noise_and_derivatives>(256u, 256u, 3u, {0.f, 0.f}, 1.f / 32.f, perlin_value, perlin_dx, perlin_dy);
    Texture_ID perlin_texture = renderer.get_texture(TEXTURE_FORMAT_RGB, 256u, 256u, TYPE_UBYTE, perlin_value);
    Texture_ID perlin_dx_texture = renderer.get_texture(TEXTURE_FORMAT_RGB, 256u, 256u, TYPE_UBYTE, perlin_dx);
    Texture_ID perlin_dy_texture = renderer.get_texture(TEXTURE_FORMAT_RGB, 256u, 256u, TYPE_UBYTE, perlin_dy);
    free(perlin_memory);

    u8* simplex_value = generate_noise_texture<simplex_noise>(256u, 256u, 3u, {0.f, 0.f}, 1.f / 32.f);
    Texture_ID simplex_texture = renderer.get_texture(TEXTURE_FORMAT_RGB, 256u, 256u, TYPE_UBYTE, simplex_value);
    free(simplex_value);

    Tween<vec2> red_square_position = {{0.f, 0.f}};

    Texture_Animation_Playing_ID anim = unknown_texture_animation_playing;

	// ---- main loop ---- //

	while(running){
        //DEV_LOG_frame_duration;

        // ---- update

        u32 nupdates = timing.nupdates_before_render();
        for(u32 iupdate = 0; iupdate != nupdates; ++iupdate){
            SDL_Event event;
            while(SDL_PollEvent(&event)){
                switch(event.type){
                    case SDL_QUIT:
                    {
                        running = false;
                        break;
                    }
                    default:
                        break;
                }

                keyboard.register_event(event);
                mouse.register_event(event);
            }

            particle_emitter.update();

            if(keyboard.space.npressed > 0){
                audio.start_playing(asset.get_audio("second"));
            }

            red_square_position.tick();
            if(keyboard.arrow_up.npressed > 0){
                red_square_position.from({-1.f, -1.f}, {1.f, 1.f}, 120u);
            }

            if(keyboard.arrow_down.npressed > 0){
                if(anim != unknown_texture_animation_playing){
                    texture_animation.stop_playing(anim);
                }
                anim = texture_animation.start_playing(asset.get_texture_animation("running_woman"));
            }

            if(keyboard.arrow_left.npressed > 0){
                particle_emitter.burst(10u);
            }

            if(keyboard.function_F1.npressed > 0){
                BEEWAX_INTERNAL::DEV_reparse_tweakables();
            }

            // ---- setup the next game frame

            keyboard.next_frame();
            mouse.next_frame();
            audio.mix_next_frame();
            texture_animation.next_frame();
        }

        // ---- render

        renderer.start_frame();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Camera_2D camera;
        camera.center = {0.f, 0.f};
        camera.height = 2.f;
        camera.aspect_ratio = window.aspect_ratio();
        mat3 camera_mat = camera.camera_matrix();

        ((uniform_camera_2D*)renderer.get_uniform(camera_2D))->matrix = to_std140(camera_mat);
        renderer.submit_uniform(camera_2D);

        // NOTE(hugo): red square
        if(true)
        {
            Vertex_Batch_ID vertex_batch = renderer.get_vertex_batch(xyrgba, PRIMITIVE_TRIANGLE_STRIP);
            vertex_xyrgba* vertices = (vertex_xyrgba*)renderer.get_vertices(vertex_batch, 4u);
            vertices[0] = {{-0.1f, -0.1f}, {1.f, 0.f, 0.f, 1.f}};
            vertices[1] = {{0.1f, -0.1f}, {1.f, 0.f, 0.f, 1.f}};
            vertices[2] = {{-0.1f, 0.1f}, {1.f, 0.f, 0.f, 1.f}};
            vertices[3] = {{0.1f, 0.1f}, {1.f, 0.f, 0.f, 1.f}};

            vertices[0].vposition += red_square_position;
            vertices[1].vposition += red_square_position;
            vertices[2].vposition += red_square_position;
            vertices[3].vposition += red_square_position;

            renderer.use_shader(polygon_2D);
            renderer.submit_vertex_batch(vertex_batch);
            renderer.free_vertex_batch(vertex_batch);
        }

        // NOTE(hugo): font rendering
        if(true)
        {
            vec2 baseline = {300.f, 300.f};
            float baseline_end_x = font_renderer.render_string(asset.get_font("roboto"), "Asobisof", baseline.x, baseline.y, 250.f);

            float underline_height = 10.f;
            Vertex_Batch_ID vertex_batch = renderer.get_vertex_batch(xyrgba, PRIMITIVE_TRIANGLE_STRIP);
            vertex_xyrgba* vertices = (vertex_xyrgba*)renderer.get_vertices(vertex_batch, 4u);
            ivec2 imin = {(int)baseline.x, (int)(baseline.y - 2.f * underline_height)};
            ivec2 imax = {(int)baseline_end_x, (int)(baseline.y - underline_height)};
            vec2 min = window.pixel_to_screen_coordinates(imin);
            vec2 max = window.pixel_to_screen_coordinates(imax);
            vertices[0] = {min, {1.f, 0.f, 0.f, 1.f}};
            vertices[1] = {{max.x, min.y}, {1.f, 0.f, 0.f, 1.f}};
            vertices[2] = {{min.x, max.y}, {1.f, 0.f, 0.f, 1.f}};
            vertices[3] = {max, {1.f, 0.f, 0.f, 1.f}};

            renderer.use_shader(screen_polygon_2D);
            renderer.submit_vertex_batch(vertex_batch);
            renderer.free_vertex_batch(vertex_batch);
        }

        // NOTE(hugo): perlin textures
        if(false)
        {
            Vertex_Batch_ID noise_batch = renderer.get_vertex_batch(xyuv, PRIMITIVE_TRIANGLE_STRIP);
            vertex_xyuv* noise_vertices = (vertex_xyuv*)renderer.get_vertices(noise_batch, 4u);
            noise_vertices[0] = {{-0.5f, -0.5f}, {0.f, 0.f}};
            noise_vertices[1] = {{0.5f, -0.5f}, {1.f, 0.f}};
            noise_vertices[2] = {{-0.5f, 0.5f}, {0.f, 1.f}};
            noise_vertices[3] = {{0.5f, 0.5f}, {1.f, 1.f}};

            renderer.use_shader(polygon_tex_2D);
            renderer.setup_texture_unit(0u, perlin_texture);
            renderer.submit_vertex_batch(noise_batch);
            renderer.free_vertex_batch(noise_batch);

            Vertex_Batch_ID dx_batch = renderer.get_vertex_batch(xyuv, PRIMITIVE_TRIANGLE_STRIP);
            vertex_xyuv* dx_vertices = (vertex_xyuv*)renderer.get_vertices(dx_batch, 4u);
            dx_vertices[0] = {{-1.5f, -0.5f}, {0.f, 0.f}};
            dx_vertices[1] = {{-0.5f, -0.5f}, {1.f, 0.f}};
            dx_vertices[2] = {{-1.5f, 0.5f}, {0.f, 1.f}};
            dx_vertices[3] = {{-0.5f, 0.5f}, {1.f, 1.f}};

            renderer.use_shader(polygon_tex_2D);
            renderer.setup_texture_unit(0u, perlin_dx_texture);
            renderer.submit_vertex_batch(dx_batch);
            renderer.free_vertex_batch(dx_batch);

            Vertex_Batch_ID dy_batch = renderer.get_vertex_batch(xyuv, PRIMITIVE_TRIANGLE_STRIP);
            vertex_xyuv* dy_vertices = (vertex_xyuv*)renderer.get_vertices(dy_batch, 4u);
            dy_vertices[0] = {{0.5f, -0.5f}, {0.f, 0.f}};
            dy_vertices[1] = {{1.5f, -0.5f}, {1.f, 0.f}};
            dy_vertices[2] = {{0.5f, 0.5f}, {0.f, 1.f}};
            dy_vertices[3] = {{1.5f, 0.5f}, {1.f, 1.f}};

            renderer.use_shader(polygon_tex_2D);
            renderer.setup_texture_unit(0u, perlin_dy_texture);
            renderer.submit_vertex_batch(dy_batch);
            renderer.free_vertex_batch(dy_batch);
        }

        // NOTE(hugo): simplex texture
        bool show_simplex = DEV_Tweak(bool, false);
        if(show_simplex)
        {
            Vertex_Batch_ID texture_batch = renderer.get_vertex_batch(xyuv, PRIMITIVE_TRIANGLE_STRIP);
            vertex_xyuv* texture_vertices = (vertex_xyuv*)renderer.get_vertices(texture_batch, 4u);
            texture_vertices[0] = {{-0.25f, -0.25f}, {0.f, 0.f}};
            texture_vertices[1] = {{0.25f, -0.25f}, {1.f, 0.f}};
            texture_vertices[2] = {{-0.25f, 0.25f}, {0.f, 1.f}};
            texture_vertices[3] = {{0.25f, 0.25f}, {1.f, 1.f}};

            renderer.use_shader(polygon_tex_2D);
            renderer.setup_texture_unit(0u, simplex_texture, nearest_clamp);
            renderer.submit_vertex_batch(texture_batch);
            renderer.free_vertex_batch(texture_batch);
        }

        // NOTE(hugo): texture animation
        if(anim != unknown_texture_animation_playing)
        {
            Texture_Animation_Frame* anim_frame = texture_animation.get_frame(anim);

            Vertex_Batch_ID animation_batch = renderer.get_vertex_batch(xyuv, PRIMITIVE_TRIANGLE_STRIP);
            vertex_xyuv* animation_vertices = (vertex_xyuv*)renderer.get_vertices(animation_batch, 4u);
            animation_vertices[0] = {{0.25f, -0.25f}, anim_frame->uvmin};
            animation_vertices[1] = {{0.75f, -0.25f}, {anim_frame->uvmax.x, anim_frame->uvmin.y}};
            animation_vertices[2] = {{0.25f, 0.25f}, {anim_frame->uvmin.x, anim_frame->uvmax.y}};
            animation_vertices[3] = {{0.75f, 0.25f}, anim_frame->uvmax};

            renderer.use_shader(polygon_tex_2D);
            renderer.setup_texture_unit(0u, anim_frame->texture, nearest_clamp);
            renderer.submit_vertex_batch(animation_batch);
            renderer.free_vertex_batch(animation_batch);
        }

        // NOTE(hugo): particles
        if(particle_emitter.particles.size > 0u)
        {
            Vertex_Batch_ID particle_batch = particle_emitter.batch_as_quad_xyuv(&renderer);

            renderer.use_shader(polygon_tex_2D);
            renderer.setup_texture_unit(0u, simplex_texture, nearest_clamp);
            renderer.submit_vertex_batch(particle_batch);
            renderer.free_vertex_batch(particle_batch);
        }

        // ---- setup the next rendering frame

        renderer.end_frame();
		window.swap_buffers();

        DEV_LOG_timing_entries();
        DEV_next_frame();
	}

    // ---- custom cleanup

    renderer.free_texture(perlin_texture);
    renderer.free_texture(perlin_dx_texture);
    renderer.free_texture(perlin_dy_texture);
    renderer.free_texture(simplex_texture);

	// ---- termination ---- //

    asset.terminate();
    audio.terminate();
    renderer.free_resources();
	window.terminate();
	SDL_Quit();

    DEV_terminate();

    return 0;
}
