#ifndef H_TEXTURE
#define H_TEXTURE

struct Texture_Asset{
    u8* bitmap;
    u32 width;
    u32 height;
    Texture texture;
};

struct Texture_View{
    vec2 uvmin;
    vec2 uvmax;
    Texture texture;
};

struct Texture_Animation_Asset{
    struct Texture_Animation_Frame{
        Texture_View view;
        u32 frame_duration;
    };
    array<Texture_Animation_Frame> animation_frames;
};

void make_texture_asset_from_png_file(Texture_Asset* asset, const File_Path& path, Render_Layer* renderer);
void free_texture_asset(Texture_Asset* asset, Render_Layer* renderer);

#endif
