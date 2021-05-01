namespace BEEWAX_INTERNAL{
    void LOG_function_raw(char* format, ...);
    void LOG_function(const char* file, uint line, uint LOG_LEVEL, const char* format, ...);

    // ---------- data

    static bool LOG_LEVEL_status[] = {true, true, true, true};

    static const char* LOG_LEVEL_name[] = {"TRACE", "INFO", "WARNING", "ERROR"};

#if defined(PLATFORM_WINDOWS) && !defined(LOG_NO_COLOR)
    static const char* LOG_resetcolor = "\033[0m";
    static const char* LOG_pathcolor = "\033[93m";
    static const char* LOG_LEVEL_color[] = {"\033[30;1m", "\033[34;1m", "\033[35;1m", "\033[31;1m"};

#elif defined(PLATFORM_LINUX) && !defined(LOG_NO_COLOR)
    static const char* LOG_resetcolor = "\x1b[0m";
    static const char* LOG_pathcolor = "\x1b[93m";
    static const char* LOG_LEVEL_color[] = {"\x1b[30;1m", "\x1b[34;1m", "\x1b[35;1m", "\x1b[31;1m"};

#else
    static const char* LOG_resetcolor = "";
    static const char* LOG_pathcolor = "";
    static const char* LOG_LEVEL_color[] = {"", "", "", ""};

#endif

    static FILE* LOG_FILE_ptr = nullptr;
}

void setup_LOG(){
#if defined(PLATFORM_WINDOWS)
    HANDLE stdo = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD stdo_mode;
    GetConsoleMode(stdo, &stdo_mode);
    SetConsoleMode(stdo, stdo_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN);
#endif
}

void LOG_LEVEL_enable(u32 LOG_LEVEL){
    if(LOG_LEVEL < 4u) BEEWAX_INTERNAL::LOG_LEVEL_status[LOG_LEVEL] = true;
}

void LOG_DISABLE_LEVEL(u32 LOG_LEVEL){
    if(LOG_LEVEL < 4u) BEEWAX_INTERNAL::LOG_LEVEL_status[LOG_LEVEL] = false;
}

void LOG_FILE(const char* filename){
    if(BEEWAX_INTERNAL::LOG_FILE_ptr) fclose(BEEWAX_INTERNAL::LOG_FILE_ptr);
    if(filename) BEEWAX_INTERNAL::LOG_FILE_ptr = fopen(filename, "w");
}

namespace BEEWAX_INTERNAL{

    void LOG_function_raw(const char* format, ...){
        // NOTE(hugo): stderr
        {
            va_list args;
            va_start(args, format);
            vfprintf(stderr, format, args);
            va_end(args);

            fprintf(stderr, "\n");
            fflush(stderr);
        }

        // NOTE(hugo): file
        if(BEEWAX_INTERNAL::LOG_FILE_ptr){
            va_list args;
            va_start(args, format);
            vfprintf(BEEWAX_INTERNAL::LOG_FILE_ptr, format, args);
            va_end(args);

            fprintf(BEEWAX_INTERNAL::LOG_FILE_ptr, "\n");
            fflush(BEEWAX_INTERNAL::LOG_FILE_ptr);
        }
    }

    void LOG_function(const char* file, u32 line, u32 LOG_LEVEL, const char* format, ...){
        if(LOG_LEVEL_status[LOG_LEVEL]){
            time_t current_time = time(NULL);
            struct tm* local_time = localtime(&current_time);

            char time_string[128u];
            size_t time_length = strftime(time_string, sizeof(time_string), "%H:%M:%S", local_time);
            time_string[time_length] = '\0';

            // NOTE(hugo): stderr
            {
                fprintf(stdout, "%s%s %s%-7s %s%s:%-5d: %s",
                        LOG_resetcolor, time_string, LOG_LEVEL_color[LOG_LEVEL], LOG_LEVEL_name[LOG_LEVEL], LOG_pathcolor, file, line, LOG_resetcolor);

                va_list args;
                va_start(args, format);
                vfprintf(stdout, format, args);
                va_end(args);

                fprintf(stdout, "\n");
                if(LOG_LEVEL == LOG_LEVEL_TRACE || LOG_LEVEL == LOG_LEVEL_ERROR) fflush(stderr);
            }

            // NOTE(hugo): file
            if(BEEWAX_INTERNAL::LOG_FILE_ptr){
                fprintf(BEEWAX_INTERNAL::LOG_FILE_ptr, "%s %-7s %s:%-5d", time_string, LOG_LEVEL_name[LOG_LEVEL], file, line);

                va_list args;
                va_start(args, format);
                vfprintf(BEEWAX_INTERNAL::LOG_FILE_ptr, format, args);
                va_end(args);

                fprintf(BEEWAX_INTERNAL::LOG_FILE_ptr, "\n");
                if(LOG_LEVEL == LOG_LEVEL_TRACE || LOG_LEVEL == LOG_LEVEL_ERROR) fflush(BEEWAX_INTERNAL::LOG_FILE_ptr);
            }
        }
    }

}
