// REF(hugo):
// https://halt.software/dead-simple-layouts/

struct Layout_Rect{
    ivec2 min;
    ivec2 max;
};

Layout_Rect cut_L(Layout_Rect& parent, u32 size){
    Layout_Rect output;
    output.min = parent.min;
    parent.min.x = min(parent.min.x + (s32)size, parent.max.x);
    output.max.x = parent.min.x;
    output.max.y = parent.max.y;
    return output;
}
Layout_Rect cut_R(Layout_Rect& parent, u32 size){
    Layout_Rect output;
    output.max = parent.max;
    parent.max.x = max(parent.min.x, parent.max.x - (s32)size);
    output.min.x = parent.max.x;
    output.min.y = parent.min.y;
    return output;
}
Layout_Rect cut_B(Layout_Rect& parent, u32 size){
    Layout_Rect output;
    output.min = parent.min;
    parent.min.y = min(parent.min.y + (s32)size, parent.max.y);
    output.max.y = parent.min.y;
    output.max.x = parent.max.x;
    return output;
}
Layout_Rect cut_T(Layout_Rect& parent, u32 size){
    Layout_Rect output;
    output.max = parent.max;
    parent.max.y = max(parent.min.y, parent.max.y - (s32)size);
    output.min.y = parent.max.y;
    output.min.x = parent.min.x;
    return output;
}

void contract_L(Layout_Rect& parent, u32 size){
    parent.min.x = min(parent.min.x + (s32)size, parent.max.x);
}
void contract_R(Layout_Rect& parent, u32 size){
    parent.max.x = max(parent.min.x, parent.max.x - (s32)size);
}
void contract_B(Layout_Rect& parent, u32 size){
    parent.min.y = min(parent.min.y + (s32)size, parent.max.y);
}
void contract_T(Layout_Rect& parent, u32 size){
    parent.max.y = max(parent.min.y, parent.max.y - (s32)size);
}

void extend_L(Layout_Rect& parent, u32 size){
    parent.min.x = parent.min.x - (s32)size;
}
void extend_R(Layout_Rect& parent, u32 size){
    parent.max.x = parent.max.x + (s32)size;
}
void extend_B(Layout_Rect& parent, u32 size){
    parent.min.y = parent.min.y - (s32)size;
}
void extend_T(Layout_Rect& parent, u32 size){
    parent.max.y = parent.max.y + (s32)size;
}

Layout_Rect map_to_target(const Layout_Rect& rect, u32 in_width, u32 in_height, u32 out_width, u32 out_height){
    Layout_Rect output;
    output.min.x = ceil_s32 ((float)(rect.min.x * out_width ) / (float)in_width );
    output.min.y = ceil_s32 ((float)(rect.min.y * out_height) / (float)in_height);
    output.max.x = floor_s32((float)(rect.max.x * out_width ) / (float)in_width );
    output.max.y = floor_s32((float)(rect.max.y * out_height) / (float)in_height);
    return output;
}

// ----

// REF(hugo):
// https://github.com/memononen/fontstash

constexpr u32 font_stash_default_dimension = 128u;
constexpr u32 font_stash_padding = 2u;

struct Font_Stash{
    void create();
    void destroy();

    void stash_code_point(s32 code_point, u32 font_size);
    s32 measure_str(const char* str, u32 font_size);

    // ----

    struct Font_Data{
        Font_Asset* asset;

        struct Code_Point_Key{
            s32 code_point;
            float font_size;
        };
        struct Code_Point_Data{
            vec2 min;
            vec2 max;
        };
        hashmap<Code_Point_Key, Code_Point_Data> cache;
    } font_data;

    Rect_Packer packer;

    void* image;
    u32 dimension;

    Texture texture;
    s32 texture_dirty;
};

DECLARE_EQUALITY_OPERATOR(Font_Stash::Font_Data::Code_Point_Key);

void draw_text(Font_Stash* stash, const char* str, ivec2 baseline, u32 font_size, float depth, u32 color, u32 target_width, u32 target_height);
void draw_text(Font_Stash* stash, const char* str, const Layout_Rect& rect, float depth, u32 color, u32 target_width, u32 target_height);
