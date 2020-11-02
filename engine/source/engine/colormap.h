#ifndef H_COLORMAP
#define H_COLORMAP

u8 color_from_float(const float value, const float min_value, const float max_value);

// NOTE(hugo): Polynomials fitted to matplotlib colormaps with value in [0, 1]
// https://www.shadertoy.com/view/WlfXRN
void viridis(const float value, float& r, float& g, float& b);
void plasma(const float value, float& r, float& g, float& b);
void magma(const float value, float& r, float& g, float& b);
void inferno(const float value, float& r, float& g, float& b);

#endif
