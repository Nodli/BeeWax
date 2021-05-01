vec3 Camera_3D_FP::right(){
    vec3 eX = {1.f, 0.f, 0.f};
    return bw::rotated(eX, inverse(view_quaternion));
};

vec3 Camera_3D_FP::up(){
    vec3 eY = {0.f, 1.f, 0.f};
    return bw::rotated(eY, inverse(view_quaternion));
};

vec3 Camera_3D_FP::forward(){
    vec3 eZ = {0.f, 0.f,-1.f};
    return bw::rotated(eZ, inverse(view_quaternion));
};

mat4 Camera_3D_FP::view_matrix(){
    // NOTE(hugo): world space --(translation + rotation)-> view space
    mat4 wv_translation_mat = mat_translation3D(- position);
    mat4 wv_rotation_mat = to_mat4(view_quaternion);

    return wv_rotation_mat * wv_translation_mat;
}

// NOTE(hugo): view space --(projection scaling)-> clip space
mat4 Camera_3D_FP::orthographic_matrix(){
    return mat_orthographic3D(vertical_span, aspect_ratio, near_plane, far_plane);
}
mat4 Camera_3D_FP::perspective_matrix(){
    return mat_perspective3D(vertical_fov, aspect_ratio, near_plane, far_plane);
}
mat4 Camera_3D_FP::perspective_matrix_infinite(){
    return mat_infinite_perspective3D(vertical_fov, aspect_ratio, near_plane);
}

void Camera_3D_FP::rotate(const float pitch, const float yaw, const float roll){
    quat rotation_quat = quat_from_euler(pitch, yaw, roll);
    view_quaternion = rotation_quat * view_quaternion;
}

void Camera_3D_FP::rotate_to_show_centered(const vec3 vec){
    UNUSED(vec);
}

vec3 Camera_3D_Orbit::position(){
    return orbit_center - orbit_radius * forward();
}

vec3 Camera_3D_Orbit::right(){
    vec3 eX = {1.f, 0.f, 0.f};
    return bw::rotated(eX, inverse(view_quaternion));
};
vec3 Camera_3D_Orbit::up(){
    vec3 eY = {0.f, 1.f, 0.f};
    return bw::rotated(eY, inverse(view_quaternion));
};
vec3 Camera_3D_Orbit::forward(){
    vec3 eZ = {0.f, 0.f,-1.f};
    return bw::rotated(eZ, inverse(view_quaternion));
};

mat4 Camera_3D_Orbit::view_matrix(){
    vec3 default_direction = {0., 0.,-1.};
    vec3 viewer_relative_position = - default_direction * orbit_radius;
    viewer_relative_position = bw::rotated(viewer_relative_position, inverse(view_quaternion));
    vec3 viewer_position = orbit_center + viewer_relative_position;

    // NOTE(hugo): world space --(translation + rotation)-> view space
    mat4 wv_translation_mat = mat_translation3D(- viewer_position);
    mat4 wv_rotation_mat = to_mat4(view_quaternion);

    return wv_rotation_mat * wv_translation_mat;
}

// NOTE(hugo): view space --(projection scaling)-> clip space
mat4 Camera_3D_Orbit::orthographic_matrix(){
    return mat_orthographic3D(vertical_span, aspect_ratio, near_plane, far_plane);
}
mat4 Camera_3D_Orbit::perspective_matrix(){
    return mat_perspective3D(vertical_fov, aspect_ratio, near_plane, far_plane);
}
mat4 Camera_3D_Orbit::perspective_matrix_infinite(){
    return mat_infinite_perspective3D(vertical_fov, aspect_ratio, near_plane);
}

void Camera_3D_Orbit::rotate(const float pitch, const float yaw, const float roll){
    quat rotation_quat = quat_from_euler(- pitch, - yaw, - roll);
    view_quaternion = rotation_quat * view_quaternion;
}
