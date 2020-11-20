#ifndef SHAPE_2D_H
#define SHAPE_2D_H

namespace c2D{
    struct Line{
        vec2 begin;
        vec2 end;
    };

    struct Circle{
        vec2 center;
        float radius;
    };

    struct Rect{
        vec2 center() const;
        vec2 size() const;

        vec2 min;
        vec2 max;
    };
    Rect moved(const Rect& rect, const vec2 direction);
    Rect scaled(const Rect& rect, const vec2 scale);
    Rect extended(const Rect& rect, const vec2 extension);

    struct Capsule{
        vec2 begin;
        vec2 end;
        float radius;
    };

    // NOTE(hugo): inv_direction should be computed before doing collision detection
    struct Ray{
        vec2 origin;
        vec2 direction;
        vec2 inv_direction;
        float length;
    };

    struct Manifold{
        u32 count;
        float depth[2];
        vec2 contact[2];
        vec2 normal[2];
    };
}

#endif
