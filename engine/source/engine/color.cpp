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
