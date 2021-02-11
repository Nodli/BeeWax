#if defined(PLATFORM_LINUX)

void malloc_vmemory(vmemory& memory){
    memory.ptr = mmap(NULL, memory.bytesize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if(!memory.ptr){
        LOG_ERROR("Failed to allocate virtual memory using mmap");
    }
}

void free_vmemory(vmemory& memory){
    s32 operation_result = munmap(memory.ptr, memory.bytesize);

    if(operation_result != 0){
        LOG_ERROR("Failed to free virtual memory using munmap");
    }

    memory.ptr = nullptr;
}

#elif defined(PLATFORM_WINDOWS)

void malloc_vmemory(vmemory& memory){
    memory.ptr = VirtualAlloc(NULL, memory.bytesize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if(!memory.ptr){
        LOG_ERROR("Failed to allocate virtual memory using VirtualAlloc");
    }
}

void free_vmemory(vmemory& memory){
    s32 operation_result = VirtualFree(memory.ptr, memory.bytesize, MEM_RELEASE);

    if(operation_result != 0){
        LOG_ERROR("Failed to free virtual memory using VirtualFree");
    }

    memory.ptr = nullptr;
}

#elif defined(PLATFORM_EMSCRIPTEN)

#else
    static_assert(false, "os.cpp is not implemented for this platform");

#endif



