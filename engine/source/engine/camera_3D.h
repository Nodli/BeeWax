#ifndef H_CAMERA_3D
#define H_CAMERA_3D

// NOTE(hugo): world is in a right handed coordinates system
//             +Y  -Z
//              | /
//              |/
//     -X <---- 0 ---> +X
//             /|
//            / |
//          +Z -Y

struct Camera_3D_FP{
    vec3 right();
    vec3 up();
    vec3 forward();

    mat4 camera_matrix();

    void rotate(const float pitch, const float yaw, const float roll);
    void rotate_to_show_centered(const vec3 vec);

    // NOTE(hugo): default with camera_quaternion = {1., 0., 0., 0.}
    // * view_origin =      position.x,  position.y,  position.z
    // * view_direction =   0,  0,  1
    // * right =           -1,  0,  0
    // * up =               0,  1,  0
    // * forward =          0,  0,  1

    vec3 position;
    float camera_near_plane;
    quat camera_quaternion;
    float vertical_fov;
    float aspect_ratio; // NOTE(hugo): width / height
};

struct Camera_3D_Orbit{
    vec3 right();
    vec3 up();
    vec3 forward();

    mat4 camera_matrix();

    void rotate(const float pitch, const float yaw, const float roll);

    // NOTE(hugo): default with camera_quaternion = {1., 0., 0., 0.}
    // * view_origin =      position.x,  position.y,  position.z - orbit_radius
    // * view_direction =   0,  0,  1
    // * right =           -1,  0,  0
    // * up =               0,  1,  0
    // * forward =          0,  0,  1

    vec3 orbit_center;
    float orbit_radius;
    float camera_near_plane;
    quat camera_quaternion;
    float vertical_fov;
    float aspect_ratio; // NOTE(hugo): width / height
};


#endif
