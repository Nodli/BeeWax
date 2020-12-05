void Asset_Manager::terminate(){
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

void Asset_Manager::from_asset_file(const File_Path& path){
    buffer<u8> file = read_file(path, "r");
    assert(file.data);
    DEFER{::free(file.data);};
    const char* end_char_ptr = (const char*)file.data + (file.size - 1u);

    const char* cursor = (const char*)file.data;

    constexpr char asset_start_char = ':';

    auto go_to_declaration = [&end_char_ptr, &asset_start_char](const char* icursor) -> const char*{

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

const char* Asset_Manager::make_audio_asset(Asset_Tag tag, const char* cursor){
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

const char* Asset_Manager::make_texture_asset(Asset_Tag tag, const char* cursor){
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

const char* Asset_Manager::make_font_asset(Asset_Tag tag, const char* cursor){
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

const char* Asset_Manager::make_texture_animation_asset(Asset_Tag tag, const char* cursor){
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

    asset->frames.set_capacity(nrows * ncolumns);

    for(u32 irow = 0u; irow != nrows; ++irow){
        for(u32 icol = 0u; icol != ncolumns; ++icol){
            Texture_Animation_Frame frame;

            frame.texture = texture_asset->texture;
            frame.uvmin.x = (float)(origin_x + icol * frame_width) / (float)texture_asset->width;
            frame.uvmin.y = (float)(origin_y + irow * frame_height) / (float)texture_asset->height;
            frame.uvmax.x = (float)(origin_x + (icol + 1u) * frame_width) / (float)texture_asset->width;
            frame.uvmax.y = (float)(origin_x + (irow + 1u) * frame_height) / (float)texture_asset->height;

            asset->frames.push(frame);
        }
    }
    asset->frame_duration = duration;

    return number_start;
}

Audio_Asset* Asset_Manager::get_audio(const Asset_Tag& tag){
    Audio_Asset* asset = audio.search(tag);
    assert(asset);
    return asset;
}

Texture_Asset* Asset_Manager::get_texture(const Asset_Tag& tag){
    Texture_Asset* asset = texture.search(tag);
    assert(asset);
    return asset;
}

Texture_Animation_Asset* Asset_Manager::get_texture_animation(const Asset_Tag& tag){
    Texture_Animation_Asset* asset = texture_animation.search(tag);
    assert(asset);
    return asset;
}

Font_Asset* Asset_Manager::get_font(const Asset_Tag& tag){
    Font_Asset* asset = font.search(tag);
    assert(asset);
    return asset;
}
