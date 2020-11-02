#if defined(DEBUG_TOOLS)

struct Debug_Timing_Entry{
    const char* file = nullptr;
    const char* function = nullptr;
    u32 line = 0u;
    u32 hit_count = 0u;
    u64 cycle_counter = 0u;
};

struct Debug_Timing_Event{
    u64 cycle_counter = 0u;
    u32 entry_index = 0u;
};

namespace BEEWAX_INTERNAL{
    constexpr u16 debug_timing_chunk_size = 256;
    static dchunkarena<Debug_Timing_Entry, debug_timing_chunk_size> debug_timing_entries;
    static dchunkarena<Debug_Timing_Event, debug_timing_chunk_size> debug_timing_events;
}

struct Debug_Timing_Container{
    Debug_Timing_Container(Debug_Timing_Entry* entry) : debug_timing_entry(entry){
        start_cycles = cycle_counter();
    }
    ~Debug_Timing_Container(){
        ++(debug_timing_entry->hit_count);
        debug_timing_entry->cycle_counter += (cycle_counter() - start_cycles);
    };

    Debug_Timing_Entry* debug_timing_entry;
    u64 start_cycles;
};

#define TIMED_BLOCK()                                                                                                                           \
static Debug_Timing_Entry* CONCATENATE(debug_timing_entry_ptr_variable_at_, __LINE__) = nullptr;                                                \
if(!CONCATENATE(debug_timing_entry_ptr_variable_at_, __LINE__)){                                                                                \
    CONCATENATE(debug_timing_entry_ptr_variable_at_, __LINE__) = BEEWAX_INTERNAL::debug_timing_entries.get();                                   \
    CONCATENATE(debug_timing_entry_ptr_variable_at_, __LINE__)->file = __FILE__;                                                                \
    CONCATENATE(debug_timing_entry_ptr_variable_at_, __LINE__)->function = __func__;                                                            \
    CONCATENATE(debug_timing_entry_ptr_variable_at_, __LINE__)->line = __LINE__;                                                                \
}                                                                                                                                               \
Debug_Timing_Container CONCATENATE(debug_timing_container_variable_at_, __LINE__)(CONCATENATE(debug_timing_entry_ptr_variable_at_, __LINE__));

void debug_display_timing_entries(){
    u32 nentries = 0u;
    dchunkarena<Debug_Timing_Entry, BEEWAX_INTERNAL::debug_timing_chunk_size>::chunk* current_chunk = BEEWAX_INTERNAL::debug_timing_entries.head;

    auto display_chunk = [&](u32 entries_to_display){
        for(u32 ientry = 0u; ientry != entries_to_display; ++ientry){
            Debug_Timing_Entry& entry = current_chunk->data[ientry];
            LOG_RAW("Debug_Timing_Entry [%d]: %s HITS(%d) CYCLES(%d) AVG(%d)", nentries + ientry, entry.function, entry.hit_count, entry.cycle_counter, entry.cycle_counter / entry.hit_count);
            ++nentries;
        }
    };

    if(current_chunk && BEEWAX_INTERNAL::debug_timing_entries.current_chunk_space != 0u){
        display_chunk(BEEWAX_INTERNAL::debug_timing_chunk_size - BEEWAX_INTERNAL::debug_timing_entries.current_chunk_space);
        current_chunk = current_chunk->next;
    }

    while(current_chunk){
        display_chunk(BEEWAX_INTERNAL::debug_timing_chunk_size);
        current_chunk = current_chunk->next;
    }
}

void debug_next_frame(){
    BEEWAX_INTERNAL::debug_timing_entries.clear();
    BEEWAX_INTERNAL::debug_timing_events.clear();
}

void debug_free(){
    BEEWAX_INTERNAL::debug_timing_entries.free();
    BEEWAX_INTERNAL::debug_timing_events.free();
}

#else

#define TIMED_BLOCK()
void debug_display_timing_entries(){}
void debug_next_frame(){}
void debug_free(){}

#endif
