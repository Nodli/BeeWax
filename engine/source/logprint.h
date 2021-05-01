#ifndef H_LOGPRINT
#define H_LOGPRINT

void setup_LOG();

constexpr u32 LOG_LEVEL_TRACE = 0u;
constexpr u32 LOG_LEVEL_INFO = 1u;
constexpr u32 LOG_LEVEL_WARNING = 2u;
constexpr u32 LOG_LEVEL_ERROR = 3u;

void LOG_LEVEL_enable(u32 LOG_LEVEL);
void LOG_LEVEL_disable(u32 LOG_LEVEL);
void LOG_FILE(const char* filename);

#define LOG_RAW(...)        BEEWAX_INTERNAL::LOG_function_raw(__VA_ARGS__)
#define LOG_TRACE(...)      BEEWAX_INTERNAL::LOG_function(FILENAME, __LINE__, 0, __VA_ARGS__)
#define LOG_INFO(...)       BEEWAX_INTERNAL::LOG_function(FILENAME, __LINE__, 1, __VA_ARGS__)
#define LOG_WARNING(...)    BEEWAX_INTERNAL::LOG_function(FILENAME, __LINE__, 2, __VA_ARGS__)
#define LOG_ERROR(...)      BEEWAX_INTERNAL::LOG_function(FILENAME, __LINE__, 3, __VA_ARGS__)

namespace BEEWAX_INTERNAL{
    void LOG_function_raw(const char* format, ...);
    void LOG_function(const char* file, u32 line, u32 LOG_LEVEL, const char* format, ...);
};
#endif
