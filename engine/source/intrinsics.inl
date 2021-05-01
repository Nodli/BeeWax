// ---- atomic operations

template<typename T>
inline T atomic_compare_exchange(volatile T* atomic, T new_value, T previous_value, bool can_fail_exchange){
    static_assert((sizeof(T) == 1u || sizeof(T) == 2u || sizeof(T) == 4u || sizeof(T) == 8u),
            "atomic_compare_exchange is not implemented for this type");

#if defined(COMPILER_MSVC)
    if constexpr (sizeof(T) == 1u)
        return _InterlockedCompareExchange8(atomic, new_value, previous_value);
    else if constexpr (sizeof(T) == 2u)
        return _InterlockedCompareExchange16(atomic, new_value, previous_value);
    else if constexpr (sizeof(T) == 4u)
        return _InterlockedCompareExchange(atomic, new_value, previous_value);
    else if constexpr (sizeof(T) == 8u)
        return _InterlockedCompareExchange64(atomic, new_value, previous_value);
#elif defined(COMPILER_GCC)
    static_assert(__atomic_always_lock_free(sizeof(T), NULL));
    T output;
    __atomic_compare_exchange(atomic, &new_value, &output, can_fail_exchange, __ATOMIC_ACQ_REL, __ATOMIC_ACQ_REL);
    return output;
#else
    static_assert(false, "atomic_compare_exchange not implemented");
#endif
}

template<typename T>
inline T atomic_exchange(volatile T* atomic, T new_value){
    static_assert((sizeof(T) == 1u || sizeof(T) == 2u || sizeof(T) == 4u || sizeof(T) == 8u),
            "atomic_exchange is not implemented for this type");

#if defined(COMPILER_MSVC)
    if constexpr (sizeof(T) == 1u)
        return _InterlockedExchange8(atomic, new_value);
    else if constexpr (sizeof(T) == 2u)
        return _InterlockedExchange16(atomic, new_value);
    else if constexpr (sizeof(T) == 4u){
        long* reinterpret_new_value = (long*)&new_value;
        long return_value = _InterlockedExchange((volatile long*)atomic, *reinterpret_new_value);
        T* return_value_as_T = (T*)&return_value;
        return *return_value_as_T;
    }
    else if constexpr (sizeof(T) == 8u){
        LONG64* reinterpret_new_value = (LONG64*)&new_value;
        LONG64 return_value = _InterlockedExchange64((volatile LONG64*)atomic, *reinterpret_new_value);
        T* return_value_as_T = (T*)&return_value;
        return *return_value_as_T;
    }
#elif defined(COMPILER_GCC)
    static_assert(__atomic_always_lock_free(sizeof(T), NULL));
    T output;
    __atomic_exchange(atomic, &new_value, &output, __ATOMIC_ACQ_REL);
    return output;
#else
    static_assert(false, "atomic_exchange not implemented");
#endif
}

template<typename T>
inline T atomic_get(volatile T* atomic){
    static_assert((sizeof(T) == 1u || sizeof(T) == 2u || sizeof(T) == 4u || sizeof(T) == 8u),
            "atomic_get is not implemented for this type");

#if defined(COMPILER_MSVC)
    return *atomic;
#elif defined(COMPILER_GCC)
    static_assert(__atomic_always_lock_free(sizeof(T), NULL));
    // NOTE(hugo): __ATOMIC_ACQ_REL is invalid for __atomic_load_n
    __atomic_thread_fence(__ATOMIC_RELEASE);
    return __atomic_load_n(atomic, __ATOMIC_ACQUIRE);
#else
    static_assert(false, "atomic_get not implemented");
#endif
}

template<typename T>
inline void atomic_set(volatile T* atomic, T value){
    static_assert((sizeof(T) == 1u || sizeof(T) == 2u || sizeof(T) == 4u || sizeof(T) == 8u),
            "atomic_store is not implemented for this type");
#if defined(COMPILER_MSVC)
    atomic_exchange<T>(atomic, value);
#elif defined(COMPILER_GCC)
    static_assert(__atomic_always_lock_free(sizeof(T), NULL));
    // NOTE(hugo): __ATOMIC_ACQ_REL is invalid for __atomic_store
    __atomic_thread_fence(__ATOMIC_ACQUIRE);
    __atomic_store(atomic, &value, __ATOMIC_RELEASE);
#else
    static_assert(false, "atomic_set not implemented");
#endif
}

// ---- endianness conversion

template<typename T>
inline T atomic_byteswap(T value){
    static_assert((sizeof(T) == 2u || sizeof(T) == 4u || sizeof(T) == 8u),
            "byteswap() is not implemented for this type");

#if defined(COMPILER_MSVC)
    if constexpr (sizeof(T) == 2u)
        return _byteswap_ushort(*(ushort*)&value);
    else if constexpr (sizeof(T) == 4u)
        return _byteswap_ulong(*(u32*)&value);
    else if constexpr (sizeof(T) == 8u)
        return _byteswap_uint64(*(u64*)&value);
#elif defined(COMPILER_GCC)
    if constexpr (sizeof(T) == 2u)
        return __builtin_bswap16(*(ushort*)&value);
    else if constexpr (sizeof(T) == 4u)
        return __builtin_bswap32(*(u32*)&value);
    else if constexpr (sizeof(T) == 8u)
        return __builtin_bswap64(*(u64*)&value);
#else
    static_assert(false, "atomic_byteswap not implemented");
#endif
}
