#if defined(DEVELOPPER_MODE)

namespace BEEWAX_INTERNAL{
    struct Tracked_Memory{
        Tracked_Memory* prev;
        Tracked_Memory* next;

        const char* filename;
        const char* function;
        u64 line;
        u64 bytesize;
    };
    static_assert(sizeof(Tracked_Memory) == 48u);

    static Tracked_Memory memtracker_head = {nullptr, nullptr, nullptr, nullptr, 0u, 0u};

    void* memtracker_malloc(size_t bytesize, const char* filename, const char* function, const u32 line){
        size_t memtracker_bytesize = bytesize + sizeof(Tracked_Memory);

        Tracked_Memory* header = (Tracked_Memory*)::malloc(memtracker_bytesize);
        if(header){
            header->prev = &memtracker_head;
            header->next = memtracker_head.next;

            memtracker_head.next = header;
            if(header->next) header->next->prev = header;

            header->filename = filename;
            header->function = function;
            header->bytesize = bytesize;
            header->line = line;

            header = header + 1u;
        }

        return (void*)(header);
    }

    void* memtracker_calloc(size_t count, size_t bytesize, const char* filename, const char* function, const u32 line){
        size_t allocation_bytesize = count * bytesize;
        size_t memtracker_bytesize = allocation_bytesize + sizeof(Tracked_Memory);

        Tracked_Memory* header = (Tracked_Memory*)::calloc(1u, memtracker_bytesize);
        if(header){
            header->prev = &memtracker_head;
            header->next = memtracker_head.next;

            memtracker_head.next = header;
            if(header->next) header->next->prev = header;

            header->filename = filename;
            header->function = function;
            header->bytesize = allocation_bytesize;
            header->line = line;

            header = header + 1u;
        }

        return (void*)(header);
    }

    void* memtracker_realloc(void* ptr, size_t bytesize, const char* filename, const char* function, const u32 line){
        size_t memtracker_bytesize = bytesize + sizeof(Tracked_Memory);

        Tracked_Memory* header;
        if(ptr) header = (Tracked_Memory*)::realloc((void*)((Tracked_Memory*)ptr - 1u), memtracker_bytesize);
        else    header = (Tracked_Memory*)::realloc(nullptr, memtracker_bytesize);

        if(header){
            if(!ptr){
                header->prev = &memtracker_head;
                header->next = memtracker_head.next;
            }
            header->prev->next = header;
            if(header->next) header->next->prev = header;


            header->filename = filename;
            header->function = function;
            header->bytesize = bytesize;
            header->line = line;

            header = header + 1u;
        }

        return (void*)(header);
    }

    void memtracker_free(void* ptr){
        Tracked_Memory* header = (Tracked_Memory*)ptr;
        if(header){
            header = header - 1u;

            header->prev->next = header->next;
            if(header->next)    header->next->prev = header->prev;
        }

        ::free(header);
    }

    void memtracker_summary(){
        LOG_INFO("-------- memtracker_summary --------");
        LOG_INFO("HEAD:     %p", &memtracker_head);
        LOG_INFO("next:     %p", memtracker_head.next);

        Tracked_Memory* ptr = memtracker_head.next;
        while(ptr){
            LOG_INFO("-- Tracked_Memory --");
            LOG_INFO("header:   %p", ptr);
            LOG_INFO("data:     %p", (Tracked_Memory*)ptr + 1u);
            LOG_INFO("prev:     %p", ptr->prev);
            LOG_INFO("next:     %p", ptr->next);
            LOG_INFO("filename: %s", ptr->filename);
            LOG_INFO("function: %s", ptr->function);
            LOG_INFO("line:     %d", ptr->line);
            LOG_INFO("bytesize: %p", ptr->bytesize);
            LOG_INFO("--");


            ptr = ptr->next;
        }
        LOG_INFO("------------------------------------");
    }

    void memtracker_leakcheck(){
        ENGINE_CHECK(memtracker_head.next == nullptr, "MEMORY LEAK !");

        Tracked_Memory* ptr = memtracker_head.next;
        while(ptr){
            LOG_ERROR("---- LEAK\nFILENAME: %s\nFUNCTION: %s\nLINE: %" PRId64 "\nBYTESIZE: %" PRId64, ptr->filename, ptr->function, ptr->line, ptr->bytesize);
            ptr = ptr->next;
        }
    }
}

#endif
