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
        asset->data = (s16*)bw_malloc(wav_bytesize);
        asset->nsamples = wav_bytesize / sizeof(s16);

        memcpy(asset->data, wav_buffer, wav_bytesize);
        SDL_FreeWAV(wav_buffer);

    // NOTE(hugo): conversion is required
    }else if(converter_return_code == 1){
        converter.len = wav_bytesize;
        converter.buf = (u8*)bw_malloc(converter.len * converter.len_mult);

        memcpy(converter.buf, wav_buffer, wav_bytesize);
        SDL_FreeWAV(wav_buffer);

        SDL_CHECK(SDL_ConvertAudio(&converter) == 0);

        asset->data = (s16*)converter.buf;
        asset->nsamples = converter.len_cvt / sizeof(s16);
    }
}

void free_audio_asset(Audio_Asset* asset){
    bw_free(asset->data);
}

// ---- audio player 2

static void audio_callback(void* user_ptr, u8* out_stream, s32 out_stream_size);

void Audio_Player_SDL::create(){
    SDL_AudioSpec required_spec;
    required_spec.freq = 44100u;
    required_spec.format = AUDIO_S16;
    required_spec.channels = 1u;
    required_spec.samples = BEEWAX_INTERNAL::audio_device_buffer_in_samples;
    required_spec.callback = audio_callback;
    required_spec.userdata = (void*)&mixer;

    SDL_AudioSpec returned_spec;
    device = SDL_OpenAudioDevice(nullptr, 0, &required_spec, &returned_spec, 1u);
    SDL_CHECK(device != 0);
    device_spec = returned_spec;

    memset(mixer.buffer, 0x00, sizeof(mixer.buffer));
    memset(mixer.audio_info, 0x00, sizeof(mixer.audio_info));

    SDL_PauseAudioDevice(device, 0);
}

void Audio_Player_SDL::destroy(){
    SDL_PauseAudioDevice(device, 1);
    SDL_CloseAudioDevice(device);
}

static u32 Audio_Player_SDL_virtualize_index(u32 index){
    return index + 1u;
}
static u32 Audio_Player_SDL_devirtualize_index(u32 index){
    return index - 1u;
}

Audio_Channel Audio_Player_SDL::start(const Audio_Asset* asset){
    Audio_Channel output;
    output.virtual_index = 0u;

    for(u32 iinfo = 0u; iinfo != BEEWAX_INTERNAL::audio_info_capacity; ++iinfo){
        Audio_Info& info = mixer.audio_info[iinfo];

        if(atomic_get(&info.state) == Audio_Info::FREE){
            info.asset = asset;
            //info.cursor = 0u;
            atomic_set(&info.state, Audio_Info::PLAY);

            output.virtual_index = Audio_Player_SDL_virtualize_index(iinfo);
            output.generation = info.generation;
            break;
        }
    }

    return output;
}

void Audio_Player_SDL::stop(const Audio_Channel& channel){
    u32 index = Audio_Player_SDL_devirtualize_index(channel.virtual_index);

    if(channel.virtual_index
    && index < BEEWAX_INTERNAL::audio_info_capacity
    && atomic_get(&(mixer.audio_info[index].state)) > Audio_Info::STOP){

        atomic_set(&(mixer.audio_info[index].state), Audio_Info::STOP);
    }
}

void Audio_Player_SDL::pause(){
    SDL_PauseAudioDevice(device, 0);
}

void Audio_Player_SDL::resume(){
    SDL_PauseAudioDevice(device, 1);
}

static void audio_callback(void* user_ptr, u8* out_stream, s32 out_stream_size){

    Audio_Player_SDL::Audio_Mixer* mixer = (Audio_Player_SDL::Audio_Mixer*)user_ptr;
    u32 out_samples = out_stream_size / sizeof(s16);

    assert(mixer);
    assert(out_samples == BEEWAX_INTERNAL::audio_device_buffer_in_samples);

    // NOTE(hugo): reset mixer buffer
    memset(mixer->buffer, 0x00, BEEWAX_INTERNAL::audio_device_buffer_in_samples);

    // NOTE(hugo): update & mix into buffer
    for(u32 iinfo = 0u; iinfo != BEEWAX_INTERNAL::audio_device_buffer_in_samples; ++iinfo){
        Audio_Player_SDL::Audio_Info& info = mixer->audio_info[iinfo];
        Audio_Player_SDL::Audio_Info::State audio_state = atomic_get(&info.state);

        if(audio_state == Audio_Player_SDL::Audio_Info::PLAY){
            u32 mix_cursor = 0u;
            u32 asset_cursor = info.cursor;

            while(mix_cursor != BEEWAX_INTERNAL::audio_device_buffer_in_samples && asset_cursor != info.asset->nsamples){
                mixer->buffer[mix_cursor++] += (float)info.asset->data[asset_cursor++];
            }

            info.cursor = asset_cursor;

            // NOTE(hugo): end of asset samples ie STOP
            if(asset_cursor == info.asset->nsamples){
                audio_state = Audio_Player_SDL::Audio_Info::STOP;
            }

        }else if(audio_state == Audio_Player_SDL::Audio_Info::LOOP && info.asset->nsamples > 0u){
            u32 mix_cursor = 0u;
            u32 asset_cursor = info.cursor;

            while(mix_cursor != BEEWAX_INTERNAL::audio_device_buffer_in_samples){
                if(asset_cursor != info.asset->nsamples) asset_cursor = 0u;
                mixer->buffer[mix_cursor++] += (float)info.asset->data[asset_cursor++];
            }

            info.cursor = asset_cursor;
        }

        if(audio_state == Audio_Player_SDL::Audio_Info::STOP){
            info.asset = nullptr;
            info.cursor = 0u;

            atomic_set(&info.state, Audio_Player_SDL::Audio_Info::FREE);
        }
    }

    s16* out = (s16*)out_stream;

    // NOTE(hugo): convert and copy to stream
    for(u32 isample = 0u; isample != BEEWAX_INTERNAL::audio_device_buffer_in_samples; ++isample){
        s16 sample = (s16)min_max<float>(mixer->buffer[isample], SHRT_MIN, SHRT_MAX);
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
