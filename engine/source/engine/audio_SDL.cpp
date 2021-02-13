// ---- audio asset

void make_audio_asset_from_wav_file(Audio_Asset* asset, const File_Path& path, const Audio_Player* player){
    SDL_AudioSpec wav_spec;
    u8* wav_buffer;
    u32 wav_bytesize;
    SDL_CHECK(SDL_LoadWAV(path.data, &wav_spec, &wav_buffer, &wav_bytesize));

    // NOTE(hugo): convert to the device specification when needed
    SDL_AudioCVT converter;
    s32 converter_return_code = SDL_BuildAudioCVT(&converter,
            wav_spec.format, wav_spec.channels, wav_spec.freq,
            player->device_spec.format, player->device_spec.channels, player->device_spec.freq);
    SDL_CHECK(!(converter_return_code < 0));

    // NOTE(hugo): wav_spec is usable by the device_spec
    if(converter_return_code == 0){
        asset->data = (s16*)malloc(wav_bytesize);
        asset->nsamples = wav_bytesize / sizeof(s16);

        memcpy(asset->data, wav_buffer, wav_bytesize);
        SDL_FreeWAV(wav_buffer);

    // NOTE(hugo): conversion is required
    }else if(converter_return_code == 1){
        converter.len = wav_bytesize;
        converter.buf = (u8*)malloc(converter.len * converter.len_mult);

        memcpy(converter.buf, wav_buffer, wav_bytesize);
        SDL_FreeWAV(wav_buffer);

        SDL_CHECK(SDL_ConvertAudio(&converter) == 0);

        asset->data = (s16*)converter.buf;
        asset->nsamples = converter.len_cvt / sizeof(s16);
    }
}

void free_audio_asset(Audio_Asset* asset){
    ::free(asset->data);
}

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

Audio_Reference Audio_Player::start_playing(const Audio_Asset* asset){
    Component_Reference ref;
    Play_Info* info = asset_playing_queue.create(ref);
    info->asset = asset;
    info->cursor = 0u;

    return {ref};
}

void Audio_Player::stop_playing(const Audio_Reference& ref){
    asset_playing_queue.remove(ref);
}

bool Audio_Player::is_playing(const Audio_Reference& ref){
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

void Audio_Player::pause_audio(){
    SDL_PauseAudioDevice(device, 0);
}

void Audio_Player::resume_audio(){
    SDL_PauseAudioDevice(device, 1);
}

// ---- hardware / software detection

void audio_detect_devices(){
    int ndevices = SDL_GetNumAudioDevices(0);
    LOG_INFO("audio devices:");
    for(int idevice = 0; idevice != ndevices; ++idevice){
        LOG_INFO("- index : %i name : %s", idevice, SDL_GetAudioDeviceName(idevice, 0));
    }
}

void audio_detect_device_status(SDL_AudioDeviceID device){
    SDL_AudioStatus device_status = SDL_GetAudioDeviceStatus(device);
    switch(device_status){
        case SDL_AUDIO_STOPPED:
            LOG_INFO("audio device status: stopped\n");
            break;

        case SDL_AUDIO_PLAYING:
            LOG_INFO("audio device status: playing\n");
            break;

        case SDL_AUDIO_PAUSED:
            LOG_INFO("audio device status: paused\n");
            break;

        default:
            LOG_INFO("Audio Device status : UNKNOWN\n");
            break;
    }
}

void audio_detect_drivers(){
    int ndrivers = SDL_GetNumAudioDrivers();
    LOG_INFO("audio drivers");
    for(int idriver = 0; idriver != ndrivers; ++idriver){
        LOG_INFO("- index : %i name : %s", idriver, SDL_GetAudioDriver(idriver));
    }
}

void audio_detect_current_driver(){
    const char* audio_driver_name = SDL_GetCurrentAudioDriver();
    if(audio_driver_name){
        LOG_INFO("current audio driver name : %s", audio_driver_name);
    }else{
        LOG_INFO("no audio driver available");
    }
}
