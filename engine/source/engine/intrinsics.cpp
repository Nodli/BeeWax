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
    else if constexpr (sizeof(T) == 4u)
        return _InterlockedExchange(atomic, new_value);
    else if constexpr (sizeof(T) == 8u)
        return _InterlockedExchange64((LONG64*)atomic, (LONG64)new_value);
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

// ---- cycle counter

#if defined(AVAILABLE_RDTSC)
u64 cycle_counter(){
    return __rdtsc();
}
#endif

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

// ---- bitscan

u32 bitscan_LM(u32 value){
    assert(value != 0u);

#if defined(COMPILER_MSVC)
    static_assert(sizeof(u32) == sizeof(unsigned long));
    u32 output;
    _BitScanForward((unsigned long*)&output, value);
    return output;
#elif defined(COMPILER_GCC)
    return __builtin_ctz(value);
#else
    static_assert(false, "bitsan_LM(u32) not implemented");
#endif

}
u32 bitscan_ML(u32 value){
    assert(value != 0u);

#if defined(COMPILER_MSVC)
    u32 output;
    static_assert(sizeof(u32) == sizeof(unsigned long));
    _BitScanReverse((unsigned long*)&output, value);
    return output;
#elif defined(COMPILER_GCC)
    return __builtin_clz(value);
#else
    static_assert(false, "bitscan_ML(u32) not implemented");
#endif
}

u32 bitscan_LM(u64 value){
    assert(value != 0u);

#if defined(COMPILER_MSVC)
    u32 output;
    static_assert(sizeof(u64) == sizeof(unsigned __int64));
    _BitScanForward((unsigned long*)&output, value);
    return output;
#elif defined(COMPILER_GCC)
    return __builtin_ctz(value);
#else
    static_assert(false, "bitsan_LM(u64) not implemented");
#endif
}
u32 bitscan_ML(u64 value){
    assert(value != 0u);

#if defined(COMPILER_MSVC)
    u32 output;
    static_assert(sizeof(u64) == sizeof(unsigned __int64));
    _BitScanReverse((unsigned long*)&output, value);
    return output;
#elif defined(COMPILER_GCC)
    return __builtin_clz(value);
#else
    static_assert(false, "bitscan_ML(u64) not implemented");
#endif
}

// ---- cpu capabilities

#if defined(AVAILABLE_CPUID)
// NOTE(hugo): platform-independant __cpuidex
// /eax/ contains the code of the information to retrieve
// /ecx/ contains the value of ECX before calling cpuid
// REF(hugo):
// https://docs.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-2008/hskdteyh(v=vs.90)
// https://github.com/Mysticial/FeatureDetector/blob/master/src/x86/cpu_x86.cpp
static inline void cpuidex_internal(s32* out, s32 eax, s32 ecx){
#if defined(COMPILER_MSVC)
    __cpuidex(out, eax, ecx);

#elif defined(COMPILER_GCC)
    __cpuid_count(eax, ecx, out[0], out[1], out[2], out[3]);

#else
    static_assert(false, "cpuidex_internal() is not implemented for this platform");

#endif
}

namespace BEEWAX_INTERNAL{
    static s32 simd_instruction_set = -1;
}

s32 detect_vector_capabilities(){
    if(BEEWAX_INTERNAL::simd_instruction_set != -1)
        return BEEWAX_INTERNAL::simd_instruction_set;

    // NOTE(hugo): start detection using cpuid
    BEEWAX_INTERNAL::simd_instruction_set = 0;

    s32 cpuinfo[4];
    cpuidex_internal(cpuinfo, 0, 0);

    if(cpuinfo[0] == 0)
        return BEEWAX_INTERNAL::simd_instruction_set;

    cpuidex_internal(cpuinfo, 1, 0);

#define CPUINFO_DETECTION(WORD_INDEX, BIT_INDEX)        \
    if((cpuinfo[WORD_INDEX] & (1 << BIT_INDEX)) == 0)   \
        return BEEWAX_INTERNAL::simd_instruction_set;

    CPUINFO_DETECTION(3, 0);    // FPU (x87 FPU on chip)
    CPUINFO_DETECTION(3, 23);   // MMX (MMX technology)
    CPUINFO_DETECTION(3, 15);   // CMOV (Conditional move/compare instruction)
    CPUINFO_DETECTION(3, 24);   // FXSR (FXSAVE/FXRSTOR)
    CPUINFO_DETECTION(3, 25);   // SSE
    BEEWAX_INTERNAL::simd_instruction_set = 1;

    CPUINFO_DETECTION(3, 26);   // SSE2
    BEEWAX_INTERNAL::simd_instruction_set = 2;

    CPUINFO_DETECTION(2, 0);    // SSE3
    BEEWAX_INTERNAL::simd_instruction_set = 3;

    CPUINFO_DETECTION(2, 9);    // SSSE3 (Supplemental Streaming SIMD Extensions 3)
    BEEWAX_INTERNAL::simd_instruction_set = 4;

    CPUINFO_DETECTION(2, 19);   // SSE4.1
    BEEWAX_INTERNAL::simd_instruction_set = 5;

    CPUINFO_DETECTION(2, 23);   // POPCNT (POPCNT instruction support)
    CPUINFO_DETECTION(2, 20);   // SSE4.2
    BEEWAX_INTERNAL::simd_instruction_set = 6;

#undef CPUINFO_DETECTION

    return BEEWAX_INTERNAL::simd_instruction_set;
}
#endif

#if defined(AVAILABLE_MULTITHREADING)
u32 detect_physical_cores(){
#if defined(PLATFORM_WINDOWS)
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;

#elif defined(PLATFORM_LINUX)
    return (u32)sysconf(_SC_NPROCESSORS_ONLN);

#else
    static_assert(false, "detect_physical_cores() is not implemented for this platform");

#endif
}
#endif
