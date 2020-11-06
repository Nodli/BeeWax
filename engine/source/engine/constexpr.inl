// ---- bithacks

constexpr bool is_pow2(u32 number){
    return (number != 0u) && ((number & (number - 1u)) == 0u);
}

constexpr bool is_pow2(u64 number){
    return (number != 0u) && ((number & (number - 1u)) == 0u);
}

constexpr u32 next_pow2(u32 number){
    assert(number != 0u);
    //number = number + (number == 0u);

    --number;
    number |= number >> 1u;
    number |= number >> 2u;
    number |= number >> 4u;
    number |= number >> 8u;
    number |= number >> 16u;
    ++number;

    return number;
}

constexpr u32 get_rightmost_set_bit(u32 number){
    return number & (~number + 1u);
}

// ---- useful functions

template<typename T, u32 size>
constexpr u32 carray_size(T(&)[size]){
    return size;
}

template<typename T>
constexpr void swap(T& A, T& B){

    // NOTE(hugo): avoids calling ~T()
    uchar temp_memory[sizeof(T)];
    T* temp = (T*)temp_memory;

    memcpy((void*)temp, (void*)&A, sizeof(T));
    memcpy((void*)&A, (void*)&B, sizeof(T));
    memcpy((void*)&B, (void*)temp, sizeof(T));
}

// ---- math functions

template<typename T>
constexpr T sign(const T x){
    return (T)((T(0) < x) - (x < T(0)));
}

template<typename T>
constexpr T signbit(const T x){
    return x >> (sizeof(x) - 1);
}

template<typename T>
constexpr T abs(const T x){
    return (x < 0) ? - x : x;
}

template<typename T>
constexpr T min(const T A, const T B){
    return (A < B) ? A : B;
}

template<typename T>
constexpr T max(const T A, const T B){
    return (A > B) ? A : B;
}

template<typename T>
constexpr T clamp(const T x, const T min, const T max){
    return (x > max) ? max : ((x < min) ? min : x);
};

template<typename T, typename U>
constexpr T fast_floor(const U x){
    return (x < U(0)) ? T(x - U(1)) : T(x);
};

template<typename T>
constexpr T divide_ceil(const T dividend, const T divisor){
    T intermediate = dividend + sign(dividend) * (abs(divisor) - T(1));
    return intermediate / divisor;
};

template<typename T>
constexpr T divide_round(const T dividend, const T divisor){
    T intermediate = dividend + sign(dividend) * divisor / 2;
    return intermediate / divisor;
};

template<typename T>
constexpr T mix(const T begin, const T end, const T interpolator){
    return (T(1) - interpolator) * begin + interpolator * end;
}

template<typename T>
constexpr T to_radians(const T value_degree){
    return value_degree / T(180) * PI<T>;
}

template<typename T>
constexpr T to_degree(const T value_radians){
    return value_radians / PI<T> * T(180);
}

template<typename T>
constexpr T range_normalize(const T x, const T range_min, const T range_max){
    return clamp((x - range_min) / (range_max - range_min), T(0), T(1));
}
