void detect_audio_devices(){
    int ndevices = SDL_GetNumAudioDevices(0);
    LOG_INFO("Audio Devices");
    for(int idevice = 0; idevice != ndevices; ++idevice){
        LOG_INFO("- index : %i name : %s", idevice, SDL_GetAudioDeviceName(idevice, 0));
    }
}

void detect_audio_drivers(){
    int ndrivers = SDL_GetNumAudioDrivers();
    LOG_INFO("Audio Drivers");
    for(int idriver = 0; idriver != ndrivers; ++idriver){
        LOG_INFO("- index : %i name : %s", idriver, SDL_GetAudioDriver(idriver));
    }
}

void detect_current_audio_driver(){
    const char* audio_driver_name = SDL_GetCurrentAudioDriver();
    if(audio_driver_name){
        LOG_INFO("Current Audio Driver name : %s", audio_driver_name);
    }else{
        LOG_INFO("No current Audio Driver available");
    }
}

void detect_device_status(SDL_AudioDeviceID device){
    SDL_AudioStatus device_status = SDL_GetAudioDeviceStatus(device);
    switch(device_status){
        case SDL_AUDIO_STOPPED:
            LOG_INFO("Audio Device status : STOPPED\n");
            break;

        case SDL_AUDIO_PLAYING:
            LOG_INFO("Audio Device status : PLAYING\n");
            break;

        case SDL_AUDIO_PAUSED:
            LOG_INFO("Audio Device status : PAUSED\n");
            break;

        default:
            LOG_INFO("Audio Device status : UNKNOWN\n");
            break;
    }
}

//struct Sound_Manager

void audio_callback(void* user_ptr, unsigned char* stream, int stream_size){
    memset(stream, 0, stream_size);
}


#if 0
// NOTE(hugo): the buffer is uninitialized
void synthesizer_audio_callback(void* user_ptr, unsigned char* stream, int stream_size){
    Synthesizer* synth = (Synthesizer*)user_ptr;

    for(int ibyte = 0; ibyte != stream_size; ibyte += synth->device_spec.channels){
        for(int ichannel = 0; ichannel != synth->device_spec.channels; ++ichannel){
            //Synthesizer::Wave* wave = &synth->waves[iwave];

            float wave_samples_per_second = (float)synth->device_spec.freq / wave->frequency;
            float current_wave_state = fmod(synth->sample_counter, wave_samples_per_second);
            float normalized_wave_state = current_wave_state / wave_samples_per_second;

            switch(wave->type){
                case Synthesizer::WaveType::NONE:
                    break;
                case Synthesizer::WaveType::SINE:
                    stream[ibyte] = sin(normalized_wave_state * CST::PI<float>) * (float)wave->amplitude;
                    break;
                case Synthesizer::WaveType::TRIANGLE:
                    stream[ibyte] = (normalized_wave_state - 1.f) * 2.f * wave->amplitude;
                    break;
                case Synthesizer::WaveType::SQUARE:
                    stream[ibyte] = (normalized_wave_state > 0.5f) ? - wave->amplitude : wave->amplitude;
                    break;
            }
        }

        ++synth->sample_counter;
    }
}
#endif
