// ---- format conversion

u32 rgba32(float r, float g, float b, float a){
    r = min_max(r, 0.f, 1.f);
    g = min_max(g, 0.f, 1.f);
    b = min_max(b, 0.f, 1.f);
    a = min_max(a, 0.f, 1.f);

    u32 out = 0u;
    out |= ((u32)(a * 0xFF + 0.5f) & 0xFF) << 24u;
    out |= ((u32)(b * 0xFF + 0.5f) & 0xFF) << 16u;
    out |= ((u32)(g * 0xFF + 0.5f) & 0xFF) << 8u;
    out |= ((u32)(r * 0xFF + 0.5f) & 0xFF);

    return out;
}

u32 rgba32(const vec4& rgba){
    return rgba32(rgba.r, rgba.g, rgba.b, rgba.a);
}

vec4 rgbaf(u32 rgba){
    vec4 out;
    out.r = (float)(rgba & 0x000000FF) * 0xFF;
    out.g = (float)((rgba & 0x0000FF00) >> 8u) * 0xFF;
    out.b = (float)((rgba & 0x00FF0000) >> 16u) * 0xFF;
    out.a = (float)((rgba & 0xFF000000) >> 24u) * 0xFF;
    return out;
}

u32 rgba32_set_r(u32 rgba, float r){
    r = min_max(r, 0.f, 1.f);
    return (rgba & 0xFFFFFF00) | ((u32)(r * 0xFF + 0.5f) & 0xFF);
}
u32 rgba32_set_g(u32 rgba, float g){
    g = min_max(g, 0.f, 1.f);
    return (rgba & 0xFFFF00FF) | (((u32)(g * 0xFF + 0.5f) & 0xFF) << 8u);
}
u32 rgba32_set_b(u32 rgba, float b){
    b = min_max(b, 0.f, 1.f);
    return (rgba & 0xFF00FFFF) | (((u32)(b * 0xFF + 0.5f) & 0xFF) << 16u);
}
u32 rgba32_set_a(u32 rgba, float a){
    a = min_max(a, 0.f, 1.f);
    return (rgba & 0x00FFFFFF) | (((u32)(a * 0xFF + 0.5f) & 0xFF) << 24u);
}

u32 uv32(float u, float v){
    u = min_max(u, 0.f, 1.f);
    v = min_max(v, 0.f, 1.f);

    u32 out = 0u;
    out |= ((u32)(v * 0xFFFF + 0.5f) & 0xFFFF) << 16u;
    out |= ((u32)(u * 0xFFFF + 0.5f) & 0xFFFF);

    return out;
}

u32 uv32(const vec2& uv){
    return uv32(uv.u, uv.v);
}

vec2 uvf(u32 uv){
    vec2 out;
    out.u = (float)(uv & 0x0000FFFF) * 0xFFFF;
    out.v = (float)((uv & 0xFFFF0000) >> 16u) * 0xFFFF;
    return out;
}

u32 uv32_u(u32 uv, float u){
    u = min_max(u, 0.f, 1.f);
    return (uv & 0xFFFF0000) | ((u32)(u * 0xFFFF + 0.5f) & 0x0000FFFF);

}
u32 uv32_v(u32 uv, float v){
    v = min_max(v, 0.f, 1.f);
    return (uv & 0xFFFF0000) | ((u32)(v * 0xFFFF + 0.5f) & 0xFFFF0000);
}

// ---- color space conversion

float schan_to_chan(const float c){
    if(c < 0.04045f) return c / 12.92f;
    else return pow((c + 0.055f) * (1.f / 1.055f), 2.4f);
}

float chan_to_schan(const float c){
    if(c < 0.0031308f) return c * 12.92f;
    else return 1.055f * pow(c, 1.f / 2.4f) - 0.055f;
}

vec4 srgba_to_rgba(const vec4& c){
    return {
        schan_to_chan(c.r),
        schan_to_chan(c.g),
        schan_to_chan(c.b),
        c.a
    };
}

vec4 rgba_to_srgba(const vec4& c){
    return {
        chan_to_schan(c.r),
        chan_to_schan(c.g),
        chan_to_schan(c.b),
        c.a
    };
}

vec4 rgba_to_hsva(const vec4& c_in){
    float r = c_in.r;
    float g = c_in.g;
    float b = c_in.b;
    float K = 0.f;

    if(g < b){
        swap(g, b);
        K = -1.f;
    }
    if(r < g){
        swap(r, g);
        K = -2.f / 6.f - K;
    }

    float chroma = r - min(g, b);
    float h = fabs(K + (g - b) / (6.f * chroma + 1e-20f));
    float s = chroma / (r + 1e-20f);
    float v = r;

    return {min_max(h, 0.f, 1.f), min_max(s, 0.f, 1.f), min_max(v, 0.f, 1.f), c_in.a};
}

vec4 hsva_to_rgba(const vec4& c_in){
    float chroma = c_in.v * c_in.s;
    float cmin = c_in.v - chroma;

    float fract = c_in.h * 3.f;
    u32 index = min((u32)fract, 2u);
    fract = 2.f * (fract - (float)index);

    float V1 = chroma * min(2.f - fract, 1.f);
    float V2 = chroma * min(fract, 1.f);

    vec4 out;
    out.data[index] = V1 + cmin;
    out.data[(index + 1u) % 3u] = V2 + cmin;
    out.data[(index + 2u) % 3u] = cmin;
    out.a = c_in.a;

    return out;
}

// ---- operation

vec4 mix(const vec4& cA, const vec4& cB, const float t){
    float opp_t = 1.f - t;
    return cA * opp_t + cB * t;
}

vec4 inverted(vec4 rgbaf){
    rgbaf.r = 1.f - rgbaf.r;
    rgbaf.g = 1.f - rgbaf.g;
    rgbaf.b = 1.f - rgbaf.b;
    return rgbaf;
}

u32 inverted(u32 rgba){
    u32 out = rgba & 0xFF000000;
    out |= (0xFF - rgba & 0xFF) & 0xFF;
    out |= ((0xFF - (rgba >>  8u) & 0xFF) & 0xFF) <<  8u;
    out |= ((0xFF - (rgba >> 16u) & 0xFF) & 0xFF) << 16u;
    return out;
}

vec4 complementary_hue(vec4 hsva){
    float chue = hsva.h + 0.5f;
    if(chue > 1.f) chue = chue - 1.f;
    return {chue, hsva.s, hsva.v, hsva.a};
}

vec4 fibonacci_hue(vec4 hsva){
    float fhue = hsva.h + GOLDEN_RATIO_FRACT;
    if(fhue > 1.f) fhue = fhue - 1.f;
    return {fhue, hsva.s, hsva.v, hsva.a};
}

// ---- colormaps

void viridis(float value, float& r, float& g, float& b){
    value = min_max(value, 0.f, 1.f);

    constexpr float c0[3] = {0.2777273272234177, 0.005407344544966578, 0.3340998053353061};
    constexpr float c1[3] = {0.1050930431085774, 1.404613529898575, 1.384590162594685};
    constexpr float c2[3] = {-0.3308618287255563, 0.214847559468213, 0.09509516302823659};
    constexpr float c3[3] = {-4.634230498983486, -5.799100973351585, -19.33244095627987};
    constexpr float c4[3] = {6.228269936347081, 14.17993336680509, 56.69055260068105};
    constexpr float c5[3] = {4.776384997670288, -13.74514537774601, -65.35303263337234};
    constexpr float c6[3] = {-5.435455855934631, 4.645852612178535, 26.3124352495832};

    r = c0[0] + value * (c1[0] + value * ( c2[0] + value * ( c3[0] + value * ( c4[0] + value * (c5[0] + value * c6[0])))));
    g = c0[1] + value * (c1[1] + value * ( c2[1] + value * ( c3[1] + value * ( c4[1] + value * (c5[1] + value * c6[1])))));
    b = c0[2] + value * (c1[2] + value * ( c2[2] + value * ( c3[2] + value * ( c4[2] + value * (c5[2] + value * c6[2])))));
}

void plasma(float value, float& r, float& g, float& b){
    value = min_max(value, 0.f, 1.f);

    constexpr float c0[3] = {0.05873234392399702, 0.02333670892565664, 0.5433401826748754};
    constexpr float c1[3] = {2.176514634195958, 0.2383834171260182, 0.7539604599784036};
    constexpr float c2[3] = {-2.689460476458034, -7.455851135738909, 3.110799939717086};
    constexpr float c3[3] = {6.130348345893603, 42.3461881477227, -28.51885465332158};
    constexpr float c4[3] = {-11.10743619062271, -82.66631109428045, 60.13984767418263};
    constexpr float c5[3] = {10.02306557647065, 71.41361770095349, -54.07218655560067};
    constexpr float c6[3] = {-3.658713842777788, -22.93153465461149, 18.19190778539828};

    r = c0[0] + value * (c1[0] + value * ( c2[0] + value * ( c3[0] + value * ( c4[0] + value * (c5[0] + value * c6[0])))));
    g = c0[1] + value * (c1[1] + value * ( c2[1] + value * ( c3[1] + value * ( c4[1] + value * (c5[1] + value * c6[1])))));
    b = c0[2] + value * (c1[2] + value * ( c2[2] + value * ( c3[2] + value * ( c4[2] + value * (c5[2] + value * c6[2])))));
}

void magma(float value, float& r, float& g, float& b){
    value = min_max(value, 0.f, 1.f);

    constexpr float c0[3] = {-0.002136485053939582, -0.000749655052795221, -0.005386127855323933};
    constexpr float c1[3] = {0.2516605407371642, 0.6775232436837668, 2.494026599312351};
    constexpr float c2[3] = {8.353717279216625, -3.577719514958484, 0.3144679030132573};
    constexpr float c3[3] = {-27.66873308576866, 14.26473078096533, -13.64921318813922};
    constexpr float c4[3] = {52.17613981234068, -27.94360607168351, 12.94416944238394};
    constexpr float c5[3] = {-50.76852536473588, 29.04658282127291, 4.23415299384598};
    constexpr float c6[3] = {18.65570506591883, -11.48977351997711, -5.601961508734096};

    r = c0[0] + value * (c1[0] + value * ( c2[0] + value * ( c3[0] + value * ( c4[0] + value * (c5[0] + value * c6[0])))));
    g = c0[1] + value * (c1[1] + value * ( c2[1] + value * ( c3[1] + value * ( c4[1] + value * (c5[1] + value * c6[1])))));
    b = c0[2] + value * (c1[2] + value * ( c2[2] + value * ( c3[2] + value * ( c4[2] + value * (c5[2] + value * c6[2])))));
}

void inferno(float value, float& r, float& g, float& b){
    value = min_max(value, 0.f, 1.f);

    constexpr float c0[3] = {0.0002189403691192265, 0.001651004631001012, -0.01948089843709184};
    constexpr float c1[3] = {0.1065134194856116, 0.5639564367884091, 3.932712388889277};
    constexpr float c2[3] = {11.60249308247187, -3.972853965665698, -15.9423941062914};
    constexpr float c3[3] = {-41.70399613139459, 17.43639888205313, 44.35414519872813};
    constexpr float c4[3] = {77.162935699427, -33.40235894210092, -81.80730925738993};
    constexpr float c5[3] = {-71.31942824499214, 32.62606426397723, 73.20951985803202};
    constexpr float c6[3] = {25.13112622477341, -12.24266895238567, -23.07032500287172};

    r = c0[0] + value * (c1[0] + value * ( c2[0] + value * ( c3[0] + value * ( c4[0] + value * (c5[0] + value * c6[0])))));
    g = c0[1] + value * (c1[1] + value * ( c2[1] + value * ( c3[1] + value * ( c4[1] + value * (c5[1] + value * c6[1])))));
    b = c0[2] + value * (c1[2] + value * ( c2[2] + value * ( c3[2] + value * ( c4[2] + value * (c5[2] + value * c6[2])))));
}
