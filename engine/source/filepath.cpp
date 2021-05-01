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
    while(str = strchr(str, path_separator_to_replace()), str != nullptr){
        *str = path_separator();
    }
}

File_Path asset_path(){
    return "./data";
}

File_Path::File_Path() : data(""), size(0u) {}

File_Path::operator const char*() const{
    return data;
}

File_Path::operator char*(){
    return data;
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

void File_Path::extract_from(const char* iptr, u32 isize){
    assert(size + 1u < file_path_capacity);

    memcpy(data, iptr, isize);
    size = isize;
    data[size] = '\0';

    set_correct_path_separator(data);
}

bool operator==(const File_Path& lhs, const File_Path& rhs){
    return (lhs.size == rhs.size) && (memcmp(lhs.data, rhs.data, lhs.size) == 0);
}

bool operator==(const File_Path& lhs, const char* rhs){
    u32 rhs_size = strlen(rhs);
    return (lhs.size == rhs_size) && (memcmp(lhs.data, rhs, rhs_size) == 0u);
}

bool operator==(const char* lhs, const File_Path& rhs){
    return rhs == lhs;
}
