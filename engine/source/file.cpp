// ---- io

File_Data read_file(const File_Path& path, const char* mode){
    FILE* f = fopen(path.data, mode);
    ENGINE_CHECK(f != NULL, "read_file(%s) FAILED - returning empty array", path.data);

    File_Data output;

    fseek(f, 0, SEEK_END);
    s64 fsize = ftell(f);
    assert(fsize >= 0u);
    fseek(f, 0, SEEK_SET);

    output.data = bw_malloc(fsize);
    assert(output.data);

    size_t fread_size = fread(output.data, sizeof(char), (size_t)fsize, f);
    assert((size_t)fsize >= fread_size);

    output.bytesize = (u32)fread_size;

    fclose(f);

    return output;
}

File_Data read_file_cstring(const File_Path& path){
    FILE* f = fopen(path.data, "r");
    ENGINE_CHECK(f != NULL, "read_file_cstring(%s) FAILED - returning nullptr", path.data);

    File_Data output;

    fseek(f, 0, SEEK_END);
    s64 fsize = ftell(f);
    assert(fsize == 0u || fsize > 0u);
    fseek(f, 0, SEEK_SET);

    output.data = bw_malloc(fsize + 1u);
    assert(output.data);

    size_t fread_size = fread(output.data, sizeof(char), (size_t)fsize, f);
    assert((size_t)fsize >= fread_size);

    ((char*)output.data)[fread_size] = '\0';
    output.bytesize = fread_size + 1u;

    fclose(f);

    return output;
}

void write_file(const File_Path& path, const u8* data, size_t bytesize){
    FILE* f = fopen(path.data, "wb");
    ENGINE_CHECK(f != NULL, "write_file(%s) FAILED - aborting write", path.data);

    fwrite(data, 1, bytesize, f);
    fclose(f);
}
