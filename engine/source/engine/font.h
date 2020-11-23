#ifndef H_FONT
#define H_FONT

#define ASCII_PRINTABLE R"(' !"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~';)"

// REF(hugo):
// https://medium.com/@evanwallace/easy-scalable-text-rendering-on-the-gpu-c3f4d782c5ac
// http://chanae.walon.org/pub/ttf/ttf_glyphs.htm
// https://github.com/justinmeiners/stb-truetype-example/blob/master/main.c

typedef u32 Font_ID;

struct Font_Manager{
    // NOTE(hugo): do not use this between start_frame() and end_frame()
    void terminate();

    // NOTE(hugo): the shader has hard-coded values for /glyph_padding/ & /glyph_edge_value/
    Font_ID font_from_file(const File_Path& path, const char* character_string, float font_size, s32 glyph_padding = 5, s32 glyph_edge_value = 180);
    void remove_font(Font_ID font);

    // NOTE(hugo): acquisition & release of the renderer resources to render strings using this font
    void start_frame(Font_ID font);
    void end_frame(Font_ID font);

    // NOTE(hugo): returns baseline_x at the end of the string
    float batch_string(Font_ID font, const char* string, float baseline_x, float baseline_y, float font_size);
    void render_batch(Font_ID font);

    // ---- font data

    struct CodePoint_Info{
        vec2 uv_min;
        vec2 uv_max;
        s16 quad_offset_x;
        s16 quad_offset_y;
        s16 quad_width;
        s16 quad_height;
        float cursor_advance;
    };

    struct Font_Data{
        buffer<u8> file;
        stbtt_fontinfo info;
        dhashmap<char, CodePoint_Info> codepoint_to_info;

        float bitmap_font_size = 0.f;
        u32 bitmap_width = 0u;
        u32 bitmap_height = 0u;
        u8* bitmap_data = nullptr;

        Texture_ID texture;
        Vertex_Batch_ID batch;
    };
    dpool<Font_Data> fonts;

    // ---- rendering data

    Window* window = nullptr;
    Renderer* renderer = nullptr;
};

#endif
