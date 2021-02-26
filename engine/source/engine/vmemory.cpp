namespace BEEWAX_INTERNAL{
    static size_t vmemory_pagesize = 0u;
}

void setup_vmemory(){
    BEEWAX_INTERNAL::vmemory_pagesize = detect_pagesize();
}

#if defined(PLATFORM_LINUX)
#elif defined(PLATFORM_WINDOWS)
#else
    static_assert(false, "Virtual_Arena::initialize() not implemented for this platform");
#endif

void Virtual_Arena::initialize(size_t bytesize){
    assert(bytesize != 0u && BEEWAX_INTERNAL::vmemory_pagesize != 0u);

    vbytesize = round_up_multiple(bytesize, BEEWAX_INTERNAL::vmemory_pagesize);
    commit_page_count = 0u;
    cursor = 0u;

#if defined(PLATFORM_LINUX)
    ENGINE_CHECK(vmemory = mmap(nullptr, vbytesize, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0), "FAILED to mmap");
#elif defined(PLATFORM_WINDOWS)
    ENGINE_CHECK(vmemory = VirtualAlloc(nullptr, vbytesize, MEM_RESERVE, PAGE_NOACCESS), "FAILED VirtualAlloc");
#else
    static_assert(false, "Virtual_Arena::initialize() not implemented for this platform");
#endif
}

void Virtual_Arena::terminate(){
    assert(vmemory && vbytesize != 0u);

#if defined(PLATFORM_LINUX)
    ENGINE_CHECK(munmap(vmemory, vbytesize) == 0u, "FAILED munmap");
#elif defined(PLATFORM_WINDOWS)
    ENGINE_CHECK(VirtualFree(vmemory, 0u, MEM_RELEASE), "FAILED VirtualFree");
#else
    static_assert(false, "Virtual_Arena::terminate() not implemented for this platform");
#endif

    *this = Virtual_Arena();
}

void Virtual_Arena::reset(){
#if defined(PLATFORM_LINUX)
    ENGINE_CHECK(mprotect(vmemory, commit_page_count * BEEWAX_INTERNAL::vmemory_pagesize, PROT_NONE) == 0, "FAILED mprotect");
#elif defined(PLATFORM_WINDOWS)
    ENGINE_CHECK(VirtualFree(vmemory, commit_page_count * BEEWAX_INTERNAL::vmemory_pagesize, MEM_DECOMMIT), "FAILED VirtualFree");
#else
    static_assert(false, "Virtual_Arena::reset() not implemented for this platform");
#endif

    commit_page_count = 0u;
    cursor = 0u;
}

void Virtual_Arena::reset_to_cursor(){
    size_t new_commit_bytesize = round_up_multiple(cursor, BEEWAX_INTERNAL::vmemory_pagesize);
    size_t current_commit_bytesize = commit_page_count * BEEWAX_INTERNAL::vmemory_pagesize;

    if(new_commit_bytesize != current_commit_bytesize){
        assert(new_commit_bytesize < current_commit_bytesize);

        void* base_vmemory = (void*)((u8*)vmemory + new_commit_bytesize);
        size_t to_decommit = current_commit_bytesize - new_commit_bytesize;

#if defined(PLATFORM_LINUX)
        ENGINE_CHECK(mprotect(base_vmemory, to_decommit, PROT_NONE) == 0, "FAILED mprotect");
#elif defined(PLATFORM_WINDOWS)
        ENGINE_CHECK(VirtualFree(base_vmemory, to_decommit, MEM_DECOMMIT), "FAILED VirtualFree");
#else
        static_assert(false, "Virtual_Arena::reset() not implemented for this platform");
#endif

        commit_page_count = new_commit_bytesize / BEEWAX_INTERNAL::vmemory_pagesize;
    }
}

Virtual_Arena_Memory Virtual_Arena::malloc(size_t bytesize, size_t alignment){
    size_t commit_bytesize = commit_page_count * BEEWAX_INTERNAL::vmemory_pagesize;

    size_t padding = align_offset_next((uintptr_t)vmemory + cursor, alignment);
    size_t new_cursor = cursor + padding + bytesize;
    ENGINE_CHECK(new_cursor <= vbytesize, "OUT OF VIRTUAL MEMORY !");

    if(new_cursor > commit_bytesize){
        size_t to_commit = round_up_multiple(new_cursor - commit_bytesize, BEEWAX_INTERNAL::vmemory_pagesize);
        void* base_vmemory = (void*)((u8*)vmemory + commit_bytesize);

#if defined(PLATFORM_LINUX)
        ENGINE_CHECK(mprotect(base_vmemory, to_commit, PROT_READ | PROT_WRITE) == 0, "FAILED mprotect");
#elif defined(PLATFORM_WINDOWS)
        ENGINE_CHECK(VirtualAlloc(base_vmemory, to_commit, MEM_COMMIT, PAGE_READWRITE), "FAILED VirtualAlloc");
#else
        static_assert(false, "Virtual_Arena::malloc() not implemented for this platform");
#endif

        commit_page_count += to_commit / BEEWAX_INTERNAL::vmemory_pagesize;
    }

    Virtual_Arena_Memory out;
    out.ptr = (void*)((u8*)vmemory + cursor + padding);
    out.previous_cursor = cursor;

    cursor = new_cursor;
    return out;
}

void Virtual_Arena::free(const Virtual_Arena_Memory& memory){
    assert(memory.previous_cursor < cursor);
    cursor = memory.previous_cursor;
}
