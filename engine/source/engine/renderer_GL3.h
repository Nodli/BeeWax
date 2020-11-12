#ifndef H_RENDERER_GL3
#define H_RENDERER_GL3

enum Renderer_Primitive{
    PRIMITIVE_TRIANGLES = GL_TRIANGLES,
    PRIMITIVE_TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
    PRIMITIVE_LINES = GL_LINES,
    PRIMITIVE_LINE_STRIP = GL_LINE_STRIP,
    PRIMITIVE_NONE
};

enum Texture_Format{
    TEXTURE_FORMAT_R = GL_RED,
    // NOTE(hugo): GL_RGBA should be prefered because GPUs don't like 24-byte alignment
    TEXTURE_FORMAT_RGB = GL_RGB,
    TEXTURE_FORMAT_RGBA = GL_RGBA,
    TEXTURE_FORMAT_NONE
};
inline u32 texture_format_channels(Texture_Format format){
    switch(format){
        case TEXTURE_FORMAT_RGBA:
            return 4u;
        case TEXTURE_FORMAT_RGB:
            return 3u;
        case TEXTURE_FORMAT_R:
            return 1u;
        default:
            LOG_ERROR("format with value %d missing in texture_format_channels", format);
            assert(false);
            return 0u;
    }
};

typedef u32 Vertex_Batch_ID;
typedef u32 Texture_ID;
typedef u32 Sampler_ID;

struct Renderer_GL3{
    void setup_resources();
    void free_resources();

    void start_frame();
    void end_frame();

    // ---- stateful interface

    void* get_uniform(Uniform_Name name);
    void submit_uniform(Uniform_Name name);

    void use_shader(Shader_Name name);

    // NOTE(hugo):
    // same primitive and vertex format for everyone
    // pipeline state must be setup before drawing
    // !WARNING! vertices in the extensions will need to be reuploaded every time another batch is drawn
    Vertex_Batch_ID get_vertex_batch(Vertex_Format_Name name, Renderer_Primitive primitive, u32 required_capacity = 0u);
    void free_vertex_batch(Vertex_Batch_ID batch);
    void* get_vertices(Vertex_Batch_ID batch, u32 nvertices);
    void submit_vertex_batch(Vertex_Batch_ID batch);

    // NOTE(hugo): data_type & data are optional if texture data are unknown at creation
    Texture_ID get_texture(Texture_Format format, u32 witdh, u32 height, Renderer_Data_Type data_type = TYPE_UBYTE, void* data = nullptr);
    void free_texture(Texture_ID texture);
    void update_texture(Texture_ID texture, Renderer_Data_Type data_type, void* data);

    void setup_texture_unit(u32 texture_unit, Texture_ID texture, Sampler_Name sampler_name = Sampler_Name::SAMPLER_NONE);

    // ---- data

    u64 frame_count = 0u;

    // -- vertex batching

    struct Vertex_Batch_Entry{
        Vertex_Format_Name format = VERTEX_FORMAT_NONE;
        Renderer_Primitive primitive;

        size_t vbo_bytesize = 0u;
        size_t vbo_position = 0u;
        void* vbo_mapping = nullptr;

        Vertex_Format_Name vao_format = VERTEX_FORMAT_NONE;
        GL::Vertex_Array vao = 0u;
        GL::Buffer vbo = 0u;

        darray<GLint> first;
        darray<GLsizei> count;

        // ----

        darray<u8> extension_data;
        darray<GLint> extension_first;
        darray<GLsizei> extension_count;
    };
    struct {
        Vertex_Format_Name shared_format = VERTEX_FORMAT_NONE;
        GL::Vertex_Array shared_vao = 0u;
        GL::Vertex_Array shared_vbo = 0u;

        darray<u32> free_batches;
        darray<Vertex_Batch_Entry> batches;
    } vertex_batch_storage;

    // -- textures

    struct Texture_Entry{
        GL::Texture texture = 0u;
        Texture_Format format = TEXTURE_FORMAT_NONE;
        u32 width = 0u;
        u32 height = 0u;
    };
    struct{
        darray<u32> free_textures;
        darray<Texture_Entry> textures;
    } texture_storage;

    // -- static storage

    struct Uniform_Storage_Entry{
        GL::Buffer buffer = 0u;
        size_t bytesize = 0u;
    };
    Uniform_Storage_Entry uniform_storage[Uniform_Name::NUMBER_OF_UNIFORM_NAMES];

    struct Shader_Storage_Entry{
        GL::Shader shader = 0u;
    };
    Shader_Storage_Entry shader_storage[Shader_Name::NUMBER_OF_SHADER_NAMES];

    struct Vertex_Format_Entry{
        u32 number_of_attributes = 0u;
        Vertex_Format_Attribute* attributes = nullptr;
        size_t vertex_bytesize = 0u;
    };
    Vertex_Format_Entry vertex_format_storage[Vertex_Format_Name::NUMBER_OF_VERTEX_FORMAT_NAMES];

    struct Sampler_Entry{
        GL::Sampler sampler = 0u;
    };
    Sampler_Entry sampler_storage[Sampler_Name::NUMBER_OF_SAMPLER_NAMES];
};

#endif
