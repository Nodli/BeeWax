namespace BEEWAX_INTERNAL{

    static const u64 xoroshiro128_JUMP[] = {0xdf900294d8f554a5, 0x170865df4b3201fc};
    static const u64 xoroshiro128_LONG_JUMP[] = { 0xd2a98b26625eee7b, 0xdddf9b1090aa7ac1 };

    u64 xoroshiro128plus_step(xoroshiro128plus_state& seed = BEEWAX_INTERNAL::xoroshiro128plus_seed){
        const u64 s0 = seed.s0;
        u64 s1 = seed.s1;
        const u64 output = s0 + s1;

        auto rotL = [](const u64 data, u32 bits){
            return (data << bits) | (data >> (64 - bits));
        };

        s1 ^= s0;
        seed.s[0] = rotL(s0, 24u) ^ s1 ^ (s1 << 16u);
        seed.s[1] = rotL(s1, 37u);

        return output;
    }

    // NOTE(hugo): equivalent to 2^64 steps ie 2^64 non-overlapping sequences
    xoroshiro128plus_state xoroshiro128plus_offset_small(xoroshiro128plus_state& seed = BEEWAX_INTERNAL::xoroshiro128plus_seed){
        xoroshiro128plus_state output = {0u, 0u};

        for(u32 i = 0u; i != (sizeof(xoroshiro128_JUMP) / sizeof(*xoroshiro128_JUMP)); ++i){
            for(u32 b = 0u; b != 64u; ++b){
                if(xoroshiro128_JUMP[i] & ((u64)1u) << b){
                    output.s0 ^= seed.s0;
                    output.s1 ^= seed.s1;
                }
                xoroshiro128plus_step(seed);
            }
        }

        return output;
    }

    // NOTE(hugo): equivalent to 2^96 steps ie 2^32 non-overlapping sequences
    xoroshiro128plus_state xoroshiro128plus_offset_large(xoroshiro128plus_state& seed = BEEWAX_INTERNAL::xoroshiro128plus_seed){
        xoroshiro128plus_state output = {0u, 0u};

        for(u32 i = 0u; i != (sizeof(xoroshiro128_LONG_JUMP) / sizeof(*xoroshiro128_LONG_JUMP)); ++i){
            for(u32 b = 0u; b != 64u; ++b){
                if(xoroshiro128_LONG_JUMP[i] & ((u64)1u) << b){
                    output.s0 ^= seed.s0;
                    output.s1 ^= seed.s1;
                }
                xoroshiro128plus_step(seed);
            }
        }

        return output;
    }
}

void random_seed(u64* state, BEEWAX_INTERNAL::xoroshiro128plus_state& seed){
    seed.s0 = state[0];
    seed.s1 = state[1];
}

void random_seed_with_time(BEEWAX_INTERNAL::xoroshiro128plus_state& seed){
    u64 temp = ((u64)time(NULL));
    u64 state[2] = {temp << 32u | temp, temp << 32u | temp};
    random_seed(state, seed);
}

BEEWAX_INTERNAL::xoroshiro128plus_state random_seed_copy(){
    BEEWAX_INTERNAL::xoroshiro128plus_state output = BEEWAX_INTERNAL::xoroshiro128plus_seed;
    return output;
}

DISABLE_WARNING_PUSH
DISABLE_WARNING_TYPE_PUNNING
DISABLE_WARNING_UNINITIALIZED

bool random_bool(BEEWAX_INTERNAL::xoroshiro128plus_state& seed){
    u64 temp = xoroshiro128plus_step(seed);
    return (bool)(temp >> 63u);
}

s32 random_s32(BEEWAX_INTERNAL::xoroshiro128plus_state& seed){
    u32 temp = random_u32(seed);
    return *(s32*)(&temp);
}
u32 random_u32(BEEWAX_INTERNAL::xoroshiro128plus_state& seed){
    u64 temp = xoroshiro128plus_step(seed);
    return (u32)(temp >> 32u);
}

s64 random_s64(BEEWAX_INTERNAL::xoroshiro128plus_state& seed){
    u64 temp = xoroshiro128plus_step(seed);
    return *(s64*)&temp;
}
u64 random_u64(BEEWAX_INTERNAL::xoroshiro128plus_state& seed){
    return xoroshiro128plus_step(seed);
}

float random_float(BEEWAX_INTERNAL::xoroshiro128plus_state& seed){
    u64 temp = xoroshiro128plus_step(seed);
    return (temp >> (41u)) * 0x1.0p-23f;
}
double random_double(BEEWAX_INTERNAL::xoroshiro128plus_state& seed){
    u64 temp = xoroshiro128plus_step(seed);
    return (temp >> 11u) * 0x1.0p-53;
}

u32 random_u32_range_uniform(u32 max, BEEWAX_INTERNAL::xoroshiro128plus_state& seed){
    assert(max > 0u);
    u32 bucket_size = UINT32_MAX / max + (UINT32_MAX % max == max - 1u);
    assert(bucket_size > 0u);

    u32 output;

    do{
        output = random_u32(seed) / bucket_size;
    }while(output > (max - 1u));

    return output;
}

DISABLE_WARNING_POP

u32* random_permutation(u32 permutation_size, BEEWAX_INTERNAL::xoroshiro128plus_state& seed){
    assert(permutation_size > 0);

    u32* output = (u32*)malloc((size_t)permutation_size * sizeof(u32));

    for(u32 index = 0u; index != permutation_size; ++index){
        output[index] = index;
    }

    random_permutation_in_place(output, permutation_size, seed);

    return output;
}

void random_permutation_in_place(u32* data, u32 permutation_size, BEEWAX_INTERNAL::xoroshiro128plus_state& seed){
    assert(permutation_size > 0);

    for(u32 istep = 0u; istep != permutation_size - 1u; ++istep){
        u32 random_index = random_u32_range_uniform(permutation_size - istep, seed);
        u32 temp = data[istep];
        data[istep] = data[istep + random_index];
        data[istep + random_index] = temp;
    }
}

void random_partial_permutation_in_place(u32* data, u32 data_size, u32 permutation_size, BEEWAX_INTERNAL::xoroshiro128plus_state& seed){
    assert(data_size > 0 && permutation_size > 0 && permutation_size <= data_size);

    for(u32 istep = 0; istep != permutation_size - 1u; ++istep){
        u32 random_index = random_u32_range_uniform(data_size - istep, seed);
        swap(data[istep], data[istep + random_index]);
    }
}
// NOTE(hugo): Muller , Marsaglia (‘Normalised Gaussians’)
vec2 random_on_unit_circle(BEEWAX_INTERNAL::xoroshiro128plus_state& seed){
    float angle = PI * 2.f * random_float(seed);
    return {cos(angle), sin(angle)};
}

vec2 random_on_unit_disc(BEEWAX_INTERNAL::xoroshiro128plus_state& seed){
    float a = random_float(seed);
    float b = random_float(seed);

    float r, theta;
    if(a * a > b * b){
        r = a;
        theta = PI * 0.25f * b / a;
    }else{
        r = b;
        theta = PI * 0.5f - PI * 0.25f * a / b;
    }

    return {r * cos(theta), r * sin(theta)};
}
