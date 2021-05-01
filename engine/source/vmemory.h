#ifndef H_VMEMORY
#define H_VMEMORY

void setup_vmemory();

// REF(hugo):
// https://marccgk.github.io/blog/a-virtual-memory-linear-arena/

struct Virtual_Arena_Memory{
    void* ptr = nullptr;
    size_t previous_cursor = 0u;
};

struct Virtual_Arena{
    void create(size_t bytesize);
    void destroy();

    void reset();
    void reset_to_cursor();

    Virtual_Arena_Memory allocate(size_t bytesize, size_t alignment);
    void free(const Virtual_Arena_Memory& memory);

    template<typename T>
    Virtual_Arena_Memory allocate(u32 nT);

    // ---- data

    void* vmemory;
    size_t vbytesize;

    size_t commit_page_count;
    size_t cursor;
};

// ----

template<typename T>
Virtual_Arena_Memory Virtual_Arena::allocate(u32 nT){
    return allocate(nT * sizeof(T), alignof(T));
}

#endif
