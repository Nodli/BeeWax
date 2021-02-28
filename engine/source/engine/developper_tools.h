#if !defined(DEVELOPPER_MODE)

// ---- timing

#define DEV_Timed_Block

// ---- tweakable

#define DEV_Tweak(TYPE, DEFAULT_VALUE, LABEL) DEFAULT_VALUE

// ---- debug renderer

#define DEV_Debug_Renderer

// ---- setup / terminate

#define DEV_setup()
#define DEV_next_frame()
#define DEV_terminate()

#define DEV_ImGui()

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
    static array<DEV_Timing_Entry> DEV_timing_entries;

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

namespace BEEWAX_INTERNAL{
    void DEV_timing_ImGui(){
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
}

// ---- tweakable

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
    struct DEV_Tweakable_Value{
        union{
            bool as_bool;
            s32 as_s32;
            float as_float;
            char* as_string;
        };
        size_t mem_size;
    };
    struct DEV_Tweakable_Entry{
        File_Path file;
        u32 line;
        const char* label;
        DEV_Tweakable_Type type;
        DEV_Tweakable_Value value;
    };
    constexpr u16 DEV_tweakable_nentries = 256;
    static array<DEV_Tweakable_Entry> DEV_tweakable_entries;
    static array<void*> DEV_tweakables_malloc;

    static u32 DEV_search_tweakable_entry(const char* label){
        for(u32 itweak = 0u; itweak != DEV_tweakable_entries.size; ++itweak){
            if(DEV_tweakable_entries[itweak].label == label) return itweak;
        }
        return UINT32_MAX;
    }

    static u32 DEV_get_new_tweakable_entry(){
        u32 entry_index = DEV_tweakable_entries.size;
        DEV_tweakable_entries.push_empty();
        return entry_index;
    }

    static void DEV_reparse_tweakables(){
        // NOTE(hugo): free any previous memory used by the tweakables
        for(u32 ialloc = 0u; ialloc != DEV_tweakables_malloc.size; ++ialloc){
            ::free(DEV_tweakables_malloc[ialloc]);
        }
        DEV_tweakables_malloc.clear();

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
            char* type_position = tweakable_position; while(*type_position == ' '){++type_position;}

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

                    size_t string_length = string_end - string_start;
                    size_t string_bytesize = string_length + 1u;

                    char* tweakable_memory = (char*)malloc(string_bytesize);
                    ENGINE_CHECK(tweakable_memory, "FAILED malloc");
                    DEV_tweakables_malloc.push(tweakable_memory);
                    memcpy(tweakable_memory, string_start, string_length);
                    tweakable_memory[string_length] = '\0';

                    entry.value.as_string = tweakable_memory;
                    entry.value.mem_size = string_bytesize;
                    break;
                }
            }
        }

        //NOTE(hugo): free the files from memory
        for(auto& to_free : path_to_content){
            ::free(to_free.value().data);
        }
        path_to_content.free();
    }

    static int DEV_tweakable_ImGui_InputTexCallback(ImGuiInputTextCallbackData* data){
        DEV_Tweakable_Value* entry_value = (DEV_Tweakable_Value*)data->UserData;
        if(data->EventFlag == ImGuiInputTextFlags_CallbackResize && data->BufSize > entry_value->mem_size){
            size_t new_bytesize = round_up_pow2(data->BufSize);

            void* new_memory = malloc(new_bytesize);
            ENGINE_CHECK(new_memory, "FAILED malloc");
            DEV_tweakables_malloc.push(new_memory);

            entry_value->as_string = (char*)new_memory;
            entry_value->mem_size = new_bytesize;

            data->Buf = (char*)new_memory;
            data->BufSize = new_bytesize;
        }
        return 0;
    }

    // NOTE(hugo):
    // WARNING: values are not written to the source file when changed in the ImGui
    static void DEV_tweakable_ImGui(u32 window_width, u32 window_height){
        ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);
        ImGui::SetNextWindowPos({(float)window_width - 200.f, 0.f}, ImGuiCond_Appearing);
        ImGui::SetNextWindowSize({200.f, (float)window_height}, ImGuiCond_Appearing);
        if(ImGui::Begin("DEV_Tweak")){
            ImGui::PushItemWidth(-1);

            for(u32 ientry = 0u; ientry != DEV_tweakable_entries.size; ++ientry){
                DEV_Tweakable_Entry& entry = DEV_tweakable_entries[ientry];

                switch(entry.type){
                    case Tweakable_bool:
                        ImGui::Separator();
                        ImGui::Checkbox(entry.label, &entry.value.as_bool);
                        break;
                    case Tweakable_s32:
                        ImGui::Separator();
                        ImGui::Text("%s", entry.label);
                        ImGui::DragInt(entry.label, &entry.value.as_s32);
                        break;
                    case Tweakable_float:
                        ImGui::Separator();
                        ImGui::Text("%s", entry.label);
                        ImGui::DragFloat(entry.label, &entry.value.as_float);
                        break;
                    case Tweakable_string:
                        ImGui::Separator();
                        ImGui::Text("%s", entry.label);
                        ImGui::InputText(entry.label, entry.value.as_string, entry.value.mem_size,
                                ImGuiInputTextFlags_CallbackResize, DEV_tweakable_ImGui_InputTexCallback, &entry.value);
                        break;
                }
            }

            ImGui::PopItemWidth();
        }
        ImGui::End();
    }
}

#define DEV_Tweak_Create_bool(VARIABLE, DEFAULT_VALUE)  VARIABLE.as_bool = DEFAULT_VALUE
#define DEV_Tweak_Create_s32(VARIABLE, DEFAULT_VALUE)   VARIABLE.as_s32 = DEFAULT_VALUE
#define DEV_Tweak_Create_float(VARIABLE, DEFAULT_VALUE) VARIABLE.as_float = DEFAULT_VALUE
#define DEV_Tweak_Create_string(VARIABLE, DEFAULT_VALUE)    \
[](){                                                       \
    size_t string_length = strlen(DEFAULT_VALUE);           \
    size_t string_bytesize = string_length + 1u;            \
    void* ptr = malloc(string_bytesize);                    \
    ENGINE_CHECK(ptr, "FAILED malloc");                     \
    memcpy(ptr, DEFAULT_VALUE, string_bytesize);            \
    DEV_tweakables_malloc.push(ptr);                        \
    VARIABLE.as_string = (char*)ptr;                        \
    VARIABLE.mem_size = string_bytesize;                    \
    return ptr;                                             \
}()

#define DEV_Tweak(TYPE, DEFAULT_VALUE, LABEL)                                                                           \
[](){                                                                                                                   \
    using namespace BEEWAX_INTERNAL;                                                                                    \
    static u32 DEV_tweakable_entry_index = UINT_MAX;                                                                    \
    if(DEV_tweakable_entry_index == UINT_MAX){                                                                          \
        DEV_tweakable_entry_index = DEV_search_tweakable_entry(LABEL);                                                  \
    }                                                                                                                   \
    if(DEV_tweakable_entry_index == UINT_MAX){                                                                          \
        DEV_tweakable_entry_index = DEV_get_new_tweakable_entry();                                                      \
        DEV_tweakable_entries[DEV_tweakable_entry_index].file = __FILE__;                                               \
        DEV_tweakable_entries[DEV_tweakable_entry_index].line = __LINE__;                                               \
        DEV_tweakable_entries[DEV_tweakable_entry_index].label = LABEL;                                                 \
        DEV_tweakable_entries[DEV_tweakable_entry_index].type = CONCATENATE(Tweakable_, TYPE);                          \
        CONCATENATE(DEV_Tweak_Create_, TYPE)(DEV_tweakable_entries[DEV_tweakable_entry_index].value, DEFAULT_VALUE);    \
    }                                                                                                                   \
    return CONCATENATE(DEV_tweakable_entries[DEV_tweakable_entry_index].value.as_, TYPE);                               \
}()

// ---- debug renderer

#define DEV_Debug_Renderer do{ GL::set_debug_message_callback(); }while(0)

// ---- setup / terminate

void DEV_setup(){
}
void DEV_next_frame(){
}
void DEV_terminate(){
    BEEWAX_INTERNAL::DEV_timing_entries.free();

    BEEWAX_INTERNAL::DEV_tweakable_entries.free();

    for(u32 ialloc = 0u; ialloc != BEEWAX_INTERNAL::DEV_tweakables_malloc.size; ++ialloc){
        ::free(BEEWAX_INTERNAL::DEV_tweakables_malloc[ialloc]);
    }
    BEEWAX_INTERNAL::DEV_tweakables_malloc.free();
}

void DEV_ImGui(u32 window_width, u32 window_height){
    BEEWAX_INTERNAL::DEV_timing_ImGui();
    BEEWAX_INTERNAL::DEV_tweakable_ImGui(window_width, window_height);
}

#endif
