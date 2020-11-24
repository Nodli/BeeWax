#ifndef H_BYTEOPERATION
#define H_BYTEOPERATION

// ---- alignment
// NOTE(hugo): align must be a power of two in [1, 64]

size_t align_up_offset(uintptr_t ptr_align, size_t align);
size_t align_down_offset(uintptr_t ptr_align, size_t align);

void* void_align_up(void* ptr, size_t align);
void* void_align_down(void* ptr, size_t align);

bool void_is_aligned(void* ptr, size_t align);

template<typename T>
T* align_up(const T* ptr, size_t align);
template<typename T>
T* align_down(const void* ptr, size_t align);

template<typename T>
bool is_aligned(const T* ptr, size_t align);

// ---- bitset

template<typename T>
void set_bit(T& bitset, u32 bit_index);
template<typename T>
void unset_bit(T& bitset, u32 bit_index);
template<typename T>
void toggle_bit(T& bitset, u32 bit_index);
template<typename T>
T extract_bit(T& bitset, u32 bit_index);

#endif
