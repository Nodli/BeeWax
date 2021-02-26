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
    void initialize(size_t bytesize);
    void terminate();

    void reset();
    void reset_to_cursor();

    Virtual_Arena_Memory malloc(size_t bytesize, size_t alignment);
    void free(const Virtual_Arena_Memory& memory);

    template<typename T>
    Virtual_Arena_Memory malloc(u32 nT){
        return malloc(nT * sizeof(T), alignof(T));
    }

    // ---- data

    void* vmemory = nullptr;
    size_t vbytesize = 0u;

    size_t commit_page_count = 0u;
    size_t cursor = 0u;
};

#endif
