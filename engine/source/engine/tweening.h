struct Tweening_Manager{
    template<typename T>
    struct Tweening_Entry{
        T* ptr;
        T base;
        T range;
        u64 tick_start;
        u64 tick_duration;
    };

#define FOR_EACH_TWEENING_TYPE(FUNCTION)    \
FUNCTION(float)

#define DECLARE_TWEENING_TYPE(type)                                                 \
    void push(type* ptr, type start, type end, u64 tick_duration){                  \
        u32 entry_index = CONCATENATE(container_, type).insert_empty();             \
        CONCATENATE(container_, type)[entry_index].ptr = ptr;                       \
        CONCATENATE(container_, type)[entry_index].base = start;                   \
        CONCATENATE(container_, type)[entry_index].range = end - start;             \
        CONCATENATE(container_, type)[entry_index].tick_start = tick_current;       \
        CONCATENATE(container_, type)[entry_index].tick_duration = tick_duration;   \
        ++active_tweens;                                                            \
    }                                                                               \
    dpool<Tweening_Entry<type>> CONCATENATE(container_, type);

    FOR_EACH_TWEENING_TYPE(DECLARE_TWEENING_TYPE)

#undef DECLARE_TWEENING_TYPE

#define UPDATE_TWEENING_TYPE(type)                                                                              \
    for(u32 ientry = CONCATENATE(container_, type).get_first();                                                 \
            ientry < CONCATENATE(container_, type).capacity;                                                    \
            ientry = CONCATENATE(container_, type).get_next(ientry)){                                           \
        Tweening_Entry<type> entry = CONCATENATE(container_, type)[ientry];                                     \
        float interp = clamp((float)(tick_current - entry.tick_start), 0.f, 1.f) / (float)entry.tick_duration;  \
        *(entry.ptr) = entry.base + entry.range * interp;                                                       \
        if(tick_current >= (entry.tick_start + entry.tick_duration)){                                           \
            CONCATENATE(container_, type).remove(ientry);                                                       \
            --active_tweens;                                                                                    \
        }                                                                                                       \
    }

    // NOTE(hugo): returns the number of remaining tweens
    u32 update(){
        tick_current = timer_ticks();
        FOR_EACH_TWEENING_TYPE(UPDATE_TWEENING_TYPE)
        return active_tweens;
    }

#undef UPDATE_TWEENING_TYPE

    // ---- data

    u32 active_tweens = 0u;
    u64 tick_current = 0u;
};
