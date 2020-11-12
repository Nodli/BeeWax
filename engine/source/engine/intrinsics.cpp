// ---- atomic operations

template<typename T>
inline T atomic_compare_exchange(volatile T* atomic, T new_value, T previous_value, bool can_fail_exchange){
#if defined(COMPILER_MSVC)
    if constexpr (sizeof(T) == 8u)
        return _InterlockedCompareExchange8(atomic, new_value, previous_value);
    else if constexpr (sizeof(T) == 16u)
        return _InterlockedCompareExchange16(atomic, new_value, previous_value);
    else if constexpr (sizeof(T) == 32u)
        return _InterlockedCompareExchange(atomic, new_value, previous_value);
    else if constexpr (sizeof(T) == 64u)
        return _InterlockedCompareExchange64(atomic, new_value, previous_value);
    else
#elif defined(COMPILER_GCC)
    if constexpr (sizeof(T) == 8u || sizeof(T) == 16u || sizeof(T) == 32u || sizeof(T) == 64u){
        T output;
        __atomic_compare_exchange(atomic, &new_value, &output, can_fail_exchange, __ATOMIC_ACQ_REL, __ATOMIC_ACQ_REL);
        return output;
    }
    else
#endif
        static_assert(false, "atomic_compare_exchange is not implemented for this type");
}

template<typename T>
inline T atomic_exchange(volatile T* atomic, T new_value){
#if defined(COMPILER_MSVC)
    if constexpr (sizeof(T) == 8u)
        return _InterlockedExchange8(atomic, new_value);
    else if constexpr (sizeof(T) == 16u)
        return _InterlockedExchange16(atomic, new_value);
    else if constexpr (sizeof(T) == 32u)
        return _InterlockedExchange(atomic, new_value);
    else if constexpr (sizeof(T) == 64u)
        return _InterlockedExchange64(atomic, new_value);
    else
#elif defined(COMPILER_GCC)
    if constexpr (sizeof(T) == 8u || sizeof(T) == 16u || sizeof(T) == 32u || sizeof(T) == 64u){
        T output;
        __atomic_exchange(atomic, &new_value, &output, __ATOMIC_ACQ_REL);
        return output;
    }
    else
#endif
        static_assert(false, "atomic_exchange is not implemented for this type");
}

template<typename T>
inline T atomic_get(volatile T* atomic){
#if defined(COMPILER_MSVC)
    if constexpr (sizeof(T) == 8u || sizeof(T) == 16u || sizeof(T) == 32u || sizeof(T) == 64u, "")
        return *atomic;
    else
#elif defined(COMPILER_GCC)
    if constexpr (sizeof(T) == 8u || sizeof(T) == 16u || sizeof(T) == 32u || sizeof(T) == 64u, "")
        return __atomic_load_n(atomic, __ATOMIC_ACQ_REL);
    else
#endif
        static_assert(false, "atomic_get is not implemented for this type");
}

template<typename T>
inline void atomic_set(volatile T* atomic, T value){
#if defined(COMPILER_GCC)
    static_assert(sizeof(T) == 8u || sizeof(T) == 16u || sizeof(T) == 32u || sizeof(T) == 64u, "")
    __atomic_store(atomic, &value, __ATOMIC_ACQ_REL);
#else
    atomic_exchange(atomic, value);
#endif
}

// ---- cycle counter

u64 cycle_counter(){
    return __rdtsc();
}

// ---- endianness conversion

DISABLE_WARNING_PUSH
DISABLE_WARNING_SIGN_CONVERSION
#if defined(COMPILER_MSVC)
    u16 byteswap(u16 x){
        return _byteswap_ushort(x);
    }
    s16 byteswap(s16 x){
        return _byteswap_ushort(x);
    }
    u32 byteswap(u32 x){
        return _byteswap_ulong(x);
    }
    s32 byteswap(s32 x){
        return _byteswap_ulong(x);
    }
    u64 byteswap(u64 x){
        return _byteswap_uint64(x);
    }
    s64 byteswap(s64 x){
        return _byteswap_uint64(x);
    }

#elif defined(COMPILER_GCC)
    u16 byteswap(u16 x){
        return __builtin_bswap16(x);
    }
    s16 byteswap(s16 x){
        return __builtin_bswap16(*(u16*)&x);
    }
    u32 byteswap(u32 x){
        return __builtin_bswap32(x);
    }
    s32 byteswap(s32 x){
        return __builtin_bswap32(*(u32*)&x);
    }
    u64 byteswap(u64 x){
        return __builtin_bswap64(x);
    }
    s64 byteswap(s64 x){
        return __builtin_bswap64(*(u64*)&x);
    }

#else
    static_assert(false, "byteswap() is not implemented for this platform");

#endif
DISABLE_WARNING_POP

// ---- cpu capabilities

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

#define CPUINFO_DETECTION(WORD_INDEX, BIT_INDEX)                \
    if(cpuinfo[WORD_INDEX] & (1 << BIT_INDEX) == 0)             \
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

