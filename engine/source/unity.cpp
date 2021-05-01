#if defined(APPLICATION_SETTINGS)
    #include APPLICATION_SETTINGS
#endif

#include "unity_header.h"
#include "unity_source.cpp"
#include "engine_main.cpp"
#include "engine.cpp"

// ---- post-engine include

#if defined(APPLICATION_UNITY)
    #include APPLICATION_UNITY

#else
    void    user_config()       { printf("WARNING: no user_config()\n"); };
    void*   user_create()       { printf("WARNING: no user_create()\n"); return nullptr;};
    void    user_destroy(void*) { printf("WARNING: no user_destroy()\n"); };

#endif
