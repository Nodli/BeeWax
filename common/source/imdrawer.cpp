void ImDrawer::create(){
    commands.create();
    buffers.create();
    indexed_buffers.create();
}

void ImDrawer::destroy(){
    for(auto& buffer : buffers)         get_engine().render_layer.free_buffer(buffer.buffer);
    for(auto& buffer : indexed_buffers) get_engine().render_layer.free_buffer(buffer.buffer);
    commands.destroy();
    buffers.destroy();
    indexed_buffers.destroy();
}

void ImDrawer::new_frame(){
    commands.clear();
    for(auto& buffer : buffers){
        get_engine().render_layer.checkout(buffer.buffer);
        buffer.vertex_count = 0u;
    }
    for(auto& buffer : indexed_buffers){
        get_engine().render_layer.checkout(buffer.buffer);
        buffer.vertex_count = 0u;
        buffer.index_count = 0u;
    }
}

void ImDrawer::draw(){
    for(auto& buffer : buffers){
        get_engine().render_layer.commit(buffer.buffer);
    }
    for(auto& buffer : indexed_buffers){
        get_engine().render_layer.commit(buffer.buffer);
    }

    Shader_Name current_shader = SHADER_NONE;

    for(auto& command : commands){
        if(current_shader != command.shader){
            get_engine().render_layer.use_shader(command.shader);
            current_shader = command.shader;
        }

        switch(command.type){
            case POLYGON:
                get_engine().render_layer.draw(buffers[command.polygon.buffer_index].buffer, PRIMITIVE_TRIANGLES, command.polygon.vertex_index, command.polygon.vertex_count);
                break;
            case POLYGON_INDEXED:
                get_engine().render_layer.draw(indexed_buffers[command.polygon_indexed.buffer_index].buffer, PRIMITIVE_TRIANGLES, TYPE_UINT, command.polygon_indexed.index_index, command.polygon_indexed.index_count);
                break;
            case POLYGON_TEXTURED:
                get_engine().render_layer.setup_texture_unit(0u, command.polygon_textured.texture, nearest_nearest_clamp);
                get_engine().render_layer.draw(buffers[command.polygon.buffer_index].buffer, PRIMITIVE_TRIANGLES, command.polygon_textured.vertex_index, command.polygon_textured.vertex_count);
                break;
            default:
                assert(false);
                break;
        }
    }
}

static constexpr u32 nvertices_per_buffer = 4096u * 16u;
static constexpr u32 nindices_per_buffer = 3u * nvertices_per_buffer;

static u32 get_buffer_with_format(ImDrawer& drawer, Vertex_Format_Name vformat, size_t vbytesize, u32 nvertices){
    for(u32 ibuffer = 0u; ibuffer != drawer.buffers.size; ++ibuffer){
        ImDrawer::Buffer& buffer = drawer.buffers[ibuffer];

        if(buffer.vertex_format_name == vformat
        && (buffer.buffer.bytesize / vbytesize - buffer.vertex_count) >= nvertices){
            return ibuffer;
        }
    }

    u32 new_buffer_index = drawer.buffers.size;

    ImDrawer::Buffer new_buffer;
    new_buffer.vertex_format_name = vformat;
    new_buffer.vertex_count = 0u;
    new_buffer.buffer = get_engine().render_layer.get_transient_buffer(nvertices_per_buffer * vbytesize);

    get_engine().render_layer.format(new_buffer.buffer, vformat);
    get_engine().render_layer.checkout(new_buffer.buffer);

    drawer.buffers.push(new_buffer);
    return new_buffer_index;
}

static u32 get_indexed_buffer_with_format(ImDrawer& drawer, Vertex_Format_Name vformat, size_t vbytesize, u32 nvertices, u32 nindices){
    for(u32 ibuffer = 0u; ibuffer != drawer.indexed_buffers.size; ++ibuffer){
        ImDrawer::Indexed_Buffer& buffer = drawer.indexed_buffers[ibuffer];

        if(buffer.vertex_format_name == vformat
        && (buffer.buffer.vbytesize / vbytesize - buffer.vertex_count) >= nvertices
        && (buffer.buffer.ibytesize / sizeof(u32) - buffer.index_count) >= nindices){
            return ibuffer;
        }
    }

    u32 new_buffer_index = drawer.indexed_buffers.size;

    ImDrawer::Indexed_Buffer new_buffer;
    new_buffer.vertex_format_name = vformat;
    new_buffer.vertex_count = 0u;
    new_buffer.index_count = 0u;
    new_buffer.buffer = get_engine().render_layer.get_transient_buffer_indexed(
            nvertices_per_buffer * vbytesize,
            nindices_per_buffer * sizeof(u32));

    get_engine().render_layer.format(new_buffer.buffer, vformat);
    get_engine().render_layer.checkout(new_buffer.buffer);

    drawer.indexed_buffers.push(new_buffer);
    return new_buffer_index;
}

void ImDrawer::command_image(const Texture& texture, vec2 position, vec2 size, float depth, Shader_Name shader){
    // NOTE(hugo): find buffer
    u32 buffer_index = get_buffer_with_format(*this, xyzuv, sizeof(vertex_xyzuv), 6u);
    Buffer& buffer = buffers[buffer_index];

    // NOTE(hugo): emit vertices
    u32 vindex = buffer.vertex_count;
    vertex_xyzuv* vptr = (vertex_xyzuv*)buffer.buffer.ptr + vindex;

    vec2 hsize = size * 0.5f;

    *vptr++ = {{position.x - hsize.x, position.y - hsize.y, depth}, uv32(0.f, 0.f)};
    *vptr++ = {{position.x + hsize.x, position.y - hsize.y, depth}, uv32(1.f, 0.f)};
    *vptr++ = {{position.x + hsize.x, position.y + hsize.y, depth}, uv32(1.f, 1.f)};

    *vptr++ = {{position.x - hsize.x, position.y - hsize.y, depth}, uv32(0.f, 0.f)};
    *vptr++ = {{position.x + hsize.x, position.y + hsize.y, depth}, uv32(1.f, 1.f)};
    *vptr++ = {{position.x - hsize.x, position.y + hsize.y, depth}, uv32(0.f, 1.f)};

    buffer.vertex_count += 6u;

    // NOTE(hugo): queue command
    Command command;
    command.type = Command_Type::POLYGON_TEXTURED;
    command.shader = shader;
    command.polygon_textured.buffer_index = buffer_index;
    command.polygon_textured.vertex_index = vindex;
    command.polygon_textured.vertex_count = 6u;
    command.polygon_textured.texture = texture;

    commands.push(command);
}

void ImDrawer::command_disc(vec2 position, float radius, float depth, u32 rgba, float dpix, Shader_Name shader){
    u32 nvertices_perimeter = circle_sectors(radius, dpix);
    assert(nvertices_perimeter > 2u);

    u32 nvertices = nvertices_perimeter + 1u;
    u32 nindices = 3u * nvertices_perimeter;

    // NOTE(hugo): find buffer
    u32 buffer_index = get_indexed_buffer_with_format(*this, xyzrgba, sizeof(vertex_xyzrgba), nvertices, nindices);
    Indexed_Buffer& buffer = indexed_buffers[buffer_index];

    // NOTE(hugo): emit vertices
    u32 vindex = buffer.vertex_count;
    {
        vertex_xyzrgba* vptr = (vertex_xyzrgba*)buffer.buffer.vptr + vindex;
        *vptr++ = {{position.x, position.y, depth}, rgba};
        *vptr++ = {{position.x + radius, position.y, depth}, rgba};
        for(u32 ivert = 1u; ivert != nvertices_perimeter; ++ivert){
            float rad = 2.f * PI * (float)ivert / (float)nvertices_perimeter;
            float vradx = radius * bw::cos(rad);
            float vrady = radius * bw::sin(rad);
            *vptr++ = {{position.x + vradx, position.y + vrady, depth}, rgba};
        }
    }

    // NOTE(hugo): emit indices
    u32 iindex = buffer.index_count;
    {
        u32* iptr = (u32*)buffer.buffer.iptr + iindex;
        for(u32 itri = 0u; itri != nvertices_perimeter - 1u; ++itri){
            *iptr++ = vindex + 0u;
            *iptr++ = vindex + itri + 1u;
            *iptr++ = vindex + itri + 2u;
        }
        *iptr++ = vindex + 0u;
        *iptr++ = vindex + nvertices_perimeter;
        *iptr++ = vindex + 1u;
    }

    // NOTE(hugo): update buffer
    buffer.vertex_count += nvertices;
    buffer.index_count += nindices;

    // NOTE(hugo): queue command
    Command command;
    command.type = Command_Type::POLYGON_INDEXED;
    command.shader = shader;
    command.polygon_indexed.buffer_index = buffer_index;
    command.polygon_indexed.index_index = iindex;
    command.polygon_indexed.index_count = nindices;

    commands.push(command);
}

void ImDrawer::command_disc_arc(vec2 position, vec2 arc_start, float arc_span, float depth, u32 rgba, float dpix, Shader_Name shader){
    float radius = length(arc_start);
    u32 nsectors = circle_arc_sectors(radius, arc_span, dpix);
    u32 nvertices_perimeter = nsectors + 1u;
    u32 nvertices = nvertices_perimeter + 1u;
    u32 nindices = 3u * nsectors;

    // NOTE(hugo): find buffer
    u32 buffer_index = get_indexed_buffer_with_format(*this, xyzrgba, sizeof(vertex_xyzrgba), nvertices, nindices);
    Indexed_Buffer& buffer = indexed_buffers[buffer_index];

    // NOTE(hugo): emit vertices
    u32 vindex = buffer.vertex_count;
    {
        vertex_xyzrgba* vptr = (vertex_xyzrgba*)buffer.buffer.vptr + vindex;
        *vptr++ = {{position.x, position.y, depth}, rgba};
        *vptr++ = {{position.x + arc_start.x, position.y + arc_start.y, depth}, rgba};
        for(u32 ivert = 1u; ivert != nvertices_perimeter; ++ivert){
            float rad = arc_span * (float)ivert / (float)(nvertices_perimeter - 1u);
            vec2 vrad = rotated(arc_start, rad);
            *vptr++ = {{position.x + vrad.x, position.y + vrad.y, depth}, rgba};
        }
    }

    // NOTE(hugo): emit indices
    u32 iindex = buffer.index_count;
    {
        u32* iptr = (u32*)buffer.buffer.iptr + iindex;
        for(u32 itri = 0u; itri != nvertices_perimeter - 1u; ++itri){
            *iptr++ = vindex + 0u;
            *iptr++ = vindex + itri + 1u;
            *iptr++ = vindex + itri + 2u;
        }
    }

    // NOTE(hugo): update buffer
    buffer.vertex_count += nvertices;
    buffer.index_count += nindices;

    // NOTE(hugo): queue command
    Command command;
    command.type = Command_Type::POLYGON_INDEXED;
    command.shader = shader;
    command.polygon_indexed.buffer_index = buffer_index;
    command.polygon_indexed.index_index = iindex;
    command.polygon_indexed.index_count = nindices;

    commands.push(command);
}

void ImDrawer::command_capsule(vec2 pA, vec2 pB, float radius, float depth, u32 rgba, float dpix, Shader_Name shader){
    u32 nindices_body = 6u;

    u32 nsectors_cap = circle_arc_sectors(radius, PI, dpix);
    u32 nvertices_cap = nsectors_cap + 1u;
    u32 nindices_cap = 3u * (nsectors_cap - 1u);

    u32 nvertices = 2u * nvertices_cap;
    u32 nindices = nindices_body + 2u * nindices_cap;

    // NOTE(hugo): find buffer
    u32 buffer_index = get_indexed_buffer_with_format(*this, xyzrgba, sizeof(vertex_xyzrgba), nvertices, nindices);
    Indexed_Buffer& buffer = indexed_buffers[buffer_index];

    // NOTE(hugo): emit vertices
    u32 vindex = buffer.vertex_count;
    {
        vec2 dirAB = normalized(pB - pA);
        vec2 ortho_r = vec2({dirAB.y, - dirAB.x}) * radius;
        vec2 ortho_l = - ortho_r;

        vertex_xyzrgba* vptr = (vertex_xyzrgba*)buffer.buffer.vptr + vindex;

        // NOTE(hugo): capA vertices
        *vptr++ = {{pA.x + ortho_l.x, pA.y + ortho_l.y, depth}, rgba};
        for(u32 ivert = 1u; ivert != nsectors_cap; ++ivert){
            float rad = PI * (float)ivert / (float)(nsectors_cap);
            vec2 vrad = rotated(ortho_l, rad);
            *vptr++ = {{pA.x + vrad.x, pA.y + vrad.y, depth}, rgba};
        }
        *vptr++ = {{pA.x + ortho_r.x, pA.y + ortho_r.y, depth}, rgba};

        // NOTE(hugo): capB vertices
        *vptr++ = {{pB.x + ortho_r.x, pB.y + ortho_r.y, depth}, rgba};
        for(u32 ivert = 1u; ivert != nsectors_cap; ++ivert){
            float rad = PI * (float)ivert / (float)(nsectors_cap);
            vec2 vrad = rotated(ortho_r, rad);
            *vptr++ = {{pB.x + vrad.x, pB.y + vrad.y, depth}, rgba};
        }
        *vptr++ = {{pB.x + ortho_l.x, pB.y + ortho_l.y, depth}, rgba};
    }

    // NOTE(hugo): emit indices
    u32 iindex = buffer.index_count;
    {
        u32* iptr = (u32*)buffer.buffer.iptr + iindex;

        u32 iA_l = vindex;
        u32 iA_r = iA_l + nsectors_cap;
        u32 iB_r = iA_r + 1u;
        u32 iB_l = iB_r + nsectors_cap;

        // NOTE(hugo): body indices
        *iptr++ = iA_l;
        *iptr++ = iA_r;
        *iptr++ = iB_r;
        *iptr++ = iA_l;
        *iptr++ = iB_r;
        *iptr++ = iB_l;

        // NOTE(hugo): capA & capB indices
        auto emit_indices_cap = [&](u32 ibegin, u32 iend){
            assert(ibegin < iend);

            u32 dindex = iend - ibegin;
            u32 ntri_end = (dindex - 1u) / 2u;
            u32 ntri_begin = ntri_end + (dindex + 1u) % 2u;

            u32* begin_iptr = iptr;
            for(u32 itri_begin = 0u; itri_begin != ntri_begin; ++itri_begin){
                begin_iptr[6u * itri_begin + 0u] = itri_begin + ibegin;
                begin_iptr[6u * itri_begin + 1u] = itri_begin + ibegin + 1u;
                begin_iptr[6u * itri_begin + 2u] = iend - itri_begin;
            }
            u32* end_iptr = iptr + 3u;
            for(u32 itri_end = 0u; itri_end != ntri_end; ++itri_end){
                end_iptr[6u * itri_end + 0u] = iend - 1u - itri_end;
                end_iptr[6u * itri_end + 1u] = iend - itri_end;
                end_iptr[6u * itri_end + 2u] = itri_end + ibegin + 1u;
            }
            iptr += 3u * (ntri_end + ntri_begin);
        };
        emit_indices_cap(iA_l, iA_r);
        emit_indices_cap(iB_r, iB_l);
    }

    // NOTE(hugo): update buffer
    buffer.vertex_count += nvertices;
    buffer.index_count += nindices;

    // NOTE(hugo): queue command
    Command command;
    command.type = Command_Type::POLYGON_INDEXED;
    command.shader = shader;
    command.polygon_indexed.buffer_index = buffer_index;
    command.polygon_indexed.index_index = iindex;
    command.polygon_indexed.index_count = nindices;

    commands.push(command);
}

void ImDrawer::command_circle(vec2 position, float radius_start, float dradius, float depth, u32 rgba, float dpix, Shader_Name shader){
    float radius_min = radius_start;
    float radius_max = radius_start + dradius;

    u32 nvertices_perimeter = circle_sectors(radius_max, dpix);
    assert(nvertices_perimeter > 2u);

    u32 nvertices = 2u * nvertices_perimeter;
    u32 nindices = 6u * nvertices_perimeter;

    // NOTE(hugo): find buffer
    u32 buffer_index = get_indexed_buffer_with_format(*this, xyzrgba, sizeof(vertex_xyzrgba), nvertices, nindices);
    Indexed_Buffer& buffer = indexed_buffers[buffer_index];

    // NOTE(hugo): emit vertices
    u32 vindex = buffer.vertex_count;
    {
        vertex_xyzrgba* vptr = (vertex_xyzrgba*)buffer.buffer.vptr + vindex;
        *vptr++ = {{position.x + radius_min, position.y, depth}, rgba};
        *vptr++ = {{position.x + radius_max, position.y, depth}, rgba};
        for(u32 ivert = 1u; ivert != nvertices_perimeter; ++ivert){
            float rad = 2.f * PI * (float)ivert / (float)nvertices_perimeter;
            float vradx = bw::cos(rad);
            float vrady = bw::sin(rad);
            *vptr++ = {{position.x + radius_min * vradx, position.y + radius_min * vrady, depth}, rgba};
            *vptr++ = {{position.x + radius_max * vradx, position.y + radius_max * vrady, depth}, rgba};
        }
    }

    // NOTE(hugo): emit indices
    u32 iindex = buffer.index_count;
    {
        u32* iptr = (u32*)buffer.buffer.iptr + iindex;
        for(u32 isector = 0u; isector != nvertices_perimeter - 1u; ++isector){
            *iptr++ = vindex + isector * 2u + 0u;
            *iptr++ = vindex + isector * 2u + 1u;
            *iptr++ = vindex + isector * 2u + 2u;
            *iptr++ = vindex + isector * 2u + 2u;
            *iptr++ = vindex + isector * 2u + 1u;
            *iptr++ = vindex + isector * 2u + 3u;
        }
        *iptr++ = vindex + (nvertices_perimeter - 1u) * 2u + 0u;
        *iptr++ = vindex + (nvertices_perimeter - 1u) * 2u + 1u;
        *iptr++ = vindex + 0u;
        *iptr++ = vindex + 0u;
        *iptr++ = vindex + (nvertices_perimeter - 1u) * 2u + 1u;
        *iptr++ = vindex + 1u;
    }

    // NOTE(hugo): update buffer
    buffer.vertex_count += nvertices;
    buffer.index_count += nindices;

    // NOTE(hugo): queue command
    Command command;
    command.type = Command_Type::POLYGON_INDEXED;
    command.shader = shader;
    command.polygon_indexed.buffer_index = buffer_index;
    command.polygon_indexed.index_index = iindex;
    command.polygon_indexed.index_count = nindices;

    commands.push(command);
}

void ImDrawer::command_circle_arc(vec2 position, vec2 arc_start, float dradius, float arc_span, float depth, u32 rgba, float dpix, Shader_Name shader){
    float radius_min = length(arc_start);
    float radius_max = radius_min + dradius;

    u32 nsectors = circle_arc_sectors(radius_max, arc_span, dpix);
    u32 nvertices_perimeter = nsectors + 1u;
    assert(nvertices_perimeter > 2u);

    u32 nvertices = 2u * nvertices_perimeter;
    u32 nindices = 6u * nvertices_perimeter;

    // NOTE(hugo): find buffer
    u32 buffer_index = get_indexed_buffer_with_format(*this, xyzrgba, sizeof(vertex_xyzrgba), nvertices, nindices);
    Indexed_Buffer& buffer = indexed_buffers[buffer_index];

    float min_to_max_ratio = radius_max / radius_min;
    vec2 arc_end = min_to_max_ratio * arc_start;

    // NOTE(hugo): emit vertices
    u32 vindex = buffer.vertex_count;
    {
        vertex_xyzrgba* vptr = (vertex_xyzrgba*)buffer.buffer.vptr + vindex;
        *vptr++ = {{position.x + arc_start.x, position.y + arc_start.y, depth}, rgba};
        *vptr++ = {{position.x + arc_end.x, position.y + arc_end.y, depth}, rgba};
        for(u32 ivert = 1u; ivert != nvertices_perimeter; ++ivert){
            float rad = arc_span * (float)ivert / (float)(nvertices_perimeter - 1u);
            vec2 vrad_min = rotated(arc_start, rad);
            vec2 vrad_max = min_to_max_ratio * vrad_min;
            *vptr++ = {{position.x + vrad_min.x, position.y + vrad_min.y, depth}, rgba};
            *vptr++ = {{position.x + vrad_max.x, position.y + vrad_max.y, depth}, rgba};
        }
    }

    // NOTE(hugo): emit indices
    u32 iindex = buffer.index_count;
    {
        u32* iptr = (u32*)buffer.buffer.iptr + iindex;
        for(u32 isector = 0u; isector != nvertices_perimeter - 1u; ++isector){
            *iptr++ = vindex + isector * 2u + 0u;
            *iptr++ = vindex + isector * 2u + 1u;
            *iptr++ = vindex + isector * 2u + 2u;
            *iptr++ = vindex + isector * 2u + 2u;
            *iptr++ = vindex + isector * 2u + 1u;
            *iptr++ = vindex + isector * 2u + 3u;
        }
    }

    // NOTE(hugo): update buffer
    buffer.vertex_count += nvertices;
    buffer.index_count += nindices;

    // NOTE(hugo): queue command
    Command command;
    command.type = Command_Type::POLYGON_INDEXED;
    command.shader = shader;
    command.polygon_indexed.buffer_index = buffer_index;
    command.polygon_indexed.index_index = iindex;
    command.polygon_indexed.index_count = nindices;

    commands.push(command);
}
