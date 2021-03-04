// ---- format conversion

u32 rgba32(const float r, const float g, const float b, const float a){
    assert(r >= 0. && g >= 0. && b >= 0. && a >= 0. && r <= 1. && g <= 1. && b <= 1. && a <= 1.);

    u32 out = 0;
    out |= ((u32)(r * 255.f + 0.5f) & 0xFF) << 24u;
    out |= ((u32)(g * 255.f + 0.5f) & 0xFF) << 16u;
    out |= ((u32)(b * 255.f + 0.5f) & 0xFF) << 8u;
    out |= ((u32)(a * 255.f + 0.5f) & 0xFF);

    return out;
}

u32 rgba32(const vec4& rgba){
    return rgba32(rgba.r, rgba.g, rgba.b, rgba.a);
}

// ---- color space conversion

float srgb_to_rgb(const float c){
    if(c < 0.04045f) return c / 12.92f;
    else return pow((c + 0.055f) * (1.f / 1.055f), 2.4f);
}

float rgb_to_srgb(const float c){
    if(c < 0.0031308f) return c * 12.92f;
    else return 1.055f * pow(c, 1.f / 2.4f) - 0.055f;
}

vec4 srgb_to_rgb(const vec4& c){
    return {
        srgb_to_rgb(c.r),
        srgb_to_rgb(c.g),
        srgb_to_rgb(c.b),
        c.a
    };
}

vec4 rgb_to_srgb(const vec4& c){
    return {
        rgb_to_srgb(c.r),
        rgb_to_srgb(c.g),
        rgb_to_srgb(c.b),
        c.a
    };
}

vec4 rgb_to_hsv(const vec4& c_in){
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

    return {h, s, v, c_in.a};
}

vec4 mix(const vec4& cA, const vec4& cB, const float t){
    float opp_t = 1.f - t;
    return cA * opp_t + cB * t;
}

// ---- colormaps

void viridis(const float value, float& r, float& g, float& b){
    assert(value >= 0.f && value <= 1.f);

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

void plasma(const float value, float& r, float& g, float& b){
    assert(value >= 0.f && value <= 1.f);

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

void magma(const float value, float& r, float& g, float& b){
    assert(value >= 0.f && value <= 1.f);

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

void inferno(const float value, float& r, float& g, float& b){
    assert(value >= 0.f && value <= 1.f);

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
