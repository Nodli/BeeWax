#ifndef H_QUAT
#define H_QUAT

// NOTE(hugo): no rotation means a scalar unit quaternion {1.f, 0.f, 0.f, 0.f} ie not default initialization
// NOTE(hugo): only unit / normalized quaternion can represent rotations
// NOTE(hugo): {s, i, j, k} and {-s, -i, -j, -k} represent the same rotation

namespace quaternion{
    template<typename T>
    struct quat;
}

typedef quaternion::quat<float> quat;
typedef quaternion::quat<double> dquat;

template<typename T>
struct quaternion::quat{

    quat<T>& operator*=(const quat<T>& rhs);

    union{
        T data[4] = {1, 0, 0, 0};
        struct{
            T s;
            T i;
            T j;
            T k;
        };
    };
};

template<typename T>
quaternion::quat<T> operator*(const quaternion::quat<T>& lhs, const quaternion::quat<T>& rhs);
template<typename T>
[[nodiscard]] quaternion::quat<T> normalized(const quaternion::quat<T>& quat);
template<typename T>
[[nodiscard]] quaternion::quat<T> conjugate(const quaternion::quat<T>& quat);
template<typename T>
[[nodiscard]] quaternion::quat<T> inverse(const quaternion::quat<T>& quat);

template<typename T>
mat::mat3<T> to_mat3(const quaternion::quat<T>& quat);
template<typename T>
mat::mat4<T> to_mat4(const quaternion::quat<T>& quat);

template<typename T>
quaternion::quat<T> from_mat3(const mat::mat3<T>& matrix);
template<typename T>
quaternion::quat<T> from_mat4(const mat::mat4<T>& matrix);

template<typename T>
quaternion::quat<T> quat_from_axis(const T angle, const vec::vec3<T> axis);

// NOTE(hugo): angles in radian such as
// * angleX = pitch     (nose up / down)
// * angleY = yaw       (nose left / right);
// * angleZ = roll      (nose roll left / nose roll right)
template<typename T>
quaternion::quat<T> quat_from_euler(const T pitch, const T yaw, const T roll);

template<typename T>
[[nodiscard]] vec::vec3<T> rotate(const vec::vec3<T>& v, const quaternion::quat<T>& normalized_quat);

// NOTE(hugo): qA and qB must be unit quaternions
template<typename T>
quaternion::quat<T> slerp(const quaternion::quat<T>& qA, const quaternion::quat<T>& qB, float t);

#include "quat.inl"

#endif
