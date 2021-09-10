void Font_Stash::create(){
    font_data.asset = nullptr;
    font_data.cache.create();

    packer.create();
    packer.set_packing_area(font_stash_default_dimension, font_stash_default_dimension);

    image = bw_calloc(font_stash_default_dimension * font_stash_default_dimension, sizeof(u8));
    dimension = font_stash_default_dimension;

    texture = Render_Layer_Invalid_Texture;
    texture_dirty = false;
}

void Font_Stash::destroy(){
    font_data.cache.destroy();

    packer.destroy();

    bw_free(image);

    get_engine().render_layer.free_texture(texture);
}

static void increase_stash_area(Font_Stash* stash){
    u32 new_dim = stash->dimension * 2u;

    // NOTE(hugo): increase packing area
    stash->packer.set_packing_area(new_dim, new_dim);

    // NOTE(hugo): reallocate and memcpy with new stride
    void* new_stash = bw_calloc(new_dim * new_dim, sizeof(u8));

    for(u32 iy = 0u; iy != stash->dimension; ++iy){
        size_t src_offset = (stash->dimension - iy - 1u) * stash->dimension;
        size_t dst_offset = (stash->dimension - iy - 1u) * new_dim;

        u8* src_ptr = (u8*)stash->image + src_offset;
        u8* dst_ptr = (u8*)new_stash    + dst_offset;

        memcpy(dst_ptr, src_ptr, stash->dimension);
    }

    bw_free(stash->image);
    stash->image = new_stash;
    stash->dimension = new_dim;

    for(auto& code_point : stash->font_data.cache){
        code_point.value().min *= 0.5f;
        code_point.value().max *= 0.5f;
    }

    stash->texture_dirty = true;
}

static void prepare_stash_texture(Font_Stash* stash){
    if(stash->texture_dirty){
        // NOTE(hugo): increase texture size
        if(stash->texture.width != stash->dimension || stash->texture == Render_Layer_Invalid_Texture){
            assert(stash->texture.height == stash->texture.width);

            get_engine().render_layer.free_texture(stash->texture);
            stash->texture = get_engine().render_layer.get_texture(
                    TEXTURE_FORMAT_R_BYTE,
                    stash->dimension, stash->dimension,
                    TYPE_UBYTE, stash->image);

        // NOTE(hugo): reupload dirty stash
        }else{
            get_engine().render_layer.update_texture(
                    stash->texture, 0u, 0u, stash->dimension, stash->dimension, TYPE_UBYTE, stash->image);
        }

        stash->texture_dirty = false;
    }
}

void Font_Stash::stash_code_point(s32 code_point, u32 font_size){
    assert(font_data.asset);

    Font_Data::Code_Point_Key key;
    key.code_point = code_point;
    key.font_size = font_size;

    Font_Data::Code_Point_Data* data;

    if(font_data.cache.get(key, data)){
        float font_scale = stbtt_ScaleForPixelHeight(&font_data.asset->info, (float)font_size);

        s32 min_x;
        s32 max_x;
        s32 min_y;
        s32 max_y;
        stbtt_GetCodepointBitmapBox(&font_data.asset->info, code_point, font_scale, font_scale, &min_x, &min_y, &max_x, &max_y);

        u32 bmp_width = (u32)(max_x - min_x);
        u32 bmp_height = (u32)(max_y - min_y);

        if(bmp_width != 0u && bmp_height != 0u){
            uivec2 origin = packer.insert_rect(bmp_width + font_stash_padding, bmp_height + font_stash_padding);
            while(origin.x == UINT32_MAX){
                increase_stash_area(this);
                origin = packer.insert_rect(bmp_width + font_stash_padding, bmp_height + font_stash_padding);
            }

            // NOTE(hugo): inverted y because stbtt_MakeCodepointBitmap rasterizes upside-down wrt. GPU textures
            float denom = 1.f / (dimension - 1u);

            data->min.x = origin.x * denom;
            data->max.y = origin.y * denom;
            data->max.x = (origin.x + bmp_width)  * denom;
            data->min.y = (origin.y + bmp_height) * denom;

            size_t offset = origin.y * dimension + origin.x;

            stbtt_MakeCodepointBitmap(&font_data.asset->info,
                    (uchar*)image + offset, bmp_width, bmp_height, dimension,
                    font_scale, font_scale, code_point);

            texture_dirty = true;

        }else{
            font_data.cache.remove(key);

        }
    }
}

s32 Font_Stash::measure_str(const char* str, u32 font_size){
    assert(font_data.asset);

    float font_scale = stbtt_ScaleForPixelHeight(&font_data.asset->info, (float)font_size);

    float width = 0.f;
    while(*str){
        s32 advance;
        stbtt_GetCodepointHMetrics(&font_data.asset->info, *str, &advance, nullptr);

        width += font_scale * advance;
        if(str[1u]) width += font_scale * stbtt_GetCodepointKernAdvance(&font_data.asset->info, str[0u], str[1u]);

        ++str;
    }

    return ceil_s32(width);
}

DEFINE_EQUALITY_OPERATOR(Font_Stash::Font_Data::Code_Point_Key);

void draw_text(Font_Stash* stash, const char* str, ivec2 baseline, u32 font_size, float depth, u32 rgba, u32 target_width, u32 target_height){
    float font_scale = stbtt_ScaleForPixelHeight(&stash->font_data.asset->info, (float)font_size);

    // NOTE(hugo): stash the code point bitmaps
    u32 str_size = 0u;
    while(str[str_size]) stash->stash_code_point(str[str_size++], font_size);

    prepare_stash_texture(stash);

    // NOTE(hugo): generate vertices
    size_t buffer_bytesize = str_size * 6u * sizeof(vertex_xyzrgbauv);
    Transient_Buffer buffer = get_engine().render_layer.get_transient_buffer(buffer_bytesize);
    get_engine().render_layer.format(buffer, xyzrgbauv);
    get_engine().render_layer.checkout(buffer);

    vertex_xyzrgbauv* vptr = (vertex_xyzrgbauv*)buffer.ptr;

    u32 vertex_count = 0u;

    while(*str){
        Font_Stash::Font_Data::Code_Point_Key key;
        key.code_point = *str;
        key.font_size = font_size;

        // NOTE(hugo): a code point may not be in the cache after stash_code_point
        // if the code point cannot be rasterized (empty or undefined)
        Font_Stash::Font_Data::Code_Point_Data* data;
        if(stash->font_data.cache.search(key, data)){
            // NOTE(hugo): bounding box of the codepoint wrt. (current_x, baseline_y)
            s32 min_x;
            s32 min_y;
            s32 max_x;
            s32 max_y;
            stbtt_GetCodepointBox(&stash->font_data.asset->info, *str, &min_x, &min_y, &max_x, &max_y);

            auto furthest_s32 = [](const float x){
                s32 xi = (s32)x;
                return (x < 0.f) ? (xi - 1) : (xi + 1);
            };

            ivec2 BL = {baseline.x + furthest_s32(min_x * font_scale), baseline.y + furthest_s32(min_y * font_scale)};
            ivec2 BR = {baseline.x + furthest_s32(max_x * font_scale), baseline.y + furthest_s32(min_y * font_scale)};
            ivec2 TL = {baseline.x + furthest_s32(min_x * font_scale), baseline.y + furthest_s32(max_y * font_scale)};
            ivec2 TR = {baseline.x + furthest_s32(max_x * font_scale), baseline.y + furthest_s32(max_y * font_scale)};

            float denom_width = 1.f / (float)(target_width - 1u);
            float denom_height = 1.f / (float)(target_height - 1u);

            *vptr++ = {{(float)BL.x * denom_width, (float)BL.y * denom_height, depth}, rgba, uv32((*data).min.x, (*data).min.y)};
            *vptr++ = {{(float)BR.x * denom_width, (float)BR.y * denom_height, depth}, rgba, uv32((*data).max.x, (*data).min.y)};
            *vptr++ = {{(float)TR.x * denom_width, (float)TR.y * denom_height, depth}, rgba, uv32((*data).max.x, (*data).max.y)};

            *vptr++ = {{(float)BL.x * denom_width, (float)BL.y * denom_height, depth}, rgba, uv32((*data).min.x, (*data).min.y)};
            *vptr++ = {{(float)TR.x * denom_width, (float)TR.y * denom_height, depth}, rgba, uv32((*data).max.x, (*data).max.y)};
            *vptr++ = {{(float)TL.x * denom_width, (float)TL.y * denom_height, depth}, rgba, uv32((*data).min.x, (*data).max.y)};

            vertex_count += 6u;
        }

        // NOTE(hugo): advancing the /baseline/ even when the codepoint was not in the cache
        // because the code point may occupy space even when empty or undefined
        s32 advance;
        stbtt_GetCodepointHMetrics(&stash->font_data.asset->info, *str, &advance, nullptr);
        baseline.x += (font_scale * advance);

        if(str[1u]) baseline.x += font_scale * stbtt_GetCodepointKernAdvance(&stash->font_data.asset->info, str[0u], str[1u]);

        ++str;
    }

    get_engine().render_layer.commit(buffer);

    // NOTE(hugo): draw
    get_engine().render_layer.use_shader(text);
    get_engine().render_layer.setup_texture_unit(0u, stash->texture, nearest_clamp);
    get_engine().render_layer.draw(buffer, PRIMITIVE_TRIANGLES, 0u, vertex_count);

    get_engine().render_layer.free_buffer(buffer);
}

// TODO(hugo):
// * account for the first character's min_x being negative ie out of rect
// * measure text to fit width too instead of fitting height only
// * alignment type within rect (left ; right ; center)
void draw_text(Font_Stash* stash, const char* str, const Layout_Rect& rect, float depth, u32 color, u32 target_width, u32 target_height){
    u32 font_size = rect.max.y - rect.min.y;

    s32 ascent;
    s32 descent;
    stbtt_GetFontVMetrics(&stash->font_data.asset->info, &ascent, &descent, nullptr);
    float baseline_ratio = (float)(- descent) / (float)(ascent - descent);

    s32 baseline_y = rect.min.y + floor_s32(baseline_ratio * font_size);
    ivec2 baseline = {rect.min.x, baseline_y};

    draw_text(stash, str, baseline, font_size, depth, color, target_width, target_height);
}
