// ---- constexpr

template<typename T, size_t size>
constexpr size_t carray_size(T(&)[size]){
    return size;
}

template<typename T>
constexpr void swap(T& A, T& B){
    T temp = A;
    A = B;
    B = temp;
}

constexpr float constexpr_sqrt_recursion(const float x, const float ycurrent, const float yprevious){
    if (ycurrent == yprevious){
        return ycurrent;
    }else{
        return constexpr_sqrt_recursion(x, 0.5f * (ycurrent + x / ycurrent), ycurrent);
    }
}

// NOTE(hugo): unnecessary to check against NaN or infinite because this is used at compile time on user provided values
constexpr float constexpr_sqrt(const float x){
    if(x >= 0.f){
        return constexpr_sqrt_recursion(x, x, 0.f);
    }else{
        return 0.f;
    }
}

template<typename T>
constexpr T sign(const T x){
    return (T)((s32)(0 < x) - (s32)(x < 0));
}

template<typename T>
constexpr T abs(const T x){
    return (x < (T)0) ? - x : x;
}

template<typename T>
constexpr T min(const T A, const T B){
    return (A < B) ? A : B;
}

template<typename T>
constexpr T max(const T A, const T B){
    return (A > B) ? A : B;
}

template<typename T>
constexpr T min_max(const T x, const T min_value, const T max_value){
    return min(max(x, min_value), max_value);
};

// ---- constexpr template type indexing

template<typename T, typename ... Types>
struct Type_Indexer_Recursion;

// NOTE(hugo): increment the index for each non-T type in the parameter pack
template<typename T, typename U, typename ... Types>
struct Type_Indexer_Recursion<T, U, Types...>{
    static constexpr size_t recursion_index = 1u + Type_Indexer_Recursion<T, Types...>::recursion_index;
};

// NOTE(hugo): bottom of the recursion when the type is equal
template<typename T, typename ... Types>
struct Type_Indexer_Recursion<T, T, Types...>{
    static constexpr size_t recursion_index = 0u;
};

template<typename ... Types>
template<typename T>
constexpr size_t Type_Indexer<Types...>::type_index(){
    return Type_Indexer_Recursion<T, Types...>::recursion_index;
}

// ---- bitset

template<typename T>
void set_bit(T& bitset, u32 bit_index){
    assert(bit_index < sizeof(T) * 8u);
    bitset |= ((T)1u << bit_index);
}

template<typename T>
void unset_bit(T& bitset, u32 bit_index){
    assert(bit_index < sizeof(T) * 8u);
    bitset &= (~((T)1u << bit_index));
}

template<typename T>
void toggle_bit(T& bitset, u32 bit_index){
    assert(bit_index < sizeof(T) * 8u);
    bitset ^= ((T)1u << bit_index);
}

template<typename T>
T extract_bit(T& bitset, u32 bit_index){
    assert(bit_index < sizeof(T) * 8u);
    return bitset & ((T)1u << bit_index);
}
