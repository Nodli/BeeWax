#ifndef H_COLLISION
#define H_COLLISION

// REF(hugo): https://www.youtube.com/watch?v=-_IspRG548E
// REF(hugo): https://www.youtube.com/watch?v=MDusDn8oTSE

namespace c2D{
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
}

#endif
