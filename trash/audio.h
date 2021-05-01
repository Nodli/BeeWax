// ---- audio player

// NOTE(hugo): 735 samples per frame per channel at 60 fps with a frequency of 44100Hz
constexpr u32 audio_device_buffer_in_samples = 512u;
constexpr u32 audio_manager_buffer_in_samples = 4096u;
// NOTE(hugo): generator should be at least one frame ahead of reader at all times
constexpr u32 audio_manager_generator_offset_in_samples = audio_manager_buffer_in_samples - 1u;

struct Audio_Reference : Component_Reference{};
constexpr Audio_Reference unknown_audio_reference = {unknown_component_reference};

struct Audio_Player{
    void setup();
    void terminate();

    Audio_Reference start(const Audio_Asset* asset);
    void stop(const Audio_Reference& reference);
    bool is_valid(const Audio_Reference& reference);

    void mix_next_frame();

    void pause();
    void resume();

    // ---- data

    SDL_AudioDeviceID device;
    SDL_AudioSpec device_spec;

    float* mix_buffer = nullptr;
    struct Audio_State{
        s16* buffer = nullptr;
        u32 nsamples = 0u;

        // NOTE(hugo): offset in samples
        // read_cursor:         writen by the audio thread, read    by the generator thread
        // generator_cursor:    read   by the audio thread, written by the generator thread
        volatile u32 reader_cursor = 0u;
        volatile u32 generator_cursor = 0u;
    } state;

    struct Play_Info{
        const Audio_Asset* asset = nullptr;
        u32 cursor = 0u;
    };
    Component_Storage<Play_Info> asset_playing_queue;
};

// ---- audio player

static void audio_callback(void* user_ptr, u8* out_stream, s32 out_stream_size);

void Audio_Player::setup(){
    SDL_AudioSpec required_spec;
    required_spec.freq = 44100u;
    required_spec.format = AUDIO_S16;
    required_spec.channels = 1u;
    required_spec.samples = audio_device_buffer_in_samples;
    required_spec.callback = audio_callback;
    required_spec.userdata = (void*)&state;

    SDL_AudioSpec returned_spec;
    device = SDL_OpenAudioDevice(nullptr, 0, &required_spec, &returned_spec, 0);
    SDL_CHECK(device != 0);
    device_spec = returned_spec;

    state.nsamples = audio_manager_buffer_in_samples;
    state.buffer = (s16*)malloc(sizeof(s16) * state.nsamples);
    ENGINE_CHECK(state.buffer, "FAILED MALLOC");

    mix_buffer = (float*)malloc(sizeof(float) * audio_manager_generator_offset_in_samples);
    ENGINE_CHECK(mix_buffer, "FAILED MALLOC");

    SDL_PauseAudioDevice(device, 0);
}

void Audio_Player::terminate(){
    SDL_PauseAudioDevice(device, 1);

    ::free(state.buffer);
    ::free(mix_buffer);
    asset_playing_queue.free();

    SDL_CloseAudioDevice(device);
}

Audio_Reference Audio_Player::start(const Audio_Asset* asset){
    Component_Reference ref;
    Play_Info* info = asset_playing_queue.create(ref);
    info->asset = asset;
    info->cursor = 0u;

    return {ref};
}

void Audio_Player::stop(const Audio_Reference& ref){
    asset_playing_queue.remove(ref);
}

bool Audio_Player::is_valid(const Audio_Reference& ref){
    return asset_playing_queue.is_valid(ref);
}

void Audio_Player::mix_next_frame(){
    u32 reader_cursor = atomic_get(&state.reader_cursor);
    u32 generator_cursor = state.generator_cursor;

    // NOTE(hugo): compute the number of samples to mix and the copy ranges to the staging buffer
    u32 samples_mixed;
    u32 samples_to_mix;
    u32 first_start = generator_cursor;
    u32 first_samples;
    u32 second_start = 0u;
    u32 second_samples;
    if(generator_cursor >= reader_cursor){
        samples_mixed = generator_cursor - reader_cursor;
        samples_to_mix = audio_manager_generator_offset_in_samples - samples_mixed;
        first_samples = min(samples_to_mix, state.nsamples - generator_cursor);
        second_samples = samples_to_mix - first_samples;

    // NOTE(hugo): generator_cursor < reader_cursor
    }else{
        samples_mixed = state.nsamples - reader_cursor + generator_cursor;
        samples_to_mix = audio_manager_generator_offset_in_samples - samples_mixed;
        first_samples = samples_to_mix;
        second_samples = 0u;
    }

    u32 total_samples = first_samples + second_samples;

    // NOTE(hugo): zero the mix buffer
    for(u32 isample = 0u; isample != total_samples; ++isample){
        mix_buffer[isample] = 0.f;
    }

    // NOTE(hugo): mix the samples
    {
        u32 iplay = 0u;
        while(iplay != asset_playing_queue.storage.size){
            Play_Info& info = asset_playing_queue.storage[iplay].data;

            u32 mix_sample = 0u;
            u32 play_cursor = info.cursor;
            while(mix_sample != total_samples && play_cursor != info.asset->nsamples){
                mix_buffer[mix_sample] += (float)info.asset->data[play_cursor];
                ++mix_sample;
                ++play_cursor;
            }

            if(play_cursor != info.asset->nsamples){
                info.cursor = play_cursor;
                ++iplay;

            }else{
                asset_playing_queue.remove_by_storage_index(iplay);
            }
        }
    }

    // NOTE(hugo): convert to s16 and copy to the ring buffer
    for(u32 isample = 0u; isample != first_samples; ++isample){
        s16 sample = (s16)min_max<float>(mix_buffer[isample], SHRT_MIN, SHRT_MAX);
        state.buffer[first_start + isample] = sample;
    }
    for(u32 isample = 0u; isample != second_samples; ++isample){
        s16 sample = (s16)min_max<float>(mix_buffer[first_samples + isample], SHRT_MIN, SHRT_MAX);
        state.buffer[second_start + isample] = sample;
    }

    // NOTE(hugo): update the generator cursor for the audio callback
    u32 new_generator_cursor = (reader_cursor + audio_manager_generator_offset_in_samples) % state.nsamples;
    atomic_set(&state.generator_cursor, new_generator_cursor);
}

static void audio_callback(void* user_ptr, u8* out_stream, s32 out_stream_size){
    Audio_Player::Audio_State* state = (Audio_Player::Audio_State*)user_ptr;

    u32 reader_cursor = state->reader_cursor;
    u32 generator_cursor = atomic_get(&state->generator_cursor);
    if(reader_cursor == generator_cursor){
        memset(out_stream, 0, out_stream_size);
        return;
    }

    u32 out_nsamples = (u32)out_stream_size / sizeof(s16);
    u32 end_samples;
    u32 start_samples;
    if(generator_cursor > reader_cursor){
        end_samples = generator_cursor - reader_cursor;
        start_samples = 0u;
    }else{
        end_samples = state->nsamples - reader_cursor;
        start_samples = generator_cursor;
    }

    // NOTE(hugo): compute the memcpy ranges from the staging buffer
    u32 first_start = reader_cursor;
    u32 first_samples = min(out_nsamples, end_samples);

    u32 second_start = 0u;
    u32 second_samples = min(out_nsamples - first_samples, start_samples);

    u32 copy_samples = first_samples + second_samples;
    u32 zero_samples = out_nsamples - copy_samples;

    s16* out_samples = (s16*)out_stream;
    memcpy(out_samples, state->buffer + first_start, first_samples * sizeof(s16));
    memcpy(out_samples + first_samples, state->buffer + second_start, second_samples * sizeof(s16));
    memset(out_samples + copy_samples, 0, zero_samples * sizeof(s16));

    if(zero_samples){
        LOG_TRACE("audio starvation; zero: %d", zero_samples);
    }

    u32 new_reader_cursor = reader_cursor + first_samples;
    if(new_reader_cursor == state->nsamples){
        new_reader_cursor = second_samples;
    }
    atomic_set<u32>(&state->reader_cursor, new_reader_cursor);
}

void Audio_Player::pause(){
    SDL_PauseAudioDevice(device, 0);
}

void Audio_Player::resume(){
    SDL_PauseAudioDevice(device, 1);
}
