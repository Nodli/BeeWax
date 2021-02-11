#ifndef H_FRAME_TIMING
#define H_FRAME_TIMING

// NOTE(hugo): Fuzzy Time Snapping
// REF(hugo): https://medium.com/@tglaiel/how-to-make-your-game-run-at-60fps-24c61210fe75
struct Frame_Timing{
    void initialize(u64 timer, u64 timer_frequency,
            u32 target_frames_per_second, double snapping_error,
            u32 max_skipped_frames, u32 smoothing_frames);
    void terminate();

    void add_frequency(u64 timer_frequency, u32 frames_per_second);
    u32 nupdates_before_render(u64 timer);

    void reset_timing();

    // ---- data

    u64 ticks_per_frame;
    s64 max_snap_error_ticks;
    u32 max_skipped_frames;
    array<u64> snap_ticks;

    u32 smoothing_index;
    array<u64> smoothing_history;
    u64 previous_timer;
    u64 accumulator;
};

#endif
