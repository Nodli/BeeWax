void Frame_Timing::create(u64 timer, u64 timer_frequency,
        u32 target_frames_per_second, double snapping_error,
        u32 in_max_skipped_frames, u32 smoothing_frames){

    snap_ticks.create();
    smoothing_history.create();

    ticks_per_frame = timer_frequency / target_frames_per_second;
    max_snap_error_ticks = (u64)(snapping_error * ticks_per_frame);
    max_skipped_frames = in_max_skipped_frames;
    snap_ticks.push(timer_frequency / target_frames_per_second);
    smoothing_index = 0u;
    smoothing_history.resize(smoothing_frames);
    for(auto& hist : smoothing_history){
        hist = ticks_per_frame;
    }
    previous_timer = 0u;
    accumulator = 0u;
    frame_count = 0u;
}

void Frame_Timing::destroy(){
    snap_ticks.destroy();
    smoothing_history.destroy();
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
    u64 dtimer_smooth = smoothing_sum / smoothing_history.size;

#if defined(DEVELOPPER_MODE)
    if(frame_count % 60u == 0u){
        float frame_duration_sec = dtimer / (float)timer_frequency();
        float frame_duration_sec_smooth = dtimer_smooth / (float)timer_frequency();
        LOG_INFO("FPS: %f | %f", 1.f / frame_duration_sec, 1.f / frame_duration_sec_smooth);
    }
#endif

    accumulator += dtimer_smooth;

    u32 nframes = accumulator / ticks_per_frame;
    accumulator -= nframes * ticks_per_frame;

    previous_timer = timer;

    frame_count += nframes;
    return nframes;
}

void Frame_Timing::reset_timing(){
    previous_timer = 0u;
    accumulator = 0u;
}
