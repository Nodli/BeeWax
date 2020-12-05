template<typename T>
quaternion::quat<T>& quaternion::quat<T>::operator*=(const quaternion::quat<T>& rhs){
    quaternion::quat<T> lhs = *this;

    // NOTE(hugo): [lhs.s * rhs.s - dot(lhs, rhs), lhs.s * rhs + rhs.s * lhs + cross(lhs, rhs)]
    // https://lucidar.me/fr/quaternions/quaternion-product/
    s = lhs.s * rhs.s - lhs.i * rhs.i - lhs.j * rhs.j - lhs.k * rhs.k;
    i = lhs.s * rhs.i + lhs.i * rhs.s + lhs.j * rhs.k - lhs.k * rhs.j;
    j = lhs.s * rhs.j - lhs.i * rhs.k + lhs.j * rhs.s + lhs.k * rhs.i;
    k = lhs.s * rhs.k + lhs.i * rhs.j - lhs.j * rhs.i + lhs.k * rhs.s;

    return *this;
}

template<typename T>
quaternion::quat<T> operator*(const quaternion::quat<T>& lhs, const quaternion::quat<T>& rhs){
    return quaternion::quat<T>(lhs) *= rhs;
}

template<typename T>
quaternion::quat<T> normalized(const quaternion::quat<T>& quat){
    const T div = sqrt(quat.s * quat.s + quat.i * quat.i + quat.j * quat.j + quat.k * quat.k);
    return {quat.s / div, quat.i / div, quat.j / div, quat.k / div};
}

template<typename T>
quaternion::quat<T> conjugate(const quaternion::quat<T>& quat){
    return {quat.s, - quat.i, - quat.j, - quat.k};
}

template<typename T>
quaternion::quat<T> inverse(const quaternion::quat<T>& quat){
    const T div = quat.s * quat.s + quat.i * quat.i + quat.j * quat.j + quat.k * quat.k;
    assert(div != (T)0u);
    return {quat.s, - quat.i / div, - quat.j / div, - quat.k / div};
}

template<typename T>
mat::mat3<T> to_mat3(const quaternion::quat<T>& quat){
    // NOTE(hugo): https://lucidar.me/fr/quaternions/quaternion-to-rotation-matrix/
    return {1 - 2 * quat.j * quat.j - 2 * quat.k * quat.k, 2 * quat.i * quat.j + 2 * quat.k * quat.s, 2 * quat.i * quat.k - 2 * quat.j * quat.s,
            2 * quat.i * quat.j - 2 * quat.k * quat.s, 1 - 2 * quat.i * quat.i - 2 * quat.k * quat.k, 2 * quat.j * quat.k + 2 * quat.i * quat.s,
            2 * quat.i * quat.k + 2 * quat.j * quat.s, 2 * quat.j * quat.k - 2 * quat.i * quat.s, 1 - 2 * quat.i * quat.i - 2 * quat.j * quat.j};
}

template<typename T>
mat::mat4<T> to_mat4(const quaternion::quat<T>& quat){
    // NOTE(hugo): https://lucidar.me/fr/quaternions/quaternion-to-rotation-matrix/
    return {1 - 2 * quat.j * quat.j - 2 * quat.k * quat.k, 2 * quat.i * quat.j + 2 * quat.k * quat.s, 2 * quat.i * quat.k - 2 * quat.j * quat.s, 0.f,
        2 * quat.i * quat.j - 2 * quat.k * quat.s, 1 - 2 * quat.i * quat.i - 2 * quat.k * quat.k, 2 * quat.j * quat.k + 2 * quat.i * quat.s, 0.f,
        2 * quat.i * quat.k + 2 * quat.j * quat.s, 2 * quat.j * quat.k - 2 * quat.i * quat.s, 1 - 2 * quat.i * quat.i - 2 * quat.j * quat.j, 0.f,
        0.f, 0.f, 0.f, 1.f};
}

namespace BEEWAX_INTERNAL{
    // NOTE(hugo): mij is the ith row and the jth column
    template<typename T>
    inline quaternion::quat<T> quaternion_from_mat3x3(
            const T m00, const T m10, const T m20,
            const T m01, const T m11, const T m21,
            const T m02, const T m12, const T m22
            ){

        quat output;
        T interm;

        if (m22 < (T)(0)){
            if (m00 > m11){
                interm = (T)(1) + m00 - m11 - m22;
                output = {interm, m01 + m10, m20 + m02, m12 - m21};
            }else{
                interm = (T)(1) - m00 + m11 - m22;
                output = {m01 + m10, interm, m12 + m21, m20 - m02};
            }
        }else{
            if (m00 < - m11){
                interm = (T)(1) - m00 - m11 + m22;
                output = {m20 + m02, m12 + m21, interm, m01 - m10};
            }else{
                interm = (T)(1) + m00 + m11 + m22;
                output = {m12 - m21, m20 - m02, m01 - m10, interm};
            }
        }

        T multiplicator = (T)(0.5) / sqrt(interm);
        return {output.s * multiplicator, output.i * multiplicator, output.j * multiplicator, output.k * multiplicator};
    }
}

// NOTE(hugo): https://d3cw3dd2w32x2b.cloudfront.net/wp-content/uploads/2015/01/matrix-to-quat.pdf
template<typename T>
quaternion::quat<T> from_mat3(const mat::mat3<T>& matrix){
    return BEEWAX_INTERNAL::quaternion_from_mat3x3(
            matrix.data[0],
            matrix.data[1],
            matrix.data[2],
            matrix.data[3],
            matrix.data[4],
            matrix.data[5],
            matrix.data[6],
            matrix.data[7],
            matrix.data[8]
    );
}

template<typename T>
quaternion::quat<T> from_mat4(const mat::mat4<T>& matrix){
    return BEEWAX_INTERNAL::quaternion_from_mat3x3(
            matrix.data[0],
            matrix.data[1],
            matrix.data[2],
            matrix.data[4],
            matrix.data[5],
            matrix.data[6],
            matrix.data[8],
            matrix.data[9],
            matrix.data[10]
    );
}

template<typename T>
quaternion::quat<T> quat_from_axis(const T angle, const vec::vec3<T> axis){
    const T half_angle = angle / (T)2;
    const T temp_sin = sin(half_angle);
    return {cos(half_angle), temp_sin * axis.x, temp_sin * axis.y, temp_sin * axis.z};
}

template<typename T>
quaternion::quat<T> quat_from_euler(const T pitch, const T yaw, const T roll){
    float half_pitch = 0.5f * pitch;
    float half_yaw = 0.5f * yaw;
    float half_roll = 0.5f * roll;

    float cosX = cos(half_pitch);
    float cosY = cos(half_yaw);
    float cosZ = cos(half_roll);

    float sinX = sin(half_pitch);
    float sinY = sin(half_yaw);
    float sinZ = sin(half_roll);

    return {
        cosX * cosY * cosZ + sinX * sinY * sinZ,
        sinX * cosY * cosZ - cosX * sinY * sinZ,
        cosX * sinY * cosZ + sinX * cosY * sinZ,
        cosX * cosY * sinZ - sinX * sinY * cosZ
    };
}

template<typename T>
[[nodiscard]] vec::vec3<T> rotate(const vec::vec3<T>& v, const quaternion::quat<T>& quat){
    // NOTE(hugo): fast implementation
    // https://blog.molecular-matters.com/2013/05/24/a-faster-quaternion-vector-multiplication/
    vec::vec3<T> imaginary_quat = {quat.i, quat.j, quat.k};
    vec::vec3<T> temp = T(2) * cross(imaginary_quat, v);
    return v + quat.s * temp + cross(imaginary_quat, temp);

    // NOTE(hugo): usual expression
    //quaternion::quat<T> vquat = {0, v.x, v.y, v.z};
    //return quat * vquat * conjugate(quat);

    // NOTE(hugo): full expression
    //return {
    //    v.x * (q.s * q.s + q.i * q.i - q.j * q.j - q.k * q.k)
    //    + v.y * (2 * q.i * q.j - 2 * q.s * q.k)
    //    + v.z * (2 * q.i * q.k + 2 * q.s * q.j),
    //    v.x * (2 * q.s * q.k + 2 * q.i * q.j)
    //    + v.y * (q.s * q.s - q.i * q.i + q.j * q.j - q.k * q.k)
    //    + v.z * (- 2 * q.s * q.i + 2 * q.j * q.k),
    //    v.x * (- 2 * q.s * q.j + 2 * q.i * q.k)
    //    + v.y * (2 * q.s * q.i + 2 * q.j * q.k)
    //    + v.z * (q.s * q.s - q.i * q.i - q.j * q.j + q.k * q.k)
    //};
}

template<typename T>
quaternion::quat<T> slerp(const quaternion::quat<T>& qA, const quaternion::quat<T>& qB, float t){
    float quat_dot = qA.s * qB.s + qA.i * qB.i + qA.j * qB.j + qA.k * qB.k;
    float qA_sign = 1.f;

    // NOTE(hugo): quat_dot needs to be positive to have the shortest rotation path
    if(quat_dot < 0.f){
        qA_sign = -1.f;
        quat_dot = - quat_dot;
    }

    // NOTE(hugo): linear interpolation is the limit of spherical interpolation when the arc angle is small
    constexpr float linear_interpolation_threshold = 0.999f;
    if(quat_dot > linear_interpolation_threshold){
        float t_opp = 1.f - t;
        return {
            qA_sign * qA.s * t_opp + qB.s * t,
            qA_sign * qA.i * t_opp + qB.i * t,
            qA_sign * qA.j * t_opp + qB.j * t,
            qA_sign * qA.k * t_opp + qB.k * t
        };
    }

    float arc_angle = acos(quat_dot);
    float subarc_angle = t * arc_angle;
    float sin_arc_angle = sin(arc_angle);
    float sin_subarc_angle = sin(arc_angle);
    float ratio = sin_subarc_angle / sin_arc_angle;

    float qA_factor = cos(subarc_angle) - quat_dot * ratio;
    float qB_factor = ratio;

    return {
        qA_factor * qA.s + qB_factor * qB.s,
        qA_factor * qA.i + qB_factor * qB.i,
        qA_factor * qA.j + qB_factor * qB.j,
        qA_factor * qA.k + qB_factor * qB.k
    };
}
