#ifndef H_COLLISION
#define H_COLLISION

namespace codet{
    inline bool point_circle(vec2* point, Circle* circle);
    inline bool point_rect(vec2* point, Rect* rect);
    inline bool point_capsule(vec2* point, Capsule* capsule);
    inline bool point_ray(vec2* point, Ray* ray);

    inline bool circle_circle(Circle* circleA, Circle* circleB);
    inline bool circle_rect(Circle* circle, Rect* rect);
    inline bool circle_capsule(Circle* circle, Capsule* capsule);
    inline bool circle_ray(Circle* circle, Ray* ray);

    inline bool rect_rect(Rect* rectA, Rect* rectB);

    // NOTE(hugo): inv_direction.x should not be zero
    inline bool rect_ray_axisX(Rect* rect, Ray* ray);
    // NOTE(hugo): inv_direction.y should not be zero
    inline bool rect_ray_axisY(Rect* rect, Ray* ray);
    // NOTE(hugo): inv_direction.x and inv_direction.y should not be zero
    inline bool rect_ray_axisXY(Rect* rect, Ray* ray);
}

#endif
