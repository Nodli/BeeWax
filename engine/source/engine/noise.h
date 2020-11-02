#ifndef H_NOISE
#define H_NOISE

// REF(hugo):
// https://mrl.nyu.edu/~perlin/paper445.pdf
// https://mrl.nyu.edu/~perlin/doc/oscar.html
// http://webstaff.itn.liu.se/~stegu/simplexnoise/simplexnoise.pdf
// https://en.wikipedia.org/wiki/Simplex_noise
// https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/perlin-noise-part-2/perlin-noise-computing-derivatives

// https://digitalfreepen.com/2017/06/20/range-perlin-noise.html
// Perlin normalization constants are sqrt(N / 4) with N the number of dimensions

// https://briansharpe.wordpress.com/2012/01/13/simplex-noise/#comment-36
// https://math.stackexchange.com/questions/474638/radius-and-amplitude-of-kernel-for-simplex-noise
// 1D max  = ((1 - x^2)^4 * dot(x, 1)) +  ((1 - (x - 1)^2)^4 * dot(x - 1, -1)) with x = 1 / 2 = 0.5
//         = (0.75)^4 * 0.5 + (0.75)^4 * 0.5
//         = (0.75)^4
//         = 0.31640625
// 1D norm = 1 / (1 * 1D max) = 3.160493827160493827
// 2D max  = ((0.5 - |v|^2)^4 * |v|) + ((0.5 - |-v|^2)^4 * |v|) with |v| = sqrt(2 / 3) / 2 = sqrt(1 / 6)
//         = (1/3)^4 * sqrt(2 / 3)
//         = 0.010080204702811
// 2D norm = 1 / 2D max = 99.204334582718712

// https://stackoverflow.com/questions/18340334/simplex-noise-summation
// Perlin and Simplex Noise work as follow
// - find the definition (position of the simplex vertices) of the simplex based on the point position
//    Perlin : segment / square / cube
//    Simplex : segment / triangle / tetrahedron
// - determine random gradients ni and compute dot(vertex_to_x, gradient) at each of the simplex vertices
// - noise = weighted sum of the previous results
//    Perlin : linear interpolation based on distance(simplex vertex, point)
//    Simplex : radially symetric attenuation function based on distance(simplex vertex, point)
//              max(0.5 - dist ^ 2, 0.) ^ 4

// NOTE(hugo): Ken Perlin's "Improved Noise" but using a hash function instead of a permutation table
// noise values are in [-1; 1)
float perlin_noise(const float x);
float perlin_noise(const float x, const float y);
vec2 perlin_derivatives(const float x, const float y);
void perlin_noise_and_derivatives(const float x, const float y, float& value, vec2& derivatives);

// NOTE(hugo): Ken Perlin's "Simplex Noise" which is patented for 3D or more dimensions until 08/01/2022
float simplex_noise(const float x);
float simplex_noise(const float x, const float y);
vec2 simplex_derivatives(const float x, const float y);

// REF(hugo): http://weber.itn.liu.se/~stegu/TNM084-2019/bridson-siggraph2007-curlnoise.pdf
template<float (*noise_derivatives_function)(const float x, const float y)>
vec2 noise_curl(const float x, const float y);

template<float (*noise_function)(const float x, const float y)>
u8* generate_noise_texture(u32 width, u32 height, u32 nchannels, vec2 origin, float span){
    u8* texture_data = (u8*)malloc(width * height * nchannels);

    for(u32 iy = 0; iy != 256; ++iy){
        float coord_iy = origin.y + iy * span;
        for(u32 ix = 0; ix != 256; ++ix){
            float noise_value = noise_function(origin.x + ix * span, coord_iy);
            u8 color_value = color_from_float(noise_value, -1.f, 1.f);

            for(u32 ichannel = 0u; ichannel != nchannels; ++ichannel){
                texture_data[index2D(ix, iy, 256u) * nchannels + ichannel] = color_value;
            }
        }
    }

    return texture_data;
}

template<void (*noise_function)(const float x, const float y, float& value, vec2& derivatives)>
void* generate_noise_derivatives_textures(u32 width, u32 height, u32 nchannels, vec2 origin, float span, u8*& noise, u8*& noise_dx, u8*& noise_dy){
    size_t texture_size = (size_t)(width * height * nchannels);
    void* memory = nullptr;
    memory = malloc(texture_size * 3u);
    noise = (u8*)memory;
    noise_dx = (u8*)memory + texture_size;
    noise_dy = (u8*)memory + 2u * texture_size;

    for(u32 iy = 0; iy != 256; ++iy){
        float coord_iy = origin.y + iy * span;
        for(u32 ix = 0; ix != 256; ++ix){

            float value;
            vec2 deriv;
            noise_function(origin.x + ix * span, coord_iy, value, deriv);

            u8 color_value = color_from_float(value, -1.f, 1.f);
            u8 dx_value = color_from_float(deriv.x, -1.f, 1.f);
            u8 dy_value = color_from_float(deriv.y, -1.f, 1.f);

            for(u32 ichannel = 0u; ichannel != nchannels; ++ichannel){
                noise[index2D(ix, iy, 256u) * nchannels + ichannel] = color_value;
                noise_dx[index2D(ix, iy, 256u) * nchannels + ichannel] = dx_value;
                noise_dy[index2D(ix, iy, 256u) * nchannels + ichannel] = dy_value;
            }
        }
    }

    return memory;
}

#if 0
// TODO(hugo): https://en.wikipedia.org/wiki/Worley_noise
// TODO(hugo): http://iquilezles.org/www/articles/smoothvoronoi/smoothvoronoi.htm
// TODO(hugo): http://iquilezles.org/www/articles/voronoise/voronoise.htm
// TODO(hugo): https://www.reddit.com/r/VoxelGameDev/comments/ee94wg/supersimplex_the_better_opensimplex_new_gradient/
#endif

#endif
