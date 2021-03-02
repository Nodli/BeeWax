#ifndef H_RENDERER_USER_DEFINES
#define H_RENDERER_USER_DEFINES

enum Data_Type{
    TYPE_FLOAT = GL_FLOAT,
    TYPE_USHORT = GL_UNSIGNED_SHORT,
    TYPE_UBYTE = GL_UNSIGNED_BYTE,
    TYPE_NONE
};

// NOTE(hugo):
// * asset textures should use SRGB
// * favor formats with 4 channels
// * synchronized with texture_format_info in renderer_GL3.cpp
// * stores GL's /internalformat/
enum Texture_Format{
    TEXTURE_FORMAT_SRGB_BYTE = GL_SRGB8,
    TEXTURE_FORMAT_SRGBA_BYTE = GL_SRGB8_ALPHA8,

    TEXTURE_FORMAT_R_BYTE = GL_RED,
    TEXTURE_FORMAT_RGB_BYTE = GL_RGB,
    TEXTURE_FORMAT_RGBA_BYTE = GL_RGBA,

    TEXTURE_FORMAT_NONE
};

enum Texture_Parameters{
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

enum Primitive_Type{
    PRIMITIVE_TRIANGLES = GL_TRIANGLES,
    PRIMITIVE_TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
    PRIMITIVE_LINES = GL_LINES,
    PRIMITIVE_LINE_STRIP = GL_LINE_STRIP,
    PRIMITIVE_LINE_LOOP = GL_LINE_LOOP,
    PRIMITIVE_NONE
};

struct Vertex_Format_Attribute{
    Data_Type type = TYPE_NONE;
    u32 size = 0u;
    size_t offset = 0u;
};

// ----- RENDERER SETUP MANUAL -----

// UNIFORM DECLARATION :
// - declare a uniform struct /uniform_NAME/
// - insert NAME in the FOR_EACH_UNIFORM_NAME macro
// - use the uniform in a shader as : layout (std140) uniform u_NAME { /*/ } NAME;

// VERTEX FORMAT DECLARATION :
// - declare a vertex struct /vertex_NAME/
// - define a static description of the vertex format as /vertex_format_attributes_NAME/
// - insert NAME in the FOR_EACH_VERTEX_FORMAT_NAME macro

// SHADER DECLARATION :
// - define the static shader code as /vertex_shader_NAME/ and /fragment_shader_NAME/
// - insert NAME in the FOR_EACH_SHADER_NAME macro
// - declare the uniforms used by the shader in FOR_EACH_UNIFORM_SHADER_PAIR macro as (UNIFORM_NAME, SHADER_NAME)
// - declare the texture variables used by the shader in FOR_EACH_TEXTURE_SHADER_PAIR macro as (TEX_VAR_NAME, SHADER_NAME)

// SAMPLER DECLARATION :
// - insert NAME in the FOR_EACH_SAMPLER_NAME macro as (NAME, MIN_FILTER, MAG_FILTER, WRAP_S, WRAP_T, WRAP_R)

// ---------------------------------

// ---- engine settings

struct uniform_camera_2D{
    mat3_std140 matrix;
};

struct uniform_pattern_info{
    vec4 center_height_aspectratio;
    float pattern_size;
};

struct vertex_xyzrgba{
    vec3 vposition;
    vec4 vcolor;
};
static Vertex_Format_Attribute vertex_format_attributes_xyzrgba[2] = {
    {TYPE_FLOAT, 3u, offsetof(vertex_xyzrgba, vposition)},
    {TYPE_FLOAT, 4u, offsetof(vertex_xyzrgba, vcolor)}
};

struct vertex_xyzuv{
    vec3 vposition;
    vec2 vtexcoord;
};
static Vertex_Format_Attribute vertex_format_attributes_xyzuv[2] = {
    {TYPE_FLOAT, 3u, offsetof(vertex_xyzuv, vposition)},
    {TYPE_FLOAT, 2u, offsetof(vertex_xyzuv, vtexcoord)}
};

#if defined(OPENGL_DESKTOP)
static const char* GLSL_version=
R"(#version 330)" "\n";
#elif defined(OPENGL_ES)
static const char* GLSL_version=
R"(#version 300 es)" "\n"
R"(precision mediump float;)" "\n";
#else
    static_assert(false, "no GLSL_version for the specified OpenGL version");
#endif

// NOTE(hugo): use with glDrawArrays(GL_TRIANGLES, 0u, 3u);
static const char* emit_fullscreen_triangle = R"(
    out vec2 screenspace_coord;

    void main(){
        vec2 screenpos = vec2(
            -1. + float((gl_VertexID & 1) << 2),
            -1. + float((gl_VertexID & 2) << 2)
        );

        // NOTE(hugo): furthest z below 1.
        float z = intBitsToFloat(floatBitsToInt(1) - 1);

        gl_Position = vec4(screenpos.xy, z, 1.);
        screenspace_coord = vec2(
            (screenpos.x + 1.) * 0.5,
            (screenpos.y + 1.) * 0.5
        );
    }
)";

static const char* shader_header_editor_pattern = GLSL_version;;
static const char* vertex_shader_editor_pattern = emit_fullscreen_triangle;
static const char* fragment_shader_editor_pattern = R"(
    layout(std140) uniform u_pattern_info{
        vec4 center_height_aspectratio;
        float pattern_size;
    } pattern_info;

    in vec2 screenspace_coord;
    out vec4 output_color;

    void main(){
        vec2 center = pattern_info.center_height_aspectratio.xy;
        float width = pattern_info.center_height_aspectratio.z * pattern_info.center_height_aspectratio.w;
        float height = pattern_info.center_height_aspectratio.z;
        float pattern_size = pattern_info.pattern_size;

        vec2 coord_world = (screenspace_coord - vec2(0.5)) * vec2(width, height) + center;
        float dcoord_world = fwidth(coord_world.x);

        vec2 dist_pattern = mod(coord_world + dcoord_world, pattern_size);
        float pattern = float(dist_pattern.x < 2. * dcoord_world || dist_pattern.y < 2. * dcoord_world);

        vec2 dist_major = abs(coord_world);
        float major_axis = float(dist_major.x < dcoord_world || dist_major.y < dcoord_world);

        float minor_size = 5. * pattern_size;
        vec2 dist_minor = mod(coord_world + dcoord_world, minor_size);
        float minor_axis = float(dist_minor.x < 2. * dcoord_world || dist_minor.y < 2. * dcoord_world);

        vec3 background_color = vec3(0.12, 0.11, 0.1);
        vec3 pattern_color = background_color * 1.1;
        vec3 major_axis_color = background_color * 0.7;
        vec3 minor_axis_color = background_color * 0.9;
        vec3 color = mix(mix(mix(background_color, pattern_color, pattern), minor_axis_color, minor_axis), major_axis_color, major_axis);

        output_color = vec4(color, 1.);
    }
)";

// NOTE(hugo): polygon_2D
// /vcolor/ is expected in linear space
static const char* shader_header_polygon_2D = GLSL_version;
static const char* vertex_shader_polygon_2D = R"(
    layout (std140) uniform u_camera_2D{
        mat3 camera_matrix;
    } camera_2D;

    layout(location = 0) in vec3 vposition;
    layout(location = 1) in vec4 vcolor;

    out vec4 fragment_color;

    void main(){
        gl_Position = vec4(camera_2D.camera_matrix * vec3(vposition.xy, 1.), 1.);
        gl_Position.z = vposition.z;
        fragment_color = vcolor;
    }
)";
static const char* fragment_shader_polygon_2D = R"(
    in vec4 fragment_color;
    out vec4 output_color;

    void main(){
        output_color = fragment_color;
    }
)";

// NOTE(hugo): polygon_2D_tex
// /tex/ is expected in linear space ie use _SRGB or _SRGBA texture formats
static const char* shader_header_polygon_2D_tex = GLSL_version;
static const char* vertex_shader_polygon_2D_tex = R"(
    layout (std140) uniform u_camera_2D{
        mat3 camera_matrix;
    } camera_2D;

    layout(location = 0) in vec3 vposition;
    layout(location = 1) in vec2 vtexcoord;

    out vec2 fragment_texcoord;

    void main(){
        gl_Position = vec4(camera_2D.camera_matrix * vec3(vposition.xy, 1.), 1.);
        gl_Position.z = vposition.z;
        fragment_texcoord = vtexcoord;
    }
)";
static const char* fragment_shader_polygon_2D_tex = R"(
    uniform sampler2D tex;

    in vec2 fragment_texcoord;
    out vec4 output_color;

    void main(){
        vec4 sample_color = texture(tex, fragment_texcoord);

        if(sample_color.a == 0.)
            discard;

        output_color = sample_color;
    }
)";

#define FOR_EACH_UNIFORM_NAME_ENGINE(FUNCTION)          \
FUNCTION(camera_2D)                                     \
FUNCTION(pattern_info)                                  \

#define FOR_EACH_VERTEX_FORMAT_NAME_ENGINE(FUNCTION)    \
FUNCTION(xyzrgba)                                       \
FUNCTION(xyzuv)                                         \

#define FOR_EACH_SHADER_NAME_ENGINE(FUNCTION)           \
FUNCTION(polygon_2D)                                    \
FUNCTION(polygon_2D_tex)                                \
FUNCTION(editor_pattern)                                \

#define FOR_EACH_UNIFORM_SHADER_PAIR_ENGINE(FUNCTION)   \
FUNCTION(camera_2D, polygon_2D)                         \
FUNCTION(camera_2D, polygon_2D_tex)                     \
FUNCTION(pattern_info, editor_pattern)                  \

#define FOR_EACH_TEXTURE_SHADER_PAIR_ENGINE(FUNCTION)   \
FUNCTION(tex, 0, polygon_2D_tex)                        \

#define FOR_EACH_SAMPLER_NAME_ENGINE(FUNCTION)                                                  \
FUNCTION(nearest_clamp, FILTER_NEAREST, FILTER_NEAREST, WRAP_CLAMP, WRAP_CLAMP, WRAP_CLAMP)     \
FUNCTION(linear_clamp, FILTER_LINEAR, FILTER_LINEAR, WRAP_CLAMP, WRAP_CLAMP, WRAP_CLAMP)        \

// ---- user settings

#if defined(RENDERER_GL3_USER_SETTINGS)
    #include RENDERER_GL3_USER_SETTINGS
#else
    #define FOR_EACH_UNIFORM_NAME_USER(FUNCTION)
    #define FOR_EACH_VERTEX_FORMAT_NAME_USER(FUNCTION)
    #define FOR_EACH_SHADER_NAME_USER(FUNCTION)
    #define FOR_EACH_UNIFORM_SHADER_PAIR_USER(FUNCTION)
    #define FOR_EACH_TEXTURE_SHADER_PAIR_USER(FUNCTION)
    #define FOR_EACH_SAMPLER_NAME_USER(FUNCTION)
#endif

// ---- settings

#define FOR_EACH_UNIFORM_NAME(FUNCTION)     \
FOR_EACH_UNIFORM_NAME_ENGINE(FUNCTION)      \
FOR_EACH_UNIFORM_NAME_USER(FUNCTION)

#define FOR_EACH_VERTEX_FORMAT_NAME(FUNCTION)     \
FOR_EACH_VERTEX_FORMAT_NAME_ENGINE(FUNCTION)      \
FOR_EACH_VERTEX_FORMAT_NAME_USER(FUNCTION)

#define FOR_EACH_SHADER_NAME(FUNCTION)     \
FOR_EACH_SHADER_NAME_ENGINE(FUNCTION)      \
FOR_EACH_SHADER_NAME_USER(FUNCTION)

#define FOR_EACH_UNIFORM_SHADER_PAIR(FUNCTION)     \
FOR_EACH_UNIFORM_SHADER_PAIR_ENGINE(FUNCTION)      \
FOR_EACH_UNIFORM_SHADER_PAIR_USER(FUNCTION)

#define FOR_EACH_TEXTURE_SHADER_PAIR(FUNCTION)     \
FOR_EACH_TEXTURE_SHADER_PAIR_ENGINE(FUNCTION)      \
FOR_EACH_TEXTURE_SHADER_PAIR_USER(FUNCTION)

#define FOR_EACH_SAMPLER_NAME(FUNCTION)     \
FOR_EACH_SAMPLER_NAME_ENGINE(FUNCTION)      \
FOR_EACH_SAMPLER_NAME_USER(FUNCTION)

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
