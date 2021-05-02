#ifndef H_SSTRING
#define H_SSTRING

template<u32 capacity = 60u>
struct sstring{
    static_assert(capacity > 0u);

    sstring();

    // NOTE(hugo): const char* to sstring
    template<u32 cstring_capacity>
    constexpr sstring(const char (&cstring)[cstring_capacity]);

    template<u32 rhs_capacity>
    sstring& operator=(const sstring<rhs_capacity>& rhs);
    sstring& operator=(const char* rhs);

    void extract_from(const char* iptr, u32 isize);

    // ---- data

    u32 size;
    char data[capacity];
};

template<u32 lhs_capacity, u32 rhs_capacity>
bool operator==(const sstring<lhs_capacity>& lhs, const sstring<rhs_capacity>& rhs);

template<u32 capacity>
bool operator==(const sstring<capacity>& lhs, const char* rhs);

template<u32 capacity>
bool operator==(const char* lhs, const sstring<capacity>& rhs);

template<u32 lhs_capacity, u32 rhs_capacity>
bool operator!=(const sstring<lhs_capacity>& lhs, const sstring<rhs_capacity>& rhs);

template<u32 capacity>
bool operator!=(const sstring<capacity>& lhs, const char* rhs);

template<u32 capacity>
bool operator!=(const char* lhs, const sstring<capacity>& rhs);

template<u32 capacity>
u32 hashmap_hash(const sstring<capacity>& str);

#include "sstring.inl"

#endif
