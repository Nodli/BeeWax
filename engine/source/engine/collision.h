#ifndef H_COLLISION
#define H_COLLISION

// REF(hugo): https://www.youtube.com/watch?v=-_IspRG548E
// REF(hugo): https://www.youtube.com/watch?v=MDusDn8oTSE

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

    // ---- boolean collision

    inline bool point_circle(const vec2& point, const Circle& circle);
    inline bool point_rect(const vec2& point, const Rect& rect);
    inline bool point_capsule(const vec2& point, const Capsule& capsule);
    inline bool point_ray(const vec2& point, const Ray& ray);

    inline bool circle_circle(const Circle& circleA, const Circle& circleB);
    inline bool circle_rect(const Circle& circle, const Rect& rect);
    inline bool circle_capsule(const Circle& circle, const Capsule& capsule);
    inline bool circle_ray(const Circle& circle, const Ray& ray);

    inline bool rect_rect(const Rect& rectA, const Rect& rectB);
    inline bool rect_capsule(const Rect& rect, const Capsule& capsule);

    // NOTE(hugo): inv_direction.x should not be zero
    inline bool rect_ray_axisX(const Rect& rect, Ray& ray);
    // NOTE(hugo): inv_direction.y should not be zero
    inline bool rect_ray_axisY(const Rect& rect, Ray& ray);
    // NOTE(hugo): inv_direction.x and inv_direction.y should not be zero
    inline bool rect_ray_axisXY(const Rect& rect, Ray& ray);

    // ---- detailed collision

    // NOTE(hugo):
    // - normals are from shapeA to shapeB
    // - contacts represent the point / plane of the collision (on the shapeB)

    inline void circle_circle_manifold(const Circle& circleA, const Circle& circleB, Manifold& manifold);
}

namespace c3D{
    struct Line{
        vec3 begin;
        vec3 end;
    };

    struct Sphere{
        vec3 center;
        float radius;
    };

    struct Cube{
        vec3 center() const;
        vec3 size() const;

        vec3 min;
        vec3 max;
    };

    struct Capsule{
        vec2 begin;
        vec2 end;
        float radius;
    };

    struct Ray{
        vec3 origin;
        vec3 direction;
        vec3 inv_direction;
        float length;
    };
}

#endif
