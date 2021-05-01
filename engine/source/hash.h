#ifndef H_HASH
#define H_HASH

// REF(hugo): http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-param
// REF(hugo): https://softwareengineering.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed
u32 hash_FNV1a_32ptr(const u8* data, const size_t bytesize);
u64 hash_FNV1a_64ptr(const u8* data, const size_t bytesize);

u32 hash_FNV1a_32str(const char* data);
u64 hash_FNV1a_64str(const char* data);

// REF(hugo): https://probablydance.com/2018/06/16/fibonacci-hashing-the-optimization-that-the-world-forgot-or-a-better-alternative-to-integer-modulo/
u32 hash_fibonacci(const u32 data);
u64 hash_fibonacci(const u64 data);

// REF(hugo): http://burtleburtle.net/bob/hash/integer.html
// NOTE(hugo): deadbeef_hash "isn't too bad, provided you promise to use at least the 17 lowest bits"
u32 hash_wang(const u32 data);
u32 hash_deadbeef(const u32 data);

// REF(hugo): https://nullprogram.com/blog/2018/07/31/
u32 hash_xorshift(const u32 data);
u64 hash_xorshift(const u64 data);

// REF(hugo): https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
u32 hash_murmur3(const u8* data, const size_t bytesize, const u32 seed);

u32 hash_combine(const u32 current, const u32 to_append);

#endif
