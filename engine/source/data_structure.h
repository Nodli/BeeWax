#ifndef H_DATA_STRUCTURE
#define H_DATA_STRUCTURE

// REF(hugo):
// https://ourmachinery.com/post/data-structures-part-1-bulk-data/
// https://ourmachinery.com/post/data-structures-part-2-indices/
// https://ourmachinery.com/post/data-structures-part-3-arrays-of-arrays/
// https://ourmachinery.com/post/minimalist-container-library-in-c-part-1/
// https://ourmachinery.com/post/minimalist-container-library-in-c-part-2/
// http://bitsquid.blogspot.com/2011/09/managing-decoupling-part-4-id-lookup.html

// ---- array

template<typename T>
struct array{
    void create();
    void destroy();

    const T& operator[](size_t index) const;
    T& operator[](size_t index);

    T& push(const T& v);
    T pop();
    T& insert(size_t index, const T& v);
    T* insert_multi(size_t index, size_t count);
    void remove(size_t index);
    void remove_multi(size_t index, size_t count);
    void remove_swap(size_t index);

    void resize(size_t new_size);
    void reserve(size_t new_capacity);
    void clear();

    // ---- iterator

    typedef T* iterator;

    iterator begin();
    iterator end();
    const iterator begin() const;
    const iterator end() const;

    // ---- data

    T* data;
    size_t size;
    size_t capacity;
};

template<typename T>
void copy(array<T>* dest, array<T>* src);

// ---- hashmap
// - user-provided identifiers
// - get, search & remove are o(1)
// - iterable
//

// -- hashmap::get()
//  kT key;
//  vT* vptr;
//  if(hashmap.get(key, vptr)){
//      key was not in hashmap ; initialize vptr
//  }
//  key is in hashmap ; use vptr

// -- hashmap::search()
//  kT key;
//  vT* vptr;
//  if(hashmap.search(key, vptr)){
//      key was in hashmap ; use vptr
//  }

// -- hashmap::remove()
//  kT key;
//  if(hashmap.remove(key)){
//      key was removed
//  }
//  key was not in hashmap

// REF(hugo):
// https://github.com/attractivechaos/klib/blob/master/khash.h
// https://attractivechaos.wordpress.com/2018/01/13/revisiting-hash-table-performance/

inline u32 hashmap_hash(const u32& key);
inline u32 hashmap_hash(const s32& key);
inline u32 hashmap_hash(const char* str);

template<typename kT>
u32 hashmap_hash(const kT*& key);
template<typename kT>
u32 hashmap_hash(const kT& key);

#define kcalloc(count, bytesize)    bw_calloc(count, bytesize)
#define kmalloc(bytesize)           bw_malloc(bytesize)
#define krealloc(pointer, bytesize) bw_realloc(pointer, bytesize)
#define kfree(ptr)                  bw_free(ptr)
#include "khash.h"

template<typename kT, typename vT>
struct hashmap{
    // ---- khash

    #define khash_hash_key(key)                 (hashmap_hash(key))
    #define khash_hash_compare(hashA, hashB)    ((hashA) == (hashB))
    KHASH_INIT(kinstance, kT, vT, 1, khash_hash_key, khash_hash_compare)
    #undef khash_hash_key
    #undef khash_hash_compare

    // ----

    void create();
    void destroy();

    size_t size();
    size_t capacity();

    u32 get(const kT& key, vT*& v);
    u32 search(const kT& key, vT*& v) const;
    u32 remove(const kT& key);
    u32 remove_func(const kT& key, void (*destroy_value)(vT& v));

    void reserve(size_t new_capacity);
    void clear();

    // -- iterator

    struct iterator;
    struct iterator{
        kT& key();
        const kT& key() const;

        vT& value();
        const vT& value() const;

        iterator& operator*();
        const iterator& operator*() const;

        iterator& operator++();
        bool operator!=(const iterator& iter) const;

        // ---- data

        khash_t(kinstance)* ptr;
        khiter_t iter;
    };

    iterator begin();
    iterator end();
    const iterator begin() const;
    const iterator end() const;

    // ---- data

    khash_t(kinstance) data;
};

#include "data_structure.inl"

#undef kfree

#endif
