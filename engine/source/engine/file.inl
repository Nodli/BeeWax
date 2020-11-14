template<size_t isize>
constexpr File_Path::File_Path(const char (&idata)[isize]){
    memcpy(data, idata, isize);
    size = isize;
}

template<typename T>
T get_bytes(u8*& cursor){
    T output = *(T*)cursor;
    cursor += sizeof(T);
    return output;
}

template<typename T>
T get_bytes_LE(u8*& cursor){
    return get_bytes<T>(cursor);
}

template<typename T>
T get_bytes_BE(u8*& cursor){
    return byteswap(get_bytes<T>(cursor));
}

