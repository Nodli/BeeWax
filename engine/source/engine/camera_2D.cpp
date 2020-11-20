c2D::Rect Camera_2D::view_rect(){
    c2D::Rect output;

    float half_height = 0.5f * height;
    float half_width = half_height * aspect_ratio;
    output.min.x = center.x - half_width;
    output.min.y = center.y - half_height;
    output.max.x = center.x + half_width;
    output.max.y = center.y + half_height;

    return output;
}

float Camera_2D::width(){
    return height * aspect_ratio;
}

mat3 Camera_2D::camera_matrix(){
    float width = height * aspect_ratio;
    assert(width > 0.f && height > 0.f);

    // NOTE(hugo):
    // world --(translation)-> view --(projecion scaling)-> clip
    // unoptimized is: mat_orthographic2D(width, height) * mat_tranlation2D(- center)
    return mat3_rm(
        2.f / width,    0.f,            - 2.f * center.x / width,
        0.f,            2.f / height,   - 2.f * center.y / height,
        0.f,            0.f,            1.f
    );
}

vec2 Camera_2D::screen_to_world_coordinates(const vec2& screen_coord){
    float width = height * aspect_ratio;
    return {screen_coord.x * width + center.x, screen_coord.y * height + center.y};
}

vec2 Camera_2D::world_to_screen_coordinates(const vec2& world_coord){
    float width = height * aspect_ratio;
    return {(world_coord.x - center.x) / width, (world_coord.y - center.y) / height};
}

void move_to_show(Camera_2D& camera, vec2 position){
    float half_height = camera.height / 2.f;

    float half_width = half_height * camera.aspect_ratio;
    float x_left = (camera.center.x - half_width) - position.x;
    float x_right = position.x - (camera.center.x + half_width);
    float x_dcenter = - max(x_left, 0.f) + max(x_right, 0.f);

    float y_left = (camera.center.y - half_height) - position.y;
    float y_right = position.y - (camera.center.y + half_height);
    float y_dcenter = - max(y_left, 0.f) + max(y_right, 0.f);

    camera.center.x += x_dcenter;
    camera.center.y += y_dcenter;
}

void extend_to_show(Camera_2D& camera, vec2 position){
    float half_width = camera.height * camera.aspect_ratio / 2.f;
    float half_height = camera.height / 2.f;

    vec2 required_min = {min(camera.center.x - half_width, position.x), min(camera.center.y - half_height, position.y)};
    vec2 required_max = {max(camera.center.x + half_width, position.x), max(camera.center.y + half_height, position.y)};

    camera.center = (required_min + required_max) / 2.f;
}

void keep_in_box(Camera_2D& camera, vec2 position, c2D::Rect screenspace_box){
    // NOTE(hugo): no aspect ratio otherwise /screenspace_box/ would have to be aspect ratio corrected too
    float screenspace_factor = 2.f / camera.height;
    vec2 to_position = position - camera.center;
    vec2 screenspace_position = to_position * screenspace_factor;

    if(screenspace_position.x < screenspace_box.min.x){
        float screenspace_dx = screenspace_position.x - screenspace_box.min.x;
        camera.center.x = camera.center.x + screenspace_dx / screenspace_factor;
    }else if(screenspace_position.x > screenspace_box.max.x){
        float screenspace_dx = screenspace_position.x - screenspace_box.max.x;
        camera.center.x = camera.center.x + screenspace_dx / screenspace_factor;
    }
    if(screenspace_position.y < screenspace_box.min.y){
        float screenspace_dy = screenspace_position.y - screenspace_box.min.y;
        camera.center.y = camera.center.y + screenspace_dy / screenspace_factor;
    }else if(screenspace_position.y > screenspace_box.max.y){
        float screenspace_dy = screenspace_position.y - screenspace_box.max.y;
        camera.center.y = camera.center.y + screenspace_dy / screenspace_factor;
    }
}

void move_to_position_smooth(Camera_2D& camera, vec2 position, float smoothing_ratio){
    vec2 to_position = position - camera.center;
    camera.center += to_position * smoothing_ratio;
}
