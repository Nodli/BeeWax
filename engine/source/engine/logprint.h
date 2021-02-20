#ifndef H_LOGPRINT
#define H_LOGPRINT

#define LOG_RAW(...)        BEEWAX_INTERNAL::raw_log_function(__VA_ARGS__)
#define LOG_TRACE(...)      BEEWAX_INTERNAL::log_function(FILENAME, __LINE__, 0, __VA_ARGS__)
#define LOG_INFO(...)       BEEWAX_INTERNAL::log_function(FILENAME, __LINE__, 1, __VA_ARGS__)
#define LOG_WARNING(...)    BEEWAX_INTERNAL::log_function(FILENAME, __LINE__, 2, __VA_ARGS__)
#define LOG_ERROR(...)      BEEWAX_INTERNAL::log_function(FILENAME, __LINE__, 3, __VA_ARGS__)

#define LOG_LEVEL_TRACE 0u
#define LOG_LEVEL_INFO 1u
#define LOG_LEVEL_WARNING 2u
#define LOG_LEVEL_ERROR 3u

#define LOG_LEVEL_OFF(index) do{ assert(index < BEEWAX_INTERNAL::nlog_levels); BEEWAX_INTERNAL::log_levels_disabled[index] = false; }while(false)
#define LOG_LEVEL_ON(index) do{ assert(index < BEEWAX_INTERNAL::nlog_levels); BEEWAX_INTERNAL::log_levels_disabled[index] = true; }while(false)
#define LOG_SET_FILE(filename) do{ if(BEEWAX_INTERNAL::log_file){fclose(BEEWAX_INTERNAL::log_file);} BEEWAX_INTERNAL::log_file = fopen(filename, "w");}while(false);

void setup_LOG();

namespace BEEWAX_INTERNAL{
    // ----------
    constexpr u32 nlog_levels = 4;
    static bool log_levels_disabled[nlog_levels];
    static const char* log_levels_names[nlog_levels] = {
        "TRACE", "INFO", "WARNING", "ERROR"
    };

#if defined(PLATFORM_WINDOWS)
    static const char* log_levels_resetcolor = "\033[0m";
    static const char* log_levels_pathcolor = "\033[93m";
    static const char* log_levels_colors[nlog_levels] = {
        "\033[30;1m", "\033[34;1m", "\033[35;1m", "\033[31;1m"
    };
#elif defined(PLATFORM_LINUX)
    static const char* log_levels_resetcolor = "\x1b[0m";
    static const char* log_levels_pathcolor = "\x1b[93m";
    static const char* log_levels_colors[nlog_levels] = {
        "\x1b[30;1m", "\x1b[34;1m", "\x1b[35;1m", "\x1b[31;1m"
    };
#else
    static const char* log_levels_resetcolor = "";
    static const char* log_levels_pathcolor = "";
    static const char* log_levels_colors[nlog_levels] = {
        "", "", "", ""
    };
#endif

    constexpr u32 log_max_time_string_size = 16;

    static FILE* log_file;
    // ----------

    void raw_log_function(const char* format, ...);
    void log_function(const char* file, uint line, uint level, const char* format, ...);
}

#endif
