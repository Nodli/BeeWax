namespace BEEWAX_INTERNAL{
    struct stbtt_glyph_data{
        u8* bitmap;
        s32 x_offset;
        s32 y_offset;
        s32 width;
        s32 height;
        float advance;
        char codepoint;
    };
}

void Font_Renderer::make_bitmap_from_file(const File_Path& path, const char* string, float font_size, s32 glyph_padding, s32 glyph_edge_value){
    buffer<u8> file = read_file(path);
    if(!file.data || file.size == 0u){
        LOG_ERROR("stbtt_InitFont() - FAILED for path: %s", path.data);
        return;
    }

    stbtt_fontinfo info;
    if(!stbtt_InitFont(&info, (const u8*)file.data, 0)){
        LOG_ERROR("stbtt_InitFont() - FAILED for path: %s", path.data);
        return;
    }

    assert(*string != '\0');

    bitmap_font_size = font_size;

    // ----

    float font_scale = stbtt_ScaleForPixelHeight(&info, font_size);
    float glyph_value_delta = (float)glyph_edge_value / (float)glyph_padding;

    u32 string_size = strlen(string);

    // NOTE(hugo): generating an sdf bitmap for each codepoint
    darray<BEEWAX_INTERNAL::stbtt_glyph_data> glyph_data;
    glyph_data.set_min_capacity(string_size);

    s32 width_total = 0u;
    s32 height_max = 0u;
    for(u32 icodepoint = 0u; icodepoint != string_size; ++icodepoint){
        BEEWAX_INTERNAL::stbtt_glyph_data data;
        data.codepoint = string[icodepoint];
        data.bitmap = stbtt_GetCodepointSDF(&info,
                font_scale, string[icodepoint], glyph_padding, glyph_edge_value, glyph_value_delta,
                &data.width, &data.height, &data.x_offset, &data.y_offset);

        data.y_offset = - data.y_offset - data.height;

        s32 temp_advance;
        stbtt_GetCodepointHMetrics(&info, string[icodepoint], &temp_advance, nullptr);
        data.advance = (float)temp_advance * font_scale;

        glyph_data.push(data);

        width_total += data.width;
        height_max = max(height_max, data.height);
    }

    // NOTE(hugo): determine the atlas dimensions
    bitmap_width = width_total;
    bitmap_height = height_max;
    bitmap_data = (u8*)calloc(bitmap_width * bitmap_height, sizeof(u8));

    // NOTE(hugo): copy the glyph bitmaps in the atlas
    //             cursor starts at the top left of the bitmap
    u32 cursor_x = 0u;
    u32 cursor_y = 0u;
    for(u32 icodepoint = 0u; icodepoint != glyph_data.size; ++icodepoint){
        BEEWAX_INTERNAL::stbtt_glyph_data& glyph = glyph_data[icodepoint];

        // NOTE(hugo): stbtt_GetCodepointSDF returns nullptr for empty codepoints
        //             glyph bitmaps need to be flipped
        if(glyph.bitmap){
            u32 dest_offset = cursor_y * bitmap_width + cursor_x;
            u32 src_offset = 0u;
            for(u32 codepoint_y = 0; codepoint_y != glyph.height; ++codepoint_y){
                memcpy(bitmap_data + dest_offset, glyph.bitmap + src_offset, glyph.width);
                dest_offset += bitmap_width;
                src_offset += glyph.width;
            }
            stbtt_FreeSDF(glyph.bitmap, nullptr);
        }

        // NOTE(hugo): adding the codepoint to the hashmap
        bool entry_created;
        codepoint_info* cp_info = codepoint_to_info.get(glyph.codepoint, entry_created);
        cp_info->uv_min = {
            (float)cursor_x / (float)bitmap_width,
            (float)(cursor_y + glyph.height) / (float)bitmap_height
        };
        cp_info->uv_max = {
            (float)(cursor_x + glyph.width) / (float)bitmap_width,
            (float)cursor_y / (float)bitmap_height
        };
        cp_info->quad_offset_x = glyph.x_offset;
        cp_info->quad_offset_y = glyph.y_offset;
        cp_info->quad_width = glyph.width;
        cp_info->quad_height = glyph.height;
        cp_info->cursor_advance = glyph.advance;

        cursor_x += glyph.width;
    }

    texture = renderer->get_texture(TEXTURE_FORMAT_R, bitmap_width, bitmap_height, TYPE_UBYTE, bitmap_data);

    // TODO(hugo): use a better atlas size determination method to have a square atlas
#if 0
    auto stbtt_glyph_data_height_compare = [](const BEEWAX_INTERNAL::stbtt_glyph_data& A, const BEEWAX_INTERNAL::stbtt_glyph_data& B){
        return BEEWAX_INTERNAL::comparison_decreasing_order(A.height, B.height);
    };
    qsort<BEEWAX_INTERNAL::stbtt_glyph_data, stbtt_glyph_data_height_compare>(sdf_bitmaps.data, sdf_bitmaps.size);

    u32 width_midpoint = width_total / 2u;
    u32 width_midpoint_index = 0u;
    u32 temp_width_sum = 0u;
    while(width_midpoint_index < sdf_bitmaps.size && temp_width_sum < width_midpoint){
        ++width_midpoint_index;
    }

    for(u32 isdf = 0u; isdf != sdf_bitmaps.size; ++isdf){
        LOG_TRACE("%c %d %d", sdf_bitmaps[isdf].codepoint, sdf_bitmaps[isdf].width, sdf_bitmaps[isdf].height);
    }
#endif
}

void Font_Renderer::free(){
    renderer->free_texture(texture);
    ::free(file.data);
    ::free(bitmap_data);
    codepoint_to_info.free();
}

void Font_Renderer::start_frame(){
    batch = renderer->get_vertex_batch(xyuv, PRIMITIVE_TRIANGLES);
}

void Font_Renderer::end_frame(){
    renderer->free_vertex_batch(batch);
}

float Font_Renderer::batch_string(const char* string, float baseline_x, float baseline_y, float font_size){
    float relative_font_scale = font_size / bitmap_font_size;
    u32 string_size = strlen(string);

    vertex_xyuv* vertices = (vertex_xyuv*)renderer->get_vertices(batch, 6u * string_size);

    for(u32 icodepoint = 0u; icodepoint != string_size; ++icodepoint){
        char cp = string[icodepoint];
        codepoint_info* cp_info = codepoint_to_info.search(cp);
        if(!cp_info){
            cp_info = codepoint_to_info.search(' ');
            assert(cp_info);
        }

        ivec2 iquad_min = {
            (s32)fast_floor<float>(baseline_x + (float)cp_info->quad_offset_x * relative_font_scale),
            (s32)fast_floor<float>(baseline_y + (float)cp_info->quad_offset_y * relative_font_scale)
        };
        ivec2 iquad_max = {
            (s32)fast_floor<float>((float)iquad_min.x + (float)cp_info->quad_width * relative_font_scale),
            (s32)fast_floor<float>((float)iquad_min.y + (float)cp_info->quad_height * relative_font_scale)
        };

        vec2 quad_min = window->pixel_to_screen_coordinates(iquad_min);
        vec2 quad_max = window->pixel_to_screen_coordinates(iquad_max);

        vertices[0] = {{quad_min.x, quad_min.y}, cp_info->uv_min};
        vertices[1] = {{quad_max.x, quad_min.y}, {cp_info->uv_max.x, cp_info->uv_min.y}};
        vertices[2] = {{quad_min.x, quad_max.y}, {cp_info->uv_min.x, cp_info->uv_max.y}};
        vertices[3] = {{quad_min.x, quad_max.y}, {cp_info->uv_min.x, cp_info->uv_max.y}};
        vertices[4] = {{quad_max.x, quad_min.y}, {cp_info->uv_max.x, cp_info->uv_min.y}};
        vertices[5] = {{quad_max.x, quad_max.y}, cp_info->uv_max};

        vertices = vertices + 6u;

        baseline_x += cp_info->cursor_advance * relative_font_scale;
    }

    return baseline_x;
}

void Font_Renderer::render(){
    renderer->use_shader(font_2D);
    renderer->setup_texture_unit(0u, texture, linear_clamp);
    renderer->submit_vertex_batch(batch);
}
