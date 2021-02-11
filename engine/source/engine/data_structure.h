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
