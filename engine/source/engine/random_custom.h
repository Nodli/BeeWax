#ifndef H_RANDOM_CUSTOM
#define H_RANDOM_CUSTOM

// REF(hugo):
// https://ourmachinery.com/apidoc/foundation/random.h.html
// http://prng.di.unimi.it/xoroshiro128plus.c
// https://stackoverflow.com/questions/62020542/generate-random-double-number-in-range-0-1-in-c
// http://prng.di.unimi.it/

// ---- seed

namespace BEEWAX_INTERNAL{
    union xoroshiro128plus_state{
        struct{
            u64 s0;
            u64 s1;
        };
        u64 s[2u];
    } xoroshiro128plus_seed = { ((u64)rand()) << 32u || ((u64)rand()), ((u64)rand()) << 32u || ((u64)rand())};
}

typedef BEEWAX_INTERNAL::xoroshiro128plus_state random_seed_type;

void random_seed(u64* state, BEEWAX_INTERNAL::xoroshiro128plus_state& seed = BEEWAX_INTERNAL::xoroshiro128plus_seed);
void random_seed_with_time(BEEWAX_INTERNAL::xoroshiro128plus_state& seed = BEEWAX_INTERNAL::xoroshiro128plus_seed);
random_seed_type random_seed_copy();

// ----

bool random_bool(BEEWAX_INTERNAL::xoroshiro128plus_state& seed = BEEWAX_INTERNAL::xoroshiro128plus_seed);

s32 random_s32(BEEWAX_INTERNAL::xoroshiro128plus_state& seed = BEEWAX_INTERNAL::xoroshiro128plus_seed);
u32 random_u32(BEEWAX_INTERNAL::xoroshiro128plus_state& seed = BEEWAX_INTERNAL::xoroshiro128plus_seed);

s64 random_s64(BEEWAX_INTERNAL::xoroshiro128plus_state& seed = BEEWAX_INTERNAL::xoroshiro128plus_seed);
u64 random_u64(BEEWAX_INTERNAL::xoroshiro128plus_state& seed = BEEWAX_INTERNAL::xoroshiro128plus_seed);

// NOTE(hugo): [0, 1)
float random_float(BEEWAX_INTERNAL::xoroshiro128plus_state& seed = BEEWAX_INTERNAL::xoroshiro128plus_seed);
double random_double(BEEWAX_INTERNAL::xoroshiro128plus_state& seed = BEEWAX_INTERNAL::xoroshiro128plus_seed);

// NOTE(hugo): [0, max[ but uses rejection sampling
u32 random_u32_range_uniform(u32 max, BEEWAX_INTERNAL::xoroshiro128plus_state& seed = BEEWAX_INTERNAL::xoroshiro128plus_seed);

// ---- permutation

u32* random_permutation(u32 permutation_size, BEEWAX_INTERNAL::xoroshiro128plus_state& seed = BEEWAX_INTERNAL::xoroshiro128plus_seed);
void random_permutation_in_place(u32* data, u32 permutation_size, BEEWAX_INTERNAL::xoroshiro128plus_state& seed = BEEWAX_INTERNAL::xoroshiro128plus_seed);
void random_partial_permutation_in_place(u32* data, u32 data_size, u32 permutation_size, BEEWAX_INTERNAL::xoroshiro128plus_state& seed = BEEWAX_INTERNAL::xoroshiro128plus_seed);

// ---- n-sphere sampling
// REF(hugo): http://extremelearning.com.au/how-to-generate-uniformly-random-points-on-n-spheres-and-n-balls/

vec2 random_on_unit_circle(BEEWAX_INTERNAL::xoroshiro128plus_state& seed = BEEWAX_INTERNAL::xoroshiro128plus_seed);
vec2 random_on_unit_disc(BEEWAX_INTERNAL::xoroshiro128plus_state& seed = BEEWAX_INTERNAL::xoroshiro128plus_seed);

#endif
