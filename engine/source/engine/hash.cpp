u32 FNV1a_32ptr(const u8* data, const size_t bytesize){
    constexpr u32 offset_basis = 2166136261;
    constexpr u32 FNV_prime = (u32(1) << 24) + (u32(1) << 8) + 0x93;

    u32 hash = offset_basis;
    const u8* begin = data;
    const u8* end = begin + bytesize;
    while(begin < end){
        hash ^= (u32)*begin++;
        hash *= FNV_prime;
    }

    return hash;
}

u64 FNV1a_64ptr(const u8* data, const size_t bytesize){
    constexpr u64 offset_basis = 14695981039346656037u;
    constexpr u64 FNV_prime = (u64(1) << 40) + (u64(1) << 8) + 0x93;

    u64 hash = offset_basis;
    const u8* begin = data;
    const u8* end = begin + bytesize;
    while(begin < end){
        hash ^= (u64)*begin++;
        hash *= FNV_prime;
    }

    return hash;
}

u32 FNV1a_32str(const char* data){
    constexpr u32 offset_basis = 2166136261;
    constexpr u32 FNV_prime = (u32(1) << 24) + (u32(1) << 8) + 0x93;

    u32 hash = offset_basis;
    while(*data){
        hash ^= (u32)*data++;
        hash *= FNV_prime;
    }
    return hash;
}

u64 FNV1a_64str(const char* data){
    constexpr u64 offset_basis = 14695981039346656037u;
    constexpr u64 FNV_prime = (u64(1) << 40) + (u64(1) << 8) + 0x93;

    u64 hash = offset_basis;
    while(*data){
        hash ^= (u64)*data++;
        hash *= FNV_prime;
    }

    return hash;
}

// NOTE(hugo): fibo is 2^output_bits / golden ratio rounded to the nearest integer that is not a multiple of two
u32 fibonacci_hash(const u32 data){
    constexpr u32 fibo32 = 2654435769u;
    return data * fibo32;
}

u64 fibonacci_hash(const u64 data){
    constexpr u64 fibo64 = 11400714819323198486u;
    return data * fibo64;
}

u32 wang_hash(const u32 data)
{
    u32 hash = data;
    hash = (hash + 0x7ed55d16) + (hash << 12);
    hash = (hash ^ 0xc761c23c) ^ (hash >> 19);
    hash = (hash + 0x165667b1) + (hash << 5);
    hash = (hash + 0xd3a2646c) ^ (hash << 9);
    hash = (hash + 0xfd7046c5) + (hash << 3);
    hash = (hash ^ 0xb55a4f09) ^ (hash >> 16);
    return hash;
}

u32 deadbeef_hash(const u32 data)
{
    u32 hash = data;
    hash = hash ^ (hash >> 4u);
    hash = (hash ^ 0xdeadbeef) + (hash << 5u);
    hash = hash ^ (hash >> 11u);
    return hash;
}

u32 xorshift_hash(const u32 data){
    u32 hash = data;
    hash ^= (hash << 13);
    hash ^= (hash >> 17);
    hash ^= (hash << 5);
    return hash;
}

u64 xorshift_hash(const u64 data){
    u32 hash = data;
    hash ^= (hash << 13);
    hash ^= (hash >> 7);
    hash ^= (hash << 17);
    return hash;
}

u32 murmur3(const u8* data, const size_t bytesize, const u32 seed){
    const u32 nblocks = bytesize / 4u;
    u32 hash = seed;
    constexpr u32 c1 = 0xcc9e2d51;
    constexpr u32 c2 = 0x1b873593;
    constexpr u32 c3 = 0xe6546b64;
    constexpr u32 c4 = 0x85ebca6b;
    constexpr u32 c5 = 0xc2b2ae35;

    // NOTE(hugo): 4-byte blocks
    for(u32 iblock = 0u; iblock != nblocks; ++iblock){
        u32 block = data[iblock];

        block *= c1;
        block = (block << 15u) | (block >> (32u - 15u));
        block *= c2;

        hash ^= block;
        hash = (hash << 13u) | (hash >> (32u - 13u));
        hash = hash * 5u + c3;
    }

    // NOTE(hugo): remaining bytes
    const u8* remaining_ptr = data + nblocks * 4u;
    u32 remainder = 0u;
    switch(bytesize & 3u){

        case 3:
            remainder ^= remaining_ptr[2u] << 16u;
            [[fallthrough]];
        case 2:
            remainder ^= remaining_ptr[1u] << 8u;
            [[fallthrough]];
        case 1:
            remainder ^= remaining_ptr[0u];
            remainder *= c1;
            remainder = (remainder << 15u) | (remainder >> (32u - 15u));
            remainder *= c2;

            hash ^= remainder;
    }

    // NOTE(hugo): finalization
    hash ^= (u32)bytesize;

    hash ^= hash >> 16u;
    hash *= c4;
    hash ^= hash >> 13u;
    hash *= c5;
    hash ^= hash >> 16u;

    return hash;
}
