// ---- cycle counter

u64 cycle_counter();

// ---- endianness conversion

u16 byteswap(u16 x);
s16 byteswap(s16 x);
u32 byteswap(u32 x);
s32 byteswap(s32 x);
u64 byteswap(u64 x);
s64 byteswap(s64 x);

// NOTE(hugo): remove byteswap on an implicitly-converted value
template <typename T>
T byteswap(T) = delete;

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

