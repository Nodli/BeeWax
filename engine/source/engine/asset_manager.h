#ifndef H_ASSET_MANAGER
#define H_ASSET_MANAGER

struct Texture_Asset{
    u8* bitmap;
    u32 width;
    u32 height;
    Texture_ID texture;
};

Texture_Asset texture_asset_from_png_file(const File_Path& path, Renderer* renderer){
    Texture_Asset asset;

    s32 width, height, nchannels;
    asset.bitmap = stbi_load(path.data, &width, &height, &nchannels, 4u);
    asset.width = width;
    asset.height = height;

    asset.texture = renderer->get_texture(TEXTURE_FORMAT_RGBA, width, height, TYPE_UBYTE, asset.bitmap);

    return asset;
}

void free_texture_asset(Texture_Asset& asset, Renderer* renderer){
    ::free(asset.bitmap);
    renderer->free_texture(asset.texture);
}

struct Asset_Manager{
    typedef sstring<60u> Asset_Tag;

    void terminate();
    void from_asset_file(const File_Path& path);

    Audio_Asset* get_audio(const Asset_Tag& tag);
    Texture_Asset* get_texture(const Asset_Tag& tag);
    Texture_Animation_Asset* get_texture_animation(const Asset_Tag& tag);
    Font_Asset* get_font(const Asset_Tag& tag);

    // ---- internal

    const char* make_audio_asset(Asset_Tag tag, const char* cursor);
    const char* make_texture_asset(Asset_Tag tag, const char* cursor);
    const char* make_texture_animation_asset(Asset_Tag tag, const char* cursor);
    const char* make_font_asset(Asset_Tag tag, const char* cursor);

    // ---- data

    Audio_Player* audio_player = nullptr;
    Renderer* renderer = nullptr;

    Audio_Asset audio_default = {nullptr, 0u};
    dhashmap<Asset_Tag, Audio_Asset> audio;

    Texture_Asset texture_default;
    dhashmap<Asset_Tag, Texture_Asset> texture;

    Texture_Animation_Asset texture_animation_default;
    dhashmap<Asset_Tag, Texture_Animation_Asset> texture_animation;

    Font_Asset font_default;
    dhashmap<Asset_Tag, Font_Asset> font;
};

#endif

