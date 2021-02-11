#ifndef H_RENDERER_GL3
#define H_RENDERER_GL3

struct Transient_Buffer_GL3{
    void* ptr = nullptr;
    size_t bytesize = 0u;

    GL::Vertex_Array vao = 0u;
    GL::Buffer vbo = 0u;
};
struct Texture_GL3{
    u32 width = 0u;
    u32 height = 0u;

    Texture_Format format = TEXTURE_FORMAT_NONE;
    GL::Texture texture = 0u;
};

struct Renderer_GL3{
    void setup();
    void terminate();

    // -- resources

    Transient_Buffer_GL3 get_transient_buffer(size_t bytesize);
    void free_transient_buffer(Transient_Buffer_GL3& buffer);

    void format_transient_buffer(const Transient_Buffer_GL3& buffer, Vertex_Format_Name format);
    void checkout_transient_buffer(Transient_Buffer_GL3& buffer);
    void commit_transient_buffer(Transient_Buffer_GL3& buffer);

    Texture_GL3 get_texture(Texture_Format format, u32 witdh, u32 height, Data_Type data_type, void* data);
    void free_texture(Texture_GL3& texture);
    void update_texture(Texture_GL3& texture, u32 ox, u32 oy, u32 width, u32 height, Data_Type data_type, void* data);

    // -- state

    void use_shader(Shader_Name name);
    void update_uniform(Uniform_Name name, void* ptr);
    void setup_texture_unit(u32 texture_unit, const Texture_GL3& texture, Sampler_Name sampler_name);

    // -- draw

    void draw(const Transient_Buffer_GL3& buffer, Primitive_Type primitive, u32 index, u32 count);

    // ---- data

    struct Uniform_Entry{
        GL::Buffer buffer = 0u;
        size_t bytesize;
    };
    Uniform_Entry uniform_storage[Uniform_Name::NUMBER_OF_UNIFORM_NAMES];

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

    struct Shader_Entry{
        GL::Shader shader = 0u;
    };
    Shader_Entry shader_storage[Shader_Name::NUMBER_OF_SHADER_NAMES];
};

#endif
