#ifndef H_OS
#define H_OS

struct vmemory {
    void* ptr = nullptr;
    size_t bytesize = 0u;
};

void malloc_vmemory(vmemory& memory);
void free_vmemory(vmemory& memory);

#endif
