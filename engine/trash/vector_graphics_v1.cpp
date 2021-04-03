
#if 0
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
    assert(vptr && iptr);
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

#endif
