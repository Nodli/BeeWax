#ifndef H_TIME
#define H_TIME

void setup_timer();

u64 timer_frequency();
u64 timer_ticks();
double timer_seconds();
void timer_sleep(u32 milliseconds);

#endif
