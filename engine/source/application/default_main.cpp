using namespace bw;

int main(){

	// ---- initialization ---- //
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
		printf("Failed SDL_Init() %s\n", SDL_GetError());
	}

    // ---- window

    const char* window_name = "default_main";
	Window_Settings window_settings;
	window_settings.name = window_name;
	window_settings.width = 1920;
	window_settings.height = 1080;

	Window window;
	window.initialize(window_settings);

    Frame_Timing timing;
    timing.initialize(60u);

    // ---- rendering

	gl3wInit();
	glClearColor(0.f, 0.f, 0.f, 1.f);

    Renderer renderer;
    renderer.setup_resources();

    Font_Rendering font_rendering;
    font_rendering.setup_from_file("./data/Roboto_Font/Roboto-Black.ttf");

    GL::set_debug_message_callback();

    // ---- input

    Keyboard_State keyboard;
    Mouse_State mouse;

    // ---- state

	bool running = true;
    double saved_time = timer_seconds();
    UNUSED(saved_time);

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

	// ---- main loop ---- //
	while(running){

        // ---- update

        u32 nupdates = timing.nupdates_before_render();
        for(u32 iupdate = 0; iupdate != nupdates; ++iupdate){

            SDL_Event event;
            while(SDL_PollEvent(&event)){
                switch(event.type){
                    case SDL_QUIT:
                        running = false;
                        break;

                    default:
                        break;
                }
            }
            keyboard.register_event(event);
            mouse.register_event(event);
        }

        // ---- render

        renderer.next_frame();
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

            renderer.use_shader(polygon_2D);
            renderer.submit_batch(vertex_batch);
            renderer.free_vertex_batch(vertex_batch);
        }

        // NOTE(hugo): font rendering
        if(true)
        {
            Vertex_Batch_ID font_batch = renderer.get_vertex_batch(xyrgba, PRIMITIVE_LINES);
            font_rendering.batch_string(&renderer, font_batch, "I", {-1.15f, -0.f}, 0.20f, {1.f, 0.f, 0.f, 1.f});

            renderer.use_shader(polygon_2D);
            renderer.submit_batch(font_batch);
            renderer.free_vertex_batch(font_batch);
        }

        // NOTE(hugo): perlin textures
        if(true)
        {
            Vertex_Batch_ID noise_batch = renderer.get_vertex_batch(xyuv, PRIMITIVE_TRIANGLE_STRIP);
            vertex_xyuv* noise_vertices = (vertex_xyuv*)renderer.get_vertices(noise_batch, 4u);
            noise_vertices[0] = {{-0.5f, -0.5f}, {0.f, 0.f}};
            noise_vertices[1] = {{0.5f, -0.5f}, {1.f, 0.f}};
            noise_vertices[2] = {{-0.5f, 0.5f}, {0.f, 1.f}};
            noise_vertices[3] = {{0.5f, 0.5f}, {1.f, 1.f}};

            renderer.use_shader(polygon_tex_2D);
            renderer.use_texture(perlin_texture, 0u);
            renderer.submit_batch(noise_batch);
            renderer.free_vertex_batch(noise_batch);

            Vertex_Batch_ID dx_batch = renderer.get_vertex_batch(xyuv, PRIMITIVE_TRIANGLE_STRIP);
            vertex_xyuv* dx_vertices = (vertex_xyuv*)renderer.get_vertices(dx_batch, 4u);
            dx_vertices[0] = {{-1.5f, -0.5f}, {0.f, 0.f}};
            dx_vertices[1] = {{-0.5f, -0.5f}, {1.f, 0.f}};
            dx_vertices[2] = {{-1.5f, 0.5f}, {0.f, 1.f}};
            dx_vertices[3] = {{-0.5f, 0.5f}, {1.f, 1.f}};

            renderer.use_shader(polygon_tex_2D);
            renderer.use_texture(perlin_dx_texture, 0u);
            renderer.submit_batch(dx_batch);
            renderer.free_vertex_batch(dx_batch);

            Vertex_Batch_ID dy_batch = renderer.get_vertex_batch(xyuv, PRIMITIVE_TRIANGLE_STRIP);
            vertex_xyuv* dy_vertices = (vertex_xyuv*)renderer.get_vertices(dy_batch, 4u);
            dy_vertices[0] = {{0.5f, -0.5f}, {0.f, 0.f}};
            dy_vertices[1] = {{1.5f, -0.5f}, {1.f, 0.f}};
            dy_vertices[2] = {{0.5f, 0.5f}, {0.f, 1.f}};
            dy_vertices[3] = {{1.5f, 0.5f}, {1.f, 1.f}};

            renderer.use_shader(polygon_tex_2D);
            renderer.use_texture(perlin_dy_texture, 0u);
            renderer.submit_batch(dy_batch);
            renderer.free_vertex_batch(dy_batch);
        }

        // NOTE(hugo): simplex texture
        if(false)
        {
            Vertex_Batch_ID texture_batch = renderer.get_vertex_batch(xyuv, PRIMITIVE_TRIANGLE_STRIP);
            vertex_xyuv* texture_vertices = (vertex_xyuv*)renderer.get_vertices(texture_batch, 4u);
            texture_vertices[0] = {{-0.25f, -0.25f}, {0.f, 0.f}};
            texture_vertices[1] = {{0.25f, -0.25f}, {1.f, 0.f}};
            texture_vertices[2] = {{-0.25f, 0.25f}, {0.f, 1.f}};
            texture_vertices[3] = {{0.25f, 0.25f}, {1.f, 1.f}};

            renderer.use_shader(polygon_tex_2D);
            renderer.use_texture(simplex_texture, 0u);
            renderer.submit_batch(texture_batch);
            renderer.free_vertex_batch(texture_batch);
        }

        // ---- setup next frame

		window.swap_buffers();

        double current_time = timer_seconds();
        //LOG_TRACE("nupdates: %d sec/frame: %f fps: %f", nupdates, current_time - saved_time, 1 / (current_time - saved_time));
        saved_time = current_time;

        debug_display_timing_entries();
        debug_next_frame();
	}

    // ---- texture cleanup

    renderer.free_texture(perlin_texture);
    renderer.free_texture(perlin_dx_texture);
    renderer.free_texture(perlin_dy_texture);
    renderer.free_texture(simplex_texture);

	// ---- termination ---- //
    font_rendering.terminate();
    renderer.free_resources();
	window.terminate();
	SDL_Quit();

    debug_free();

    return 0;
}
