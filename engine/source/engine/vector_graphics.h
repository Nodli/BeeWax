#ifndef H_VECTOR_GRAPHICS
#define H_VECTOR_GRAPHICS

constexpr size_t vector_graphics_vertex_capacity = 128u;
constexpr size_t vector_graphics_index_capacity = 4u * vector_graphics_vertex_capacity;

// NOTE(hugo): anti-aliasing based on alpha coverage ie draw_antialiasing must be in a transparent render pass after opaque geometry
// * with alpha blending
// * with LEQUAL depth test
// * without depth buffer output
// -- SHADER_NAME:
// * draw() with polygon_2D
// * draw_antialiasing() with polygon_2D_norm

struct Vector_Graphics_Renderer{
    void terminate();

    void simple_polygon(u32 nvertices, vec2* vertices, u32* indices, u32 depth, u32 rgba, bool antialiasing = true);
    void disc(float radius, u32 depth, u32 rgba, bool antialiasing = true);

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
