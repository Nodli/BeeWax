#ifndef H_IMDRAWER
#define H_IMDRAWER

#include <vector>

struct ImDrawer{
    enum Command_Type{
        LINE,
        POLYGON,
        POLYGON_TEXTURED,
        NUMBER_OF_COMMAND_TYPES,
        NONE = NUMBER_OF_COMMAND_TYPES
    };
    struct Command_Polygon{
        u32 buffer_index;
        u32 vertex_index;
        u32 vertex_count;
    };
    struct Command_Line : Command_Polygon{};
    struct Command_Polygon_Textured{
        Texture texture;
        u32 buffer_index;
        u32 vertex_index;
        u32 vertex_count;
    };
    struct Command{
        Command_Type type;
        union{
            Command_Line line;
            Command_Polygon polygon;
            Command_Polygon_Textured polygon_textured;
        };
    };
    struct Buffer{
        Vertex_Format_Name vertex_format_name;
        Transient_Buffer buffer;
        size_t vertex_count;
    };

    ImDrawer();
    ~ImDrawer();

    void new_frame();
    void draw();

    void command_sprite(const Texture_Asset* asset, vec2 pos, vec2 size, float depth);
    void command_quad(vec2 BL, vec2 BR, vec2 UR, vec2 UL, float depth, u32 rgba);
    void command_line(vec2 A, vec2 B, float depth, u32 rgba);

    // ---- data

    std::vector<Command> commands;
    std::vector<Buffer> buffers;
};

#endif
