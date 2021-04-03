#ifndef H_EDITOR
#define H_EDITOR

enum Editor_Mode{
    MODE_DEFAULT,
    MODE_EDIT_POLYGON,
    MODE_CREATE_POLYGON,
};

struct Geometry_Cache{
    xydrgba* vertices();
    u16* indices();

    // ---- data

    void* memory = nullptr;
};
Geometry_Cache malloc_geometry_cache();
void free_geometry_cache();

struct Editor_Polygon{
    void generate_cache();

    array<vec2> vertices;
    u32 byte_color;
    array<xydrgba> cache;
};
struct Entity{
    array<Editor_Polygon> polygons;
};

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
    void transition_to_mode(Editor_Mode new_mode);

    // -- MODE_CREATE_POLYGON

    struct{
        array<vec2> vertices;
    } CREATE_POLYGON_state;

    u32 polygon_geometry_request(u32 nvertices);
    void polygon_geometry_generate(u32 nvertices, u32* vertices, xydrgba* output);

    // ---- rendering

    Camera_2D camera = {{0.f, 0.f}, 10.f, 1.f};

    // ---- storage

    Component_Storage<Entity> entity_storage;
    Virtual_Arena arena;
};

#endif
