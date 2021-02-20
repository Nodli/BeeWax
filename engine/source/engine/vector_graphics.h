#ifndef H_VECTOR_GRAPHICS
#define H_VECTOR_GRAPHICS

// REF(hugo):
// https://www.researchgate.net/publication/347987699_A_Fast_Parametric_Ellipse_Algorithm

constexpr size_t vector_graphics_vertex_capacity = 128u;
constexpr size_t vector_graphics_index_capacity_multi = 6u;

float next_depth(float depth);

// NOTE(hugo): The anti-aliasing of segment_round (and convex poly. in general) is wrong but good enough for now.
// The reason is because we are using a polygonal approximation of a circle for the caps. Pushing vertices away from
// the vertices for anti-aliasing means that the anti-aliasing is *stronger* in the direction of the vertices.
// When doing this we are anti-aliasing a *true* circle and not the polygonal appoximation.
// The right way to do this would probably be to use a per-polygon-side approach and push away to anti-aliasing
// in the direction of the normal to the polygonal segment.
// This is why we can see spikes with more intense color in the anti-aliasing along the vertices.

struct Vector_Graphics_Renderer{
    void terminate();

    void segment_round(vec2 A, vec2 B, float radius, float depth, vec4 rgba, float dpix);
    void circle(vec2 center, float radius, float depth, vec4 rgba, float dpix);

    void draw();
    void next_frame();

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
