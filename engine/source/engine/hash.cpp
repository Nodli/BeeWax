u32 FNV1a_32ptr(u8* data, size_t bytesize){
    constexpr u32 offset_basis = 2166136261;
    constexpr u32 FNV_prime = (u32(1) << 24) + (u32(1) << 8) + 0x93;

    u32 hash = offset_basis;
    u8* begin = data;
    u8* end = begin + bytesize;
    while(begin < end){
        hash ^= (u32)*begin++;
        hash *= FNV_prime;
    }

    return hash;
}

u64 FNV1a_64ptr(u8* data, size_t bytesize){
    constexpr u64 offset_basis = 14695981039346656037u;
    constexpr u64 FNV_prime = (u64(1) << 40) + (u64(1) << 8) + 0x93;

    u64 hash = offset_basis;
    u8* begin = data;
    u8* end = begin + bytesize;
    while(begin < end){
        hash ^= (u64)*begin++;
        hash *= FNV_prime;
    }

    return hash;
}

// NOTE(hugo): fibo is 2^output_bits / golden ratio rounded to the nearest integer that is not a multiple of two
u32 fibonacci_hash(u32 data){
    constexpr u32 fibo32 = 2654435769u;
    return data * fibo32;
}

u64 fibonacci_hash(u64 data){
    constexpr u64 fibo64 = 11400714819323198486u;
    return data * fibo64;
}

u32 wang_hash(u32 data)
{
    data = (data + 0x7ed55d16) + (data << 12);
    data = (data ^ 0xc761c23c) ^ (data >> 19);
    data = (data + 0x165667b1) + (data << 5);
    data = (data + 0xd3a2646c) ^ (data << 9);
    data = (data + 0xfd7046c5) + (data << 3);
    data = (data ^ 0xb55a4f09) ^ (data >> 16);
    return data;
}

u32 deadbeef_hash(u32 data)
{
    data = data ^ (data >> 4u);
    data = (data ^ 0xdeadbeef) + (data << 5u);
    data = data ^ (data >> 11u);
    return data;
}

u32 combined_wang_hash(u32* data, size_t size){
    u32 hash = 0;
    u32* begin = data;
    u32* end = begin + size;
    while(begin < end){
        hash ^= (u32)*begin++;
        hash = wang_hash(hash);
    }

    return hash;
}

u32 xorshift_hash(u32 data){
    data ^= (data << 13);
    data ^= (data >> 17);
    data ^= (data << 5);
    return data;
}

u64 xorshift_hash(u64 data){
    data ^= (data << 13);
    data ^= (data >> 7);
    data ^= (data << 17);
    return data;
}
