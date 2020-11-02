#ifndef H_FONT
#define H_FONT

// https://medium.com/@evanwallace/easy-scalable-text-rendering-on-the-gpu-c3f4d782c5ac
// NOTE(hugo): http://chanae.walon.org/pub/ttf/ttf_glyphs.htm
struct Font_Rendering{
    void setup_from_file(const char* ttf_file);
    void terminate();

    void batch_string(Renderer* renderer, Vertex_Batch_ID batch, const char* string, vec2 baseline_position, float font_scale, vec4 color);

    // ---- data

    buffer<u8> font_file;
    stbtt_fontinfo font_info;

    float font_scaling = 0.f;
    float font_ascend = 0.f;
    float font_descent = 0.f;
    float font_baseline_to_baseline = 0.f;
    darray<vec2> font_vertices;

    struct codepoint_info{
        s32 glyph_index = 0;
        float glyph_advance_width = 0.f;
        float glyph_left_side_bearing = 0.f;

        u32 vertex_offset = 0u;
        u32 number_of_vertices = 0u;
    };
    codepoint_info ascii[256u];
};

#endif
