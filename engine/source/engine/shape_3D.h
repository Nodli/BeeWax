#ifndef SHAPE_3D_H
#define SHAPE_3D_H

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
