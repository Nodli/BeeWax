#if !defined(DEVELOPPER_MODE)

#define DEV_debug_break();

#else

FORCE_INLINE void DEV_debug_break(){
#if defined(COMPILER_MSVC)
    __debugbreak();

#elif defined(COMPILER_GCC)
    raise(SIGTRAP);

#else
    static_assert("debug_break.h is not implemented for this compiler");

#endif
}

#endif
