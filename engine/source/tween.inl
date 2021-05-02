template<typename T>
Tween<T>::operator T () const{
    u32 tick_duration = abs(repeat_tick_duration);
    float interpolator = tick_duration ? ((float)tick_count / (float)tick_duration) : (0.f);
    return base + range * function(interpolator);
}

template<typename T>
Tween<T>& Tween<T>::operator=(const T& rhs){
    base = rhs;
    tick_count = 0u;
    repeat_tick_duration = 0u;
}

template<typename T>
Tween<T>& Tween<T>::operator=(const Tween<T>& rhs){
    tick_count = rhs.tick_count;
    repeat_tick_duration = rhs.tick_duration;
    base = rhs.base;
    range = rhs.range;
    return *this;
}

template<typename T>
void Tween<T>::interpolate(const T& start, const T& stop, const u32 itick_duration, bool repeat, Tween_Function ifunc){
    base = start;
    range = stop - start;
    tick_count = 0u;
    repeat_tick_duration = repeat ? (- itick_duration) : (itick_duration);
    function = ifunc;
}

template<typename T>
void Tween<T>::tick(){
    u32 tick_duration = (u32)abs(repeat_tick_duration);
    tick_count = min(tick_count + 1u, tick_duration);
    if(tick_count == tick_duration && repeat_tick_duration < 0){
        tick_count = 0u;
    }
}

template<typename T>
float Tween<T>::progress(){
    if(repeat_tick_duration) return (float)tick_count / (float)abs(repeat_tick_duration);
    return 0.f;
}

template<Tween_Function func>
float ease_yoyo(float t){
    return func(abs(2.f * (t - 0.5f)));
}
