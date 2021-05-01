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

struct Geometry_Mesh_3D{
    void create(u32 nvertices, u32 nindices);
    void destroy();

    u32 nvertices = 0u;
    u32 nindices = 0u;

    vec3* positions = nullptr;
    vec3* normals = nullptr;
    u16* indices = nullptr;
};

// NOTE(hugo): explicit free required
Geometry_Mesh_3D generate_sphere_uv(vec3 position, float radius, u32 nlon, u32 nlat);
Geometry_Mesh_3D generate_cube(vec3 position, float size, u32 ntess = 0u);
Geometry_Mesh_3D generate_cuboid(vec3 position, vec3 size, u32 ntess = 0u);

// ---- geometric predicates

// NOTE(hugo):
// * > 0.f means p is on the RIGHT of (vA, vB)
// * = 0.f means p is on (vA, vB)
// * < 0.f means p is on the LEFT of (vA, vB)
float point_side_segment(const vec2& p, const vec2& vA, const vec2& vB);

bool point_inside_triangle(const vec2& p, const vec2& tA, const vec2& tB, const vec2& tC);

bool line_intersect_line(const vec2& pA, const vec2& dA, const vec2& pB, const vec2& dB, vec2& out);

// NOTE(hugo):
// * > 0.f means the polygon winding order is CLOCKWISE
// * < 0.f means the polygon winding order is COUNTER-CLOCKWISE
float polygon_signed_area(const u32 nvertices, const vec2* vertices);

// ---- triangulation

// REF(hugo):
// https://www.geometrictools.com/Documentation/TriangulationByEarClipping.pdf

// NOTE(hugo): ear clipping triangulation
// * vertices are ordered in counter-clockwise order
// * requires a 'simple' polygon ie no overlapping edges (twist / loop) and no hole
// * out_indices is provided by the user with size 3 * (nvertices - 2u)
void triangulation_2D(u32 nvertices, vec2* vertices, u32* out_indices);

// ---- geometric error

// REF(hugo):
// - circle chord to arc error
// https://www.artwork.com/gerber/oasis2gbr/arc_recognition.htm

// NOTE(hugo):
// /ncap_vertices/ is the number of vertices in the spherical caps without counting the two body vertices
// ie ncap_vertices = 1 represents a triangular cap
float circular_cap_chord_to_arc_error(u32 ncap_vertices, float radius);
u32 circular_cap_vertices(float radius, float max_error);

// NOTE(hugo):
// /nvertices/ is the number of vertices approximating the circle
// nvertices must be 3 or more
float circle_chord_to_arc_error(u32 nvertices, float radius);
u32 circle_vertices(float radius, float max_error);

#endif
