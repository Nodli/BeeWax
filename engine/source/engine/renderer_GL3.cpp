static void renderer_setup_uniform_storage(Renderer_GL3* renderer){
    UNUSED(renderer);
#define SETUP_UNIFORM_STORAGE(UNIFORM_NAME)                                                                 \
    {                                                                                                       \
        glGenBuffers(1, &renderer->uniform_storage[UNIFORM_NAME].buffer);                                   \
        renderer->uniform_storage[UNIFORM_NAME].bytesize = sizeof(CONCATENATE(uniform_, UNIFORM_NAME));     \
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
#define SETUP_SHADER_STORAGE(SHADER_NAME)                                                                                                           \
    {                                                                                                                                               \
        const char* vertex_shader = CONCATENATE(vertex_shader_, SHADER_NAME);                                                                       \
        const char* fragment_shader = CONCATENATE(fragment_shader_, SHADER_NAME);                                                                   \
        renderer->shader_storage[SHADER_NAME].shader = GL::create_program(GL_VERTEX_SHADER, vertex_shader, GL_FRAGMENT_SHADER, fragment_shader);    \
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

#define SETUP_UNIFORM_SHADER_BINDING(UNIFORM_NAME, SHADER_NAME)                                                                                     \
    {                                                                                                                                               \
        GLuint index_in_shader = glGetUniformBlockIndex(renderer->shader_storage[SHADER_NAME].shader, STRINGIFY(CONCATENATE(u_, UNIFORM_NAME)));    \
        assert(index_in_shader != GL_INVALID_INDEX);                                                                                                \
        glUniformBlockBinding(renderer->shader_storage[SHADER_NAME].shader, index_in_shader, uniform_binding_counter);                              \
        glBindBufferBase(GL_UNIFORM_BUFFER, uniform_binding_counter, renderer->uniform_storage[UNIFORM_NAME].buffer);                               \
        ++uniform_binding_counter;                                                                                                                  \
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
#define SETUP_VERTEX_FORMAT_STORAGE(VERTEX_FORMAT_NAME)                                                                                                             \
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

void Renderer_GL3::setup_resources(){
    renderer_setup_uniform_storage(this);
    renderer_setup_shader_storage(this);
    renderer_setup_vertex_format_storage(this);
    renderer_setup_sampler_storage(this);

    renderer_setup_uniform_shader_binding(this);
    renderer_setup_texture_shader_binding(this);

    renderer_setup_vertex_batch_storage(this);
    renderer_setup_texture_storage(this);
}

void Renderer_GL3::free_resources(){
    renderer_free_texture_storage(this);
    renderer_free_vertex_batch_storage(this);

    renderer_free_texture_shader_binding(this);
    renderer_free_uniform_shader_binding(this);

    renderer_free_sampler_storage(this);
    renderer_free_vertex_format_storage(this);
    renderer_free_shader_storage(this);
    renderer_free_uniform_storage(this);

    *this = Renderer_GL3();
}

void Renderer_GL3::start_frame(){
    assert(vertex_batch_storage.free_batches.size == vertex_batch_storage.batches.size);
}

void Renderer_GL3::end_frame(){
}

void* Renderer_GL3::get_uniform(Uniform_Name name){
    glBindBuffer(GL_UNIFORM_BUFFER, uniform_storage[name].buffer);
    glBufferData(GL_UNIFORM_BUFFER, (GLsizeiptr)uniform_storage[name].bytesize, NULL, GL_STREAM_DRAW);
    void* ptr = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
    glBindBuffer(GL_UNIFORM_BUFFER, 0u);
    return ptr;
}

void Renderer_GL3::submit_uniform(Uniform_Name name){
    glBindBuffer(GL_UNIFORM_BUFFER, uniform_storage[name].buffer);
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0u);
}

void Renderer_GL3::use_shader(Shader_Name name){
    glUseProgram(shader_storage[name].shader);
}

static void use_vertex_format(Renderer_GL3* renderer, Vertex_Format_Name name){
    Renderer_GL3::Vertex_Format_Entry* entry = &(renderer->vertex_format_storage[name]);
    for(u32 iattribute = 0u; iattribute != entry->number_of_attributes; ++iattribute){
        glEnableVertexAttribArray(iattribute);
        Vertex_Format_Attribute* attribute = entry->attributes + iattribute;
        glVertexAttribPointer(iattribute, (GLint)attribute->size, attribute->type, GL_FALSE, (GLsizei)entry->vertex_bytesize, (void*)attribute->offset);
    }
}

Vertex_Batch_ID Renderer_GL3::get_vertex_batch(Vertex_Format_Name name, Renderer_Primitive primitive){
    // NOTE(hugo): trying to find a free batch
    for(u32 ifree_batch = 0u; ifree_batch != vertex_batch_storage.free_batches.size; ++ifree_batch){
        u32 free_batch_index = vertex_batch_storage.free_batches[ifree_batch];
        Vertex_Batch_Entry* entry = &vertex_batch_storage.batches[free_batch_index];

        if(entry->format == name){
            vertex_batch_storage.free_batches.remove_swap(ifree_batch);

            entry->primitive = primitive;
            entry->vbo_position = 0u;
            if(entry->vbo_bytesize){
                glBindBuffer(GL_ARRAY_BUFFER, entry->vbo);
                glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)entry->vbo_bytesize, NULL, GL_STREAM_DRAW);
                entry->vbo_mapping = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
                glBindBuffer(GL_ARRAY_BUFFER, 0u);
            }

            return {free_batch_index};
        }
    }

    // NOTE(hugo): creating a new batch, nothing to map because the buffer is empty
    Vertex_Batch_Entry new_batch;
    new_batch.format = name;
    new_batch.primitive = primitive;

    glGenVertexArrays(1, &new_batch.vao);
    glGenBuffers(1, &new_batch.vbo);

    glBindVertexArray(new_batch.vao);
    glBindBuffer(GL_ARRAY_BUFFER, new_batch.vbo);
    use_vertex_format(this, name);
    glBindBuffer(GL_ARRAY_BUFFER, 0u);
    glBindVertexArray(0u);

    u32 output_index = vertex_batch_storage.batches.size;
    vertex_batch_storage.batches.push(new_batch);

    return {output_index};
};

void Renderer_GL3::free_vertex_batch(Vertex_Batch_ID batch){
    Vertex_Batch_Entry* entry = &vertex_batch_storage.batches[batch];

    if(entry->vbo_bytesize && entry->vbo_mapping){
        glBindBuffer(GL_ARRAY_BUFFER, entry->vbo);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, 0u);
        entry->vbo_mapping = nullptr;
    }

    // NOTE(hugo): update bytesize and free extensions and draw data
    entry->vbo_bytesize = max(entry->vbo_bytesize, entry->vbo_position + entry->extension_data.size);
    entry->vbo_position = 0u;

    // TODO(hugo): use clear() to avoid allocations
    entry->first.free();
    entry->count.free();
    entry->extension_data.free();
    entry->extension_first.free();
    entry->extension_count.free();

    // NOTE(hugo): make the batch available for reuse
    vertex_batch_storage.free_batches.push(batch);
}

void* Renderer_GL3::get_vertices(Vertex_Batch_ID batch, u32 nvertices){
    Vertex_Batch_Entry* entry = &vertex_batch_storage.batches[batch];
    Vertex_Format_Entry* format_entry = &vertex_format_storage[entry->format];
    size_t requested_bytesize = (size_t)nvertices * format_entry->vertex_bytesize;

    // NOTE(hugo): get from the buffer mapping
    if(entry->vbo_position + requested_bytesize <= entry->vbo_bytesize){
        void* output_ptr = (void*)((u8*)entry->vbo_mapping + entry->vbo_position);
        entry->first.push((GLint)(entry->vbo_position / format_entry->vertex_bytesize));
        entry->count.push((GLsizei)nvertices);
        entry->vbo_position = entry->vbo_position + requested_bytesize;
        return output_ptr;

    // NOTE(hugo): get from the extension
    }else{
        u32 extension_index = entry->extension_data.size;
        entry->extension_data.set_size((u32)(extension_index + requested_bytesize));
        entry->extension_first.push((GLint)(extension_index / format_entry->vertex_bytesize));
        entry->extension_count.push((GLsizei)nvertices);
        return (void*)&entry->extension_data[extension_index];
    }
}

void Renderer_GL3::submit_batch(Vertex_Batch_ID batch){
    Vertex_Batch_Entry* entry = &vertex_batch_storage.batches[batch];

    if(entry->vbo_bytesize){
        if(entry->vbo_mapping){
            glBindBuffer(GL_ARRAY_BUFFER, entry->vbo);
            glUnmapBuffer(GL_ARRAY_BUFFER);
            glBindBuffer(GL_ARRAY_BUFFER, 0u);
            entry->vbo_mapping = nullptr;
        }

        glBindVertexArray(entry->vao);
        glMultiDrawArrays(entry->primitive, entry->first.data, entry->count.data, (GLsizei)entry->first.size);
    }

    // TODO(hugo): tag what's inside the shared vao using generations to avoid reuploading
    if(entry->extension_data.size){
        glBindVertexArray(vertex_batch_storage.shared_vao);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_batch_storage.shared_vbo);
        if(vertex_batch_storage.shared_format != entry->format){
            use_vertex_format(this, entry->format);
            vertex_batch_storage.shared_format = entry->format;
        }
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)entry->extension_data.size, entry->extension_data.data, GL_STREAM_DRAW);
        glMultiDrawArrays(entry->primitive, entry->extension_first.data, entry->extension_count.data, (GLsizei)entry->extension_first.size);
        glBindBuffer(GL_ARRAY_BUFFER, 0u);
    }
}

// NOTE(hugo): output can be 1, 2 or 4
static inline GLint compute_texture_row_alignment(u32 width, u32 height, Texture_Format format, Renderer_Data_Type type){
    size_t row_bytesize = width * texture_format_channels(format) * (GLint)renderer_data_type_bytes(type);
    return (GLint)min(4u, get_rightmost_set_bit(row_bytesize));
}

Texture_ID Renderer_GL3::get_texture(Texture_Format format, u32 width, u32 height, Renderer_Data_Type data_type, void* data){
    // NOTE(hugo): trying to find a free texture
    for(u32 ifree_texture = 0u; ifree_texture != texture_storage.free_textures.size; ++ifree_texture){
        u32 free_texture_index = texture_storage.free_textures[ifree_texture];
        Texture_Entry* entry = &texture_storage.textures[free_texture_index];

        if(entry->format == format && entry->width == width && entry->height == height){
            texture_storage.free_textures.remove_swap(ifree_texture);
            return {free_texture_index};
        }
    }

    // NOTE(hugo): create a new texture
    Texture_Entry new_texture;
    glGenTextures(1u, &new_texture.texture);
    new_texture.width = width;
    new_texture.height = height;

    glBindTexture(GL_TEXTURE_2D, new_texture.texture);

    // NOTE(hugo): set the unpacking alignment for this operation when necessary
    GLint row_alignment = compute_texture_row_alignment(width, height, format, data_type);
    if(row_alignment != GL::default_unpack_alignment){
        glPixelStorei(GL_UNPACK_ALIGNMENT, row_alignment);
        glTexImage2D(GL_TEXTURE_2D, 0, format, (GLsizei)width, (GLsizei)height, 0, format, data_type, data);
        glPixelStorei(GL_UNPACK_ALIGNMENT, GL::default_unpack_alignment);
    }else{
        glTexImage2D(GL_TEXTURE_2D, 0, format, (GLsizei)width, (GLsizei)height, 0, format, data_type, data);
    }

    // NOTE(hugo): default MIP mapping to have a complete texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    u32 output_index = texture_storage.textures.size;
    texture_storage.textures.push(new_texture);

    return {output_index};
}

void Renderer_GL3::free_texture(Texture_ID texture){
    texture_storage.free_textures.push(texture);
}

void Renderer_GL3::update_texture(Texture_ID texture, Renderer_Data_Type data_type, void* data){
    Texture_Entry* entry = &texture_storage.textures[texture];
    glBindTexture(GL_TEXTURE_2D, entry->texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (GLsizei)entry->width, (GLsizei)entry->height, entry->format, data_type, data);
}

void Renderer_GL3::setup_texture_unit(u32 texture_unit, Texture_ID texture, Sampler_Name sampler_name){
    // NOTE(hugo): bind locations are contiguous values
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    if(sampler_name == Sampler_Name::SAMPLER_NONE){
        glBindSampler(texture_unit, 0u);
    }else{
        glBindSampler(texture_unit, sampler_storage[sampler_name].sampler);
    }
    glBindTexture(GL_TEXTURE_2D, texture_storage.textures[texture].texture);
}
