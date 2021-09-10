#ifndef H_MAT
#define H_MAT

// NOTE(hugo): mat members
// NOTE(hugo): OpenGL uses column-major matrices ie faster on the y axis

namespace mat{
    template<typename T>
    struct mat2;
    template<typename T>
    struct mat3;
    template<typename T>
    struct mat4;
}

typedef mat::mat2<int> imat2;
typedef mat::mat2<unsigned int> uimat2;
typedef mat::mat2<float> mat2;
typedef mat::mat2<double> dmat2;

typedef mat::mat3<int> imat3;
typedef mat::mat3<unsigned int> uimat3;
typedef mat::mat3<float> mat3;
typedef mat::mat3<double> dmat3;

typedef mat::mat4<int> imat4;
typedef mat::mat4<unsigned int> uimat4;
typedef mat::mat4<float> mat4;
typedef mat::mat4<double> dmat4;

// ---- constexpr row-major constructor ---- //

// NOTE(hugo): matrices use OpenGL column-major storage
//             these constructors take row-major values to create matrices

template<typename T>
constexpr mat::mat2<T> mat2_rm(
        T xx, T yx,
        T xy, T yy);
template<typename T>
constexpr mat::mat3<T> mat3_rm(
        T xx, T yx, T zx,
        T xy, T yy, T zy,
        T xz, T yz, T zz);
template<typename T>
constexpr mat::mat4<T> mat4_rm(
        T xx, T yx, T zx, T tx,
        T xy, T yy, T zy, T ty,
        T xz, T yz, T zz, T tz,
        T xt, T yt, T zt, T tt);

// ---- mat2 ---- //
template<typename T>
struct mat::mat2{

    mat2<T>& operator+=(const mat2<T>& mat);
    mat2<T>& operator+=(const T& v);
    mat2<T>& operator-=(const mat2<T>& mat);
    mat2<T>& operator-=(const T& v);
    mat2<T>& operator*=(const mat2<T>& mat);
    mat2<T>& operator*=(const T& v);
    mat2<T>& operator/=(const T& v);

    mat2<T> operator+() const;
    mat2<T> operator-() const;

    bool operator==(const mat2<T>& mat) const;

    // ---- data

    T data[4];
};

template<typename T>
mat::mat2<T> operator+(const mat::mat2<T>& lhs, const mat::mat2<T>& rhs);
template<typename T>
mat::mat2<T> operator-(const mat::mat2<T>& lhs, const mat::mat2<T>& rhs);
template<typename T>
mat::mat2<T> operator*(const mat::mat2<T>& lhs, const mat::mat2<T>& rhs);

template<typename T>
mat::mat2<T> operator+(const mat::mat2<T>& lhs, const T& rhs);
template<typename T>
mat::mat2<T> operator-(const mat::mat2<T>& lhs, const T& rhs);
template<typename T>
mat::mat2<T> operator*(const mat::mat2<T>& lhs, const T& rhs);
template<typename T>
mat::mat2<T> operator/(const mat::mat2<T>& lhs, const T& rhs);

template<typename T>
mat::mat2<T> operator+(const T& lhs, const mat::mat2<T>& rhs);
template<typename T>
mat::mat2<T> operator-(const T& lhs, const mat::mat2<T>& rhs);
template<typename T>
mat::mat2<T> operator*(const T& lhs, const mat::mat2<T>& rhs);
template<typename T>
mat::mat2<T> operator/(const T& lhs, const mat::mat2<T>& rhs);

template<typename T>
[[nodiscard]] T determinant(const mat::mat2<T>& mat);
template<typename T>
[[nodiscard]] mat::mat2<T> transpose(const mat::mat2<T>& mat);
template<typename T>
[[nodiscard]] mat::mat2<T> inverse(const mat::mat2<T>& mat);

template<typename T>
vec::vec2<T> operator*(const mat::mat2<T>& mat, const vec::vec2<T>& vec);

// ---- mat3 ---- //
template<typename T>
struct mat::mat3{

    mat3<T>& operator+=(const mat3<T>& mat);
    mat3<T>& operator+=(const T& v);
    mat3<T>& operator-=(const mat3<T>& mat);
    mat3<T>& operator-=(const T& v);
    mat3<T>& operator*=(const mat3<T>& mat);
    mat3<T>& operator*=(const T& v);
    mat3<T>& operator/=(const T& v);

    mat3<T> operator+() const;
    mat3<T> operator-() const;

    bool operator==(const mat3<T>& mat) const;

    // ---- data

    T data[9];
};

template<typename T>
mat::mat3<T> operator+(const mat::mat3<T>& lhs, const mat::mat3<T>& rhs);
template<typename T>
mat::mat3<T> operator-(const mat::mat3<T>& lhs, const mat::mat3<T>& rhs);
template<typename T>
mat::mat3<T> operator*(const mat::mat3<T>& lhs, const mat::mat3<T>& rhs);

template<typename T>
mat::mat3<T> operator+(const mat::mat3<T>& lhs, const T& rhs);
template<typename T>
mat::mat3<T> operator-(const mat::mat3<T>& lhs, const T& rhs);
template<typename T>
mat::mat3<T> operator*(const mat::mat3<T>& lhs, const T& rhs);
template<typename T>
mat::mat3<T> operator/(const mat::mat3<T>& lhs, const T& rhs);

template<typename T>
mat::mat3<T> operator+(const T& lhs, const mat::mat3<T>& rhs);
template<typename T>
mat::mat3<T> operator-(const T& lhs, const mat::mat3<T>& rhs);
template<typename T>
mat::mat3<T> operator*(const T& lhs, const mat::mat3<T>& rhs);
template<typename T>
mat::mat3<T> operator/(const T& lhs, const mat::mat3<T>& rhs);

template<typename T>
[[nodiscard]] T determinant(const mat::mat3<T>& mat);
template<typename T>
[[nodiscard]] mat::mat3<T> transpose(const mat::mat3<T>& mat);
template<typename T>
[[nodiscard]] mat::mat3<T> inverse(const mat::mat3<T>& mat);

template<typename T>
vec::vec3<T> operator*(const mat::mat3<T>& mat, const vec::vec3<T>& vec);

// ---- mat4 ---- //
template<typename T>
struct mat::mat4{

    mat4<T>& operator+=(const mat4<T>& mat);
    mat4<T>& operator+=(const T& v);
    mat4<T>& operator-=(const mat4<T>& mat);
    mat4<T>& operator-=(const T& v);
    mat4<T>& operator*=(const mat4<T>& mat);
    mat4<T>& operator*=(const T& v);
    mat4<T>& operator/=(const T& v);

    mat4<T> operator+() const;
    mat4<T> operator-() const;

    bool operator==(const mat4<T>& mat) const;

    // ---- data

    T data[16];
};

template<typename T>
mat::mat4<T> operator+(const mat::mat4<T>& lhs, const mat::mat4<T>& rhs);
template<typename T>
mat::mat4<T> operator-(const mat::mat4<T>& lhs, const mat::mat4<T>& rhs);
template<typename T>
mat::mat4<T> operator*(const mat::mat4<T>& lhs, const mat::mat4<T>& rhs);

template<typename T>
mat::mat4<T> operator+(const mat::mat4<T>& lhs, const T& rhs);
template<typename T>
mat::mat4<T> operator-(const mat::mat4<T>& lhs, const T& rhs);
template<typename T>
mat::mat4<T> operator*(const mat::mat4<T>& lhs, const T& rhs);
template<typename T>
mat::mat4<T> operator/(const mat::mat4<T>& lhs, const T& rhs);

template<typename T>
mat::mat4<T> operator+(const T& lhs, const mat::mat4<T>& rhs);
template<typename T>
mat::mat4<T> operator-(const T& lhs, const mat::mat4<T>& rhs);
template<typename T>
mat::mat4<T> operator*(const T& lhs, const mat::mat4<T>& rhs);
template<typename T>
mat::mat4<T> operator/(const T& lhs, const mat::mat4<T>& rhs);

template<typename T>
[[nodiscard]] T determinant(const mat::mat4<T>& mat);
template<typename T>
[[nodiscard]] mat::mat4<T> transpose(const mat::mat4<T>& mat);
template<typename T>
[[nodiscard]] mat::mat4<T> inverse(const mat::mat4<T>& mat);

template<typename T>
vec::vec4<T> operator*(const mat::mat4<T>& mat, const vec::vec4<T>& vec);

// ---- matrix & coordinate basis

template<typename T>
mat::mat3<T> mat3_from_ortho_basis_3D(const vec::vec3<T> right, const vec::vec3<T> up, const vec::vec3<T> forward);
template<typename T>
mat::mat4<T> mat4_from_ortho_basis_3D(const vec::vec3<T> right, const vec::vec3<T> up, const vec::vec3<T> forward);

// NOTE(hugo): angles in radian such as
// * angleX = pitch     (nose up / down)
// * angleY = yaw       (nose left / right);
// * angleZ = roll      (nose roll left / nose roll right)
template<typename T>
mat::mat4<T> mat4_from_euler(const T pitch, const T yaw, const T roll);

template<typename T>
vec::vec3<T> mat4_to_euler(const mat::mat4<T>& mat);

// ---- mat2 / mat3 / mat4 for std140---- //

struct mat2_std140{
    float data[8];
};
struct mat3_std140{
    float data[12];
};
struct mat4_std140 {
    float data[16];
};

inline mat2_std140 to_std140(const mat2& mat);
inline mat3_std140 to_std140(const mat3& mat);
inline mat4_std140 to_std140(const mat4& mat);

// ---- identity matrix

// NOTE(hugo): identity_matrix is undefined without specialization
template<typename T>
constexpr T identity_matrix;

template<typename T>
constexpr mat::mat2<T> identity_matrix<mat::mat2<T>> = {
    (T)1, (T)0,
    (T)0, (T)1
};
template<typename T>
constexpr mat::mat3<T> identity_matrix<mat::mat3<T>> = {
    (T)1, (T)0, (T)0,
    (T)0, (T)1, (T)0,
    (T)0, (T)0, (T)1
};
template<typename T>
constexpr mat::mat4<T> identity_matrix<mat::mat4<T>> = {
    (T)1, (T)0, (T)0, (T)0,
    (T)0, (T)1, (T)0, (T)0,
    (T)0, (T)0, (T)1, (T)0,
    (T)0, (T)0, (T)0, (T)1
};

#include "mat.inl"

#endif
