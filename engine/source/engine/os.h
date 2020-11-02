#ifndef H_OS
#define H_OS

struct virtual_memory{
    void* ptr = nullptr;
    size_t bytesize = 0u;
};

void alloc_virtual_memory(virtual_memory& memory);
void free_virtual_memory(virtual_memory& memory);

struct vchunk{
    void* reference_ptr = nullptr;
    size_t start = 0u;
    size_t bytesize = 0u;
};

// ---- varena

struct varena{
    void reserve(size_t arena_bytesize);
    void free();

    vchunk push_chunk(size_t bytesize, size_t alignment);
    void pop_chunk(vchunk chunk);

    void* push(size_t bytesize, size_t alignment);
    void pop(size_t bytesize, size_t alignment);

    template<typename T>
    void* push();
    template<typename T>
    void pop();

    void clear();

    // ---- data

    virtual_memory memory;
    size_t position = 0u;
};

#endif
