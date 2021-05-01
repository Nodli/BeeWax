#ifndef H_TRACKED_MEMORY
#define H_TRACKED_MEMORY

// NOTE(hugo): macro required for FILENAME, __func__ and __LINE__

#if defined(DEVELOPPER_MODE)

    #define bw_malloc(bytesize)             BEEWAX_INTERNAL::memtracker_malloc(bytesize, FILENAME, __func__, __LINE__)
    #define bw_calloc(count, bytesize)      BEEWAX_INTERNAL::memtracker_calloc(count, bytesize, FILENAME, __func__, __LINE__)
    #define bw_realloc(pointer, bytesize)   BEEWAX_INTERNAL::memtracker_realloc(pointer, bytesize, FILENAME, __func__, __LINE__)
    #define bw_free(ptr)                    BEEWAX_INTERNAL::memtracker_free(ptr)

    #define DEV_Memtracker_Summary()        BEEWAX_INTERNAL::memtracker_summary()
    #define DEV_Memtracker_Leakcheck()      BEEWAX_INTERNAL::memtracker_leakcheck()

    namespace BEEWAX_INTERNAL{
        void* memtracker_malloc(size_t bytesize, const char* filename, const char* function, const u32 line);
        void* memtracker_calloc(size_t count, size_t bytesize, const char* filename, const char* function, const u32 line);
        void* memtracker_realloc(void* ptr, size_t bytesize, const char* filename, const char* function, const u32 line);
        void memtracker_free(void* ptr);
        void memtracker_summary();
        void memtracker_leakcheck();
    }

#else

    #define bw_malloc(bytesize)             ::malloc(bytesize)
    #define bw_calloc(count, bytesize)      ::calloc(count, bytesize)
    #define bw_realloc(pointer, bytesize)   ::realloc(pointer, bytesize)
    #define bw_free(ptr)                    ::free(ptr)

    #define DEV_Memtracker_Summary()
    #define DEV_Memtracker_Leakcheck()

#endif

#endif
