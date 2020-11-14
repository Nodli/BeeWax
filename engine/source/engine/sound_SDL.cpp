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
