// NOTE(hugo): Optimize operator*= wrt. cache cf. "GDC2003 Ericson Memory Optimization"

// ---- constexpr row-major constructor

template<typename T>
constexpr mat::mat2<T> mat2_rm(
        T xx, T yx,
        T xy, T yy){
    return {
        xx, xy,
        yx, yy
    };
}

template<typename T>
constexpr mat::mat3<T> mat3_rm(
        T xx, T yx, T zx,
        T xy, T yy, T zy,
        T xz, T yz, T zz){
    return {
        xx, xy, xz,
        yx, yy, yz,
        zx, zy, zz,
    };
}

template<typename T>
constexpr mat::mat4<T> mat4_rm(
        T xx, T yx, T zx, T tx,
        T xy, T yy, T zy, T ty,
        T xz, T yz, T zz, T tz,
        T xt, T yt, T zt, T tt){
    return {
        xx, xy, xz, xt,
        yx, yy, yz, yt,
        zx, zy, zz, zt,
        tx, ty, tz, tt,
    };
}

// ---- mat::mat2

template<typename T>
mat::mat2<T>& mat::mat2<T>::operator+=(const mat::mat2<T>& mat){
    data[0] += mat.data[0];
    data[1] += mat.data[1];
    data[2] += mat.data[2];
    data[3] += mat.data[3];

    return *this;
}
template<typename T>
mat::mat2<T>& mat::mat2<T>::operator+=(const T& v){
    data[0] += v;
    data[1] += v;
    data[2] += v;
    data[3] += v;

    return *this;
}
template<typename T>
mat::mat2<T>& mat::mat2<T>::operator-=(const mat::mat2<T>& mat){
    data[0] -= mat.data[0];
    data[1] -= mat.data[1];
    data[2] -= mat.data[2];
    data[3] -= mat.data[3];

    return *this;
}
template<typename T>
mat::mat2<T>& mat::mat2<T>::operator-=(const T& v){
    data[0] -= v;
    data[1] -= v;
    data[2] -= v;
    data[3] -= v;

    return *this;
}
template<typename T>
mat::mat2<T>& mat::mat2<T>::operator*=(const mat::mat2<T>& mat){
    T this_copy[4] = {data[0], data[1], data[2], data[3]};

    data[0] = this_copy[0] * mat.data[0] + this_copy[2] * mat.data[1];
    data[1] = this_copy[1] * mat.data[0] + this_copy[3] * mat.data[1];

    data[2] = this_copy[0] * mat.data[2] + this_copy[2] * mat.data[3];
    data[3] = this_copy[1] * mat.data[2] + this_copy[3] * mat.data[3];

    return *this;
}
template<typename T>
mat::mat2<T>& mat::mat2<T>::operator*=(const T& v){
    data[0] *= v;
    data[1] *= v;
    data[2] *= v;
    data[3] *= v;

    return *this;
}
template<typename T>
mat::mat2<T>& mat::mat2<T>::operator/=(const T& v){
    data[0] /= v;
    data[1] /= v;
    data[2] /= v;
    data[3] /= v;

    return *this;
}

template<typename T>
mat::mat2<T> mat::mat2<T>::operator+() const{
    return *this;
}
template<typename T>
mat::mat2<T> mat::mat2<T>::operator-() const{
    mat::mat2<T> copy;
    copy.data[0] = - this->data[0];
    copy.data[1] = - this->data[1];
    copy.data[2] = - this->data[2];
    copy.data[3] = - this->data[3];
    return copy;
}

template<typename T>
bool mat::mat2<T>::operator==(const mat::mat2<T>& mat) const{
    return (data[0] == mat.data[0])
        && (data[1] == mat.data[1])
        && (data[2] == mat.data[2])
        && (data[3] == mat.data[3]);
}

template<typename T>
mat::mat2<T> operator+(const mat::mat2<T>& lhs, const mat::mat2<T>& rhs){
    return mat::mat2<T>(lhs) += rhs;
}
template<typename T>
mat::mat2<T> operator-(const mat::mat2<T>& lhs, const mat::mat2<T>& rhs){
    return mat::mat2<T>(lhs) -= rhs;
}
template<typename T>
mat::mat2<T> operator*(const mat::mat2<T>& lhs, const mat::mat2<T>& rhs){
    return mat::mat2<T>(lhs) *= rhs;
}

template<typename T>
mat::mat2<T> operator+(const mat::mat2<T>& lhs, const T& rhs){
    return mat::mat2<T>(lhs) += rhs;
}
template<typename T>
mat::mat2<T> operator-(const mat::mat2<T>& lhs, const T& rhs){
    return mat::mat2<T>(lhs) -= rhs;
}
template<typename T>
mat::mat2<T> operator*(const mat::mat2<T>& lhs, const T& rhs){
    return mat::mat2<T>(lhs) *= rhs;
}
template<typename T>
mat::mat2<T> operator/(const mat::mat2<T>& lhs, const T& rhs){
    return mat::mat2<T>(lhs) /= rhs;
}

template<typename T>
mat::mat2<T> operator+(const T& lhs, const mat::mat2<T>& rhs){
    return mat::mat2<T>(rhs) += lhs;
}
template<typename T>
mat::mat2<T> operator-(const T& lhs, const mat::mat2<T>& rhs){
    return mat::mat2<T>(rhs) -= lhs;
}
template<typename T>
mat::mat2<T> operator*(const T& lhs, const mat::mat2<T>& rhs){
    return mat::mat2<T>(rhs) *= lhs;
}
template<typename T>
mat::mat2<T> operator/(const T& lhs, const mat::mat2<T>& rhs){
    return mat::mat2<T>(rhs) /= lhs;
}

template<typename T>
T determinant(const mat::mat2<T>& mat){

    // A B
    // C D
    constexpr int A = 0;
    constexpr int B = 2;
    constexpr int C = 1;
    constexpr int D = 3;

    return mat.data[A] * mat.data[D] - mat.data[B] * mat.data[C];
}

template<typename T>
mat::mat2<T> transpose(const mat::mat2<T>& mat){

    // A B
    // C D
    constexpr int A = 0;
    constexpr int B = 2;
    constexpr int C = 1;
    constexpr int D = 3;

    return mat::mat2<T>(mat.data[A], mat.data[C], mat.data[B], mat.data[D]);
}

template<typename T>
mat::mat2<T> inverse(const mat::mat2<T>& mat){
    // NOTE(hugo): https://en.wikipedia.org/wiki/Invertible_matrix#Inversion_of_2_%C3%97_2_matrices

    T det = determinant(mat);
    T factor;
    if(det){
        factor = 1 / det;
    }else{
        return mat::mat2<T>();
    }

    // A B
    // C D
    constexpr int A = 0;
    constexpr int B = 2;
    constexpr int C = 1;
    constexpr int D = 3;

    return mat::mat2<T>(
            mat.data[D] * factor,
            - mat.data[B] * factor,
            - mat.data[C] * factor,
            mat.data[A] * factor);
}

template<typename T>
vec::vec2<T> operator*(const mat::mat2<T>& mat, const vec::vec2<T>& vec){
    return vec::vec2<T>(mat.data[0] * vec.data[0] + mat.data[2] * vec.data[1],
            mat.data[1] * vec.data[0] + mat.data[3] * vec.data[1]);

}

// ---- mat::mat3

template<typename T>
mat::mat3<T>& mat::mat3<T>::operator+=(const mat::mat3<T>& mat){
    data[0] += mat.data[0];
    data[1] += mat.data[1];
    data[2] += mat.data[2];
    data[3] += mat.data[3];
    data[4] += mat.data[4];
    data[5] += mat.data[5];
    data[6] += mat.data[6];
    data[7] += mat.data[7];
    data[8] += mat.data[8];

    return *this;
}
template<typename T>
mat::mat3<T>& mat::mat3<T>::operator+=(const T& v){
    data[0] += v;
    data[1] += v;
    data[2] += v;
    data[3] += v;
    data[4] += v;
    data[5] += v;
    data[6] += v;
    data[7] += v;
    data[8] += v;

    return *this;
}
template<typename T>
mat::mat3<T>& mat::mat3<T>::operator-=(const mat::mat3<T>& mat){
    data[0] -= mat.data[0];
    data[1] -= mat.data[1];
    data[2] -= mat.data[2];
    data[3] -= mat.data[3];
    data[4] -= mat.data[4];
    data[5] -= mat.data[5];
    data[6] -= mat.data[6];
    data[7] -= mat.data[7];
    data[8] -= mat.data[8];

    return *this;
}
template<typename T>
mat::mat3<T>& mat::mat3<T>::operator-=(const T& v){
    data[0] -= v;
    data[1] -= v;
    data[2] -= v;
    data[3] -= v;
    data[4] -= v;
    data[5] -= v;
    data[6] -= v;
    data[7] -= v;
    data[8] -= v;

    return *this;
}
template<typename T>
mat::mat3<T>& mat::mat3<T>::operator*=(const mat::mat3<T>& mat){
    T this_copy[9] = {data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8]};

    data[0] = this_copy[0] * mat.data[0] + this_copy[3] * mat.data[1] + this_copy[6] * mat.data[2];
    data[1] = this_copy[1] * mat.data[0] + this_copy[4] * mat.data[1] + this_copy[7] * mat.data[2];
    data[2] = this_copy[2] * mat.data[0] + this_copy[5] * mat.data[1] + this_copy[8] * mat.data[2];

    data[3] = this_copy[0] * mat.data[3] + this_copy[3] * mat.data[4] + this_copy[6] * mat.data[5];
    data[4] = this_copy[1] * mat.data[3] + this_copy[4] * mat.data[4] + this_copy[7] * mat.data[5];
    data[5] = this_copy[2] * mat.data[3] + this_copy[5] * mat.data[4] + this_copy[8] * mat.data[5];

    data[6] = this_copy[0] * mat.data[6] + this_copy[3] * mat.data[7] + this_copy[6] * mat.data[8];
    data[7] = this_copy[1] * mat.data[6] + this_copy[4] * mat.data[7] + this_copy[7] * mat.data[8];
    data[8] = this_copy[2] * mat.data[6] + this_copy[5] * mat.data[7] + this_copy[8] * mat.data[8];

    return *this;
}
template<typename T>
mat::mat3<T>& mat::mat3<T>::operator*=(const T& v){
    data[0] *= v;
    data[1] *= v;
    data[2] *= v;
    data[3] *= v;
    data[4] *= v;
    data[5] *= v;
    data[6] *= v;
    data[7] *= v;
    data[8] *= v;

    return *this;
}
template<typename T>
mat::mat3<T>& mat::mat3<T>::operator/=(const T& v){
    data[0] /= v;
    data[1] /= v;
    data[2] /= v;
    data[3] /= v;
    data[4] /= v;
    data[5] /= v;
    data[6] /= v;
    data[7] /= v;
    data[8] /= v;

    return *this;
}

template<typename T>
mat::mat3<T> mat::mat3<T>::operator+() const{
    return *this;
}
template<typename T>
mat::mat3<T> mat::mat3<T>::operator-() const{
    mat::mat3<T> copy;
    copy.data[0] = - this->data[0];
    copy.data[1] = - this->data[1];
    copy.data[2] = - this->data[2];
    copy.data[3] = - this->data[3];
    copy.data[4] = - this->data[4];
    copy.data[5] = - this->data[5];
    copy.data[6] = - this->data[6];
    copy.data[7] = - this->data[7];
    copy.data[8] = - this->data[8];
    return copy;
}

template<typename T>
bool mat::mat3<T>::operator==(const mat::mat3<T>& mat) const{
    return (data[0] == mat.data[0])
        && (data[1] == mat.data[1])
        && (data[2] == mat.data[2])
        && (data[3] == mat.data[3])
        && (data[4] == mat.data[4])
        && (data[5] == mat.data[5])
        && (data[6] == mat.data[6])
        && (data[7] == mat.data[7])
        && (data[8] == mat.data[8]);
}

template<typename T>
mat::mat3<T> operator+(const mat::mat3<T>& lhs, const mat::mat3<T>& rhs){
    return mat::mat3<T>(lhs) += rhs;
}
template<typename T>
mat::mat3<T> operator-(const mat::mat3<T>& lhs, const mat::mat3<T>& rhs){
    return mat::mat3<T>(lhs) -= rhs;
}
template<typename T>
mat::mat3<T> operator*(const mat::mat3<T>& lhs, const mat::mat3<T>& rhs){
    return mat::mat3<T>(lhs) *= rhs;
}

template<typename T>
mat::mat3<T> operator+(const mat::mat3<T>& lhs, const T& rhs){
    return mat::mat3<T>(lhs) += rhs;
}
template<typename T>
mat::mat3<T> operator-(const mat::mat3<T>& lhs, const T& rhs){
    return mat::mat3<T>(lhs) -= rhs;
}
template<typename T>
mat::mat3<T> operator*(const mat::mat3<T>& lhs, const T& rhs){
    return mat::mat3<T>(lhs) *= rhs;
}
template<typename T>
mat::mat3<T> operator/(const mat::mat3<T>& lhs, const T& rhs){
    return mat::mat3<T>(lhs) /= rhs;
}

template<typename T>
mat::mat3<T> operator+(const T& lhs, const mat::mat3<T>& rhs){
    return mat::mat3<T>(rhs) += lhs;
}
template<typename T>
mat::mat3<T> operator-(const T& lhs, const mat::mat3<T>& rhs){
    return mat::mat3<T>(rhs) -= lhs;
}
template<typename T>
mat::mat3<T> operator*(const T& lhs, const mat::mat3<T>& rhs){
    return mat::mat3<T>(rhs) *= lhs;
}
template<typename T>
mat::mat3<T> operator/(const T& lhs, const mat::mat3<T>& rhs){
    return mat::mat3<T>(rhs) /= lhs;
}

template<typename T>
T determinant(const mat::mat3<T>& mat){
    // NOTE(hugo): Sarrus Rule
    // NOTE(hugo): https://en.wikipedia.org/wiki/Invertible_matrix#Inversion_of_3_%C3%97_3_matrices

    // A B C
    // D E F
    // G H I
    constexpr int A = 0;
    constexpr int B = 3;
    constexpr int C = 6;
    constexpr int D = 1;
    constexpr int E = 4;
    constexpr int F = 7;
    constexpr int G = 2;
    constexpr int H = 5;
    constexpr int I = 8;

    return mat.data[A] * mat.data[E] * mat.data[I] + mat.data[B] * mat.data[F] * mat.data[G] + mat.data[C] * mat.data[D] * mat.data[H]
        - mat.data[C] * mat.data[E] * mat.data[G] - mat.data[F] * mat.data[H] * mat.data[A] - mat.data[I] * mat.data[B] * mat.data[D];
}

template<typename T>
mat::mat3<T> transpose(const mat::mat3<T>& mat){

    // A B C
    // D E F
    // G H I
    constexpr int A = 0;
    constexpr int B = 3;
    constexpr int C = 6;
    constexpr int D = 1;
    constexpr int E = 4;
    constexpr int F = 7;
    constexpr int G = 2;
    constexpr int H = 5;
    constexpr int I = 8;

    return mat::mat3<T>(
            mat.data[A],
            mat.data[D],
            mat.data[G],
            mat.data[B],
            mat.data[E],
            mat.data[H],
            mat.data[C],
            mat.data[F],
            mat.data[I]);
}

template<typename T>
mat::mat3<T> inverse(const mat::mat3<T>& mat){
    // NOTE(hugo): https://en.wikipedia.org/wiki/Invertible_matrix#Inversion_of_3_%C3%97_3_matrices

    T det = determinant(mat);
    T factor;
    if(det){
        factor = 1 / det;
    }else{
        return mat::mat3<T>();
    }

    // A B C
    // D E F
    // G H I
    constexpr int A = 0;
    constexpr int B = 3;
    constexpr int C = 6;
    constexpr int D = 1;
    constexpr int E = 4;
    constexpr int F = 7;
    constexpr int G = 2;
    constexpr int H = 5;
    constexpr int I = 8;

    return mat::mat3<T>(
            (mat.data[E] * mat.data[I] - mat.data[F] * mat.data[H]) * factor,
            (mat.data[C] * mat.data[H] - mat.data[B] * mat.data[I]) * factor,
            (mat.data[B] * mat.data[F] - mat.data[C] * mat.data[E]) * factor,
            (mat.data[F] * mat.data[G] - mat.data[D] * mat.data[I]) * factor,
            (mat.data[A] * mat.data[I] - mat.data[C] * mat.data[G]) * factor,
            (mat.data[C] * mat.data[D] - mat.data[A] * mat.data[F]) * factor,
            (mat.data[D] * mat.data[H] - mat.data[E] * mat.data[G]) * factor,
            (mat.data[B] * mat.data[G] - mat.data[A] * mat.data[H]) * factor,
            (mat.data[A] * mat.data[E] - mat.data[B] * mat.data[D]) * factor);
}

template<typename T>
vec::vec3<T> operator*(const mat::mat3<T>& mat, const vec::vec3<T>& vec){
    return vec::vec3<T>(mat.data[0] * vec.data[0] + mat.data[3] * vec.data[1] + mat.data[6] * vec.data[2],
            mat.data[1] * vec.data[0] + mat.data[4] * vec.data[1] + mat.data[7] * vec.data[2],
            mat.data[2] * vec.data[0] + mat.data[5] * vec.data[1] + mat.data[8] * vec.data[2]);

}

// ---- mat::mat4

template<typename T>
mat::mat4<T>& mat::mat4<T>::operator+=(const mat::mat4<T>& mat){
    data[0] += mat.data[0];
    data[1] += mat.data[1];
    data[2] += mat.data[2];
    data[3] += mat.data[3];
    data[4] += mat.data[4];
    data[5] += mat.data[5];
    data[6] += mat.data[6];
    data[7] += mat.data[7];
    data[8] += mat.data[8];
    data[9] += mat.data[9];
    data[10] += mat.data[10];
    data[11] += mat.data[11];
    data[12] += mat.data[12];
    data[13] += mat.data[13];
    data[14] += mat.data[14];
    data[15] += mat.data[15];

    return *this;
}
template<typename T>
mat::mat4<T>& mat::mat4<T>::operator+=(const T& v){
    data[0] += v;
    data[1] += v;
    data[2] += v;
    data[3] += v;
    data[4] += v;
    data[5] += v;
    data[6] += v;
    data[7] += v;
    data[8] += v;
    data[9] += v;
    data[10] += v;
    data[11] += v;
    data[12] += v;
    data[13] += v;
    data[14] += v;
    data[15] += v;

    return *this;
}
template<typename T>
mat::mat4<T>& mat::mat4<T>::operator-=(const mat::mat4<T>& mat){
    data[0] -= mat.data[0];
    data[1] -= mat.data[1];
    data[2] -= mat.data[2];
    data[3] -= mat.data[3];
    data[4] -= mat.data[4];
    data[5] -= mat.data[5];
    data[6] -= mat.data[6];
    data[7] -= mat.data[7];
    data[8] -= mat.data[8];
    data[9] -= mat.data[9];
    data[10] -= mat.data[10];
    data[11] -= mat.data[11];
    data[12] -= mat.data[12];
    data[13] -= mat.data[13];
    data[14] -= mat.data[14];
    data[15] -= mat.data[15];

    return *this;
}
template<typename T>
mat::mat4<T>& mat::mat4<T>::operator-=(const T& v){
    data[0] -= v;
    data[1] -= v;
    data[2] -= v;
    data[3] -= v;
    data[4] -= v;
    data[5] -= v;
    data[6] -= v;
    data[7] -= v;
    data[8] -= v;
    data[9] -= v;
    data[10] -= v;
    data[11] -= v;
    data[12] -= v;
    data[13] -= v;
    data[14] -= v;
    data[15] -= v;

    return *this;
}

template<typename T>
mat::mat4<T>& mat::mat4<T>::operator*=(const mat::mat4<T>& mat){
    T this_copy[16] = {data[0], data[1], data[2], data[3],
        data[4], data[5], data[6], data[7],
        data[8], data[9], data[10], data[11],
        data[12], data[13], data[14], data[15]};

    data[0] = this_copy[0] * mat.data[0] + this_copy[4] * mat.data[1] + this_copy[8] * mat.data[2] + this_copy[12] * mat.data[3];
    data[1] = this_copy[1] * mat.data[0] + this_copy[5] * mat.data[1] + this_copy[9] * mat.data[2] + this_copy[13] * mat.data[3];
    data[2] = this_copy[2] * mat.data[0] + this_copy[6] * mat.data[1] + this_copy[10] * mat.data[2] + this_copy[14] * mat.data[3];
    data[3] = this_copy[3] * mat.data[0] + this_copy[7] * mat.data[1] + this_copy[11] * mat.data[2] + this_copy[15] * mat.data[3];

    data[4] = this_copy[0] * mat.data[4] + this_copy[4] * mat.data[5] + this_copy[8] * mat.data[6] + this_copy[12] * mat.data[7];
    data[5] = this_copy[1] * mat.data[4] + this_copy[5] * mat.data[5] + this_copy[9] * mat.data[6] + this_copy[13] * mat.data[7];
    data[6] = this_copy[2] * mat.data[4] + this_copy[6] * mat.data[5] + this_copy[10] * mat.data[6] + this_copy[14] * mat.data[7];
    data[7] = this_copy[3] * mat.data[4] + this_copy[7] * mat.data[5] + this_copy[11] * mat.data[6] + this_copy[15] * mat.data[7];

    data[8] = this_copy[0] * mat.data[8] + this_copy[4] * mat.data[9] + this_copy[8] * mat.data[10] + this_copy[12] * mat.data[11];
    data[9] = this_copy[1] * mat.data[8] + this_copy[5] * mat.data[9] + this_copy[9] * mat.data[10] + this_copy[13] * mat.data[11];
    data[10] = this_copy[2] * mat.data[8] + this_copy[6] * mat.data[9] + this_copy[10] * mat.data[10] + this_copy[14] * mat.data[11];
    data[11] = this_copy[3] * mat.data[8] + this_copy[7] * mat.data[9] + this_copy[11] * mat.data[10] + this_copy[15] * mat.data[11];

    data[12] = this_copy[0] * mat.data[12] + this_copy[4] * mat.data[13] + this_copy[8] * mat.data[14] + this_copy[12] * mat.data[15];
    data[13] = this_copy[1] * mat.data[12] + this_copy[5] * mat.data[13] + this_copy[9] * mat.data[14] + this_copy[13] * mat.data[15];
    data[14] = this_copy[2] * mat.data[12] + this_copy[6] * mat.data[13] + this_copy[10] * mat.data[14] + this_copy[14] * mat.data[15];
    data[15] = this_copy[3] * mat.data[12] + this_copy[7] * mat.data[13] + this_copy[11] * mat.data[14] + this_copy[15] * mat.data[15];

    return *this;
}

template<typename T>
mat::mat4<T>& mat::mat4<T>::operator*=(const T& v){
    data[0] *= v;
    data[1] *= v;
    data[2] *= v;
    data[3] *= v;
    data[4] *= v;
    data[5] *= v;
    data[6] *= v;
    data[7] *= v;
    data[8] *= v;
    data[9] *= v;
    data[10] *= v;
    data[11] *= v;
    data[12] *= v;
    data[13] *= v;
    data[14] *= v;
    data[15] *= v;

    return *this;
}
template<typename T>
mat::mat4<T>& mat::mat4<T>::operator/=(const T& v){
    data[0] /= v;
    data[1] /= v;
    data[2] /= v;
    data[3] /= v;
    data[4] /= v;
    data[5] /= v;
    data[6] /= v;
    data[7] /= v;
    data[8] /= v;
    data[9] /= v;
    data[10] /= v;
    data[11] /= v;
    data[12] /= v;
    data[13] /= v;
    data[14] /= v;
    data[15] /= v;

    return *this;
}

template<typename T>
mat::mat4<T> mat::mat4<T>::operator+() const{
    return *this;
}
template<typename T>
mat::mat4<T> mat::mat4<T>::operator-() const{
    mat::mat4<T> copy;
    copy.data[0] = - this->data[0];
    copy.data[1] = - this->data[1];
    copy.data[2] = - this->data[2];
    copy.data[3] = - this->data[3];
    copy.data[4] = - this->data[4];
    copy.data[5] = - this->data[5];
    copy.data[6] = - this->data[6];
    copy.data[7] = - this->data[7];
    copy.data[8] = - this->data[8];
    copy.data[9] = - this->data[9];
    copy.data[10] = - this->data[10];
    copy.data[11] = - this->data[11];
    copy.data[12] = - this->data[12];
    copy.data[13] = - this->data[13];
    copy.data[14] = - this->data[14];
    copy.data[15] = - this->data[15];

    return copy;
}

template<typename T>
bool mat::mat4<T>::operator==(const mat::mat4<T>& mat) const{
    return (data[0] == mat.data[0])
        && (data[1] == mat.data[1])
        && (data[2] == mat.data[2])
        && (data[3] == mat.data[3])
        && (data[4] == mat.data[4])
        && (data[5] == mat.data[5])
        && (data[6] == mat.data[6])
        && (data[7] == mat.data[7])
        && (data[8] == mat.data[8])
        && (data[9] == mat.data[9])
        && (data[10] == mat.data[10])
        && (data[11] == mat.data[11])
        && (data[12] == mat.data[12])
        && (data[13] == mat.data[13])
        && (data[14] == mat.data[14])
        && (data[15] == mat.data[15]);
}

template<typename T>
mat::mat4<T> operator+(const mat::mat4<T>& lhs, const mat::mat4<T>& rhs){
    return mat::mat4<T>(lhs) += rhs;
}
template<typename T>
mat::mat4<T> operator-(const mat::mat4<T>& lhs, const mat::mat4<T>& rhs){
    return mat::mat4<T>(lhs) -= rhs;
}
template<typename T>
mat::mat4<T> operator*(const mat::mat4<T>& lhs, const mat::mat4<T>& rhs){
    return mat::mat4<T>(lhs) *= rhs;
}

template<typename T>
mat::mat4<T> operator+(const mat::mat4<T>& lhs, const T& rhs){
    return mat::mat4<T>(lhs) += rhs;
}
template<typename T>
mat::mat4<T> operator-(const mat::mat4<T>& lhs, const T& rhs){
    return mat::mat4<T>(lhs) -= rhs;
}
template<typename T>
mat::mat4<T> operator*(const mat::mat4<T>& lhs, const T& rhs){
    return mat::mat4<T>(lhs) *= rhs;
}
template<typename T>
mat::mat4<T> operator/(const mat::mat4<T>& lhs, const T& rhs){
    return mat::mat4<T>(lhs) /= rhs;
}

template<typename T>
mat::mat4<T> operator+(const T& lhs, const mat::mat4<T>& rhs){
    return mat::mat4<T>(rhs) += lhs;
}
template<typename T>
mat::mat4<T> operator-(const T& lhs, const mat::mat4<T>& rhs){
    return mat::mat4<T>(rhs) -= lhs;
}
template<typename T>
mat::mat4<T> operator*(const T& lhs, const mat::mat4<T>& rhs){
    return mat::mat4<T>(rhs) *= lhs;
}
template<typename T>
mat::mat4<T> operator/(const T& lhs, const mat::mat4<T>& rhs){
    return mat::mat4<T>(rhs) /= lhs;
}

#if 0
template<typename T>
T determinant(const mat::mat4<T>& mat){
}

template<typename T>
mat::mat4<T> transpose(const mat::mat4<T>& mat){
}

template<typename T>
mat::mat4<T> inverse(const mat::mat4<T>& mat){
}
#endif

template<typename T>
vec::vec4<T> operator*(const mat::mat4<T>& mat, const vec::vec4<T>& vec){
    return {
        mat.data[0] * vec.data[0] + mat.data[4] * vec.data[1] + mat.data[8] * vec.data[2] + mat.data[12] * vec.data[3],
        mat.data[1] * vec.data[0] + mat.data[5] * vec.data[1] + mat.data[9] * vec.data[2] + mat.data[13] * vec.data[3],
        mat.data[2] * vec.data[0] + mat.data[6] * vec.data[1] + mat.data[10] * vec.data[2] + mat.data[14] * vec.data[3],
        mat.data[3] * vec.data[0] + mat.data[7] * vec.data[1] + mat.data[11] * vec.data[2] + mat.data[15] * vec.data[3]
    };
}

// ---- additional matrix math

template<typename T>
mat::mat3<T> mat3_from_orthonormal_basis(const vec::vec3<T> right, const vec::vec3<T> up, const vec::vec3<T> forward){
    return {
        right.x, right.y, right.z,
        up.x, up.y, up.z,
        forward.x, forward.y, forward.z
    };
}

template<typename T>
mat::mat4<T> mat4_from_orthonormal_basis(const vec::vec3<T> right, const vec::vec3<T> up, const vec::vec3<T> forward){
    return {
        right.x, right.y, right.z, (T)(0),
        up.x, up.y, up.z, (T)(0),
        forward.x, forward.y, forward.z, (T)(0),
        (T)(0), (T)(0), (T)(0), (T)(1)
    };
}

// ---- std140 conversions ---- //

inline mat2_std140 to_std140(const mat2& mat){
    mat2_std140 output;

    output.data[0] = mat.data[0];
    output.data[1] = mat.data[1];

    output.data[2] = mat.data[2];
    output.data[3] = mat.data[3];

    return output;
}
inline mat3_std140 to_std140(const mat3& mat){
    mat3_std140 output;

    output.data[0] = mat.data[0];
    output.data[1] = mat.data[1];
    output.data[2] = mat.data[2];

    output.data[4] = mat.data[3];
    output.data[5] = mat.data[4];
    output.data[6] = mat.data[5];

    output.data[8] = mat.data[6];
    output.data[9] = mat.data[7];
    output.data[10] = mat.data[8];

    return output;
}
inline mat4_std140 to_std140(const mat4& mat){
    mat4_std140 output;

    for(u32 ifloat = 0u; ifloat != 16; ++ifloat){
        output.data[ifloat] = mat.data[ifloat];
    }

    return output;
}

