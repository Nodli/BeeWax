template<typename T>
void Bitset<T>::set_bit(u32 bit_index){
    assert(bit_index < sizeof(T) * 8u);
    data |= (T)(1u << bit_index);
}

template<typename T>
void Bitset<T>::unset_bit(u32 bit_index){
    assert(bit_index < sizeof(T) * 8);
    data &= (T)(~(1u << bit_index));
}

template<typename T>
void Bitset<T>::toggle_bit(u32 bit_index){
    assert(bit_index < sizeof(T) * 8);
    data ^= (T)(1u << bit_index);
}

template<typename T>
T Bitset<T>::get_bit(u32 bit_index){
    assert(bit_index < sizeof(T) * 8);
    return data & (T)(1u << bit_index);
}
