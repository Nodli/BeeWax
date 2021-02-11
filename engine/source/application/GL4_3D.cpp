using namespace bw;

// REF(hugo):
// https://learnopengl.com/
// - OpenGL DSA
// https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions
// - UBO performance in OpenGL
// https://github.com/buildaworldnet/IrrlichtBAW/wiki/Best-IGPUBuffer-Types-for-UBO-data-sourcing
// - various shadow map techniques
// https://therealmjp.github.io/posts/shadow-maps/
// https://mynameismjp.wordpress.com/2013/09/10/shadow-maps/
// - normal depth bias
// https://web.archive.org/web/20180524211931/http://www.dissidentlogic.com/old/images/NormalOffsetShadows/GDC_Poster_NormalOffset.png
// - optimized PCF
// http://www.ludicon.com/castano/blog/articles/shadow-mapping-summary-part-1/
// - receiver plane depth bias
// http://developer.amd.com/wordpress/media/2012/10/Isidoro-ShadowMapping.pdf
// - The Rendering Technology of Killzone 2
// https://de.slideshare.net/guerrillagames/the-rendering-technology-of-killzone-2
// - Secrets of CryEngine 3 Graphics Technology
// https://de.slideshare.net/TiagoAlexSousa/secrets-of-cryengine-3-graphics-technology
// - Filmic Tonemapping
// http://filmicworlds.com/blog/filmic-tonemapping-operators/
// - PBR in Remember Me
// https://www.fxguide.com/fxfeatured/game-environments-parta-remember-me-rendering/
// - SIGGRAPH 2012: Physics and Math of Shading
// https://blog.selfshadow.com/publications/s2012-shading-course/hoffman/s2012_pbs_physics_math_notes.pdf
// https://blog.selfshadow.com/publications/s2012-shading-course/hoffman/s2012_pbs_physics_math_slides.pdf
// - PBR in Film and Game Production: Practical Implementation at tri-Ace
// http://renderwonk.com/publications/s2010-shading-course/gotanda/course_note_practical_implementation_at_triace.pdf
// https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
// - Real Shading in UE4
// https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
// - Filament
// https://google.github.io/filament/Filament.md.html#materialsystem/standardmodel
// https://google.github.io/filament/images/material_chart.jpg
// - RenderMan / PxrDisney
// https://renderman.pixar.com/resources/RenderMan_20/PxrDisney.html
// - ACES Filmic Tone Mapping Curve
// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/

// TODO(hugo): use triple buffered ubos with glBindBufferRange instead of glBindBufferBase

struct u_Camera{
    vec4 worldspace_position;
    mat4_std140 VP;
};

struct u_Material{
    vec4 color_roughness;
    vec2 reflectance_metalness;
};

struct u_Shadowmap_Light{
    mat4_std140 VP;
    vec4 worldspace_direction_texel_size;
    vec4 radiance;
};

struct u_Tone_Mapping{
    float exposure;
};

struct vertex_xyz_rgba{
    vec3 position;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
};
struct vertex_xyz_uv{
    vec3 position;
    u16 u;
    u16 v;
};
struct vertex_xyz_nxyz{
    vec3 position;
    vec3 normal;
};

u8 float_to_unorm8(float x){
    return (u8)(min_max(x * 255.f, 0.f, 255.f) + 0.5f);
}
u16 float_to_unorm16(float x){
    return (u16)(min_max(x * 65535.f, 0.f, 65535.f) + 0.5f);
}

// REF(hugo):
// https://aras-p.info/texts/CompactNormalStorage.html

vec2 spheremap_transform_encode(vec3 normal, vec3 view)
{
    float f = bw::sqrt(8.f * normal.z + 8.f);
    return {normal.x / f + 0.5f, normal.y / f + 0.5f};
}
vec3 spheremap_transform_decode(vec2 encoded, vec3 view)
{
    vec2 fencoded = encoded * 4.f - 2.f;
    float f = dot(fencoded, fencoded);
    float g = bw::sqrt(1.f - f * 0.25f);
    return {
        fencoded.x * g,
        fencoded.y * g,
        1.f - f * 0.5f
    };
}

// REF(hugo):
// https://de.slideshare.net/guerrillagames/the-rendering-technology-of-killzone-2

void clipping_planes_from_sphere(float& near_plane, float& far_plane,
        vec3 view_position, vec3 view_direction, vec3 sphere_center, float sphere_radius,
        float min_padding = 0.5f, float min_near_plane = 0.1f, float min_near_to_far = 0.1f){

    vec3 pos_to_min = (sphere_center - sphere_radius * view_direction) - view_position;
    vec3 pos_to_max = (sphere_center + sphere_radius * view_direction) - view_position;

    float dmin = dot(pos_to_min, view_direction);
    float dmax = dot(pos_to_max, view_direction);

    near_plane = max(min_near_plane, dmin - min_padding);
    far_plane = max(min_near_plane + min_near_to_far, dmax + min_padding);
}

struct GPU_Mesh{
    GL::Buffer buffer;
    GL::Vertex_Array vao;
};

GPU_Mesh mesh_to_gpu(const Mesh& mesh, GLint gpu_alignment){
    size_t indices_offset = 0u;
    size_t indices_bytesize = mesh.nindices * sizeof(u16);
    size_t positions_offset = indices_offset + indices_bytesize + align_offset_next(indices_offset + indices_bytesize, gpu_alignment);
    size_t positions_bytesize = mesh.nvertices * sizeof(vec3);
    size_t normals_offset = positions_offset + positions_bytesize + align_offset_next(positions_offset + positions_bytesize, gpu_alignment);
    size_t normals_bytesize = mesh.nvertices * sizeof(vec3);
    size_t storage_bytesize = normals_offset + normals_bytesize;

    GL::Buffer storage;
    glCreateBuffers(1u, &storage);

    glNamedBufferStorage(storage, storage_bytesize, nullptr, GL_MAP_WRITE_BIT | GL_CLIENT_STORAGE_BIT);
    void* storage_ptr = glMapNamedBufferRange(storage, 0u, storage_bytesize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    memcpy(storage_ptr, mesh.indices, indices_bytesize);
    memcpy((void*)((u8*)storage_ptr + positions_offset), mesh.positions, positions_bytesize);
    memcpy((void*)((u8*)storage_ptr + normals_offset), mesh.normals, normals_bytesize);
    glUnmapNamedBuffer(storage);

    GL::Vertex_Array vao;
    glCreateVertexArrays(1u, &vao);

    glVertexArrayElementBuffer(vao, storage);
    glVertexArrayVertexBuffer(vao, 0u, storage, positions_offset, sizeof(vec3));
    glVertexArrayVertexBuffer(vao, 1u, storage, normals_offset, sizeof(vec3));

    glEnableVertexArrayAttrib(vao, 0u);
    glEnableVertexArrayAttrib(vao, 1u);
    glVertexArrayAttribFormat(vao, 0u, 3u, GL_FLOAT, GL_FALSE, 0u);
    glVertexArrayAttribFormat(vao, 1u, 3u, GL_FLOAT, GL_FALSE, 0u);
    glVertexArrayAttribBinding(vao, 0u, 0u);
    glVertexArrayAttribBinding(vao, 1u, 1u);

    return {storage, vao};
}

#if 0
void resample_equirectangular_to_cubemap(float* equi_data, u32 equi_width, equi_height, u32 cubemap_width, u32 cubemap_height){
    assert(cubemap_width != 0u && cubemap_height != 0u);

    size_t face_bytesize = cubemap_width * cubemap_height * sizeof(float);
    void* cubemap_ptr = malloc(6u * face_bytesize);

    float* faces[6u] = {
        cubemap_ptr,
        cubemap_ptr + face_bytesize,
        cubemap_ptr + face_bytesize * 2u
        cubemap_ptr + face_bytesize * 4u
        cubemap_ptr + face_bytesize * 4u
        cubemap_ptr + face_bytesize * 5u
    };

    for(u32 iface = 0u; iface != 6u; ++iface){
        for(u32 iw = 0u; iw != cubemap_width; ++iw){
            for(u32 ih = 0u; ih != cubemap_height; ++ih){
                vec3 cartesian = cubemap_to_cartesian(iface, (float)iw / (float)(cubemap_width - 1u), (float)ih / (float)(cubemap_height - 1u));
                vec2 cartesian


            }
        }
    }
}
#endif

int main(int argc, char* argv[]){

	// ---- initialization ---- //

	SDL_CHECK(SDL_Init(SDL_INIT_EVERYTHING) == 0);
    setup_timer();
    setup_LOG();

    stbi_set_flip_vertically_on_load(true);

    // ---- window

    const char* window_name = "default_main";
	Window_Settings window_settings;
	window_settings.name = window_name;
	window_settings.width = 1280;
	window_settings.height = 720;
	window_settings.mode = Window_Settings::mode_windowed;
	window_settings.synchronization = Window_Settings::synchronize;
	window_settings.buffering = Window_Settings::buffering_double;
	window_settings.OpenGL_major = 4u;
	window_settings.OpenGL_minor = 6u;

	Window window;
	window.initialize(window_settings);

    Frame_Timing timing;
    timing.initialize(timer_ticks(), timer_frequency(), 60u, 0.05, 10u, 10u);

    // ---- rendering

	gl3wInit();
    GL::set_debug_message_callback();
    glClearDepth(1.f);
	glClearColor(0.f, 0.f, 0.f, 1.f);
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

    //glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //glEnable(GL_FRAMEBUFFER_SRGB);
    //glViewport(0, 0, window.width, window.height);

    // ---- input

    Keyboard_State keyboard = {};
    keyboard.initialize();

    Mouse_State mouse;

    // ---- state

	bool running = true;

    // ---- developper tools

    DEV_setup();

	// ---- main loop ---- //

    GLint alignment;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);

    Mesh sphere = generate_sphere_uv({0.f, 2.f, 0.f}, 1., 100u, 100u);
    GPU_Mesh sphere_gpu = mesh_to_gpu(sphere, alignment);
    Mesh ground = generate_cuboid({0.f, 0.f, 0.f}, {5.f, 0.1f, 5.f});
    GPU_Mesh ground_gpu = mesh_to_gpu(ground, alignment);

    vec3 scene_center = {0.f, 0.f, 0.f};
    float scene_radius = 4.f;

    const char* vertex_shader_material = R"(
        #version 460 core

        layout(location = 0) in vec3 in_position;
        layout(location = 1) in vec3 in_normal;

        out vertex_out{
            vec3 worldspace_position;
            vec3 normal;
        } out_data;

        layout(std140, binding = 0) uniform u_Camera{
            vec4 worldspace_position;
            mat4 VP;
        } camera;

        void main(){
            gl_Position = camera.VP * vec4(in_position, 1.);
            out_data.worldspace_position = in_position;
            out_data.normal = in_normal;
        }
    )";
    const char* fragment_shader_material = R"(
        #version 460 core

        in vertex_out{
            vec3 worldspace_position;
            vec3 normal;
        } in_data;

        layout(location = 0) out vec4 out_color;

        layout(std140, binding = 0) uniform u_Camera{
            vec4 worldspace_position;
            mat4 VP;
        } camera;

        layout(std140, binding = 1) uniform u_Material{
            vec4 color_roughness;
            vec2 reflectance_metalness;
        } material;

        layout(std140, binding = 2) uniform u_Shadowmap_Light{
            mat4 VP;
            vec4 worldspace_direction_texel_size;
            vec4 radiance;
        } shadowmap_light;
        layout(binding = 0) uniform sampler2DShadow shadowmap_light_depth_map;

        // ---- constants

        const float PI = 3.14159265358979323846;
        const float EPSILON = 0.00001;

        // NOTE(hugo): depends on the shadowmap texture resolution - should probably be tweaked depending on the cascade
        const float depth_bias_magic_constant = 4.5;

        const float normal_scaling_magic_constant = 1.;
        const float planar_depth_bias_magic_constant = 1.;

        // NOTE(hugo): average specular of a dielectric material
        const float specular_dielectric = 0.4;

        // ----

        float shadowmap_sample_optimized_pcf(sampler2DShadow sampler_depth_map, vec2 base_uv, float u, float v, vec2 shadowmap_size_inv, float depth, vec2 planar_depth_bias){
            vec2 duv = vec2(u, v) * shadowmap_size_inv;

            vec2 uv = base_uv + duv;
            float planar_depth = depth + planar_depth_bias_magic_constant * dot(duv, planar_depth_bias);

            return texture(sampler_depth_map, vec3(uv, planar_depth));
        }

        // NOTE(hugo): returns the percent of light
        float shadowmap_optimized_pcf(sampler2DShadow sampler_depth_map, vec3 pos, vec2 planar_depth_bias){
            ivec2 shadowmap_size = textureSize(sampler_depth_map, 0);
            vec2 shadowmap_size_inv = vec2(1.) / shadowmap_size;

            float depth_bias = - depth_bias_magic_constant * dot(shadowmap_size_inv, abs(planar_depth_bias));

            vec2 uv = pos.xy * shadowmap_size;

            vec2 base_uv;
            base_uv.x = floor(uv.x + 0.5);
            base_uv.y = floor(uv.y + 0.5);

            float s = uv.x + 0.5 - base_uv.x;
            float t = uv.y + 0.5 - base_uv.y;

            base_uv -= vec2(0.5);
            base_uv *= shadowmap_size_inv;

            float uw0 = (5 * s - 6);
            float uw1 = (11 * s - 28);
            float uw2 = -(11 * s + 17);
            float uw3 = -(5 * s + 1);

            float u0 = (4 * s - 5) / uw0 - 3;
            float u1 = (4 * s - 16) / uw1 - 1;
            float u2 = -(7 * s + 5) / uw2 + 1;
            float u3 = -s / uw3 + 3;

            float vw0 = (5 * t - 6);
            float vw1 = (11 * t - 28);
            float vw2 = -(11 * t + 17);
            float vw3 = -(5 * t + 1);

            float v0 = (4 * t - 5) / vw0 - 3;
            float v1 = (4 * t - 16) / vw1 - 1;
            float v2 = -(7 * t + 5) / vw2 + 1;
            float v3 = -t / vw3 + 3;

            float shadow = 0.;

            shadow += uw0 * vw0 * shadowmap_sample_optimized_pcf(sampler_depth_map, base_uv, u0, v0, shadowmap_size_inv, pos.z + depth_bias, planar_depth_bias);
            shadow += uw1 * vw0 * shadowmap_sample_optimized_pcf(sampler_depth_map, base_uv, u1, v0, shadowmap_size_inv, pos.z + depth_bias, planar_depth_bias);
            shadow += uw2 * vw0 * shadowmap_sample_optimized_pcf(sampler_depth_map, base_uv, u2, v0, shadowmap_size_inv, pos.z + depth_bias, planar_depth_bias);
            shadow += uw3 * vw0 * shadowmap_sample_optimized_pcf(sampler_depth_map, base_uv, u3, v0, shadowmap_size_inv, pos.z + depth_bias, planar_depth_bias);

            shadow += uw0 * vw1 * shadowmap_sample_optimized_pcf(sampler_depth_map, base_uv, u0, v1, shadowmap_size_inv, pos.z + depth_bias, planar_depth_bias);
            shadow += uw1 * vw1 * shadowmap_sample_optimized_pcf(sampler_depth_map, base_uv, u1, v1, shadowmap_size_inv, pos.z + depth_bias, planar_depth_bias);
            shadow += uw2 * vw1 * shadowmap_sample_optimized_pcf(sampler_depth_map, base_uv, u2, v1, shadowmap_size_inv, pos.z + depth_bias, planar_depth_bias);
            shadow += uw3 * vw1 * shadowmap_sample_optimized_pcf(sampler_depth_map, base_uv, u3, v1, shadowmap_size_inv, pos.z + depth_bias, planar_depth_bias);

            shadow += uw0 * vw2 * shadowmap_sample_optimized_pcf(sampler_depth_map, base_uv, u0, v2, shadowmap_size_inv, pos.z + depth_bias, planar_depth_bias);
            shadow += uw1 * vw2 * shadowmap_sample_optimized_pcf(sampler_depth_map, base_uv, u1, v2, shadowmap_size_inv, pos.z + depth_bias, planar_depth_bias);
            shadow += uw2 * vw2 * shadowmap_sample_optimized_pcf(sampler_depth_map, base_uv, u2, v2, shadowmap_size_inv, pos.z + depth_bias, planar_depth_bias);
            shadow += uw3 * vw2 * shadowmap_sample_optimized_pcf(sampler_depth_map, base_uv, u3, v2, shadowmap_size_inv, pos.z + depth_bias, planar_depth_bias);

            shadow += uw0 * vw3 * shadowmap_sample_optimized_pcf(sampler_depth_map, base_uv, u0, v3, shadowmap_size_inv, pos.z + depth_bias, planar_depth_bias);
            shadow += uw1 * vw3 * shadowmap_sample_optimized_pcf(sampler_depth_map, base_uv, u1, v3, shadowmap_size_inv, pos.z + depth_bias, planar_depth_bias);
            shadow += uw2 * vw3 * shadowmap_sample_optimized_pcf(sampler_depth_map, base_uv, u2, v3, shadowmap_size_inv, pos.z + depth_bias, planar_depth_bias);
            shadow += uw3 * vw3 * shadowmap_sample_optimized_pcf(sampler_depth_map, base_uv, u3, v3, shadowmap_size_inv, pos.z + depth_bias, planar_depth_bias);

            return shadow * 1.0f / 2704;
        }

        // ----

        // NOTE(hugo): approximate cook-torrance's distribution function
        float trowbridge_reitz_GGX(float roughness, float dot_H_N){
            float a = dot_H_N * roughness;
            float k = roughness / (1. - dot_H_N * dot_H_N + a * a);
            return k * k * (1. / PI);
        }

        // NOTE(hugo): approximate cook-torrance's fresnel equation
        vec3 schlick_fresnel(vec3 F0, float dot_L_N){
            float f = pow(1. - dot_L_N, 5.);
            return vec3(f) + F0 * (1. - f);
        }

        // NOTE(hugo): approximate cook-torrance's geometric function
        float smith_GGX(float roughness, float dot_L_N, float dot_V_N){
            float GGX_V = dot_L_N * (dot_V_N * (1. - roughness) + roughness);
            float GGX_L = dot_V_N * (dot_L_N * (1. - roughness) + roughness);
            return 0.5 / (GGX_V + GGX_L);
        }

        // ----

        void main(){
            vec3 V = normalize(camera.worldspace_position.xyz - in_data.worldspace_position);
            vec3 N = in_data.normal;
            float dot_V_N = abs(dot(V, N)) + EPSILON;

            vec3 shadowmap_lighting;
            {
                vec3 L = - shadowmap_light.worldspace_direction_texel_size.xyz;
                vec3 H = (L + V) * 0.5;

                float dot_L_N = clamp(dot(L, N), 0., 1.);
                float dot_H_N = clamp(dot(H, N), 0., 1.);
                float dot_L_H = clamp(dot(L, H), 0., 1.);

                // NOTE(hugo): normal bias
                float angular_scaling = sqrt(1. - dot_L_N * dot_L_N);
                float depth_scaling = angular_scaling / dot_L_N;
                float normal_scaling = normal_scaling_magic_constant * shadowmap_light.worldspace_direction_texel_size.w * angular_scaling;
                vec3 worldspace_position_normal_scaled = in_data.worldspace_position + N * normal_scaling;

                // NOTE(hugo): shadowmap reprojection
                vec4 shadowmap_position = shadowmap_light.VP * vec4(worldspace_position_normal_scaled, 1.);
                shadowmap_position.xyz = shadowmap_position.xyz / shadowmap_position.w;

                // NOTE(hugo): shadowmap light coverage
                float light_percent;
                {
                    shadowmap_position.xy = shadowmap_position.xy * 0.5 + vec2(0.5);
                    shadowmap_position.z = min(shadowmap_position.z, 1.);

                    // NOTE(hugo): derivatives of u, v and depth wrt. x & y
                    vec3 duvdepth_dx = dFdxFine(shadowmap_position.xyz);
                    vec3 duvdepth_dy = dFdyFine(shadowmap_position.xyz);

                    // NOTE(hugo): compute derivatives of depth wrt. u & v
                    vec2 ddepth_duv;
                    ddepth_duv.x = duvdepth_dy.y * duvdepth_dx.z - duvdepth_dx.y * duvdepth_dy.z;
                    ddepth_duv.y = duvdepth_dx.x * duvdepth_dy.z - duvdepth_dy.x * duvdepth_dx.z;
                    ddepth_duv *= 1. / ((duvdepth_dx.x * duvdepth_dy.y) - (duvdepth_dx.y * duvdepth_dy.x));

                    // NOTE(hugo): sample shadowmap
                    light_percent = shadowmap_optimized_pcf(shadowmap_light_depth_map, shadowmap_position.xyz, ddepth_duv);
                    if(dot(N, L) < 0.) light_percent = 0.;
                }

                // NOTE(hugo): parametrization remapping
                float roughness = material.color_roughness.w * material.color_roughness.w;
                vec3 diffuse_color = (1. - material.reflectance_metalness.y) * material.color_roughness.xyz;
                float F0_dielectric = 0.16 * material.reflectance_metalness.x * material.reflectance_metalness.x * (1. - material.reflectance_metalness.y);
                vec3 F0_metallic = material.color_roughness.xyz * material.reflectance_metalness.y;
                vec3 F0 = F0_dielectric + F0_metallic;

                // NOTE(hugo): PBR
                // NOTE(hugo): V = G / (2 * dot_L_N * dot_V_N)
                vec3 F = schlick_fresnel(F0, dot_L_H);
                float D = trowbridge_reitz_GGX(roughness, dot_H_N);
                float V = smith_GGX(roughness, dot_L_N, dot_V_N);
                vec3 specular_BRDF = F * (D * V);

                vec3 diffuse_BRDF = diffuse_color * vec3(1. / PI);

                shadowmap_lighting = (diffuse_BRDF + specular_BRDF) * shadowmap_light.radiance.xyz * light_percent * dot_L_N;
            }

            vec3 ambiant_lighting = vec3(0.);

            out_color = vec4(shadowmap_lighting + ambiant_lighting, 1.);
        }
    )";
    GL::Program program_material = GL::create_program(GL_VERTEX_SHADER, vertex_shader_material, GL_FRAGMENT_SHADER, fragment_shader_material);

    const char* vertex_shader_shadowmap = R"(
        #version 460 core

        layout(location = 0) in vec3 in_xyz;

        layout(std140, location = 0, binding = 0) uniform u_Shadowmap_Camera{
            mat4 VP;
        } shadowmap_camera;

        void main(){
            gl_Position = shadowmap_camera.VP * vec4(in_xyz, 1.);
        }
    )";
    const char* fragment_shader_shadowmap = R"(
        #version 460 core
        void main(){
        }
    )";
    GL::Program program_shadowmap = GL::create_program(GL_VERTEX_SHADER, vertex_shader_shadowmap, GL_FRAGMENT_SHADER, fragment_shader_shadowmap);

    const char* vertex_shader_tonemapping = R"(
        #version 460 core

        out vertex_out{
            vec2 texcoord;
        } out_data;

        void main(){
            vec2 screenpos = vec2(
                -1. + float((gl_VertexID & 1) << 2),
                -1. + float((gl_VertexID & 2) << 2)
            );

            gl_Position = vec4(screenpos.xy, 0.5, 1.);
            out_data.texcoord = vec2(
                (screenpos.x + 1.) * 0.5,
                (screenpos.y + 1.) * 0.5
            );
        }
    )";
    const char* fragment_shader_tonemapping = R"(
        #version 460 core

        in vertex_out{
            vec2 texcoord;
        } in_data;

        layout(location = 0) out vec4 out_color;

        layout(binding = 0) uniform sampler2D hdr_texture;

        // ----

        vec3 tone_mapping_ACES(vec3 color){
            const float A = 2.51;
            const float B = 0.03;
            const float C = 2.43;
            const float D = 0.59;
            const float E = 0.14;

            vec3 mapped = (color * (A * color + B)) / (color * (C * color + D) + E);
            return clamp(mapped, 0., 1.);
        }

        vec3 tone_mapping_Uncharted2(vec3 x){
            const float A = 0.15;
            const float B = 0.5;
            const float C = 0.1;
            const float D = 0.2;
            const float E = 0.02;
            const float F = 0.3;
            const float W = 11.2;

            return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
        }

        // ----

        void main(){
            vec3 hdr = texture(hdr_texture, in_data.texcoord).rgb;

            //vec3 color = tone_mapping_Uncharted2(hdr);
            vec3 color = tone_mapping_ACES(hdr);

            out_color = vec4(color, 1.);
        }
    )";
    GL::Program program_tonemapping = GL::create_program(GL_VERTEX_SHADER, vertex_shader_tonemapping, GL_FRAGMENT_SHADER, fragment_shader_tonemapping);

    GL::Buffer ubo_camera;
    glCreateBuffers(1u, &ubo_camera);
    glNamedBufferStorage(ubo_camera, sizeof(u_Camera), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
    u_Camera* ubo_camera_ptr = (u_Camera*)glMapNamedBufferRange(ubo_camera, 0u, sizeof(u_Camera), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

    GL::Buffer ubo_material;
    glCreateBuffers(1u, &ubo_material);
    glNamedBufferStorage(ubo_material, sizeof(u_Material), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
    u_Material* ubo_material_ptr = (u_Material*)glMapNamedBufferRange(ubo_material, 0u, sizeof(u_Material), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

    GL::Buffer ubo_shadowmap_camera;
    glCreateBuffers(1u, &ubo_shadowmap_camera);
    glNamedBufferStorage(ubo_shadowmap_camera, sizeof(u_Shadowmap_Light), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
    u_Shadowmap_Light* ubo_shadowmap_light_ptr = (u_Shadowmap_Light*)glMapNamedBufferRange(ubo_shadowmap_camera, 0u, sizeof(u_Shadowmap_Light), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

    GL::Buffer ubo_tone_mapping;
    glCreateBuffers(1u, &ubo_tone_mapping);
    glNamedBufferStorage(ubo_tone_mapping, sizeof(u_Tone_Mapping), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
    u_Tone_Mapping* ubo_tone_mapping_ptr = (u_Tone_Mapping*)glMapNamedBufferRange(ubo_tone_mapping, 0u, sizeof(u_Tone_Mapping), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

    constexpr u32 shadowmap_dim = 4096u;

#if 0
    // NOTE(hugo): equirectangular ibl source

    s32 ibl_width;
    s32 ibl_height;
    s32 ibl_channels;
    float* ibl_data = stbi_loadf("data/texture/checker4.png", &ibl_width, &ibl_height, &ibl_channels, 3u);
    ENGINE_CHECK(ibl_data != NULL, "FAILED to stbi_loadf");

    GL::Texture ibl_source;
    glCreateTextures(GL_TEXTURE_2D, 1u, &ibl_source);
    glTextureStorage2D(ibl_source, 1u, GL_RGB16F, ibl_width, ibl_height);
    glTextureSubImage2D(ibl_source, 0u, 0u, 0u, GL_RGBA, GL_FLOAT, ibl_data);

    stbi_image_free(ibl_data);

    // NOTE(hugo): conversion to cubemap

    //equirectangular_to_cubemap(ibl_data, (u32)ibl_width, (u32)ibl_height);

    GL::Texture ibl_sampler;
    glSamplerParameteri(shadowmap_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(shadowmap_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(shadowmap_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(shadowmap_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif

    GL::Texture shadowmap_depth;
    glCreateTextures(GL_TEXTURE_2D, 1u, &shadowmap_depth);
    glTextureStorage2D(shadowmap_depth, 1u, GL_DEPTH_COMPONENT32, shadowmap_dim, shadowmap_dim);

    GL::Sampler shadowmap_sampler;
    glCreateSamplers(1u, &shadowmap_sampler);
    glSamplerParameteri(shadowmap_sampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glSamplerParameteri(shadowmap_sampler, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glSamplerParameteri(shadowmap_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(shadowmap_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(shadowmap_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glSamplerParameteri(shadowmap_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float shadowmap_border[4] = {1.f, 1.f, 1.f, 1.f};
    glSamplerParameterfv(shadowmap_sampler, GL_TEXTURE_BORDER_COLOR, shadowmap_border);

    GL::Framebuffer fbo_shadowmap;
    glCreateFramebuffers(1, &fbo_shadowmap);
    glNamedFramebufferTexture(fbo_shadowmap, GL_DEPTH_ATTACHMENT, shadowmap_depth, 0u);
    GLenum fbo_shadowmap_draw_buffer = GL_NONE;
    glNamedFramebufferDrawBuffers(fbo_shadowmap, 1u, &fbo_shadowmap_draw_buffer);

    GL::Texture offscreen_hdr;
    glCreateTextures(GL_TEXTURE_2D, 1u, &offscreen_hdr);
    glTextureStorage2D(offscreen_hdr, 1u, GL_RGBA16F, window.width, window.height);

    GL::Sampler tonemapping_sampler;
    glCreateSamplers(1u, &tonemapping_sampler);
    glSamplerParameteri(tonemapping_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(tonemapping_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GL::Texture offscreen_depth;
    glCreateTextures(GL_TEXTURE_2D, 1u, &offscreen_depth);
    glTextureStorage2D(offscreen_depth, 1u, GL_DEPTH_COMPONENT32, window.width, window.height);

    GL::Framebuffer fbo_hdr;
    glCreateFramebuffers(1, &fbo_hdr);
    glNamedFramebufferTexture(fbo_hdr, GL_COLOR_ATTACHMENT0, offscreen_hdr, 0u);
    glNamedFramebufferTexture(fbo_hdr, GL_DEPTH_ATTACHMENT, offscreen_depth, 0u);

    GLint fbo_screen;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fbo_screen);

    // ----

    // NOTE(hugo): shadowmap setup
    {
        vec3 shadow_direction = {1.f, -1.f, -1.f};
        shadow_direction = normalized(shadow_direction);

        Camera_3D_Orbit camera;
        camera.orbit_center = scene_center;
        camera.orbit_radius = 2.f * scene_radius;
        //camera.vertical_fov = to_radians(90.f);
        camera.vertical_span = 4.f * scene_radius;
        camera.aspect_ratio = 1.f;

        camera.view_quaternion = inverse(quat_from_vector_to_vector(camera.forward(), shadow_direction));

        clipping_planes_from_sphere(camera.near_plane, camera.far_plane,
                camera.position(), camera.forward(), scene_center, scene_radius);

        mat4 shadowmap_matrix = camera.orthographic_matrix() * camera.view_matrix();

        ubo_shadowmap_light_ptr->VP = to_std140(shadowmap_matrix);
        ubo_shadowmap_light_ptr->worldspace_direction_texel_size.x = shadow_direction.x;
        ubo_shadowmap_light_ptr->worldspace_direction_texel_size.y = shadow_direction.y;
        ubo_shadowmap_light_ptr->worldspace_direction_texel_size.z = shadow_direction.z;
        ubo_shadowmap_light_ptr->worldspace_direction_texel_size.w = (float)camera.vertical_span / (float)shadowmap_dim;
        ubo_shadowmap_light_ptr->radiance = {3., 3., 3., 0.};
    }
    glFlushMappedNamedBufferRange(ubo_shadowmap_camera, 0u, sizeof(u_Shadowmap_Light));

    // NOTE(hugo): camera setup

    Camera_2D camera_2D;
    camera_2D.center = {0.f, 0.f};
    camera_2D.height = 2.f;
    camera_2D.aspect_ratio = window.aspect_ratio();

    Camera_3D_Orbit camera_3D_orbit;
    camera_3D_orbit.orbit_center = {0.f, 0.f, 0.f};
    camera_3D_orbit.orbit_radius = 5.f;
    camera_3D_orbit.vertical_fov = to_radians(90.f);
    //camera_3D_orbit.vertical_span = 2.f;
    camera_3D_orbit.aspect_ratio = window.aspect_ratio();
    clipping_planes_from_sphere(camera_3D_orbit.near_plane, camera_3D_orbit.far_plane,
            camera_3D_orbit.position(), camera_3D_orbit.forward(), scene_center, scene_radius);

    Camera_3D_FP camera_3D_fp;
    camera_3D_fp.position = {0.f, 0.f, 5.f};
    camera_3D_fp.vertical_fov = to_radians(90.f);
    //camera_3D_fp.vertical_span = 2.f;
    camera_3D_fp.aspect_ratio = window.aspect_ratio();
    clipping_planes_from_sphere(camera_3D_fp.near_plane, camera_3D_fp.far_plane,
            camera_3D_fp.position, camera_3D_fp.forward(), scene_center, scene_radius);

    u32 camera_type = 0u;

    // NOTE(hugo): mainloop

	while(running){
        //DEV_LOG_frame_duration;

        // ---- update

        u32 nupdates = timing.nupdates_before_render(timer_ticks());
        for(u32 iupdate = 0; iupdate != nupdates; ++iupdate){
            SDL_Event event;
            while(SDL_PollEvent(&event)){
                switch(event.type){
                    case SDL_QUIT:
                    {
                        running = false;
                        break;
                    }
                    default:
                        break;
                }

                keyboard.register_event(event);
                mouse.register_event(event);
            }

            // ---- process the frame

            if(keyboard.number_1.npressed) camera_type = 0;
            else if(keyboard.number_2.npressed) camera_type = 1;
            else if(keyboard.number_3.npressed) camera_type = 2;

            constexpr float velocity = 0.2f;
            constexpr float angular_velocity = 2.5f;

            if(camera_type == 0u){
                if(keyboard.arrow_left.state == Device_Button::STATE_DOWN){
                    camera_2D.center.x += velocity;
                }
                if(keyboard.arrow_right.state == Device_Button::STATE_DOWN){
                    camera_2D.center.x += - velocity;
                }
                if(keyboard.arrow_up.state == Device_Button::STATE_DOWN){
                    camera_2D.center.y += - velocity;
                }
                if(keyboard.arrow_down.state == Device_Button::STATE_DOWN){
                    camera_2D.center.y += velocity;
                }
            }else if(camera_type == 1u){
                if(keyboard.arrow_left.state == Device_Button::STATE_DOWN){
                    camera_3D_orbit.rotate(0.f, - to_radians(angular_velocity), 0.f);
                }
                if(keyboard.arrow_right.state == Device_Button::STATE_DOWN){
                    camera_3D_orbit.rotate(0.f, to_radians(angular_velocity), 0.f);
                }
                if(keyboard.arrow_up.state == Device_Button::STATE_DOWN){
                    camera_3D_orbit.rotate(- to_radians(angular_velocity), 0.f, 0.f);
                }
                if(keyboard.arrow_down.state == Device_Button::STATE_DOWN){
                    camera_3D_orbit.rotate(to_radians(angular_velocity), 0.f, 0.f);
                }
                if(keyboard.key_w.state == Device_Button::STATE_DOWN){
                    camera_3D_orbit.orbit_center += velocity * camera_3D_orbit.forward();
                }
                if(keyboard.key_s.state == Device_Button::STATE_DOWN){
                    camera_3D_orbit.orbit_center += - velocity * camera_3D_orbit.forward();
                }
                if(keyboard.key_a.state == Device_Button::STATE_DOWN){
                    camera_3D_orbit.orbit_center += - velocity * camera_3D_orbit.right();
                }
                if(keyboard.key_d.state == Device_Button::STATE_DOWN){
                    camera_3D_orbit.orbit_center += velocity * camera_3D_orbit.right();
                }

                clipping_planes_from_sphere(camera_3D_orbit.near_plane, camera_3D_orbit.far_plane,
                        camera_3D_orbit.position(), camera_3D_orbit.forward(), scene_center, scene_radius);

            }else if(camera_type == 2u){
                if(keyboard.arrow_left.state == Device_Button::STATE_DOWN){
                    camera_3D_fp.rotate(0.f, - to_radians(angular_velocity), 0.f);
                }
                if(keyboard.arrow_right.state == Device_Button::STATE_DOWN){
                    camera_3D_fp.rotate(0.f, to_radians(angular_velocity), 0.f);
                }
                if(keyboard.arrow_up.state == Device_Button::STATE_DOWN){
                    camera_3D_fp.rotate(- to_radians(angular_velocity), 0.f, 0.f);
                }
                if(keyboard.arrow_down.state == Device_Button::STATE_DOWN){
                    camera_3D_fp.rotate(to_radians(angular_velocity), 0.f, 0.f);
                }
                if(keyboard.key_w.state == Device_Button::STATE_DOWN){
                    camera_3D_fp.position += velocity * camera_3D_fp.forward();
                }
                if(keyboard.key_s.state == Device_Button::STATE_DOWN){
                    camera_3D_fp.position -= velocity * camera_3D_fp.forward();
                }
                if(keyboard.key_a.state == Device_Button::STATE_DOWN){
                    camera_3D_fp.position -= velocity * camera_3D_fp.right();
                }
                if(keyboard.key_d.state == Device_Button::STATE_DOWN){
                    camera_3D_fp.position += velocity * camera_3D_fp.right();
                }

                clipping_planes_from_sphere(camera_3D_fp.near_plane, camera_3D_fp.far_plane,
                        camera_3D_fp.position, camera_3D_fp.forward(), scene_center, scene_radius);
            }

            // ---- setup the next game frame

            keyboard.next_frame();
            mouse.next_frame();
        }

        // ---- render

        // NOTE(hugo): shadowmapping pass

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glViewport(0u, 0u, shadowmap_dim, shadowmap_dim);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_shadowmap);
        glClear(GL_DEPTH_BUFFER_BIT);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0u, ubo_shadowmap_camera);
        glUseProgram(program_shadowmap);
        glBindVertexArray(sphere_gpu.vao);
        glDrawElements(GL_TRIANGLES, sphere.nindices, GL_UNSIGNED_SHORT, 0u);
        glBindVertexArray(ground_gpu.vao);
        glDrawElements(GL_TRIANGLES, ground.nindices, GL_UNSIGNED_SHORT, 0u);

        // NOTE(hugo): offscreen rendering pass

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glViewport(0u, 0u, window.width, window.height);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_hdr);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(camera_type == 0u){
            ubo_camera_ptr->worldspace_position = {camera_2D.center.x, camera_2D.center.y, scene_radius, 0.};
            ubo_camera_ptr->VP = to_std140(mat3D_from_mat2D(camera_2D.camera_matrix()));
        }else if(camera_type == 1u){
            vec3 camera_position = camera_3D_orbit.position();
            ubo_camera_ptr->worldspace_position = {camera_position.x, camera_position.y, camera_position.z, 0.};
            ubo_camera_ptr->VP = to_std140(camera_3D_orbit.perspective_matrix() * camera_3D_orbit.view_matrix());
        }else if(camera_type == 2u){
            ubo_camera_ptr->worldspace_position = {camera_3D_fp.position.x, camera_3D_fp.position.y, camera_3D_fp.position.z, 0.};
            ubo_camera_ptr->VP = to_std140(camera_3D_fp.perspective_matrix() * camera_3D_fp.view_matrix());
        }
        glFlushMappedNamedBufferRange(ubo_camera, 0u, sizeof(u_Camera));


        glBindBufferBase(GL_UNIFORM_BUFFER, 0u, ubo_camera);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1u, ubo_material);
        glBindBufferBase(GL_UNIFORM_BUFFER, 2u, ubo_shadowmap_camera);
        glBindTextureUnit(0u, shadowmap_depth);
        glBindSampler(0u, shadowmap_sampler);
        glUseProgram(program_material);

        ubo_material_ptr->color_roughness.x = 1.f;
        ubo_material_ptr->color_roughness.y = 0.766f;
        ubo_material_ptr->color_roughness.z = 0.336f;
        ubo_material_ptr->color_roughness.w = 0.6f;
        ubo_material_ptr->reflectance_metalness.x = 0.f;
        ubo_material_ptr->reflectance_metalness.y = 1.f;
        glFlushMappedNamedBufferRange(ubo_material, 0u, sizeof(u_Material));

        glBindVertexArray(sphere_gpu.vao);
        glDrawElements(GL_TRIANGLES, sphere.nindices, GL_UNSIGNED_SHORT, 0u);

        glBindVertexArray(ground_gpu.vao);
        glDrawElements(GL_TRIANGLES, ground.nindices, GL_UNSIGNED_SHORT, 0u);

        // NOTE(hugo): screen rendering pass

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glEnable(GL_FRAMEBUFFER_SRGB);
        glViewport(0u, 0u, window.width, window.height);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_screen);
        glClear(GL_COLOR_BUFFER_BIT);

        ubo_tone_mapping_ptr->exposure = 2.;
        glFlushMappedNamedBufferRange(ubo_tone_mapping, 0u, sizeof(u_Tone_Mapping));

        glUseProgram(program_tonemapping);
        glBindTextureUnit(0u, offscreen_hdr);
        glBindSampler(0u, tonemapping_sampler);

        glDrawArrays(GL_TRIANGLES, 0u, 3u);

        glDisable(GL_FRAMEBUFFER_SRGB);

        // ---- setup the next rendering frame

		window.swap_buffers();

        DEV_LOG_timing_entries();
        DEV_next_frame();
	}

	// ---- termination ---- //

	window.terminate();
	SDL_Quit();

    DEV_terminate();

    return EXIT_SUCCESS;
}
