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

#else

// ---- initialization

#define DEV_INITIALIZE  \
do{                     \
}while(0)               \

#define DEV_NEXT_FRAME                              \
do{                                                 \
    BEEWAX_INTERNAL::DEV_timing_entries.clear();    \
    BEEWAX_INTERNAL::DEV_timing_events.clear();     \
}while(0)

#define DEV_TERMINATE                               \
do{                                                 \
    BEEWAX_INTERNAL::DEV_timing_entries.free();     \
    BEEWAX_INTERNAL::DEV_timing_events.free();      \
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

    static inline DEV_Timing_Entry* DEV_get_new_timed_block(const char* file, const char* function, u32 line){
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
        TWEAKABLE_bool,
        TWEAKABLE_u32,
        TWEAKABLE_s32,
        TWEAKABLE_float,
    };

    union DEV_Tweakable_Value{
        bool as_bool;
        u32 as_u32;
        s32 as_s32;
        float as_float;
    };

    struct DEV_Tweakable_Entry{
        const char* name;
        DEV_Tweakable_Type type;
        DEV_Tweakable_Value value;
    };

    constexpr u16 DEV_tweakable_chunk_size = 256;
    static dchunkarena<DEV_Tweakable_Entry, DEV_tweakable_chunk_size> DEV_tweakable_entries;

    static inline DEV_Tweakable_Entry* get_new_or_existing_tweakable(const char* name, DEV_Tweakable_Type type, DEV_Tweakable_Value default_value){

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
};

#define DEV_TWEAKABLE(TYPE, NAME, DEFAULT_VALUE)                                                    \
[](){                                                                                               \
    using namespace BEEWAX_INTERNAL;                                                                \
    static DEV_Tweakable_Entry* entry = nullptr;                                                    \
    if(!entry){                                                                                     \
        DEV_Tweakable_Value default_value;                                                          \
        CONCATENATE(default_value.as_, TYPE) = DEFAULT_VALUE;                                       \
        entry = get_new_or_existing_tweakable(NAME, CONCATENATE(TWEAKABLE_, TYPE), default_value);  \
    }                                                                                               \
    return CONCATENATE(entry->value.as_, TYPE);                                                     \
}()

#endif
