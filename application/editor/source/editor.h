#ifndef H_EDITOR
#define H_EDITOR

enum Editor_Mode{
    MODE_DEFAULT,
    MODE_EDIT_POLYGON,
    MODE_CREATE_POLYGON,
};

struct Storage_Polygon{
    array<vec2> vertices = {};
    u32 byte_color = {};
    Static_Buffer_Indexed buffer = {};
};
struct Entity{
    array<Storage_Polygon> polygons = {};
};

// NOTE(hugo):
// vertex capacity = nvertices
// index  capacity = 3 * (nvertices - 2)
void polygon_bytesize(u32 nvertices, u32& nindices, size_t& vbytesize, size_t& ibytesize);
void polygon_generate(u32 nvertices, vec2* vertices, u32 color_rgba, u32 base_index, vertex_xydrgba* out_vertices, u32* out_indices, Virtual_Arena& arena);

struct Editor{
    void initialize();
    void terminate();

    void update_state();
    void update_mode();
    void update_common();
    void update();

    void render_storage();
    void render_mode();
    void render_ui();
    void render();

    // ---- editor state

    struct{
        vec2 mouse_screen = {0.f, 0.f};
        vec2 mouse_world = {0.f, 0.f};
        vec4 color = {.5f, .5f, .5f, 1.f};
        Editor_Mode mode = MODE_DEFAULT;
    } state;
    void initialize_mode(Editor_Mode mode);
    void terminate_mode(Editor_Mode mode);
    void transition_to_mode(Editor_Mode new_mode);

    // -- MODE_CREATE_POLYGON

    struct{
        // NOTE(hugo): staging vertex (mouse position) stored at vertices[0u]
        array<vec2> vertices = {};
        Transient_Buffer_Indexed buffer = {};
    } CREATE_POLYGON_state;

    // ---- rendering

    Camera_2D camera = {{0.f, 0.f}, 10.f, 1.f};

    // ---- storage

    Component_Storage<Entity> entity_storage;
    Virtual_Arena arena;
};

#endif
