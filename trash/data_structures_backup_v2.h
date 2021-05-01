#ifndef H_DATA_STRUCTURE
#define H_DATA_STRUCTURE

// REF(hugo):
// https://ourmachinery.com/post/data-structures-part-1-bulk-data/
// https://ourmachinery.com/post/data-structures-part-2-indices/
// https://ourmachinery.com/post/data-structures-part-3-arrays-of-arrays/
// https://ourmachinery.com/post/minimalist-container-library-in-c-part-1/
// https://ourmachinery.com/post/minimalist-container-library-in-c-part-2/
// http://bitsquid.blogspot.com/2011/09/managing-decoupling-part-4-id-lookup.html

// ---- buffer

template<typename T>
struct buffer{
    T& operator[](u32 index);
    const T& operator[](u32 index) const;

    T* data = 0u;
    u32 size = 0u;
};

// -------- storage types
// ---- contiguous

template<typename T>
struct contiguous_iterator{
    T& operator*();
    const T& operator*() const;

    contiguous_iterator<T>& operator++();
    bool operator!=(const contiguous_iterator<T>& rhs) const;

    // ---- data

    T* ptr = nullptr;
};

template<typename T>
struct contiguous_storage{
    T& operator[](u32 index);
    const T& operator[](u32 index) const;

    void increase_capacity();
    void increase_capacity_min(u32 min_capacity);
    void free();

    void copy(u32 dest, u32 src, u32 size);
    void move(u32 dest, u32 src, u32 size);
    void zero(u32 start, u32 size);

    // ---- iterator

    contiguous_iterator<T> begin();
    contiguous_iterator<T> end();
    contiguous_iterator<T> iterator(u32 index);

    // ---- data

    u32 capacity = 0u;
    T* data = nullptr;
};

template<typename T>
contiguous_storage<T> deep_copy(contiguous_storage<T> to_copy, u32 copy_size);

// ---- bucketized

template<typename T, u32 bucket_size>
struct bucketized_iterator{
    T& operator*();
    const T& operator*() const;

    bucketized_iterator<T, bucket_size>& operator++();
    bool operator!=(const bucketized_iterator<T, bucket_size>& rhs) const;

    // ---- data

    T** bucket_array = nullptr;
    u32 index = 0u;
    u32 subindex = 0u;
};

template<typename T, u32 bucket_size>
struct bucketized_storage{
    T& operator[](u32 index);
    const T& operator[](u32 index) const;

    void increase_capacity();
    void increase_capacity_min(u32 min_capacity);
    void free();

    void copy(u32 dest, u32 src, u32 size);
    void move(u32 dest, u32 src, u32 size);
    void zero(u32 start, u32 size);

    // ---- iterator

    bucketized_iterator<T, bucket_size> begin();
    bucketized_iterator<T, bucket_size> end();
    bucketized_iterator<T, bucket_size> iterator(u32 index);

    // ---- data

    u32 capacity = 0u;
    T** bucket_array = nullptr;
};

template<typename T>
using bucketized_storage_32 = bucketized_storage<T, 32u>;

// ---- static

template<typename T, u32 static_capacity>
struct static_storage{
    T& operator[](u32 index);
    const T& operator[](u32 index) const;

    void increase_capacity();
    void increase_capacity_min(u32 min_capacity);
    void free();

    void copy(u32 dest, u32 src, u32 size);
    void move(u32 dest, u32 src, u32 size);
    void zero(u32 start, u32 size);

    // ---- iterator

    contiguous_iterator<T> begin();
    contiguous_iterator<T> end();
    contiguous_iterator<T> iterator(u32 index);

    // ---- data

    constexpr static u32 capacity = static_capacity;
    T data[static_capacity];
};

// -------- containers
// ---- array

template<typename T, typename Storage_Type = contiguous_storage<T>>
struct array{
    T& operator[](u32 index);
    const T& operator[](u32 index) const;

    void insert_empty(u32 index);
    void insert(u32 index, const T& value);
    void insert_multi(u32 index, u32 nelement);
    void remove(u32 index);
    void remove_multi(u32 index, u32 nelement);

    // NOTE(hugo): removes the /index/th element and replaces it with the one at the end of the array
    void remove_swap(u32 index);

    u32 push_empty();
    u32 push(const T& value);
    void pop();

    void set_size(u32 new_size);
    void set_min_capacity(u32 new_capacity);

    void clear();
    void free();

    // ---- iterator

    auto begin();
    auto end();

    // ---- data

    Storage_Type storage;
    u32 size = 0u;
};

template<typename T, typename Storage_Type>
array<T, Storage_Type> deep_copy(array<T, Storage_Type> to_copy);

// ---- pool
// - stable identifiers
// - access, insert & remove are o(1)
// - non-iterable

// NOTE(hugo): use ptr because
// * pointers are faster than indices because there is no index-to-ptr phase (non-negligible cost for some Storage_Type)
// * pointer translation may be required when extending BUT extensions happend when there is no free bucket
//   ie no translation is required for the general use case

constexpr u32 freelist_no_bucket = UINT32_MAX;

template<typename T>
union Pool_Bucket{
    T type;
    u32 next;
};

template<typename T, typename Storage_Type = bucketized_storage_32<Pool_Bucket<T>>>
struct pool{
    T& operator[](u32 identifier);
    const T& operator[](u32 identifier) const;

    u32 insert_empty();
    u32 insert(const T& value);
    void remove(u32 identifier);

    void set_min_capacity(u32 new_capacity);

    void clear();
    void free();

    // ---- data

    Storage_Type storage;
    u32 size = 0u;
    u32 head_bucket = freelist_no_bucket;
};

// ---- hashmap
// - user-provided identifiers
// - get, search & remove are o(1)
// - iterable

template<typename Key_Type>
u32 hashmap_hash_key(const Key_Type& key);
u32 hashmap_hash_key(const u32& key);
u32 hashmap_hash_key(const s32& key);

// REF(hugo):
// https://github.com/attractivechaos/klib/blob/master/khash.h
// https://attractivechaos.wordpress.com/2018/01/13/revisiting-hash-table-performance/

#define kfree(P) ::free(P)
#include "khash.h"

// TODO(hugo): get and search should return references

template<typename Key_Type, typename Value_Type>
struct hashmap{
    Value_Type* get(const Key_Type& key, bool& was_created);
    Value_Type* search(const Key_Type& key);
    void remove(const Key_Type& key);

    void set_min_capacity(u32 min_capacity);

    void clear();
    void free();

    // ---- iterator

    auto begin();
    auto end();

    // ---- khash

    #define internal_hash_function(key) hashmap_hash_key(key)
    #define internal_hash_equal(hashA, hashB) ((hashA) == (hashB))
    KHASH_INIT(internal, Key_Type, Value_Type, 1, internal_hash_function, internal_hash_equal)
    #undef internal_hash_function
    #undef internal_hash_equal

    khash_t(internal) storage = {};

    struct khash_iterator{
        khash_iterator& operator*();
        const khash_iterator& operator*() const;
        khash_iterator& operator++();
        bool operator!=(const khash_iterator& rhs) const;

        Key_Type& key();
        const Key_Type& key() const;

        Value_Type& value();
        const Value_Type& value() const;

        // ---- data

        khash_t(internal)* storage;
        khiter_t iterator;
    };
};

#include "data_structure.inl"

#endif

// ---- buffer

template<typename T>
const T& buffer<T>::operator[](u32 index) const{
    assert(index < size);
    return data[index];
}

template<typename T>
T& buffer<T>::operator[](u32 index){
    return const_cast<T&>(static_cast<const buffer<T>&>(*this)[index]);
}

// -------- storage types
// ---- contiguous

template<typename T>
const T& contiguous_iterator<T>::operator*() const{
    return *ptr;
}

template<typename T>
T& contiguous_iterator<T>::operator*(){
    return const_cast<T&>(static_cast<const contiguous_iterator<T>&>(*this).operator*());
}

template<typename T>
contiguous_iterator<T>& contiguous_iterator<T>::operator++(){
    ++ptr;
    return *this;
}

template<typename T>
bool contiguous_iterator<T>::operator!=(const contiguous_iterator<T>& rhs) const{
    return ptr != rhs.ptr;
}

template<typename T>
const T& contiguous_storage<T>::operator[](u32 index) const{
    assert(index < capacity);
    return data[index];
}

template<typename T>
T& contiguous_storage<T>::operator[](u32 index){
    return const_cast<T&>(static_cast<const contiguous_storage<T>&>(*this)[index]);
}

template<typename T>
static void reallocate_to_capacity(contiguous_storage<T>* storage, u32 new_capacity){
    assert(new_capacity > storage->capacity);

    void* new_data = ::realloc((void*)storage->data, (size_t)new_capacity * sizeof(T));
    assert(new_data);

    storage->data = (T*)new_data;
    storage->capacity = new_capacity;
}

template<typename T>
void contiguous_storage<T>::increase_capacity(){
    u32 new_capacity = max(32u, round_up_pow2(capacity) * 2u);
    return reallocate_to_capacity(this, new_capacity);
}

template<typename T>
void contiguous_storage<T>::increase_capacity_min(u32 min_capacity){
    return reallocate_to_capacity(this, min_capacity);
}

template<typename T>
void contiguous_storage<T>::free(){
    ::free(data);
    *this = contiguous_storage<T>();
}

template<typename T>
void contiguous_storage<T>::copy(u32 dest, u32 src, u32 size){
    assert((dest + size) <= capacity
        && (src + size) <= capacity
        && (((dest + size) <= src) | ((src + size) <= dest)));
    memcpy(data + dest, data + src, size * sizeof(T));
}

template<typename T>
void contiguous_storage<T>::move(u32 dest, u32 src, u32 size){
    assert((dest + size) <= capacity && (src + size) <= capacity);
    memmove(data + dest, data + src, size * sizeof(T));
}

template<typename T>
void contiguous_storage<T>::zero(u32 start, u32 size){
    assert((start + size) <= capacity);
    memset(data + start, 0u, size * sizeof(T));
}

template<typename T>
contiguous_iterator<T> contiguous_storage<T>::begin(){
    contiguous_iterator<T> iter;
    iter.ptr = data;
    return iter;
}

template<typename T>
contiguous_iterator<T> contiguous_storage<T>::end(){
    contiguous_iterator<T> iter;
    iter.ptr = data + capacity;
    return iter;
}

template<typename T>
contiguous_iterator<T> contiguous_storage<T>::iterator(u32 index){
    contiguous_iterator<T> iter;
    iter.ptr = data + index;
    return iter;
}

template<typename T>
contiguous_storage<T> deep_copy(contiguous_storage<T> to_copy, u32 copy_size){
    assert(!(copy_size > to_copy.capacity));

    contiguous_storage<T> copy;
    reallocate_to_capacity(&copy, to_copy.capacity);
    memcpy(copy.data, to_copy.data, copy_size * sizeof(T));

    return copy;
}

// ---- bucketized

template<typename T, u32 bucket_size>
const T& bucketized_iterator<T, bucket_size>::operator*() const{
    return bucket_array[index][subindex];
}

template<typename T, u32 bucket_size>
T& bucketized_iterator<T, bucket_size>::operator*(){
    return const_cast<T&>(static_cast<const bucketized_iterator<T, bucket_size>&>(*this).operator*());
}

template<typename T, u32 bucket_size>
bucketized_iterator<T, bucket_size>& bucketized_iterator<T, bucket_size>::operator++(){
    ++subindex;
    if(subindex == bucket_size){
        ++index;
        subindex = 0u;
    }
    return *this;
}

template<typename T, u32 bucket_size>
bool bucketized_iterator<T, bucket_size>::operator!=(const bucketized_iterator<T, bucket_size>& rhs) const{
    return index != rhs.index || subindex != rhs.subindex;
}

template<typename T, u32 bucket_size>
const T& bucketized_storage<T, bucket_size>::operator[](u32 index) const{
    assert(index < capacity);
    u32 bucket_index = index / bucket_size;
    u32 bucket_subindex = index - bucket_index * bucket_size;
    return bucket_array[bucket_index][bucket_subindex];
}

template<typename T, u32 bucket_size>
T& bucketized_storage<T, bucket_size>::operator[](u32 index){
    return const_cast<T&>(static_cast<const bucketized_storage<T, bucket_size>&>(*this)[index]);
}

template<typename T, u32 bucket_size>
void bucketized_storage<T, bucket_size>::increase_capacity(){
    u32 bucket_array_size = capacity / bucket_size;
    u32 new_bucket_array_size = bucket_array_size + 1u;

    void* new_bucket_array = realloc(bucket_array, new_bucket_array_size * sizeof(bucket_array));
    assert(new_bucket_array);
    bucket_array = (T**)new_bucket_array;

    void* new_bucket = malloc(sizeof(T) * bucket_size);
    assert(new_bucket);
    bucket_array[bucket_array_size] = (T*)new_bucket;

    capacity += bucket_size;
}

template<typename T, u32 bucket_size>
void bucketized_storage<T, bucket_size>::increase_capacity_min(u32 min_capacity){
    u32 bucket_array_size = capacity / bucket_size;
    u32 min_bucket_array_size = min_capacity / bucket_size + ((min_capacity % bucket_size) != 0u);
    assert(min_bucket_array_size > bucket_array_size);

    void* new_bucket_array = realloc(bucket_array, min_bucket_array_size * sizeof(bucket_array));
    assert(new_bucket_array);
    bucket_array = (T**)new_bucket_array;

    for(u32 ibucket = bucket_array_size; ibucket < min_bucket_array_size; ++ibucket){
        void* new_bucket = malloc(sizeof(T) * bucket_size);
        assert(new_bucket);
        bucket_array[ibucket] = (T*)new_bucket;
    }

    capacity = min_bucket_array_size * bucket_size;
}

template<typename T, u32 bucket_size>
void bucketized_storage<T, bucket_size>::free(){
    u32 bucket_array_size = capacity / bucket_size;
    for(u32 ibucket = 0u; ibucket != bucket_array_size; ++ibucket){
        ::free(bucket_array[ibucket]);
    }
    ::free(bucket_array);

    *this = bucketized_storage<T, bucket_size>();
}

template<typename T, u32 bucket_size>
void bucketized_storage<T, bucket_size>::copy(u32 dest, u32 src, u32 size){
    assert((dest + size) <= capacity && (src + size) <= capacity);
    for(u32 index = 0u; index != size; ++index){
        operator[](dest + index) = operator[](src + index);
    }
}

template<typename T, u32 bucket_size>
void bucketized_storage<T, bucket_size>::move(u32 dest, u32 src, u32 size){
    copy(dest, src, size);
}

template<typename T, u32 bucket_size>
void bucketized_storage<T, bucket_size>::zero(u32 start, u32 size){
    assert((start + size) <= capacity);

    for(u32 index = 0u; index != size; ++index){
        memset(&operator[](start + index), 0u, sizeof(T));
    }
}

template<typename T, u32 bucket_size>
bucketized_iterator<T, bucket_size> bucketized_storage<T, bucket_size>::begin(){
    bucketized_iterator<T, bucket_size> iter;
    iter.bucket_array = bucket_array;
    iter.index = 0u;
    iter.subindex = 0u;
    return iter;
}

template<typename T, u32 bucket_size>
bucketized_iterator<T, bucket_size> bucketized_storage<T, bucket_size>::end(){
    bucketized_iterator<T, bucket_size> iter;
    iter.index = capacity / bucket_size;
    iter.subindex = 0u;
    return iter;
}

template<typename T, u32 bucket_size>
bucketized_iterator<T, bucket_size> bucketized_storage<T, bucket_size>::iterator(u32 index){
    bucketized_iterator<T, bucket_size> iter;
    iter.index = index / bucket_size;
    iter.subindex = index % bucket_size;
    return iter;
}

// ---- static

template<typename T, u32 static_capacity>
const T& static_storage<T, static_capacity>::operator[](u32 index) const{
    assert(index < capacity);
    return data[index];
}

template<typename T, u32 static_capacity>
T& static_storage<T, static_capacity>::operator[](u32 index){
    return const_cast<T&>(static_cast<const static_storage<T, static_capacity>&>(*this)[index]);
}

template<typename T, u32 static_capacity>
void static_storage<T, static_capacity>::increase_capacity(){
    ENGINE_CHECK(false, "static_storage::increase_capacity()");
}

template<typename T, u32 static_capacity>
void static_storage<T, static_capacity>::increase_capacity_min(u32 min_capacity){
    ENGINE_CHECK(false, "static_storage::increase_capacity()");
}

template<typename T, u32 static_capacity>
void static_storage<T, static_capacity>::free(){
    *this = static_storage<T, static_capacity>();
}

template<typename T, u32 static_capacity>
void static_storage<T, static_capacity>::copy(u32 dest, u32 src, u32 size){
    assert((dest + size) <= capacity
        && (src + size) <= capacity
        && (((dest + size) <= src) | ((src + size) <= dest)));
    memcpy(data + dest, data + src, size * sizeof(T));
}

template<typename T, u32 static_capacity>
void static_storage<T, static_capacity>::move(u32 dest, u32 src, u32 size){
    assert((dest + size) <= capacity && (src + size) <= capacity);
    memmove(data + dest, data + src, size * sizeof(T));
}

template<typename T, u32 static_capacity>
void static_storage<T, static_capacity>::zero(u32 start, u32 size){
    assert((start + size) <= capacity);
    memset(data + start, 0u, size * sizeof(T));
}

template<typename T, u32 static_capacity>
contiguous_iterator<T> static_storage<T, static_capacity>::begin(){
    contiguous_iterator<T> iter;
    iter.ptr = data;
    return iter;
}

template<typename T, u32 static_capacity>
contiguous_iterator<T> static_storage<T, static_capacity>::end(){
    contiguous_iterator<T> iter;
    iter.ptr = data + capacity;
    return iter;
}

template<typename T, u32 static_capacity>
contiguous_iterator<T> static_storage<T, static_capacity>::iterator(u32 index){
    contiguous_iterator<T> iter;
    iter.ptr = data + index;
    return iter;
}

// -------- containers
// ---- array

template<typename T, typename Storage_Type>
const T& array<T, Storage_Type>::operator[](u32 index) const{
    assert(index < size);
    return storage[index];
}

template<typename T, typename Storage_Type>
T& array<T, Storage_Type>::operator[](u32 index){
    return const_cast<T&>(static_cast<const array<T, Storage_Type>&>(*this)[index]);
}

template<typename T, typename Storage_Type>
void array<T, Storage_Type>::insert_empty(u32 index){
    if(size == storage.capacity){
        storage.increase_capacity();
    }

    if(index < size){
        storage.move(index + 1u, index, size - index);
    }
    new((void*)&storage[index]) T{};
    ++size;
}

template<typename T, typename Storage_Type>
void array<T, Storage_Type>::insert(u32 index, const T& value){
    insert_empty(index);
    storage[index] = value;
}

template<typename T, typename Storage_Type>
void array<T, Storage_Type>::insert_multi(u32 index, u32 nelement){
    if(size + nelement > storage.capacity){
        storage.increase_capacity_min(size + nelement);
    }

    if(index < size){
        storage.move(index + nelement, index, size - index);
    }
    for(u32 ielement = 0u; ielement != nelement; ++ielement){
        new((void*)&storage[index + ielement]) T{};
    }
    size += nelement;
}

template<typename T, typename Storage_Type>
void array<T, Storage_Type>::remove(u32 index){
    assert(index < size);

    if(index < size - 1u){
        storage.move(index, index + 1u, size - index - 1u);
    }
    --size;
}

template<typename T, typename Storage_Type>
void array<T, Storage_Type>::remove_multi(u32 index, u32 nelement){
    assert(index + nelement <= size);

    if(index + nelement < size){
        storage.move(index, index + nelement, size - index + nelement);
    }
    size -= nelement;
}

template<typename T, typename Storage_Type>
void array<T, Storage_Type>::remove_swap(u32 index){
    assert(index < size);

    --size;
    storage[index] = storage[size];
}

template<typename T, typename Storage_Type>
u32 array<T, Storage_Type>::push_empty(){
    if(size == storage.capacity){
        storage.increase_capacity();
    }

    u32 index = size;
    new((void*)&storage[index]) T{};
    ++size;

    return index;
}

template<typename T, typename Storage_Type>
u32 array<T, Storage_Type>::push(const T& value){
    u32 index = size;
    push_empty();
    storage[index] = value;
    return index;
}

template<typename T, typename Storage_Type>
void array<T, Storage_Type>::pop(){
    assert(size);
    --size;
}

template<typename T, typename Storage_Type>
void array<T, Storage_Type>::set_size(u32 new_size){
    if(new_size > storage.capacity){
        storage.increase_capacity_min(new_size);
    }
    for(u32 index = size; index != new_size; ++index){
        new((void*)&storage[index]) T{};
    }
    size = new_size;
}

template<typename T, typename Storage_Type>
void array<T, Storage_Type>::set_min_capacity(u32 new_capacity){
    if(new_capacity > storage.capacity){
        storage.increase_capacity_min(new_capacity);
    }
}

template<typename T, typename Storage_Type>
void array<T, Storage_Type>::clear(){
    size = 0u;
}

template<typename T, typename Storage_Type>
void array<T, Storage_Type>::free(){
    storage.free();
    *this = array<T, Storage_Type>();
}

template<typename T, typename Storage_Type>
auto array<T, Storage_Type>::begin(){
    return storage.begin();
}

template<typename T, typename Storage_Type>
auto array<T, Storage_Type>::end(){
    return storage.iterator(size);
}

template<typename T, typename Storage_Type>
array<T, Storage_Type> deep_copy(array<T, Storage_Type> to_copy){
    array<T, Storage_Type> copy;
    copy.size = to_copy.size;
    copy.storage = deep_copy(to_copy.storage, to_copy.size);
    return copy;
}

// ---- pool

template<typename T, typename Storage_Type>
const T& pool<T, Storage_Type>::operator[](u32 index) const{
    return storage.operator[](index).type;
}

template<typename T, typename Storage_Type>
T& pool<T, Storage_Type>::operator[](u32 index){
    return const_cast<T&>(static_cast<const pool<T, Storage_Type>&>(*this)[index]);
}

template<typename T, typename Storage_Type>
u32 pool<T, Storage_Type>::insert_empty(){
    if(head_bucket == freelist_no_bucket){
        u32 previous_capacity = storage.capacity;
        storage.increase_capacity();

        head_bucket = previous_capacity;
        for(u32 index = previous_capacity; index < storage.capacity - 1u; ++index){
            storage[index].next = index + 1u;
        }
        storage[storage.capacity - 1u].next = freelist_no_bucket;
    }

    u32 index = head_bucket;
    head_bucket = storage[index].next;
    new((void*)&storage[index].type) T{};
    ++size;
    return index;
}

template<typename T, typename Storage_Type>
u32 pool<T, Storage_Type>::insert(const T& value){
    u32 index = insert_empty();
    storage[index].type = value;
    return index;
}

template<typename T, typename Storage_Type>
void pool<T, Storage_Type>::remove(u32 index){
    assert(index < storage.capacity);

    storage[index].next = head_bucket;
    head_bucket = index;
    --size;
}

template<typename T, typename Storage_Type>
void pool<T, Storage_Type>::set_min_capacity(u32 new_capacity){
    if(new_capacity > storage.capacity){
        u32 previous_capacity = storage.capacity;
        storage.increase_capacity_min(new_capacity);

        for(u32 index = previous_capacity; index < storage.capacity - 1u; ++index){
            storage[index].next = index + 1u;
        }
        storage[storage.capacity - 1u].next = head_bucket;
        head_bucket = previous_capacity;
    }
}

template<typename T, typename Storage_Type>
void pool<T, Storage_Type>::clear(){
    if(storage.capacity > 0u){
        size = 0u;
        head_bucket = 0u;

        for(u32 index = 0u; index < storage.capacity - 1u; ++index){
            storage[index].next = index + 1u;
        }
        storage[storage.capacity - 1u].next = freelist_no_bucket;
    }
}

template<typename T, typename Storage_Type>
void pool<T, Storage_Type>::free(){
    storage.free();
    *this = pool<T, Storage_Type>();
}

// ---- hashmap

template<typename K>
u32 hashmap_hash_key(const K& key){
    return FNV1a_32ptr((uchar*)&key, sizeof(K));
}
u32 hashmap_hash_key(const u32& key){
    return xorshift_hash(key);
}
u32 hashmap_hash_key(const s32& key){
    return xorshift_hash(*(u32*)&key);
}

template<typename Key_Type, typename Value_Type>
Value_Type* hashmap<Key_Type, Value_Type>::get(const Key_Type& key, bool& was_created){
    s32 return_code;
    khiter_t iter = kh_put(internal, &storage, key, &return_code);
    assert(return_code != -1);

    if(return_code > 0){
        was_created = true;
        Value_Type* output = &kh_value(&storage, iter);
        new((void*)output) Value_Type{};
        return output;

    }else{
        was_created = false;
        return &kh_value(&storage, iter);
    }
}

template<typename Key_Type, typename Value_Type>
Value_Type* hashmap<Key_Type, Value_Type>::search(const Key_Type& key){
    khiter_t iter = kh_get(internal, &storage, key);
    if(iter != kh_end(&storage)){
        return &kh_value(&storage, iter);
    }
    return nullptr;
}

template<typename Key_Type, typename Value_Type>
void hashmap<Key_Type, Value_Type>::remove(const Key_Type& key){
    khiter_t iter = kh_get(internal, &storage, key);
    kh_del(internal, &storage, iter);
}

template<typename Key_Type, typename Value_Type>
void hashmap<Key_Type, Value_Type>::set_min_capacity(u32 min_capacity){
    if(min_capacity > kh_n_buckets(&storage)){
        kh_resize(internal, &storage, min_capacity);
    }
}

template<typename Key_Type, typename Value_Type>
void hashmap<Key_Type, Value_Type>::clear(){
    kh_clear(internal, &storage);
}

template<typename Key_Type, typename Value_Type>
void hashmap<Key_Type, Value_Type>::free(){
    // NOTE(hugo): custom kh_destroy because /storage/ is stack-allocated instead of using kh_init
    if(storage.keys)   kfree((void*)storage.keys);
    if(storage.flags)  kfree((void*)storage.flags);
    if(storage.vals)   kfree((void*)storage.vals);

    *this = hashmap<Key_Type, Value_Type>();
}

template<typename Key_Type, typename Value_Type>
auto hashmap<Key_Type, Value_Type>::begin(){

    khiter_t temp = kh_begin(&storage);
    while(temp != kh_end(&storage) && !kh_exist(&storage, temp)){
        ++temp;
    }

    khash_iterator iter;
    iter.storage = &storage;
    iter.iterator = temp;

    return iter;
}

template<typename Key_Type, typename Value_Type>
auto hashmap<Key_Type, Value_Type>::end(){
    khash_iterator iter;
    iter.iterator = kh_end(&storage);
    return iter;
}

template<typename Key_Type, typename Value_Type>
typename hashmap<Key_Type, Value_Type>::khash_iterator& hashmap<Key_Type, Value_Type>::khash_iterator::operator*(){
    return const_cast<typename hashmap<Key_Type, Value_Type>::khash_iterator&>(static_cast<const khash_iterator&>(*this).operator*());
}

template<typename Key_Type, typename Value_Type>
const typename hashmap<Key_Type, Value_Type>::khash_iterator& hashmap<Key_Type, Value_Type>::khash_iterator::operator*() const{
    return *this;
}

template<typename Key_Type, typename Value_Type>
typename hashmap<Key_Type, Value_Type>::khash_iterator& hashmap<Key_Type, Value_Type>::khash_iterator::operator++(){
    while(iterator != kh_end(storage) && (++iterator, !kh_exist(storage, iterator)));
    return *this;
}

template<typename Key_Type, typename Value_Type>
bool hashmap<Key_Type, Value_Type>::khash_iterator::operator!=(const hashmap<Key_Type, Value_Type>::khash_iterator& rhs) const{
    return iterator != rhs.iterator;
}

template<typename Key_Type, typename Value_Type>
Key_Type& hashmap<Key_Type, Value_Type>::khash_iterator::key(){
    return kh_key(storage, iterator);
}
template<typename Key_Type, typename Value_Type>
const Key_Type& hashmap<Key_Type, Value_Type>::khash_iterator::key() const{
    return kh_key(storage, iterator);
}
template<typename Key_Type, typename Value_Type>
Value_Type& hashmap<Key_Type, Value_Type>::khash_iterator::value(){
    return kh_value(storage, iterator);
}
template<typename Key_Type, typename Value_Type>
const Value_Type& hashmap<Key_Type, Value_Type>::khash_iterator::value() const{
    return kh_value(storage, iterator);
}
