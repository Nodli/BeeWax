#ifndef H_DEFER_MACRO
#define H_DEFER_MACRO

// NOTE(hugo): DEFER is optimized away when using optimizations (assembly checked for gcc & msvc)
//             copying /action/ outputs smaller assembly than by ref. without optimizations

template<typename DEFER_Action>
struct DEFER_Container{
    DEFER_Container(DEFER_Action&& input_action) : action(input_action){}
    ~DEFER_Container(){ action(); }
    DEFER_Action action;
};

// NOTE(hugo): capture by copy and not reference because variable values can change between DEFER and the execution
// ex : this would free(nullptr) and leak memory with a capture by reference
// {
//     void* ptr = bw_malloc(...);
//     DEFER { bw_free(ptr) };
//
//     ptr = nullptr;
// }

#define DEFER DEFER_Container CONCATENATE(DEFER_variable_at_, __LINE__) = [=]() mutable

#endif
