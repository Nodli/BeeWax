#ifndef H_VECTOR_GRAPHICS
#define H_VECTOR_GRAPHICS

// REF(hugo):
// https://www.researchgate.net/publication/347987699_A_Fast_Parametric_Ellipse_Algorithm

constexpr size_t vector_graphics_vertex_capacity = 128u;
constexpr size_t vector_graphics_index_capacity = 4u * vector_graphics_vertex_capacity;

float next_depth(float depth);

// NOTE(hugo): anti-aliasing uses fragment blending ie depth tests should fail

struct Vector_Graphics_Renderer{
    void terminate();

    void rect(vec2 min, vec2 max, float depth, vec4 rgba, float dpix, bool anti_aliasing = true);
    void segment(vec2 A, vec2 B, float radius, float depth, vec4 rgba, float dpix, bool anti_aliasing = true);
    void disc(vec2 center, float radius, float depth, vec4 rgba, float dpix, bool anti_aliasing = true);
    void disc_sector(vec2 center, float rad_min, float rad_max, float depth, vec4 rgba, float dpix, bool anti_aliasing = true);

    void rect_round(vec2 min, vec2 max, float depth, vec4 rgba, float dpix, bool anti_aliasing = true);
    void segment_round(vec2 A, vec2 B, float radius, float depth, vec4 rgba, float dpix, bool anti_aliasing = true);

    void draw();
    void new_frame();

    // ---- data

    struct Batch_Info{
        size_t vcursor = 0u;
        size_t icursor = 0u;
        Transient_Buffer_Indexed buffer = {};
    };
    u32 batch_count = 0u;
    array<Batch_Info> batch_storage = {};

    Renderer* renderer = nullptr;
};

#endif
