// ---- io

array<u8> read_file(const File_Path& path, const char* mode){
    FILE* f = fopen(path.data, mode);
    ENGINE_CHECK(f != NULL, "read_file(%s) FAILED - returning empty array", path.data);

    array<u8> buffer;

    fseek(f, 0, SEEK_END);
    s64 fsize = ftell(f);
    assert(!(fsize < 0));
    fseek(f, 0, SEEK_SET);

    buffer.reserve(fsize);

    size_t fread_size = fread(buffer.data, sizeof(char), (size_t)fsize, f);
    assert(fread_size <= (size_t)fsize);
    buffer.size = (u32)fread_size;

    fclose(f);

    return buffer;
}

char* read_file_cstring(const File_Path& path){
    FILE* f = fopen(path.data, "r");
    ENGINE_CHECK(f != NULL, "read_file_cstring(%s) FAILED - returning nullptr", path.data);

    fseek(f, 0, SEEK_END);
    s64 fsize = ftell(f);
    assert(!(fsize < 0));
    fseek(f, 0, SEEK_SET);

    ++fsize;

    char* data = (char*)bw_malloc((size_t)fsize + 1u);
    if(data){
        size_t fread_size = fread(data, sizeof(char), (size_t)fsize, f);
        assert(fread_size < (size_t)fsize);
        data[fread_size] = '\0';
    }

    fclose(f);

    return data;
}

void write_file(const File_Path& path, const u8* data, size_t bytesize){
    FILE* f = fopen(path.data, "wb");
    ENGINE_CHECK(f != NULL, "write_file(%s) FAILED - aborting write", path.data);

    fwrite(data, 1, bytesize, f);
    fclose(f);
}
