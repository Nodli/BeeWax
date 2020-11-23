#ifndef H_FILE_UTILS
#define H_FILE_UTILS

buffer<u8> read_file(const File_Path& path); // NOTE(hugo): free required
char* read_file_cstring(const File_Path& path); // NOTE(hugo): free required

void write_file(const File_Path& path, const u8* data, size_t bytesize);

template<typename T>
T get_bytes(u8*& cursor);

template<typename T>
T get_bytes_LE(u8*& cursor);

template<typename T>
T get_bytes_BE(u8*& cursor);

#include "file.inl"

#endif
