template<typename T>
T unpacker::get_bytes(){
    T output = *(T*)cursor;
    cursor = cursor + sizeof(T);
    return output;
}

template<typename T>
T unpacker::get_bytes_LE(){
    return get_bytes<T>();
}

template<typename T>
T unpacker::get_bytes_BE(){
    return byteswap(get_bytes<T>());
}

