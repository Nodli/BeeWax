#ifndef H_UTILS
#define H_UTILS

// ---- constexpr

template<typename T, size_t size>
constexpr size_t carray_size(T(&)[size]);

template<typename T>
constexpr void swap(T& A, T& B);

// REF(hugo): https://en.wikipedia.org/wiki/Newton%27s_method#Square_root
constexpr float constexpr_sqrt(const float x);

template<typename T>
constexpr T sign(const T x);
template<typename T>
constexpr T abs(const T x);
template<typename T>
constexpr T min(const T A, const T B);
template<typename T>
constexpr T max(const T A, const T B);
template<typename T>
constexpr T min_max(const T x, const T min_value, const T max_value);

// ---- constexpr constants

const float PI = 3.14159265358979323846L;
const float GOLDEN_RATIO = 1.6180339887498948482045L;

// ---- constexpr template type indexing

template<typename ... Types>
struct Type_Indexer{
    template<typename T>
    constexpr size_t type_index();
};

// ---- bitset

template<typename T>
constexpr size_t bitset_capacity(u32 bit_capacity);

template<typename T>
void set_bit(T& bitset, u32 bit_index);
template<typename T>
void unset_bit(T& bitset, u32 bit_index);
template<typename T>
void toggle_bit(T& bitset, u32 bit_index);
template<typename T>
T extract_bit(T& bitset, u32 bit_index);

// ---- bithacks

// REF(hugo):
// http://www.graphics.stanford.edu/~seander/bithacks.html
// https://stackoverflow.com/questions/466204/rounding-up-to-next-power-of-2

bool is_pow2(u32 number);
bool is_pow2(u64 number);
bool is_pow2(size_t number);

// NOTE(hugo):
// round_up_pow2(0u) = 0u
// round_up_multiple(0u, /multiple/) = 0u;
u32 round_up_pow2(u32 number);
u32 round_up_multiple(u32 number, u32 multiple);

u32 get_rightmost_set_bit(u32 number);

// ---- alignment
// NOTE(hugo): align must be a power of two in [1, 64]
// REF(hugo):
// https://github.com/KabukiStarship/KabukiToolkit/wiki/Fastest-Method-to-Align-Pointers#3-kabuki-toolkit-memory-alignment-algorithm
// https://stackoverflow.com/questions/4840410/how-to-align-a-pointer-in-c

size_t align_offset_next(uintptr_t ptr_align, size_t align);
size_t align_offset_prev(uintptr_t ptr_align, size_t align);

void* align_next(void* ptr, size_t align);
void* align_prev(void* ptr, size_t align);

bool is_aligned(void* ptr, size_t align);

// ---- math

float fast_floor(const float x);
float mix(const float min, const float max, const float t);
float to_radians(const float degree);
float to_degree(const float radian);
float normalize(const float x, const float range_min, const float range_max);

float cos(float t);
float sin(float t);
float tan(float t);
float sqrt(float t);
float acos(float t);
float asin(float t);
float atan(float t);
float atan2(float y, float x);

// ---- grid indexing

// NOTE(hugo): converts 2D coordinates into an index in a 1D array
//             row major    i2D(x, y, width)
//             column major i2D(y, x, height)
u32 index2D(u32 major_axis_coord, u32 minor_axis_coord, u32 major_axis_size);

// NOTE(hugo): converts a 1D array index into 2D coordinates
//             row major    c2D(index, width, x, y)
//             column major c2D(index, height, y, x)
void coord2D(u32 index, u32 major_axis_size, u32& major_axis_coord, u32& minor_axis_coord);

// NOTE(hugo): out_neighbor must have a size of 4
//             row major    neigh4_i2D(index, width, width * height, data);
//             column major neigh4_i2D(index, height, width * height, data);
u32 neigh4_index2D(u32 index, u32 major_axis_size, u32 max_index, u32* out_neighbor);

// NOTE(hugo): out_neighbor must have a size of 2 * 4 = 8
//             row major    neigh4_c2D(x, y, width, height, data);
//             column major neigh4_c2D(y, x, height, width, data);
u32 neigh4_coord2D(u32 major_axis_coord, u32 minor_axis_coord, u32 major_axis_size, u32 minor_axis_size, u32* out_neighbor);

// ---- colormaps

// NOTE(hugo): Polynomials fitted to matplotlib colormaps with value in [0, 1]
// https://www.shadertoy.com/view/WlfXRN
void viridis(const float value, float& r, float& g, float& b);
void plasma(const float value, float& r, float& g, float& b);
void magma(const float value, float& r, float& g, float& b);
void inferno(const float value, float& r, float& g, float& b);

// ---- type pair

template<typename U, typename V>
struct pair{
    U first;
    V second;
};

template<typename U, typename V>
bool operator==(const pair<U, V>& lhs, const pair<U, V>& rhs);
template<typename U, typename V>
bool operator!=(const pair<U, V>& lhs, const pair<U, V>& rhs);
template<typename U, typename V>
bool operator<(const pair<U, V>& lhs, const pair<U, V>& rhs);
template<typename U, typename V>
bool operator<=(const pair<U, V>& lhs, const pair<U, V>& rhs);
template<typename U, typename V>
bool operator>(const pair<U, V>& lhs, const pair<U, V>& rhs);
template<typename U, typename V>
bool operator>=(const pair<U, V>& lhs, const pair<U, V>& rhs);

// ---- internal

// NOTE(hugo): generic comparison fuctions for qsort that avoid overflow
namespace BEEWAX_INTERNAL{
    template<typename T>
    s32 comparison_increasing_order(const T& A, const T& B);

    template<typename T>
    s32 comparison_decreasing_order(const T& A, const T& B);
}

#include "utils.inl"

#endif
