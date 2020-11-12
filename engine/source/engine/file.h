#ifndef H_FILE_UTILS
#define H_FILE_UTILS

char path_separator();

// NOTE(hugo): using File_Path instead of const char* as a function parameter
// outputs the same assembly even without optimizations (assembly checked gcc & msvc)
constexpr u32 file_path_capacity = 255;
struct File_Path{
    template<size_t isize>
    constexpr File_Path(char (&idata)[isize]);

    File_Path& operator=(File_Path& rhs);
    File_Path& operator=(const char* rhs);
    File_Path& operator/(File_Path& rhs);
    File_Path& operator/(const char* rhs);

    // ---- data

    char data[file_path_capacity] = {'\0'};
    u8 size = 0u;
};

buffer<u8> read_file(File_Path& path); // NOTE(hugo): free required
char* read_file_cstring(const char* const path); // NOTE(hugo): free required

void write_file(const char* const path, u8* data, size_t bytesize);

template<typename T>
T get_bytes(u8*& cursor);

template<typename T>
T get_bytes_LE(u8*& cursor);

template<typename T>
T get_bytes_BE(u8*& cursor);

#include "file.inl"

#endif
