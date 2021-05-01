float ease_identity(float t){
    return t;
}

float ease_out_quint(float t){
    float interm = 1.f - t;
    float interm2 = interm * interm;
    return 1.f - interm2 * interm2 * interm;
}

float ease_out_back(float t){
    constexpr float c1 = 1.70158f;
    constexpr float c3 = c1 + 1.f;

    float interm = t - 1.f;
    float interm2 = interm * interm;

    return 1.f + c3 * interm2 * interm + c1 * interm2;
}
