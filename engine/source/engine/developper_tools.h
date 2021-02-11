// NOTE(hugo): nothing is thread safe as is
// a mutex could be used in the initialization part as it's only executed once

#if !defined(DEVELOPPER_MODE)

// ---- timing

#define DEV_Timed_Block

#define DEV_LOG_timing_entries()
#define DEV_LOG_frame_duration(TIMER)

// ---- tweakable

#define DEV_Tweak(TYPE, DEFAULT_VALUE) DEFAULT_VALUE

// ---- debug renderer

#define DEV_Debug_Renderer

// ---- setup / terminate

#define DEV_setup()
#define DEV_next_frame()
#define DEV_terminate()

#else

// ---- timing

namespace BEEWAX_INTERNAL{
    struct DEV_Timing_Entry{
        const char* file = nullptr;
        const char* function = nullptr;
        u32 line = 0u;
        u32 hit_count = 0u;
        u64 cycle_counter = 0u;
    };
    constexpr u32 DEV_timing_nentries = 256u;
    static array<DEV_Timing_Entry> DEV_timing_entries;

    // TODO(hugo): support timing events
    //struct DEV_Timing_Event{
    //    u64 cycle_counter = 0u;
    //    u32 entry_index = 0u;
    //};
    //constexpr u32 DEV_timing_nevents = 256u;
    //static array<DEV_Timing_Entry> DEV_timing_events;

    static u32 DEV_get_new_timing_entry(const char* file, const char* function, const u32 line){
        u32 entry_index = DEV_timing_entries.size;
        DEV_timing_entries.push_empty();
        DEV_timing_entries[entry_index].file = file;
        DEV_timing_entries[entry_index].function = function;
        DEV_timing_entries[entry_index].line = line;
        return entry_index;
    };

    struct DEV_Timing_Container{
        DEV_Timing_Container(u32 ientry_index) : entry_index(ientry_index){
            start_cycles = cycle_counter();
        }
        ~DEV_Timing_Container(){
            DEV_Timing_Entry& entry = DEV_timing_entries[entry_index];
            ++(entry.hit_count);
            entry.cycle_counter += (cycle_counter() - start_cycles);
        };

        u32 entry_index;
        u64 start_cycles;
    };
}

#define DEV_Timed_Block                                                                                                             \
static u32 CONCATENATE(DEV_timing_entry_at_, __LINE__) = UINT_MAX;                                                                  \
if(CONCATENATE(DEV_timing_entry_at_, __LINE__) == UINT_MAX){                                                                        \
    CONCATENATE(DEV_timing_entry_at_, __LINE__) = BEEWAX_INTERNAL::DEV_get_new_timing_entry(__FILE__, __func__, __LINE__);          \
}                                                                                                                                   \
BEEWAX_INTERNAL::DEV_Timing_Container CONCATENATE(DEV_timing_container_at_, __LINE__)(CONCATENATE(DEV_timing_entry_at_, __LINE__));

void DEV_LOG_timing_entries(){
    using namespace BEEWAX_INTERNAL;
    for(u32 ientry = 0u; ientry != DEV_timing_entries.size; ++ientry){
        DEV_Timing_Entry& entry = DEV_timing_entries[ientry];
        LOG_RAW("DEV_Timing_Entry [%d]: %s HITS(%d) CYCLES(%d) AVG(%d)",
                ientry,
                entry.function,
                entry.hit_count,
                entry.cycle_counter,
                entry.cycle_counter / entry.hit_count);
    }
}

#define DEV_LOG_frame_duration(TIMER)                                               \
do{                                                                                 \
    static u64 previous_timer = 0u;                                                 \
    u64 timer_freq = timer_frequency();                                             \
    u64 dtimer = TIMER - previous_timer;                                            \
    previous_timer = TIMER;                                                         \
    double dsec = (double)dtimer / (double)timer_freq;                              \
    LOG_RAW("DEV_Frame_Time: %u ticks, %f ms, %f FPS", dtimer, dsec, 1. / dsec);    \
}while(false)

// ---- tweakable

#if 0
// NOTE(hugo): parsing the following expression
// DEV_Tweak(__spaces__ type __spaces__ , __spaces__ value __spaces__)
// REF(hugo): https://blog.tuxedolabs.com/2018/03/13/hot-reloading-hardcoded-parameters.html

namespace BEEWAX_INTERNAL{
    enum DEV_Tweakable_Type{
        Tweakable_bool,
        Tweakable_s32,
        Tweakable_float,
        Tweakable_string,
    };
    union DEV_Tweakable_Value{
        bool as_bool;
        s32 as_s32;
        float as_float;
        char* as_string;
    };
    struct DEV_Tweakable_Entry{
        File_Path file;
        u32 line;
        DEV_Tweakable_Type type;
        DEV_Tweakable_Value value;
    };
    constexpr u16 DEV_tweakable_nentries = 256;
    static array<DEV_Tweakable_Entry> DEV_tweakable_entries;
    static array<char*> DEV_string_alloc;

    static u32 DEV_get_new_tweakable_entry(){
        u32 entry_index = DEV_tweakable_entries.size;
        DEV_tweakable_entries.push_empty();
        return entry_index;
    }

    static void DEV_reparse_tweakables(){
        // NOTE(hugo): free any previous memory used by the tweakables
        for(u32 ialloc = 0u; ialloc != DEV_string_alloc.size; ++ialloc){
            ::free(DEV_string_alloc[ialloc]);
        }
        DEV_string_alloc.clear();

        hashmap<File_Path, buffer<u8>> path_to_content;
        for(u32 ientry = 0u; ientry != DEV_tweakable_entries.size; ++ientry){
            DEV_Tweakable_Entry& entry = DEV_tweakable_entries[ientry];

            // NOTE(hugo): register the file in the hashmap and read it to memory when necessary
            bool file_not_loaded;
            buffer<u8>* file_content = path_to_content.get(entry.file, file_not_loaded);
            if(file_not_loaded){
                *file_content = read_file(entry.file, "r");
            }

            // NOTE(hugo): go to the line of the DEV_Tweak
            char* cursor = (char*)file_content->data;
            u32 cursor_line = 1u;
            while(cursor_line != entry.line){
                while(*(cursor++) != '\n');
                ++cursor_line;
            }

            const char* tweakable_expression = "DEV_Tweak(";

            // NOTE(hugo): go to the position of the DEV_Tweak
            char* tweakable_position = strstr(cursor, tweakable_expression);
            tweakable_position += strlen(tweakable_expression);

            // NOTE(hugo): type checking
            char* type_position = tweakable_position;
            while(*type_position == ' '){++type_position;}

            switch(entry.type){
                case Tweakable_bool:
                {
                    const char* type_expression = "bool";
                    assert(memcmp(type_position, type_expression, strlen(type_expression)) == 0u);
                    type_position += strlen(type_expression);
                    break;
                }
                case Tweakable_s32:
                {
                    const char* type_expression = "s32";
                    assert(memcmp(type_position, type_expression, strlen(type_expression)) == 0u);
                    type_position += strlen(type_expression);
                    break;
                }
                case Tweakable_float:
                {
                    const char* type_expression = "float";
                    assert(memcmp(type_position, type_expression, strlen(type_expression)) == 0u);
                    type_position += strlen(type_expression);
                    break;
                }
                case Tweakable_string:
                {
                    const char* type_expression = "string";
                    assert(memcmp(type_position, type_expression, strlen(type_expression)) == 0u);
                    type_position += strlen(type_expression);
                    break;
                }
            }

            char* value_position = type_position;
            while(*value_position == ' '){++value_position;}
            assert(*value_position++ == ',');
            while(*value_position == ' '){++value_position;}

            // NOTE(hugo): parsing the tweakable value
            switch(entry.type){
                case Tweakable_bool:
                {
                    const char* true_expression = "true";
                    const char* false_expression = "false";
                    if(memcmp(value_position, true_expression, strlen(true_expression)) == 0u){
                        entry.value.as_bool = true;
                    }else if(memcmp(value_position, false_expression, strlen(false_expression)) == 0u){
                        entry.value.as_bool = false;
                    }else{
                        assert(false);
                    }
                    break;
                }
                case Tweakable_s32:
                {
                    char* endptr;
                    entry.value.as_s32 = (s32)strtol(value_position, &endptr, 10);
                    assert(endptr != value_position);
                    break;
                }
                case Tweakable_float:
                {
                    char* endptr;
                    entry.value.as_float = strtof(value_position, &endptr);
                    assert(endptr != value_position);
                    break;
                }
                case Tweakable_string:
                {
                    char* string_start = value_position;
                    assert(*string_start++ == '"');

                    char* string_end = string_start;
                    while(*string_end != '"'){++string_end;}

                    char* tweakable_memory = (char*)malloc(string_end - string_start);
                    assert(tweakable_memory);
                    memcpy(tweakable_memory, string_start, string_end - string_start);
                    DEV_string_alloc.push(tweakable_memory);

                    entry.value.as_string = tweakable_memory;
                    break;
                }
            }
        }

        //NOTE(hugo): free the files from memory
        for(auto& to_free : path_to_content){
            ::free(to_free.value.data);
        }
        path_to_content.free();
    }
}

#define DEV_Tweak(TYPE, DEFAULT_VALUE)                                                                      \
[](){                                                                                                       \
    using namespace BEEWAX_INTERNAL;                                                                        \
    static u32 DEV_tweakable_entry_index = UINT_MAX;                                                        \
    if(DEV_tweakable_entry_index == UINT_MAX){                                                              \
        DEV_tweakable_entry_index = DEV_get_new_tweakable_entry();                                          \
        DEV_tweakable_entries[DEV_tweakable_entry_index].file = __FILE__;                                   \
        DEV_tweakable_entries[DEV_tweakable_entry_index].line = __LINE__;                                   \
        DEV_tweakable_entries[DEV_tweakable_entry_index].type = CONCATENATE(Tweakable_, TYPE);              \
        CONCATENATE(DEV_tweakable_entries[DEV_tweakable_entry_index].value.as_, TYPE) = DEFAULT_VALUE;      \
    }                                                                                                       \
    return CONCATENATE(DEV_tweakable_entries[DEV_tweakable_entry_index].value.as_, TYPE);                   \
}()
#endif

// ---- debug renderer

#define DEV_Debug_Renderer do{ GL::set_debug_message_callback(); }while(0)

// ---- setup / terminate

void DEV_setup(){
}
void DEV_next_frame(){
}
void DEV_terminate(){
    BEEWAX_INTERNAL::DEV_timing_entries.free();
}

#endif
