#ifndef H_DEBUG_TOOLS
#define H_DEBUG_TOOLS

#if !defined(DEBUG_BUILD)

// ---- debug break
#define DEV_debug_break()

// ---- assert
// REF(hugo):
// http://cnicholson.net/2009/02/stupid-c-tricks-adventures-in-assert/
// http://pulsarengine.com/2008/11/assert/
#define assert(condition)   do{ (void)sizeof(condition); }while(false)

#else

// ---- debug break
#if defined(COMPILER_MSVC)
    #define DEV_debug_break() do{ __debugbreak(); }while(false)
#elif defined(COMPILER_GCC)
    #define DEV_debug_break() do{ raise(SIGTRAP); }while(false)
#else
    static_assert("debug_break.h is not implemented for this compiler");
#endif

// ---- assert
#define assert(condition)                                               \
do{                                                                     \
    if(!(condition)){                                                   \
        LOG_ERROR("FAILED assert: " #condition);                        \
        DEV_debug_break();                                              \
    }                                                                   \
}while(false)

#endif

#endif
