#ifndef H_RENDERER_USER_DEFINES
#define H_RENDERER_USER_DEFINES

// ---- setup types

enum Renderer_Data_Type{
    TYPE_FLOAT = GL_FLOAT,
    TYPE_UBYTE = GL_UNSIGNED_BYTE,
    TYPE_NONE
};

struct Vertex_Format_Attribute{
    Renderer_Data_Type type = TYPE_NONE;
    u32 size = 0u;
    size_t offset = 0u;
};

// ----- MANUAL -----

// VERTEX FORMAT DECLARATION :
// - declare a vertex struct /vertex_NAME/
// - declare and write a static description of the vertex format /vertex_member_descriptors_NAME/
// - insert NAME in the FOR_EACH_VERTEX_FORMAT macro

// UNIFORM BLOCK DECLARATION :
// - declare a uniform struct /uniform_format_NAME/
// - insert NAME in the FOR_EACH_UNIFORM_BLOCK macro

// SHADER DECLARATION :
// - declare and write the static shaders code as /vertex_NAME/ and /fragment_NAME/
// - insert NAME in the FOR_EACH_SHADER macro
// - insert necessary (UNIFORM_BLOCK_NAME, NAME) bindings in the FOR_EACH_UNIFORM_BLOCK_SHADER_PAIR

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
    #version 450

    in vec4 fragment_color;
    out vec4 output_color;

    void main(){
        output_color = fragment_color;
    }
)";

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
    #version 450

    uniform sampler2D texA;

    in vec2 fragment_texcoord;
    out vec4 output_color;

    void main(){
        output_color = texture(texA, fragment_texcoord);
    }
)";

#define FOR_EACH_UNIFORM_NAME(FUNCTION)         \
FUNCTION(camera_2D)                             \

#define FOR_EACH_VERTEX_FORMAT_NAME(FUNCTION)   \
FUNCTION(xyrgba)                                \
FUNCTION(xyuv)                                  \

#define FOR_EACH_SHADER_NAME(FUNCTION)          \
FUNCTION(polygon_2D)                            \
FUNCTION(polygon_tex_2D)                        \

#define FOR_EACH_UNIFORM_SHADER_PAIR(FUNCTION)  \
FUNCTION(camera_2D, polygon_2D)                 \
FUNCTION(camera_2D, polygon_tex_2D)             \

#define FOR_EACH_TEXTURE_SHADER_PAIR(FUNCTION)  \
FUNCTION(texA, 0, polygon_tex_2D)               \

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

#endif
