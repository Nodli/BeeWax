void set_log_file(const char* filename){
    assert(filename);
    if(BEEWAX_INTERNAL::log_file){
        fclose(BEEWAX_INTERNAL::log_file);
    }
    BEEWAX_INTERNAL::log_file = fopen(filename, "w");
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

            fprintf(stderr, "\x1b[0m" "%s %s%-7s" "\x1b[0m" "\x1b[93m"  " %s:%-5d: " "\x1b[0m", time_string, log_levels_colors[level], log_levels_names[level], file, line);

            va_list args;
            va_start(args, format);
            vfprintf(stderr, format, args);
            va_end(args);

            fprintf(stderr, "\n");
            fflush(stderr);
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
            fflush(log_file);
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
