static inline u32 texture_format_channels(Texture_Format format){
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
static inline size_t data_type_bytes(Data_Type type){
    switch(type){
        case TYPE_FLOAT:
            return 4u;
        case TYPE_UBYTE:
            return 1u;
        default:
            LOG_ERROR("type with value %d missing in data_type_bytes", type);
            assert(false);
            return 0u;
    }
}

// NOTE(hugo): alignment is 1, 2 or 4
static inline GLint compute_texture_row_alignment(u32 width, u32 height, Texture_Format format, Data_Type type){
    size_t row_bytesize = width * texture_format_channels(format) * (GLint)data_type_bytes(type);
    return (GLint)min(4u, get_rightmost_set_bit(row_bytesize));
}

static void use_vertex_format(Renderer_GL3* renderer, Vertex_Format_Name format_name){
    const Renderer_GL3::Vertex_Format_Entry& format = renderer->vertex_format_storage[format_name];

    for(u32 iattribute = 0u; iattribute != format.number_of_attributes; ++iattribute){
        glEnableVertexAttribArray(iattribute);
        const Vertex_Format_Attribute& attribute = format.attributes[iattribute];
        glVertexAttribPointer(iattribute, (GLint)attribute.size, attribute.type, GL_FALSE, (GLsizei)format.vertex_bytesize, (void*)attribute.offset);
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

#if 0
static void renderer_setup_vertex_batch_storage(Renderer_GL3* renderer){
    glGenVertexArrays(1u, &renderer->vertex_batch_storage.shared_vao);
    glGenBuffers(1u, &renderer->vertex_batch_storage.shared_vbo);
}
static void renderer_free_vertex_batch_storage(Renderer_GL3* renderer){
    glDeleteVertexArrays(1u, &renderer->vertex_batch_storage.shared_vao);
    glDeleteBuffers(1u, &renderer->vertex_batch_storage.shared_vbo);

    for(u32 ibatch = 0u; ibatch != renderer->vertex_batch_storage.batches.size; ++ibatch){
        Renderer_GL3::Vertex_Batch_Entry* entry = &renderer->vertex_batch_storage.batches[ibatch];
        glDeleteVertexArrays(1u, &entry->vao);
        glDeleteBuffers(1u, &entry->vbo);
        entry->first.free();
        entry->count.free();
        entry->extension_data.free();
        entry->extension_first.free();
        entry->extension_count.free();
    }
    renderer->vertex_batch_storage.free_batches.free();
    renderer->vertex_batch_storage.batches.free();
}

static void renderer_setup_texture_storage(Renderer_GL3* renderer){
    UNUSED(renderer);
}
static void renderer_free_texture_storage(Renderer_GL3* renderer){
    for(u32 itexture = 0u; itexture != renderer->texture_storage.textures.size; ++itexture){
        Renderer_GL3::Texture_Entry* entry = &renderer->texture_storage.textures[itexture];
        glDeleteTextures(1u, &entry->texture);
    }
    renderer->texture_storage.free_textures.free();
    renderer->texture_storage.textures.free();
}
#endif

void Renderer_GL3::setup(){
    renderer_setup_uniform_storage(this);
    renderer_setup_shader_storage(this);
    renderer_setup_vertex_format_storage(this);
    renderer_setup_sampler_storage(this);

    renderer_setup_uniform_shader_binding(this);
    renderer_setup_texture_shader_binding(this);
}

void Renderer_GL3::terminate(){
    renderer_free_texture_shader_binding(this);
    renderer_free_uniform_shader_binding(this);

    renderer_free_sampler_storage(this);
    renderer_free_vertex_format_storage(this);
    renderer_free_shader_storage(this);
    renderer_free_uniform_storage(this);

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

void Renderer_GL3::format_transient_buffer(const Transient_Buffer_GL3& buffer, Vertex_Format_Name format){
    glBindVertexArray(buffer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);

    use_vertex_format(this, format);

    glBindBuffer(GL_ARRAY_BUFFER, 0u);
    glBindVertexArray(0u);
}

void Renderer_GL3::commit_transient_buffer(Transient_Buffer_GL3& buffer){
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0u);

    buffer.ptr = nullptr;
}

void Renderer_GL3::checkout_transient_buffer(Transient_Buffer_GL3& buffer){
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
    buffer.ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0u, buffer.bytesize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, 0u);
}

Texture_GL3 Renderer_GL3::get_texture(Texture_Format format, u32 width, u32 height, Data_Type data_type, void* data){
    Texture_GL3 texture;
    texture.width = width;
    texture.height = height;

    glGenTextures(1u, &texture.texture);

    glBindTexture(GL_TEXTURE_2D, texture.texture);

    // NOTE(hugo): set the unpacking alignment for this operation when necessary
    GLint row_alignment = compute_texture_row_alignment(width, height, format, data_type);
    if(row_alignment != GL::default_unpack_alignment){
        glPixelStorei(GL_UNPACK_ALIGNMENT, row_alignment);
        glTexImage2D(GL_TEXTURE_2D, 0, format, (GLsizei)width, (GLsizei)height, 0, format, data_type, data);
        glPixelStorei(GL_UNPACK_ALIGNMENT, GL::default_unpack_alignment);
    }else{
        glTexImage2D(GL_TEXTURE_2D, 0, format, (GLsizei)width, (GLsizei)height, 0, format, data_type, data);
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

void Renderer_GL3::update_texture(Texture_GL3& texture, u32 ox, u32 oy, u32 width, u32 height, Data_Type data_type, void* data){
    assert(!((ox + width) > texture.width) && !((oy + height) > texture.height));
    glBindTexture(GL_TEXTURE_2D, texture.texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0u, ox, oy, width, height, texture.format, data_type, data);
}

// -- draw

void Renderer_GL3::draw(const Transient_Buffer_GL3& buffer, Primitive_Type primitive, u32 index, u32 count){
    glBindVertexArray(buffer.vao);
    glDrawArrays(primitive, index, count);
    glBindVertexArray(0u);
}
