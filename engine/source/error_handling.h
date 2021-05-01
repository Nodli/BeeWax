#ifndef H_ERROR_HANDLING
#define H_ERROR_HANDLING

// NOTE(hugo): using extern and not static here because the variable is used by the macro ie needs to be available in other compilation units
namespace BEEWAX_INTERNAL{
    extern char CRASH_sprintf_scratch[2048];
}

#define ERROR_HANDLING_CRASH(MESSAGE_FORMAT, ...)                                                                               \
do{                                                                                                                             \
    sprintf(BEEWAX_INTERNAL::CRASH_sprintf_scratch, "FILE: %s\nLINE: %u\n" MESSAGE_FORMAT, FILENAME, __LINE__, ##__VA_ARGS__);  \
    LOG_ERROR("\n%s", BEEWAX_INTERNAL::CRASH_sprintf_scratch);                                                                  \
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", BEEWAX_INTERNAL::CRASH_sprintf_scratch, NULL);                      \
    DEV_debug_break();                                                                                                          \
}while(false)

// --

#define ENGINE_CHECK(EXPRESSION, MESSAGE_FORMAT, ...)                                                                   \
do{                                                                                                                     \
    bool result = (bool)(EXPRESSION);                                                                                   \
    if(result != true){                                                                                                 \
        ERROR_HANDLING_CRASH("**ENGINE ERROR**\nEXPRESSION: %s\nMESSAGE: " MESSAGE_FORMAT, #EXPRESSION, ##__VA_ARGS__); \
    }                                                                                                                   \
}while(false)

#define SDL_CHECK(EXPRESSION)                                                                               \
do                                                                                                          \
{                                                                                                           \
    bool result = (bool)(EXPRESSION);                                                                       \
    if (result != true){                                                                                    \
        ERROR_HANDLING_CRASH("**SDL ERROR**\nEXPRESSION: %s\nMESSAGE: %s", #EXPRESSION, SDL_GetError());    \
    }                                                                                                       \
} while(false)

#endif
