// ---- coordinate conversion

float radrange_2PI(float radians){
    radians = fmod(radians, 2.f * PI);
    if(radians < 0.f) radians += 2.f * PI;
    return radians;
}

float radrange_PI(float radians){
    return radrange_2PI(radians + PI) - PI;
}

float radrange_PI_to_2PI(float radians){
    if(radians < 0.f) radians = 2.f * PI + radians;
    return radians;
}

float radrange_2PI_to_PI(float radians){
    if(radians > PI) radians = radians - 2.f * PI;
    return radians;
}

vec3 spherical_to_cartesian(float theta, float phi){
    return {
        sin(theta) * sin(phi),
        cos(theta),
        sin(theta) * cos(phi),
    };
}

vec2 cartesian_to_spherical(float x, float y, float z){
    vec2 out;
    out.x = acos(y);

    constexpr float zaxis_threshold = 0.001f;
    if((abs(x) + abs(z)) < zaxis_threshold){
        out.y = 0.f;
    }else{
        out.y = radrange_PI_to_2PI(atan2(x, z));
    }

    return out;
}

vec2 spherical_to_equirectangular(float theta, float phi){
    return {
        phi * (1.f / PI),
        1.f - theta * (1.f / PI),
    };
}

vec2 equirectangular_to_spherical(float x, float y){
    return {
        (1.f - y) * PI,
        x * PI,
    };
}

vec2 cartesian_to_cubemap(float x, float y, float z, u32& face_index){
    float du;
    float dv;
    float ddepth;

    if(abs(x) >= abs(y) && abs(x) >= abs(z)){
        dv = y;
        if(x > 0.){
            du = z;
            ddepth = x;
            face_index = 0u;
        }else{
            du = -z;
            ddepth = -x;
            face_index = 1u;
        }
    }else if(abs(y) >= abs(x) && abs(y) >= abs(z)){
        du = x;
        if(y > 0.){
            dv = z;
            ddepth = y;
            face_index = 2u;
        }else{
            dv = -z;
            ddepth = -y;
            face_index = 3u;
        }
    }else{
        assert(abs(z) >= abs(y) && abs(z) >= abs(x));

        dv = y;
        if(z > 0.){
            du = -x;
            ddepth = z;
            face_index = 4u;
        }else{
            du = x;
            ddepth = -z;
            face_index = 5u;
        }
    }

    float denorm = 0.5f / ddepth;
    return {du * denorm + 0.5f, dv * denorm + 0.5f};
}

vec3 cubemap_to_cartesian(float face_u, float face_v, u32 face_index){
    float du = face_u - 0.5f;
    float dv = face_v - 0.5f;
    float ddepth = 0.5f;

    float norm = 1. / sqrt(du * du + dv * dv + ddepth * ddepth);
    du *= norm;
    dv *= norm;
    ddepth *= norm;

    switch(face_index){
        case 0: return { ddepth,    dv,         du};
        case 1: return {-ddepth,    dv,        -du};
        case 2: return { du,        ddepth,     dv};
        case 3: return { du,       -ddepth,    -dv};
        case 4: return {-du,        dv,     ddepth};
        case 5: return { du,        dv,    -ddepth};
        default : ENGINE_CHECK(false, "INVALID face_index %u", face_index); return {};
    }
}

// ---- mesh generation

void Mesh::set_capacity(u32 i_nvertices, u32 i_nindices){
    nvertices = i_nvertices;
    nindices = i_nindices;

    size_t bytesize = 2u * nvertices * sizeof(vec3) + nindices * sizeof(u16);
    void* ptr = malloc(bytesize);

    positions = (vec3*)ptr;
    normals = positions + nvertices;
    indices = (u16*)(normals + nvertices);
}

void Mesh::free(){
    ::free(positions);
    *this = Mesh();
}

Mesh generate_sphere_uv(vec3 position, float radius, u32 nlong, u32 nlat){
    assert(nlat > 0u && nlong > 2u);
    u32 nvertices = 2u + nlat * nlong;
    u32 nindices = 3u * (nlong + nlong + (nlat - 1u) * 2u * nlong);

    Mesh mesh;
    mesh.set_capacity(nvertices, nindices);

    // NOTE(hugo): north & south singularities
    mesh.positions[0] = position + vec3({0.f, radius, 0.f});
    mesh.normals[0] = {0.f, 1.f, 0.f};
    mesh.positions[1] = position + vec3({0.f, - radius, 0.f});
    mesh.normals[1] = {0.f, - 1.f, 0.f};

    // NOTE(hugo): other vertices
    u32 ivert = 2u;
    for(u32 ilat = 0u; ilat != nlat; ++ilat){
        float theta = (float)(ilat + 1u) / (float)(nlat + 1u) * to_radians(180.f);
        for(u32 ilong = 0u; ilong != nlong; ++ilong){
            float phi = (float)(ilong) / (float)(nlong) * to_radians(360.f);

            vec3 unitary_vector = spherical_to_cartesian(theta, phi);
            mesh.positions[ivert] = position + unitary_vector * radius;
            mesh.normals[ivert] = unitary_vector;

            ++ivert;
        }
    }

    // NOTE(hugo): north cap
    u32 iind = 0u;
    for(u32 itri = 0u; itri != nlong - 1u; ++itri){
        mesh.indices[iind++] = 0u;
        mesh.indices[iind++] = 2u + itri;
        mesh.indices[iind++] = 3u + itri;
    }
    mesh.indices[iind++] = 0u;
    mesh.indices[iind++] = 2u + (nlong - 1u);
    mesh.indices[iind++] = 2u;

    // NOTE(hugo): surface
    for(u32 ilat = 0u; ilat != nlat - 1u; ++ilat){
        u32 base_north = 2u + ilat * nlong;
        u32 base_south = base_north + nlong;

        u32 current_north = base_north;
        u32 current_south = base_south;

        for(u32 ilong = 0u; ilong != nlong - 1u; ++ilong){
            mesh.indices[iind++] = current_north;
            mesh.indices[iind++] = current_south;
            mesh.indices[iind++] = current_south + 1u;

            mesh.indices[iind++] = current_north;
            mesh.indices[iind++] = current_south + 1u;
            mesh.indices[iind++] = current_north + 1u;

            ++current_north;
            ++current_south;
        }

        mesh.indices[iind++] = current_north;
        mesh.indices[iind++] = current_south;
        mesh.indices[iind++] = base_south;

        mesh.indices[iind++] = current_north;
        mesh.indices[iind++] = base_south;
        mesh.indices[iind++] = base_north;
    }

    // NOTE(hugo): south cap
    u32 south_cap_index = 2u + (nlat - 1u) * nlong;
    for(u32 itri = 0u; itri != nlong - 1u; ++itri){
        mesh.indices[iind++] = 1u;
        mesh.indices[iind++] = south_cap_index + itri;
        mesh.indices[iind++] = south_cap_index + 1u + itri;
    }
    mesh.indices[iind++] = 1u;
    mesh.indices[iind++] = south_cap_index + (nlong - 1u);
    mesh.indices[iind++] = south_cap_index;

    return mesh;
}

namespace BEEWAX_INTERNAL{
    static vec3 cube_vertices[] = {
        {-0.5f, -0.5f,  0.5f},
        { 0.5f, -0.5f,  0.5f},
        {-0.5f,  0.5f,  0.5f},
        { 0.5f,  0.5f,  0.5f},

        {-0.5f, -0.5f, -0.5f},
        { 0.5f, -0.5f, -0.5f},
        {-0.5f,  0.5f, -0.5f},
        { 0.5f,  0.5f, -0.5f},
    };
    static u8 cube_vertices_order[] = {
        0u, 1u, 2u, 3u,
        1u, 5u, 3u, 7u,
        5u, 4u, 7u, 6u,
        4u, 0u, 6u, 2u,
        2u, 3u, 6u, 7u,
        4u, 5u, 0u, 1u,
    };
    static vec3 cube_normals[] = {
        { 0.f,  0.f,  1.f},
        { 1.f,  0.f,  0.f},
        { 0.f,  0.f, -1.f},
        {-1.f,  0.f,  0.f},
        { 0.f,  1.f,  0.f},
        { 0.f, -1.f,  0.f},
    };
    static u16 cube_indices[] = {
        0u, 1u, 2u, 2u, 1u, 3u,
        4u, 5u, 6u, 6u, 5u, 7u,
        8u, 9u, 10u, 10u, 9u, 11u,
        12u, 13u, 14u, 14u, 13u, 15u,
        16u, 17u, 18u, 18u, 17u, 19u,
        20u, 21u, 22u, 22u, 21u, 23u,
    };
}

Mesh generate_cube(vec3 position, float size, u32 ntess){
    return generate_cuboid(position, {size, size, size}, ntess);
}

Mesh generate_cuboid(vec3 position, vec3 size, u32 ntess){
    u32 nvertices = carray_size(BEEWAX_INTERNAL::cube_vertices_order);
    u32 nindices = carray_size(BEEWAX_INTERNAL::cube_indices);

    Mesh mesh;
    mesh.set_capacity(nvertices, nindices);
    for(u32 ivert = 0u; ivert != nvertices; ++ivert){
        const vec3& vert = BEEWAX_INTERNAL::cube_vertices[BEEWAX_INTERNAL::cube_vertices_order[ivert]];
        mesh.positions[ivert] = position + vec3({vert.x * size.x, vert.y * size.y, vert.z * size.z});
        mesh.normals[ivert] = BEEWAX_INTERNAL::cube_normals[ivert / 4u];
    }
    memcpy(mesh.indices, BEEWAX_INTERNAL::cube_indices, carray_size(BEEWAX_INTERNAL::cube_indices) * sizeof(BEEWAX_INTERNAL::cube_indices[0]));

    return mesh;
}

// ---- geometric predicates

float point_side_segment(const vec2& p, const vec2& vA, const vec2& vB){
    return cross(p - vA, vB - vA);
}

bool point_inside_triangle(const vec2& p, const vec2& tA, const vec2& tB, const vec2& tC){
    float sideAB = sign(point_side_segment(p, tA, tB));
    return sideAB == sign(point_side_segment(p, tB, tC)) && sideAB == sign(point_side_segment(p, tC, tA));
}

// ---- triangulation

namespace BEEWAX_INTERNAL{
    struct trig_2D_LLE{
        u32 prev;
        u32 next;
    };
}

// NOTE(hugo):
// v0: working ; but unoptimized for large, mostly convex polygons
// v1: non-working ; unoptimized for large, mostly reflex polygons

void triangulation_2D_v0(vec2* vertices, u32 nvertices, u32* nindices){
    assert(nvertices > 2u);

    BEEWAX_INTERNAL::trig_2D_LLE* connectivity_LL = (BEEWAX_INTERNAL::trig_2D_LLE*)malloc(nvertices * sizeof(BEEWAX_INTERNAL::trig_2D_LLE));
    DEFER{
        ::free((void*)connectivity_LL);
    };

    auto is_reflex = [&](u32 iprev, u32 ivert, u32 inext){
        return point_side_segment(vertices[iprev], vertices[ivert], vertices[inext]) > 0.f;
    };
    auto is_empty = [&](u32 iprev, u32 ivert, u32 inext){
        u32 itest = connectivity_LL[inext].next;
        while(itest != iprev){
            if(point_inside_triangle(vertices[itest], vertices[iprev], vertices[ivert], vertices[inext])){
                return false;
            }
            itest = connectivity_LL[itest].next;
        }
        return true;
    };

    {
        // NOTE(hugo): build the connectivity ; backwards so that the reflex_LL is in forward order
        connectivity_LL[nvertices - 1u].prev = nvertices - 2u;
        connectivity_LL[nvertices - 1u].next = 0u;

        u32 iprev = nvertices - 3u;
        u32 ivert = nvertices - 2u;
        u32 inext = nvertices - 1u;
        while(ivert != 0u){
            connectivity_LL[ivert].prev = iprev;
            connectivity_LL[ivert].next = inext;

            inext = ivert;
            ivert = iprev;
            --iprev;
        }

        connectivity_LL[0u].prev = nvertices - 1u;
        connectivity_LL[0u].next = 1u;
    }

    {
        // NOTE(hugo): find and remove ears
        u32 iprev = nvertices - 1u;
        u32 ivert = 0u;
        u32 inext = 1u;
        while(iprev != inext){
            if(!is_reflex(iprev, ivert, inext) && is_empty(iprev, ivert, inext)){
                *nindices++ = iprev;
                *nindices++ = ivert;
                *nindices++ = inext;

                connectivity_LL[iprev].next = inext;
                connectivity_LL[inext].prev = iprev;

            }else{
                iprev = ivert;

            }

            ivert = inext;
            inext = connectivity_LL[inext].next;
        }
    }

}

void triangulation_2D_v1(vec2* vertices, u32 nvertices, u32* nindices){
    assert(nvertices > 2u);

    constexpr u32 not_reflex = UINT32_MAX;
    constexpr u32 end_reflex_LL = UINT32_MAX - 1u;

    BEEWAX_INTERNAL::trig_2D_LLE* connectivity_LL = (BEEWAX_INTERNAL::trig_2D_LLE*)malloc(nvertices * sizeof(BEEWAX_INTERNAL::trig_2D_LLE));
    u32* reflex_LL = (u32*)malloc(nvertices * sizeof(u32));
    memset(reflex_LL, 0xFF, nvertices * sizeof(u32));
    DEFER{
        ::free((void*)reflex_LL);
        ::free((void*)connectivity_LL);
    };

    auto is_reflex = [&](u32 iprev, u32 ivert, u32 inext){
        return point_side_segment(vertices[iprev], vertices[ivert], vertices[inext]) > 0.f;
    };
    auto is_empty_reflex = [&](u32 iprev, u32 ivert, u32 inext, u32 ireflex){
        // NOTE(hugo): previous vertices
        {
            u32 itest = connectivity_LL[iprev].prev;
            u32 itest_next = iprev;
            while(itest_next < inext){
                if(reflex_LL[itest] != not_reflex && point_inside_triangle(vertices[itest], vertices[iprev], vertices[ivert], vertices[inext])){
                    return false;
                }
                itest_next = itest;
                itest = connectivity_LL[itest].prev;
            }
        }

        // NOTE(hugo): reflex vertices
        {
            u32 itest = ireflex;
            while(itest != end_reflex_LL){
                if(point_inside_triangle(vertices[itest], vertices[iprev], vertices[ivert], vertices[inext])){
                    return false;
                }
                itest = reflex_LL[itest];
            }
        }
        return true;
    };

    u32 reflex_head = end_reflex_LL;

    {
        // NOTE(hugo): build the connectivity ; backwards so that the reflex_LL is in forward order
        connectivity_LL[nvertices - 1u].prev = nvertices - 2u;
        connectivity_LL[nvertices - 1u].next = 0u;
        if(is_reflex(nvertices - 2u, nvertices - 1u, 0u)){
            reflex_LL[nvertices - 1u] = reflex_head;
            reflex_head = nvertices - 1u;
        }

        u32 iprev = nvertices - 3u;
        u32 ivert = nvertices - 2u;
        u32 inext = nvertices - 1u;
        while(ivert != 0u){
            connectivity_LL[ivert].prev = iprev;
            connectivity_LL[ivert].next = inext;
            if(is_reflex(iprev, ivert, inext)){
                reflex_LL[ivert] = reflex_head;
                reflex_head = ivert;
            }

            inext = ivert;
            ivert = iprev;
            --iprev;
        }

        connectivity_LL[0u].prev = nvertices - 1u;
        connectivity_LL[0u].next = 1u;
        if(is_reflex(nvertices - 1u, 0u, 1u)){
            reflex_LL[0u] = reflex_head;
            reflex_head = 0u;
        }
    }

    {
        u32 reflex_index = reflex_head;

        // NOTE(hugo): find and remove ears
        u32 iprev = nvertices - 1u;
        u32 ivert = 0u;
        u32 inext = 1u;
        while(iprev != inext){
            if(ivert == reflex_index){
                reflex_index = reflex_LL[reflex_index];

                iprev = ivert;
                ivert = inext;
                inext = connectivity_LL[inext].next;

            }else if(is_empty_reflex(iprev, ivert, inext, reflex_index)){
                *nindices++ = iprev;
                *nindices++ = ivert;
                *nindices++ = inext;

                if(reflex_index == inext && !is_reflex(iprev, inext, connectivity_LL[inext].next)) reflex_index = reflex_LL[reflex_index];

                connectivity_LL[iprev].next = inext;
                connectivity_LL[inext].prev = iprev;

                if(!is_reflex(connectivity_LL[iprev].prev, iprev, inext)){
                    ivert = iprev;
                    iprev = connectivity_LL[iprev].prev;

                }else{
                    ivert = inext;
                    inext = connectivity_LL[inext].next;
                }

            }else{
                iprev = ivert;
                ivert = inext;
                inext = connectivity_LL[inext].next;

            }
        }
    }
}

void triangulation_2D(vec2* vertices, u32 nvertices, u32* nindices){
    // NOTE(hugo): cf. benchmark ; bw::utest::t_compare_triangulation_2D()
    //triangulation_2D_v0(vertices, nvertices, nindices);
    triangulation_2D_v1(vertices, nvertices, nindices);

    // TODO(hugo): make something to avoid testing non reflex vertices when testing previous vertices in is_empty_reflex ;
    // it would most likely require a double LL before it would require removal (forward iteration from base_reflex should be fine)
}

// ---- geometric error

float circular_cap_chord_to_arc_error(u32 ncap_vertices, float radius){
    assert(ncap_vertices > 0u);
    float rad = PI / (float)(ncap_vertices + 1u);
    return radius * (1.f - cos(rad * 0.5f));
}

u32 circular_cap_vertices(float radius, float max_error){
    // NOTE(hugo): PI / (2 * acos(1 - max_error / radius))
    u32 nvertices = bw::ceil(0.5f * PI / acos(1.f - max_error / radius));
    return max(nvertices, 1u);
}

float circle_chord_to_arc_error(u32 nvertices, float radius){
    assert(nvertices > 2);
    float rad = 2.f * PI / (float)(nvertices);
    return radius * (1.f - cos(rad * 0.5f));
}

u32 circle_vertices(float radius, float max_error){
    // NOTE(hugo): PI / acos(1 - max_error / radius)
    u32 nvertices = bw::ceil(PI / acos(1.f - max_error / radius));
    return max(nvertices, 3u);
}
