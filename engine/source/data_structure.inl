// ---- array

namespace BEEWAX_INTERNAL{
    // NOTE(hugo): next power of two
    inline size_t array_next_capacity(size_t capacity){
        capacity = capacity * 2u;

        capacity |= capacity >> 1u;
        capacity |= capacity >> 2u;
        capacity |= capacity >> 4u;

        if constexpr (sizeof(capacity) > 1u) capacity |= capacity >> 8u;
        if constexpr (sizeof(capacity) > 2u) capacity |= capacity >> 16u;
        if constexpr (sizeof(capacity) > 4u) capacity |= capacity >> 32u;

        return max((size_t)16u, capacity);
    }

    template<typename T>
    void array_reallocate_to_capacity(array<T>& array, size_t new_capacity){
        void* new_data = bw_realloc((void*)array.data, new_capacity * sizeof(T));
        assert(new_data);

        array.data = (T*)new_data;
        array.capacity = new_capacity;
    }

    template<typename T>
    void array_increase_capacity(array<T>& array){
        size_t new_capacity = BEEWAX_INTERNAL::array_next_capacity(array.capacity);
        array_reallocate_to_capacity(array, new_capacity);
    }

    template<typename T>
    void array_increase_capacity_min(array<T>& array, size_t min_capacity){
        size_t new_capacity = BEEWAX_INTERNAL::array_next_capacity(max(array.capacity, min_capacity));
        array_reallocate_to_capacity(array, new_capacity);
    }
}


template<typename T>
void array<T>::create(){
    data = nullptr;
    size = (size_t)0u;
    capacity = (size_t)0u;
}

template<typename T>
void array<T>::destroy(){
    bw_free(data);
}

template<typename T>
const T& array<T>::operator[](size_t index) const{
    assert(index < size);
    return data[index];
}

template<typename T>
T& array<T>::operator[](size_t index){
    assert(index < size);
    return data[index];
}

template<typename T>
T& array<T>::push(const T& v){
    if(size == capacity) BEEWAX_INTERNAL::array_increase_capacity(*this);
    data[size] = v;
    return data[size++];
}

template<typename T>
T array<T>::pop(){
    assert(size);
    --size;
    return data[size];
}

template<typename T>
T& array<T>::insert(size_t index, const T& v){
    assert(!(index > size));

    if(size == capacity) BEEWAX_INTERNAL::array_increase_capacity(*this);
    if(index != size) memmove(data + index + 1u, data + index, (size - index) * sizeof(T));
    data[index] = v;
    ++size;
    return data[index];
}

template<typename T>
T* array<T>::insert_multi(size_t index, size_t count){
    assert(!(index > size));

    if(size + count > capacity) BEEWAX_INTERNAL::array_increase_capacity_min(*this, size + count);
    if(index != size) memmove(data + index + count, data + index, (size - index) * sizeof(T));
    size += count;
    return data + index;
}

template<typename T>
void array<T>::remove(size_t index){
    assert(index < size);

    if(index < size - 1u) memmove(data + index, data + index + 1u, (size - index - 1u) * sizeof(T));
    --size;
}

template<typename T>
void array<T>::remove_multi(size_t index, size_t count){
    assert(index + count < size + 1u);

    if(index + count < size) memmove(data + index, data + index + count, (size - index - count) * sizeof(T));
    size -= count;
}

template<typename T>
void array<T>::remove_swap(size_t index){
    assert(index < size);
    --size;
    data[index] = data[size];
}

template<typename T>
void array<T>::resize(size_t new_size){
    if(new_size > capacity) BEEWAX_INTERNAL::array_increase_capacity_min(*this, new_size);
    size = new_size;
}

template<typename T>
void array<T>::reserve(size_t new_capacity){
    if(new_capacity > capacity) BEEWAX_INTERNAL::array_increase_capacity_min(*this, new_capacity);
}

template<typename T>
void array<T>::clear(){
    size = 0u;
}

template<typename T>
typename array<T>::iterator array<T>::begin(){
    return data;
}

template<typename T>
typename array<T>::iterator array<T>::end(){
    return data + size;
}

template<typename T>
const typename array<T>::iterator array<T>::begin() const{
    return data;
}

template<typename T>
const typename array<T>::iterator array<T>::end() const{
    return data + size;
}

template<typename T>
void copy(array<T>* dest, array<T>* src){
    assert(dest != src);
    dest.reserve(src.size);
    memcpy(dest.data, src.data, src.size * sizeof(T));
}

// ---- hashmap

inline u32 hashmap_hash(const u32& key){
    return hash_xorshift(key);
}
inline u32 hashmap_hash(const s32& key){
    return hash_xorshift(*(u32*)&key);
}
inline u32 hashmap_hash(const char* str){
    return hash_FNV1a_32str(str);
}
inline u32 hashmap_hash(const char* str, size_t strlen){
    return hash_FNV1a_32ptr((u8*)str, strlen);
}
template<typename kT>
u32 hashmap_hash(const kT*& key){
    return hash_FNV1a_32ptr((u8*)key, sizeof(kT));
}

template<typename kT>
u32 hashmap_hash(const kT& key){
    return hash_FNV1a_32ptr((u8*)&key, sizeof(kT));
}

template<typename kT, typename vT>
void hashmap<kT, vT>::create(){
    // NOTE(hugo): kh_init without heap allocation
    memset(&data, 0x00, sizeof(data));
}

template<typename kT, typename vT>
void hashmap<kT, vT>::destroy(){
    // NOTE(hugo): kh_destroy without heap allocation
    if(data.keys)   kfree((void*)data.keys);
    if(data.flags)  kfree((void*)data.flags);
    if(data.vals)   kfree((void*)data.vals);
}

template<typename kT, typename vT>
size_t hashmap<kT, vT>::size(){
    return (size_t)data.size;
}

template<typename kT, typename vT>
size_t hashmap<kT, vT>::capacity(){
    return (size_t)data.nbuckets;
}

template<typename kT, typename vT>
u32 hashmap<kT, vT>::get(const kT& key, vT*& v){
    s32 kreturn;
    khiter_t iter = kh_put(kinstance, &data, key, &kreturn);
    assert(kreturn != -1);

    v = &kh_value(&data, iter);
    return kreturn > 0 ? 1u : 0u;
}

template<typename kT, typename vT>
u32 hashmap<kT, vT>::search(const kT& key, vT*& v) const{
    khiter_t iter = kh_get(kinstance, &data, key);
    if(iter != kh_end(&data)){
        v = &(kh_value(&data, iter));
        return 1u;
    }
    return 0u;
}

template<typename kT, typename vT>
u32 hashmap<kT, vT>::remove(const kT& key){
    khiter_t iter = kh_get(kinstance, &data, key);
    if(iter != kh_end(&data)){
        kh_del(kinstance, &data, iter);
        return 1u;
    }
    return 0u;
}

template<typename kT, typename vT>
u32 hashmap<kT, vT>::remove_func(const kT& key, void (*destroy_value)(vT& v)){
    khiter_t iter = kh_get(kinstance, &data, key);
    if(iter != kh_end(&data)){
        destroy_value(kh_value(&data, iter));
        kh_del(kinstance, &data, iter);
        return 1u;
    }
    return 0u;
}

template<typename kT, typename vT>
void hashmap<kT, vT>::reserve(size_t new_capacity){
    if(new_capacity > data.nbuckets) kh_resize(kinstance, &data, new_capacity);
}

template<typename kT, typename vT>
void hashmap<kT, vT>::clear(){
    kh_clear(kinstance, &data);
}

// -- iterator

template<typename kT, typename vT>
kT& hashmap<kT, vT>::iterator::key(){
    return kh_key(ptr, iter);
}

template<typename kT, typename vT>
const kT& hashmap<kT, vT>::iterator::key() const{
    return kh_key(ptr, iter);
}

template<typename kT, typename vT>
vT& hashmap<kT, vT>::iterator::value(){
    return kh_value(ptr, iter);
}

template<typename kT, typename vT>
const vT& hashmap<kT, vT>::iterator::value() const{
    return kh_value(ptr, iter);
}

template<typename kT, typename vT>
typename hashmap<kT, vT>::iterator& hashmap<kT, vT>::iterator::operator*(){
    return *this;
}

template<typename kT, typename vT>
const typename hashmap<kT, vT>::iterator& hashmap<kT, vT>::iterator::operator*() const{
    return *this;
}

template<typename kT, typename vT>
typename hashmap<kT, vT>::iterator& hashmap<kT, vT>::iterator::operator++(){
    while(iter != kh_end(ptr) && (++iter, !kh_exist(ptr, iter)));
    return *this;
}

template<typename kT, typename vT>
bool hashmap<kT, vT>::iterator::operator!=(const hashmap<kT, vT>::iterator& rhs) const{
    return iter != rhs.iter || ptr != rhs.ptr;
}

template<typename kT, typename vT>
typename hashmap<kT, vT>::iterator hashmap<kT, vT>::begin(){
    iterator iter;
    iter.ptr = &data;

    khiter_t kiter = kh_begin(&data);
    while(kiter != kh_end(&data) && !kh_exist(&data, kiter)) ++kiter;
    iter.iter = kiter;

    return iter;
}

template<typename kT, typename vT>
typename hashmap<kT, vT>::iterator hashmap<kT, vT>::end(){
    iterator iter;
    iter.ptr = &data;
    iter.iter = kh_end(&data);
    return iter;
}

template<typename kT, typename vT>
const typename hashmap<kT, vT>::iterator hashmap<kT, vT>::begin() const{
    iterator iter;
    iter.ptr = &data;

    khiter_t kiter = kh_begin(&data);
    while(kiter != kh_end(&data) && !kh_exist(&data, kiter)) ++kiter;
    iter.iter = kiter;

    return iter;
}

template<typename kT, typename vT>
const typename hashmap<kT, vT>::iterator hashmap<kT, vT>::end() const{
    iterator iter;
    iter.ptr = &data;
    iter.iter = kh_end(&data);
    return iter;
}
