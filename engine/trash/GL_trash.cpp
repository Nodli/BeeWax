#if 0
    // NOTE(hugo): rgba
    vertex_xyz_rgba vertices_rgba[] = {
        {{0.5f, 0.5f, 0.f}, 0x00, 0xFF, 0x00, 0xFF},
        {{0.5f, -0.5f, 0.f}, 0x00, 0xFF, 0x00, 0xFF},
        {{-0.5f, -0.5f, 0.f}, 0x00, 0xFF, 0x00, 0xFF},
        {{-0.5f, 0.5f, 0.f}, 0x00, 0xFF, 0x00, 0xFF}
    };
    u16 indices_rgba[] = {0u, 1u, 3u, 1u, 2u, 3u};

    GL::Vertex_Array vao_rgba;
    glCreateVertexArrays(1u, &vao_rgba);

    GL::Buffer vbo_rgba;
    glCreateBuffers(1u, &vbo_rgba);
    glNamedBufferStorage(vbo_rgba, carray_size(vertices_rgba) * sizeof(vertices_rgba[0]), vertices_rgba, GL_CLIENT_STORAGE_BIT);

    GL::Buffer ibo_rgba;
    glCreateBuffers(1u, &ibo_rgba);
    glNamedBufferStorage(ibo_rgba, carray_size(indices) * sizeof(indices[0]), indices, GL_CLIENT_STORAGE_BIT);

    glVertexArrayVertexBuffer(vao_rgba, 0u, vbo_rgba, 0u, sizeof(vertices_rgba[0]));
    glVertexArrayElementBuffer(vao_rgba, ibo_rgba);
    glEnableVertexArrayAttrib(vao_rgba, 0u);
    glEnableVertexArrayAttrib(vao_rgba, 1u);
    glVertexArrayAttribFormat(vao_rgba, 0u, 3u, GL_FLOAT, GL_FALSE, offsetof(vertex_xyz_rgba, xyz));
    glVertexArrayAttribFormat(vao_rgba, 1u, 4u, GL_UNSIGNED_BYTE, GL_FALSE, offsetof(vertex_xyz_rgba, r));
    glVertexArrayAttribBinding(vao_rgba, 0u, 0u);
    glVertexArrayAttribBinding(vao_rgba, 1u, 0u);

    const char* vertex_shader_rgba = R"(
        #version 460 core

        layout (location = 0) in vec3 in_xyz;
        layout (location = 1) in vec4 in_rgba;

        layout (location = 0) out vec4 next_color;

        layout (std140, location = 0, binding = 0) uniform u_Camera{
            vec4 worldspace_position;
            mat4 VP;
        } camera;

        void main(){
            gl_Position = camera.VP * vec4(in_xyz, 1.);
            next_color = in_rgba;
        }
    )";
    const char* fragment_shader_rgba = R"(
        #version 460 core

        layout(location = 0) in vec4 next_color;

        layout(location = 0) out vec4 out_color;

        void main(){
            out_color = next_color;
        }
    )";
    GL::Program program_rgba = GL::create_program(GL_VERTEX_SHADER, vertex_shader_rgba, GL_FRAGMENT_SHADER, fragment_shader_rgba);

    GL::Buffer ubo_camera;
    glCreateBuffers(1u, &ubo_camera);
    glNamedBufferStorage(ubo_camera, sizeof(u_Camera), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
    u_Camera* ubo_camera_ptr = (u_Camera*)glMapNamedBufferRange(ubo_camera, 0u, sizeof(u_Camera), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

    // NOTE(hugo): drawing
    glBindBufferBase(GL_UNIFORM_BUFFER, 0u, ubo_camera);
    glUseProgram(program_rgba);
    glBindVertexArray(vao_rgba);
    glDrawElements(GL_TRIANGLES, 6u, GL_UNSIGNED_SHORT, 0u);

    // NOTE(hugo): uv
    vertex_xyz_uv vertices_uv[] = {
        {{0.5f, 0.5f, 0.f}, float_to_unorm16(1.f), float_to_unorm16(1.f)},
        {{0.5f, -0.5f, 0.f}, float_to_unorm16(1.f), float_to_unorm16(0.f)},
        {{-0.5f, -0.5f, 0.f}, float_to_unorm16(0.f), float_to_unorm16(0.f)},
        {{-0.5f, 0.5f, 0.f}, float_to_unorm16(0.f), float_to_unorm16(1.f)}
    };
    u16 indices_uv[] = {0u, 1u, 3u, 1u, 2u, 3u};

    GL::Vertex_Array vao_uv;
    glCreateVertexArrays(1u, &vao_uv);

    GL::Buffer vbo_uv;
    glCreateBuffers(1u, &vbo_uv);
    glNamedBufferStorage(vbo_uv, carray_size(vertices_uv) * sizeof(vertices_uv[0]), vertices_uv, GL_CLIENT_STORAGE_BIT);

    GL::Buffer ibo_uv;
    glCreateBuffers(1u, &ibo_uv);
    glNamedBufferStorage(ibo_uv, carray_size(indices) * sizeof(indices[0]), indices, GL_CLIENT_STORAGE_BIT);

    glVertexArrayVertexBuffer(vao_uv, 0u, vbo_uv, 0u, sizeof(vertices_uv[0]));
    glVertexArrayElementBuffer(vao_uv, ibo_uv);
    glEnableVertexArrayAttrib(vao_uv, 0u);
    glEnableVertexArrayAttrib(vao_uv, 1u);
    glVertexArrayAttribFormat(vao_uv, 0u, 3u, GL_FLOAT, GL_FALSE, offsetof(vertex_xyz_uv, xyz));
    glVertexArrayAttribFormat(vao_uv, 1u, 2u, GL_UNSIGNED_SHORT, GL_TRUE, offsetof(vertex_xyz_uv, u));
    glVertexArrayAttribBinding(vao_uv, 0u, 0u);
    glVertexArrayAttribBinding(vao_uv, 1u, 0u);

    GL::Sampler sampler;
    glCreateSamplers(1u, &sampler);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);

    s32 texture_width;
    s32 texture_height;
    s32 texture_channels;
    void* texture_data = stbi_load("data/texture/checker4.png", &texture_width, &texture_height, &texture_channels, 4u);
    ENGINE_CHECK(texture_data != NULL, "FAILED to stbi_load");

    GL::Texture texture;
    glCreateTextures(GL_TEXTURE_2D, 1u, &texture);
    glTextureStorage2D(texture, 1u, GL_SRGB8_ALPHA8, texture_width, texture_height);
    glTextureSubImage2D(texture, 0u, 0u, 0u, texture_width, texture_height, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
    glGenerateTextureMipmap(texture);

    stbi_image_free(texture_data);

    const char* vertex_shader_uv = R"(
        #version 460 core

        layout (location = 0) in vec3 in_xyz;
        layout (location = 1) in vec2 in_uv;

        layout (location = 0) out vec2 next_uv;

        layout (std140, location = 0, binding = 0) uniform u_Camera{
            vec4 worldspace_position;
            mat4 VP;
        } camera;

        void main(){
            gl_Position = camera.VP * vec4(in_xyz, 1.);
            next_uv = in_uv;
        }
    )";
    const char* fragment_shader_uv = R"(
        #version 460 core

        layout(location = 0) in vec2 next_uv;

        layout(location = 0) out vec4 out_color;

        layout(location = 0, binding = 0) uniform sampler2D sampler;

        void main(){
            out_color = texture(sampler, next_uv);
        }
    )";
    GL::Program program_uv = GL::create_program(GL_VERTEX_SHADER, vertex_shader_uv, GL_FRAGMENT_SHADER, fragment_shader_uv);

    GL::Buffer ubo_camera;
    glCreateBuffers(1u, &ubo_camera);
    glNamedBufferStorage(ubo_camera, sizeof(u_Camera), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
    u_Camera* ubo_camera_ptr = (u_Camera*)glMapNamedBufferRange(ubo_camera, 0u, sizeof(u_Camera), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

    // NOTE(hugo): when drawing
    glBindBufferBase(GL_UNIFORM_BUFFER, 0u, ubo_camera);
    glUseProgram(program_uv);
    glBindTextureUnit(0u, texture);
    glBindSampler(0u, sampler);
    glBindVertexArray(vao_uv);
    glDrawElements(GL_TRIANGLES, 6u, GL_UNSIGNED_SHORT, 0u);
#endif
