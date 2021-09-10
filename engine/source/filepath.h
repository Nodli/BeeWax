#ifndef H_FILEPATH
#define H_FILEPATH

// REF(hugo): https://www.youtube.com/watch?v=kPR8h4-qZdk

constexpr char path_separator();
constexpr char path_separator_to_replace();
void set_correct_path_separator(char* str);

// NOTE(hugo): using File_Path instead of const char* as a function parameter
// outputs the same assembly even without optimizations (gcc & msvc)
struct File_Path{
    static constexpr size_t bytesize = 255u;
    static_assert(bytesize && bytesize < 256u);

    /*
    File_Path();

    // NOTE(hugo): const char* to File_Path
    template<size_t isize>
    constexpr File_Path(const char (&idata)[isize]);
    */

    void extract_from(const char* str, size_t strlen);

    File_Path& operator=(const File_Path& path);
    File_Path& operator=(const char* str);
    File_Path& operator/=(const File_Path& path);
    File_Path& operator/=(const char* str);

    size_t size() const;
    static constexpr size_t capacity();

    // ---- data

    union{
        char data[bytesize];
        struct{
            char padding[bytesize - 1u];
            u8 empty_bytes;
        };
    };
};

bool operator==(const File_Path& L, const File_Path& R);
bool operator==(const File_Path& path, const char* str);
bool operator==(const char* str, const File_Path& path);

bool operator!=(const File_Path& L, const File_Path& R);
bool operator!=(const File_Path& path, const char* str);
bool operator!=(const char* str, const File_Path& path);

u32 hashmap_hash(const File_Path& path);

// ----

/*
File_Path::File_Path() : data(""), size(0u) {}

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
*/

#endif
