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

size_t round_up_multiple(size_t number, size_t multipler){
    assert(multipler != 0u);
    size_t remainder = number % multipler;
    return number + (multipler - remainder) * (remainder != 0u);
}

u32 get_rightmost_set_bit(u32 number){
    return number & (~number + 1u);
}

// ---- normalized integer

u32 float_to_unorm32(float f){
    assert(f >= 0.f && f <= 1.f);
    return (u32)((double)f * UINT32_MAX);
}

float unorm32_to_float(u32 u){
    return (float)((double)u / (double)UINT32_MAX);
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

float floor(const float x){
    return floorf(x);
}

s32 floor_s32(const float x){
    s32 xi = (s32)x;
    return (x < 0.f) ? (xi - 1) : xi;
}

float ceil(const float x){
    return ceilf(x);
}

s32 ceil_s32(const float x){
    s32 xi = (s32)x;
    return (x > 0.f) ? (xi + 1) : xi;
}

float pow(const float x, const float pow){
    return powf(x, pow);
}

float mix(const float min, const float max, const float t){
    return min + t * (max - min);
}

float normalize(const float x, const float range_min, const float range_max){
    return min_max((x - range_min) / (range_max - range_min), 0.f, 1.f);
}

float to_radians(const float degree){
    return degree / 180.f * PI;
}

float to_degree(const float radian){
    return radian / PI * 180.f;
}

float cos(float t){
    return cosf(t);
}

float sin(float t){
    return sinf(t);
}

float tan(float t){
    return tanf(t);
}

float sqrt(float t){
    return sqrtf(t);
}

float acos(float t){
    return acosf(t);
}

float asin(float t){
    return asinf(t);
}

float atan(float t){
    return atanf(t);
}

float atan2(float y, float x){
    return atan2f(y, x);
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
