template<u32 capacity>
template<u32 cstring_capacity>
constexpr sstring<capacity>::sstring(const char (&cstring)[cstring_capacity]){
    assert(cstring_capacity <= capacity);
    memcpy(data, cstring, cstring_capacity);
    size = cstring_capacity - 1u;
}

template<u32 capacity>
void sstring<capacity>::extract_from(const char* iptr, u32 isize){
    assert(isize + 1u < capacity);
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

template<u32 capacity>
template<u32 rhs_capacity>
bool sstring<capacity>::operator==(const sstring<rhs_capacity>& rhs) const{
    return (size == rhs.size) && (memcmp(data, rhs.data, size) == 0);
}

template<u32 capacity>
bool sstring<capacity>::operator==(const char* rhs) const{
    u32 rhs_size = strlen(rhs);
    return (size == rhs_size) && (memcpy(data, rhs, size) == 0u);
}

template<u32 capacity>
u32 hashmap_hash_key(const sstring<capacity>& key){
    return FNV1a_32ptr((uchar*)key.data, key.size);
}
