static inline float perlin_quintic_interpolator(const float t){
    return t * t * t * (10.f + t * (- 15.f + t * 6.f));
}

static inline float perlin_quintic_interpolator_derivative(const float t){
    return 30.f * t * t * (1.f + t * (- 2.f + t));
}

float perlin_noise(const float x){
    float x_integral = floorf(x);
    float x_decimal = x - x_integral;

    s32 origin = (s32)x_integral;
    float dL = x_decimal;
    float dR = - 1.f + x_decimal;

    // NOTE(hugo): determine gradients
    u32 seedL = fibonacci_hash((u32)origin);
    u32 seedR = fibonacci_hash((u32)origin + 1u);
    float gradL = random_float_normalized(seedL);
    float gradR = random_float_normalized(seedR);

    // NOTE(hugo): extrapolate the slopes to x in the unit
    // this corresponds to dot(gradient, direction to x) but in 1D
    float extrapL = dL * gradL;
    float extrapR = dR * gradR;

    // NOTE(hugo): interpolation and scaling to [-1, 1]
    //constexpr float MAGIC_NORMALIZER = 1.f / sqrt(1.f / 4.f);
    constexpr float MAGIC_NORMALIZER = 2.f;
    return MAGIC_NORMALIZER * mix(extrapL, extrapR, perlin_quintic_interpolator(x_decimal));
}

// NOTE(hugo):
// using a permutation to have more consistent results
// 8 directions can be generated (with the diagonals normalized)
// sequence (x) :  0  sqrt(2)/2  1  sqrt(2)/2  0 -sqrt(2)/2 -1 -sqrt(2)/2
// phased   (y) :  1  sqrt(2)/2  0 -sqrt(2)/2 -1 -sqrt(2)/2  0  sqrt(2)/2
// the permutation can contain 8 elements without skewing probabilities because UINT_MAX is a multiple of 8
// compute :    no static cache         more instructions but no cache misses
// cache :      32 byte static cache    less instructions but potential cache misses
// the cache version should be better in the most common use case because the noise function is expected to be called multiple times
static inline vec2 perlin_gradient_2D_compute(u32 hash){
    s32 x_value = (hash & 0x3) >> (hash & 0x2);
    s32 x_sign = (hash & 0x4);
    s32 x_sqrt2 = (hash & 0x1);

    u32 y_hash = hash + 2u;
    s32 y_value = (y_hash & 0x3) >> (y_hash & 0x2);
    s32 y_sign = (y_hash & 0x4);
    s32 y_sqrt2 = (y_hash & 0x1);

    constexpr float sqrt_contribution = HALF_SQRT2<float> - 1.f;

    vec2 output;
    output.x = x_sign * (x_value + x_sqrt2 * sqrt_contribution);
    output.y = y_sign * (y_value + y_sqrt2 * sqrt_contribution);

    return output;
}

static inline vec2 perlin_gradient_2D_cache(u32 hash){
    float cache[8u] = {
        0.f,
        HALF_SQRT2<float>,
        1.f,
        HALF_SQRT2<float>,
        0.f,
       -HALF_SQRT2<float>,
       -1.f,
       -HALF_SQRT2<float>
    };

    u32 x_index = hash & 0x7;
    u32 y_index = (hash + 2u) & 0x7;

    vec2 output;
    output.x = cache[x_index];
    output.y = cache[y_index];

    return output;
}

constexpr float perlin_noise_2D_MAGIC_NORMALIZER = 1.414213562373095; // NOTE(hugo): 1.f / sqrt(2.f / 4.f)

// NOTE(hugo): gradient must be an array of size 4
//             extrap must be an array of size 4
static inline void setup_perlin_noise_2D(const float x, const float y,
        float& x_decimal, float& y_decimal,
        vec2* gradient, float* extrap,
        float& x_interpolator, float& y_interpolator){

    float x_integral = floorf(x);
    x_decimal = x - x_integral;
    float y_integral = floorf(y);
    y_decimal = y - y_integral;

    s32 x_origin = (s32)x_integral;
    s32 y_origin = (s32)y_integral;

    // NOTE(hugo): L for low and H for high (H = L + 1)
    float x_dL = x_decimal;
    float x_dH = - 1.f + x_decimal;
    float y_dL = y_decimal;
    float y_dH = - 1.f + y_decimal;

    // NOTE(hugo): determine gradients LL HL HH LH
    s32 hash_data[2];
    u32 hash;

    hash_data[0] = x_origin;
    hash_data[1] = y_origin;
    hash = combined_hash_32<fibonacci_hash>((u32*)hash_data, 2u);
    gradient[0] = perlin_gradient_2D_cache(hash);

    ++hash_data[0];
    hash = combined_hash_32<fibonacci_hash>((u32*)hash_data, 2u);
    gradient[1] = perlin_gradient_2D_cache(hash);

    ++hash_data[1];
    hash = combined_hash_32<fibonacci_hash>((u32*)hash_data, 2u);
    gradient[2] = perlin_gradient_2D_cache(hash);

    --hash_data[0];
    hash = combined_hash_32<fibonacci_hash>((u32*)hash_data, 2u);
    gradient[3] = perlin_gradient_2D_cache(hash);

    // NOTE(hugo): extrapolate the gradients
    // ie we are deriving the slope in the direction of the point based on the gradient
    extrap[0] = x_dL * gradient[0].x + y_dL * gradient[0].y;
    extrap[1] = x_dH * gradient[1].x + y_dL * gradient[1].y;
    extrap[2] = x_dH * gradient[2].x + y_dH * gradient[2].y;
    extrap[3] = x_dL * gradient[3].x + y_dH * gradient[3].y;

    // NOTE(hugo): interpolators
    x_interpolator = perlin_quintic_interpolator(x_decimal);
    y_interpolator = perlin_quintic_interpolator(y_decimal);
}

static inline void evaluate_perlin_noise_2D(const float* const extrap, const float x_interpolator, const float y_interpolator,
        float& output){

    // NOTE(hugo): interpolation and scaling to [-1, 1]
    // ie we are weighting the extrapolated gradients at the point
    float mix_LL_HL = mix(extrap[0], extrap[1], x_interpolator);
    float mix_LH_HH = mix(extrap[3], extrap[2], x_interpolator);
    output = perlin_noise_2D_MAGIC_NORMALIZER * mix(mix_LL_HL, mix_LH_HH, y_interpolator);
}

static inline void derivate_perlin_noise_2D(const float x_decimal, const float y_decimal, const vec2* const gradient, const float* const extrap, const float x_interpolator, const float y_interpolator,
        vec2& output){

    float x_interpolator_derivative = perlin_quintic_interpolator_derivative(x_decimal);
    float y_interpolator_derivative = perlin_quintic_interpolator_derivative(y_decimal);

    // NOTE(hugo): derivatives
    // X = quintic interpolator on x and it's derivative X'
    // Y = quintic interpolator on y and it's derivative Y'
    // g0, g1, g2, g3 the gradient vectors
    // e0, e1, e2, e3 the extrap values
    // dNdx = X' * ((1 - Y) * (e1 - e0) + Y * (e2 - e3)) + X * ((1 - Y) * (g1.x - g0.x) + Y * (g2.x - g3.x)) + Y * (g3.x - g0.x) + g0.x
    // dNdy = Y' * ((1 - X) * (e3 - e0) + X * (e2 - e1)) + Y * ((1 - X) * (g3.y - g0.y) + X * (g2.y - g1.y)) + X * (g1.y - g0.y) + g0.y
    float intermediate_x = 1.f - y_interpolator;
    output.x = x_interpolator_derivative * (intermediate_x * (extrap[1] - extrap[0]) + y_interpolator * (extrap[2] - extrap[3]))
        + x_interpolator * (intermediate_x * (gradient[1].x - gradient[0].x) + y_interpolator * (gradient[2].x - gradient[3].x))
        + y_interpolator * (gradient[3].x - gradient[0].x)
        + gradient[0].x;

    float intermediate_y = 1.f - x_interpolator;
    output.y = y_interpolator_derivative * (intermediate_y * (extrap[3] - extrap[0]) + x_interpolator * (extrap[2] - extrap[1]))
        + y_interpolator * (intermediate_y * (gradient[3].y - gradient[0].y) + x_interpolator * (gradient[2].y - gradient[1].y))
        + x_interpolator * (gradient[1].y - gradient[0].y)
        + gradient[0].y;

    output *= perlin_noise_2D_MAGIC_NORMALIZER;
}

float perlin_noise(const float x, const float y){
    float x_decimal, y_decimal;
    vec2 gradient[4];
    float extrap[4];
    float x_interpolator, y_interpolator;
    setup_perlin_noise_2D(x, y, x_decimal, y_decimal, gradient, extrap, x_interpolator, y_interpolator);

    float output;
    evaluate_perlin_noise_2D(extrap, x_interpolator, y_interpolator, output);
    return output;
}

vec2 perlin_derivatives(const float x, const float y){
    float x_decimal, y_decimal;
    vec2 gradient[4];
    float extrap[4];
    float x_interpolator, y_interpolator;
    setup_perlin_noise_2D(x, y, x_decimal, y_decimal, gradient, extrap, x_interpolator, y_interpolator);

    vec2 output;
    derivate_perlin_noise_2D(x_decimal, y_decimal, gradient, extrap, x_interpolator, y_interpolator, output);
    return output;
}

void perlin_noise_and_derivatives(const float x, const float y, float& value, vec2& derivatives){
    float x_decimal, y_decimal;
    vec2 gradient[4];
    float extrap[4];
    float x_interpolator, y_interpolator;
    setup_perlin_noise_2D(x, y, x_decimal, y_decimal, gradient, extrap, x_interpolator, y_interpolator);
    evaluate_perlin_noise_2D(extrap, x_interpolator, y_interpolator, value);
    derivate_perlin_noise_2D(x_decimal, y_decimal, gradient, extrap, x_interpolator, y_interpolator, derivatives);
}

float simplex_noise(const float x){
    float x_integral = floorf(x);
    float x_decimal = x - x_integral;

    s32 origin = (s32)x_integral;
    float dL = x_decimal;
    float dR = - 1.f + x_decimal;

    // NOTE(hugo): determine gradients
    u32 seedL = fibonacci_hash((u32)origin);
    u32 seedR = fibonacci_hash((u32)origin + 1u);
    float gradL = random_float_normalized(seedL);
    float gradR = random_float_normalized(seedR);

    // NOTE(hugo): determining the weights and adding the weighted contributions
    // the unweighted contributions correspond to dot(gradient, direction to x) but in 1D
    float output = 0.f;

    constexpr float squared_simplex_height = 1.f;

    float weightL = squared_simplex_height - dL * dL;
    if(weightL > 0.f){
        weightL *= weightL;
        weightL *= weightL;

        float extrapL = dL * gradL;
        output += weightL * extrapL;
    }

    float weightR = squared_simplex_height - dR * dR;
    if(weightR > 0.f){
        weightR = weightR * weightR;
        weightR = weightR * weightR;

        float extrapR = dR * gradR;
        output += weightR * extrapR;
    }

    // NOTE(hugo): scaling to [-1, 1]
    constexpr float MAGIC_NORMALIZER = 3.160493827160493827;
    return MAGIC_NORMALIZER * output;
}

static inline void setup_simplex_noise_2D(const float x, const float y,
        s32& grid_x_origin, s32& grid_y_origin, float& origin_to_point_x, float& origin_to_point_y, float& weight_origin,
        s32& grid_x_corner, s32& grid_y_corner, float& corner_to_point_x, float& corner_to_point_y, float& weight_corner,
        float& opposite_to_point_x, float& opposite_to_point_y, float& weight_opposite
    ){

    constexpr float skew_factor = 0.36602540378443864676; // NOTE(hugo): (sqrt(3.f) - 1.f) * 0.5f
    constexpr float unskew_factor = 0.21132486540518711774; // NOTE(hugo): (3.f - sqrt(3.f)) / 6.f
    constexpr float squared_simplex_height = 0.5f;

    // NOTE(hugo): can be used to have a more 'expected' feature density with ~1 simplex per 1.f unit in (x, y)
    //float x = ix * squared_simplex_height;
    //float y = iy * squared_simplex_height;

    // NOTE(hugo): coordinate skewing ie hypercubic honeycomb transformed to a square grid
    float skew_temp = (x + y) * skew_factor;
    float grid_x = x + skew_temp;
    float grid_y = y + skew_temp;

    grid_x_origin = (s32)fast_floor<float, s32>(grid_x);
    grid_y_origin = (s32)fast_floor<float, s32>(grid_y);

    float unskew_temp = (float)(grid_x_origin + grid_y_origin) * unskew_factor;
    float origin_x = grid_x_origin - unskew_temp;
    float origin_y = grid_y_origin - unskew_temp;

    origin_to_point_x = x - origin_x;
    origin_to_point_y = y - origin_y;

    // NOTE(hugo): simplicial subdivision
    // in 2D we need to know if the triangle is the top (dy > dx) or bottom (dx > dy) triangle of
    // the skewed square with a diagonal [(0, 0), (1, 1)]
    if(origin_to_point_x > origin_to_point_y){
        grid_x_corner = 1;
        grid_y_corner = 0;
    }else{
        grid_x_corner = 0;
        grid_y_corner = 1;
    }

    corner_to_point_x = origin_to_point_x - (float)grid_x_corner + unskew_factor;
    corner_to_point_y = origin_to_point_y - (float)grid_y_corner + unskew_factor;
    opposite_to_point_x = origin_to_point_x - 1.f + 2.f * unskew_factor;
    opposite_to_point_y = origin_to_point_y - 1.f + 2.f * unskew_factor;

    // NOTE(hugo): kernel summation ie summing weights * contributions over the simplex vertices
    // gradient selection is delayed until needed
    weight_origin = squared_simplex_height - (origin_to_point_x * origin_to_point_x + origin_to_point_y * origin_to_point_y);
    weight_corner = squared_simplex_height - (corner_to_point_x * corner_to_point_x + corner_to_point_y * corner_to_point_y);
    weight_opposite = squared_simplex_height - (opposite_to_point_x * opposite_to_point_x + opposite_to_point_y * opposite_to_point_y);
}

float simplex_noise(const float x, const float y){
    s32 grid_x_origin, grid_y_origin;
    float origin_to_point_x, origin_to_point_y, weight_origin;
    s32 grid_x_corner, grid_y_corner;
    float corner_to_point_x, corner_to_point_y, weight_corner;
    float opposite_to_point_x, opposite_to_point_y, weight_opposite;
    setup_simplex_noise_2D(x, y,
            grid_x_origin, grid_y_origin, origin_to_point_x, origin_to_point_y, weight_origin,
            grid_x_corner, grid_y_corner, corner_to_point_x, corner_to_point_y, weight_corner,
            opposite_to_point_x, opposite_to_point_y, weight_opposite);

    float output = 0.f;

    if(weight_origin > 0.f){
        float weight_origin_pow2 = weight_origin * weight_origin;
        float weight_origin_pow4 = weight_origin_pow2 * weight_origin_pow2;
        s32 hash_data_origin[2] = {grid_x_origin, grid_y_origin};
        u32 hash_origin = combined_hash_32<fibonacci_hash>((u32*)hash_data_origin, 2u);
        vec2 gradient_origin = perlin_gradient_2D_cache(hash_origin);
        float extrap_origin = origin_to_point_x * gradient_origin.x + origin_to_point_y * gradient_origin.y;

        output += weight_origin_pow4 * extrap_origin;
    }

    if(weight_corner > 0.f){
        float weight_corner_pow2 = weight_corner * weight_corner;
        float weight_corner_pow4 = weight_corner_pow2 * weight_corner_pow2;
        s32 hash_data_corner[2] = {grid_x_origin + grid_x_corner, grid_y_origin + grid_y_corner};
        u32 hash_corner = combined_hash_32<fibonacci_hash>((u32*)hash_data_corner, 2u);
        vec2 gradient_corner = perlin_gradient_2D_cache(hash_corner);
        float extrap_corner = corner_to_point_x * gradient_corner.x + corner_to_point_y * gradient_corner.y;

        output += weight_corner_pow4 * extrap_corner;
    }

    if(weight_opposite > 0.f){
        float weight_opposite_pow2 = weight_opposite * weight_opposite;
        float weight_opposite_pow4 = weight_opposite_pow2 * weight_opposite_pow2;
        s32 hash_data_opposite[2] = {grid_x_origin + 1, grid_y_origin + 1};
        u32 hash_opposite = combined_hash_32<fibonacci_hash>((u32*)hash_data_opposite, 2u);
        vec2 gradient_opposite = perlin_gradient_2D_cache(hash_opposite);
        float extrap_opposite = opposite_to_point_x * gradient_opposite.x + opposite_to_point_y * gradient_opposite.y;

        output += weight_opposite_pow4 * extrap_opposite;
    }

    // NOTE(hugo): output scaling to [-1, 1]
    constexpr float MAGIC_NORMALIZER = 99.204334582718712;
    output *= MAGIC_NORMALIZER;

    return output;
}

vec2 simplex_derivatives(const float x, const float y){
    s32 grid_x_origin, grid_y_origin;
    float origin_to_point_x, origin_to_point_y, weight_origin;
    s32 grid_x_corner, grid_y_corner;
    float corner_to_point_x, corner_to_point_y, weight_corner;
    float opposite_to_point_x, opposite_to_point_y, weight_opposite;
    setup_simplex_noise_2D(x, y,
            grid_x_origin, grid_y_origin, origin_to_point_x, origin_to_point_y, weight_origin,
            grid_x_corner, grid_y_corner, corner_to_point_x, corner_to_point_y, weight_corner,
            opposite_to_point_x, opposite_to_point_y, weight_opposite);

    vec2 derivatives = {0.f, 0.f};

    if(weight_origin > 0.f){
        float weight_origin_pow2 = weight_origin * weight_origin;
        float weight_origin_pow4 = weight_origin_pow2 * weight_origin_pow2;
        s32 hash_data_origin[2] = {grid_x_origin, grid_y_origin};
        u32 hash_origin = combined_hash_32<fibonacci_hash>((u32*)hash_data_origin, 2u);
        vec2 gradient_origin = perlin_gradient_2D_cache(hash_origin);
        float extrap_origin = origin_to_point_x * gradient_origin.x + origin_to_point_y * gradient_origin.y;

        derivatives.x += gradient_origin.x * weight_origin_pow4 - 8.f * origin_to_point_x * weight_origin_pow2 * weight_origin * extrap_origin;
        derivatives.y += gradient_origin.y * weight_origin_pow4 - 8.f * origin_to_point_y * weight_origin_pow2 * weight_origin * extrap_origin;
    }

    if(weight_corner > 0.f){
        float weight_corner_pow2 = weight_corner * weight_corner;
        float weight_corner_pow4 = weight_corner_pow2 * weight_corner_pow2;
        s32 hash_data_corner[2] = {grid_x_origin + grid_x_corner, grid_y_origin + grid_y_corner};
        u32 hash_corner = combined_hash_32<fibonacci_hash>((u32*)hash_data_corner, 2u);
        vec2 gradient_corner = perlin_gradient_2D_cache(hash_corner);
        float extrap_corner = corner_to_point_x * gradient_corner.x + corner_to_point_y * gradient_corner.y;

        derivatives.x += gradient_corner.x * weight_corner_pow4 - 8.f * corner_to_point_x * weight_corner_pow2 * weight_corner * extrap_corner;
        derivatives.y += gradient_corner.y * weight_corner_pow4 - 8.f * corner_to_point_y * weight_corner_pow2 * weight_corner * extrap_corner;
    }

    if(weight_opposite > 0.f){
        float weight_opposite_pow2 = weight_opposite * weight_opposite;
        float weight_opposite_pow4 = weight_opposite_pow2 * weight_opposite_pow2;
        s32 hash_data_opposite[2] = {grid_x_origin + 1, grid_y_origin + 1};
        u32 hash_opposite = combined_hash_32<fibonacci_hash>((u32*)hash_data_opposite, 2u);
        vec2 gradient_opposite = perlin_gradient_2D_cache(hash_opposite);
        float extrap_opposite = opposite_to_point_x * gradient_opposite.x + opposite_to_point_y * gradient_opposite.y;

        derivatives.x += gradient_opposite.x * weight_opposite_pow4 - 8.f * opposite_to_point_x * weight_opposite_pow2 * weight_opposite * extrap_opposite;
        derivatives.y += gradient_opposite.y * weight_opposite_pow4 - 8.f * opposite_to_point_y * weight_opposite_pow2 * weight_opposite * extrap_opposite;
    }

    // NOTE(hugo): output scaling to [-1, 1]
    constexpr float MAGIC_NORMALIZER = 99.204334582718712;
    derivatives *= MAGIC_NORMALIZER;

    return derivatives;
}

void simplex_noise_and_derivatives(const float x, const float y, float& value, vec2& derivatives){
    s32 grid_x_origin, grid_y_origin;
    float origin_to_point_x, origin_to_point_y, weight_origin;
    s32 grid_x_corner, grid_y_corner;
    float corner_to_point_x, corner_to_point_y, weight_corner;
    float opposite_to_point_x, opposite_to_point_y, weight_opposite;
    setup_simplex_noise_2D(x, y,
            grid_x_origin, grid_y_origin, origin_to_point_x, origin_to_point_y, weight_origin,
            grid_x_corner, grid_y_corner, corner_to_point_x, corner_to_point_y, weight_corner,
            opposite_to_point_x, opposite_to_point_y, weight_opposite);

    value = 0.f;
    derivatives = {0.f, 0.f};

    if(weight_origin > 0.f){
        float weight_origin_pow2 = weight_origin * weight_origin;
        float weight_origin_pow4 = weight_origin_pow2 * weight_origin_pow2;
        s32 hash_data_origin[2] = {grid_x_origin, grid_y_origin};
        u32 hash_origin = combined_hash_32<fibonacci_hash>((u32*)hash_data_origin, 2u);
        vec2 gradient_origin = perlin_gradient_2D_cache(hash_origin);
        float extrap_origin = origin_to_point_x * gradient_origin.x + origin_to_point_y * gradient_origin.y;

        value += weight_origin_pow4 * extrap_origin;
        derivatives.x += gradient_origin.x * weight_origin_pow4 - 8.f * origin_to_point_x * weight_origin_pow2 * weight_origin * extrap_origin;
        derivatives.y += gradient_origin.y * weight_origin_pow4 - 8.f * origin_to_point_y * weight_origin_pow2 * weight_origin * extrap_origin;
    }

    if(weight_corner > 0.f){
        float weight_corner_pow2 = weight_corner * weight_corner;
        float weight_corner_pow4 = weight_corner_pow2 * weight_corner_pow2;
        s32 hash_data_corner[2] = {grid_x_origin + grid_x_corner, grid_y_origin + grid_y_corner};
        u32 hash_corner = combined_hash_32<fibonacci_hash>((u32*)hash_data_corner, 2u);
        vec2 gradient_corner = perlin_gradient_2D_cache(hash_corner);
        float extrap_corner = corner_to_point_x * gradient_corner.x + corner_to_point_y * gradient_corner.y;

        value += weight_corner_pow4 * extrap_corner;
        derivatives.x += gradient_corner.x * weight_corner_pow4 - 8.f * corner_to_point_x * weight_corner_pow2 * weight_corner * extrap_corner;
        derivatives.y += gradient_corner.y * weight_corner_pow4 - 8.f * corner_to_point_y * weight_corner_pow2 * weight_corner * extrap_corner;
    }

    if(weight_opposite > 0.f){
        float weight_opposite_pow2 = weight_opposite * weight_opposite;
        float weight_opposite_pow4 = weight_opposite_pow2 * weight_opposite_pow2;
        s32 hash_data_opposite[2] = {grid_x_origin + 1, grid_y_origin + 1};
        u32 hash_opposite = combined_hash_32<fibonacci_hash>((u32*)hash_data_opposite, 2u);
        vec2 gradient_opposite = perlin_gradient_2D_cache(hash_opposite);
        float extrap_opposite = opposite_to_point_x * gradient_opposite.x + opposite_to_point_y * gradient_opposite.y;

        value += weight_opposite_pow4 * extrap_opposite;
        derivatives.x += gradient_opposite.x * weight_opposite_pow4 - 8.f * opposite_to_point_x * weight_opposite_pow2 * weight_opposite * extrap_opposite;
        derivatives.y += gradient_opposite.y * weight_opposite_pow4 - 8.f * opposite_to_point_y * weight_opposite_pow2 * weight_opposite * extrap_opposite;
    }

    // NOTE(hugo): output scaling to [-1, 1]
    constexpr float MAGIC_NORMALIZER = 99.204334582718712;
    value *= MAGIC_NORMALIZER;
    derivatives *= MAGIC_NORMALIZER;
}

template<float (*noise_derivatives_function)(const float x, const float y)>
vec2 noise_curl(const float x, const float y){
    vec2 potential = noise_derivatives_function(x, y);
    potential.y = - potential.y;
    return potential;
}
