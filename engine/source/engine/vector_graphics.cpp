void Vector_Graphics_Renderer::terminate(){
    for(auto& info : storage) renderer->free_transient_buffer(info.buffer);
    storage.free();
    for(auto& info : storage_antialiasing) renderer->free_transient_buffer(info.buffer);
    storage_antialiasing.free();
}

// NOTE(hugo):
// * search in used storage
// * search in free storage
// |* checkout the buffer when not already in use
// |* make reused storage the last active batch
// * reserve new storage
// * push new storage
// * make new storage the last active batch

#define GET_BATCH_INDEX_CONTENT(FORMAT)                                                                     \
    for(u32 ibatch = 0u; ibatch != active; ++ibatch){                                                       \
        u32 vcapacity = storage[ibatch].buffer.vbytesize / sizeof(vertex_ ## FORMAT);                       \
        u32 icapacity = storage[ibatch].buffer.ibytesize / sizeof(u16);                                     \
                                                                                                            \
        if(vcapacity - storage[ibatch].vcursor >= nvertices                                                 \
        && icapacity - storage[ibatch].icursor >= nindices){                                                \
            return ibatch;                                                                                  \
        }                                                                                                   \
    }                                                                                                       \
                                                                                                            \
    for(u32 ibatch = active; ibatch != storage.size; ++ibatch){                                             \
        u32 vcapacity = storage[ibatch].buffer.vbytesize / sizeof(vertex_ ## FORMAT);                       \
        u32 icapacity = storage[ibatch].buffer.ibytesize / sizeof(u16);                                     \
                                                                                                            \
        if(vcapacity >= nvertices                                                                           \
        && icapacity >= nindices){                                                                          \
                                                                                                            \
            renderer->checkout(storage[ibatch].buffer);                                                     \
            storage[ibatch].vcursor = 0u;                                                                   \
            storage[ibatch].icursor = 0u;                                                                   \
                                                                                                            \
            if(ibatch != active) swap(storage[active], storage[ibatch]);                                    \
            return active++;                                                                                \
        }                                                                                                   \
    }                                                                                                       \
                                                                                                            \
    constexpr size_t default_vbytesize = vector_graphics_vertex_capacity * sizeof(vertex_ ## FORMAT);       \
    constexpr size_t default_ibytesize = vector_graphics_index_capacity * sizeof(u16);                      \
                                                                                                            \
    size_t round_nvertices = round_up_pow2(nvertices);                                                      \
    size_t round_nindices = round_up_pow2(nindices);                                                        \
    size_t vbytesize = round_nvertices * sizeof(vertex_ ## FORMAT);                                         \
    size_t ibytesize = round_nindices * sizeof(u16);                                                        \
    vbytesize = max(vbytesize, default_vbytesize);                                                          \
    ibytesize = max(ibytesize, default_ibytesize);                                                          \
                                                                                                            \
    Transient_Buffer_Indexed buffer = renderer->get_transient_buffer_indexed(vbytesize, ibytesize);         \
    renderer->format(buffer, FORMAT);                                                                       \
    renderer->checkout(buffer);                                                                             \
                                                                                                            \
    u32 push_index = storage.size;                                                                          \
    storage.push({0u, 0u, buffer});                                                                         \
                                                                                                            \
    if(push_index != active) swap(storage[active], storage[push_index]);                                    \
    return active++;

static u32 get_batch_index(u32 nvertices, u32 nindices, Vector_Graphics_Renderer* vg_renderer){
    u32& active = vg_renderer->active;
    array<Vector_Graphics_Renderer::Batch_Info>& storage = vg_renderer->storage;
    Renderer* renderer = vg_renderer->renderer;

    GET_BATCH_INDEX_CONTENT(xyzrgba);
}

static u32 get_batch_index_antialiasing(u32 nvertices, u32 nindices, Vector_Graphics_Renderer* vg_renderer){
    u32& active = vg_renderer->active_antialiasing;
    array<Vector_Graphics_Renderer::Batch_Info>& storage = vg_renderer->storage_antialiasing;
    Renderer* renderer = vg_renderer->renderer;

    GET_BATCH_INDEX_CONTENT(xyzrgba_norm);
}

void Vector_Graphics_Renderer::simple_polygon(u32 nvertices, vec2* vertices, u32* indices, u32 depth, u32 rgba, bool antialiasing){
    {
        u32 nindices = 3u * (nvertices - 2u);

        u32 batch_index = get_batch_index(nvertices, nindices, this);
        Batch_Info& batch = storage[batch_index];

        vertex_xyzrgba* vptr = (vertex_xyzrgba*)batch.buffer.vptr + batch.vcursor;
        u16* iptr = (u16*)batch.buffer.iptr + batch.icursor;
        assert(vptr && iptr);
        u32 base_index = batch.vcursor;

        batch.vcursor += nvertices;
        batch.icursor += nindices;

        // --

        for(u32 ivert = 0u; ivert != nvertices; ++ivert){
            vptr[ivert] = {{vertices[ivert].x, vertices[ivert].y}, depth, rgba};
        }
        for(u32 iind = 0u; iind != nindices; ++iind){
            iptr[iind] = base_index + indices[iind];
        }
    }

    if(antialiasing){
        u32 rgba_antialiasing = rgba32_a(rgba, 0.f);

        // NOTE(hugo):
        // nvertices_antialiasing: 1 copy of each vertex + 2 antialiasing vertices per vertex : one for each segment orientation
        // nindices_antialiasing: 6 for the extension of each segment + 3 for the joint to the next segment
        u32 nvertices_antialiasing = 3u * nvertices;
        u32 nindices_antialiasing = 9u * nvertices;

        u32 batch_index = get_batch_index_antialiasing(nvertices_antialiasing, nindices_antialiasing, this);
        Batch_Info& batch = storage_antialiasing[batch_index];

        vertex_xyzrgba_norm* vptr = (vertex_xyzrgba_norm*)batch.buffer.vptr + batch.vcursor;
        u16* iptr = (u16*)batch.buffer.iptr + batch.icursor;
        assert(vptr && iptr);
        u32 base_index = batch.vcursor;

        batch.vcursor += nvertices_antialiasing;
        batch.icursor += nindices_antialiasing;

        // --

        for(u32 ivert = 0u; ivert != nvertices - 1u; ++ivert){
            const vec2& vert = vertices[ivert];
            const vec2& nvert = vertices[ivert + 1u];
            vec2 dir = nvert - vert;
            vec2 ortho = normalized(vec2({dir.y, - dir.x}));

            *vptr++ = {{vert.x, vert.y}, depth, rgba, {0.f, 0.f}};
            *vptr++ = {{vert.x, vert.y}, depth, rgba_antialiasing, ortho};
            *vptr++ = {{nvert.x, nvert.y}, depth, rgba_antialiasing, ortho};

            // NOTE(hugo): extension's outer triangle (vert, vert's extension, nvert's extension)
            *iptr++ = base_index + ivert * 3u;
            *iptr++ = base_index + ivert * 3u + 1u;
            *iptr++ = base_index + ivert * 3u + 2u;

            // NOTE(hugo): extension's inner triangle (vert, nvert's extension, nvert)
            *iptr++ = base_index + ivert * 3u;
            *iptr++ = base_index + ivert * 3u + 2u;
            *iptr++ = base_index + ivert * 3u + 3u;

            // NOTE(hugo): extension's joint (nvert, nvert's extension, nvert's extension on the next segment)
            *iptr++ = base_index + ivert * 3u + 3u;
            *iptr++ = base_index + ivert * 3u + 2u;
            *iptr++ = base_index + ivert * 3u + 4u;
        }

        u32 ivert = nvertices - 1u;

        const vec2& vert = vertices[ivert];
        const vec2& nvert = vertices[0u];
        vec2 dir = nvert - vert;
        vec2 ortho = normalized(vec2({dir.y, - dir.x}));

        *vptr++ = {{vert.x, vert.y}, depth, rgba, {0.f, 0.f}};
        *vptr++ = {{vert.x, vert.y}, depth, rgba_antialiasing, ortho};
        *vptr++ = {{nvert.x, nvert.y}, depth, rgba_antialiasing, ortho};

        // NOTE(hugo): extension's outer triangle (vert, vert's extension, nvert's extension)
        *iptr++ = base_index + ivert * 3u;
        *iptr++ = base_index + ivert * 3u + 1u;
        *iptr++ = base_index + ivert * 3u + 2u;

        // NOTE(hugo): extension's inner triangle (vert, nvert's extension, nvert)
        *iptr++ = base_index + ivert * 3u;
        *iptr++ = base_index + ivert * 3u + 2u;
        *iptr++ = base_index + 0u;

        // NOTE(hugo): extension's joint (nvert, nvert's extension, nvert's extension on the next segment)
        *iptr++ = base_index + 0u;
        *iptr++ = base_index + ivert * 3u + 2u;
        *iptr++ = base_index + 1u;
    }
}

void Vector_Graphics_Renderer::draw(){
    for(u32 ibatch = 0u; ibatch != active; ++ibatch){
        Batch_Info& batch = storage[ibatch];

        renderer->commit(batch.buffer);
        renderer->draw(batch.buffer, PRIMITIVE_TRIANGLES, TYPE_USHORT, batch.icursor, 0u);
    }
}

void Vector_Graphics_Renderer::draw_antialiasing(){
    for(u32 ibatch = 0u; ibatch != active_antialiasing; ++ibatch){
        Batch_Info& batch = storage_antialiasing[ibatch];

        renderer->commit(batch.buffer);
        renderer->draw(batch.buffer, PRIMITIVE_TRIANGLES, TYPE_USHORT, batch.icursor, 0u);
    }
}

void Vector_Graphics_Renderer::new_frame(){
    active = 0u;
    active_antialiasing = 0u;
}
