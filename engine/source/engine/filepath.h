#ifndef H_FILEPATH
#define H_FILEPATH

char path_separator();

// NOTE(hugo): using File_Path instead of const char* as a function parameter
// outputs the same assembly even without optimizations (assembly checked gcc & msvc)
constexpr u32 file_path_capacity = 255;
struct File_Path{
    template<size_t isize>
    constexpr File_Path(const char (&idata)[isize]);
    File_Path(){};

    File_Path& operator=(const File_Path& rhs);
    File_Path& operator=(const char* rhs);
    File_Path& operator/(const File_Path& rhs);
    File_Path& operator/(const char* rhs);

    bool operator==(const File_Path& rhs) const;

    // ---- data

    char data[file_path_capacity] = {'\0'};
    u8 size = 0u;
};

static u32 dhashmap_hash_key(const File_Path& key);

// ----

template<size_t isize>
constexpr File_Path::File_Path(const char (&idata)[isize]){
    memcpy(data, idata, isize);
    size = isize;
}

static u32 dhashmap_hash_key(const File_Path& key){
    return FNV1a_32ptr((uchar*)key.data, key.size);
}

#endif
