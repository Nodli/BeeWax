// NOTE(hugo): using a global static and not a scoped static in timer_freqency avoids thread safety guards
// and timer_frequency() is thread safe anyway because we would assign multiple times to the same value
namespace BEEWAX_INTERNAL{
    static u64 timer_frequency_cache = 0u;
}
u64 timer_frequency(){
    if(BEEWAX_INTERNAL::timer_frequency_cache == 0u){
        BEEWAX_INTERNAL::timer_frequency_cache = SDL_GetPerformanceFrequency();
    }
    return BEEWAX_INTERNAL::timer_frequency_cache;
}

u64 timer_ticks(){
    return SDL_GetPerformanceCounter();
}

double timer_seconds(){
    return (double)(timer_ticks()) / (double)(timer_frequency());
}

void timer_sleep(u32 milliseconds){
    SDL_Delay(milliseconds);
}
