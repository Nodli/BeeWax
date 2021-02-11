#ifndef H_TEXTURE
#define H_TEXTURE

struct Texture_View{
    vec2 uvmin;
    vec2 uvmax;
    Texture texture;
};

struct Texture_Asset{
    u8* bitmap;
    u32 width;
    u32 height;
    Texture texture;
};

void make_texture_asset_from_png_file(Texture_Asset* asset, const File_Path& path, Renderer* renderer);
void free_texture_asset(Texture_Asset* asset, Renderer* renderer);

#endif
