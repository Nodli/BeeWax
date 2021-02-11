#ifndef H_CAMERA_2D
#define H_CAMERA_2D

// NOTE(hugo): aspect_ratio is width / height

struct Camera_Rect{
    vec2 min = {};
    vec2 max = {};
};

struct Camera_2D{
    Camera_Rect view_rect();
    float width();

    vec2 right();
    vec2 up();

    mat3 camera_matrix();
    vec2 screen_to_world_coordinates(const vec2& screen_coord);
    vec2 world_to_screen_coordinates(const vec2& world_coord);

    // ---- camera data

    vec2 center = {};
    float height = 0.f;
    float aspect_ratio = 0.f;
};

void move_to_show(Camera_2D& camera, vec2 position);
void extend_to_show(Camera_2D& camera, vec2 position);
void keep_in_box(Camera_2D& camera, vec2 position, Camera_Rect screenspace_box);
void move_to_position_smooth(Camera_2D& camera, vec2 position, float smoothing_ratio);

#endif
