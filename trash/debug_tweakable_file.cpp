#define DEV_TWEAKABLE(TYPE, NAME, DEFAULT_VALUE) DEFAULT_VALUE
#define DEV_DISPLAY_TWEAKABLE_ENTRIES

#define DEV_INITIALIZE                                                                  \
do{                                                                                     \
    BEEWAX_INTERNAL::DEV_import_tweakable_entries_from_file("./data/tweakable.txt");    \
}while(0)

#define DEV_TERMINATE                                                               \
do{                                                                                 \
    BEEWAX_INTERNAL::DEV_export_tweakable_entries_to_file("./data/tweakable.txt");  \
    free(BEEWAX_INTERNAL::DEV_tweakable_file_mapping);                              \
    BEEWAX_INTERNAL::DEV_tweakable_file_mapping = nullptr;                          \
    BEEWAX_INTERNAL::DEV_tweakable_entries.free();                                  \
}while(0)

namespace BEEWAX_INTERNAL{
    enum DEV_Tweakable_Type{
        TWEAKABLE_BOOLEAN,
        TWEAKABLE_INTEGER,
        TWEAKABLE_REAL,
        TWEAKABLE_STRING,
    };

    union DEV_Tweakable_Value{
        bool as_bool;
        s32 as_s32;
        float as_float;
        char* as_string;
    };

    struct DEV_Tweakable_Entry{
        const char* file;
        DEV_Tweakable_Type type;
        DEV_Tweakable_Value value;
    };

    constexpr u16 DEV_tweakable_chunk_size = 256;
    static dchunkarena<DEV_Tweakable_Entry, DEV_tweakable_chunk_size> DEV_tweakable_entries;

    static char* DEV_tweakable_file_mapping = nullptr;

    static DEV_Tweakable_Entry* DEV_search_existing_tweakable(const char* name, DEV_Tweakable_Type type){
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

        return nullptr;
    }

    static DEV_Tweakable_Entry* DEV_get_new_or_existing_tweakable(const char* name, DEV_Tweakable_Type type, DEV_Tweakable_Value default_value){

        DEV_Tweakable_Entry* entry = DEV_search_existing_tweakable(name, type);

        // NOTE(hugo): no relevant tweakable was found ie make a new one
        if(!entry){
            entry = DEV_tweakable_entries.get();

            entry->name = name;
            entry->type = type;
            entry->value = default_value;
        }

        return entry;
    }

    static void DEV_read_entire_file(const char* filename, char*& out_data, size_t& out_bytesize){
        FILE* file = fopen(filename, "rb");
        if(file == NULL){
            LOG_ERROR("DEV_read_entire_file(%s) FAILED - aborting", filename);
            out_data = nullptr;
            out_bytesize = 0u;
            return;
        }

        s64 start_index = ftell(file);
        fseek(file, 0, SEEK_END);
        s64 end_index = ftell(file);
        fseek(file, start_index, SEEK_SET);

        s64 file_bytesize = end_index - start_index;
        if(file_bytesize == 0u){
            LOG_INFO("DEV_read_entire_file(%s) - file was empty", filename);
            out_data = nullptr;
            out_bytesize = 0u;
            return;
        }

        char* file_data = (char*)malloc(file_bytesize);
        fread(file_data, 1u, file_bytesize, file);
        fclose(file);

        out_data = file_data;
        out_bytesize = file_bytesize;
    }

    static DEV_Tweakable_Entry DEV_parse_tweakable_entry_at_cursor(char*& cursor){
        DEV_Tweakable_Entry entry;

        // NOTE(hugo): name
        entry.name = cursor;
        while(*cursor != ' ') ++cursor;
        *cursor = '\0';
        ++cursor;

        // NOTE(hugo): type and value
        if(memcmp(cursor, "BOOLEAN", 7u) == 0){
            entry.type = TWEAKABLE_BOOLEAN;
            while(*cursor != ' ') ++cursor;
            ++cursor;

            if(memcmp(cursor, "true", 4u) == 0){
                entry.value.as_BOOLEAN = true;
            }else{
                entry.value.as_BOOLEAN = false;
            }
            while(*cursor != '\n' && *cursor != EOF) ++cursor;

        }else if(memcmp(cursor, "INTEGER", 7u) == 0){
            entry.type = TWEAKABLE_INTEGER;
            while(*cursor != ' ') ++cursor;
            ++cursor;

            sscanf(cursor, "%d", &entry.value.as_INTEGER);
            while(*cursor != '\n' && *cursor != EOF) ++cursor;

        }else if(memcmp(cursor, "REAL", 4u) == 0){
            entry.type = TWEAKABLE_REAL;
            while(*cursor != ' ') ++cursor;
            ++cursor;

            sscanf(cursor, "%f", &entry.value.as_REAL);
            while(*cursor != '\n' && *cursor != EOF) ++cursor;

        }else if(memcmp(cursor, "STRING", 6u) == 0){
            entry.type = TWEAKABLE_STRING;
            while(*cursor != ' ') ++cursor;
            ++cursor;

            entry.value.as_STRING = cursor;
            while(*cursor != '\n' && *cursor != EOF) ++cursor;
            *cursor = '\0';

        }else{
            LOG_ERROR("DEV_parse_tweakable_entry_at_cursor - unknown type starting as %.*s", cursor, 4u);
        }
        ++cursor;
        return entry;
    }

    static void DEV_import_tweakable_entries_from_file(const char* filename){

        // NOTE(hugo): read entire file
        size_t tweakable_file_bytesize;
        DEV_read_entire_file(filename, DEV_tweakable_file_mapping, tweakable_file_bytesize);
        if(!DEV_tweakable_file_mapping){
            return;
        }

        char* cursor = DEV_tweakable_file_mapping;
        while((cursor - DEV_tweakable_file_mapping) < tweakable_file_bytesize){
            DEV_Tweakable_Entry* entry = DEV_tweakable_entries.get();
            *entry = DEV_parse_tweakable_entry_at_cursor(cursor);
        }
    }

    void DEV_reload_tweakable_entries_from_file(const char* filename){
        LOG_INFO("DEV_reload_tweakable_entries_from_file()");

        // NOTE(hugo): read entire file
        char* new_file_mapping;
        size_t tweakable_file_bytesize;
        DEV_read_entire_file(filename, new_file_mapping, tweakable_file_bytesize);
        if(!new_file_mapping){
            return;
        }

        char* cursor = new_file_mapping;
        while((cursor - new_file_mapping) < tweakable_file_bytesize){
            DEV_Tweakable_Entry parsed_entry = DEV_parse_tweakable_entry_at_cursor(cursor);
            DEV_Tweakable_Entry* entry = DEV_search_existing_tweakable(parsed_entry.name, parsed_entry.type);
            if(!entry){
                entry = DEV_tweakable_entries.get();
            }
            entry->name = parsed_entry.name;
            entry->type = parsed_entry.type;
            entry->value = parsed_entry.value;
        }

        free(DEV_tweakable_file_mapping);
        DEV_tweakable_file_mapping = new_file_mapping;
    }

    static void DEV_export_tweakable_entries_to_file(const char* filename){
        dchunkarena<DEV_Tweakable_Entry, DEV_tweakable_chunk_size>::chunk* current_chunk = BEEWAX_INTERNAL::DEV_tweakable_entries.head;

        FILE* file = fopen(filename, "wb");
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
#define DEV_RELOAD_TWEAKABLE_ENTRIES do{ BEEWAX_INTERNAL::DEV_reload_tweakable_entries_from_file("./data/tweakable.txt"); }while(0)



