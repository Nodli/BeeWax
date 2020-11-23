buffer<u8> read_file(const File_Path& path){
    FILE* f = fopen(path.data, "rb");
    if(f == NULL){
        LOG_ERROR("read_file(%s) FAILED - returning buffer with nullptr", path.data);
        return {nullptr, 0u};
    }

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

char* read_file_cstring(const File_Path& path){
    FILE* f = fopen(path.data, "rb");
    if(f == NULL){
        LOG_ERROR("read_file_cstring(%s) FAILED - returning nullptr", path.data);
        return nullptr;
    }

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

void write_file(const File_Path& path, const u8* data, size_t bytesize){
    FILE* f = fopen(path.data, "wb");
    if(f == NULL){
        LOG_ERROR("write_file(%s) FAILED - aborting write", path.data);
        return;
    }

    fwrite(data, 1, bytesize, f);
    fclose(f);
}
