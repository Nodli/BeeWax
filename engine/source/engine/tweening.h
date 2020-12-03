#ifndef H_TWEENING
#define H_TWEENING

// REF(hugo): https://easings.net/fr
typedef float (*Tween_Function)(float t);

float ease_none(float t);

float ease_out_quint(float t);
float ease_out_back(float t);

template<Tween_Function>
float yoyo(float t);

template<typename T>
struct Tween{
    // NOTE(hugo): use as T
    operator T () const;
    Tween<T>& operator=(const Tween<T>& rhs);

    void set(const T& value);
    void from(const T& start, const T& stop,
            const u32 tick_duration, bool repeat = false,
            Tween_Function func = &ease_out_quint);
    void tick();

    // ---- data

    T value = {};
    T base = {};
    T range = {};
    u32 tick_count = 0u;
    s32 repeat_tick_duration = 1;
    Tween_Function function = &ease_none;
};

float ease_none(float t){
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

template<Tween_Function func>
float yoyo(float t){
    return func(abs(2.f * (t - 0.5f)));
}

template<typename T>
Tween<T>::operator T () const{
    return value;
}

template<typename T>
Tween<T>& Tween<T>::operator=(const Tween<T>& rhs){
    tick_count = rhs.tick_count;
    repeat_tick_duration = rhs.tick_duration;
    value = rhs.value;
    base = rhs.base;
    range = rhs.range;
    return *this;
}

template<typename T>
void Tween<T>::set(const T& ivalue){
    value = value;
    base = value;
    range = {};
    tick_count = 1u;
    repeat_tick_duration = 1u;
    return *this;
}

template<typename T>
void Tween<T>::from(const T& start, const T& stop, const u32 itick_duration, bool repeat, Tween_Function ifunc){
    value = start;
    base = start;
    range = stop - start;
    tick_count = 0u;
    repeat_tick_duration = repeat ? (- itick_duration) : (itick_duration);
    function = ifunc;
}

template<typename T>
void Tween<T>::tick(){
    bool repeat = repeat_tick_duration < 0;
    u32 tick_duration = abs(repeat_tick_duration);

    value = base + range * function((float)tick_count / (float)tick_duration);
    tick_count = min(tick_count + 1u, tick_duration);

    if(repeat && tick_count == tick_duration){
        tick_count = 0u;
    }
}

#endif
