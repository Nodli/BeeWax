#ifndef H_FILEPATH
#define H_FILEPATH

constexpr u32 file_path_capacity = 255u;
static_assert(((file_path_capacity + 1u) & file_path_capacity) == 0u);


constexpr char path_separator();
constexpr char path_separator_to_replace();
void set_correct_path_separator(char* str);

struct File_Path;
File_Path asset_path();

// NOTE(hugo): using File_Path instead of const char* as a function parameter
// outputs the same assembly even without optimizations (assembly checked gcc & msvc)
struct File_Path{

    File_Path();

    // NOTE(hugo): const char* to File_Path
    template<size_t isize>
    constexpr File_Path(const char (&idata)[isize]);

    File_Path& operator=(const File_Path& rhs);
    File_Path& operator=(const char* rhs);
    File_Path& operator/(const File_Path& rhs);
    File_Path& operator/(const char* rhs);

    void extract_from(const char* ptr, u32 size);

    // ---- data

    u8 size;
    char data[file_path_capacity];
};

bool operator==(const File_Path& lhs, const File_Path& rhs);
bool operator==(const File_Path& lhs, const char* rhs);
bool operator==(const char* lhs, const File_Path& rhs);

bool operator!=(const File_Path& lhs, const File_Path& rhs);
bool operator!=(const File_Path& lhs, const char* rhs);
bool operator!=(const char* lhs, const File_Path& rhs);

u32 hashmap_hash(const File_Path& path);

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
