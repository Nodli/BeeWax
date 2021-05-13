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

constexpr float PI = 3.14159265358979323846L;
constexpr float GOLDEN_RATIO = 1.6180339887498948482045L;
constexpr float GOLDEN_RATIO_FRACT = 0.6180339887498948482045L;

// ---- constexpr template type indexing

template<typename ... Types>
struct Type_Indexer{
    template<typename T>
    constexpr size_t type_index();

    constexpr size_t type_count();
};

// ---- bitset

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

template<typename T>
bool is_pow2(T number);

// NOTE(hugo):
// round_up_pow2(0u) = 0u
// round_up_pow2(/already_power_of_two/) = /already_power_of_two/
// round_up_multiple(0u, /multiple/) = 0u;
// round_up_multiple(/already_multiple_of_m/, /m/) = /already_multiple_of_m/;
u32 round_up_pow2(u32 number);
u32 round_up_multiple(u32 number, u32 multiple);
size_t round_up_multiple(size_t number, size_t multiple);

u32 get_rightmost_set_bit(u32 number);

// ---- normalized integer

u32 float_to_unorm32(float f);
float unorm32_to_float(u32 u);

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

float floor(const float x);
float ceil(const float x);
s32 floor_s32(const float x);
s32 ceil_s32(const float x);
float pow(const float x, const float pow);

float mix(const float min, const float max, const float t);
float normalize(const float x, const float range_min, const float range_max);

float to_radians(const float degree);
float to_degree(const float radian);

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

#include "utils.inl"

#endif
