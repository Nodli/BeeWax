void setup_LOG(){
#if defined(PLATFORM_WINDOWS)
    HANDLE stdo = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD stdo_mode;
    GetConsoleMode(stdo, &stdo_mode);
    SetConsoleMode(stdo, stdo_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN);
#endif
}

void BEEWAX_INTERNAL::log_function(const char* file, uint line, uint level, const char* format, ...){
    if(!log_levels_disabled[level]){
        time_t current_time = time(NULL);
        struct tm* local_time = localtime(&current_time);

        // NOTE(hugo): stderr
        {
            char time_string[log_max_time_string_size];
            size_t time_length = strftime(time_string, sizeof(time_string), "%H:%M:%S", local_time);
            time_string[time_length] = '\0';

            fprintf(stdout, "%s%s %s%-7s %s%s:%-5d: %s",
                    log_levels_resetcolor, time_string, log_levels_colors[level], log_levels_names[level], log_levels_nocolor, file, line, log_levels_resetcolor);

            va_list args;
            va_start(args, format);
            vfprintf(stdout, format, args);
            va_end(args);

            fprintf(stdout, "\n");
            if(level == LOG_LEVEL_ERROR || level == LOG_LEVEL_TRACE) fflush(stderr);
        }

        // NOTE(hugo): file
        if(log_file){
            char time_string[log_max_time_string_size];
            size_t time_length = strftime(time_string, sizeof(time_string), "%H:%M:%S", local_time);
            time_string[time_length] = '\0';

            fprintf(log_file, "%s %-7s %s:%-5d", time_string, log_levels_names[level], file, line);

            va_list args;
            va_start(args, format);
            vfprintf(log_file, format, args);
            va_end(args);

            fprintf(log_file, "\n");
            if(level == LOG_LEVEL_ERROR || level == LOG_LEVEL_TRACE) fflush(log_file);
        }
    }
}

void BEEWAX_INTERNAL::raw_log_function(const char* format, ...){
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
    if(log_file){
        va_list args;
        va_start(args, format);
        vfprintf(log_file, format, args);
        va_end(args);

        fprintf(log_file, "\n");
        fflush(log_file);
    }
}
