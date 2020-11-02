void Font_Rendering::setup_from_file(const char* ttf_tile){
    font_file = read_file("./data/Roboto_Font/Roboto-Black.ttf");

    if(!stbtt_InitFont(&font_info, (const unsigned char*)font_file.data, 0)){
        LOG_ERROR("Failed stbtt_InitFont()");
        return;
    }

    font_scaling = stbtt_ScaleForMappingEmToPixels(&font_info, 1.f);

    s32 integer_ascend;
    s32 integer_descent;
    s32 integer_line_gap;
    stbtt_GetFontVMetrics(&font_info, &integer_ascend, &integer_descent, &integer_line_gap);
    font_ascend = (float)integer_ascend * font_scaling;
    font_descent = (float)integer_descent * font_scaling;
    font_baseline_to_baseline = font_ascend - font_descent + (float)integer_line_gap * font_scaling;

    for(u32 icodepoint = 0u; icodepoint != 256; ++icodepoint){
        unsigned char codepoint = icodepoint;
        codepoint_info& info = ascii[codepoint];
        info.glyph_index = stbtt_FindGlyphIndex(&font_info, codepoint);

        s32 integer_advance_width;
        s32 integer_left_side_bearing;
        stbtt_GetGlyphHMetrics(&font_info, info.glyph_index, &integer_advance_width, &integer_left_side_bearing);
        info.glyph_advance_width = integer_advance_width * font_scaling;
        info.glyph_left_side_bearing = integer_left_side_bearing * font_scaling;

        // TODO(hugo): use scratch memory here
        stbtt_vertex* glyph_vertices;
        s32 glyph_nvertices = stbtt_GetGlyphShape(&font_info, info.glyph_index, &glyph_vertices);

        // NOTE(hugo): count the number of vertices we need in the vertex buffer
        info.number_of_vertices = 0u;
        for(u32 ivertex = 0u; ivertex != glyph_nvertices; ++ivertex){
            if(glyph_vertices[ivertex].type == STBTT_vline){
                info.number_of_vertices += 2u;
            }else if(glyph_vertices[ivertex].type == STBTT_vcurve){
                info.number_of_vertices += 4u;
            }else if(glyph_vertices[ivertex].type == STBTT_vcubic){
                info.number_of_vertices += 8u;
            }
        }

        // NOTE(hugo): copy the vertices in the vertex buffer
        info.vertex_offset = font_vertices.size;

        if(info.number_of_vertices != 0u){
            font_vertices.set_size(font_vertices.size + info.number_of_vertices);

            vec2* output_vertices = &font_vertices[info.vertex_offset];
            u32 output_ivertex = 0u;

            vec2 previous;
            vec2 current;
            for(u32 ivertex = 0u; ivertex != glyph_nvertices; ++ivertex){
                stbtt_vertex* glyph_vertex = glyph_vertices + ivertex;
                if(glyph_vertex->type == STBTT_vline){
                    current.x = glyph_vertex->x * font_scaling;
                    current.y = glyph_vertex->y * font_scaling;

                    output_vertices[output_ivertex++] = previous;
                    output_vertices[output_ivertex++] = current;

                    previous = current;

                }else if(glyph_vertex->type == STBTT_vcurve){
                    vec2 control = {glyph_vertex->cx * font_scaling, glyph_vertex->cy * font_scaling};

                    current.x = glyph_vertex->x * font_scaling;
                    current.y = glyph_vertex->y * font_scaling;

                    output_vertices[output_ivertex++] = previous;
                    output_vertices[output_ivertex++] = control;
                    output_vertices[output_ivertex++] = control;
                    output_vertices[output_ivertex++] = current;

                    previous = current;

                }else if(glyph_vertex->type == STBTT_vcubic){
                    vec2 controlA = {glyph_vertex->cx * font_scaling, glyph_vertex->cy * font_scaling};
                    vec2 controlB = {glyph_vertex->cx1 * font_scaling, glyph_vertex->cy1 * font_scaling};
                    vec2 intermediate = (controlA + controlB) / 2.f;

                    current.x = glyph_vertex->x * font_scaling;
                    current.y = glyph_vertex->y * font_scaling;

                    output_vertices[output_ivertex++] = previous;
                    output_vertices[output_ivertex++] = controlA;
                    output_vertices[output_ivertex++] = controlA;
                    output_vertices[output_ivertex++] = intermediate;
                    output_vertices[output_ivertex++] = intermediate;
                    output_vertices[output_ivertex++] = controlB;
                    output_vertices[output_ivertex++] = controlB;
                    output_vertices[output_ivertex++] = current;

                    previous = current;

                }else if(glyph_vertex->type == STBTT_vmove){
                    previous.x = glyph_vertex->x * font_scaling;
                    previous.y = glyph_vertex->y * font_scaling;
                }
            }
        }

        stbtt_FreeShape(&font_info, glyph_vertices);
    }
}

void Font_Rendering::terminate(){
    free(font_file.data);
    font_vertices.free();
    *this = Font_Rendering();
}

void Font_Rendering::batch_string(Renderer* renderer, Vertex_Batch_ID batch, const char* string, vec2 baseline_origin, float font_scale, vec4 color){
    while(*string != '\0'){
        codepoint_info& info = ascii[*string];

        vec2* codepoint_vertices = &font_vertices[info.vertex_offset];
        vertex_xyrgba* batch_vertices = (vertex_xyrgba*)renderer->get_vertices(batch, info.number_of_vertices);

        for(u32 ivertex = 0u; ivertex != info.number_of_vertices; ++ivertex){
            batch_vertices[ivertex].vposition = (codepoint_vertices[ivertex] * font_scale) + baseline_origin;
            batch_vertices[ivertex].vcolor = color;
        }

        float kerning = 0.f;
        codepoint_info& next_info = ascii[*(string + 1u)];
        kerning = stbtt_GetGlyphKernAdvance(&font_info, info.glyph_index, next_info.glyph_index) * font_scaling;

        baseline_origin.x += (info.glyph_advance_width + kerning) * font_scale;
        ++string;
    }
}
