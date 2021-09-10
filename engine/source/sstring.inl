/*
template<size_t bytesize>
sstring::string() : data(""), empty_bytes(bytesize){
}

template<size_t bytesize>
template<size_t cstr_bytesize>
constexpr sstring<bytesize>::sstring(const char (&cstr)[cstr_bytesize]){
    static_assert(cstr_bytesize < bytesize || ctr_bytesize == bytesize);
    memcpy(data, cstr, cstr_bytesize);
    empty_bytes = bytesize - cstr_bytesize;
}
*/

template<size_t bytesize>
size_t sstring<bytesize>::strlen() const{
    return max(bytesize - empty_bytes, (size_t) 1u) - 1u;
}

template<size_t bytesize>
constexpr size_t sstring<bytesize>::strcap(){
    return bytesize - 1u;
}

template<size_t bytesize>
void sstring<bytesize>::extract_from(const char* str, size_t strlen){
    assert(strlen < bytesize);

    memcpy(data, str, strlen);
    data[strlen + 1u] = '\0';
    empty_bytes = bytesize - strlen - 1u;
}

template<size_t bytesize>
template<size_t str_bytesize>
sstring<bytesize>& sstring<bytesize>::operator=(const sstring<str_bytesize>& str){
    size_t str_strlen = str.strlen();
    assert(str_strlen < bytesize);

    memcpy(data, str.data, str_strlen + 1u);
    empty_bytes = str.empty_bytes;
    return *this;
}

template<size_t bytesize>
sstring<bytesize>& sstring<bytesize>::operator=(const char* str){
    size_t str_strlen = ::strlen(str);
    assert(str_strlen < bytesize);

    memcpy(data, str, str_strlen + 1u);
    empty_bytes = bytesize - str_strlen - 1u;
    return *this;
}

template<size_t bytesizeL, size_t bytesizeR>
bool operator==(const sstring<bytesizeL>& L, const sstring<bytesizeR>& R){
    size_t L_strlen = L.strlen();
    size_t R_strlen= R.strlen();
    return (L_strlen == R_strlen) && (memcmp(L.data, R.data, L_strlen) == 0);
}

template<size_t bytesize>
bool operator==(const sstring<bytesize>& sstr, const char* cstr){
    size_t sstr_strlen = sstr.strlen();
    size_t cstr_strlen = ::strlen(cstr);
    return (sstr_strlen == cstr_strlen) && (memcmp(sstr.data, cstr, sstr_strlen) == 0);
}

template<size_t bytesize>
bool operator==(const char* cstr, const sstring<bytesize>& sstr){
    return sstr == cstr;
}

template<size_t bytesizeL, size_t bytesizeR>
bool operator!=(const sstring<bytesizeL>& L, const sstring<bytesizeR>& R){
    return !(L == R);
}

template<size_t bytesize>
bool operator!=(const sstring<bytesize>& sstr, const char* cstr){
    return !(sstr == cstr);
}

template<size_t bytesize>
bool operator!=(const char* cstr, const sstring<bytesize>& sstr){
    return !(cstr == sstr);
}

template<size_t bytesize>
u32 hashmap_hash(const sstring<bytesize>& str){
    return hashmap_hash((const char*)str.data, str.strlen());
}
