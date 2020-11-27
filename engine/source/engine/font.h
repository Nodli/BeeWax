#ifndef H_FONT
#define H_FONT

#define ASCII_PRINTABLE R"(' !"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~';)"

// REF(hugo):
// https://medium.com/@evanwallace/easy-scalable-text-rendering-on-the-gpu-c3f4d782c5ac
// http://chanae.walon.org/pub/ttf/ttf_glyphs.htm
// https://github.com/justinmeiners/stb-truetype-example/blob/master/main.c

struct Font_Asset{
    struct CodePoint_Info{
        vec2 uv_min;
        vec2 uv_max;
        s16 quad_offset_x;
        s16 quad_offset_y;
        s16 quad_width;
        s16 quad_height;
        float cursor_advance;
    };

    buffer<u8> file;
    stbtt_fontinfo info;
    dhashmap<char, CodePoint_Info> codepoint_to_info;

    // NOTE(hugo): keep the cpu bitmap to make texture updates
    float bitmap_font_size = 0.f;
    u32 bitmap_width = 0u;
    u32 bitmap_height = 0u;
    u8* bitmap_data = nullptr;

    Texture_ID texture;
};

Font_Asset font_asset_from_ttf_file(const File_Path& path,
    const char* character_string, u32 character_string_size, float font_size,
    Renderer* renderer);
void free_font_asset(Font_Asset& asset, Renderer* renderer);

struct Font_Renderer{
    float render_string(Font_Asset* asset, const char* string, float baseline_x, float baseline_y, float font_size);

    // ---- data

    u32 width;
    u32 height;
    Renderer* renderer = nullptr;
};

#endif
