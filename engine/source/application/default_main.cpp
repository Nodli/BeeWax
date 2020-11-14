using namespace bw;

int main(int argc, char* argv[]){

	// ---- initialization ---- //

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
		printf("Failed SDL_Init() %s\n", SDL_GetError());
	}

    // ---- window

    const char* window_name = "default_main";
	Window_Settings window_settings;
	window_settings.name = window_name;
	window_settings.width = 1280;
	window_settings.height = 720;
	//window_settings.sync = Window_Settings::SINGLE_BUFFER_NOSYNC;

	Window window;
	window.initialize(window_settings);

    Frame_Timing timing;
    timing.initialize(60u);

    // ---- rendering

	gl3wInit();
	glClearColor(0.f, 0.f, 0.f, 1.f);

    Renderer renderer;
    renderer.setup_resources();

    Font_Renderer font_renderer;
    font_renderer.window = &window;
    font_renderer.renderer = &renderer;
    font_renderer.make_bitmap_from_file("./data/Roboto_Font/Roboto-Italic.ttf", ASCII_PRINTABLE, window_settings.height / 10.f, 5, 180);

    //DEV_DEBUG_RENDERER;

    // ---- audio

    Audio_Manager audio;
    audio.setup();

    // ---- input

    Keyboard_State keyboard;
    Mouse_State mouse;

    // ---- state

	bool running = true;

    // ---- texture generation

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

    // ---- audio loading

    Audio_Buffer_ID audio_imperial_march = audio.load_wav("./data/audio/ImperialMarch60.wav");
    audio.play_buffer(audio_imperial_march);

    // ---- developper tools

    DEV_INITIALIZE;
    DEV_DISPLAY_TWEAKABLE_ENTRIES;

	// ---- main loop ---- //
	while(running){
        //DEV_LOG_FRAME_TIME;

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

            // ---- setup the next game frame

            keyboard.reset();
            mouse.reset();
            audio.next_frame();
        }

        // ---- render

        renderer.start_frame();
        font_renderer.start_frame();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Camera_2D camera;
        camera.center = {0.f, 0.f};
        camera.height = 2.f;
        camera.aspect_ratio = window.aspect_ratio();
        mat3 camera_mat = camera.camera_matrix();

        ((uniform_camera_2D*)renderer.get_uniform(camera_2D))->matrix = to_std140(camera_mat);
        renderer.submit_uniform(camera_2D);

        // NOTE(hugo): red square
        if(false)
        {
            Vertex_Batch_ID vertex_batch = renderer.get_vertex_batch(xyrgba, PRIMITIVE_TRIANGLE_STRIP);
            vertex_xyrgba* vertices = (vertex_xyrgba*)renderer.get_vertices(vertex_batch, 4u);
            vertices[0] = {{-0.1f, -0.1f}, {1.f, 0.f, 0.f, 1.f}};
            vertices[1] = {{0.1f, -0.1f}, {1.f, 0.f, 0.f, 1.f}};
            vertices[2] = {{-0.1f, 0.1f}, {1.f, 0.f, 0.f, 1.f}};
            vertices[3] = {{0.1f, 0.1f}, {1.f, 0.f, 0.f, 1.f}};

            renderer.use_shader(polygon_2D);
            renderer.submit_vertex_batch(vertex_batch);
            renderer.free_vertex_batch(vertex_batch);
        }

        // NOTE(hugo): font rendering
        if(true)
        {
            vec2 baseline = {300.f, 300.f};
            float baseline_end_x = font_renderer.batch_string("Asobisof", baseline.x, baseline.y, 500.f / 2.f);
            font_renderer.render();

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
        bool show_simplex = DEV_TWEAKABLE(BOOLEAN, "show_simplex", false);
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

        // ---- setup the next rendering frame

        font_renderer.end_frame();
        renderer.end_frame();
		window.swap_buffers();

        DEV_DISPLAY_TIMING_ENTRIES;
        DEV_NEXT_FRAME;
	}

    // ---- texture cleanup

    renderer.free_texture(perlin_texture);
    renderer.free_texture(perlin_dx_texture);
    renderer.free_texture(perlin_dy_texture);
    renderer.free_texture(simplex_texture);

    // ---- audio cleanup

    audio.unload_buffer(audio_imperial_march);

	// ---- termination ---- //
    audio.terminate();
    font_renderer.free();
    renderer.free_resources();
	window.terminate();
	SDL_Quit();

    DEV_TERMINATE;

    return 0;
}
