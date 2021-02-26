#ifndef H_ERROR_HANDLING
#define H_ERROR_HANDLING

static char CRASH_sprintf_scratch[2048];

#define SELECT_NARG(MACRO_PREFIX, ARG5, ARG4, ARG3, ARG2, ARG1, MACRO_SUFFIX, ...) MACRO_PREFIX ## _ ## MACRO_SUFFIX

#if defined(PLATFORM_LAYER_SDL)
    #define CRASH(MESSAGE_FORMAT, ...)                                                                          \
    do{                                                                                                         \
        sprintf(CRASH_sprintf_scratch, "FILE: %s\nLINE: %u\n" MESSAGE_FORMAT, FILENAME, __LINE__, ##__VA_ARGS__); \
        LOG_ERROR("\n%s", CRASH_sprintf_scratch);                                                               \
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", CRASH_sprintf_scratch, NULL);                   \
        abort();                                                                                                \
    }while(false)
#else
    #define CRASH(...)              \
    do{                             \
        LOG_ERROR(__VA_ARGS__);     \
        LOG_FLUSH();                \
        abort();                    \
    }while(false)
#endif

// ---- ENGINE_CHECK

#define ENGINE_CHECK(EXPRESSION, MESSAGE_FORMAT, ...)                                                       \
do{                                                                                                         \
    bool result = (bool)(EXPRESSION);                                                                       \
    if(result != true){                                                                                     \
        CRASH("**ENGINE ERROR**\nEXPRESSION: %s\nMESSAGE: " MESSAGE_FORMAT, #EXPRESSION, ##__VA_ARGS__);    \
    }                                                                                                       \
}while(false)


// ---- SDL_CHECK

#if defined(PLATFORM_LAYER_SDL)
    #define SDL_CHECK(EXPRESSION)                                                       \
	do                                                                                  \
	{                                                                                   \
		bool result = (bool)(EXPRESSION);                                               \
		if (result != true){                                                            \
			CRASH("**SDL ERROR**: %s\nEXPRESSION: %s", SDL_GetError(), #EXPRESSION);    \
		}                                                                               \
	} while(false)
#endif

// ---- VK_CHECK

#if defined(RENDERER_VULKAN)
    #define VK_CHECK(EXPRESSION)                                                                        \
	do                                                                                                  \
	{                                                                                                   \
		VkResult result = (EXPRESSION);                                                                 \
		if (result != VK_SUCCESS){                                                                      \
			CRASH("**VULKAN ERROR**: %s\nEXPRESSION: %s", VK::result_as_string(result), #EXPRESSION);   \
		}                                                                                               \
	} while(false)
#endif

#endif
