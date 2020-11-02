char path_separator(){
#if defined(PLATFORM_WINDOWS)
    return '\\';
#elif defined(PLATFORM_LINUX)
    return '/';
#else
    return '\0';
#endif
}

buffer<u8> read_file(const char* const path){
    FILE* f = fopen(path, "rb");

    buffer<u8> buffer;

    fseek(f, 0, SEEK_END);
    s64 fsize = ftell(f);
    assert(!(fsize < 0));
    fseek(f, 0, SEEK_SET);

    buffer.data = (unsigned char*)malloc((size_t)fsize);
    if(buffer.data){
        size_t fread_size = fread(buffer.data, sizeof(char), (size_t)fsize, f);
        assert(fread_size == (size_t)fsize);
        buffer.size = (u32)fread_size;
    }

    fclose(f);

    return buffer;
}

char* read_file_cstring(const char* const path){
    FILE* f = fopen(path, "rb");

    fseek(f, 0, SEEK_END);
    s64 fsize = ftell(f);
    assert(!(fsize < 0));
    fseek(f, 0, SEEK_SET);

    char* data = (char*)malloc((size_t)fsize + 1u);
    if(data){
        size_t fread_size = fread(data, sizeof(char), (size_t)fsize, f);
        assert(fread_size == (size_t)fsize);
        data[fsize] = '\0';
    }

    fclose(f);

    return data;
}
