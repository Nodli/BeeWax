// TODO(hugo): (reminder) make sure there is do{}while(0) around multi-statement macro
// REF: https://stackoverflow.com/questions/3377137/conversion-to-size-t-from-int-may-change-the-sign-of-the-result-gcc-c

#ifndef H_MACRO
#define H_MACRO

    #if defined(COMPILER_MSVC)
        #define MSVC_DISABLE_WARNING(warning_number)     __pragma(warning(disable : warning_number))

        #define DISABLE_WARNING_PUSH                        __pragma(warning(push))
        #define DISABLE_WARNING_POP                         __pragma(warning(pop))

        #define DISABLE_WARNING_UNREFERENCED_PARAMETER      MSVC_DISABLE_WARNING(4100)
        #define DISABLE_WARNING_UNREFERENCED_FUNCTION       MSVC_DISABLE_WARNING(4505)
        #define DISABLE_WARNING_TYPE_PUNNING
        #define DISABLE_WARNING_UNINITIALIZED
        #define DISABLE_WARNING_SIGN_CONVERSION             MSVC_DISABLE_WARNING(4267)
        #define DISABLE_WARNING_STATIC_STRING

        #define DISABLE_OPTIMIZATIONS                       __pragma(optimize("", off))
        #define ENABLE_OPTIMIZATIONS                        __pragma(optimize("", on))

        #define FORCE_INLINE                                __forceinline

        #define PUSH_PACK                                   __pragma(pack(push, 1))
        #define POP_PACK                                    __pragma(pack(pop))

    #elif defined(COMPILER_GCC)
        #define GCC_PRAGMA_QUOTE(text)                    _Pragma(#text)
        #define GCC_DISABLE_WARNING(warning_name)         GCC_PRAGMA_QUOTE(GCC diagnostic ignored #warning_name)

        #define DISABLE_WARNING_PUSH                        GCC_PRAGMA_QUOTE(GCC diagnostic push)
        #define DISABLE_WARNING_POP                         GCC_PRAGMA_QUOTE(GCC diagnostic pop)

        #define DISABLE_WARNING_UNREFERENCED_PARAMETER      GCC_DISABLE_WARNING(-Wunused-parameter)
        #define DISABLE_WARNING_UNREFERENCED_FUNCTION       GCC_DISABLE_WARNING(-Wunused-function)
        #define DISABLE_WARNING_TYPE_PUNNING                GCC_DISABLE_WARNING(-Wstrict-aliasing)
        #define DISABLE_WARNING_UNINITIALIZED               GCC_DISABLE_WARNING(-Wuninitialized)
        #define DISABLE_WARNING_SIGN_CONVERSION             GCC_DISABLE_WARNING(-Wsign-conversion)
        #define DISABLE_WARNING_STATIC_STRING               GCC_DISABLE_WARNING(-Wwrite-strings)

        #define DISABLE_OPTIMIZATIONS                       GCC_PRAGMA_QUOTE(GCC push_options) GCC_PRAGMA_QUOTE(GCC optimize("O0"))
        #define ENABLE_OPTIMIZATIONS                        GCC_PRAGMA_QUOTE(GCC pop_options)

        #define FORCE_INLINE                                __attribute__((always_inline)) inline

        #define PUSH_PACK                                   GCC_PRAGMA_QUOTE(pack(push, 1))
        #define PUSH_POP                                    GCC_PRAGMA_QUOTE(pack(pop))

    #endif

    // NOTE(hugo): CONCATENATE must go through CONCATENATE_INTERMEDIATE so that the A and B expressions are expanded
    #define CONCATENATE_INTERMEDIATE(A, B) A ## B
    #define CONCATENATE(A, B) CONCATENATE_INTERMEDIATE(A, B)
    #define STRINGIFY_INTERMEDIATE(string) #string
    #define STRINGIFY(string) STRINGIFY_INTERMEDIATE(string)

    #define UNUSED(VARIABLE) ((void)(VARIABLE))

    #define TRUE 1
    #define FALSE 0

    #define ADD_TO_ENUM(entry_name, ...) entry_name,

    #define KILOBYTES(nKB)   ((u64) (nKB) << 10)
    #define MEGABYTES(nMB)   ((u64) (nMB) << 20)
    #define GIGABYTES(nGB)   ((u64) (nGB) << 20)

    #include "defer_macro.h"

// NOTE(hugo): undefine the macros on second include
#else

    #if defined(COMPILER_MSVC)
        #undef MSVC_DISABLE_WARNING
    #elif defined(COMPILER_GCC)
        #undef GCC_PRAGMA_QUOTE
        #undef GCC_DISABLE_WARNING
    #endif

    #undef DISABLE_WARNING_PUSH
    #undef DISABLE_WARNING_POP

    #undef DISABLE_WARNING_UNREFERENCED_PARAMETER
    #undef DISABLE_WARNING_UNREFERENCED_FUNCTION
    #undef DISABLE_WARNING_TYPE_PUNNING
    #undef DISABLE_WARNING_UNINITIALIZED
    #undef DISABLE_WARNING_SIGN_CONVERSION
    #undef DISABLE_WARNING_STATIC_STRING

    #undef DISABLE_OPTIMIZATIONS
    #undef ENABLE_OPTIMIZATIONS

    #undef FORCE_INLINE

    #undef PUSH_PACK
    #undef POP_PACK

    #undef CONCATENATE_INTERMEDIATE
    #undef CONCATENATE
    #undef STRINGIFY
    #undef UNUSED
    #undef TRUE
    #undef FALSE
    #undef ADD_TO_ENUM
    #undef KILOBYTES
    #undef MEGABYTES
    #undef GIGABYTES
    #include "defer_macro.h"

#endif
