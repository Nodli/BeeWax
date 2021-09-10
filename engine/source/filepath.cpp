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

void File_Path::extract_from(const char* str, size_t strlen){
    assert(strlen < bytesize);

    memcpy(data, str, strlen);
    data[strlen + 1u] = '\0';
    empty_bytes = bytesize - strlen - 1u;

    set_correct_path_separator(data);
}

File_Path& File_Path::operator=(const File_Path& path){
    memcpy(data, path.data, path.size() + 1u);
    empty_bytes = path.empty_bytes;

    return *this;
}

File_Path& File_Path::operator=(const char* str){
    size_t str_size = strlen(str);
    assert(str_size < bytesize);

    memcpy(data, str, str_size + 1u);
    empty_bytes = bytesize - str_size - 1u;

    set_correct_path_separator(data);

    return *this;
}

File_Path& File_Path::operator/=(const File_Path& path){
    size_t this_size = size();
    size_t path_size = path.size();
    size_t new_size = this_size + 1u + path_size;

    assert(new_size < bytesize);

    data[this_size] = path_separator();
    memcpy(data + this_size + 1u, path.data, path_size + 1u);
    empty_bytes = bytesize - new_size - 1u;

    return *this;
}

File_Path& File_Path::operator/=(const char* str){
    size_t this_size = size();
    size_t str_size = strlen(str);
    size_t new_size = this_size + 1u + str_size;
    assert(new_size < bytesize);

    data[this_size] = path_separator();
    memcpy(data + this_size + 1u, str, str_size + 1u);
    empty_bytes = bytesize - new_size - 1u;

    set_correct_path_separator(data + this_size + 1u);

    return *this;
}

size_t File_Path::size() const{
    return max(bytesize - empty_bytes, (size_t) 1u) - 1u;
}

constexpr size_t File_Path::capacity(){
    return bytesize - 1u;
}

bool operator==(const File_Path& L, const File_Path& R){
    return (L.empty_bytes == R.empty_bytes) && (memcmp(L.data, R.data, L.size()) == 0);
}

bool operator==(const File_Path& path, const char* str){
    size_t path_size = path.size();
    size_t str_size = strlen(str);
    return (path_size == str_size) && (memcmp(path.data, str, path_size) == 0);
}

bool operator==(const char* str, const File_Path& path){
    return path == str;
}

bool operator!=(const File_Path& L, const File_Path& R){
    return !(L == R);
}

bool operator!=(const File_Path& path, const char* str){
    return !(path == str);
}

bool operator!=(const char* str, const File_Path& path){
    return !(str == path);
}

u32 hashmap_hash(const File_Path& path){
    return hashmap_hash(path.data, path.size());
}
