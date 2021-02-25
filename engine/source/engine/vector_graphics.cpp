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

void Vector_Graphics_Renderer::rect(vec2 min, vec2 max, float depth, vec4 rgba, float dpix, bool anti_aliasing){
    float size_aa = 0.45f * dpix;
    float offset_aa = 0.90f * dpix;
    vec4 rgba_aa = {rgba.r, rgba.g, rgba.b, 0.f};

    // NOTE(hugo): coverage based anti-aliasing for thin rect
    {
        float dx = max.x - min.x;
        float dy = max.y - min.y;

        float dsize = 0.5f * dpix;
        float coverage = 1.f;

        if(dx < dpix){
            float mid_x = 0.5f * (max.x + min.x);

            min.x = mid_x - dpix;
            max.x = mid_x + dpix;

            coverage = bw::min(dx / dpix, 1.f);

            anti_aliasing = false;
        }

        if(dy < dpix){
            float mid_y = 0.5f * (max.y + min.y);

            min.y = mid_y - dpix;
            max.y = mid_y + dpix;

            coverage = coverage * bw::min(dy / dpix, 1.f);

            anti_aliasing = false;
        }

        rgba.a = coverage * rgba.a;
    }

    // --

    u32 nvertices = 4u;
    u32 nindices = 6u;

    if(anti_aliasing){
        min.x = min.x + size_aa;
        min.y = min.y + size_aa;
        max.x = max.x - size_aa;
        max.y = max.y - size_aa;

        nvertices += 8u;
        nindices += 36u;
    }

    // --

    u32 batch_index = get_batch_index_xyzrgba(nvertices, nindices, batch_count, batch_storage, renderer);
    Batch_Info& batch = batch_storage[batch_index];

    vertex_xyzrgba* vptr = (vertex_xyzrgba*)batch.buffer.vptr + batch.vcursor;
    u16* iptr = (u16*)batch.buffer.iptr + batch.icursor;
    u32 base_index = batch.vcursor;

    batch.vcursor += nvertices;
    batch.icursor += nindices;

    // --

    // NOTE(hugo):
    //
    //      9  8
    //      |  |
    //  10--3--2--7
    //      |  |
    //  11--0--1--6
    //      |  |
    //      4  5
    //

    {
        // NOTE(hugo): body
        *vptr++ = {{min.x, min.y, depth}, rgba};
        *vptr++ = {{max.x, min.y, depth}, rgba};
        *vptr++ = {{max.x, max.y, depth}, rgba};
        *vptr++ = {{min.x, max.y, depth}, rgba};
    }
    {
        // NOTE(hugo): body
        *iptr++ = base_index + 0u;
        *iptr++ = base_index + 1u;
        *iptr++ = base_index + 2u;
        *iptr++ = base_index + 0u;
        *iptr++ = base_index + 2u;
        *iptr++ = base_index + 3u;
    }

    if(anti_aliasing){
        {
            // NOTE(hugo): aa south
            *vptr++ = {{min.x, min.y - offset_aa, depth}, rgba_aa};
            *vptr++ = {{max.x, min.y - offset_aa, depth}, rgba_aa};

            // NOTE(hugo): aa east
            *vptr++ = {{max.x + offset_aa, min.y, depth}, rgba_aa};
            *vptr++ = {{max.x + offset_aa, max.y, depth}, rgba_aa};

            // NOTE(hugo): aa north
            *vptr++ = {{max.x, max.y + offset_aa, depth}, rgba_aa};
            *vptr++ = {{min.x, max.y + offset_aa, depth}, rgba_aa};

            // NOTE(hugo): aa west
            *vptr++ = {{min.x - offset_aa, max.y, depth}, rgba_aa};
            *vptr++ = {{min.x - offset_aa, min.y, depth}, rgba_aa};
        }
        {
            // NOTE(hugo): aa south + south to east
            *iptr++ = base_index + 4u;
            *iptr++ = base_index + 5u;
            *iptr++ = base_index + 1u;
            *iptr++ = base_index + 4u;
            *iptr++ = base_index + 1u;
            *iptr++ = base_index + 0u;

            *iptr++ = base_index + 5u;
            *iptr++ = base_index + 6u;
            *iptr++ = base_index + 1u;

            // NOTE(hugo): aa east + east to north
            *iptr++ = base_index + 6u;
            *iptr++ = base_index + 7u;
            *iptr++ = base_index + 2u;
            *iptr++ = base_index + 6u;
            *iptr++ = base_index + 2u;
            *iptr++ = base_index + 1u;

            *iptr++ = base_index + 7u;
            *iptr++ = base_index + 8u;
            *iptr++ = base_index + 2u;

            // NOTE(hugo): aa north + north to west
            *iptr++ = base_index + 8u;
            *iptr++ = base_index + 9u;
            *iptr++ = base_index + 3u;
            *iptr++ = base_index + 8u;
            *iptr++ = base_index + 3u;
            *iptr++ = base_index + 2u;

            *iptr++ = base_index + 9u;
            *iptr++ = base_index + 10u;
            *iptr++ = base_index + 3u;

            // NOTE(hugo): aa west + west to south
            *iptr++ = base_index + 10u;
            *iptr++ = base_index + 11u;
            *iptr++ = base_index + 0u;
            *iptr++ = base_index + 10u;
            *iptr++ = base_index + 0u;
            *iptr++ = base_index + 3u;

            *iptr++ = base_index + 11u;
            *iptr++ = base_index + 4u;
            *iptr++ = base_index + 0u;
        }
    }
}

void Vector_Graphics_Renderer::segment(vec2 A, vec2 B, float radius, float depth, vec4 rgba, float dpix, bool anti_aliasing){
    float size_aa = 0.45f * dpix;
    float offset_aa = 0.90f * dpix;
    vec4 rgba_aa = {rgba.r, rgba.g, rgba.a, 0.f};

    // NOTE(hugo): coverage based anti-aliasing for thin lines
    {
        float diameter_to_dpix = 2.f * radius / dpix;
        if(diameter_to_dpix < 1.f){
            radius = dpix;
            anti_aliasing = false;
            rgba.a = 0.5f * diameter_to_dpix * rgba.a;
        }
    }

    vec2 uAB = normalized(B - A);
    vec2 ortho = {uAB.y, - uAB.x};

    // --

    u32 nvertices = 4u;
    u32 nindices = 6u;

    if(anti_aliasing){
        A = A + size_aa * uAB;
        B = B - size_aa * uAB;
        radius = radius - size_aa;

        nvertices += 8u;
        nindices += 36u;
    }

    // --

    u32 batch_index = get_batch_index_xyzrgba(nvertices, nindices, batch_count, batch_storage, renderer);
    Batch_Info& batch = batch_storage[batch_index];

    vertex_xyzrgba* vptr = (vertex_xyzrgba*)batch.buffer.vptr + batch.vcursor;
    u16* iptr = (u16*)batch.buffer.iptr + batch.icursor;
    u32 base_index = batch.vcursor;

    batch.vcursor += nvertices;
    batch.icursor += nindices;

    // --

    vec2 dwidth = ortho * radius;

    vec2 v0 = A - dwidth;
    vec2 v1 = A + dwidth;
    vec2 v2 = B + dwidth;
    vec2 v3 = B - dwidth;

    // NOTE(hugo):
    //
    //      9  8
    //      |  |
    //  10--3--2--7
    //      |  |
    //  11--0--1--6
    //      |  |
    //      4  5
    //
    // AL = 0
    // AR = 1
    // BL = 3
    // BR = 2

    {
        // NOTE(hugo): body
        *vptr++ = {{v0.x, v0.y, depth}, rgba};
        *vptr++ = {{v1.x, v1.y, depth}, rgba};
        *vptr++ = {{v2.x, v2.y, depth}, rgba};
        *vptr++ = {{v3.x, v3.y, depth}, rgba};
    }
    {
        // NOTE(hugo): body
        *iptr++ = base_index + 0u;
        *iptr++ = base_index + 1u;
        *iptr++ = base_index + 2u;
        *iptr++ = base_index + 0u;
        *iptr++ = base_index + 2u;
        *iptr++ = base_index + 3u;
    }

    if(anti_aliasing){
        vec2 dheight_aa = uAB * offset_aa;
        vec2 dwidth_aa = ortho * offset_aa;

        vec2 v4 = v0 - dheight_aa;
        vec2 v5 = v1 - dheight_aa;
        vec2 v6 = v1 + dwidth_aa;
        vec2 v7 = v2 + dwidth_aa;
        vec2 v8 = v2 + dheight_aa;
        vec2 v9 = v3 + dheight_aa;
        vec2 v10 = v3 - dwidth_aa;
        vec2 v11 = v4 - dwidth_aa;

        {
            // NOTE(hugo): aa south
            *vptr++ = {{v4.x, v4.y, depth}, rgba_aa};
            *vptr++ = {{v5.x, v5.y, depth}, rgba_aa};

            // NOTE(hugo): aa east
            *vptr++ = {{v6.x, v6.y, depth}, rgba_aa};
            *vptr++ = {{v7.x, v7.y, depth}, rgba_aa};

            // NOTE(hugo): aa north
            *vptr++ = {{v8.x, v8.y, depth}, rgba_aa};
            *vptr++ = {{v9.x, v9.y, depth}, rgba_aa};

            // NOTE(hugo): aa west
            *vptr++ = {{v10.x, v10.y, depth}, rgba_aa};
            *vptr++ = {{v11.x, v11.y, depth}, rgba_aa};
        }
        {
            // NOTE(hugo): aa south + south to east
            *iptr++ = base_index + 4u;
            *iptr++ = base_index + 5u;
            *iptr++ = base_index + 1u;
            *iptr++ = base_index + 4u;
            *iptr++ = base_index + 1u;
            *iptr++ = base_index + 0u;

            *iptr++ = base_index + 5u;
            *iptr++ = base_index + 6u;
            *iptr++ = base_index + 1u;

            // NOTE(hugo): aa east + east to north
            *iptr++ = base_index + 6u;
            *iptr++ = base_index + 7u;
            *iptr++ = base_index + 2u;
            *iptr++ = base_index + 6u;
            *iptr++ = base_index + 2u;
            *iptr++ = base_index + 1u;

            *iptr++ = base_index + 7u;
            *iptr++ = base_index + 8u;
            *iptr++ = base_index + 2u;

            // NOTE(hugo): aa north + north to west
            *iptr++ = base_index + 8u;
            *iptr++ = base_index + 9u;
            *iptr++ = base_index + 3u;
            *iptr++ = base_index + 8u;
            *iptr++ = base_index + 3u;
            *iptr++ = base_index + 2u;

            *iptr++ = base_index + 9u;
            *iptr++ = base_index + 10u;
            *iptr++ = base_index + 3u;

            // NOTE(hugo): aa west + west to south
            *iptr++ = base_index + 10u;
            *iptr++ = base_index + 11u;
            *iptr++ = base_index + 0u;
            *iptr++ = base_index + 10u;
            *iptr++ = base_index + 0u;
            *iptr++ = base_index + 3u;

            *iptr++ = base_index + 11u;
            *iptr++ = base_index + 4u;
            *iptr++ = base_index + 0u;
        }
    }
}

void Vector_Graphics_Renderer::disc(vec2 center, float radius, float depth, vec4 rgba, float dpix, bool anti_aliasing){
    float size_aa = 0.45f * dpix;
    float offset_aa = 0.90f * dpix;
    vec4 rgba_aa = {rgba.r, rgba.g, rgba.b, 0.f};

    // NOTE(hugo): coverage based anti-aliasing for small disc
    {
        float diameter_to_dpix = 2.f * radius / dpix;
        if(diameter_to_dpix < 1.f){
            anti_aliasing = false;
            rgba.a = diameter_to_dpix * rgba.a;
        }
    }

    // --

    u32 nvertices_perimeter = circle_vertices(radius, dpix * 0.33f);
    assert(nvertices_perimeter > 2u);

    u32 nvertices = nvertices_perimeter + 1u;
    u32 nindices = 3u * nvertices_perimeter;

    if(anti_aliasing){
        radius = radius - size_aa;

        nvertices += 2u * nvertices_perimeter;
        nindices += nvertices_perimeter * 9u;
    }

    // --

    u32 batch_index = get_batch_index_xyzrgba(nvertices, nindices, batch_count, batch_storage, renderer);
    Batch_Info& batch = batch_storage[batch_index];

    vertex_xyzrgba* vptr = (vertex_xyzrgba*)batch.buffer.vptr + batch.vcursor;
    u16* iptr = (u16*)batch.buffer.iptr + batch.icursor;
    u32 base_index = batch.vcursor;

    batch.vcursor += nvertices;
    batch.icursor += nindices;

    // --

    u32 index_center = base_index;
    u32 index_start = base_index + 1u;
    u32 index_start_aa = base_index + nvertices_perimeter + 1u;

    {
        vertex_xyzrgba* vptr_aa = vptr + nvertices_perimeter + 1u;

        // NOTE(hugo): center & starting vertex
        *vptr++ = {{center.x, center.y, depth}, rgba};
        *vptr++ = {{center.x + radius, center.y, depth}, rgba};

        // NOTE(hugo): perimeter vertices
        vec2 base_rot = {radius, 0.f};
        vec2 prev_rot = base_rot;
        for(u32 ivert = 1u; ivert != nvertices_perimeter; ++ivert){
            float rad = 2.f * PI * (float)(ivert) / (float)(nvertices_perimeter);

            vec2 new_rot = rotated(base_rot, rad);
            *vptr++ = {{center.x + new_rot.x, center.y + new_rot.y, depth}, rgba};

            if(anti_aliasing){
                // TODO(hugo): aa perimeter vertices
                vec2 ortho_aa = normalized(vec2({new_rot.y - prev_rot.y, prev_rot.x - new_rot.x})) * offset_aa;
                *vptr_aa++ = {{center.x + prev_rot.x + ortho_aa.x, center.y + prev_rot.y + ortho_aa.y}, rgba_aa};
                *vptr_aa++ = {{center.x + new_rot.x + ortho_aa.x, center.y + new_rot.y + ortho_aa.y}, rgba_aa};
            }

            prev_rot = new_rot;
        }

        if(anti_aliasing){
            // NOTE(hugo): aa end sector vertices
            vec2 ortho_aa = normalized(vec2({base_rot.y - prev_rot.y, prev_rot.x - base_rot.x})) * offset_aa;
            *vptr_aa++ = {{center.x + prev_rot.x + ortho_aa.x, center.y + prev_rot.y + ortho_aa.y}, rgba_aa};
            *vptr_aa++ = {{center.x + base_rot.x + ortho_aa.x, center.y + base_rot.y + ortho_aa.y}, rgba_aa};
        }
    }
    {
        // NOTE(hugo): sectors
        u32 index_current = index_start;
        for(u32 isector = 0u; isector != nvertices_perimeter - 1u; ++isector){
            *iptr++ = index_center;
            *iptr++ = index_current++;
            *iptr++ = index_current;
        }

        // NOTE(hugo): end sector
        *iptr++ = index_center;
        *iptr++ = index_current;
        *iptr++ = index_start;

        if(anti_aliasing){
            // NOTE(hugo): aa sectors + to next sector
            index_current = index_start;
            u32 index_aa = index_start_aa;
            for(u32 isector = 0u; isector != nvertices_perimeter - 1u; ++isector){
                *iptr++ = index_current++;
                *iptr++ = index_aa;
                *iptr++ = index_current;
                *iptr++ = index_aa++;
                *iptr++ = index_aa;
                *iptr++ = index_current;

                *iptr++ = index_current;
                *iptr++ = index_aa++;
                *iptr++ = index_aa;
            }

            // NOTE(hugo): aa end sector
            *iptr++ = index_current;
            *iptr++ = index_aa;
            *iptr++ = index_start;
            *iptr++ = index_aa++;
            *iptr++ = index_aa;
            *iptr++ = index_start;

            *iptr++ = index_start;
            *iptr++ = index_aa;
            *iptr++ = index_start_aa;
        }
    }
}

void Vector_Graphics_Renderer::rect_round(vec2 min, vec2 max, float depth, vec4 rgba, float dpix, bool anti_aliasing){
}

void Vector_Graphics_Renderer::segment_round(vec2 A, vec2 B, float radius, float depth, vec4 rgba, float dpix, bool anti_aliasing){
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
