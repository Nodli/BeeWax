void Frame_Timing::initialize(u64 target_frames_per_second){
    u64 ticks_per_second = timer_frequency();
    ticks_per_frame = (s64)((double)ticks_per_second / (double)target_frames_per_second);

    max_vsync_ticks_error = (s64)((double)ticks_per_second * max_vsync_relative_error);

    s64 snap_ticks_per_frame_60hz = (s64)(ticks_per_second / 60u);
    snap_ticks_per_frame[0] = snap_ticks_per_frame_60hz;
    snap_ticks_per_frame[1] = snap_ticks_per_frame_60hz * 2;
    snap_ticks_per_frame[2] = snap_ticks_per_frame_60hz * 3;
    snap_ticks_per_frame[3] = snap_ticks_per_frame_60hz * 4;
    snap_ticks_per_frame[4] = (snap_ticks_per_frame_60hz + 1) / 2;

    // NOTE(hugo): trigger resync. on first frame
    ticks_accumulator = (u64)(max_frame_skipped + 1) * (u64)ticks_per_frame;
    saved_tick_count = timer_ticks();
}

u32 Frame_Timing::nupdates_before_render(){
    u64 current_tick_count = timer_ticks();
    s64 dticks = (s64)current_tick_count - (s64)saved_tick_count;

    // NOTE(hugo): accounting for timer anomalies
    if(dticks < 0u){
        dticks = 0u;
    }else if(dticks > (s64)max_frame_skipped * (s64)ticks_per_frame){
        dticks = (s64)(max_frame_skipped * ticks_per_frame);
    }

    // NOTE(hugo): trying to snap dticks to a vsync frequency
    for(u32 isnap = 0; isnap != carray_size(snap_frequencies); ++isnap){
        if(abs((s64)snap_ticks_per_frame[isnap] - (s64)dticks) < (s64)max_vsync_ticks_error){
            dticks = snap_ticks_per_frame[isnap];
            break;
        }
    }

    ticks_accumulator += (u64)dticks;

    // NOTE(hugo): ticks_accumulator grows because the update speed is slower than the vsync speed ie resync. periodically
    if(ticks_accumulator > (u64)(max_frame_skipped) * (u64)ticks_per_frame){
        ticks_accumulator = 0;
        dticks = ticks_per_frame;
    }

    u32 nupdates = (u32)(ticks_accumulator / (u64)ticks_per_frame);
    ticks_accumulator = ticks_accumulator % (u64)ticks_per_frame;

    saved_tick_count = current_tick_count;

    return nupdates;
}
