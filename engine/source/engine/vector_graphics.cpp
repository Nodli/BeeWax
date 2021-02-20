float next_depth(float depth){
    return nextafterf(depth, 1.f);
}

void Vector_Graphics_Renderer::terminate(){
    for(auto& info : batch_storage){
        renderer->free_transient_buffer(info.buffer);
    }
    batch_storage.free();
}

// NOTE(hugo): /vertex_bytesize/ should not change for a given /storage/
static u32 get_batch_index_xyzrgba(u32 nvertices, u32 nindices,
        u32& batch_count, array<Vector_Graphics_Renderer::Batch_Info>& storage, Renderer* renderer){

    // NOTE(hugo): search in used storage
    for(u32 ibatch = 0u; ibatch != batch_count; ++ibatch){
        u32 vcapacity = storage[ibatch].buffer.vbytesize / sizeof(vertex_xyzrgba);
        u32 icapacity = storage[ibatch].buffer.ibytesize / sizeof(u16);

        if(vcapacity - storage[ibatch].vcursor >= nvertices
        && icapacity - storage[ibatch].icursor >= nindices){
            return ibatch;
        }
    }
    // NOTE(hugo): search in free storage
    for(u32 ibatch = batch_count; ibatch != storage.size; ++ibatch){
        u32 vcapacity = storage[ibatch].buffer.vbytesize / sizeof(vertex_xyzrgba);
        u32 icapacity = storage[ibatch].buffer.ibytesize / sizeof(u16);

        if(vcapacity >= nvertices
        && icapacity >= nindices){

            // NOTE(hugo): checkout the buffer when not already in use
            renderer->checkout(storage[ibatch].buffer);
            storage[ibatch].vcursor = 0u;
            storage[ibatch].icursor = 0u;
            ++batch_count;

            return ibatch;
        }
    }

    constexpr size_t default_vbytesize = vector_graphics_vertex_capacity * sizeof(vertex_xyzrgba);
    constexpr size_t default_ibytesize = vector_graphics_index_capacity_multi * vector_graphics_vertex_capacity * sizeof(u16);

    // NOTE(hugo): reserve new storage
    size_t round_nvertices = round_up_pow2(nvertices);
    size_t vbytesize = round_nvertices * sizeof(vertex_xyzrgba);
    size_t ibytesize = vector_graphics_index_capacity_multi * round_nvertices * sizeof(u16);
    vbytesize = max(vbytesize, default_vbytesize);
    ibytesize = max(ibytesize, default_ibytesize);

    Transient_Buffer_Indexed buffer = renderer->get_transient_buffer_indexed(vbytesize, ibytesize);
    renderer->format(buffer, xyzrgba);
    renderer->checkout(buffer);

    u32 batch_index = storage.size;
    storage.push({nvertices, nindices, buffer});
    ++batch_count;

    return batch_index;
}

void Vector_Graphics_Renderer::segment_round(vec2 A, vec2 B, float radius, float depth, vec4 rgba, float dpix){
    u32 ncap_vertices = circular_cap_vertices(dpix * 0.5f, radius);
    u32 aa_vertices = 4u + 2u * ncap_vertices;
    u32 nvertices = 4u + 2u + 2u * ncap_vertices + aa_vertices;
    u32 aa_indices = 12u + 2u * 6u * (ncap_vertices + 1u);
    u32 nindices = 6u + 2u * 3u * (ncap_vertices + 1u) + aa_indices;

    u32 batch_index = get_batch_index_xyzrgba(nvertices, nindices, batch_count, batch_storage, renderer);
    Batch_Info& batch = batch_storage[batch_index];
    vertex_xyzrgba* vptr = (vertex_xyzrgba*)batch.buffer.vptr + batch.vcursor;
    u16* iptr = (u16*)batch.buffer.iptr + batch.icursor;

    u32 batch_offset = batch.vcursor;

    batch.vcursor += nvertices;
    batch.icursor += nindices;

    // --

    vec2 ortho = {B.y - A.y, A.x - B.x};
    ortho = normalized(ortho);

    vec2 ortho_radius = ortho * (radius - 0.5f * dpix);
    vec2 ortho_radius_opp = - ortho_radius;
    vec2 ortho_aa = ortho * dpix * 0.65f;
    //vec2 ortho_aa = {0.f, 0.f};
    vec2 ortho_aa_opp = - ortho_aa;


    vec2 AL = A + ortho_radius_opp;
    vec2 AR = A + ortho_radius;
    vec2 BL = B + ortho_radius_opp;
    vec2 BR = B + ortho_radius;

    vec4 rgba_aa = {rgba.r, rgba.g, rgba.b, 0.f};

    u32 offset = batch_offset;
    u32 in_offset = 0u;

    // NOTE(hugo): body
    {
        *vptr++ = {{AL.x, AL.y, depth}, rgba};
        *vptr++ = {{AR.x, AR.y, depth}, rgba};
        *vptr++ = {{BR.x, BR.y, depth}, rgba};
        *vptr++ = {{BL.x, BL.y, depth}, rgba};

        *iptr++ = batch_offset + 0u;
        *iptr++ = batch_offset + 1u;
        *iptr++ = batch_offset + 3u;
        *iptr++ = batch_offset + 1u;
        *iptr++ = batch_offset + 2u;
        *iptr++ = batch_offset + 3u;

        offset += 4u;
    }

    // NOTE(hugo): body AA
    {
        vec2 AL_aa = AL + ortho_aa_opp;
        vec2 AR_aa = AR + ortho_aa;
        vec2 BL_aa = BL + ortho_aa_opp;
        vec2 BR_aa = BR + ortho_aa;

        *vptr++ = {{AL_aa.x, AL_aa.y, depth}, rgba_aa};
        *vptr++ = {{AR_aa.x, AR_aa.y, depth}, rgba_aa};
        *vptr++ = {{BR_aa.x, BR_aa.y, depth}, rgba_aa};
        *vptr++ = {{BL_aa.x, BL_aa.y, depth}, rgba_aa};

        *iptr++ = batch_offset + 4;
        *iptr++ = batch_offset + 0;
        *iptr++ = batch_offset + 7;
        *iptr++ = batch_offset + 0;
        *iptr++ = batch_offset + 3;
        *iptr++ = batch_offset + 7;

        *iptr++ = batch_offset + 1;
        *iptr++ = batch_offset + 5;
        *iptr++ = batch_offset + 2;
        *iptr++ = batch_offset + 5;
        *iptr++ = batch_offset + 6;
        *iptr++ = batch_offset + 2;

        offset += 4u;
    }

    in_offset = offset + 1u;
    // NOTE(hugo): A's cap
    {
        u16 AL_index = batch_offset + 0u;
        u16 AR_index = batch_offset + 1u;
        *vptr++ = {{A.x, A.y, depth}, rgba};
        u16 A_index = offset++;

        vec2 to_rot = ortho_radius_opp;
        for(u32 ivert = 0u; ivert != ncap_vertices; ++ivert){
            float rad = PI * (float)(ivert + 1u) / (float)(ncap_vertices + 1u);
            vec2 vert = A + rotated(to_rot, rad);
            *vptr++ = {{vert.x, vert.y, depth}, rgba};
        }

        *iptr++ = A_index;
        *iptr++ = AL_index;
        *iptr++ = offset;

        for(u32 itri = 1u; itri < ncap_vertices; ++itri){
            *iptr++ = A_index;
            *iptr++ = offset++;
            *iptr++ = offset;
        }

        *iptr++ = A_index;
        *iptr++ = offset++;
        *iptr++ = AR_index;
    }

    // NOTE(hugo): A's cap AA
    {
        u16 AL_index = batch_offset + 0u;
        u16 AR_index = batch_offset + 1u;
        u16 AL_aa_index = batch_offset + 4u;
        u16 AR_aa_index = batch_offset + 5u;

        vec2 to_rot = ortho_radius_opp + ortho_aa_opp;
        for(u32 ivert = 0u; ivert != ncap_vertices; ++ivert){
            float rad = PI * (float)(ivert + 1u) / (float)(ncap_vertices + 1u);
            vec2 vert = A + rotated(to_rot, rad);
            *vptr++ = {{vert.x, vert.y, depth}, rgba_aa};
        }

        *iptr++ = AL_index;
        *iptr++ = AL_aa_index;
        *iptr++ = in_offset;
        *iptr++ = AL_aa_index;
        *iptr++ = offset;
        *iptr++ = in_offset;

        for(u32 iquad = 1u; iquad < ncap_vertices; ++iquad){
            *iptr++ = in_offset++;
            *iptr++ = offset;
            *iptr++ = in_offset;
            *iptr++ = offset++;
            *iptr++ = offset;
            *iptr++ = in_offset;
        }

        *iptr++ = in_offset;
        *iptr++ = offset;
        *iptr++ = AR_index;
        *iptr++ = offset++;
        *iptr++ = AR_aa_index;
        *iptr++ = AR_index;
    }

    in_offset = offset + 1u;
    // NOTE(hugo): B's cap
    {
        u16 BR_index = batch_offset + 2u;
        u16 BL_index = batch_offset + 3u;
        *vptr++ = {{B.x, B.y, depth}, rgba};
        u16 B_index = offset++;

        vec2 to_rot = ortho_radius;
        for(u32 ivert = 0u; ivert != ncap_vertices; ++ivert){
            float rad = PI * (float)(ivert + 1u) / (float)(ncap_vertices + 1u);
            vec2 vert = B + rotated(to_rot, rad);
            *vptr++ = {{vert.x, vert.y, depth}, rgba};
        }

        *iptr++ = B_index;
        *iptr++ = BR_index;
        *iptr++ = offset;

        for(u32 itri = 1u; itri < ncap_vertices; ++itri){
            *iptr++ = B_index;
            *iptr++ = offset++;
            *iptr++ = offset;
        }

        *iptr++ = B_index;
        *iptr++ = offset++;
        *iptr++ = BL_index;
    }

    // NOTE(hugo): B's cap AA
    {
        u16 BL_index = batch_offset + 3u;
        u16 BR_index = batch_offset + 2u;
        u16 BL_aa_index = batch_offset + 7u;
        u16 BR_aa_index = batch_offset + 6u;

        vec2 to_rot = ortho_radius + ortho_aa;
        for(u32 ivert = 0u; ivert != ncap_vertices; ++ivert){
            float rad = PI * (float)(ivert + 1u) / (float)(ncap_vertices + 1u);
            vec2 vert = B + rotated(to_rot, rad);
            *vptr++ = {{vert.x, vert.y, depth}, rgba_aa};
        }

        *iptr++ = BR_index;
        *iptr++ = BR_aa_index;
        *iptr++ = in_offset;
        *iptr++ = BR_aa_index;
        *iptr++ = offset;
        *iptr++ = in_offset;

        for(u32 iquad = 1u; iquad < ncap_vertices; ++iquad){
            *iptr++ = in_offset++;
            *iptr++ = offset;
            *iptr++ = in_offset;
            *iptr++ = offset++;
            *iptr++ = offset;
            *iptr++ = in_offset;
        }

        *iptr++ = in_offset;
        *iptr++ = offset;
        *iptr++ = BL_index;
        *iptr++ = offset++;
        *iptr++ = BL_aa_index;
        *iptr++ = BL_index;
    }
}

void Vector_Graphics_Renderer::circle(vec2 center, float radius, float depth, vec4 rgba, float dpix){
}

void Vector_Graphics_Renderer::draw(){
    for(u32 ibatch = 0u; ibatch != batch_count; ++ibatch){
        Batch_Info& batch = batch_storage[ibatch];

        renderer->commit(batch.buffer);
        renderer->draw(batch.buffer, PRIMITIVE_TRIANGLES, TYPE_USHORT, batch.icursor, 0u);
    }
}

void Vector_Graphics_Renderer::next_frame(){
    batch_count = 0u;
}

#if 0
float Vector_Graphics_Renderer::determine_max_error(float camera_height, u32 window_height){
    return 0.5f * camera_height / (float)window_height;
}


void Vector_Graphics_Renderer::segment(vec2 A, vec2 B, vec4 color_linear, float radius, float depth, u32 ncap_vertices, float aa_border){
}

void Vector_Graphics_Renderer::segment(vec2 A, vec2 B, vec4 color_linear, float radius, float depth, float max_error){
    u32 ncap_vertices = circular_cap_vertices(max_error, radius);
    segment(A, B, color_linear, radius, depth, ncap_vertices, max_error);
}

void Vector_Graphics_Renderer::circle(vec2 center, vec4 color_linear, float radius, float depth, u32 nperi_vertices){
    assert(nperi_vertices > 2u);
    u32 nvertices = 3u * nperi_vertices;

    u32 batch_index = get_batch_index_xyzrgba(nvertices, batch_count, batch_storage, renderer);
    Batch_Info& batch = batch_storage[batch_index];
    vertex_xyzrgba* ptr = (vertex_xyzrgba*)batch.buffer.ptr + batch.cursor;
    batch.cursor += nvertices;

    vec2 start = {center.x + radius, center.y};
    vec2 end = start;

    for(u32 itri = 0u; itri != nperi_vertices - 1u; ++itri){
        ptr[0u] = {{center.x, center.y, depth}, color_linear};
        ptr[1u] = {{start.x, start.y, depth}, color_linear};

        float rad = 2.f * PI * (float)(itri + 1u) / (float)(nperi_vertices);
        start = center + rotated({radius, 0.f}, rad);
        ptr[2u] = {{start.x, start.y, depth}, color_linear};

        ptr += 3u;
    }

    ptr[0u] = {{center.x, center.y, depth}, color_linear};
    ptr[1u] = {{start.x, start.y, depth}, color_linear};
    ptr[2u] = {{end.x, end.y, depth}, color_linear};
}

void Vector_Graphics_Renderer::circle(vec2 center, vec4 color_linear, float radius, float depth, float max_error){
    u32 nperi_vertices = max(circle_vertices(max_error, radius), 3u);
    circle(center, color_linear, radius, depth, nperi_vertices);
}

#endif
