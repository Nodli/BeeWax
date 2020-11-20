#ifndef H_HASH
#define H_HASH

// REF(hugo): http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-param
// REF(hugo): https://softwareengineering.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed
u32 FNV1a_32ptr(u8* data, size_t bytesize);
u64 FNV1a_64ptr(u8* data, size_t bytesize);

// REF(hugo): https://probablydance.com/2018/06/16/fibonacci-hashing-the-optimization-that-the-world-forgot-or-a-better-alternative-to-integer-modulo/
u32 fibonacci_hash(u32 data);
u64 fibonacci_hash(u64 data);

// NOTE(hugo): /deadbeef_hash/ "isn't too bad, provided you promise to use at least the 17 lowest bits"
// REF(hugo): http://burtleburtle.net/bob/hash/integer.html
u32 wang_hash(u32 data);
u32 deadbeef_hash(u32 data);

// REF(hugo): https://nullprogram.com/blog/2018/07/31/
u32 xorshift_hash(u32 data);
u64 xorshift_hash(u64 data);

// REF(hugo): https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
u32 murmur3(u8* data, size_t bytesize, u32 seed);

template<u32 (*hash_function)(u32)>
u32 combined_hash_32(u32* data, u32 size);

template<u64 (*hash_function)(u64)>
u64 combined_hash_64(u64* data, u32 size);

// ----

template<u32 (*hash_function)(u32)>
u32 combined_hash_32(u32* data, u32 size){
    u32 hash = 0u;
    u32* begin = data;
    u32* end = data + size;
    while(begin < end){
        hash ^= hash_function(*begin++);
    }
    return hash;
}

template<u64 (*hash_function)(u64)>
u64 combined_hash_64(u64* data, u32 size){
    u64 hash = 0u;
    u64* begin = data;
    u64* end = data + size;
    while(begin < end){
        hash ^= hash_function(*begin++);
    }
    return hash;
}

#endif
