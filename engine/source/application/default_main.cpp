using namespace bw;

struct Texture_Asset{
    u8* bitmap;
    u32 width;
    u32 height;
    Texture_ID texture;
};

Texture_Asset texture_asset_from_png_file(const File_Path& path, Renderer* renderer){
    Texture_Asset asset;

    s32 width, height, nchannels;
    asset.bitmap = stbi_load(path.data, &width, &height, &nchannels, 4u);
    asset.width = width;
    asset.height = height;

    asset.texture = renderer->get_texture(TEXTURE_FORMAT_RGB, width, height, TYPE_UBYTE, asset.bitmap);

    return asset;
}

void free_texture_asset(Texture_Asset& asset, Renderer* renderer){
    ::free(asset.bitmap);
    renderer->free_texture(asset.texture);
}

struct Texture_Animation_Frame{
    Texture_ID texture;
    vec2 uvmin;
    vec2 uvmax;
};

struct Texture_Animation_Asset{
    darray<Texture_Animation_Frame> frames;
    u32 frame_duration;
};

void free_texture_animation_asset(Texture_Animation_Asset& asset){
    asset.frames.free();
}

struct Asset_Manager{
    typedef sstring<60u> Asset_Tag;

    void terminate(){
        u32 index, counter;
        for(index = audio.storage.get_first(), counter = 0u;
            index < audio.storage.capacity && counter != audio.storage.size;
            index = audio.storage.get_next(index), ++counter){

            free_audio_asset(audio.storage[index].value);
        }
        audio.free();
        for(index = texture.storage.get_first(), counter = 0u;
            index < texture.storage.capacity && counter != texture.storage.size;
            index = texture.storage.get_next(index), ++counter){

            free_texture_asset(texture.storage[index].value, renderer);
        }
        texture.free();
        for(index = texture_animation.storage.get_first(), counter = 0u;
            index < texture_animation.storage.capacity && counter != texture_animation.storage.size;
            index = texture_animation.storage.get_next(index), ++counter){

            free_texture_animation_asset(texture_animation.storage[index].value);
        }
        texture_animation.free();
        for(index = font.storage.get_first(), counter = 0u;
            index < font.storage.capacity && counter != font.storage.size;
            index = font.storage.get_next(index), ++counter){

            free_font_asset(font.storage[index].value, renderer);
        }
        font.free();
    }

    void from_asset_file(const File_Path& path){
        buffer<u8> file = read_file(path);
        assert(file.data);
        DEFER{::free(file.data);};
        const char* end_char_ptr = (const char*)file.data + (file.size - 1u);

        const char* cursor = (const char*)file.data;

        constexpr char asset_start_char = ':';

        auto go_to_declaration = [&end_char_ptr](const char* icursor) -> const char*{

            while(*icursor != asset_start_char ){
                while(*icursor != '\n'){
                    ++icursor;
                    if(icursor == end_char_ptr) return icursor;
                }
                ++icursor;
                if(icursor == end_char_ptr) return icursor;
            }
            return icursor;
        };

        while((cursor = go_to_declaration(cursor), *cursor == asset_start_char)){
            ++cursor;
            while(*cursor == ' '){++cursor;};

            // NOTE(hugo): type and tag
            const char* type_start = cursor;
            const char* type_end = type_start;
            while(*type_end != ' '){++type_end;};
            u32 type_size = type_end - type_start;

            const char* tag_start = type_end;
            while(*tag_start == ' '){++tag_start;};
            const char* tag_end = tag_start;
            while(*tag_end != ' ' && *tag_end != '\n'){++tag_end;};

            Asset_Tag tag;
            tag.extract_from(tag_start, tag_end - tag_start);

            LOG_TRACE("asset: (%.*s | %.*s)", type_size, type_start, tag.size, tag.data);

            const char* audio_type = "audio";
            const char* texture_type = "texture";
            const char* texture_animation_type = "texture_animation";
            const char* font_type = "font";

            // NOTE(hugo): parse the asset depending on type
            const char* asset_start = tag_end;
            while(*asset_start++ != '\n');
            assert(*asset_start != EOF);
            const char* asset_end;

            if(type_size == strlen(audio_type)
            && memcmp(type_start, audio_type, type_size) == 0u){
                asset_end = make_audio_asset(tag, asset_start);

            }else if(type_size == strlen(texture_type)
            && memcmp(type_start, texture_type, type_size) == 0u){
                asset_end = make_texture_asset(tag, asset_start);

            }else if(type_size == strlen(texture_animation_type)
            && memcmp(type_start, texture_animation_type, type_size) == 0u){
                asset_end = make_texture_animation_asset(tag, asset_start);

            }else if(type_size == strlen(font_type)
            && memcmp(type_start, font_type, type_size) == 0u){
                asset_end = make_font_asset(tag, asset_start);

            }else{
                asset_end = asset_start;
                LOG_ERROR("unknown asset type (%.*s | %.*s)", type_size, type_start, tag.size, tag.data);
            }
        }
    }

    const char* make_audio_asset(Asset_Tag tag, const char* cursor){
        // NOTE(hugo): parse the filename
        const char* filename_start = cursor;
        const char* filename_end = filename_start;
        while(*filename_end != '\n'){++filename_end;};

        File_Path filename;
        filename.extract_from(filename_start, filename_end - filename_start);

        // NOTE(hugo): create the asset
        bool new_asset;
        Audio_Asset* asset = audio.get(tag, new_asset);
        assert(new_asset);

        File_Path real_path = asset_path() / filename;
        *asset = audio_asset_from_wav_file(real_path, audio_player);

        return filename_end;
    }

    const char* make_texture_asset(Asset_Tag tag, const char* cursor){
        // NOTE(hugo): parse the filename
        const char* filename_start = cursor;
        const char* filename_end = filename_start;
        while(*filename_end != '\n'){++filename_end;};

        File_Path filename;
        filename.extract_from(filename_start, filename_end - filename_start);

        // NOTE(hugo): create the asset
        bool new_asset;
        Texture_Asset* asset = texture.get(tag, new_asset);
        assert(new_asset);

        File_Path real_path = asset_path() / filename;
        *asset = texture_asset_from_png_file(real_path, renderer);

        return filename_end;
    }

    const char* make_font_asset(Asset_Tag tag, const char* cursor){
        // NOTE(hugo): parse the filename, font size and character string
        const char* filename_start = cursor;
        const char* filename_end = filename_start;
        while(*filename_end != ' '){++filename_end;};

        const char* size_start = filename_end + 1u;
        while(*size_start == ' '){++size_start;};
        char* endptr;
        float font_size = strtof(size_start, &endptr);
        assert(endptr != size_start);

        while(*endptr != '\n'){++endptr;};
        const char* cstring_start = endptr + 1u;
        const char* cstring_end = cstring_start;
        while(*cstring_end != '\n'){++cstring_end;};

        File_Path filename;
        filename.extract_from(filename_start, filename_end - filename_start);

        // NOTE(hugo): create the asset
        bool new_asset;
        Font_Asset* asset = font.get(tag, new_asset);
        assert(new_asset);

        File_Path real_path = asset_path() / filename;
        *asset = font_asset_from_ttf_file(real_path, cstring_start, cstring_end - cstring_start, font_size, renderer);

        return cstring_end;
    }

    const char* make_texture_animation_asset(Asset_Tag tag, const char* cursor){
        // NOTE(hugo): texture tag
        const char* texture_tag_start = cursor;
        const char* texture_tag_end = texture_tag_start;
        while(*texture_tag_end != ' '){++texture_tag_end;};

        Asset_Tag texture_tag;
        texture_tag.extract_from(texture_tag_start, texture_tag_end - texture_tag_start);

        Texture_Asset* texture_asset = texture.search(texture_tag);
        assert(texture_asset);

        // NOTE(hugo): animation parameters
        const char* number_start = texture_tag_end;
        auto parse_number = [&](){
            number_start = number_start + 1u;
            while(*number_start == ' '){++number_start;};

            char* number_end;
            u32 output = (u32)strtol(number_start, &number_end, 10);
            assert(number_end != number_start);

            number_start = number_end;
            return output;
        };

        u32 ncolumns = parse_number();
        u32 nrows = parse_number();
        u32 origin_x = parse_number();
        u32 origin_y = parse_number();
        u32 frame_width = parse_number();
        u32 frame_height = parse_number();
        u32 duration = parse_number();

        // NOTE(hugo): create the asset
        bool new_asset;
        Texture_Animation_Asset* asset = texture_animation.get(tag, new_asset);
        assert(new_asset);

        for(u32 irow = 0u; irow != ncolumns; ++irow){
            for(u32 icol = 0u; icol != ncolumns; ++icol){
                Texture_Animation_Frame frame;

                frame.texture = texture_asset->texture;
                frame.uvmin.x = (float)(origin_x + irow * frame_width) / (float)texture_asset->width;
                frame.uvmin.y = (float)(origin_y + icol * frame_height) / (float)texture_asset->height;
                frame.uvmax.x = (float)(origin_x + (irow + 1u) * frame_width) / (float)texture_asset->width;
                frame.uvmax.y = (float)(origin_x + (irow + 1u) * frame_height) / (float)texture_asset->height;

                asset->frames.push(frame);
            }
        }
        asset->frame_duration = duration;

        return number_start;
    }

    Audio_Asset* get_audio(const Asset_Tag& tag){
        Audio_Asset* asset = audio.search(tag);
        assert(asset);
        return asset;
    }

    Texture_Asset* get_texture(const Asset_Tag& tag){
        Texture_Asset* asset = texture.search(tag);
        assert(asset);
        return asset;
    }

    Texture_Animation_Asset* get_texture_animation(const Asset_Tag& tag){
        Texture_Animation_Asset* asset = texture_animation.search(tag);
        assert(asset);
        return asset;
    }

    Font_Asset* get_font(const Asset_Tag& tag){
        Font_Asset* asset = font.search(tag);
        assert(asset);
        return asset;
    }

    // ---- data

    Audio_Player* audio_player = nullptr;
    Renderer* renderer = nullptr;

    Audio_Asset audio_default = {nullptr, 0u};
    dhashmap<Asset_Tag, Audio_Asset> audio;

    Texture_Asset texture_default;
    dhashmap<Asset_Tag, Texture_Asset> texture;

    Texture_Animation_Asset texture_animation_default;
    dhashmap<Asset_Tag, Texture_Animation_Asset> texture_animation;

    Font_Asset font_default;
    dhashmap<Asset_Tag, Font_Asset> font;
};

typedef u32 Texture_Animation_Playing_ID;
struct Texture_Animation_Player{
    void terminate(){
        to_play.free();
    }

    Texture_Animation_Playing_ID start_playing(Texture_Animation_Asset* asset){
        Play_Data play_data;
        play_data.asset = asset;
        play_data.counter = 0u;
        play_data.frame_index = 0u;

        u32 index = to_play.insert(play_data);

        return index;
    }
    void stop_playing(Texture_Animation_Playing_ID play){
        to_play.remove(play);
    }

    Texture_Animation_Frame* get_frame(Texture_Animation_Playing_ID play){
        Play_Data& play_data = to_play[play];
        return &play_data.asset->frames[play_data.frame_index];
    }

    void next_frame(){
        u32 index, counter;
        for(index = to_play.get_first(), counter = 0u;
                index < to_play.capacity;
                index = to_play.get_next(index), ++counter){

            Play_Data& play = to_play[index];
            ++play.counter;
            if(play.counter == play.asset->frame_duration){
                play.counter = 0u;
                ++play.frame_index;
                if(play.frame_index == play.asset->frames.size){
                    play.frame_index = 0u;
                }
            }
        }
    }

    // ---- data

    struct Play_Data{
        Texture_Animation_Asset* asset;
        u32 counter;
        u32 frame_index;
    };
    u64 manager_generation;
    diterpool<Play_Data> to_play;
};

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
    font_renderer.width = window.width;
    font_renderer.height = window.height;
    font_renderer.renderer = &renderer;

    //DEV_DEBUG_RENDERER;

    // ---- sprites & animations



    // ---- audio

    Audio_Player audio;
    audio.setup();

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

    audio.start_playing(asset.get_audio("swtheme"));

    // ---- developper tools

    DEV_setup();

    DEV_debug_break();

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

            if(keyboard.space.npressed > 0){
                audio.start_playing(asset.get_audio("cantina"));
            }

            if(keyboard.function_F1.npressed > 0){
                BEEWAX_INTERNAL::DEV_reparse_tweakables();
            }

            // ---- setup the next game frame

            keyboard.next_frame();
            mouse.next_frame();
            audio.mix_next_frame();
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

        // ---- setup the next rendering frame

        renderer.end_frame();
		window.swap_buffers();

        DEV_LOG_timing_entries();
        DEV_next_frame();
	}

    // ---- texture cleanup

    renderer.free_texture(perlin_texture);
    renderer.free_texture(perlin_dx_texture);
    renderer.free_texture(perlin_dy_texture);
    renderer.free_texture(simplex_texture);

    // ---- audio cleanup

	// ---- termination ---- //

    asset.terminate();
    audio.terminate();
    renderer.free_resources();
	window.terminate();
	SDL_Quit();

    DEV_terminate();

    return 0;
}
