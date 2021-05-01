#ifndef H_RENDERER_GL3
#define H_RENDERER_GL3

struct Buffer_GL3{
    void* ptr = nullptr;
    size_t bytesize = 0u;

    GL::Vertex_Array vao = 0u;
    GL::Buffer vbo = 0u;
};
struct Transient_Buffer_GL3 : Buffer_GL3 {};

struct Buffer_Indexed_GL3{
    void* vptr = nullptr;
    size_t vbytesize = 0u;
    void* iptr = nullptr;
    size_t ibytesize = 0u;

    GL::Vertex_Array vao = 0u;
    GL::Buffer vbo = 0u;
    GL::Buffer ibo = 0u;
};
struct Transient_Buffer_Indexed_GL3 : Buffer_Indexed_GL3 {};

struct Texture_GL3{
    u32 width = 0u;
    u32 height = 0u;

    Texture_Format format = TEXTURE_FORMAT_NONE;
    GL::Texture texture = 0u;
};

struct Render_Target_GL3{
    u32 width;
    u32 height;

    GL::Framebuffer framebuffer;
    union{
        GL::Renderbuffer buffers[2u];
        struct{
            GL::Renderbuffer buffer_color;
            GL::Renderbuffer buffer_depth;
        };
    };
};

struct Render_Layer_GL3{
    void create();
    void destroy();

    // -- resources

    Buffer_GL3 get_buffer(size_t bytesize);
    void free_buffer(Buffer_GL3& buffer);
    void format(const Buffer_GL3& buffer, Vertex_Format_Name format);
    void checkout(Buffer_GL3& buffer);
    void commit(Buffer_GL3& buffer);

    Buffer_Indexed_GL3 get_buffer_indexed(size_t vbytesize, size_t ibytesize);
    void free_buffer(Buffer_Indexed_GL3& buffer);
    void format(const Buffer_Indexed_GL3& buffer, Vertex_Format_Name format);
    void checkout(Buffer_Indexed_GL3& buffer);
    void commit(Buffer_Indexed_GL3& buffer);

    Transient_Buffer_GL3 get_transient_buffer(size_t bytesize);
    void free_buffer(Transient_Buffer_GL3& buffer);
    void format(const Transient_Buffer_GL3& buffer, Vertex_Format_Name format);
    void checkout(Transient_Buffer_GL3& buffer);
    void commit(Transient_Buffer_GL3& buffer);

    Transient_Buffer_Indexed_GL3 get_transient_buffer_indexed(size_t vbytesize, size_t ibytesize);
    void free_buffer(Transient_Buffer_Indexed_GL3& buffer);
    void format(const Transient_Buffer_Indexed_GL3& buffer, Vertex_Format_Name format);
    void checkout(Transient_Buffer_Indexed_GL3& buffer);
    void commit(Transient_Buffer_Indexed_GL3& buffer);


    Texture_GL3 get_texture(Texture_Format format, u32 witdh, u32 height, Data_Type data_type, void* data);
    void free_texture(Texture_GL3& texture);
    void update_texture(Texture_GL3& texture, u32 ox, u32 oy, u32 width, u32 height, Data_Type data_type, void* data);

    Render_Target_GL3 get_render_target(u32 width, u32 height);
    Render_Target_GL3 get_render_target_multisample(u32 width, u32 height, u32 samples);
    void free_render_target(Render_Target_GL3& render_target);

    // -- state

    void use_shader(Shader_Name name);
    void update_uniform(Uniform_Name name, void* ptr);
    void setup_texture_unit(u32 texture_unit, const Texture_GL3& texture, Sampler_Name sampler_name);
    void use_render_target(const Render_Target_GL3& render_target);
    void set_depth_test(const Depth_Test_Type type);

    // -- commands

    void draw(Primitive_Type primitive, u32 index, u32 count);
    void draw(const Transient_Buffer_GL3& buffer, Primitive_Type primitive, u32 index, u32 count);
    void draw(const Buffer_GL3& buffer, Primitive_Type primitive, u32 index, u32 count);
    void draw(const Transient_Buffer_Indexed_GL3& buffer, Primitive_Type primitive, Data_Type index_type, u32 count, u64 offset);
    void draw(const Buffer_Indexed_GL3& buffer, Primitive_Type primitive, Data_Type index_type, u32 count, u64 offset);

    void clear_render_target(const Render_Target_GL3& render_target);
    void copy_render_target(const Render_Target_GL3& source, const Render_Target_GL3& destination);

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

    GL::Vertex_Array empty_vao;
};

#endif
