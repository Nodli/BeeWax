#ifndef H_RANDOM_CUSTOM
#define H_RANDOM_CUSTOM

// ---- seed

namespace BEEWAX_INTERNAL{
    static union {
        u32 seed32;
        u64 seed64;
    } seed = {((u64)rand()) << 32 || (u64)rand()};
}

void seed_random_with(u64 to_use, u64& seed = BEEWAX_INTERNAL::seed.seed64);
void seed_random_with_time(u64& seed = BEEWAX_INTERNAL::seed.seed64);

// ----

s32 random_s32(u32& seed = BEEWAX_INTERNAL::seed.seed32);
u32 random_u32(u32& seed = BEEWAX_INTERNAL::seed.seed32);

s64 random_s64(u64& seed = BEEWAX_INTERNAL::seed.seed64);
u64 random_u64(u64& seed = BEEWAX_INTERNAL::seed.seed64);

float random_float_normalized(u32& seed = BEEWAX_INTERNAL::seed.seed32); // NOTE(hugo): [-1, 1)
float random_float_normalized_positive(u32& seed = BEEWAX_INTERNAL::seed.seed32); // NOTE(hugo): [0, 1)
float random_float_range(float min, float max, u32& seed = BEEWAX_INTERNAL::seed.seed32); // NOTE(hugo): [min, max)

// ---- rejection sampling

// NOTE(hugo): [0, max[
u32 random_u32_range_uniform(u32 max, u32& seed = BEEWAX_INTERNAL::seed.seed32);
float random_float_uniform(u32& seed = BEEWAX_INTERNAL::seed.seed32);

// ---- permutation

u32* random_permutation(u32 permutation_size, u32& seed = BEEWAX_INTERNAL::seed.seed32);
void random_permutation_in_place(u32* data, u32 permutation_size, u32& seed = BEEWAX_INTERNAL::seed.seed32);
void random_partial_permutation_in_place(u32* data, u32 data_size, u32 permutation_size, u32& seed = BEEWAX_INTERNAL::seed.seed32);

// ---- n-sphere sampling
// REF(hugo): http://extremelearning.com.au/how-to-generate-uniformly-random-points-on-n-spheres-and-n-balls/

vec2 random_on_unit_circle();
vec2 random_on_unit_disc();

#endif
