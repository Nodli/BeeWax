#ifndef H_FONT
#define H_FONT

#define ASCII_PRINTABLE R"(' !"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~';)"

// REF(hugo):
// https://medium.com/@evanwallace/easy-scalable-text-rendering-on-the-gpu-c3f4d782c5ac
// http://chanae.walon.org/pub/ttf/ttf_glyphs.htm
// https://github.com/justinmeiners/stb-truetype-example/blob/master/main.c

struct Font_Renderer{
    // NOTE: glyph_edge_value and glyph_padding are hard coded in the shader
    void make_bitmap_from_file(const File_Path& path, const char* string, float font_size, s32 glyph_padding, s32 glyph_edge_value);
    void free();

    void start_frame();
    void end_frame();

    // NOTE(hugo): returns baseline_x at the end of the string
    float batch_string(const char* string, float baseline_x, float baseline_y, float font_size);
    void render();

    // ---- font data

    struct codepoint_info{
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

    float bitmap_font_size = 0.f;
    u32 bitmap_width = 0u;
    u32 bitmap_height = 0u;
    u8* bitmap_data = nullptr;

    dhashmap<char, codepoint_info> codepoint_to_info;

    // ---- rendering data

    Window* window = nullptr;
    Renderer* renderer = nullptr;
    Texture_ID texture;
    Vertex_Batch_ID batch;
};

#endif
