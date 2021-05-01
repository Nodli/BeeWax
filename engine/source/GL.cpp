namespace GL{
    Handle create_shader(GLenum type,
            const char* const shader_code,
            const char* const header_code){

        Handle output = glCreateShader(type);

        const GLchar* shader_source[2] = {header_code, shader_code};
        const GLint shader_size[2] = {-1, -1};
        glShaderSource(output, 2, shader_source, shader_size);

        glCompileShader(output);
        ENGINE_CHECK(check_error_shader(output, "create_shader(GLenum, char*, GLint)"), "");

        return output;
    }

    void delete_shader(Shader shader){
        glDeleteShader(shader);
    }

    Program create_program(Shader shaderA){
        Program output = glCreateProgram();
        glAttachShader(output, shaderA);
        glLinkProgram(output);
        check_error_program(output, "create_program(Shader)");
        return output;
    }

    Program create_program(Shader shaderA, Shader shaderB){
        Program output = glCreateProgram();
        glAttachShader(output, shaderA);
        glAttachShader(output, shaderB);
        glLinkProgram(output);
        check_error_program(output, "create_program(Shader, Shader)");
        return output;
    }

    Program create_program(Shader shaderA, Shader shaderB, Shader shaderC){
        Program output = glCreateProgram();
        glAttachShader(output, shaderA);
        glAttachShader(output, shaderB);
        glAttachShader(output, shaderC);
        glLinkProgram(output);
        check_error_program(output, "create_program(Shader, Shader, Shader)");
        return output;
    }

    void delete_program(Program program){
        glDeleteProgram(program);
    }

    Program create_program(GLenum shaderA_type, const char* const shaderA_code,
            const char* const header_code){
        Program output = glCreateProgram();
        Shader shaderA = create_shader(shaderA_type, shaderA_code, header_code);
        glAttachShader(output, shaderA);
        glLinkProgram(output);
        check_error_program(output, "create_program(GLenum, char*)");
        glDeleteShader(shaderA);
        return output;
    }

    Program create_program(GLenum shaderA_type, const char* const shaderA_code,
            GLenum shaderB_type, const char* const shaderB_code,
            const char* const header_code){

        Program output = glCreateProgram();
        Shader shaderA = create_shader(shaderA_type, shaderA_code, header_code);
        Shader shaderB = create_shader(shaderB_type, shaderB_code, header_code);
        glAttachShader(output, shaderA);
        glAttachShader(output, shaderB);
        glLinkProgram(output);
        check_error_program(output, "create_program(GLenum, char*, GLenum, char*)");
        glDeleteShader(shaderA);
        glDeleteShader(shaderB);
        return output;
    }

    Program create_program(GLenum shaderA_type, const char* const shaderA_code,
            GLenum shaderB_type, const char* const shaderB_code,
            GLenum shaderC_type, const char* const shaderC_code,
            const char* const header_code){

        Program output = glCreateProgram();
        Shader shaderA = create_shader(shaderA_type, shaderA_code, header_code);
        Shader shaderB = create_shader(shaderB_type, shaderB_code, header_code);
        Shader shaderC = create_shader(shaderC_type, shaderC_code, header_code);
        glAttachShader(output, shaderA);
        glAttachShader(output, shaderB);
        glAttachShader(output, shaderC);
        glLinkProgram(output);
        check_error_program(output, "create_program(GLenum, char*, GLenum, char*, GLenum, char*)");
        glDeleteShader(shaderA);
        glDeleteShader(shaderB);
        glDeleteShader(shaderC);
        return output;
    }

    Program_Binary retrieve_program_cache(Program handle){
        GLint link_status;
        glGetProgramiv(handle, GL_LINK_STATUS, &link_status);

        assert(handle != 0 && link_status == GL_TRUE);

        Program_Binary cache;

        GLint binary_length;
        glGetProgramiv(handle, GL_PROGRAM_BINARY_LENGTH, &binary_length);
        assert(binary_length > 0);

        cache.memory = (char*)bw_malloc(sizeof(char) * (size_t)binary_length);
        glGetProgramBinary(handle, binary_length, &cache.length, &cache.format, cache.memory);

        return cache;
    }

    Program create_program_from_cache(Program_Binary cache){
        Program output = glCreateProgram();
        glProgramBinary(output, cache.format, cache.memory, cache.length);

        return output;
    }

    Texture create_immutable_texture(uint width, uint height, GLenum internal_format){
        assert(width > 0 && height > 0);

        Texture output;
        glCreateTextures(GL_TEXTURE_2D, 1, &output);
        glTextureStorage2D(output, 1, internal_format, (GLsizei)width, (GLsizei)height);
        return output;
    }

    void reallocate_immutable_texture(Texture texture, uint width, uint height, GLenum internal_format){
        assert(width > 0 && height > 0);

        glInvalidateTexImage(texture, 0);
        glTextureStorage2D(texture, 1, internal_format, (GLsizei)width, (GLsizei)height);
    }

    void delete_texture(Texture texture){
        glDeleteTextures(1, &texture);
    }

    Buffer create_immutable_buffer(size_t size, void* data, GLbitfield usage){
        assert(size > 0);

        Buffer output;
        glCreateBuffers(1, &output);
        glNamedBufferStorage(output, (GLsizeiptr)size, data, usage);
        return output;
    }

    void reallocate_immutable_buffer(Buffer buffer, size_t size, void* data, GLbitfield usage){
        assert(size > 0);

        glInvalidateBufferData(buffer);
        glNamedBufferStorage(buffer, (GLsizeiptr)size, data, usage);
    }

    void delete_buffer(Buffer buffer){
        glDeleteBuffers(1, &buffer);
    }

    void* retrieve_texture(Texture texture, uint width, uint height){
        assert(width > 0 && height > 0);

        size_t data_size = (size_t)width * (size_t)height * (size_t)4 * sizeof(char);
        void* data = bw_malloc(data_size);
        glGetTextureImage(texture, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLsizei)data_size, data);
        return data;
    }

    Texture_Data_Request request_texture(Texture texture, uint width, uint height){
        assert(width > 0 && height > 0);

        Texture_Data_Request output;

        output.size = (size_t)width * (size_t)height * (size_t)4 * sizeof(char);

        glCreateBuffers(1, &output.buffer);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, output.buffer);
        glGetTextureImage(texture, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLsizei)output.size, 0);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

        output.sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

        return output;
    }

    void* Texture_Data_Request::try_retrieval(){
        GLenum sync_status = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
        if(sync_status == GL_CONDITION_SATISFIED || sync_status == GL_ALREADY_SIGNALED){
            void* data = bw_malloc(size);
            glGetNamedBufferSubData(buffer, 0, (GLsizeiptr)size, data);

            glDeleteSync(sync);
            glDeleteBuffers(1, &buffer);

            return data;
        }else{
            return nullptr;
        }
    }

    void* Texture_Data_Request::force_retrieval(){
        GLenum sync_status = GL_TIMEOUT_EXPIRED;
        do{
            sync_status = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
        }while(sync_status != GL_CONDITION_SATISFIED && sync_status != GL_ALREADY_SIGNALED);

        void* data = bw_malloc(size);
        glGetNamedBufferSubData(buffer, 0, (GLsizeiptr)size, data);

        glDeleteSync(sync);
        glDeleteBuffers(1, &buffer);

        return data;
    }

    Compute_Capabilities detect_compute_capabilities(){
        Compute_Capabilities output;

        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &output.max_workgroup_count[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &output.max_workgroup_count[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &output.max_workgroup_count[2]);

        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &output.max_workgroup_size[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &output.max_workgroup_size[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &output.max_workgroup_size[2]);

        glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &output.max_workgroup_invocations);

        return output;
    }

    UBO_Capabilities detect_ubo_capabilities(){
        UBO_Capabilities output;

        glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &output.max_ubo_bindings);
        glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &output.max_ubo_bindings);

        glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &output.max_ubo_bindings);
        glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &output.max_ubo_bindings);
        glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, &output.max_ubo_bindings);

        return output;
    }

    u32 detect_max_samples(){
        s32 out;
        glGetIntegerv(GL_MAX_SAMPLES, &out);
        return out;
    }

    void display_error_translation(const GLenum error_code){
        switch(error_code){
            case GL_INVALID_ENUM:
                printf("GL_INVALID_ENUM : an unacceptable value is specified for an enumerated argument)\n");
                break;

            case GL_INVALID_VALUE:
                printf("GL_INVALID_VALUE : a numeric argument is out of range\n");
                break;

            case GL_INVALID_OPERATION:
                printf("GL_INVALID_OPERATION : the specified operation is not allowed in the current state\n");
                break;

            case GL_STACK_OVERFLOW:
                printf("GL_STACK_OVERFLOW : an attempt has been made to perform an operation that would cause an internal stack to overflow\n");
                break;

            case GL_STACK_UNDERFLOW:
                printf("GL_STACK_UNDERFLOW : an attempt has been made to perform an operation that would cause an internal stack to underflow\n");
                break;

            case GL_OUT_OF_MEMORY:
                printf("GL_OUT_OF_MEMORY : there is not enough memory left to execute the command\n");
                break;

            case GL_INVALID_FRAMEBUFFER_OPERATION:
                printf("GL_INVALID_FRAMEBUFFER_OPERATION : the framebuffer object is not complete\n");
                break;

            case GL_CONTEXT_LOST:
                printf("GL_CONTEXT_LOST : the OpenGL context has been lost, due to a graphics card reset\n");
                break;

            default:
                printf("UNKNOWN ERROR: %u", error_code);
                break;
        }
    }

    void display_error_translation_framebuffer(const GLenum error_code){
        switch(error_code){
            case GL_FRAMEBUFFER_UNDEFINED:
                printf("GL_FRAMEBUFFER_UNDEFINED : is returned if the specified framebuffer is the default read or draw framebuffer, but the default framebuffer does not exist\n");
                break;

            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                printf("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT : is returned if any of the framebuffer attachment points are framebuffer incomplete\n");
                break;

            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                printf("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT : is returned if the framebuffer does not have at least one image attached to it\n");
                break;

            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                printf("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER : is returned if the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment point(s) named by GL_DRAW_BUFFERi\n");
                break;

            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                printf("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER : is returned if the value of GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER\n");
                break;

            case GL_FRAMEBUFFER_UNSUPPORTED:
                printf("GL_FRAMEBUFFER_UNSUPPORTED : is returned if the combination of internal formats of the attaches images violates an implementation-dependant set of restrictions\n");
                break;

            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                printf("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE : is returned if the value of GL_RENDER_BUFFER_SAMPLES is not the same for all attached renderbuffers; if the value of GL_TEXTURE_SAMPLES is not the same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_RENDERBUFFER_SAMPLES does not match the value of GL_TEXTURE_SAMPLES\n");
                printf("is also returned if the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_TEXTURE_FIXED_LOCATIONS is not GL_TRUE for all attached textures2: The value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_TEXTURE_FIXED_LOCATIONS is not GL_TRUE for all attached textures\n");
                break;

            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                printf("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS : is returned if any framebuffer attachment is layered, and any populated attachment is not layered, or if all populated color attachments are not from textures of the same target\n");
                break;

            default:
                printf("UNKNOWN FRAMEBUFFER ERROR: %u", error_code);
                break;
        }
    }


    bool check_error(const char* const message){
        GLenum error_status = glGetError();

        while(error_status != GL_NO_ERROR){
            printf("-- OpenGL ERROR Code : %s %i\n", message, error_status);
            display_error_translation(error_status);
            error_status = glGetError();
        }

        return error_status != GL_NO_ERROR;
    }

    bool check_error_shader(const GLuint shader, const char* const message){
        GLint shader_status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_status);

        if(shader_status != GL_TRUE){
            printf("-- OpenGL ERROR in SHADER : %i MESSAGE : %s\n", shader, message);

            GLint shader_log_length;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &shader_log_length);
            assert(shader_log_length > 0);

            char* shader_log = (char*)bw_malloc(sizeof(char) * (size_t)shader_log_length);
            glGetShaderInfoLog(shader, shader_log_length, NULL, shader_log);

            GLint shader_source_length;
            glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &shader_source_length);
            assert(shader_source_length > 0);

            // NOTE(hugo): the string returned by the function will be null terminated
            // ref : https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetShaderSource.xhtml
            char* shader_source = (char*)bw_malloc(sizeof(char) * (size_t)shader_source_length);
            glGetShaderSource(shader, shader_source_length, NULL, shader_source);

            // NOTE(hugo): parse each line number, print the line and the error
            int previous_error_line_number = -1;
            char* error_line_start;
            char* error_line_end = shader_log - 1;
            for(;;){
                error_line_start = error_line_end + 1;
                error_line_end = strchr(error_line_end + 1, '\n');
                if(!error_line_end){
                    break;
                }
                // NOTE(hugo): shortening the cstring to this error only
                *error_line_end = '\0';


                int error_line_number = -1;
                // NOTE(hugo): (mesa, nvidia, amd, amd)
                if(sscanf(error_line_start, "%*d : %d (%*d)", &error_line_number) == 1
                || sscanf(error_line_start, "%*d ( %d )", &error_line_number) == 1
                || sscanf(error_line_start, "ERROR: %*d : %d", &error_line_number) == 1
                || sscanf(error_line_start, "WARNING: %*d : %d", &error_line_number) == 1){

                    if(error_line_number != previous_error_line_number){
                        int current_line = 0;
                        char* source_line_start;
                        char* source_line_end = shader_source - 1;
                        do{
                            source_line_start = source_line_end + 1;
                            source_line_end = strchr(source_line_end + 1, '\n');
                            ++current_line;
                        }while(current_line < error_line_number && source_line_end);

                        // NOTE(hugo): temporary conversion to a shorter cstring
                        if(source_line_end){
                            *source_line_end = '\0';
                            // NOTE(hugo): skipping indentation (tabs, space)
                            while((*source_line_start == 9 || *source_line_start == 32)
                                    && source_line_start != source_line_end){

                                ++source_line_start;
                            }
                            printf("LINE : %i %s\n", error_line_number, source_line_start);
                            *source_line_end = '\n';
                        }

                        previous_error_line_number = error_line_number;
                    }
                }

                printf("%s\n", error_line_start);
            }
            printf("\n");

            bw_free(shader_source);
            bw_free(shader_log);

            return false;
        }

        return true;
    }


    bool check_error_program(const GLuint program, const char* const message){
        GLint linking_status;
        glGetProgramiv(program, GL_LINK_STATUS, &linking_status);

        if(linking_status != GL_TRUE){
            GLint program_log_length;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &program_log_length);
            assert(program_log_length > 0);

            char* program_log = (char*)bw_malloc(sizeof(char) * (size_t)program_log_length);
            glGetProgramInfoLog(program, program_log_length, NULL, program_log);
            printf("-- OpenGL ERROR in PROGRAM : %i MESSAGE : %s\n", program, message);
            printf("%s\n", program_log);
            bw_free(program_log);

            return false;
        }

        return true;
    }

    bool check_error_framebuffer(const GLenum target, const char* const message){
        GLenum framebuffer_status = glCheckFramebufferStatus(target);

        if(framebuffer_status != GL_FRAMEBUFFER_COMPLETE){
            printf("-- OpenGL ERROR with an incomplete FRAMEBUFFER : %i MESSAGE : %s\n", target, message);
            display_error_translation_framebuffer(framebuffer_status);

            return false;
        }

        return true;
    }

    bool check_error_named_framebuffer(const GLuint framebuffer, const char* const message){
        GLenum framebuffer_status = glCheckNamedFramebufferStatus(framebuffer, GL_FRAMEBUFFER);

        if(framebuffer_status != GL_FRAMEBUFFER_COMPLETE){
            if(framebuffer){
                printf("-- OpenGL ERROR with an incomplete FRAMEBUFFER : %i MESSAGE : %s\n", framebuffer, message);
            }else{
                printf("-- OpenGL ERROR with the default GL_FRAMEBUFFER (check_error_named_framebuffer) MESSAGE : %s\n", message);
            }
            display_error_translation_framebuffer(framebuffer_status);

            return false;
        }

        return true;
    }

    void display_source_translation(const GLenum source_code){
        switch(source_code){
            case GL_DEBUG_SOURCE_API:
                printf("SOURCE : GL_DEBUG_SOURCE_API : generated by calls to the OpenGL API\n");
                break;

            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
                printf("SOURCE : GL_DEBUG_SOURCE_WINDOW_SYSTEM : generated by calls to a window-system API\n");
                break;

            case GL_DEBUG_SOURCE_SHADER_COMPILER:
                printf("SOURCE : GL_DEBUG_SOURCE_SHADER_COMPILER : generated by a compiler for a shading language\n");
                break;

            case GL_DEBUG_SOURCE_THIRD_PARTY:
                printf("SOURCE : GL_DEBUG_SOURCE_THIRD_PARTY : generated by an application associated with OpenGL\n");
                break;

            case GL_DEBUG_SOURCE_APPLICATION:
                printf("SOURCE : GL_DEBUG_SOURCE_APPLICATION : generated by the user of this application\n");
                break;

            case GL_DEBUG_SOURCE_OTHER:
                printf("SOURCE : GL_DEBUG_SOURCE_OTHER : generated by some unknown source\n");
                break;

            default:
                printf("UNKNOWN SOURCE ERROR: %u", source_code);
                break;
        }
    }

    void display_type_translation(const GLenum type_code){
        switch(type_code){
            case GL_DEBUG_TYPE_ERROR:
                printf("TYPE : GL_DEBUG_TYPE_ERROR : an error, typically from the API\n");
                break;

            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
                printf("TYPE : GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR : some behavior marked deprecated has been used\n");
                break;

            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
                printf("TYPE : GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR : something has invoked undefined behavior\n");
                break;

            case GL_DEBUG_TYPE_PORTABILITY:
                printf("TYPE : GL_DEBUG_TYPE_PORTABILITY : some functionality the user relies upon is not portable\n");
                break;

            case GL_DEBUG_TYPE_PERFORMANCE:
                printf("TYPE : GL_DEBUG_TYPE_PERFORMANCE : some code has triggered possible performance issues\n");
                break;

            case GL_DEBUG_TYPE_MARKER:
                printf("TYPE : GL_DEBUG_TYPE_MARKER : command stream annotation\n");
                break;

            case GL_DEBUG_TYPE_PUSH_GROUP:
                printf("TYPE : GL_DEBUG_TYPE_PUSH_GROUP : group pushing\n");
                break;

            case GL_DEBUG_TYPE_POP_GROUP:
                printf("TYPE : GL_DEBUG_TYPE_POP_GROUP : group poping\n");
                break;

            case GL_DEBUG_TYPE_OTHER:
                printf("TYPE : GL_DEBUG_TYPE_OTHER : unknown error type\n");
                break;

            default:
                printf("TYPE : UNKNOWN %u", type_code);
                break;
        }
    }

    void display_severity_translation(const GLenum severity_code){
        switch(severity_code){
            case GL_DEBUG_SEVERITY_HIGH:
                printf("SEVERITY : GL_DEBUG_SEVERITY_HIGH : all OpenGL errors, shader compilation / linking errors, or highly-dangerous undefined behavior\n");
                break;

            case GL_DEBUG_SEVERITY_MEDIUM:
                printf("SEVERITY : GL_DEBUG_SEVERITY_MEDIUM : major performance warnings, shader compilation / linking warnings, or the use of deprecated functionality\n");
                break;

            case GL_DEBUG_SEVERITY_LOW:
                printf("SEVERITY : GL_DEBUG_SEVERITY_LOW : redundant state change performance warning, or unimportant undefined behavior\n");
                break;

            case GL_DEBUG_SEVERITY_NOTIFICATION:
                printf("SEVERITY : GL_DEBUG_SEVERITY_NOTIFICATION : anything that isn't an error or performance issue\n");
                break;

            default:
                printf("UNKNOWN SEVERITY: %u", severity_code);
                break;
        }
    }

    void debug_message_callback(GLenum source,
            GLenum type,
            GLuint id,
            GLenum severity,
            GLsizei length,
            const GLchar* message,
            const void* userParam){

        //DEV_debug_break();

        if(false
        || severity == GL_DEBUG_SEVERITY_NOTIFICATION
        //|| severity == GL_DEBUG_SEVERITY_LOW
        //|| severity == GL_DEBUG_SEVERITY_MEDIUM
        //|| severity == GL_DEBUG_SEVERITY_HIGH
        ) return;

        UNUSED(length);
        UNUSED(userParam);

        printf("-- OpenGL DEBUG CALLBACK for HANDLE : %i %s\n", id, (type == GL_DEBUG_TYPE_ERROR ? "(GL_ERROR)" : ""));
        display_source_translation(source);
        display_type_translation(type);
        display_severity_translation(severity);
        printf("%s\n", message);
        check_error();
    }

    void set_debug_message_callback(){
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(debug_message_callback, nullptr);
    }

    void unset_debug_message_callback(){
        glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDisable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(nullptr, nullptr);
    }

    bool check_debug_message_log(){
        bool found_message = false;

        GLenum source;
        GLenum type;
        GLenum id;
        GLenum severity;

        GLint max_debug_message_length;
        glGetIntegerv(GL_MAX_DEBUG_MESSAGE_LENGTH, &max_debug_message_length);
        assert(max_debug_message_length > 0);

        char* debug_message_log = (char*)bw_malloc(sizeof(char) * (size_t)max_debug_message_length);

        while(glGetDebugMessageLog(1, max_debug_message_length, &source, &type, &id, &severity, nullptr, debug_message_log)){
            found_message = true;
            printf("-- OpenGL DEBUG CALLBACK for HANDLE : %i %s\n", id, (type == GL_DEBUG_TYPE_ERROR ? "(GL_ERROR)" : ""));
            display_source_translation(source);
            display_type_translation(type);
            display_severity_translation(severity);
            printf("%s\n", debug_message_log);
        }

        bw_free(debug_message_log);

        return found_message;
    }

    void set_wireframe(){
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    void unset_wireframe(){
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    void push_debug_group(const char* const groupname){
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0x0, -1, groupname);
    }

    void pop_debug_group(){
        glPopDebugGroup();
    }

    void label_object(GLenum type, GLuint handle, const char* const label){
        glObjectLabel(type, handle, -1, label);
    }

    void label_fence(GLsync fence, const char* const label){
        glObjectPtrLabel(fence, -1, label);
    }

    char* get_object_label(GLenum type, GLuint handle){
        GLint max_label_length;
        glGetIntegerv(GL_MAX_LABEL_LENGTH, &max_label_length);
        assert(max_label_length > 0);

        char* label = (char*)bw_malloc(sizeof(char) * (size_t)max_label_length);
        glGetObjectLabel(type, handle, max_label_length, nullptr, label);

        return label;
    }

    char* get_fence_label(GLsync handle){
        GLint max_label_length;
        glGetIntegerv(GL_MAX_LABEL_LENGTH, &max_label_length);
        assert(max_label_length > 0);

        char* label = (char*)bw_malloc(sizeof(char) * (size_t)max_label_length);
        glGetObjectPtrLabel(handle, max_label_length, nullptr, label);

        return label;
    }
}
