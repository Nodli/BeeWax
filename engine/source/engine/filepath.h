#ifndef H_FILEPATH
#define H_FILEPATH

constexpr u32 file_path_capacity = 255u;
struct File_Path;

constexpr char path_separator();
constexpr char path_separator_to_replace();
void set_correct_path_separator(char* str);
File_Path asset_path();

// NOTE(hugo): using File_Path instead of const char* as a function parameter
// outputs the same assembly even without optimizations (assembly checked gcc & msvc)
struct File_Path{
    template<size_t isize>
    constexpr File_Path(const char (&idata)[isize]);
    File_Path(){};

    void extract_from(const char* ptr, u32 size);

    File_Path& operator=(const File_Path& rhs);
    File_Path& operator=(const char* rhs);
    File_Path& operator/(const File_Path& rhs);
    File_Path& operator/(const char* rhs);

    bool operator==(const File_Path& rhs) const;
    bool operator==(const char* rhs) const;

    // ---- data

    static_assert(((file_path_capacity + 1u) & file_path_capacity) == 0u);

    char data[file_path_capacity] = {'\0'};
    u8 size = 0u;

};

u32 hashmap_hash_key(const File_Path& key);

// ----

template<size_t isize>
constexpr File_Path::File_Path(const char (&idata)[isize]){
    memcpy(data, idata, isize);
    size = isize - 1u;

    for(u32 ichar = 0u; ichar != size; ++ichar){
        if(data[ichar] == path_separator_to_replace()){
            data[ichar] = path_separator();
        }
    }
}

#endif
