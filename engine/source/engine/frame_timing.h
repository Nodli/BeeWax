#ifndef H_FRAME_TIMING
#define H_FRAME_TIMING

// TODO(hugo): make something more flexible with dynamic allocation so that the user can select the snap frequencies

// NOTE(hugo): Fuzzy Time Snapping
// https://medium.com/@tglaiel/how-to-make-your-game-run-at-60fps-24c61210fe75
struct Frame_Timing{
    constexpr static u32 max_frame_skipped = 8u;
    constexpr static u32 nsnap_frequencies = 5u;
    constexpr static u32 snap_frequencies[nsnap_frequencies] = {60u, 30u, 20u, 15u, 120u};
    constexpr static double max_vsync_relative_error = 0.002;

    void initialize(u64 target_frames_per_second);
    u32 nupdates_before_render(); // NOTE(hugo): number of times to update the game loop before rendering & swapping framebuffers

    s64 ticks_per_frame;
    s64 snap_ticks_per_frame[carray_size(snap_frequencies)];
    s64 max_vsync_ticks_error;

    u64 saved_tick_count;
    u64 ticks_accumulator;
};

#endif
