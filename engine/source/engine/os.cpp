#if defined(PLATFORM_LINUX)
void virtual_alloc(virtual_memory& memory){
    memory.ptr = mmap(NULL, memory.bytesize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if(!memory.ptr){
        LOG_ERROR("Failed to allocate virtual memory using mmap");
    }
}

void virtual_free(virtual_memory& memory){
    s32 operation_result = munmap(memory.ptr, memory.bytesize);

    if(operation_result != 0){
        LOG_ERROR("Failed to free virtual memory using munmap");
    }

    memory.ptr = nullptr;
}

#elif defined(PLATFORM_WINDOWS)
void virtual_alloc(virtual_memory& memory){
    memory.ptr = VirtualAlloc(NULL, memory.bytesize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if(!memory.ptr){
        LOG_ERROR("Failed to allocate virtual memory using VirtualAlloc");
    }
}

void virtual_free(virtual_memory& memory){
    s32 operation_result = VirtualFree(memory.ptr, memory.bytesize, MEM_RELEASE);

    if(operation_result != 0){
        LOG_ERROR("Failed to free virtual memory using VirtualFree");
    }

    memory.ptr = nullptr;
}

#else
    static_assert(false, "os.cpp is not implemented for this platform");
#endif


// ---- varena

void varena::reserve(size_t arena_bytesize){
    assert(memory.bytesize == 0u);
    memory.bytesize = arena_bytesize;
    virtual_alloc(memory);
    position = 0u;
}

void varena::free(){
    virtual_free(memory);
    *this = varena();
}

vchunk varena::push_chunk(size_t bytesize, size_t alignment){
    assert(memory.ptr);

    size_t alignment_offset = align_up_offset((uintptr_t)((u8*)memory.ptr + position), alignment);

    if(position + alignment_offset + bytesize > memory.bytesize){
        LOG_ERROR("Failed to push(%u, %u) using an arena", bytesize, alignment);
        return {};
    }

    void* adress = (u8*)memory.ptr + position + alignment_offset;

    vchunk output;
    output.reference_ptr = adress;
    output.start = position;
    output.bytesize = bytesize;

    position += alignment_offset + bytesize;

    return output;
}

void varena::pop_chunk(vchunk chunk){
    position = min(position, chunk.start);
}

void* varena::push(size_t bytesize, size_t alignment){
    assert(memory.ptr);
    size_t alignment_offset = align_up_offset((uintptr_t)((u8*)memory.ptr + position), alignment);

    if(position + alignment_offset + bytesize > memory.bytesize){
        LOG_ERROR("Failed to push(%u, %u) using an arena", bytesize, alignment);
        return nullptr;
    }

    void* adress = (void*)((u8*)memory.ptr + position + alignment_offset);

    position += alignment_offset + bytesize;

    return adress;
}

void varena::pop(size_t bytesize, size_t alignment){
    assert(memory.ptr);

    // NOTE(hugo): no need to check against memory.ptr because mmap returns a page-aligned pointer
    position = position
        - min(bytesize, position)
        - align_down_offset((uintptr_t)position, alignment);
}

template<typename T>
void* varena::push(){
    return push(sizeof(T), alignof(T));
}

template<typename T>
void varena::pop(){
    pop(sizeof(T), alignof(T));
}

void varena::clear(){
    position = 0u;
}
