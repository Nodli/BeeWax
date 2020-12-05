static const char* vertex_shader_glitchy_tex = vertex_shader_polygon_tex_2D;
static const char* fragment_shader_glitchy_tex = R"(
    #version 330

    uniform sampler2D texA;

    in vec2 fragment_texcoord;
    out vec4 output_color;

    float rand(float n){
        return fract(sin(n) * 43758.5453123);
    }

    const float sample_offset_probability = 0.8;
    const float fibo_multiplier = 11400715.12213028;

    void main(){
        ivec2 texA_size = textureSize(texA, 0);
        ivec2 sample_pixcoord = ivec2(fragment_texcoord * vec2(texA_size));
        int effect_index = sample_pixcoord.x % 4;

        vec2 sample_texcoord = fragment_texcoord;

        float random = rand(fragment_texcoord.y);
        if(random > sample_offset_probability){
            random = (random - sample_offset_probability) / (1. - sample_offset_probability);

            int sample_sign = random > 0.5 ? 1 : -1;
            int sample_offset = texA_size.x / 100;
            sample_offset = sample_sign * int(float(sample_offset) * mod(random * fibo_multiplier, 100.) / 100.);

            sample_texcoord.x = sample_texcoord.x + float(sample_offset) / float(texA_size);
            sample_texcoord.x = mod(sample_texcoord.x, 1.);
        }

        vec4 sample_color = texture(texA, sample_texcoord);
        if(sample_color.a == 0.)
            discard;

        if(effect_index == 0){
            output_color = vec4(sample_color.r, sample_color.r, sample_color.r, sample_color.a);
        }else if(effect_index == 1){
            output_color = vec4(0.9, sample_color.b, sample_color.g, sample_color.a);
        }else if(effect_index == 2){
            output_color = vec4(sample_color.r, 0.9, sample_color.b, sample_color.a);
        }else if(effect_index == 3){
            output_color = vec4(sample_color.r, sample_color.g, 0.9, sample_color.a);
        }
    }
)";

#define FOR_EACH_UNIFORM_NAME_USER(FUNCTION)            \

#define FOR_EACH_VERTEX_FORMAT_NAME_USER(FUNCTION)      \

#define FOR_EACH_SHADER_NAME_USER(FUNCTION)             \
FUNCTION(glitchy_tex)                                   \

#define FOR_EACH_UNIFORM_SHADER_PAIR_USER(FUNCTION)     \
FUNCTION(camera_2D, glitchy_tex)                        \

#define FOR_EACH_TEXTURE_SHADER_PAIR_USER(FUNCTION)     \
FUNCTION(texA, 0, glitchy_tex)                          \

#define FOR_EACH_SAMPLER_NAME_USER(FUNCTION)                                                    \

