#ifndef H_MATH
#define H_MATH

// ---- bithacks
// NOTE(hugo): type-specific optimizations with bithacks
// http://www.graphics.stanford.edu/~seander/bithacks.html

constexpr bool is_pow2(u32 number);
constexpr bool is_pow2(u64 number);

// REF: https://stackoverflow.com/questions/466204/rounding-up-to-next-power-of-2
constexpr u32 next_pow2(u32 number);

constexpr u32 get_rightmost_set_bit(u32 number);

// ---- utilitary functions

template<typename T, size_t size>
constexpr size_t carray_size(T(&)[size]);

template<typename T>
constexpr void swap(T& A, T& B);

// ---- math values
// NOTE(hugo): specified with 22 decimals which is enough for the 20 decimal precision of long double

template<typename T>
constexpr T ZERO = T(0.l);

template<typename T>
constexpr T PI = T(3.14159265358979323846l);
template<typename T>
constexpr T TWO_PI = T(2) * PI<T>;
template<typename T>
constexpr T HALF_PI = PI<T> / T(2);
template<typename T>
constexpr T GOLDEN_RATIO = T(1.6180339887498948482045l);
template<typename T>
constexpr T SQRT2 = T(1.4142135623730950488017l);
template<typename T>
constexpr T HALF_SQRT2 = T(0.7071067811865475244008l);
template<typename T>
constexpr T SQRT2_DECIMAL = T(0.4142135623730950488017l);

// ---- math functions

template<typename T>
constexpr T sign(const T x);

template<typename T>
constexpr T signbit(const T x);

template<typename T>
constexpr T abs(const T x);

template<typename T>
constexpr T min(const T A, const T B);

template<typename T>
constexpr T max(const T A, const T B);

template<typename T>
constexpr T clamp(const T x, const T min, const T max);

template<typename T>
constexpr T fast_floor(const T x);

template<typename T>
constexpr T divide_ceil(const T dividend, const T divisor);

template<typename T>
constexpr T divide_round(const T dividend, const T divisor);

template<typename T>
constexpr T euclidian_division(const T dividend, const T divisor, T& quotient, T& remainder);

template<typename T>
constexpr T mix(const T begin, const T end, const T interpolator);

template<typename T>
constexpr T to_radians(const T value_degree);

template<typename T>
constexpr T to_degree(const T value_radians);

template<typename T>
constexpr T normalize_to_range(const T x, const T range_min, const T range_max);

// REF(hugo): https://en.wikipedia.org/wiki/Newton%27s_method#Square_root
constexpr long double constexpr_sqrt(const long double x);

constexpr u32 time_to_frames(float duration_in_seconds, u32 frames_per_second);

#include "constexpr.inl"

#endif
