// ---- audio asset

void make_audio_asset_from_wav_file(Audio_Asset* asset, const File_Path& path, const Audio_Player_SDL* player){
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

// ---- audio player 2

static void audio_callback(void* user_ptr, u8* out_stream, s32 out_stream_size);

static void increase_audio_mem(Audio_Player_SDL* audio_player){
    Audio_Player_SDL::Audio_Info_Bucket* bucket = (Audio_Player_SDL::Audio_Info_Bucket*)malloc(sizeof(Audio_Player_SDL::Audio_Info_Bucket));
    new((void*)bucket) Audio_Player_SDL::Audio_Info_Bucket{};

    bucket->next = audio_player->mixer_state.audio_mem;
    atomic_set(&audio_player->mixer_state.audio_mem, bucket);
}

void Audio_Player_SDL::setup(){
    SDL_AudioSpec required_spec;
    required_spec.freq = 44100u;
    required_spec.format = AUDIO_S16;
    required_spec.channels = 1u;
    required_spec.samples = audio_device_buffer_in_samples;
    required_spec.callback = audio_callback;
    required_spec.userdata = (void*)&mixer_state;

    SDL_AudioSpec returned_spec;
    device = SDL_OpenAudioDevice(nullptr, 0, &required_spec, &returned_spec, 0);
    SDL_CHECK(device != 0);
    device_spec = returned_spec;

    mixer_state.mix_buffer = (float*)malloc(sizeof(float) * audio_device_buffer_in_samples);
    ENGINE_CHECK(mixer_state.mix_buffer, "FAILED MALLOC");

    increase_audio_mem(this);

    SDL_PauseAudioDevice(device, 0);
}

void Audio_Player_SDL::terminate(){
    SDL_PauseAudioDevice(device, 1);

    ::free(mixer_state.mix_buffer);
    Audio_Info_Bucket* ptr = mixer_state.audio_mem;
    while(ptr){
        ptr = ptr->next;
        ::free(ptr);
    }

    SDL_CloseAudioDevice(device);
}

Audio_Reference Audio_Player_SDL::start(const Audio_Asset* asset){
    Audio_Reference ref;

    // NOTE(hugo): find existing voice
    Audio_Info_Bucket* ptr = mixer_state.audio_mem;
    while(ptr){
        for(u32 iinfo = 0u; iinfo != audio_info_per_bucket; ++iinfo){
            Audio_Info& info = ptr->data[iinfo];
            if(atomic_get(&info.state) == Audio_Info::FREE){
                info.asset = asset;
                atomic_set(&info.state, Audio_Info::PLAY_UNIQUE);
                return {&info, info.generation};
            }
        }

        ptr = ptr->next;
    }

    // NOTE(hugo): make space for new voices
    increase_audio_mem(this);

    Audio_Info& info = mixer_state.audio_mem->data[0u];
    info.asset = asset;
    atomic_set(&info.state, Audio_Info::PLAY_UNIQUE);

    return {&info, info.generation};
}

void Audio_Player_SDL::stop(const Audio_Reference& ref){
    if(is_valid(ref)){
        atomic_set(&ref.info->state, Audio_Info::STOP);
    }
}

bool Audio_Player_SDL::is_valid(const Audio_Reference& ref){
    return ref.info && ref.generation == ref.info->generation && atomic_get(&ref.info->state) == Audio_Info::PLAY_UNIQUE;
}

void Audio_Player_SDL::pause(){
    SDL_PauseAudioDevice(device, 0);
}

void Audio_Player_SDL::resume(){
    SDL_PauseAudioDevice(device, 1);
}

static void audio_callback(void* user_ptr, u8* out_stream, s32 out_stream_size){
    Audio_Player_SDL::Audio_State* state = (Audio_Player_SDL::Audio_State*)user_ptr;
    u32 out_samples = out_stream_size / sizeof(s16);
    assert(out_samples == audio_device_buffer_in_samples);

    // NOTE(hugo): reset mix
    memset(state->mix_buffer, 0, audio_device_buffer_in_samples * sizeof(float));

    // NOTE(hugo): update & mix
    Audio_Player_SDL::Audio_Info_Bucket* bucket = state->audio_mem;
    while(bucket){
        for(u32 iinfo = 0u; iinfo != audio_info_per_bucket; ++iinfo){
            Audio_Info& info = bucket->data[iinfo];

            switch(atomic_get(&info.state)){
                case Audio_Info::PLAY_UNIQUE:{
                    u32 mix_cursor = 0u;
                    u32 asset_cursor = info.cursor;

                    while(mix_cursor != audio_device_buffer_in_samples && asset_cursor != info.asset->nsamples){
                        state->mix_buffer[mix_cursor++] += (float)info.asset->data[asset_cursor++];
                    }

                    // NOTE(hugo): finish processing for this voice
                    if(asset_cursor != info.asset->nsamples){
                        info.cursor = asset_cursor;
                        break;
                    }

                    // NOTE(hugo): end of voice's asset ie STOP
                    [[fallthrough]];
                }
                case Audio_Info::STOP:{
                    info.asset = nullptr;
                    info.cursor = 0u;
                    ++info.generation;

                    atomic_set(&info.state, Audio_Info::FREE);
                    break;
                }
                default:
                    break;
            }
        }
        bucket = bucket->next;
    }

    s16* out = (s16*)out_stream;

    // NOTE(hugo): convert and copy to stream
    for(u32 isample = 0u; isample != audio_device_buffer_in_samples; ++isample){
        s16 sample = (s16)min_max<float>(state->mix_buffer[isample], SHRT_MIN, SHRT_MAX);
        out[isample] = sample;
    }
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
