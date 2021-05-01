#ifndef H_TWEEN
#define H_TWEEN

// ---- easing functions

// REF(hugo): https://easings.net/fr
typedef float (*Tween_Function)(float t);

float ease_identity(float t);

float ease_out_quint(float t);
float ease_out_back(float t);

template<Tween_Function>
float ease_yoyo(float t);

// --

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
    Tween_Function function = &ease_identity;
};

#include "tween.inl"

#endif
