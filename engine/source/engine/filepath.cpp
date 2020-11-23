char path_separator(){
#if defined(PLATFORM_WINDOWS)
    return '\\';
#elif defined(PLATFORM_LINUX)
    return '/';
#else
    static_assert(false, "path_separator is not implemented for this platform");
#endif
}

File_Path& File_Path::operator=(const File_Path& rhs){
    memcpy(data, rhs.data, rhs.size);
    size = rhs.size;
    data[size] = '\0';
    return *this;
}

File_Path& File_Path::operator=(const char* rhs){
    u32 rhs_size = strlen(rhs);
    assert((rhs_size + 1u) <= file_path_capacity);
    memcpy(data, rhs, rhs_size);
    size = rhs_size;
    data[size] = '\0';
    return *this;
}

File_Path& File_Path::operator/(const File_Path& rhs){
    assert((size + rhs.size + 1u + 1u) <= file_path_capacity);
    data[size] = path_separator();
    memcpy(data + size + 1u, rhs.data, rhs.size);
    size += rhs.size;
    data[size] = '\0';
    return *this;
}

File_Path& File_Path::operator/(const char* rhs){
    u32 rhs_size = strlen(rhs);
    assert((size + rhs_size + 1u + 1u) <= file_path_capacity);
    data[size] = path_separator();
    memcpy(data + size + 1u, rhs, rhs_size);
    size += rhs_size;
    data[size] = '\0';
    return *this;
}

bool File_Path::operator==(const File_Path& rhs) const{
    return (size == rhs.size) && (memcmp(data, rhs.data, size) == 0);
}
