namespace BEEWAX_INTERNAL{
    static u64 timer_frequency_cache = 0u;
}

void setup_timer(){
    BEEWAX_INTERNAL::timer_frequency_cache = SDL_GetPerformanceFrequency();
    assert(BEEWAX_INTERNAL::timer_frequency_cache != 0u);
}

u64 timer_frequency(){
    assert(BEEWAX_INTERNAL::timer_frequency_cache != 0u);
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
