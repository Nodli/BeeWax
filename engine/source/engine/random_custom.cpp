// NOTE(hugo): http://www.iquilezles.org/www/articles/sfrand/sfrand.htm
constexpr u32 multiplierLLC32 = 16807u;
// NOTE(hugo):https://fr.wikipedia.org/wiki/G%C3%A9n%C3%A9rateur_congruentiel_lin%C3%A9aire#Du_bon_choix_des_param%C3%A8tres_a,_c_et_m
constexpr u64 multiplierLLC64 = 6364136223846793005u;

void seed_random_with_time(u64& seed){
    u64 stime = (u64)time(NULL);
    seed = stime << 32 | (stime ^ multiplierLLC64);
}

DISABLE_WARNING_PUSH
DISABLE_WARNING_TYPE_PUNNING
DISABLE_WARNING_UNINITIALIZED

s32 random_s32(u32& seed){
    seed *= multiplierLLC32;
    return *(s32*)(&seed);
}

u32 random_u32(u32& seed){
    seed *= multiplierLLC32;
    return seed;
}

s64 random_s64(u64& seed){
    seed *= multiplierLLC64;
    return *(s64*)(&seed);
}

u64 random_u64(u64& seed){
    seed *= multiplierLLC64;
    return seed;
}

float random_float_normalized(u32& seed){
    seed *= multiplierLLC32;

    float output;
    *(u32*)(&output) = (seed >> 9 | 0x40000000u);

    return output - 3.0f;
}

float random_float_normalized_positive(u32& seed){
    seed *= multiplierLLC32;

    float output;
    *(u32*)(&output) = (seed >> 9 | 0x3f800000);

    return output - 1.0f;
}

float random_float_range(float min, float max, u32& seed){
    return min + random_float_normalized_positive(seed) * (max - min);
}

u32 random_u32_range_uniform(u32 max, u32& seed){
    u32 output;
    u32 random_max = UINT_MAX - UINT_MAX % max;

    // NOTE(hugo): excluding numbers outside of the expected range for uniform probabilities
    do{
        seed *= multiplierLLC32;
        output = seed;
    }while(output >= random_max);

    return output % max;
}

float random_float_uniform(u32& seed){
    seed *= multiplierLLC32;

    // NOTE(hugo): excluding non-numbers ie doubled probability for a mantissa of 254
    float output;
    do{
        *(u32*)(&output) = (seed >> 9 | 0x40000000u);
    }while(float_mantissa(output) == 255);

    return output;
}

DISABLE_WARNING_POP

u32* random_permutation(u32 permutation_size, u32& seed){
    assert(permutation_size > 0);

    u32* output = (u32*)malloc((size_t)permutation_size * sizeof(u32));

    for(u32 index = 0u; index != permutation_size; ++index){
        output[index] = index;
    }

    random_permutation_in_place(output, permutation_size, seed);

    return output;
}

void random_permutation_in_place(u32* data, u32 permutation_size, u32& seed){
    assert(permutation_size > 0);

    for(u32 istep = 0u; istep != permutation_size; ++istep){
        u32 random_index = random_u32_range_uniform(permutation_size - istep, seed);
        u32 temp = data[istep];
        data[istep] = data[istep + random_index];
        data[istep + random_index] = temp;
    }
}

void random_partial_permutation_in_place(u32* data, u32 data_size, u32 permutation_size, u32& seed){
    assert(data_size > 0);
    assert(permutation_size > 0);
    assert(!(permutation_size > data_size));

    for(u32 istep = 0; istep != permutation_size; ++istep){
        u32 random_index = random_u32_range_uniform(data_size - istep, seed);
        swap(data[istep], data[istep + random_index]);
    }
}
