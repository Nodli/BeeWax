#ifndef H_SSTRING
#define H_SSTRING

template<u32 capacity>
struct sstring{
    template<u32 cstring_capacity>
    constexpr sstring(const char (&cstring)[cstring_capacity]);
    sstring(){};

    void extract_from(const char* iptr, u32 isize);

    template<u32 rhs_capacity>
    sstring& operator=(const sstring<rhs_capacity>& rhs);
    sstring& operator=(const char* rhs);

    template<u32 rhs_capacity>
    bool operator==(const sstring<rhs_capacity>& rhs) const;
    bool operator==(const char* rhs) const;

    // ---- data

    static_assert(capacity > 0u);

    char data[capacity] = {'\0'};
    u32 size = 0u;
};

template<u32 capacity>
u32 hashmap_hash_key(const sstring<capacity>& key);

#include "sstring.inl"

#endif
