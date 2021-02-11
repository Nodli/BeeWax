mat4 mat3D_from_mat2D(const mat3& mat){
    return {
        mat.data[0], mat.data[1], 0.f, mat.data[2],
        mat.data[3], mat.data[4], 0.f, mat.data[5],
        0.f,         0.f,         1.f, 0.f,
        mat.data[6], mat.data[7], 0.f, mat.data[8]
    };
}

mat3 mat_translation2D(vec2 vec){
    return mat3_rm(
        1.f, 0.f, vec.x,
        0.f, 1.f, vec.y,
        0.f, 0.f, 1.f
    );
}

mat3 mat_orthographic2D(float screen_width, float screen_height){
    return mat3_rm(
        2.f / screen_width, 0.f,                    0.f,
        0.f,                2.f / screen_height,    0.f,
        0.f,                0.f,                    1.f
    );
}

mat4 mat_translation3D(vec3 vec){
    return mat4_rm(
        1.f, 0.f, 0.f, vec.x,
        0.f, 1.f, 0.f, vec.y,
        0.f, 0.f, 1.f, vec.z,
        0.f, 0.f, 0.f, 1.f
    );
}

mat4 mat_orthographic3D(float vspan, float aspect_ratio, float znear, float zfar){
    float half_height = vspan * 0.5f;
    float half_width = aspect_ratio * half_height;
    float div_dz = 1.f / (zfar - znear);
    return mat4_rm(
        1.f / half_width,   0.f,                    0.f,              0.f,
        0.f,                1.f / half_height,      0.f,              0.f,
        0.f,                0.f,                  - div_dz,  - 0.5f * (zfar + znear) * div_dz + 0.5f,
        0.f,                0.f,                    0.f,              1.f
    );
}

mat4 mat_perspective3D(float vfov, float aspect_ratio, float znear, float zfar){
    assert(znear > 0.f && znear < zfar);
    float half_height = tan(vfov * 0.5f);
    float half_width = aspect_ratio * half_height;
    float div_dz = 1.f / (zfar - znear);
    return mat4_rm(
        1.f / half_width,   0.f,                0.f,                                    0.f,
        0.f,                1.f / half_height,  0.f,                                    0.f,
        0.f,                0.f,              - 0.5f * (zfar + znear) * div_dz - 0.5f,- 1.f * zfar * znear * div_dz,
        0.f,                0.f,              - 1.f,                                    0.f
    );
}

mat4 mat_infinite_perspective3D(float vfov, float aspect_ratio, float znear){
    float half_height = tan(vfov * 0.5f);
    float half_width = aspect_ratio * half_height;

DISABLE_WARNING_PUSH
DISABLE_WARNING_TYPE_PUNNING
    // NOTE(hugo): 2^-22
    u32 u32_infinite_precision = (0x69) << 23;
    float infinite_precision = *(float*)(&u32_infinite_precision);
DISABLE_WARNING_POP

    return mat4_rm(
        1.f / half_width,   0.f,                0.f,                                      0.f,
        0.f,                1.f / half_height,  0.f,                                      0.f,
        0.f,                0.f,                0.5f * (infinite_precision - 1.f) - 0.5f, 0.5f * (infinite_precision - 2.f) * znear,
        0.f,                0.f,              - 1.f,                                      0.f
    );
}


