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

Font_Asset font_asset_from_ttf_file(const File_Path& path,
        const char* character_string, u32 character_string_size, float font_size,
        Renderer* renderer){

    constexpr s32 glyph_padding = 5;
    constexpr s32 glyph_edge_value = 180;

    Font_Asset asset;

    asset.file = read_file(path, "rb");
    if(!asset.file.data || asset.file.size == 0u){
        LOG_ERROR("stbtt_InitFont() - FAILED for path: %s", path.data);
        return asset;
    }

    if(!stbtt_InitFont(&asset.info, (const u8*)asset.file.data, 0)){
        LOG_ERROR("stbtt_InitFont() - FAILED for path: %s", path.data);
        return asset;
    }

    assert(*character_string != '\0');

    asset.bitmap_font_size = font_size;

    // ----

    float font_scale = stbtt_ScaleForPixelHeight(&asset.info, font_size);
    float glyph_value_delta = (float)glyph_edge_value / (float)glyph_padding;

    // NOTE(hugo): generating an sdf bitmap for each codepoint
    darray<BEEWAX_INTERNAL::stbtt_glyph_data> glyph_data;
    glyph_data.set_min_capacity(character_string_size);
    DEFER{ glyph_data.free(); };

    s32 width_total = 0u;
    s32 height_max = 0u;
    for(u32 icodepoint = 0u; icodepoint != character_string_size; ++icodepoint){
        BEEWAX_INTERNAL::stbtt_glyph_data data;
        data.codepoint = character_string[icodepoint];
        data.bitmap = stbtt_GetCodepointSDF(&asset.info,
                font_scale, character_string[icodepoint], glyph_padding, glyph_edge_value, glyph_value_delta,
                &data.width, &data.height, &data.x_offset, &data.y_offset);

        data.y_offset = - data.y_offset - data.height;

        s32 temp_advance;
        stbtt_GetCodepointHMetrics(&asset.info, character_string[icodepoint], &temp_advance, nullptr);
        data.advance = (float)temp_advance * font_scale;

        glyph_data.push(data);

        width_total += data.width;
        height_max = max(height_max, data.height);
    }

    s32 temp_ascent, temp_descent, temp_linegap;
    stbtt_GetFontVMetrics(&asset.info, &temp_ascent, &temp_descent, &temp_linegap);
    asset.ascent = temp_ascent * font_scale;
    asset.descent = temp_descent * font_scale;
    asset.linegap = temp_linegap * font_scale;

    // NOTE(hugo): determine the atlas dimensions
    asset.bitmap_width = width_total;
    asset.bitmap_height = height_max;
    asset.bitmap_data = (u8*)calloc(asset.bitmap_width * asset.bitmap_height, sizeof(u8));

    // NOTE(hugo): copy the glyph bitmaps in the atlas
    //             cursor starts at the top left of the bitmap
    u32 cursor_x = 0u;
    u32 cursor_y = 0u;
    for(u32 icodepoint = 0u; icodepoint != glyph_data.size; ++icodepoint){
        BEEWAX_INTERNAL::stbtt_glyph_data& glyph = glyph_data[icodepoint];

        // NOTE(hugo): stbtt_GetCodepointSDF returns nullptr for empty codepoints
        //             glyph bitmaps need to be flipped
        if(glyph.bitmap){
            u32 dest_offset = cursor_y * asset.bitmap_width + cursor_x;
            u32 src_offset = 0u;
            for(u32 codepoint_y = 0; codepoint_y != glyph.height; ++codepoint_y){
                memcpy(asset.bitmap_data + dest_offset, glyph.bitmap + src_offset, glyph.width);
                dest_offset += asset.bitmap_width;
                src_offset += glyph.width;
            }
            stbtt_FreeSDF(glyph.bitmap, nullptr);
        }

        // NOTE(hugo): adding the codepoint to the hashmap
        bool entry_created;
        Font_Asset::CodePoint_Info* cp_info = asset.codepoint_to_info.get(glyph.codepoint, entry_created);
        cp_info->uv_min = {
            (float)cursor_x / (float)asset.bitmap_width,
            (float)(cursor_y + glyph.height) / (float)asset.bitmap_height
        };
        cp_info->uv_max = {
            (float)(cursor_x + glyph.width) / (float)asset.bitmap_width,
            (float)cursor_y / (float)asset.bitmap_height
        };
        cp_info->quad_offset_x = glyph.x_offset;
        cp_info->quad_offset_y = glyph.y_offset;
        cp_info->quad_width = glyph.width;
        cp_info->quad_height = glyph.height;
        cp_info->cursor_advance = glyph.advance;

        cursor_x += glyph.width;
    }

    asset.texture = renderer->get_texture(TEXTURE_FORMAT_R, asset.bitmap_width, asset.bitmap_height, TYPE_UBYTE, asset.bitmap_data);

#if 0
    // TODO(hugo): use a better atlas size determination method to have a square atlas

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

    return asset;
}

void free_font_asset(Font_Asset& asset, Renderer* renderer){
    ::free(asset.file.data);
    asset.codepoint_to_info.free();
    ::free(asset.bitmap_data);
    renderer->free_texture(asset.texture);
}

float baseline_to_baseline(Font_Asset* asset, float font_size){
    float relative_font_scale = font_size / asset->bitmap_font_size;
    return (asset->ascent - asset->descent + asset->linegap) * relative_font_scale;
}

vec2 Font_Renderer::render_string(Font_Asset* asset, const char* string, vec2 baseline, float font_size){

    Vertex_Batch_ID batch = renderer->get_vertex_batch(xyuv, PRIMITIVE_TRIANGLES);

    float relative_font_scale = font_size / asset->bitmap_font_size;
    vec2 new_baseline = baseline;
    u32 string_size = strlen(string);

    vertex_xyuv* vertices = (vertex_xyuv*)renderer->get_vertices(batch, 6u * string_size);

    for(u32 icodepoint = 0u; icodepoint != string_size; ++icodepoint){
        char cp = string[icodepoint];
        char next_cp = string[icodepoint + 1u];

        Font_Asset::CodePoint_Info* cp_info = asset->codepoint_to_info.search(cp);
        assert(cp_info);

        ivec2 iquad_min = {
            (s32)fast_floor<float>(new_baseline.x + (float)cp_info->quad_offset_x * relative_font_scale),
            (s32)fast_floor<float>(new_baseline.y + (float)cp_info->quad_offset_y * relative_font_scale)
        };
        ivec2 iquad_max = {
            (s32)fast_floor<float>((float)iquad_min.x + (float)cp_info->quad_width * relative_font_scale),
            (s32)fast_floor<float>((float)iquad_min.y + (float)cp_info->quad_height * relative_font_scale)
        };

        auto pixel_to_render_coordinates = [this](ivec2 pcoord) -> vec2 {
            vec2 output;
            output.x = (float)pcoord.x / (float)width * 2.f - 1.f;
            output.y = (float)pcoord.y / (float)height * 2.f - 1.f;
            return output;
        };

        vec2 quad_min = pixel_to_render_coordinates(iquad_min);
        vec2 quad_max = pixel_to_render_coordinates(iquad_max);

        vertices[0] = {{quad_min.x, quad_min.y}, cp_info->uv_min};
        vertices[1] = {{quad_max.x, quad_min.y}, {cp_info->uv_max.x, cp_info->uv_min.y}};
        vertices[2] = {{quad_min.x, quad_max.y}, {cp_info->uv_min.x, cp_info->uv_max.y}};
        vertices[3] = {{quad_min.x, quad_max.y}, {cp_info->uv_min.x, cp_info->uv_max.y}};
        vertices[4] = {{quad_max.x, quad_min.y}, {cp_info->uv_max.x, cp_info->uv_min.y}};
        vertices[5] = {{quad_max.x, quad_max.y}, cp_info->uv_max};

        vertices = vertices + 6u;

        float kerning = (float)stbtt_GetGlyphKernAdvance(&asset->info, cp, next_cp) * font_size;
        new_baseline.x += cp_info->cursor_advance * relative_font_scale + kerning;
    }

    renderer->use_shader(font_2D);
    renderer->setup_texture_unit(0u, asset->texture, linear_clamp);
    renderer->submit_vertex_batch(batch);
    renderer->free_vertex_batch(batch);

    return new_baseline;
}

Text_Box compute_string_text_box(Font_Asset* asset, const char* string, vec2 baseline, float font_size){
    u32 string_size = strlen(string);
    float relative_font_scale = font_size / asset->bitmap_font_size;

    Text_Box output;
    output.baseline = baseline;
    output.min.x = 0.f;
    output.min.y = asset->descent * relative_font_scale;
    output.max.x = 0.f;
    output.max.y = asset->ascent * relative_font_scale;

    for(u32 icodepoint = 0u; icodepoint != string_size; ++icodepoint){
        char cp = string[icodepoint];
        char next_cp = string[icodepoint + 1u];
        Font_Asset::CodePoint_Info* cp_info = asset->codepoint_to_info.search(cp);
        assert(cp_info);

        float kerning = (float)stbtt_GetGlyphKernAdvance(&asset->info, cp, next_cp) * font_size;
        output.max.x += cp_info->cursor_advance * relative_font_scale + kerning;
    }

    return output;
}

void center_vertical(Text_Box& text, const Layout_Box& layout){
    text.baseline.y = (layout.min.y + layout.max.y) * 0.5f;
}

void center_horizontal(Text_Box& text, const Layout_Box& layout){
    float text_width = text.max.x - text.min.x;
    text.baseline.x = ((layout.min.x + layout.max.x) - text_width) * 0.5f;
}
