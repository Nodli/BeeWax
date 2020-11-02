#ifndef H_GL
#define H_GL

namespace GL{

    // NOTE(hugo): typedef does not make a strong typing requirement ie a Shader is automatically cast to any GLuint typedef
    typedef GLuint Handle;
    typedef Handle Shader;
    typedef Handle Program;
    typedef Handle Texture;
    typedef Handle Framebuffer;
    typedef Handle Vertex_Array;
    typedef Handle Buffer;
    typedef Handle Sampler;

    Shader create_shader(GLenum shader_type,
            const char* const shader_code,
            GLint shader_length = -1);
    void delete_shader(Shader shader);

    Program create_program(Shader shaderA);
    Program create_program(Shader shaderA, Shader shaderB);
    Program create_program(Shader shaderA, Shader shaderB, Shader shaderC);
    // NOTE(hugo): the program owns the shaders ie they are flagged for deletion to be deleted with the program
    Program create_program(GLenum shaderA_type, const char* const shaderA_code);
    Program create_program(GLenum shaderA_type, const char* const shaderA_code,
            GLenum shaderB_type, const char* const shaderB_code);
    Program create_program(GLenum shaderA_type, const char* const shaderA_code,
            GLenum shaderB_type, const char* const shaderB_code,
            GLenum shaderC_type, const char* const shaderC_code);
    void delete_program(Program program);

    struct Program_Binary{
        GLint length;
        GLenum format;
        char* memory;
    };
    // NOTE(hugo): /!\ the caller owns the memory ie explicit free required
    Program_Binary retrieve_program_cache(Program handle);
    Program create_program_from_cache(Program_Binary cache);

    // NOTE(hugo): maybe pass the internal format as parameter
    Texture create_immutable_texture(uint width, uint height, GLenum internal_format);
    void reallocate_immutable_texture(Texture texture, uint width, uint height, GLenum internal_format);
    void delete_texture(Texture texture);

    // NOTE(hugo): usage_flags can be any combination of GL_MAP_READ_BIT, GL_MAP_WRITE_BIT, (GL_DYNAMIC_STORAGE_BIT), GL_MAP_PERSISTENT_BIT, (GL_MAP_COHERENT_BIT), (GL_CLIENT_STORAGE_BIT)
    Buffer create_immutable_buffer(size_t size, void* data = nullptr, GLbitfield usage = 0);
    void reallocate_immutable_buffer(Buffer buffer, size_t size, void* data = nullptr, GLbitfield usage = 0);
    void delete_buffer(Buffer buffer);

    void enable_depth_test();
    void disable_depth_test();
    void enable_backface_culling();
    void disable_backface_culling();

    // ---- asynchronous / synchronization ---- //

    // NOTE(hugo): /!\ the caller owns the memory ie explicit free required
    void* retrieve_texture(Texture texture, uint width, uint height);

    struct Texture_Data_Request{
        // NOTE(hugo): /!\ the caller owns the memory ie explicit free required
        //             returns nullptr when the retrieval failed
        void* try_retrieval();
        void* force_retrieval();

        size_t size;
        Buffer buffer;
        GLsync sync;
    };
    Texture_Data_Request request_texture(Texture, uint width, uint height);

    // ---- hardware detection ---- //

    struct Compute_Capabilities{
        GLint max_workgroup_count[3]; // NOTE(hugo): max number of workgroups (x, y, z) per dispatch
        GLint max_workgroup_size[3]; // NOTE(hugo): max number of invocations (x, y, z) per workgroup
        GLint max_workgroup_invocations; // NOTE(hugo): max number of invocations (x * y * z) per workgroup
    };
    Compute_Capabilities detect_compute_capabilities();

    struct UBO_Capabilities{
        GLint max_ubo_bindings;
        GLint max_ubo_bytesize;
        GLint max_vertex_ubo;
        GLint max_fragment_ubo;
        GLint max_geometry_ubo;
    };
    UBO_Capabilities detect_ubo_capabilities();

    // ---- error detection ---- //

    void display_error_translation(const GLenum error_code);
	void display_error_translation_framebuffer(const GLenum error_code);

	bool check_error(const char* const message = nullptr);
	bool check_error_shader(const GLuint shader, const char* const message = nullptr);
	bool check_error_program(const GLuint program, const char* const message = nullptr);
	bool check_error_framebuffer(const GLenum target, const char* const message = nullptr);
	bool check_error_named_framebuffer(const GLint framebuffer, const char* const message = nullptr);

    // ---- debug ---- //
    // TODO(hugo): enable only when DEBUG_BUILD is defined

    void debug_message_callback(GLenum source,
            GLenum type,
            GLuint id,
            GLenum severity,
            GLsizei length,
            const GLchar* message,
            const void* userParam);
    void set_debug_message_callback();
    void unset_debug_message_callback();
    bool check_debug_message_log();

    void push_debug_group(const char* const groupname);
    void pop_debug_group();

    // NOTE(hugo): type must be one of :
    // GL_BUFFER, GL_SHADER, GL_PROGRAM, GL_VERTEX_ARRAY, GL_QUERY, GL_PROGRAM_PIPELINE,
    // GL_TRANSFORM_FEEDBACK, GL_SAMPLER, GL_TEXTURE, GL_RENDERBUFFER, GL_FRAMEBUFFER
    void label_object(GLenum type, GLuint handle, const char* const label);
    void label_fence(GLsync fence, const char* const label);

    // NOTE(hugo): /!\ the caller owns the memory ie explicit free required
    char* get_object_label(GLenum type, GLuint handle);
    char* get_fence_label(GLsync handle);
}

#endif
