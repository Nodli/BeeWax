#ifndef H_DEFER_MACRO
#define H_DEFER_MACRO

// NOTE(hugo): DEFER is optimized away when using optimizations (assembly checked for gcc & msvc)
//             copying /action/ outputs smaller assembly than by ref. without optimizations

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

#else

#undef DEFER

#endif
