#ifndef H_BITOP
#define H_BITOP

template<typename T>
struct Bitset{
    void set_bit(u32 bit_index);
    void unset_bit(u32 bit_index);
    void toggle_bit(u32 bit_index);
    T get_bit(u32 bit_index);

    T data;
};

#endif
