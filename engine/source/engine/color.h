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

u32 rgba32(const float r, const float g, const float b, const float a);
u32 rgba32(const vec4& rgba);

// ---- color space conversion

float srgb_to_rgb(const float c);
float rgb_to_srgb(const float c);
vec4 srgb_to_rgb(const vec4& c);
vec4 rgb_to_srgb(const vec4& c);

vec4 rgb_to_hsv(const vec4& c);
vec4 hsv_to_rgb(const vec4& c);

// ---- mixing

vec4 mix(const vec4& cA, const vec4& cB, const float t);

// ---- colormaps

// NOTE(hugo): Polynomials fitted to matplotlib colormaps with value in [0, 1]
// https://www.shadertoy.com/view/WlfXRN
void viridis(const float value, float& r, float& g, float& b);
void plasma(const float value, float& r, float& g, float& b);
void magma(const float value, float& r, float& g, float& b);
void inferno(const float value, float& r, float& g, float& b);

#endif
