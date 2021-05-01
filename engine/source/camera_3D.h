#ifndef H_CAMERA_3D
#define H_CAMERA_3D

// NOTE(hugo): world is in a right handed coordinate system
//             +Y  -Z (far)
//              | /
//              |/
//     -X <---- 0 ---> +X
//             /|
//            / |
//   (near) +Z -Y

// NOTE(hugo): aspect_ratio = width / height

struct Camera_3D_FP{
    vec3 right();
    vec3 up();
    vec3 forward();

    // NOTE(hugo): camera_matrix = projection_matrix * view_matrix
    mat4 view_matrix();
    mat4 orthographic_matrix();
    mat4 perspective_matrix();
    mat4 perspective_matrix_infinite();

    // NOTE(hugo): rotate(axis_X, axis_Y, axis_Z)
    void rotate(const float pitch, const float yaw, const float roll);
    void rotate_to_show_centered(const vec3 vec);

    // NOTE(hugo): default with view_quaternion = {1., 0., 0., 0.}
    // * view_origin =      position.x,  position.y,  position.z
    // * view_direction =   0,  0, -1
    // * right =            1,  0,  0
    // * up =               0,  1,  0
    // * forward =          0,  0, -1

    vec3 position;
    quat view_quaternion = identity_quaternion<quat>;
    float aspect_ratio;

    float near_plane;
    float far_plane;
    float vertical_fov;
    float vertical_span;
};

struct Camera_3D_Orbit{
    vec3 position();

    vec3 right();
    vec3 up();
    vec3 forward();

    // NOTE(hugo): camera_matrix = projection_matrix * view_matrix
    mat4 view_matrix();
    mat4 orthographic_matrix();
    mat4 perspective_matrix();
    mat4 perspective_matrix_infinite();

    // NOTE(hugo): rotate(axis_X, axis_Y, axis_Z)
    void rotate(const float pitch, const float yaw, const float roll);

    // NOTE(hugo): default with view_quaternion = {1., 0., 0., 0.}
    // * view_origin =      position.x,  position.y,  position.z - orbit_radius
    // * view_direction =   0,  0, -1
    // * right =            1,  0,  0
    // * up =               0,  1,  0
    // * forward =          0,  0, -1

    // NOTE(hugo): view_quaternion is inverted because the camera turns around the object (cf. view_matrix())
    // ie use quat_from_vector_to_vector(expected_orientation, camera.forward());

    vec3 orbit_center;
    quat view_quaternion = identity_quaternion<quat>;
    float orbit_radius;
    float aspect_ratio;

    float near_plane;
    float far_plane;
    float vertical_fov;
    float vertical_span;
};

constexpr vec3 identity_view_direction = {0.f, 0.f, 1.f};

mat4 shadow_map_matrix();

#endif
