// ---- vec::vec2

template<typename T>
vec::vec2<T>& vec::vec2<T>::operator++(){
    ++x;
    ++y;
}
template<typename T>
vec::vec2<T> vec::vec2<T>::operator++(int){
    vec::vec2<T> copy = *this;
    operator++();
    return copy;
}
template<typename T>
vec::vec2<T>& vec::vec2<T>::operator--(){
    --x;
    --y;
}
template<typename T>
vec::vec2<T> vec::vec2<T>::operator--(int){
    vec::vec2<T> copy = *this;
    operator--();
    return copy;
}

template<typename T>
vec::vec2<T>& vec::vec2<T>::operator+=(const vec::vec2<T>& vec){
    x += vec.x;
    y += vec.y;
    return *this;
}
//template<typename T>
//vec::vec2<T>& vec::vec2<T>::operator+=(const T v){
//    x += v;
//    y += v;
//    return *this;
//}
template<typename T>
vec::vec2<T>& vec::vec2<T>::operator-=(const vec::vec2<T>& vec){
    x -= vec.x;
    y -= vec.y;
    return *this;
}
//template<typename T>
//vec::vec2<T>& vec::vec2<T>::operator-=(const T v){
//    x -= v;
//    y -= v;
//    return *this;
//}
template<typename T>
vec::vec2<T>& vec::vec2<T>::operator*=(const vec::vec2<T>& vec){
    x *= vec.x;
    y *= vec.y;
    return *this;
}
template<typename T>
vec::vec2<T>& vec::vec2<T>::operator*=(const T v){
    x *= v;
    y *= v;
    return *this;
}
template<typename T>
vec::vec2<T>& vec::vec2<T>::operator/=(const vec::vec2<T>& vec){
    x /= vec.x;
    y /= vec.y;
    return *this;
}
template<typename T>
vec::vec2<T>& vec::vec2<T>::operator/=(const T v){
    x /= v;
    y /= v;
    return *this;
}

template<typename T>
vec::vec2<T> vec::vec2<T>::operator+() const{
    return *this;
}
template<typename T>
vec::vec2<T> vec::vec2<T>::operator-() const{
    return {-x, -y};
}

template<typename T>
bool vec::vec2<T>::operator==(const vec::vec2<T>& vec) const{
    return (x == vec.x) && (y == vec.y);
}

template<typename T>
T& vec::vec2<T>::operator[](const u32 index){
    assert(index < 2u);
    return data[index];
}

template<typename T>
T vec::vec2<T>::operator[](const u32 index) const{
    assert(index < 2u);
    return data[index];
}

template<typename T>
vec::vec2<T> operator+(const vec::vec2<T>& lhs, const vec::vec2<T>& rhs){
    return vec::vec2<T>(lhs) += rhs;
}
template<typename T>
vec::vec2<T> operator-(const vec::vec2<T>& lhs, const vec::vec2<T>& rhs){
    return vec::vec2<T>(lhs) -= rhs;
}
template<typename T>
vec::vec2<T> operator*(const vec::vec2<T>& lhs, const vec::vec2<T>& rhs){
    return vec::vec2<T>(lhs) *= rhs;
}
template<typename T>
vec::vec2<T> operator/(const vec::vec2<T>& lhs, const vec::vec2<T>& rhs){
    return vec::vec2<T>(lhs) /= rhs;
}

//template<typename T>
//vec::vec2<T> operator+(const vec::vec2<T>& lhs, const T rhs){
//    return vec::vec2<T>(lhs) += rhs;
//}
//template<typename T>
//vec::vec2<T> operator-(const vec::vec2<T>& lhs, const T rhs){
//    return vec::vec2<T>(lhs) -= rhs;
//}
template<typename T>
vec::vec2<T> operator*(const vec::vec2<T>& lhs, const T rhs){
    return vec::vec2<T>(lhs) *= rhs;
}
template<typename T>
vec::vec2<T> operator/(const vec::vec2<T>& lhs, const T rhs){
    return vec::vec2<T>(lhs) /= rhs;
}

template<typename T>
vec::vec2<T> operator+(const T lhs, const vec::vec2<T>& rhs){
    return vec::vec2<T>(rhs) += lhs;
}
template<typename T>
vec::vec2<T> operator-(const T lhs, const vec::vec2<T>& rhs){
    return vec::vec2<T>(rhs) -= lhs;
}
template<typename T>
vec::vec2<T> operator*(const T lhs, const vec::vec2<T>& rhs){
    return vec::vec2<T>(rhs) *= lhs;
}
template<typename T>
vec::vec2<T> operator/(const T lhs, const vec::vec2<T>& rhs){
    return vec::vec2<T>(rhs) /= lhs;
}

template<typename T>
vec::vec2<T> abs(const vec::vec2<T>& vec){
    return vec::vec2<T>(abs(vec.x), abs(vec.y));
}

// ---- vec::vec3

template<typename T>
vec::vec3<T>& vec::vec3<T>::operator++(){
    ++x;
    ++y;
    ++z;
}
template<typename T>
vec::vec3<T> vec::vec3<T>::operator++(int){
    vec::vec3<T> copy = *this;
    operator++();
    return copy;
}
template<typename T>
vec::vec3<T>& vec::vec3<T>::operator--(){
    --x;
    --y;
    --z;
}
template<typename T>
vec::vec3<T> vec::vec3<T>::operator--(int){
    vec::vec3<T> copy = *this;
    operator--();
    return copy;
}

template<typename T>
vec::vec3<T>& vec::vec3<T>::operator+=(const vec::vec3<T>& vec){
    x += vec.x;
    y += vec.y;
    z += vec.z;
    return *this;
}
//template<typename T>
//vec::vec3<T>& vec::vec3<T>::operator+=(const T v){
//    x += v;
//    y += v;
//    z += v;
//    return *this;
//}
template<typename T>
vec::vec3<T>& vec::vec3<T>::operator-=(const vec::vec3<T>& vec){
    x -= vec.x;
    y -= vec.y;
    z -= vec.z;
    return *this;
}
//template<typename T>
//vec::vec3<T>& vec::vec3<T>::operator-=(const T v){
//    x -= v;
//    y -= v;
//    z -= v;
//    return *this;
//}
template<typename T>
vec::vec3<T>& vec::vec3<T>::operator*=(const vec::vec3<T>& vec){
    x *= vec.x;
    y *= vec.y;
    z *= vec.z;
    return *this;
}
template<typename T>
vec::vec3<T>& vec::vec3<T>::operator*=(const T v){
    x *= v;
    y *= v;
    z *= v;
    return *this;
}
template<typename T>
vec::vec3<T>& vec::vec3<T>::operator/=(const vec::vec3<T>& vec){
    x /= vec.x;
    y /= vec.y;
    z /= vec.z;
    return *this;
}
template<typename T>
vec::vec3<T>& vec::vec3<T>::operator/=(const T v){
    x /= v;
    y /= v;
    z /= v;
    return *this;
}

template<typename T>
vec::vec3<T> vec::vec3<T>::operator+() const{
    return *this;
}
template<typename T>
vec::vec3<T> vec::vec3<T>::operator-() const{
    return {-x, -y, -z};
}

template<typename T>
bool vec::vec3<T>::operator==(const vec::vec3<T>& vec) const{
    return (x == vec.x) && (y == vec.y) && (z == vec.z);
}

template<typename T>
T& vec::vec3<T>::operator[](const u32 index){
    assert(index < 3u);
    return data[index];
}

template<typename T>
T vec::vec3<T>::operator[](const u32 index) const{
    assert(index < 3u);
    return data[index];
}

template<typename T>
vec::vec3<T> operator+(const vec::vec3<T>& lhs, const vec::vec3<T>& rhs){
    return vec::vec3<T>(lhs) += rhs;
}
template<typename T>
vec::vec3<T> operator-(const vec::vec3<T>& lhs, const vec::vec3<T>& rhs){
    return vec::vec3<T>(lhs) -= rhs;
}
template<typename T>
vec::vec3<T> operator*(const vec::vec3<T>& lhs, const vec::vec3<T>& rhs){
    return vec::vec3<T>(lhs) *= rhs;
}
template<typename T>
vec::vec3<T> operator/(const vec::vec3<T>& lhs, const vec::vec3<T>& rhs){
    return vec::vec3<T>(lhs) /= rhs;
}

//template<typename T>
//vec::vec3<T> operator+(const vec::vec3<T>& lhs, const T rhs){
//    return vec::vec3<T>(lhs) += rhs;
//}
//template<typename T>
//vec::vec3<T> operator-(const vec::vec3<T>& lhs, const T rhs){
//    return vec::vec3<T>(lhs) -= rhs;
//}
template<typename T>
vec::vec3<T> operator*(const vec::vec3<T>& lhs, const T rhs){
    return vec::vec3<T>(lhs) *= rhs;
}
template<typename T>
vec::vec3<T> operator/(const vec::vec3<T>& lhs, const T rhs){
    return vec::vec3<T>(lhs) /= rhs;
}

template<typename T>
vec::vec3<T> operator+(const T lhs, const vec::vec3<T>& rhs){
    return vec::vec3<T>(rhs) += lhs;
}
template<typename T>
vec::vec3<T> operator-(const T lhs, const vec::vec3<T>& rhs){
    return vec::vec3<T>(rhs) -= lhs;
}
template<typename T>
vec::vec3<T> operator*(const T lhs, const vec::vec3<T>& rhs){
    return vec::vec3<T>(rhs) *= lhs;
}
template<typename T>
vec::vec3<T> operator/(const T lhs, const vec::vec3<T>& rhs){
    return vec::vec3<T>(rhs) /= lhs;
}

template<typename T>
vec::vec3<T> abs(const vec::vec3<T>& vec){
    return vec::vec3<T>(abs(vec.x), abs(vec.y), abs(vec.z));
}

// ---- vec::vec4

template<typename T>
vec::vec4<T>& vec::vec4<T>::operator++(){
    ++x;
    ++y;
    ++z;
    ++w;
}
template<typename T>
vec::vec4<T> vec::vec4<T>::operator++(int){
    vec::vec4<T> copy = *this;
    operator++();
    return copy;
}
template<typename T>
vec::vec4<T>& vec::vec4<T>::operator--(){
    --x;
    --y;
    --z;
    --w;
}
template<typename T>
vec::vec4<T> vec::vec4<T>::operator--(int){
    vec::vec4<T> copy = *this;
    operator--();
    return copy;
}

template<typename T>
vec::vec4<T>& vec::vec4<T>::operator+=(const vec::vec4<T>& vec){
    x += vec.x;
    y += vec.y;
    z += vec.z;
    w += vec.w;
    return *this;
}
//template<typename T>
//vec::vec4<T>& vec::vec4<T>::operator+=(const T v){
//    x += v;
//    y += v;
//    z += v;
//    w += v;
//    return *this;
//}
template<typename T>
vec::vec4<T>& vec::vec4<T>::operator-=(const vec::vec4<T>& vec){
    x -= vec.x;
    y -= vec.y;
    z -= vec.z;
    w -= vec.w;
    return *this;
}
//template<typename T>
//vec::vec4<T>& vec::vec4<T>::operator-=(const T v){
//    x -= v;
//    y -= v;
//    z -= v;
//    w -= v;
//    return *this;
//}
template<typename T>
vec::vec4<T>& vec::vec4<T>::operator*=(const vec::vec4<T>& vec){
    x *= vec.x;
    y *= vec.y;
    z *= vec.z;
    w *= vec.w;
    return *this;
}
template<typename T>
vec::vec4<T>& vec::vec4<T>::operator*=(const T v){
    x *= v;
    y *= v;
    z *= v;
    w *= v;
    return *this;
}
template<typename T>
vec::vec4<T>& vec::vec4<T>::operator/=(const vec::vec4<T>& vec){
    x /= vec.x;
    y /= vec.y;
    z /= vec.z;
    w /= vec.w;
    return *this;
}
template<typename T>
vec::vec4<T>& vec::vec4<T>::operator/=(const T v){
    x /= v;
    y /= v;
    z /= v;
    w /= v;
    return *this;
}

template<typename T>
vec::vec4<T> vec::vec4<T>::operator+() const{
    return *this;
}
template<typename T>
vec::vec4<T> vec::vec4<T>::operator-() const{
    return {-x, -y, -z, -w};
}

template<typename T>
bool vec::vec4<T>::operator==(const vec::vec4<T>& vec) const{
    return (x == vec.x) && (y == vec.y) && (z == vec.z) && (w == vec.w);
}

template<typename T>
T& vec::vec4<T>::operator[](const u32 index){
    assert(index < 4u);
    return data[index];
}

template<typename T>
T vec::vec4<T>::operator[](const u32 index) const{
    assert(index < 4u);
    return data[index];
}

template<typename T>
vec::vec4<T> operator+(const vec::vec4<T>& lhs, const vec::vec4<T>& rhs){
    return vec::vec4<T>(lhs) += rhs;
}
template<typename T>
vec::vec4<T> operator-(const vec::vec4<T>& lhs, const vec::vec4<T>& rhs){
    return vec::vec4<T>(lhs) -= rhs;
}
template<typename T>
vec::vec4<T> operator*(const vec::vec4<T>& lhs, const vec::vec4<T>& rhs){
    return vec::vec4<T>(lhs) *= rhs;
}
template<typename T>
vec::vec4<T> operator/(const vec::vec4<T>& lhs, const vec::vec4<T>& rhs){
    return vec::vec4<T>(lhs) /= rhs;
}

//template<typename T>
//vec::vec4<T> operator+(const vec::vec4<T>& lhs, const T rhs){
//    return vec::vec4<T>(lhs) += rhs;
//}
//template<typename T>
//vec::vec4<T> operator-(const vec::vec4<T>& lhs, const T rhs){
//    return vec::vec4<T>(lhs) -= rhs;
//}
template<typename T>
vec::vec4<T> operator*(const vec::vec4<T>& lhs, const T rhs){
    return vec::vec4<T>(lhs) *= rhs;
}
template<typename T>
vec::vec4<T> operator/(const vec::vec4<T>& lhs, const T rhs){
    return vec::vec4<T>(lhs) /= rhs;
}

template<typename T>
vec::vec4<T> operator+(const T lhs, const vec::vec4<T>& rhs){
    return vec::vec4<T>(rhs) += lhs;
}
template<typename T>
vec::vec4<T> operator-(const T lhs, const vec::vec4<T>& rhs){
    return vec::vec4<T>(rhs) -= lhs;
}
template<typename T>
vec::vec4<T> operator*(const T lhs, const vec::vec4<T>& rhs){
    return vec::vec4<T>(rhs) *= lhs;
}
template<typename T>
vec::vec4<T> operator/(const T lhs, const vec::vec4<T>& rhs){
    return vec::vec4<T>(rhs) /= lhs;
}

template<typename T>
vec::vec4<T> abs(const vec::vec4<T>& vec){
    return vec::vec4<T>(abs(vec.x), abs(vec.y), abs(vec.z), abs(vec.w));
}
