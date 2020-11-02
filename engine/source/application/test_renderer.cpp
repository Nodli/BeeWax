int main(int, char**){

	// ---- initialization ---- //
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
		printf("Failed SDL_Init() %s\n", SDL_GetError());
	}

    const char* window_name = "default_main";
	Window_Settings window_settings;
	window_settings.name = window_name;
	window_settings.width = 800;
	window_settings.height = 600;

	Window window;
	window.initialize(window_settings);

	gl3wInit();
	glClearColor(0.f, 0.f, 0.f, 1.f);

    Renderer renderer;
    renderer.initialize();

    GL::set_debug_message_callback();

    Frame_Timing timing;
    timing.initialize(60u);

	bool running = true;


    Camera_2D camera;

	// ---- main loop ---- //
	while(running){

        u32 nupdates = timing.nupdates_before_render();
        for(u32 iupdate = 0; iupdate != nupdates; ++iupdate){

            // NOTE(hugo): update
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

        }

        // NOTE(hugo): prepare render
        renderer.next_frame();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // NOTE(hugo): render
        Draw_Record record;

        record.state.uniform = renderer.get_uniform_data(camera_2D);
        (*(uniform_camera_2D*)(record.state.uniform->data)).matrix = to_std140(camera.camera_matrix());

        record.state.shader = polygon_2D;
        record.state.format = xyrgba;

        Buffer_Mapping vertex_mapping = renderer.get_dynamic_vertex_buffer(xyrgba, 4, record.state.vertex_data);

        vertex_xyrgba* vertex_ptr = (vertex_xyrgba*)renderer.get_mapping(vertex_mapping);

        vertex_ptr[0].vposition.x = -0.5f;
        vertex_ptr[0].vposition.y = -0.5f;
        vertex_ptr[0].vcolor.r = 1.;
        vertex_ptr[0].vcolor.g = 0.;
        vertex_ptr[0].vcolor.b = 0.;
        vertex_ptr[0].vcolor.a = 1.;

        vertex_ptr[1].vposition.x = 0.5f;
        vertex_ptr[1].vposition.y = -0.5f;
        vertex_ptr[1].vcolor.r = 0.;
        vertex_ptr[1].vcolor.g = 1.;
        vertex_ptr[1].vcolor.b = 0.;
        vertex_ptr[1].vcolor.a = 1.;

        vertex_ptr[2].vposition.x = -0.5f;
        vertex_ptr[2].vposition.y = 0.5f;
        vertex_ptr[2].vcolor.r = 0.;
        vertex_ptr[2].vcolor.g = 0.;
        vertex_ptr[2].vcolor.b = 1.;
        vertex_ptr[2].vcolor.a = 1.;

        vertex_ptr[3].vposition.x = 0.5f;
        vertex_ptr[3].vposition.y = 0.5f;
        vertex_ptr[3].vcolor.r = 1.;
        vertex_ptr[3].vcolor.g = 1.;
        vertex_ptr[3].vcolor.b = 1.;
        vertex_ptr[3].vcolor.a = 1.;

        renderer.release_mapping(vertex_mapping);

        record.command.primitive = TRIANGLE_STRIP;
        record.command.number_of_vertices = 4;

        renderer.push_record(record);

        // NOTE(hugo): swap framebuffers
        renderer.execute_draw();
		window.swap_buffers();

        double current_time = timer_seconds();
        LOG_TRACE("nupdates: %d sec/frame: %f fps: %f", nupdates, current_time - saved_time, 1 / (current_time - saved_time));
        saved_time = current_time;
	}

	// ---- termination ---- //
    renderer.terminate();
	window.terminate();
	SDL_Quit();

    return 0;
}
