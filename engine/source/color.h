#ifndef H_COLOR
#define H_COLOR

// NOTE(hugo):
// * sRGB = color-space with reverse gamma-correction ie values are convex (U) to be linear on screen
//          most stored colors (texture, ...) are in sRGB
// * RGB  = color-space with linear values ie values are linear to be concave () on screen unless corrected (enabled in easy_setup)
//          use this color-space for interpolation
// * HSV

// REF(hugo):
// - sRGB | RGB
// https://github.com/apitrace/dxsdk/blob/master/Include/d3dx_dxgiformatconvert.inl
// - RGB | HSV
// http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
// http://lolengine.net/blog/2013/01/13/fast-rgb-to-hsv

// ---- format conversion

u32 rgba32(float r, float g, float b, float a);
u32 rgba32(const vec4& rgba);
vec4 rgbaf(u32 rgba);

u32 rgba32_set_r(u32 rgba, float r);
u32 rgba32_set_g(u32 rgba, float g);
u32 rgba32_set_b(u32 rgba, float b);
u32 rgba32_set_a(u32 rgba, float a);

u32 uv32(float u, float v);
u32 uv32(const vec2& uv);
vec2 uvf(u32 uv);

u32 uv32_u(u32 uv, float u);
u32 uv32_v(u32 uv, float v);

// ---- color space conversion

float schan_to_chan(const float c);
float chan_to_schan(const float c);
vec4 srgba_to_rgba(const vec4& c);
vec4 rgba_to_srgba(const vec4& c);

vec4 rgba_to_hsva(const vec4& c);
vec4 hsva_to_rgba(const vec4& c);

// ---- operation

vec4 mix(const vec4& cA, const vec4& cB, const float t);

vec4 inverted(vec4 rgbaf);
u32 inverted(u32 rgba);

vec4 complementary_hue(vec4 hsva);
vec4 fibonacci_hue(vec4 hsva);

// ---- colormaps

// NOTE(hugo): Polynomials fitted to matplotlib colormaps with value in [0, 1]
// https://www.shadertoy.com/view/WlfXRN
void viridis(float value, float& r, float& g, float& b);
void plasma(float value, float& r, float& g, float& b);
void magma(float value, float& r, float& g, float& b);
void inferno(float value, float& r, float& g, float& b);

#endif
