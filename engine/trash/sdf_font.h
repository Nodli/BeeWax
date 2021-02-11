#ifndef H_FONT
#define H_FONT

#if 0
#define ASCII_PRINTABLE R"(' !"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~';)"

// ---- SDF font asset

// REF(hugo):
// https://medium.com/@evanwallace/easy-scalable-text-rendering-on-the-gpu-c3f4d782c5ac
// http://chanae.walon.org/pub/ttf/ttf_glyphs.htm
// https://github.com/justinmeiners/stb-truetype-example/blob/master/main.c

struct SDF_Font_Asset{
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
    hashmap<char, CodePoint_Info> codepoint_to_info;
    float internal_font_scale;
    float ascent;
    float descent;
    float linegap;

    // NOTE(hugo): keep the cpu bitmap to make texture updates
    float bitmap_font_size = 0.f;
    u32 bitmap_width = 0u;
    u32 bitmap_height = 0u;
    u8* bitmap_data = nullptr;

    Texture_ID texture;
};

SDF_Font_Asset font_asset_from_ttf_file(const File_Path& path,
    const char* character_string, u32 character_string_size, float font_size,
    Renderer* renderer);
void free_font_asset(Font_Asset& asset, Renderer* renderer);

// ---- font rendering

float baseline_to_baseline(Font_Asset* asset, float font_size);

struct Text_Box{
    vec2 baseline;
    vec2 min;
    vec2 max;
    float above_baseline;
    float below_baseline;
};
Text_Box compute_string_text_box(Font_Asset* asset, const char* string, vec2 baseline, float font_size);

struct Layout_Box{
    vec2 min;
    vec2 max;
};
void center_vertical(Text_Box& text, const Layout_Box& layout);
void center_horizontal(Text_Box& text, const Layout_Box& layout);

// NOTE(hugo): returns the end of string baseline
// /baseline/ in pixel coordinates ; bottom-left origin
vec2 draw_text(Font_Asset* asset, const char* text, vec2 baseline, float font_size)
#endif

#endif
