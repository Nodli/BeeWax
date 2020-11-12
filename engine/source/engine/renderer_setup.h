#ifndef H_RENDERER_USER_DEFINES
#define H_RENDERER_USER_DEFINES

// ---- setup types and enums

enum Renderer_Data_Type{
    TYPE_FLOAT = GL_FLOAT,
    TYPE_UBYTE = GL_UNSIGNED_BYTE,
    TYPE_NONE
};
inline size_t renderer_data_type_bytes(Renderer_Data_Type type){
    switch(type){
        case TYPE_FLOAT:
            return 4u;
        case TYPE_UBYTE:
            return 1u;
        default:
            LOG_ERROR("type with value %d missing in renderer_data_type_bytes", type);
            assert(false);
            return 0u;
    }
}

struct Vertex_Format_Attribute{
    Renderer_Data_Type type = TYPE_NONE;
    u32 size = 0u;
    size_t offset = 0u;
};

enum Renderer_Texture_Parameters{
    FILTER_DEFAULT = GL_NEAREST_MIPMAP_LINEAR,
    FILTER_NEAREST = GL_NEAREST,
    FILTER_LINEAR = GL_LINEAR,
    // NOTE(hugo): texels from the nearest mipmap
    FILTER_NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
    FILTER_LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
    // NOTE(hugo): texels interpolated from the two nearest mipmap
    FILTER_NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
    FILTER_LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR,

    WRAP_DEFAULT = GL_REPEAT,
    WRAP_CLAMP = GL_CLAMP_TO_EDGE,
    WRAP_MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
    WRAP_REPEAT = GL_REPEAT,
};

// ----- MANUAL -----

// UNIFORM DECLARATION :
// - declare a uniform struct /uniform_NAME/
// - insert NAME in the FOR_EACH_UNIFORM_NAME macro
// - use the uniform in a shader as : layout (std140) uniform u_NAME { /*/ } NAME;
// - insert NAME in the FOR_EACH_UNIFORM_SHADER_PAIR macro as (NAME, SHADER_NAME)

// VERTEX FORMAT DECLARATION :
// - declare a vertex struct /vertex_NAME/
// - define a static description of the vertex format as /vertex_format_attributes_NAME/
// - insert NAME in the FOR_EACH_VERTEX_FORMAT_NAME macro

// SHADER DECLARATION :
// - define the static shader code as /vertex_shader_NAME/ and /fragment_shader_NAME/
// - insert NAME in the FOR_EACH_SHADER_NAME macro

// SAMPLER DECLARATION :
// - insert NAME in the FOR_EACH_SAMPLER_NAME macro as (NAME, MIN_FILTER, MAG_FILTER, WRAP_S, WRAP_T, WRAP_R)

// -----------------

struct uniform_camera_2D{
    mat3_std140 matrix;
};

struct vertex_xyrgba{
    vec2 vposition;
    vec4 vcolor;
};
static Vertex_Format_Attribute vertex_format_attributes_xyrgba[2] = {
    {TYPE_FLOAT, 2u, offsetof(vertex_xyrgba, vposition)},
    {TYPE_FLOAT, 4u, offsetof(vertex_xyrgba, vcolor)}
};

struct vertex_xyuv{
    vec2 vposition;
    vec2 vtexcoord;
};
static Vertex_Format_Attribute vertex_format_attributes_xyuv[2] = {
    {TYPE_FLOAT, 2u, offsetof(vertex_xyuv, vposition)},
    {TYPE_FLOAT, 2u, offsetof(vertex_xyuv, vtexcoord)}
};

static const char* vertex_shader_polygon_2D = R"(
    #version 330

    layout (std140) uniform u_camera_2D{
        mat3 camera_matrix;
    } camera_2D;

    layout(location = 0) in vec2 vposition;
    layout(location = 1) in vec4 vcolor;

    out vec4 fragment_color;

    void main(){
        gl_Position = vec4(camera_2D.camera_matrix * vec3(vposition, 1.), 1.);
        gl_Position.z = 0.;
        fragment_color = vcolor;
    }
)";
static const char* fragment_shader_polygon_2D = R"(
    #version 330

    in vec4 fragment_color;
    out vec4 output_color;

    void main(){
        output_color = fragment_color;
    }
)";

static const char* vertex_shader_screen_polygon_2D =  R"(
    #version 330

    layout(location = 0) in vec2 vposition;
    layout(location = 1) in vec4 vcolor;

    out vec4 fragment_color;

    void main(){
        gl_Position = vec4(vposition, 0., 1.);
        fragment_color = vcolor;
    }
)";
static const char* fragment_shader_screen_polygon_2D = fragment_shader_polygon_2D;

static const char* vertex_shader_polygon_tex_2D = R"(
    #version 330

    layout (std140) uniform u_camera_2D{
        mat3 camera_matrix;
    } camera_2D;

    layout(location = 0) in vec2 vposition;
    layout(location = 1) in vec2 vtexcoord;

    out vec2 fragment_texcoord;

    void main(){
        gl_Position = vec4(camera_2D.camera_matrix * vec3(vposition, 1.), 1.);
        gl_Position.z = 0.;
        fragment_texcoord = vtexcoord;
    }
)";
static const char* fragment_shader_polygon_tex_2D = R"(
    #version 330

    uniform sampler2D texA;

    in vec2 fragment_texcoord;
    out vec4 output_color;

    void main(){
        output_color = texture(texA, fragment_texcoord);
    }
)";

static const char* vertex_shader_screen_polygon_tex_2D = R"(
    #version 330

    layout(location = 0) in vec2 vposition;
    layout(location = 1) in vec2 vtexcoord;

    out vec2 fragment_texcoord;

    void main(){
        gl_Position = vec4(vposition, 0., 1.);
        fragment_texcoord = vtexcoord;
    }
)";
static const char* fragment_shader_screen_polygon_tex_2D = fragment_shader_polygon_tex_2D;

static const char* vertex_shader_font_2D = vertex_shader_screen_polygon_tex_2D;
static const char* fragment_shader_font_2D = R"(
    #version 330

    uniform sampler2D font_bitmap;

    in vec2 fragment_texcoord;
    out vec4 output_color;

    float edge_value = 180. / 255.;
    float smoothing_value = edge_value - 0.1;

    void main(){
        float font_sample = texture(font_bitmap, fragment_texcoord).r;
        font_sample = smoothstep(smoothing_value, edge_value, font_sample);
        if(font_sample == 0.)
            discard;
        output_color = vec4(font_sample);
    }
)";

#define FOR_EACH_UNIFORM_NAME(FUNCTION)         \
FUNCTION(camera_2D)                             \

#define FOR_EACH_VERTEX_FORMAT_NAME(FUNCTION)   \
FUNCTION(xyrgba)                                \
FUNCTION(xyuv)                                  \

#define FOR_EACH_SHADER_NAME(FUNCTION)          \
FUNCTION(polygon_2D)                            \
FUNCTION(screen_polygon_2D)                     \
FUNCTION(polygon_tex_2D)                        \
FUNCTION(screen_polygon_tex_2D)                 \
FUNCTION(font_2D)                               \

#define FOR_EACH_UNIFORM_SHADER_PAIR(FUNCTION)  \
FUNCTION(camera_2D, polygon_2D)                 \
FUNCTION(camera_2D, polygon_tex_2D)             \

#define FOR_EACH_TEXTURE_SHADER_PAIR(FUNCTION)  \
FUNCTION(texA, 0, polygon_tex_2D)               \
FUNCTION(font_bitmap, 0, font_2D)               \

#define FOR_EACH_SAMPLER_NAME(FUNCTION)                                                         \
FUNCTION(nearest_clamp, FILTER_NEAREST, FILTER_NEAREST, WRAP_CLAMP, WRAP_CLAMP, WRAP_CLAMP)     \
FUNCTION(linear_clamp, FILTER_LINEAR, FILTER_LINEAR, WRAP_CLAMP, WRAP_CLAMP, WRAP_CLAMP)        \

// ---- setup

enum Uniform_Name{
    FOR_EACH_UNIFORM_NAME(ADD_TO_ENUM)
    NUMBER_OF_UNIFORM_NAMES,
    UNIFORM_NONE = NUMBER_OF_UNIFORM_NAMES
};

enum Vertex_Format_Name{
    FOR_EACH_VERTEX_FORMAT_NAME(ADD_TO_ENUM)
    NUMBER_OF_VERTEX_FORMAT_NAMES,
    VERTEX_FORMAT_NONE = NUMBER_OF_VERTEX_FORMAT_NAMES
};

enum Shader_Name{
    FOR_EACH_SHADER_NAME(ADD_TO_ENUM)
    NUMBER_OF_SHADER_NAMES,
    SHADER_NONE = NUMBER_OF_SHADER_NAMES
};

enum Sampler_Name{
    FOR_EACH_SAMPLER_NAME(ADD_TO_ENUM)
    NUMBER_OF_SAMPLER_NAMES,
    SAMPLER_NONE = NUMBER_OF_SAMPLER_NAMES
};

#endif
