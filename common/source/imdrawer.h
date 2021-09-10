#ifndef H_IMDRAWER
#define H_IMDRAWER

struct ImDrawer{
    void create();
    void destroy();

    // ----

    enum Command_Type{
        POLYGON,
        POLYGON_INDEXED,
        POLYGON_TEXTURED,
        NUMBER_OF_COMMAND_TYPES,
        NONE = NUMBER_OF_COMMAND_TYPES
    };
    struct Command_Polygon{
        u32 buffer_index;
        u32 vertex_index;
        u32 vertex_count;
    };
    struct Command_Polygon_Indexed{
        u32 buffer_index;
        u32 index_index;
        u32 index_count;
    };
    struct Command_Polygon_Textured{
        u32 buffer_index;
        u32 vertex_index;
        u32 vertex_count;
        Texture texture;
    };
    struct Command{
        Command_Type type;
        Shader_Name shader;
        union{
            Command_Polygon polygon;
            Command_Polygon_Indexed polygon_indexed;
            Command_Polygon_Textured polygon_textured;
        };
    };

    struct Buffer{
        Vertex_Format_Name vertex_format_name;
        u32 vertex_count;
        Transient_Buffer buffer;
    };
    struct Indexed_Buffer{
        Vertex_Format_Name vertex_format_name;
        u32 vertex_count;
        u32 index_count;
        Transient_Buffer_Indexed buffer;
    };

    void new_frame();
    void draw();

    void command_image(const Texture& texture, vec2 pos, vec2 size, float depth, Shader_Name shader = polygon_tex);

    void command_disc(vec2 position, float radius, float depth, u32 rgba, float dpix, Shader_Name shader = polygon);
    void command_disc_arc(vec2 position, vec2 arc_start, float arc_span, float depth, u32 rgba, float dpix, Shader_Name shader = polygon);

    void command_capsule(vec2 pA, vec2 pB, float radius, float depth, u32 rgba, float dpix, Shader_Name shader = polygon);

    void command_circle(vec2 position, float radius_start, float dradius, float depth, u32 rgba, float dpix, Shader_Name shader = polygon);
    void command_circle_arc(vec2 position, vec2 arc_start, float dradius, float arc_span, float depth, u32 rgba, float dpix, Shader_Name shader = polygon);

    // ---- data

    array<Command> commands;
    array<Buffer> buffers;
    array<Indexed_Buffer> indexed_buffers;
};

#endif
