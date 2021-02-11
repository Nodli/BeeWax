// ---- bithacks

bool is_pow2(u32 number){
    return (number != 0u) && ((number & (number - 1u)) == 0u);
}

bool is_pow2(u64 number){
    return (number != 0u) && ((number & (number - 1u)) == 0u);
}

u32 round_up_pow2(u32 number){
    --number;
    number |= number >> 1u;
    number |= number >> 2u;
    number |= number >> 4u;
    number |= number >> 8u;
    number |= number >> 16u;
    ++number;

    return number;
}

u32 round_up_multiple(u32 number, u32 multipler){
    assert(multipler != 0u);
    u32 remainder = number % multipler;
    return number + (multipler - remainder) * (remainder != 0u);
}

u32 get_rightmost_set_bit(u32 number){
    return number & (~number + 1u);
}

// ---- alignment

size_t align_offset_next(uintptr_t uptr, size_t align){
    assert(is_pow2(align));
    return (size_t)((- uptr) & (align - 1u));
}

void* align_next(void* ptr, size_t align){
    assert(is_pow2(align));
    uintptr_t uint_ptr = (uintptr_t)ptr;
    return (void*)(uint_ptr + align_offset_next(uint_ptr, align));
}

void* align_prev(void* ptr, size_t align){
    assert(is_pow2(align));
    uintptr_t uint_ptr = (uintptr_t)ptr;
    return (void*)(uint_ptr & ~(align - 1u));
}

size_t align_offset_prev(uintptr_t uptr, size_t align){
    assert(is_pow2(align));
    return (size_t)(uptr - (uintptr_t)align_prev((void*)uptr, align));
}

bool is_aligned(void* ptr, size_t align){
    return ptr == align_prev(ptr, align);
}

// ---- math

float fast_floor(const float x){
    return (x < 0.f) ? x - 1.f : x;
}

float mix(const float min, const float max, const float t){
    return min + t * (max - min);
}

float to_radians(const float degree){
    return degree / 180.f * PI;
}

float to_degree(const float radian){
    return radian / PI * 180.f;
}

float normalize(const float x, const float range_min, const float range_max){
    return min_max((x - range_min) / (range_max - range_min), 0.f, 1.f);
}

float cos(float t){
    return std::cos(t);
}

float sin(float t){
    return std::sin(t);
}

float tan(float t){
    return std::tan(t);
}

float sqrt(float t){
    return std::sqrt(t);
}

float acos(float t){
    return std::acos(t);
}

float asin(float t){
    return std::asin(t);
}

float atan(float t){
    return std::atan(t);
}

float atan2(float y, float x){
    return std::atan2(y, x);
}

// ---- grid indexing

uint index2D(uint major_axis_coord, uint minor_axis_coord, uint major_axis_size){
    return minor_axis_coord * major_axis_size + major_axis_coord;
}

void coord2D(uint index, uint major_axis_size, uint& major_axis_coord, uint& minor_axis_coord){
    major_axis_coord = index % major_axis_size;
    minor_axis_coord = index / major_axis_size;
}

uint neigh4_index2D(uint index, uint major_axis_size, uint max_index, uint* out_neighbor){
    assert(major_axis_size > 0);

    uint ncount = 0;
    if(index > major_axis_size - 1){
        *(out_neighbor++) = index - major_axis_size;
        ++ncount;
    }
    if(index + major_axis_size < max_index){
        *(out_neighbor++) = index + major_axis_size;
        ++ncount;
    }
    uint major_coord_temp = index % major_axis_size;
    if(major_coord_temp < major_axis_size - 1){
        *(out_neighbor++) = index + 1;
        ++ncount;
    }
    if(major_coord_temp > 0){ // NOTE(hugo): implies index > 0
        *(out_neighbor++) = index - 1;
        ++ncount;
    }
    return ncount;
}

uint neigh4_coord2D(uint major_axis_coord, uint minor_axis_coord, uint major_axis_size, uint minor_axis_size, uint* out_neighbor){
    uint ncount = 0;
    if(major_axis_coord + 1 < major_axis_size){
        *(out_neighbor++) = major_axis_coord + 1;
        *(out_neighbor++) = minor_axis_coord;
        ++ncount;
    }
    if(major_axis_coord > 0){
        *(out_neighbor++) = major_axis_coord - 1;
        *(out_neighbor++) = minor_axis_coord;
        ++ncount;
    }
    if(minor_axis_coord + 1 < minor_axis_size){
        *(out_neighbor++) = major_axis_coord;
        *(out_neighbor++) = minor_axis_coord + 1;
        ++ncount;
    }
    if(minor_axis_coord > 0){
        *(out_neighbor++) = major_axis_coord;
        *(out_neighbor++) = minor_axis_coord - 1;
        ++ncount;
    }
    return ncount;
}

// ---- colormaps

void viridis(const float value, float& r, float& g, float& b){
    assert(value >= 0.f && value <= 1.f);

    constexpr float c0[3] = {0.2777273272234177, 0.005407344544966578, 0.3340998053353061};
    constexpr float c1[3] = {0.1050930431085774, 1.404613529898575, 1.384590162594685};
    constexpr float c2[3] = {-0.3308618287255563, 0.214847559468213, 0.09509516302823659};
    constexpr float c3[3] = {-4.634230498983486, -5.799100973351585, -19.33244095627987};
    constexpr float c4[3] = {6.228269936347081, 14.17993336680509, 56.69055260068105};
    constexpr float c5[3] = {4.776384997670288, -13.74514537774601, -65.35303263337234};
    constexpr float c6[3] = {-5.435455855934631, 4.645852612178535, 26.3124352495832};

    r = c0[0] + value * (c1[0] + value * ( c2[0] + value * ( c3[0] + value * ( c4[0] + value * (c5[0] + value * c6[0])))));
    g = c0[1] + value * (c1[1] + value * ( c2[1] + value * ( c3[1] + value * ( c4[1] + value * (c5[1] + value * c6[1])))));
    b = c0[2] + value * (c1[2] + value * ( c2[2] + value * ( c3[2] + value * ( c4[2] + value * (c5[2] + value * c6[2])))));
}

void plasma(const float value, float& r, float& g, float& b){
    assert(value >= 0.f && value <= 1.f);

    constexpr float c0[3] = {0.05873234392399702, 0.02333670892565664, 0.5433401826748754};
    constexpr float c1[3] = {2.176514634195958, 0.2383834171260182, 0.7539604599784036};
    constexpr float c2[3] = {-2.689460476458034, -7.455851135738909, 3.110799939717086};
    constexpr float c3[3] = {6.130348345893603, 42.3461881477227, -28.51885465332158};
    constexpr float c4[3] = {-11.10743619062271, -82.66631109428045, 60.13984767418263};
    constexpr float c5[3] = {10.02306557647065, 71.41361770095349, -54.07218655560067};
    constexpr float c6[3] = {-3.658713842777788, -22.93153465461149, 18.19190778539828};

    r = c0[0] + value * (c1[0] + value * ( c2[0] + value * ( c3[0] + value * ( c4[0] + value * (c5[0] + value * c6[0])))));
    g = c0[1] + value * (c1[1] + value * ( c2[1] + value * ( c3[1] + value * ( c4[1] + value * (c5[1] + value * c6[1])))));
    b = c0[2] + value * (c1[2] + value * ( c2[2] + value * ( c3[2] + value * ( c4[2] + value * (c5[2] + value * c6[2])))));
}

void magma(const float value, float& r, float& g, float& b){
    assert(value >= 0.f && value <= 1.f);

    constexpr float c0[3] = {-0.002136485053939582, -0.000749655052795221, -0.005386127855323933};
    constexpr float c1[3] = {0.2516605407371642, 0.6775232436837668, 2.494026599312351};
    constexpr float c2[3] = {8.353717279216625, -3.577719514958484, 0.3144679030132573};
    constexpr float c3[3] = {-27.66873308576866, 14.26473078096533, -13.64921318813922};
    constexpr float c4[3] = {52.17613981234068, -27.94360607168351, 12.94416944238394};
    constexpr float c5[3] = {-50.76852536473588, 29.04658282127291, 4.23415299384598};
    constexpr float c6[3] = {18.65570506591883, -11.48977351997711, -5.601961508734096};

    r = c0[0] + value * (c1[0] + value * ( c2[0] + value * ( c3[0] + value * ( c4[0] + value * (c5[0] + value * c6[0])))));
    g = c0[1] + value * (c1[1] + value * ( c2[1] + value * ( c3[1] + value * ( c4[1] + value * (c5[1] + value * c6[1])))));
    b = c0[2] + value * (c1[2] + value * ( c2[2] + value * ( c3[2] + value * ( c4[2] + value * (c5[2] + value * c6[2])))));
}

void inferno(const float value, float& r, float& g, float& b){
    assert(value >= 0.f && value <= 1.f);

    constexpr float c0[3] = {0.0002189403691192265, 0.001651004631001012, -0.01948089843709184};
    constexpr float c1[3] = {0.1065134194856116, 0.5639564367884091, 3.932712388889277};
    constexpr float c2[3] = {11.60249308247187, -3.972853965665698, -15.9423941062914};
    constexpr float c3[3] = {-41.70399613139459, 17.43639888205313, 44.35414519872813};
    constexpr float c4[3] = {77.162935699427, -33.40235894210092, -81.80730925738993};
    constexpr float c5[3] = {-71.31942824499214, 32.62606426397723, 73.20951985803202};
    constexpr float c6[3] = {25.13112622477341, -12.24266895238567, -23.07032500287172};

    r = c0[0] + value * (c1[0] + value * ( c2[0] + value * ( c3[0] + value * ( c4[0] + value * (c5[0] + value * c6[0])))));
    g = c0[1] + value * (c1[1] + value * ( c2[1] + value * ( c3[1] + value * ( c4[1] + value * (c5[1] + value * c6[1])))));
    b = c0[2] + value * (c1[2] + value * ( c2[2] + value * ( c3[2] + value * ( c4[2] + value * (c5[2] + value * c6[2])))));
}
