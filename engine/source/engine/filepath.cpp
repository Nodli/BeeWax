constexpr char path_separator(){
#if defined(PLATFORM_WINDOWS)
    return '\\';
#elif defined(PLATFORM_LINUX)
    return '/';
#else
    static_assert(false, "path_separator is not implemented for this platform");
#endif
}

constexpr char path_separator_to_replace(){
#if defined(PLATFORM_WINDOWS)
    return '/';
#elif defined(PLATFORM_LINUX)
    return '\\';
#else
    static_assert(false, "path_separator_to_replace is not implemented for this platform");
#endif
}

void set_correct_path_separator(char* str){
    while(str = strchr(str, path_separator_to_replace())){
        *str = path_separator();
    }
}

File_Path asset_path(){
    return "./data";
}

void File_Path::extract_from(const char* iptr, u32 isize){
    assert(size + 1u < file_path_capacity);

    memcpy(data, iptr, isize);
    size = isize;
    data[size] = '\0';

    set_correct_path_separator(data);
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

    set_correct_path_separator(data);

    return *this;
}

File_Path& File_Path::operator/(const File_Path& rhs){
    assert((size + 1u + rhs.size + 1u) <= file_path_capacity);

    data[size] = path_separator();
    memcpy(data + size + 1u, rhs.data, rhs.size);
    size = size + 1u + rhs.size;
    data[size] = '\0';

    return *this;
}

File_Path& File_Path::operator/(const char* rhs){
    u32 rhs_size = strlen(rhs);
    assert((size + 1u + rhs_size + 1u) <= file_path_capacity);

    char* separator_search = data + size + 1u;

    data[size] = path_separator();
    memcpy(data + size + 1u, rhs, rhs_size);
    size = size + 1u + rhs_size;
    data[size] = '\0';

    set_correct_path_separator(separator_search);

    return *this;
}

bool File_Path::operator==(const File_Path& rhs) const{
    return (size == rhs.size) && (memcmp(data, rhs.data, size) == 0);
}

static u32 dhashmap_hash_key(const File_Path& key){
    return FNV1a_32ptr((uchar*)key.data, key.size);
}
