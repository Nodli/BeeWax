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

    #else
        static_assert("macro.h is not implemented for this compiler")

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

    #define KILOBYTES(nKB)   ((u64) (nKB) * 1024u)
    #define MEGABYTES(nMB)   ((u64) (nMB) * 1024u * 1024u)
    #define GIGABYTES(nGB)   ((u64) (nGB) * 1024u * 1024u * 1024u)

    // NOTE(hugo): outputs the same assembly as the raw function calls when using O1 (GCC & MSVC) by using :
    // - operator,(A, B) returns the rightmost argument even if the types of A and B are different
    // - shortcircuiting ||
    #define DECORATE(BEGIN, END)                                            \
    for(s32 CONCATENATE(DECORATE_variable_at_, __LINE__) = (BEGIN(), 0);    \
        !CONCATENATE(DECORATE_variable_at_, __LINE__);                      \
        ++CONCATENATE(DECORATE_variable_at_, __LINE__), END())

    #define FILENAME                                                            \
    [](){                                                                       \
        const char* filestr = "/\\" __FILE__;                                   \
        const char* ptrA = strrchr(filestr, '/') + 1u;                          \
        const char* ptrB = strrchr(filestr, '\\') + 1u;                         \
        return ptrA > ptrB ? ptrA : ptrB;                                       \
    }()

    #define DECLARE_EQUALITY_OPERATOR(TYPE)                 \
    bool operator==(const TYPE & lhs, const TYPE & rhs);    \
    bool operator!=(const TYPE & lhs, const TYPE & rhs);

    #define DEFINE_EQUALITY_OPERATOR(TYPE)                                                                              \
    bool operator==(const TYPE & lhs, const TYPE & rhs){return memcmp((void*)&lhs, (void*)&rhs, sizeof( TYPE )) == 0;}  \
    bool operator!=(const TYPE & lhs, const TYPE & rhs){return !(lhs == rhs);}

    #include "defer_macro.h"

#endif
