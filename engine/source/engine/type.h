#ifndef H_TYPE
#define H_TYPE

static_assert(sizeof(char) == 1u);
static_assert(sizeof(short) == 2u);
static_assert(sizeof(int) == 4u);
static_assert(sizeof(long long int) == 8u);

// NOTE(hugo): float (32 bits) is 7 digits precision
// NOTE(hugo): double (64 bits) is 15 digits precision
// NOTE(hugo): long double (80 bits) is 19 digits precision

static_assert(sizeof(float) == sizeof(u32) && sizeof(float) == 4u);
static_assert(sizeof(double) == sizeof(u64) && sizeof(double) == 8u);

namespace BEEWAX_INTERNAL{
    constexpr u32 float_mask_sign = 0x80000000;
    constexpr u32 float_mask_exponent = 0x7F800000;
    constexpr u32 float_mask_mantissa = 0x7FFFFF;
    constexpr u32 float_mask_sign_exponent = float_mask_sign | float_mask_exponent;         // NOTE(hugo): 0xFF800000
    constexpr u32 float_mask_exponent_mantissa = float_mask_exponent | float_mask_mantissa; // NOTE(hugo): 0x7FFFFFFF
    constexpr u32 float_mask_sign_mantissa = float_mask_sign | float_mask_mantissa;         // NOTE(hugo): 0x807FFFFF

    constexpr u64 double_mask_sign = 0x8000000000000000;
    constexpr u64 double_mask_exponent = 0x7FF0000000000000;
    constexpr u64 double_mask_mantissa = 0xFFFFFFFFFFFFF;
    constexpr u64 double_mask_sign_exponent = double_mask_sign | double_mask_exponent;          // NOTE(hugo): 0xFFF0000000000000
    constexpr u64 double_mask_exponent_mantissa = double_mask_exponent | double_mask_mantissa;  // NOTE(hugo): 0x7FFFFFFFFFFFFFFF
    constexpr u64 double_mask_sign_mantissa = double_mask_sign | double_mask_mantissa;          // NOTE(hugo): 0x800FFFFFFFFFFFFF
}

u32 float_mantissa(float f);
u32 float_exponent(float f);
u32 float_sign(float f);

bool is_infinite(float f);
bool is_nan(float f);
float measure_precision(float f);

// NOTE(hugo): ULP stands for Units in the Last Place ie bitwise difference
bool almost_equal(float fA, float fB, float delta_absolute, u32 delta_ULP);

u64 double_mantissa(double d);
u64 double_exponent(double d);
u64 double_sign(double d);

bool is_infinite(double d);
bool is_nan(double d);
double measure_precision(double d);

// NOTE(hugo): ULP stands for Units in the Last Place ie bitwise difference
bool almost_equal(double dA, double dB, double delta_absolute, u64 delta_ULP);

#include "type.inl"

#endif
