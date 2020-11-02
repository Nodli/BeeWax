vec3 Camera_3D_FP::right(){
    vec3 eX = {- 1.f, 0.f, 0.f};
    return bw::rotate(eX, inverse(camera_quaternion));
};

vec3 Camera_3D_FP::up(){
    vec3 eY = {0.f, 1.f, 0.f};
    return bw::rotate(eY, inverse(camera_quaternion));
};

vec3 Camera_3D_FP::forward(){
    vec3 eZ = {0.f, 0.f, 1.f};
    return bw::rotate(eZ, inverse(camera_quaternion));
};

mat4 Camera_3D_FP::camera_matrix(){
    // NOTE(hugo): world space --(translation + rotation)-> view space
    mat4 wv_translation_mat = mat_translation3D(- position);
    mat4 wv_rotation_mat = to_mat4(camera_quaternion);

    // NOTE(hugo): view space --(projection scaling)-> clip space
    //mat4 vp = mat_orthographic3D(10.f * aspect_ratio, 10.f, radius, 1000.f * radius);
    //mat4 vp = mat_perspective3D(vertical_fov, aspect_ratio, radius, 1000.f * radius);
    mat4 vp = mat_infinite_perspective3D(vertical_fov, aspect_ratio, camera_near_plane);

    return vp * wv_rotation_mat * wv_translation_mat;
}

void Camera_3D_FP::rotate(const float pitch, const float yaw, const float roll){
    quat rotation_quat = quat_from_euler(pitch, yaw, roll);
    camera_quaternion = rotation_quat * camera_quaternion;
}

void Camera_3D_FP::rotate_to_show_centered(const vec3 vec){
    UNUSED(vec);
}

vec3 Camera_3D_Orbit::right(){
    vec3 eX = {- 1.f, 0.f, 0.f};
    return bw::rotate(eX, inverse(camera_quaternion));
};
vec3 Camera_3D_Orbit::up(){
    vec3 eY = {0.f, 1.f, 0.f};
    return bw::rotate(eY, inverse(camera_quaternion));
};
vec3 Camera_3D_Orbit::forward(){
    vec3 eZ = {0.f, 0.f, 1.f};
    return bw::rotate(eZ, inverse(camera_quaternion));
};

mat4 Camera_3D_Orbit::camera_matrix(){
    vec3 default_direction = {0., 0., 1.};
    vec3 viewer_relative_position = - default_direction * orbit_radius;
    viewer_relative_position = bw::rotate(viewer_relative_position, inverse(camera_quaternion));
    vec3 viewer_position = orbit_center + viewer_relative_position;

    // NOTE(hugo): world space --(translation + rotation)-> view space
    mat4 wv_translation_mat = mat_translation3D(- viewer_position);
    mat4 wv_rotation_mat = to_mat4(camera_quaternion);

    // NOTE(hugo): view space --(projection scaling)-> clip space
    //mat4 vp = mat_orthographic3D(10.f * aspect_ratio, 10.f, radius, 1000.f * radius);
    //mat4 vp = mat_perspective3D(vertical_fov, aspect_ratio, radius, 1000.f * radius);
    mat4 vp = mat_infinite_perspective3D(vertical_fov, aspect_ratio, camera_near_plane);

    return vp * wv_rotation_mat * wv_translation_mat;
}

void Camera_3D_Orbit::rotate(const float pitch, const float yaw, const float roll){
    quat rotation_quat = quat_from_euler(- pitch, - yaw, - roll);
    camera_quaternion = rotation_quat * camera_quaternion;
}
