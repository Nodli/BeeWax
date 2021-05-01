#ifndef H_VECTOR_GRAPHICS
#define H_VECTOR_GRAPHICS

constexpr size_t vector_graphics_vertex_capacity = 2048u;
constexpr size_t vector_graphics_index_capacity = 4u * vector_graphics_vertex_capacity;

// NOTE(hugo): anti-aliasing based on alpha coverage ie draw_antialiasing must be in a transparent render pass after opaque geometry
// * with alpha blending
// * with LEQUAL depth test
// * without depth buffer output
// -- SHADER_NAME:
// * draw() with polygon_2D
// * draw_antialiasing() with polygon_2D_norm

struct Vector_Cache{
    vertex_xyzrgba* vertices() const;
    u16* indices() const;
    vertex_xyzrgba_norm* vertices_antialiasing() const;
    u16* indices_antialiasing() const;

    void allocate();
    void free();

    // ---- data

    u8* memory = nullptr;
    u32 nvertices = 0u;
    u32 nindices = 0u;
    u32 nvertices_antialiasing = 0u;
    u32 nindices_antialiasing = 0u;
};

struct Vector_Batcher{
    void terminate();

    // NOTE(hugo): explicit free required
    Vector_Cache cache_polygon(u32 nvertices, vec2* vertices, u32* indices, u32 depth, u32 rgba);
    //Vector_Cache cache_polyline(u32 nvertices, vec2* vertices, float width, u32 depth, u32 rgba);

    void batch_polygon(u32 nvertices, vec2* vertices, u32* indices, u32 depth, u32 rgba);
    void batch_polyline(u32 nvertices, vec2* vertices, float width, u32 depth, u32 rgba);
    void batch_cache(const Vector_Cache& cache);

    void draw();
    void draw_antialiasing();

    void new_frame();

    // ---- data

    struct Batch_Info{
        size_t vcursor = 0u;
        size_t icursor = 0u;
        Transient_Buffer_Indexed buffer = {};
    };

    u32 active = 0u;
    array<Batch_Info> storage = {};

    u32 active_antialiasing = 0u;
    array<Batch_Info> storage_antialiasing = {};

    Renderer* renderer = nullptr;
};

#endif

vertex_xyzrgba* Vector_Cache::vertices() const{
    return (vertex_xyzrgba*)memory;
}
u16* Vector_Cache::indices() const{
    return (u16*)(memory + nvertices * sizeof(vertex_xyzrgba));
}
vertex_xyzrgba_norm* Vector_Cache::vertices_antialiasing() const{
    u32 indices_padding = nindices % 2u;
    return (vertex_xyzrgba_norm*)(memory + nvertices * sizeof(vertex_xyzrgba) + (nindices + indices_padding) * sizeof(u16));
}
u16* Vector_Cache::indices_antialiasing() const{
    u32 indices_padding = nindices % 2u;
    return (u16*)(memory + nvertices * sizeof(vertex_xyzrgba) + (nindices + indices_padding) * sizeof(u16) + nvertices_antialiasing * sizeof(vertex_xyzrgba_norm));
}

void Vector_Cache::allocate(){
    static_assert(alignof(vertex_xyzrgba) >= alignof(u16) && alignof(vertex_xyzrgba_norm) >= alignof(u16));
    u32 indices_padding = nindices % 2u;
    size_t bytesize = nvertices * sizeof(vertex_xyzrgba) + nvertices_antialiasing * sizeof(vertex_xyzrgba_norm) + (nindices + indices_padding + nindices_antialiasing) * sizeof(u32);
    memory = (u8*)malloc(bytesize);
}
void Vector_Cache::free(){
    ::free(memory);
}

void Vector_Batcher::terminate(){
    for(auto& info : storage) renderer->free_transient_buffer(info.buffer);
    storage.free();
    for(auto& info : storage_antialiasing) renderer->free_transient_buffer(info.buffer);
    storage_antialiasing.free();
}

static void info_polygon(u32 nvertices, u32& nindices){
    nindices = 3u * (nvertices - 2u);
}

static void info_antialiasing(u32 nvertices, u32& nvertices_antialiasing, u32& nindices_antialiasing){
    nvertices_antialiasing = 3u * nvertices;
    nindices_antialiasing = 9u * nvertices;
}

static void emit_polygon(u32 nvertices, vec2* vertices, u32 nindices, u32* indices, u32 depth, u32 rgba, vertex_xyzrgba* vptr, u16* iptr, u32 base_index){
    for(u32 ivert = 0u; ivert != nvertices; ++ivert){
        vptr[ivert] = {{vertices[ivert].x, vertices[ivert].y}, depth, rgba};
    }
    for(u32 iind = 0u; iind != nindices; ++iind){
        iptr[iind] = base_index + indices[iind];
    }
}

static void emit_antialiasing(u32 nvertices, vec2* vertices, u32 depth, u32 rgba, vertex_xyzrgba_norm* vptr, u16* iptr, u32 base_index){
    u32 rgba_antialiasing = rgba32_a(rgba, 0.f);

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

Vector_Cache Vector_Batcher::cache_polygon(u32 nvertices, vec2* vertices, u32* indices, u32 depth, u32 rgba){
    assert(nvertices > 2u);

    Vector_Cache cache;
    cache.nvertices = nvertices;
    cache.nindices = 3u * (nvertices - 2u);
    cache.nvertices_antialiasing = 3u * nvertices;
    cache.nindices_antialiasing = 9u * nvertices;
    cache.allocate();

    emit_polygon(nvertices, vertices, cache.nindices, indices, depth, rgba, cache.vertices(), cache.indices(), 0u);
    emit_antialiasing(nvertices, vertices, depth, rgba, cache.vertices_antialiasing(), cache.indices_antialiasing(), 0u);

    return cache;
}

void Vector_Batcher::batch_polyline(u32 nvertices, vec2* vertices, float width, u32 depth, u32 rgba){
#if 0
    assert(nvertices > 1u);

    {
        u32 nsegment = nvertices - 1u;
        u32 njoint = nvertices - 2u;

        u32 nvertices_mesh = 2u + 2u + 3u * njoint;
        u32 nindices_mesh = 4u * nsegment + 3u * njoint;

        // --

        vertex_xyzrgba* vptr = nullptr;
        u16* iptr = nullptr;
        u32 base_index = 0u;

        // --

        float hwidth = width * 0.5f;
        float qwidth = width * 0.25f;

        u32 previousR;
        u32 previousL;

        // NOTE(hugo):
        {
            vec2 dir = vertices[1u] - vertices[0u];
            vec2 ortho = normalized(vec2({dir.y, - dir.x}));

            *vptr++ = {{vertices[0u] + ortho * hwidth}, depth, rgba};
            *vptr++ = {{vertices[0u] - ortho * hwidth}, depth, rgba};

            previousR = base_index;
            previousL = base_index + 1u;
        }

        // NOTE(hugo):
        for(u32 ijoint = 0u; ijoint != njoint; ++ijoint){
            const vec2& pvert = vertices[ijoint];
            const vec2& vert = vertices[1u + ijoint];
            const vec2& nvert = vertices[2u + ijoint];

            vec2 dir = vert - pvert;
            vec2 ortho = normalized(vec2({dir.y, - dir.x}));

            vec2 ndir = nvert - vert;
            vec2 northo = normalized(vec2({ndir.y, - ndir.x}));

            // NOTE(hugo): turn right (internal, next_external, prev_external)
            if(dot(ortho, ndir) > 0.f){
                *vptr++ = {{vert + (ortho + northo) * qwidth},  depth, rgba};
                *vptr++ = {{vert - ortho * hwidth},             depth, rgba};
                *vptr++ = {{vert - northo * hwidth},            depth, rgba;

            // NOTE(hugo): turn left or aligned (internal, prev_external, next_external)
            }else{
                *vptr++ = {{vert - (ortho + northo) * qwidth},  depth, rgba};
                *vptr++ = {{vert + ortho * hwidth},             depth, rgba};
                *vptr++ = {{vert + northo * hwidth},            depth, rgba};
            }

            previousR = nextR;
            previousL = nextL;
        }

        // NOTE(hugo):
        {
            vec2 dir = vertices[nvertices - 1u] - vertices[nvertices - 2u];
            vec2 ortho = normalized(vec2({dir.y, - dir.x}));

            *vptr++ = {{vertices[nvertices - 1u] + ortho * hwidth}, depth, rgba};
            *vptr++ = {{vertices[nvertices - 1u] - ortho * hwidth}, depth, rgba};

            u32 nextR = base_index + 2u + 3u * njoint;
            u32 nextL = nextR + 1u;

            *iptr++ = previousR;
            *iptr++ = nextR;
            *iptr++ = previousL;

            *iptr++ = previousL;
            *iptr++ = nextR;
            *iptr++ = nextL;
        }
    }
#endif
}

// NOTE(hugo):
// * search in used storage
// * search in free storage
// |* checkout the buffer when not already in use
// |* make reused storage the last active batch
// * reserve new storage
// * push new storage
// * make new storage the last active batch

#define VECTOR_GRAPHICS_GET_BATCH_INDEX_CONTENT(FORMAT)                                                     \
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

static u32 get_batch_index(u32 nvertices, u32 nindices, Vector_Batcher* vrenderer){
    u32& active = vrenderer->active;
    array<Vector_Batcher::Batch_Info>& storage = vrenderer->storage;
    Renderer* renderer = vrenderer->renderer;

    VECTOR_GRAPHICS_GET_BATCH_INDEX_CONTENT(xyzrgba);
}

static u32 get_batch_index_antialiasing(u32 nvertices, u32 nindices, Vector_Batcher* vrenderer){
    u32& active = vrenderer->active_antialiasing;
    array<Vector_Batcher::Batch_Info>& storage = vrenderer->storage_antialiasing;
    Renderer* renderer = vrenderer->renderer;

    VECTOR_GRAPHICS_GET_BATCH_INDEX_CONTENT(xyzrgba_norm);
}

void Vector_Batcher::batch_polygon(u32 nvertices, vec2* vertices, u32* indices, u32 depth, u32 rgba){
    assert(nvertices > 2u);

    {
        u32 nindices = 3u * (nvertices - 2u);

        // --

        u32 batch_index = get_batch_index(nvertices, nindices, this);
        Batch_Info& batch = storage[batch_index];

        vertex_xyzrgba* vptr = (vertex_xyzrgba*)batch.buffer.vptr + batch.vcursor;
        u16* iptr = (u16*)batch.buffer.iptr + batch.icursor;
        assert(vptr && iptr);
        u32 base_index = batch.vcursor;

        batch.vcursor += nvertices;
        batch.icursor += nindices;

        // --

        emit_polygon(nvertices, vertices, nindices, indices, depth, rgba, vptr, iptr, base_index);
    }

    {

        // NOTE(hugo):
        // nvertices_antialiasing: 1 copy of each vertex + 2 antialiasing vertices per vertex : one for each segment orientation
        // nindices_antialiasing: 6 for the extension of each segment + 3 for the joint to the next segment
        u32 nvertices_antialiasing = 3u * nvertices;
        u32 nindices_antialiasing = 9u * nvertices;

        // --

        u32 batch_index = get_batch_index_antialiasing(nvertices_antialiasing, nindices_antialiasing, this);
        Batch_Info& batch = storage_antialiasing[batch_index];

        vertex_xyzrgba_norm* vptr = (vertex_xyzrgba_norm*)batch.buffer.vptr + batch.vcursor;
        u16* iptr = (u16*)batch.buffer.iptr + batch.icursor;
        assert(vptr && iptr);
        u32 base_index = batch.vcursor;

        batch.vcursor += nvertices_antialiasing;
        batch.icursor += nindices_antialiasing;

        // --

        emit_antialiasing(nvertices, vertices, depth, rgba, vptr, iptr, base_index);
    }
}

void Vector_Batcher::batch_cache(const Vector_Cache& cache){
    if(cache.nvertices && cache.nindices){
        u32 batch_index = get_batch_index(cache.nvertices, cache.nindices, this);
        Batch_Info& batch = storage[batch_index];

        vertex_xyzrgba* vptr = (vertex_xyzrgba*)batch.buffer.vptr + batch.vcursor;
        u16* iptr = (u16*)batch.buffer.iptr + batch.icursor;
        assert(vptr && iptr);
        u32 base_index = batch.vcursor;

        batch.vcursor += cache.nvertices;
        batch.icursor += cache.nindices;

        // --

        vertex_xyzrgba* cache_vptr = cache.vertices();
        for(u32 ivert = 0u; ivert != cache.nvertices; ++ivert){
            vptr[ivert] = cache_vptr[ivert];
        }

        u16* cache_iptr = cache.indices();
        for(u32 iind = 0u; iind != cache.nindices; ++iind){
            iptr[iind] = base_index + cache_iptr[iind];
        }
    }

    if(cache.nvertices_antialiasing && cache.nindices_antialiasing){
        u32 batch_index = get_batch_index_antialiasing(cache.nvertices_antialiasing, cache.nindices_antialiasing, this);
        Batch_Info& batch = storage_antialiasing[batch_index];

        vertex_xyzrgba_norm* vptr = (vertex_xyzrgba_norm*)batch.buffer.vptr + batch.vcursor;
        u16* iptr = (u16*)batch.buffer.iptr + batch.icursor;
        assert(vptr && iptr);
        u32 base_index = batch.vcursor;

        batch.vcursor += cache.nvertices_antialiasing;
        batch.icursor += cache.nindices_antialiasing;

        // --

        vertex_xyzrgba_norm* cache_vptr = cache.vertices_antialiasing();
        for(u32 ivert = 0u; ivert != cache.nvertices_antialiasing; ++ivert){
            vptr[ivert] = cache_vptr[ivert];
        }

        u16* cache_iptr = cache.indices_antialiasing();
        for(u32 iind = 0u; iind != cache.nindices_antialiasing; ++iind){
            iptr[iind] = base_index + cache_iptr[iind];
        }
    }
}

void Vector_Batcher::draw(){
    for(u32 ibatch = 0u; ibatch != active; ++ibatch){
        Batch_Info& batch = storage[ibatch];

        renderer->commit(batch.buffer);
        renderer->draw(batch.buffer, PRIMITIVE_TRIANGLES, TYPE_USHORT, batch.icursor, 0u);
    }
}

void Vector_Batcher::draw_antialiasing(){
    for(u32 ibatch = 0u; ibatch != active_antialiasing; ++ibatch){
        Batch_Info& batch = storage_antialiasing[ibatch];

        renderer->commit(batch.buffer);
        renderer->draw(batch.buffer, PRIMITIVE_TRIANGLES, TYPE_USHORT, batch.icursor, 0u);
    }
}

void Vector_Batcher::new_frame(){
    active = 0u;
    active_antialiasing = 0u;
}
