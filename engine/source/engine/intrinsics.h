// ---- atomic operations

// NOTE(hugo): memory order acquire = memory read / write after the instruction are kept after
//             memory order release = memory read / write before the instruction are kept before
// NOTE(hugo): those have acquire & release memory barriers
template<typename T>
inline T atomic_compare_exchange(volatile T* atomic, T new_value, T previous_value, bool can_fail_exchange = false);
template<typename T>
inline T atomic_exchange(volatile T* atomic, T new_value);
template<typename T>
inline T atomic_get(volatile T* atomic);
template<typename T>
inline void atomic_set(volatile T* atomic, T new_value);

// ---- cycle counter

u64 cycle_counter();

// ---- endianness conversion

template<typename T>
inline T atomic_byteswap(T value);

// ---- bitscan

// NOTE(hugo):
// returns the number of zero bits before encountering a one bit
// /!\ undefined behavior when value = 0u /!\
// LM = Least significant bit to Most  significant bit
// ML = Most  significant bit to Least significant bit
u32 bitscan_LM(u32 value);
u32 bitscan_ML(u32 value);
u32 bitscan_LM(u64 value);
u32 bitscan_ML(u64 value);

// ---- cpu capabilities

// NOTE(hugo): detects the SSE version supported by the CPU
// 1 : SSE supported
// 2 : SSE2 supported
// 3 : SSE3 supported
// 4 : SSSE3 supported
// 5 : SSE4.1 supported
// 6 : SSE4.2 supported
// REF(hugo): https://github.com/vectorclass/version2/blob/master/instrset_detect.cpp
s32 detect_vector_capabilities();

// NOTE(hugo):
u32 detect_physical_cores();

