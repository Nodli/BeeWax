template<typename T>
rot::rot3<T>& rot::rot3<T>::operator*=(const rot::rot3<T>& rhs){
    rot::rot3<T> lhs = *this;

    s = lhs.s * rhs.s
        - lhs.xy * rhs.xy - lhs.zx * rhs.zx - lhs.yz * rhs.yz;
    xy = lhs.s * rhs.xy + lhs.xy * rhs.s
        + lhs.zx * rhs.yz - lhs.yz * rhs.zx;
    zx = lhs.s * rhs.zx + lhs.zx * rhs.s
        + lhs.yz * rhs.xy - lhs.xy * rhs.yz;
    yz = lhs.s * rhs.yz + lhs.yz * rhs.s
        + lhs.xy * rhs.zx - lhs.zx * rhs.xy;

    return *this;
}

template<typename T>
rot::rot3<T> operator*(const rot::rot3<T>& lhs, const rot::rot3<T>& rhs){
    return rot::rot3<T>(lhs) *= rhs;
}

template<typename T>
rot::rot3<T> normalize(const rot::rot3<T>& rot){
    const T div = std::sqrt(rot.s * rot.s + rot.xy * rot.xy + rot.xz * rot.xz + rot.yz * rot.yz);
    return {rot.s / div, rot.xy / div, rot.xz / div, rot.yz / div};
}

template<typename T>
rot::rot3<T> make_rot(const vec::vec3<T> from, const vec::vec3<T> to){
    const vec::vec3<T> rot_wedge = wedge(to, from);
    return {dot(from, to), rot_wedge.x, rot_wedge.y, rot_wedge.z};
}
template<typename T>
rot::rot3<T> make_rot(const T angle, const vec::vec3<T> bivector){
    const T sin_temp = - std::sin(angle / 2);
    return {std::cos(angle / 2), sin_temp * bivector.x, sin_temp * bivector.y, sin_temp * bivector.z};
}
template<typename T>
vec::vec3<T> rotate(const vec::vec3<T>& v, const rot::rot3<T>& r){
    vec::vec3<T> vector_temp = {
        r.s * v.x + r.xy * v.y - r.zx * v.z,
        r.s * v.y + r.yz * v.z - r.xy * v.x,
        r.s * v.z + r.zx * v.x - r.yz * v.y
    };
    T trivector_temp = v.x * r.yz + v.y * r.zx + v.z * r.xy;

    // NOTE(hugo): inverted order (vector * rotor) ie * -1
    //             the rotor is r^(-1) ie * -1
    //             = same sign as before with an additional trivector component
    return
    {
        vector_temp.x * r.s + vector_temp.y * r.xy - vector_temp.z * r.zx + trivector_temp * r.yz,
        vector_temp.y * r.s + vector_temp.z * r.yz - vector_temp.x * r.xy + trivector_temp * r.zx,
        vector_temp.z * r.s + vector_temp.x * r.zx - vector_temp.y * r.yz + trivector_temp * r.xy
    };
}

