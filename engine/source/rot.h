#ifndef H_ROT
#define H_ROT

namespace rot{
    template<typename T>
    struct rot3;
}

typedef rot::rot3<float> rot3;
typedef rot::rot3<double> drot3;

template<typename T>
struct rot::rot3{

    rot3<T>& operator*=(const rot3<T>& rot);

    union{
        T data[4];
        struct{
            T s;
            T xy;
            T zx;
            T yz;
        };
    };
};

template<typename T>
rot::rot3<T> operator*(const rot::rot3<T>& lhs, const rot::rot3<T>& rhs);
template<typename T>
[[nodiscard]] rot::rot3<T> normalized(const rot::rot3<T>& rot);

template<typename T>
rot::rot3<T> make_rot(const vec::vec3<T> from, const vec::vec3<T> to);
template<typename T>
rot::rot3<T> make_rot(const T angle, const vec::vec3<T> bivector);
template<typename T>
[[nodiscard]] vec::vec3<T> rotated(const vec::vec3<T>& v, const rot::rot3<T>& r);

#include "rot.inl"

#endif
