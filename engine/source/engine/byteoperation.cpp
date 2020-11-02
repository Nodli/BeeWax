// ---- alignment

size_t align_up_offset(uintptr_t uptr, size_t align){
    assert(is_pow2(align));
    return (size_t)((- uptr) & (align - 1u));
}

void* void_align_up(void* ptr, size_t align){
    assert(is_pow2(align));
    uintptr_t uint_ptr = (uintptr_t)ptr;
    return (void*)(uint_ptr + align_up_offset(uint_ptr, align));
}

void* void_align_down(void* ptr, size_t align){
    assert(is_pow2(align));
    uintptr_t uint_ptr = (uintptr_t)ptr;
    return (void*)(uint_ptr & ~(align - 1u));
}

size_t align_down_offset(uintptr_t uptr, size_t align){
    assert(is_pow2(align));
    return (size_t)(uptr - (uintptr_t)void_align_down((void*)uptr, align));
}

bool void_is_aligned(void* ptr, size_t align){
    return ptr == void_align_down(ptr, align);
};

// ref : https://github.com/KabukiStarship/KabukiToolkit/wiki/Fastest-Method-to-Align-Pointers#3-kabuki-toolkit-memory-alignment-algorithm
template<typename T>
T* align_up(const T* ptr, size_t align){
    return (T*)void_align_up((void*)ptr, align);
}

// ref : https://stackoverflow.com/questions/4840410/how-to-align-a-pointer-in-c
template<typename T>
T* align_down(const T* ptr, size_t align){
    return (T*)(void_align_down((void*)ptr, align));
}

template<typename T>
bool is_aligned(const T* ptr, size_t align){
    return void_is_aligned((void*)ptr, align);
}
