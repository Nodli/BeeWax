#ifndef H_FILE
#define H_FILE

// ---- io

// NOTE(hugo): explicit free required
array<u8> read_file(const File_Path& path, const char* mode);
char* read_file_cstring(const File_Path& path);

void write_file(const File_Path& path, const u8* data, size_t bytesize);

// ---- packing / unpacking

template<typename T>
T get_bytes(u8*& cursor);

template<typename T>
T get_bytes_LE(u8*& cursor);

template<typename T>
T get_bytes_BE(u8*& cursor);

// ----

template<typename T>
T get_bytes(u8*& cursor){
    T output = *(T*)cursor;
    cursor += sizeof(T);
    return output;
}

template<typename T>
T get_bytes_LE(u8*& cursor){
    return get_bytes<T>(cursor);
}

template<typename T>
T get_bytes_BE(u8*& cursor){
    return atomic_byteswap<T>(get_bytes<T>(cursor));
}

#endif
