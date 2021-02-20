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

float srgb_to_rgb(const float c);
float rgb_to_srgb(const float c);
vec4 srgb_to_rgb(const vec4& c);
vec4 rgb_to_srgb(const vec4& c);

vec4 rgb_to_hsv(const vec4& c);
vec4 hsv_to_rgb(const vec4& c);

vec4 mix(const vec4& cA, const vec4& cB, const float t);

#endif
