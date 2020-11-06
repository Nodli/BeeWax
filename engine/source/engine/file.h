#ifndef H_FILE_UTILS
#define H_FILE_UTILS

char path_separator();

buffer<u8> read_file(const char* const path); // NOTE(hugo): free required
char* read_file_cstring(const char* const path); // NOTE(hugo): free required

void write_file(const char* const path, u8* data, size_t bytesize);

struct unpacker{
    template<typename T>
    T get_bytes();

    template<typename T>
    T get_bytes_LE();

    template<typename T>
    T get_bytes_BE();

    u8* cursor;
};

#include "file.inl"

#endif
