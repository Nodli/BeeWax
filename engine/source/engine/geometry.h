#ifndef H_GEOMETRY
#define H_GEOMETRY

// ---- conventions

// NOTE(hugo):
// * vec2(x, y)
// * vec2(u, v)
// * vec2(theta, phi)
// * vec3(x, y, z)

// NOTE(hugo): world is in a right handed coordinate system
//             +Y  -Z (far)
//              | /
//              |/
//     -X <---- 0 ---> +X
//             /|
//            / |
//   (near) +Z -Y

// NOTE(hugo):
// * theta  the latitudinal  / parallel   angle in range [0, PI]     [0°; 180°] (N = 0°, S = 180°)
// * phi    the longitudinal / meridional angle in range [0, 2 * PI] [0°; 360°]

// NOTE(hugo): cubemap face order
// face_index     |     0    |     1    |     2    |     3    |     4    |     5    |
// face_direction |    +X    |    -X    |    +Y    |    -Y    |    +Z    |    -Z    |
// face_uv        | (-Z, +Y) | (+Z, +Y) | (+X, -Z) | (+X, +Z) | (+X, +Y) | (-X, +Y) |

// ---- coordinate conversion

// NOTE(hugo):
// _2PI [0, 2 * PI]
// _PI  [-PI, PI]
float radrange_2PI(float radians);
float radrange_PI(float radians);
float radrange_PI_to_2PI(float radians);
float radrange_2PI_to_PI(float radians);

// NOTE(hugo):
// (theta, phi) ([0, PI], [0, 2 * PI])
// (x, y, z)    normalized
vec3 spherical_to_cartesian(float theta, float phi);
vec2 cartesian_to_spherical(float x, float y, float z);

// NOTE(hugo):
// (theta, phi) ([0, PI], [0, 2 * PI])
// (x, y)       ([0, 2],            [0, 1])
vec2 spherical_to_equirectangular(float theta, float phi);
vec2 equirectangular_to_spherical(float x, float y);

// NOTE(hugo):
// (x, y, z)    normalized
// (u, v)       normalized
// face_index   [0u, 5u]
vec2 cartesian_to_cubemap(float x, float y, float z, u32& face_index);
vec3 cubemap_to_cartesian(float face_u, float face_v, u32 face_index);

// ---- mesh generation

struct Mesh{
    void set_capacity(u32 nvertices, u32 nindices);
    void free();

    u32 nvertices = 0u;
    u32 nindices = 0u;

    vec3* positions = nullptr;
    vec3* normals = nullptr;
    u16* indices = nullptr;
};

// NOTE(hugo): explicit free required
Mesh generate_sphere_uv(vec3 position, float radius, u32 nlon, u32 nlat);
Mesh generate_cube(vec3 position, float size, u32 ntess = 0u);
Mesh generate_cuboid(vec3 position, vec3 size, u32 ntess = 0u);

// ---- error estimation

// REF(hugo):
// - circle chord to arc error
// https://www.artwork.com/gerber/oasis2gbr/arc_recognition.htm

// NOTE(hugo):
// /ncap_vertices/ is the number of vertices in the spherical caps without counting the two body vertices
// ie ncap_vertices = 1 represents a triangular cap
float circular_cap_chord_to_arc_error(u32 ncap_vertices, float radius);
u32 circular_cap_vertices(float max_error, float radius);

float circle_chord_to_arc_error(u32 nperi_vertices, float radius);
u32 circle_vertices(float max_error, float radius);

#endif
