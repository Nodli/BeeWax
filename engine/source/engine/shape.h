#ifndef SHAPE_H
#define SHAPE_H

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

// NOTE(hugo): inv_direction is cached for faster collision but not computed in the following functions
//             the padding would add 3 floats anyway
struct Ray{
    vec2 origin;
    vec2 direction;
    vec2 inv_direction;
    float length;
};

#endif
