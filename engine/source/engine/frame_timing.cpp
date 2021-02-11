void Frame_Timing::initialize(u64 timer, u64 timer_frequency,
        u32 target_frames_per_second, double snapping_error,
        u32 in_max_skipped_frames, u32 smoothing_frames){

    ticks_per_frame = timer_frequency / target_frames_per_second;
    max_snap_error_ticks = (u64)(snapping_error * ticks_per_frame);
    max_skipped_frames = in_max_skipped_frames;
    snap_ticks.push(timer_frequency / target_frames_per_second);
    smoothing_index = 0u;
    smoothing_history.set_size(smoothing_frames);
    for(auto& hist : smoothing_history){
        hist = ticks_per_frame;
    }
    previous_timer = 0u;
    accumulator = 0u;
}

void Frame_Timing::terminate(){
    snap_ticks.free();
    smoothing_history.free();
}

void Frame_Timing::add_frequency(u64 timer_frequency, u32 frames_per_second){
    snap_ticks.push(timer_frequency / frames_per_second);
}

u32 Frame_Timing::nupdates_before_render(u64 timer){
    if(previous_timer == 0u){
        timer = max(timer, ticks_per_frame);
        previous_timer = timer - ticks_per_frame;
    }

    timer = max(timer, previous_timer);
    u64 dtimer = min(timer - previous_timer, max_skipped_frames * ticks_per_frame);

    // NOTE(hugo): snapping
    for(const auto& s : snap_ticks){
        s64 dsnap = abs((s64)s - (s64)dtimer);
        if(dsnap < max_snap_error_ticks){
            dtimer = s;
        }
    }

    // NOTE(hugo): smoothing
    smoothing_history[smoothing_index++] = dtimer;
    if(smoothing_index == smoothing_history.size) smoothing_index = 0u;
    u64 smoothing_sum = 0u;
    for(auto& s : smoothing_history){
        smoothing_sum = smoothing_sum + s;
    }
    dtimer = smoothing_sum / smoothing_history.size;

    accumulator += dtimer;

    u32 nframes = accumulator / ticks_per_frame;
    accumulator -= nframes * ticks_per_frame;

    previous_timer = timer;

    return nframes;
}

void Frame_Timing::reset_timing(){
    previous_timer = 0u;
    accumulator = 0u;
}
