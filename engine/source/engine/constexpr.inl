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

template<typename T, size_t size>
constexpr size_t carray_size(T(&)[size]){
    return size;
}

template<typename T>
constexpr void swap(T& A, T& B){
    T temp = A;
    A = B;
    B = temp;
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

template<typename T>
constexpr T fast_floor(const T x){
    return (x < T(0)) ? T(x - T(1)) : T(x);
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
constexpr T mix(const T begin, const T end, const float interpolator){
    return begin + (T)(interpolator * (end - begin));
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
constexpr T normalize_to_range(const T x, const T range_min, const T range_max){
    return clamp((x - range_min) / (range_max - range_min), T(0), T(1));
}

constexpr long double constexpr_sqrt_recursion(const long double x, const long double ycurrent, const long double yprevious){
    if (ycurrent == yprevious){
        return ycurrent;
    }else{
        return constexpr_sqrt_recursion(x, 0.5 * (ycurrent + x / ycurrent), ycurrent);
    }
}

// NOTE(hugo): unnecessary to check against NaN or infinite because this is used at compile time on user provided values
constexpr long double constexpr_sqrt(const long double x){
    if(x >= 0.){
        return constexpr_sqrt_recursion(x, x, 0.);
    }else{
        return 0.;
    }
}

constexpr u32 time_to_frames(float duration_in_seconds, u32 frames_per_second){
    float frame_duration = 1. / (float)frames_per_second;
    return fast_floor<float>(duration_in_seconds / frame_duration);
}

