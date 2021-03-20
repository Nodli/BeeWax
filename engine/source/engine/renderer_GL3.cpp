static inline void texture_format_info(Texture_Format format,
        u32& nchan, GLenum& format_type, size_t& bytes_per_chan){
    switch(format){
        case TEXTURE_FORMAT_RGBA_BYTE:
        case TEXTURE_FORMAT_SRGBA_BYTE:
            nchan = 4u;
            format_type = GL_RGB;
            bytes_per_chan = 1u;
            break;
        case TEXTURE_FORMAT_RGB_BYTE:
        case TEXTURE_FORMAT_SRGB_BYTE:
            nchan = 3u;
            format_type = GL_RGB;
            bytes_per_chan = 1u;
            break;
        case TEXTURE_FORMAT_R_BYTE:
            nchan = 1u;
            format_type = GL_RED;
            bytes_per_chan = 1u;
            break;
        default:
            LOG_ERROR("format %d missing in texture_format_info", format);
            assert(false);
    }
};

// NOTE(hugo): alignment is 1, 2 or 4
static inline GLint compute_texture_row_alignment(u32 width, u32 height, u32 nchan, size_t bytes_per_chan){
    size_t row_bytesize = width * nchan * bytes_per_chan;
    return (GLint)min(4u, get_rightmost_set_bit(row_bytesize));
}

static void use_vertex_format(Renderer_GL3* renderer, Vertex_Format_Name format_name){
    const Renderer_GL3::Vertex_Format_Entry& format = renderer->vertex_format_storage[format_name];

    for(u32 iattribute = 0u; iattribute != format.number_of_attributes; ++iattribute){
        glEnableVertexAttribArray(iattribute);
        const Vertex_Format_Attribute& attribute = format.attributes[iattribute];
        glVertexAttribPointer(iattribute, (GLint)attribute.size, attribute.type, attribute.norm, (GLsizei)format.vertex_bytesize, (void*)attribute.offset);
    }
}

static void renderer_setup_uniform_storage(Renderer_GL3* renderer){
    UNUSED(renderer);
#define SETUP_UNIFORM_STORAGE(UNIFORM_NAME)                                 \
    {                                                                       \
        GL::Buffer buffer;                                                  \
        size_t bytesize = sizeof(CONCATENATE(uniform_, UNIFORM_NAME));      \
        glGenBuffers(1, &buffer);                                           \
        glBindBuffer(GL_UNIFORM_BUFFER, buffer);                            \
        glBufferData(GL_UNIFORM_BUFFER, bytesize, NULL, GL_STREAM_DRAW);    \
        glBindBuffer(GL_UNIFORM_BUFFER, 0u);                                \
        renderer->uniform_storage[UNIFORM_NAME].buffer = buffer;            \
        renderer->uniform_storage[UNIFORM_NAME].bytesize = bytesize;        \
    }
    FOR_EACH_UNIFORM_NAME(SETUP_UNIFORM_STORAGE)
#undef SETUP_UNIFORM_STORAGE
}
static void renderer_free_uniform_storage(Renderer_GL3* renderer){
    UNUSED(renderer);
#define FREE_UNIFORM_STORAGE(UNIFORM_NAME)                                      \
    {                                                                           \
        glDeleteBuffers(1, &renderer->uniform_storage[UNIFORM_NAME].buffer);    \
    }
    FOR_EACH_UNIFORM_NAME(FREE_UNIFORM_STORAGE)
#undef FREE_UNIFORM_STORAGE
}

static void renderer_setup_shader_storage(Renderer_GL3* renderer){
    UNUSED(renderer);
#define SETUP_SHADER_STORAGE(SHADER_NAME)                                                                                                                           \
    {                                                                                                                                                               \
        const char* shader_header = CONCATENATE(shader_header_, SHADER_NAME);                                                                                       \
        const char* vertex_shader = CONCATENATE(vertex_shader_, SHADER_NAME);                                                                                       \
        const char* fragment_shader = CONCATENATE(fragment_shader_, SHADER_NAME);                                                                                   \
        renderer->shader_storage[SHADER_NAME].shader = GL::create_program(GL_VERTEX_SHADER, vertex_shader, GL_FRAGMENT_SHADER, fragment_shader, shader_header);     \
    }
    FOR_EACH_SHADER_NAME(SETUP_SHADER_STORAGE)
#undef SETUP_SHADER_STORAGE
}
static void renderer_free_shader_storage(Renderer_GL3* renderer){
    UNUSED(renderer);
#define FREE_SHADER_STORAGE(SHADER_NAME)                                    \
    {                                                                       \
        GL::delete_program(renderer->shader_storage[SHADER_NAME].shader);   \
    }
    FOR_EACH_SHADER_NAME(FREE_SHADER_STORAGE)
#undef FREE_SHADER_STORAGE
}

static void renderer_setup_uniform_shader_binding(Renderer_GL3* renderer){
    UNUSED(renderer);
    u32 uniform_binding_counter = 0u;

    // NOTE(hugo): ignore warning when no uniform / shader binding is declared
    UNUSED(uniform_binding_counter);

#define SETUP_UNIFORM_SHADER_BINDING(UNIFORM_NAME, SHADER_NAME)                                                                                         \
    {                                                                                                                                                   \
        GLuint index_in_shader = glGetUniformBlockIndex(renderer->shader_storage[SHADER_NAME].shader, STRINGIFY(CONCATENATE(u_, UNIFORM_NAME)));        \
        ENGINE_CHECK(index_in_shader != GL_INVALID_INDEX, "no uniform binding found uniform: %s shader: %s", STRINGIFY(uniform), STRINGIFY(shader));    \
        glUniformBlockBinding(renderer->shader_storage[SHADER_NAME].shader, index_in_shader, uniform_binding_counter);                                  \
        glBindBufferBase(GL_UNIFORM_BUFFER, uniform_binding_counter, renderer->uniform_storage[UNIFORM_NAME].buffer);                                   \
        ++uniform_binding_counter;                                                                                                                      \
    }
    FOR_EACH_UNIFORM_SHADER_PAIR(SETUP_UNIFORM_SHADER_BINDING)
#undef SETUP_UNIFORM_SHADER_BINDING
}
static void renderer_free_uniform_shader_binding(Renderer_GL3* renderer){
    UNUSED(renderer);
}

static void renderer_setup_texture_shader_binding(Renderer_GL3* renderer){
    UNUSED(renderer);
#define SETUP_TEXTURE_SHADER_BINDING(TEXTURE_NAME, TEXTURE_UNIT, SHADER_NAME)                                                   \
    {                                                                                                                           \
        GLint index_in_shader = glGetUniformLocation(renderer->shader_storage[SHADER_NAME].shader, STRINGIFY(TEXTURE_NAME));    \
        assert(index_in_shader != GL_INVALID_INDEX);                                                                                                \
        glUseProgram(renderer->shader_storage[SHADER_NAME].shader);                                                             \
        glUniform1i(index_in_shader, TEXTURE_UNIT);                                                                             \
    }
    FOR_EACH_TEXTURE_SHADER_PAIR(SETUP_TEXTURE_SHADER_BINDING)
#undef SETUP_TEXTURE_SHADER_BINDING
}
static void renderer_free_texture_shader_binding(Renderer_GL3* renderer){
    UNUSED(renderer);
}

static void renderer_setup_vertex_format_storage(Renderer_GL3* renderer){
    UNUSED(renderer);
#define SETUP_VERTEX_FORMAT_STORAGE(VERTEX_FORMAT_NAME)                                                                                                         \
    {                                                                                                                                                           \
        renderer->vertex_format_storage[VERTEX_FORMAT_NAME].number_of_attributes = carray_size(CONCATENATE(vertex_format_attributes_, VERTEX_FORMAT_NAME));     \
        renderer->vertex_format_storage[VERTEX_FORMAT_NAME].attributes = CONCATENATE(vertex_format_attributes_, VERTEX_FORMAT_NAME);                            \
        renderer->vertex_format_storage[VERTEX_FORMAT_NAME].vertex_bytesize = sizeof(CONCATENATE(vertex_, VERTEX_FORMAT_NAME));                                 \
    }
    FOR_EACH_VERTEX_FORMAT_NAME(SETUP_VERTEX_FORMAT_STORAGE)
#undef SETUP_VERTEX_FORMAT_STORAGE
}
static void renderer_free_vertex_format_storage(Renderer_GL3* renderer){
    UNUSED(renderer);
}

static void renderer_setup_sampler_storage(Renderer_GL3* renderer){
    UNUSED(renderer);

    // NOTE(hugo): unused SamplerParameter
    //glSamplerParameterf(sampler, GL_TEXTURE_MIN_LOD, MIN_LOD);
    //glSamplerParameterf(sampler, GL_TEXTURE_MAX_LOD, MAX_LOD);

    //float temporary_border_color = BORDER_COLOR;
    //glSamplerParameterfv(sampler, GL_TEXTURE_BORDER_COLOR, &temporary_border_color);

    //glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_MODE, COMPARE_MODE);
    //glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_FUNCTION, COMPARE_FUNCTION);

#define SETUP_SAMPLER_STORAGE(SAMPLER_NAME, MIN_FILTER, MAG_FILTER, WRAP_S, WRAP_T, WRAP_R) \
    {                                                                                       \
        GL::Sampler& sampler = renderer->sampler_storage[SAMPLER_NAME].sampler;             \
        glGenSamplers(1u, &sampler);                                                        \
                                                                                            \
        static_assert(MIN_FILTER == FILTER_DEFAULT                                          \
                    || MIN_FILTER == FILTER_NEAREST                                         \
                    || MIN_FILTER == FILTER_LINEAR                                          \
                    || MIN_FILTER == FILTER_NEAREST_MIPMAP_NEAREST                          \
                    || MIN_FILTER == FILTER_LINEAR_MIPMAP_NEAREST                           \
                    || MIN_FILTER == FILTER_NEAREST_MIPMAP_LINEAR                           \
                    || MIN_FILTER == FILTER_LINEAR_MIPMAP_LINEAR);                          \
        if constexpr (MIN_FILTER != FILTER_DEFAULT)                                         \
            glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, MIN_FILTER);                \
                                                                                            \
        static_assert(MAG_FILTER == FILTER_DEFAULT                                          \
                    || MAG_FILTER == FILTER_NEAREST                                         \
                    || MAG_FILTER == FILTER_LINEAR);                                        \
        if constexpr (MAG_FILTER != FILTER_DEFAULT)                                         \
            glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, MAG_FILTER);                \
                                                                                            \
                                                                                            \
        static_assert(WRAP_S == WRAP_DEFAULT                                                \
                    || WRAP_S == WRAP_CLAMP                                                 \
                    || WRAP_S == WRAP_MIRRORED_REPEAT                                       \
                    || WRAP_S == WRAP_REPEAT);                                              \
        if constexpr(WRAP_S != WRAP_DEFAULT)                                                \
            glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, WRAP_S);                        \
                                                                                            \
        static_assert(WRAP_T == WRAP_DEFAULT                                                \
                    || WRAP_T == WRAP_CLAMP                                                 \
                    || WRAP_T == WRAP_MIRRORED_REPEAT                                       \
                    || WRAP_T == WRAP_REPEAT);                                              \
        if constexpr(WRAP_T != WRAP_DEFAULT)                                                \
            glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, WRAP_T);                        \
                                                                                            \
        static_assert(WRAP_R == WRAP_DEFAULT                                                \
                    || WRAP_R == WRAP_CLAMP                                                 \
                    || WRAP_R == WRAP_MIRRORED_REPEAT                                       \
                    || WRAP_R == WRAP_REPEAT);                                              \
        if constexpr(WRAP_R != WRAP_DEFAULT)                                                \
            glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, WRAP_R);                        \
    }
    FOR_EACH_SAMPLER_NAME(SETUP_SAMPLER_STORAGE)
#undef SETUP_SAMPLER_STORAGE
}

static void renderer_free_sampler_storage(Renderer_GL3* renderer){
    UNUSED(renderer);
#define FREE_SAMPLER_STORAGE(SAMPLER_NAME, MIN_FILTER, MAG_FILTER, WRAP_S, WRAP_T, WRAP_R)  \
    {                                                                                       \
        GL::Sampler& sampler = renderer->sampler_storage[SAMPLER_NAME].sampler;             \
        glDeleteSamplers(1u, &sampler);                                                     \
    }
    FOR_EACH_SAMPLER_NAME(FREE_SAMPLER_STORAGE)
#undef FREE_SAMPLER_STORAGE
}

void Renderer_GL3::setup(){
    renderer_setup_uniform_storage(this);
    renderer_setup_shader_storage(this);
    renderer_setup_vertex_format_storage(this);
    renderer_setup_sampler_storage(this);

    renderer_setup_uniform_shader_binding(this);
    renderer_setup_texture_shader_binding(this);

    glGenVertexArrays(1u, &empty_vao);
}

void Renderer_GL3::terminate(){
    renderer_free_texture_shader_binding(this);
    renderer_free_uniform_shader_binding(this);

    renderer_free_sampler_storage(this);
    renderer_free_vertex_format_storage(this);
    renderer_free_shader_storage(this);
    renderer_free_uniform_storage(this);

    glDeleteVertexArrays(1u, &empty_vao);

    *this = Renderer_GL3();
}

// -- resources

Transient_Buffer_GL3 Renderer_GL3::get_transient_buffer(size_t bytesize){
    Transient_Buffer_GL3 buffer;
    glGenVertexArrays(1, &buffer.vao);
    glGenBuffers(1, &buffer.vbo);

    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)bytesize, NULL, GL_STREAM_DRAW);
    buffer.bytesize = bytesize;
    glBindBuffer(GL_ARRAY_BUFFER, 0u);

    return buffer;
}

Transient_Buffer_Indexed_GL3 Renderer_GL3::get_transient_buffer_indexed(size_t vbytesize, size_t ibytesize){
    Transient_Buffer_Indexed_GL3 buffer;
    glGenVertexArrays(1, &buffer.vao);
    glGenBuffers(1, &buffer.vbo);
    glGenBuffers(1, &buffer.ibo);

    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)vbytesize, NULL, GL_STREAM_DRAW);
    buffer.vbytesize = vbytesize;
    glBindBuffer(GL_ARRAY_BUFFER, 0u);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)ibytesize, NULL, GL_STREAM_DRAW);
    buffer.ibytesize = ibytesize;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

    return buffer;
}

void Renderer_GL3::free_transient_buffer(Transient_Buffer_GL3& buffer){
    if(buffer.ptr){
        glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, 0u);
    }

    glDeleteVertexArrays(1u, &buffer.vao);
    glDeleteBuffers(1u, &buffer.vbo);

    buffer = Transient_Buffer_GL3();
}

void Renderer_GL3::free_transient_buffer(Transient_Buffer_Indexed_GL3& buffer){
    if(buffer.vptr){
        assert(buffer.iptr);
        glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, 0u);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.ibo);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, 0u);
    }

    glDeleteVertexArrays(1u, &buffer.vao);
    glDeleteBuffers(1u, &buffer.vbo);
    glDeleteBuffers(1u, &buffer.ibo);

    buffer = Transient_Buffer_Indexed_GL3();
}

void Renderer_GL3::format(const Transient_Buffer_GL3& buffer, Vertex_Format_Name format){
    glBindVertexArray(buffer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);

    use_vertex_format(this, format);

    glBindVertexArray(0u);
    glBindBuffer(GL_ARRAY_BUFFER, 0u);
}

void Renderer_GL3::format(const Transient_Buffer_Indexed_GL3& buffer, Vertex_Format_Name format){
    glBindVertexArray(buffer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.ibo);

    use_vertex_format(this, format);

    glBindVertexArray(0u);
    glBindBuffer(GL_ARRAY_BUFFER, 0u);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);
}

// NOTE(hugo):
// * orphaning the buffer with glBufferData because it does not work with GL_WRITE_ONLY or GL_MAP_WRITE_BIT & GL_MAP_INVALIDATE_RANGE_BIT & GL_MAP_INVALIDATE_BUFFER_BIT
// * using glMapBuffer instead of glMapBufferRange because we don't need the invalidation flag anymore

void Renderer_GL3::checkout(Transient_Buffer_GL3& buffer){
    assert(buffer.ptr == nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)buffer.bytesize, NULL, GL_STREAM_DRAW);
    buffer.ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    //buffer.ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0u, buffer.bytesize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, 0u);

    assert(buffer.ptr != nullptr);
}

void Renderer_GL3::checkout(Transient_Buffer_Indexed_GL3& buffer){
    assert(buffer.vptr == nullptr && buffer.iptr == nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)buffer.vbytesize, NULL, GL_STREAM_DRAW);
    buffer.vptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    //buffer.vptr = glMapBufferRange(GL_ARRAY_BUFFER, 0u, buffer.vbytesize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, 0u);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)buffer.ibytesize, NULL, GL_STREAM_DRAW);
    buffer.iptr = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
    //buffer.iptr = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0u, buffer.ibytesize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

    assert(buffer.vptr != nullptr && buffer.iptr != nullptr);
}

void Renderer_GL3::commit(Transient_Buffer_GL3& buffer){
    assert(buffer.ptr != nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0u);

    buffer.ptr = nullptr;
}

void Renderer_GL3::commit(Transient_Buffer_Indexed_GL3& buffer){
    assert(buffer.vptr != nullptr && buffer.iptr != nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0u);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.ibo);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

    buffer.vptr = nullptr;
    buffer.iptr = nullptr;
}

Texture_GL3 Renderer_GL3::get_texture(Texture_Format format, u32 width, u32 height, Data_Type data_type, void* data){
    Texture_GL3 texture;
    texture.width = width;
    texture.height = height;

    glGenTextures(1u, &texture.texture);

    glBindTexture(GL_TEXTURE_2D, texture.texture);

    u32 nchan;
    GLenum format_type;
    size_t bytes_per_chan;
    texture_format_info(format, nchan, format_type, bytes_per_chan);

    // NOTE(hugo): set the unpacking alignment for this operation when necessary
    GLint row_alignment = compute_texture_row_alignment(width, height, nchan, bytes_per_chan);
    if(row_alignment != GL::default_unpack_alignment){
        glPixelStorei(GL_UNPACK_ALIGNMENT, row_alignment);
        glTexImage2D(GL_TEXTURE_2D, 0, format, (GLsizei)width, (GLsizei)height, 0, format_type, data_type, data);
        glPixelStorei(GL_UNPACK_ALIGNMENT, GL::default_unpack_alignment);
    }else{
        glTexImage2D(GL_TEXTURE_2D, 0, format, (GLsizei)width, (GLsizei)height, 0, format_type, data_type, data);
    }

    // NOTE(hugo): MIP mapping setup is required
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    glBindTexture(GL_TEXTURE_2D, 0u);

    return texture;
}

void Renderer_GL3::free_texture(Texture_GL3& texture){
    glDeleteTextures(1u, &texture.texture);

    texture = Texture_GL3();
}

void Renderer_GL3::update_texture(Texture_GL3& texture, u32 ox, u32 oy, u32 width, u32 height, Data_Type data_type, void* data){
    assert(!((ox + width) > texture.width) && !((oy + height) > texture.height));
    glBindTexture(GL_TEXTURE_2D, texture.texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0u, ox, oy, width, height, texture.format, data_type, data);
}

Render_Target_GL3 Renderer_GL3::get_render_target(u32 width, u32 height){
    Render_Target_GL3 render_target;

    render_target.width = width;
    render_target.height = height;

    glGenRenderbuffers(2u, render_target.buffers);
    glBindRenderbuffer(GL_RENDERBUFFER, render_target.buffer_color);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_SRGB8_ALPHA8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, render_target.buffer_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0u);

    glGenFramebuffers(1u, &render_target.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, render_target.framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, render_target.buffer_color);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, render_target.buffer_depth);
    glBindFramebuffer(GL_FRAMEBUFFER, 0u);

    return render_target;
}

void Renderer_GL3::free_render_target(Render_Target_GL3& render_target){
    glDeleteRenderbuffers(2u, render_target.buffers);
    glDeleteFramebuffers(1u, &render_target.framebuffer);
}

// -- state

void Renderer_GL3::use_shader(Shader_Name name){
    glUseProgram(shader_storage[name].shader);
}

void Renderer_GL3::update_uniform(Uniform_Name name, void* ptr){
    glBindBuffer(GL_UNIFORM_BUFFER, uniform_storage[name].buffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0u, uniform_storage[name].bytesize, ptr);
    glBindBuffer(GL_UNIFORM_BUFFER, 0u);
}

void Renderer_GL3::setup_texture_unit(u32 texture_unit, const Texture_GL3& texture, Sampler_Name sampler){
    // NOTE(hugo): bind locations are contiguous values
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindSampler(texture_unit, sampler_storage[sampler].sampler);
    glBindTexture(GL_TEXTURE_2D, texture.texture);
}

void Renderer_GL3::use_render_target(const Render_Target_GL3& render_target){
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, render_target.framebuffer);
    glViewport(0u, 0u, render_target.width, render_target.height);
}

void Renderer_GL3::set_depth_test(const Depth_Test_Type type){
    switch(type){
        case DEPTH_TEST_NONE:
            glDisable(GL_DEPTH_TEST);
            break;
        case DEPTH_TEST_LESS:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            break;
        case DEPTH_TEST_LESS_EQUAL:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            break;
        case DEPTH_TEST_EQUAL:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_EQUAL);
            break;
        case DEPTH_TEST_NOT_EQUAL:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_NOTEQUAL);
            break;
        case DEPTH_TEST_GREATER_EQUAL:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_GEQUAL);
            break;
        case DEPTH_TEST_GREATER:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_GREATER);
            break;
    }
}

// -- draw

void Renderer_GL3::draw(Primitive_Type primitive, u32 index, u32 count){
    glBindVertexArray(empty_vao);
    glDrawArrays(primitive, index, count);
    glBindVertexArray(0u);
}

void Renderer_GL3::draw(const Transient_Buffer_GL3& buffer, Primitive_Type primitive, u32 index, u32 count){
    glBindVertexArray(buffer.vao);
    glDrawArrays(primitive, index, count);
    glBindVertexArray(0u);
}

void Renderer_GL3::draw(const Transient_Buffer_Indexed_GL3& buffer, Primitive_Type primitive, Data_Type index_type, u32 count, u64 offset){
    glBindVertexArray(buffer.vao);
    glDrawElements(primitive, count, index_type, (const void*)offset);
    glBindVertexArray(0u);
}

void Renderer_GL3::clear_render_target(const Render_Target_GL3& render_target){
    // NOTE(hugo): glUseProgram(0u) otherwise the glClear triggers a vertex shader recompilation on Nvidia GPUs
    glUseProgram(0u);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer_GL3::copy_render_target(const Render_Target_GL3& source, const Render_Target_GL3& destination){
    glBindFramebuffer(GL_READ_FRAMEBUFFER, source.framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destination.framebuffer);
    glViewport(0u, 0u, destination.width, destination.height);
    glBlitFramebuffer(0u, 0u, source.width, source.height, 0u, 0u, destination.width, destination.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}
