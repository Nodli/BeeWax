#define LOG_RAW(...)        BEEWAX_INTERNAL::raw_log_function(__VA_ARGS__)
#define LOG_TRACE(...)      BEEWAX_INTERNAL::log_function(__FILE__, __LINE__, 0, __VA_ARGS__)
#define LOG_INFO(...)       BEEWAX_INTERNAL::log_function(__FILE__, __LINE__, 1, __VA_ARGS__)
#define LOG_WARNING(...)    BEEWAX_INTERNAL::log_function(__FILE__, __LINE__, 2, __VA_ARGS__)
#define LOG_ERROR(...)      BEEWAX_INTERNAL::log_function(__FILE__, __LINE__, 3, __VA_ARGS__)

#define LOG_DISABLE_LEVEL(i)    do{ assert(i >= 0 && i < BEEWAX_INTERNAL::nlog_levels); BEEWAX_INTERNAL::log_levels_disabled[i] = true;  }while(0)
#define LOG_ENABLE_LEVEL(i)     do{ assert(i >= 0 && i < BEEWAX_INTERNAL::nlog_levels); BEEWAX_INTERNAL::log_levels_disabled[i] = false; }while(0)

void set_log_file(const char* filename);

namespace BEEWAX_INTERNAL{
    // ----------
    constexpr u32 nlog_levels = 4;
    static bool log_levels_disabled[nlog_levels]; // NOTE(hugo): static memory initialized to false
    static const char* log_levels_names[nlog_levels] = {
        "TRACE", "INFO", "WARNING", "ERROR"
    };
    static const char* log_levels_colors[nlog_levels] = {
        "\x1b[94;1m", "\x1b[32;1m", "\x1b[33;1m", "\x1b[31;1m"
    };

    constexpr u32 log_max_time_string_size = 16;

    static FILE* log_file;
    // ----------

    void raw_log_function(const char* format, ...);
    void log_function(const char* file, uint line, uint level, const char* format, ...);
}
