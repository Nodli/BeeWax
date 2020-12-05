#ifndef H_VEC
#define H_VEC

// NOTE(hugo): vec members are default-initialized ie value-initialized ie zero-initialized

namespace vec{
    template<typename T>
    struct vec2;
    template<typename T>
    struct vec3;
    template<typename T>
    struct vec4;
}

typedef vec::vec2<int> ivec2;
typedef vec::vec2<unsigned int> uivec2;
typedef vec::vec2<float> vec2;
typedef vec::vec2<double> dvec2;

typedef vec::vec3<int> ivec3;
typedef vec::vec3<unsigned int> uivec3;
typedef vec::vec3<float> vec3;
typedef vec::vec3<double> dvec3;

typedef vec::vec4<int> ivec4;
typedef vec::vec4<unsigned int> uivec4;
typedef vec::vec4<float> vec4;
typedef vec::vec4<double> dvec4;

// ---- vec2 ---- //
template<typename T>
struct vec::vec2{

    vec2<T>& operator++();
    vec2<T> operator++(int);
    vec2<T>& operator--();
    vec2<T> operator--(int);

    vec2<T>& operator+=(const vec2<T>& vec);
    vec2<T>& operator+=(const T v);
    vec2<T>& operator-=(const vec2<T>& vec);
    vec2<T>& operator-=(const T v);
    vec2<T>& operator*=(const vec2<T>& vec);
    vec2<T>& operator*=(const T v);
    vec2<T>& operator/=(const vec2<T>& vec);
    vec2<T>& operator/=(const T v);

    vec2<T> operator+() const;
    vec2<T> operator-() const;

    bool operator==(const vec2<T>& vec) const;

    T& operator[](const u32 index);
    T operator[](const u32 index) const;

    union{
        T data[2] = {};
        struct{
            T x;
            T y;
        };
        struct{
            T r;
            T g;
        };
        struct{
            T s;
            T t;
        };
    };
};

template<typename T>
vec::vec2<T> operator+(const vec::vec2<T>& lhs, const vec::vec2<T>& rhs);
template<typename T>
vec::vec2<T> operator-(const vec::vec2<T>& lhs, const vec::vec2<T>& rhs);
template<typename T>
vec::vec2<T> operator*(const vec::vec2<T>& lhs, const vec::vec2<T>& rhs);
template<typename T>
vec::vec2<T> operator/(const vec::vec2<T>& lhs, const vec::vec2<T>& rhs);

template<typename T>
vec::vec2<T> operator+(const vec::vec2<T>& lhs, const T rhs);
template<typename T>
vec::vec2<T> operator-(const vec::vec2<T>& lhs, const T rhs);
template<typename T>
vec::vec2<T> operator*(const vec::vec2<T>& lhs, const T rhs);
template<typename T>
vec::vec2<T> operator/(const vec::vec2<T>& lhs, const T rhs);

template<typename T>
vec::vec2<T> operator+(const T lhs, const vec::vec2<T>& rhs);
template<typename T>
vec::vec2<T> operator-(const T lhs, const vec::vec2<T>& rhs);
template<typename T>
vec::vec2<T> operator*(const T lhs, const vec::vec2<T>& rhs);
template<typename T>
vec::vec2<T> operator/(const T lhs, const vec::vec2<T>& rhs);

template<typename T>
[[nodiscard]] vec::vec2<T> abs(const vec::vec2<T>& vec);

[[nodiscard]] float dot(const vec2& vA, const vec2& vB);
[[nodiscard]] float length(const vec2& vec);
[[nodiscard]] float sqlength(const vec2& vec);
[[nodiscard]] vec2 normalized(const vec2& vec);

// ---- vec3 ---- //
template<typename T>
struct vec::vec3{

    vec3<T>& operator++();
    vec3<T> operator++(int);
    vec3<T>& operator--();
    vec3<T> operator--(int);

    vec3<T>& operator+=(const vec3<T>& vec);
    vec3<T>& operator+=(const T v);
    vec3<T>& operator-=(const vec3<T>& vec);
    vec3<T>& operator-=(const T v);
    vec3<T>& operator*=(const vec3<T>& vec);
    vec3<T>& operator*=(const T v);
    vec3<T>& operator/=(const vec3<T>& vec);
    vec3<T>& operator/=(const T v);

    vec3<T> operator+() const;
    vec3<T> operator-() const;

    bool operator==(const vec3<T>& vec) const;

    T& operator[](const u32 index);
    T operator[](const u32 index) const;

    union{
        T data[3] = {};
        struct{
            T x;
            T y;
            T z;
        };
        struct{
            T r;
            T g;
            T b;
        };
        struct{
            T s;
            T t;
            T p;
        };
    };
};

template<typename T>
vec::vec3<T> operator+(const vec::vec3<T>& lhs, const vec::vec3<T>& rhs);
template<typename T>
vec::vec3<T> operator-(const vec::vec3<T>& lhs, const vec::vec3<T>& rhs);
template<typename T>
vec::vec3<T> operator*(const vec::vec3<T>& lhs, const vec::vec3<T>& rhs);
template<typename T>
vec::vec3<T> operator/(const vec::vec3<T>& lhs, const vec::vec3<T>& rhs);

template<typename T>
vec::vec3<T> operator+(const vec::vec3<T>& lhs, const T rhs);
template<typename T>
vec::vec3<T> operator-(const vec::vec3<T>& lhs, const T rhs);
template<typename T>
vec::vec3<T> operator*(const vec::vec3<T>& lhs, const T rhs);
template<typename T>
vec::vec3<T> operator/(const vec::vec3<T>& lhs, const T rhs);

template<typename T>
vec::vec3<T> operator+(const T lhs, const vec::vec3<T>& rhs);
template<typename T>
vec::vec3<T> operator-(const T lhs, const vec::vec3<T>& rhs);
template<typename T>
vec::vec3<T> operator*(const T lhs, const vec::vec3<T>& rhs);
template<typename T>
vec::vec3<T> operator/(const T lhs, const vec::vec3<T>& rhs);

template<typename T>
[[nodiscard]] vec::vec3<T> abs(const vec::vec3<T>& vec);

[[nodiscard]] float dot(const vec3& vA, const vec3& vB);
[[nodiscard]] vec3 cross(const vec3& vA, const vec3& vB);
// NOTE(hugo): XY, ZX, YZ
[[nodiscard]] vec3 wedge(const vec3& vA, const vec3& vB);
[[nodiscard]] float length(const vec3& vec);
[[nodiscard]] float sqlength(const vec3& vec);
[[nodiscard]] vec3 normalized(const vec3& vec);

// ---- vec4 ---- //
template<typename T>
struct vec::vec4{

    vec4<T>& operator++();
    vec4<T> operator++(int);
    vec4<T>& operator--();
    vec4<T> operator--(int);

    vec4<T>& operator+=(const vec4<T>& vec);
    vec4<T>& operator+=(const T v);
    vec4<T>& operator-=(const vec4<T>& vec);
    vec4<T>& operator-=(const T v);
    vec4<T>& operator*=(const vec4<T>& vec);
    vec4<T>& operator*=(const T v);
    vec4<T>& operator/=(const vec4<T>& vec);
    vec4<T>& operator/=(const T v);

    vec4<T> operator+() const;
    vec4<T> operator-() const;

    bool operator==(const vec4<T>& vec) const;

    T& operator[](const u32 index);
    T operator[](const u32 index) const;

    union{
        T data[4] = {};
        struct{
            T x;
            T y;
            T z;
            T w;
        };
        struct{
            T r;
            T g;
            T b;
            T a;
        };
        struct{
            T s;
            T t;
            T p;
            T q;
        };
    };
};

template<typename T>
vec::vec4<T> operator+(const vec::vec4<T>& lhs, const vec::vec4<T>& rhs);
template<typename T>
vec::vec4<T> operator-(const vec::vec4<T>& lhs, const vec::vec4<T>& rhs);
template<typename T>
vec::vec4<T> operator*(const vec::vec4<T>& lhs, const vec::vec4<T>& rhs);
template<typename T>
vec::vec4<T> operator/(const vec::vec4<T>& lhs, const vec::vec4<T>& rhs);

template<typename T>
vec::vec4<T> operator+(const vec::vec4<T>& lhs, const T rhs);
template<typename T>
vec::vec4<T> operator-(const vec::vec4<T>& lhs, const T rhs);
template<typename T>
vec::vec4<T> operator*(const vec::vec4<T>& lhs, const T rhs);
template<typename T>
vec::vec4<T> operator/(const vec::vec4<T>& lhs, const T rhs);

template<typename T>
vec::vec4<T> operator+(const T lhs, const vec::vec4<T>& rhs);
template<typename T>
vec::vec4<T> operator-(const T lhs, const vec::vec4<T>& rhs);
template<typename T>
vec::vec4<T> operator*(const T lhs, const vec::vec4<T>& rhs);
template<typename T>
vec::vec4<T> operator/(const T lhs, const vec::vec4<T>& rhs);

template<typename T>
[[nodiscard]] vec::vec4<T> abs(const vec::vec4<T>& vec);

[[nodiscard]] float dot(const vec4& vA, const vec4& vB);
[[nodiscard]] float length(const vec4& vec);
[[nodiscard]] float sqlength(const vec4& vec);
[[nodiscard]] vec4 normalized(const vec4& vec);

#include "vec.inl"

static_assert(alignof(vec2) == alignof(float));
static_assert(alignof(vec3) == alignof(float));
static_assert(alignof(vec4) == alignof(float));

#endif
