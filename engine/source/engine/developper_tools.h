// NOTE(hugo): nothing is thread safe as is
// a mutex could be used in the initialization part as it's only executed once

#if !defined(USE_DEVELOPPER_MODE)

#define DEV_INITIALIZE
#define DEV_NEXT_FRAME
#define DEV_TERMINATE

#define DEV_TIMED_BLOCK
#define DEV_DISPLAY_TIMING_ENTRIES
#define DEV_LOG_FRAME_TIME

#define DEV_DEBUG_RENDERER

#define DEV_TWEAKABLE(TYPE, NAME, DEFAULT_VALUE) DEFAULT_VALUE
#define DEV_DISPLAY_TWEAKABLE_ENTRIES

#else

// ---- initialization

#define DEV_INITIALIZE                                                                  \
do{                                                                                     \
    BEEWAX_INTERNAL::DEV_import_tweakable_entries_from_file("./data/tweakable.txt");    \
}while(0)

#define DEV_NEXT_FRAME                              \
do{                                                 \
    BEEWAX_INTERNAL::DEV_timing_events.clear();     \
}while(0)

#define DEV_TERMINATE                                                               \
do{                                                                                 \
    BEEWAX_INTERNAL::DEV_timing_entries.free();                                     \
    BEEWAX_INTERNAL::DEV_timing_events.free();                                      \
    BEEWAX_INTERNAL::DEV_export_tweakable_entries_to_file("./data/tweakable.txt");  \
    free(BEEWAX_INTERNAL::DEV_tweakable_file_mapping);                              \
    BEEWAX_INTERNAL::DEV_tweakable_file_mapping = nullptr;                          \
    BEEWAX_INTERNAL::DEV_tweakable_entries.free();                                  \
}while(0)

// ---- timing

namespace BEEWAX_INTERNAL{
    struct DEV_Timing_Entry{
        const char* file = nullptr;
        const char* function = nullptr;
        u32 line = 0u;
        u32 hit_count = 0u;
        u64 cycle_counter = 0u;
    };

    struct DEV_Timing_Event{
        u64 cycle_counter = 0u;
        u32 entry_index = 0u;
    };

    constexpr u16 DEV_timing_chunk_size = 256;
    static dchunkarena<DEV_Timing_Entry, DEV_timing_chunk_size> DEV_timing_entries;
    static dchunkarena<DEV_Timing_Event, DEV_timing_chunk_size> DEV_timing_events;

    static DEV_Timing_Entry* DEV_get_new_timed_block(const char* file, const char* function, u32 line){
        DEV_Timing_Entry* entry = BEEWAX_INTERNAL::DEV_timing_entries.get();
        entry->file = file;
        entry->function = function;
        entry->line = line;
        return entry;
    };

    void DEV_LOG_timing_entries(){
        u32 nentries = 0u;
        dchunkarena<DEV_Timing_Entry, BEEWAX_INTERNAL::DEV_timing_chunk_size>::chunk* current_chunk = BEEWAX_INTERNAL::DEV_timing_entries.head;

        auto display_chunk = [&](u32 entries_to_display){
            for(u32 ientry = 0u; ientry != entries_to_display; ++ientry){
                DEV_Timing_Entry& entry = current_chunk->data[ientry];
                LOG_RAW("DEV_Timing_Entry [%d]: %s HITS(%d) CYCLES(%d) AVG(%d)", nentries + ientry, entry.function, entry.hit_count, entry.cycle_counter, entry.cycle_counter / entry.hit_count);
                ++nentries;
            }
        };

        if(current_chunk && BEEWAX_INTERNAL::DEV_timing_entries.current_chunk_space != 0u){
            display_chunk(BEEWAX_INTERNAL::DEV_timing_chunk_size - BEEWAX_INTERNAL::DEV_timing_entries.current_chunk_space);
            current_chunk = current_chunk->next;
        }

        while(current_chunk){
            display_chunk(BEEWAX_INTERNAL::DEV_timing_chunk_size);
            current_chunk = current_chunk->next;
        }
    }

    struct DEV_Timing_Container{
        DEV_Timing_Container(DEV_Timing_Entry* ientry) : entry(ientry){
            start_cycles = cycle_counter();
        }
        ~DEV_Timing_Container(){
            ++(entry->hit_count);
            entry->cycle_counter += (cycle_counter() - start_cycles);
        };

        DEV_Timing_Entry* entry;
        u64 start_cycles;
    };
}

#define DEV_TIMED_BLOCK                                                                                                                                     \
static BEEWAX_INTERNAL::DEV_Timing_Entry* CONCATENATE(DEV_timing_entry_ptr_variable_at_, __LINE__) = nullptr;                                               \
if(!CONCATENATE(DEV_timing_entry_ptr_variable_at_, __LINE__)){                                                                                              \
    CONCATENATE(DEV_timing_entry_ptr_variable_at_, __LINE__) = BEEWAX_INTERNAL::DEV_get_new_timed_block(__FILE__, __func__, __LINE__);                      \
}                                                                                                                                                           \
BEEWAX_INTERNAL::DEV_Timing_Container CONCATENATE(DEV_timing_container_variable_at_, __LINE__)(CONCATENATE(DEV_timing_entry_ptr_variable_at_, __LINE__));

#define DEV_DISPLAY_TIMING_ENTRIES do{ BEEWAX_INTERNAL::DEV_LOG_timing_entries(); }while(0)

#define DEV_LOG_FRAME_TIME                                                                  \
double CONCATENATE(DEV_frame_start_time_at_, __LINE__) = timer_seconds();                       \
DEFER{                                                                                          \
    double DEV_frame_time = timer_seconds() - CONCATENATE(DEV_frame_start_time_at_, __LINE__);   \
    LOG_RAW("DEV_Frame_Time: %f ms, %f FPS", DEV_frame_time, 1. / DEV_frame_time);              \
};

// ---- debug rendering

#define DEV_DEBUG_RENDERER do{ GL::set_debug_message_callback(); }while(0)

// ---- tweakable

namespace BEEWAX_INTERNAL{
    enum DEV_Tweakable_Type{
        TWEAKABLE_BOOLEAN,
        TWEAKABLE_INTEGER,
        TWEAKABLE_REAL,
        TWEAKABLE_STRING,
    };

    union DEV_Tweakable_Value{
        bool as_BOOLEAN;
        s32 as_INTEGER;
        float as_REAL;
        char* as_STRING;
    };

    struct DEV_Tweakable_Entry{
        const char* name;
        DEV_Tweakable_Type type;
        DEV_Tweakable_Value value;
    };

    constexpr u16 DEV_tweakable_chunk_size = 256;
    static dchunkarena<DEV_Tweakable_Entry, DEV_tweakable_chunk_size> DEV_tweakable_entries;

    static char* DEV_tweakable_file_mapping = nullptr;

    static DEV_Tweakable_Entry* DEV_get_new_or_existing_tweakable(const char* name, DEV_Tweakable_Type type, DEV_Tweakable_Value default_value){

        // NOTE(hugo): try to find a tweakable with this name and type
        dchunkarena<DEV_Tweakable_Entry, DEV_tweakable_chunk_size>::chunk* current_chunk = BEEWAX_INTERNAL::DEV_tweakable_entries.head;

        if(current_chunk && BEEWAX_INTERNAL::DEV_tweakable_entries.current_chunk_space != 0u){
            u32 nentries = BEEWAX_INTERNAL::DEV_tweakable_chunk_size - BEEWAX_INTERNAL::DEV_tweakable_entries.current_chunk_space;
            for(u32 ientry = 0u; ientry != nentries; ++ientry){
                DEV_Tweakable_Entry* entry = &current_chunk->data[ientry];
                if(entry->type == type && strcmp(entry->name, name) == 0u){
                    return entry;
                }
            }
            current_chunk = current_chunk->next;
        }

        while(current_chunk){
            for(u32 ientry = 0u; ientry != BEEWAX_INTERNAL::DEV_tweakable_chunk_size; ++ientry){
                DEV_Tweakable_Entry* entry = &current_chunk->data[ientry];
                if(entry->type == type && strcmp(entry->name, name) == 0u){
                    return entry;
                }
            }
            current_chunk = current_chunk->next;
        }

        // NOTE(hugo): no relevant tweakable was found ie make a new one
        DEV_Tweakable_Entry* new_entry = DEV_tweakable_entries.get();

        new_entry->name = name;
        new_entry->type = type;
        new_entry->value = default_value;

        return new_entry;
    }

    static void DEV_import_tweakable_entries_from_file(const char* filename){

        // NOTE(hugo): read entire file
        FILE* file = fopen(filename, "r");
        if(file == NULL){
            LOG_ERROR("DEV_import_tweakable_entries_from_file(%s) FAILED - aborting import", filename);
            return;
        }

        s64 start_index = ftell(file);
        fseek(file, 0, SEEK_END);
        s64 end_index = ftell(file);
        fseek(file, start_index, SEEK_SET);

        s64 file_size = end_index - start_index;
        if(file_size == 0u){
            LOG_ERROR("tweakable file %s is empty", filename);
            return;
        }

        DEV_tweakable_file_mapping = (char*)malloc(file_size);
        fread(DEV_tweakable_file_mapping, 1u, file_size, file);

        fclose(file);

        // NOTE(hugo): parse entries
        char* cursor = DEV_tweakable_file_mapping;
        while((cursor - DEV_tweakable_file_mapping) < file_size){
            DEV_Tweakable_Entry* entry = DEV_tweakable_entries.get();

            // NOTE(hugo): name
            entry->name = cursor;
            while(*cursor != ' ') ++cursor;
            *cursor = '\0';
            ++cursor;

            // NOTE(hugo): type and value
            if(memcmp(cursor, "BOOLEAN", 7u) == 0){
                entry->type = TWEAKABLE_BOOLEAN;
                while(*cursor != ' ') ++cursor;
                ++cursor;

                if(memcmp(cursor, "true", 4u) == 0){
                    entry->value.as_BOOLEAN = true;
                }else{
                    entry->value.as_BOOLEAN = false;
                }
                while(*cursor != '\n' && *cursor != EOF) ++cursor;

            }else if(memcmp(cursor, "INTEGER", 7u) == 0){
                entry->type = TWEAKABLE_INTEGER;
                while(*cursor != ' ') ++cursor;
                ++cursor;

                sscanf(cursor, "%d", &entry->value.as_INTEGER);
                while(*cursor != '\n' && *cursor != EOF) ++cursor;

            }else if(memcmp(cursor, "REAL", 4u) == 0){
                entry->type = TWEAKABLE_REAL;
                while(*cursor != ' ') ++cursor;
                ++cursor;

                sscanf(cursor, "%f", &entry->value.as_REAL);
                while(*cursor != '\n' && *cursor != EOF) ++cursor;

            }else if(memcmp(cursor, "STRING", 6u) == 0){
                entry->type = TWEAKABLE_STRING;
                while(*cursor != ' ') ++cursor;
                ++cursor;

                entry->value.as_STRING = cursor;
                while(*cursor != '\n' && *cursor != EOF) ++cursor;
                *cursor = '\0';

            }else{
                LOG_ERROR("unknown type in tweakable file %s %.*s", filename, cursor, 10u);
                assert(false);
                return;
            }
            ++cursor;
        }
    }

    static void DEV_export_tweakable_entries_to_file(const char* filename){
        dchunkarena<DEV_Tweakable_Entry, DEV_tweakable_chunk_size>::chunk* current_chunk = BEEWAX_INTERNAL::DEV_tweakable_entries.head;

        FILE* file = fopen(filename, "w");
        if(file == NULL){
            LOG_ERROR("DEV_export_tweakable_entries_to_file(%s) FAILED - aborting export", filename);
            return;
        }

        auto write_entry_to_file = [](FILE* file, DEV_Tweakable_Entry* entry){
            // NOTE(hugo): name
            fprintf(file, "%s ", entry->name);

            // NOTE(hugo): type and value
            switch(entry->type){
                case TWEAKABLE_BOOLEAN:
                    fprintf(file, "%s %s\n", "BOOLEAN", entry->value.as_BOOLEAN ? "true" : "false");
                    break;
                case TWEAKABLE_INTEGER:
                    fprintf(file, "%s %d\n", "INTEGER", entry->value.as_INTEGER);
                    break;
                case TWEAKABLE_REAL:
                    fprintf(file, "%s %f\n", "REAL", entry->value.as_REAL);
                    break;
                case TWEAKABLE_STRING:
                    fprintf(file, "%s %s\n", "STRING", entry->value.as_STRING);
                    break;
                default:
                    LOG_ERROR("entry->type with value %d is missing", entry->type);
                    assert(false);
                    break;
            }
        };

        if(current_chunk && BEEWAX_INTERNAL::DEV_tweakable_entries.current_chunk_space != 0u){
            u32 nentries = BEEWAX_INTERNAL::DEV_tweakable_chunk_size - BEEWAX_INTERNAL::DEV_tweakable_entries.current_chunk_space;
            for(u32 ientry = 0u; ientry != nentries; ++ientry){
                DEV_Tweakable_Entry* entry = &current_chunk->data[ientry];
                write_entry_to_file(file, entry);
            }
            current_chunk = current_chunk->next;
        }

        while(current_chunk){
            for(u32 ientry = 0u; ientry != BEEWAX_INTERNAL::DEV_tweakable_chunk_size; ++ientry){
                DEV_Tweakable_Entry* entry = &current_chunk->data[ientry];
                write_entry_to_file(file, entry);
            }
            current_chunk = current_chunk->next;
        }

        fclose(file);
    }

    void DEV_LOG_tweakable_entries(){
        u32 nentries = 0u;
        dchunkarena<DEV_Tweakable_Entry, BEEWAX_INTERNAL::DEV_tweakable_chunk_size>::chunk* current_chunk = BEEWAX_INTERNAL::DEV_tweakable_entries.head;

        auto display_chunk = [&](u32 entries_to_display){
            for(u32 ientry = 0u; ientry != entries_to_display; ++ientry){
                DEV_Tweakable_Entry* entry = &current_chunk->data[ientry];
                switch(entry->type){
                    case TWEAKABLE_BOOLEAN:
                        LOG_RAW("DEV_Tweakable_Entry [%d]: %s BOOLEAN %s", nentries, entry->name, entry->value.as_BOOLEAN ? "true" : "false");
                        break;
                    case TWEAKABLE_INTEGER:
                        LOG_RAW("DEV_Tweakable_Entry [%d]: %s INTEGER %d", nentries, entry->name, entry->value.as_INTEGER);
                        break;
                    case TWEAKABLE_REAL:
                        LOG_RAW("DEV_Tweakable_Entry [%d]: %s REAL %f", nentries, entry->name, entry->value.as_REAL);
                        break;
                    case TWEAKABLE_STRING:
                        LOG_RAW("DEV_Tweakable_Entry [%d]: %s STRING %s", nentries, entry->name, entry->value.as_STRING);
                        break;
                    default:
                        LOG_ERROR("DEV_Tweakable_Entry [%d]: entry->type with value %d is missing", nentries, entry->type);
                        assert(false);
                        break;
                }
                ++nentries;
            }
        };

        if(current_chunk && BEEWAX_INTERNAL::DEV_tweakable_entries.current_chunk_space != 0u){
            display_chunk(BEEWAX_INTERNAL::DEV_tweakable_chunk_size - BEEWAX_INTERNAL::DEV_tweakable_entries.current_chunk_space);
            current_chunk = current_chunk->next;
        }

        while(current_chunk){
            display_chunk(BEEWAX_INTERNAL::DEV_tweakable_chunk_size);
            current_chunk = current_chunk->next;
        }
    }

};

#define DEV_TWEAKABLE(TYPE, NAME, DEFAULT_VALUE)                                                    \
[](){                                                                                               \
    using namespace BEEWAX_INTERNAL;                                                                \
    static DEV_Tweakable_Entry* entry = nullptr;                                                    \
    if(!entry){                                                                                     \
        DEV_Tweakable_Value default_value;                                                          \
        DISABLE_WARNING_PUSH                                                                        \
        DISABLE_WARNING_STATIC_STRING                                                               \
        CONCATENATE(default_value.as_, TYPE) = DEFAULT_VALUE;                                       \
        DISABLE_WARNING_POP                                                                         \
        entry = DEV_get_new_or_existing_tweakable(NAME, CONCATENATE(TWEAKABLE_, TYPE), default_value);  \
    }                                                                                               \
    return CONCATENATE(entry->value.as_, TYPE);                                                     \
}()

#define DEV_DISPLAY_TWEAKABLE_ENTRIES do{ BEEWAX_INTERNAL::DEV_LOG_tweakable_entries(); }while(0)

#endif
