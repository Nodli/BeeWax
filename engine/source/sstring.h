#ifndef H_SSTRING
#define H_SSTRING

// REF(hugo): https://www.youtube.com/watch?v=kPR8h4-qZdk

template<size_t bytesize = 64u>
struct sstring{
    static_assert(bytesize);

    /*
    sstring();

    // NOTE(hugo): const char* to sstring
    template<size_t cstr_bytesize>
    constexpr sstring(const char (&cstr)[str_bytesize]);
    */

    void extract_from(const char* str, size_t strlen);

    template<size_t str_bytesize>
    sstring& operator=(const sstring<str_bytesize>& str);
    sstring& operator=(const char* str);

    size_t strlen() const;
    static constexpr size_t strcap();

    // ---- data

    union{
        char data[bytesize];
        struct{
            char padding[bytesize - 1u];
            u8 empty_bytes;
        };
    };
};

template<size_t bytesizeL, size_t bytesizeR>
bool operator==(const sstring<bytesizeL>& L, const sstring<bytesizeR>& R);

template<size_t bytesize>
bool operator==(const sstring<bytesize>& sstr, const char* cstr);

template<size_t bytesize>
bool operator==(const char* cstr, const sstring<bytesize>& sstr);

template<size_t bytesizeL, size_t bytesizeR>
bool operator!=(const sstring<bytesizeL>& lhs, const sstring<bytesizeR>& rhs);

template<size_t bytesize>
bool operator!=(const sstring<bytesize>& lhs, const char* rhs);

template<size_t bytesize>
bool operator!=(const char* lhs, const sstring<bytesize>& rhs);

template<size_t bytesize>
u32 hashmap_hash(const sstring<bytesize>& str);

#include "sstring.inl"

#endif
