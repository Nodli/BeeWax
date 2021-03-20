#ifndef H_COLLISION
#define H_COLLISION

// REF(hugo): https://www.youtube.com/watch?v=-_IspRG548E
// REF(hugo): https://www.youtube.com/watch?v=MDusDn8oTSE

namespace cshape{

    // ---- 2D

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

    struct Polygon{
        u32 nvertices;
        vec2* vertices;
    };

    struct Manifold{
        u32 count;
        float depth[2];
        vec2 contact[2];
        vec2 normal[2];
    };
}

// ---- boolean collision

inline bool point_circle(const vec2& point, const cshape::Circle& circle);
inline bool point_rect(const vec2& point, const cshape::Rect& rect);
inline bool point_capsule(const vec2& point, const cshape::Capsule& capsule);
inline bool point_ray(const vec2& point, const cshape::Ray& ray);

inline bool circle_circle(const cshape::Circle& circleA, const cshape::Circle& circleB);
inline bool circle_rect(const cshape::Circle& circle, const cshape::Rect& rect);
inline bool circle_capsule(const cshape::Circle& circle, const cshape::Capsule& capsule);
//inline bool circle_ray(const cshape::Circle& circle, const cshape::Ray& ray);

inline bool rect_rect(const cshape::Rect& rectA, const cshape::Rect& rectB);
//inline bool rect_capsule(const cshape::Rect& rect, const cshape::Capsule& capsule);

// NOTE(hugo):
// * axisX  : inv_direction.x should not be zero
// * axisY  : inv_direction.y should not be zero
// * axisXY : inv_direction.x & inv_direction.y should not be zero
inline bool rect_ray_axisX(const cshape::Rect& rect, cshape::Ray& ray);
inline bool rect_ray_axisY(const cshape::Rect& rect, cshape::Ray& ray);
inline bool rect_ray_axisXY(const cshape::Rect& rect, cshape::Ray& ray);

// ---- detailed collision

// NOTE(hugo):
// - manifold.normal are from shapeA to shapeB
// - manifold.contact represent the point / plane of the collision (on the shapeB)

inline void circle_circle_manifold(const cshape::Circle& circleA, const cshape::Circle& circleB, cshape::Manifold& manifold);

// ---- GJK & predicates

// REF(hugo):
// https://caseymuratori.com/blog_0003
// https://www.youtube.com/watch?v=Qupqu1xe7Io
// https://www.youtube.com/watch?v=MDusDn8oTSE
// Hill Climbing: https://graphics.stanford.edu/courses/cs468-01-fall/Papers/cameron.pdf

vec2 furthest_point(const vec2& direction, const cshape::Polygon& poly);

template<typename TypeA, typename TypeB>
bool GJK(const TypeA& shapeA, const TypeB& shapeB);

#endif
