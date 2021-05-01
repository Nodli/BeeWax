#ifndef H_DEBUG_TOOLS
#define H_DEBUG_TOOLS

// NOTE(hugo): macros required to break / assert at the macro position

// REF(hugo):
// http://cnicholson.net/2009/02/stupid-c-tricks-adventures-in-assert/
// http://pulsarengine.com/2008/11/assert/

#if defined(DEBUG_BUILD)

    #if defined(COMPILER_MSVC)
        #define DEV_debug_break() do{ __debugbreak(); }while(false)
    #elif defined(COMPILER_GCC)
        #define DEV_debug_break() do{ raise(SIGTRAP); }while(false)
    #endif

    #define assert(condition)                                               \
    do{                                                                     \
        if(!(condition)){                                                   \
            LOG_ERROR("FAILED assert: " #condition);                        \
            DEV_debug_break();                                              \
        }                                                                   \
    }while(false)

#else

    #define DEV_debug_break()

    #define assert(condition)   do{ (void)sizeof(condition); }while(false)

#endif

#endif
