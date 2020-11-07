#ifndef H_DEFER_MACRO
#define H_DEFER_MACRO

// NOTE(hugo): passing the action by copy is
// - the same when using O2 (GCC & MSVC)
// - faster when compiling without optimizations

struct DEFER_Creator{
};

template<typename DEFER_Action>
struct DEFER_Temporary{
    DEFER_Action action;
};

template<typename DEFER_Action>
struct DEFER_Container{
    DEFER_Container(DEFER_Temporary<DEFER_Action> temp) : action(temp.action){}
    ~DEFER_Container(){
        action();
    }

    DEFER_Action action;
};

template<typename DEFER_Action>
DEFER_Temporary<DEFER_Action> operator+(DEFER_Creator, DEFER_Action&& action){
    return DEFER_Temporary<DEFER_Action>{action};
};

// NOTE(hugo): capture by copy and not reference because variable values can change between DEFER and the execution
// ex : this would free(nullptr) and leak memory with a capture by reference
// {
//     void* ptr = malloc(...);
//     DEFER { free(ptr) };
//
//     ptr = nullptr;
// }
#define DEFER DEFER_Container CONCATENATE(DEFER_variable_at_, __LINE__) = DEFER_Creator() + [=]() mutable

// NOTE(hugo): outputs the same assembly as the raw function calls when using O1 (GCC & MSVC) by using :
// - operator,(A, B) returns the rightmost argument even if the types of A and B are different
// - shortcircuiting ||
#define DECORATE(BEGIN, END)                                            \
for(s32 CONCATENATE(DECORATE_variable_at_, __LINE__) = (BEGIN(), 0);    \
    !CONCATENATE(DECORATE_variable_at_, __LINE__);                      \
    ++CONCATENATE(DECORATE_variable_at_, __LINE__), END())

#else

#undef DEFER
#undef DECORATE(BEGIN, END)

#endif
