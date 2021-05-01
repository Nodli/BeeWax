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
    value = ivalue;
    base = ivalue;
    range = {};
    tick_count = 1u;
    repeat_tick_duration = 1u;
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

template<Tween_Function func>
float ease_yoyo(float t){
    return func(abs(2.f * (t - 0.5f)));
}
