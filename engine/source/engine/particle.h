#ifndef H_PARTICLE
#define H_PARTICLE

struct Particle_Emitter{

    enum Shape{
        Disc,
        Rect
    };
    union Shape_Descriptor{
        struct{
            vec2 center;
            float radius;
        } circle;
        struct{
            vec2 min;
            vec2 max;
        } rect;
    };

    void terminate();

    void update();
    void burst(u32 nparticles);
    Vertex_Batch_ID batch_as_quad_xyuv(Renderer* renderer);

    // ---- internal

    struct Particle{
        vec2 position;
        vec2 velocity;
        float angle;
        float angular_velocity;
        float size;
        u32 frame_counter;
    };

    void spawn_process(Particle& p);
    void update_process(Particle& p);

    // ---- data

    Shape shape;
    Shape_Descriptor desc = {};

    // NOTE(hugo): duration_min_range[1] must not be zero
    u32 duration_min_range[2u];
    float theta_min_range[2u];
    float velocity_min_range[2u];
    float angular_velocity_min_range[2u];
    float size_min_range[2u];

    u32 frame_counter = 0u;
    s32 particles_per_frame = 0u;

    diterpool<Particle> particles;
};

#endif
