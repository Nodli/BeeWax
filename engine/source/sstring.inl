template<u32 capacity>
sstring<capacity>::sstring() : data(""), size(0u) {}

template<u32 capacity>
sstring<capacity>::operator char* (){
    return data;
}

template<u32 capacity>
template<u32 cstring_capacity>
constexpr sstring<capacity>::sstring(const char (&cstring)[cstring_capacity]){
    assert(!(cstring_capacity > capacity));
    memcpy(data, cstring, cstring_capacity);
    size = cstring_capacity - 1u;
}

template<u32 capacity>
void sstring<capacity>::extract_from(const char* iptr, u32 isize){
    assert((isize + 1u) < capacity);
    memcpy(data, iptr, isize);
    size = isize;
    data[size] = '\0';
}

template<u32 capacity>
template<u32 rhs_capacity>
sstring<capacity>& sstring<capacity>::operator=(const sstring<rhs_capacity>& rhs){
    assert(rhs.size < capacity);
    memcpy(data, rhs.data, rhs.size);
    size = rhs.size;
    data[size] = '\0';
    return *this;

}

template<u32 capacity>
sstring<capacity>& sstring<capacity>::operator=(const char* rhs){
    u32 rhs_size = strlen(rhs);
    assert(rhs_size < capacity);
    memcpy(data, rhs, rhs_size);
    size = rhs_size;
    data[size] = '\0';
    return *this;
}

template<u32 lhs_capacity, u32 rhs_capacity>
bool operator==(const sstring<lhs_capacity>& lhs, const sstring<rhs_capacity>& rhs){
    return (lhs.size == rhs.size) && (memcmp(lhs.data, rhs.data, lhs.size) == 0);
}

template<u32 capacity>
bool operator==(const sstring<capacity>& lhs, const char* rhs){
    u32 rhs_size = strlen(rhs);
    return (lhs.size == rhs_size) && (memcmp(lhs.data, rhs, rhs_size) == 0u);
}

template<u32 capacity>
bool operator==(const char* lhs, const sstring<capacity>& rhs){
    return rhs == lhs;
}
