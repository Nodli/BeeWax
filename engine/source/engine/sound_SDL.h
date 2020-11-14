static void audio_mix_samples_s16(s16* destination, s16* source, u32 nsamples){
    for(u32 isample = 0u; isample != nsamples; ++isample){
        destination[isample] += source[isample];
    }
}

static void audio_callback(void* user_ptr, u8* out_stream, s32 out_stream_size);

typedef u32 Audio_Buffer_ID;
struct Audio_Playing_ID{
    u32 index;
    u64 generation;
};

// NOTE(hugo): 735 samples per frame per channel at 60 fps with a frequency of 44100Hz
constexpr u32 audio_device_buffer_in_samples = 256u;
constexpr u32 audio_manager_buffer_in_samples = 4096u;
constexpr u32 audio_manager_generator_offset_in_samples = audio_manager_buffer_in_samples - 1u;

struct Audio_Manager{
    void setup(){
        SDL_AudioSpec required_spec;
        required_spec.freq = 44100u;
        required_spec.format = AUDIO_S16;
        required_spec.channels = 1u;
        required_spec.samples = audio_device_buffer_in_samples;
        required_spec.callback = audio_callback;
        required_spec.userdata = (void*)&callback_data;

        SDL_AudioSpec returned_spec;
        device = SDL_OpenAudioDevice(nullptr, 0, &required_spec, &returned_spec, 0);
        if(!device){
            LOG_ERROR("SDL_OpenAudioDevice() FAILED - %s", SDL_GetError());
        }

        device_spec = returned_spec;
        LOG_INFO("audio device specification:");
        LOG_INFO("- frequency : %i", device_spec.freq);
        LOG_INFO("- format: %s", (device_spec.format == AUDIO_S16) ? "device_s16" : "unknown");
        LOG_INFO("- channels: %i", device_spec.channels);
        LOG_INFO("- silence: %d", device_spec.silence);
        LOG_INFO("- samples: %i", device_spec.samples);
        LOG_INFO("- callback: %p", device_spec.callback);
        LOG_INFO("- userdata: %p", device_spec.userdata);

        callback_data.nsamples = audio_manager_buffer_in_samples;
        callback_data.buffer = (u8*)malloc(sizeof(s16) * callback_data.nsamples);
        if(!callback_data.buffer){
            LOG_ERROR("malloc FAILED - keeping the previous memory");
        }

        SDL_PauseAudioDevice(device, 0);
    }

    void terminate(){
        // NOTE(hugo): free loaded buffers
        for(u32 ibuffer = buffer_pool.get_first();
            ibuffer < buffer_pool.capacity;
            ibuffer = buffer_pool.get_next(ibuffer)){

            unload_buffer(ibuffer);
        }
        buffer_pool.free();
        play_pool.free();

        SDL_CloseAudioDevice(device);
    }

    void next_frame(){
        size_t sample_bytesize = sizeof(s16);

        u32 reader_cursor = atomic_get(&callback_data.reader_cursor);
        u32 generator_cursor = callback_data.generator_cursor;
        u32 expected_generator_cursor = (reader_cursor + audio_manager_generator_offset_in_samples);
        if(expected_generator_cursor >= callback_data.nsamples){
            expected_generator_cursor -= callback_data.nsamples;
        }

        // NOTE(hugo): single memcpy
        if(expected_generator_cursor > generator_cursor){
            u32 samples_to_write = expected_generator_cursor - generator_cursor;
            u8* write_ptr = callback_data.buffer + generator_cursor * sample_bytesize;
            memset(write_ptr, 0, samples_to_write * sample_bytesize);

            for(u32 iplay = play_pool.get_first();
                    iplay < play_pool.capacity;
                    iplay = play_pool.get_next(iplay)){

                Play_Data& play_data = play_pool[iplay];
                Buffer_Data& buffer = buffer_pool[play_data.buffer];

                if(play_data.cursor + samples_to_write < buffer.nsamples){
                    audio_mix_samples_s16((s16*)write_ptr, (s16*)(buffer.data + play_data.cursor * sample_bytesize), samples_to_write);
                    //memcpy(write_ptr, buffer.data + play_data.cursor * sample_bytesize, samples_to_write * sample_bytesize);
                    play_data.cursor += samples_to_write;

                }else{
                    u32 samples_to_copy = buffer.nsamples - play_data.cursor;
                    audio_mix_samples_s16((s16*)write_ptr, (s16*)(buffer.data + play_data.cursor * sample_bytesize), samples_to_copy);
                    //memcpy(write_ptr, buffer.data + play_data.cursor * sample_bytesize, samples_to_copy * sample_bytesize);
                    play_pool.remove(iplay);
                }
            }

        // NOTE(hugo): two memcpy and wrap around
        }else if(expected_generator_cursor < generator_cursor){
            u32 samples_to_write_end = callback_data.nsamples - generator_cursor;
            u8* write_end = callback_data.buffer + generator_cursor * sample_bytesize;
            memset(write_end, 0, samples_to_write_end * sample_bytesize);

            u32 samples_to_write_start = expected_generator_cursor;
            u8* write_start = callback_data.buffer;
            memset(write_start, 0, samples_to_write_start * sample_bytesize);

            u32 samples_to_write = samples_to_write_end + samples_to_write_start;

            for(u32 iplay = play_pool.get_first();
                    iplay < play_pool.capacity;
                    iplay = play_pool.get_next(iplay)){

                Play_Data& play_data = play_pool[iplay];
                Buffer_Data& buffer = buffer_pool[play_data.buffer];

                if(play_data.cursor + samples_to_write < buffer.nsamples){
                    audio_mix_samples_s16((s16*)write_end, (s16*)(buffer.data + play_data.cursor * sample_bytesize), samples_to_write_end);
                    audio_mix_samples_s16((s16*)write_start, (s16*)(buffer.data + (play_data.cursor + samples_to_write_end) * sample_bytesize), samples_to_write_start);
                    //memcpy(write_end, buffer.data + play_data.cursor * sample_bytesize, samples_to_write_end * sample_bytesize);
                    //memcpy(write_start, buffer.data + (play_data.cursor + samples_to_write_end) * sample_bytesize, samples_to_write_start * sample_bytesize);
                    play_data.cursor += samples_to_write;

                }else{
                    u32 samples_to_copy = buffer.nsamples - play_data.cursor;
                    audio_mix_samples_s16((s16*)write_end, (s16*)(buffer.data + play_data.cursor * sample_bytesize), min(samples_to_copy, samples_to_write_end));
                    //memcpy(write_end, buffer.data + play_data.cursor * sample_bytesize, min(samples_to_copy, samples_to_write_end) * sample_bytesize);
                    if(samples_to_copy > samples_to_write_end){
                        audio_mix_samples_s16((s16*)write_start, (s16*)(buffer.data + (play_data.cursor + samples_to_write_end) * sample_bytesize), (samples_to_copy - samples_to_write_end));
                        //memcpy(write_start, buffer.data + (play_data.cursor + samples_to_write_end) * sample_bytesize, (samples_to_copy - samples_to_write_end) * sample_bytesize);
                    }
                    play_pool.remove(iplay);
                }
            }

        }

        atomic_set(&callback_data.generator_cursor, expected_generator_cursor);
    }

    Audio_Buffer_ID load_wav(const File_Path& path){
        SDL_AudioSpec wav_spec;
        u8* wav_buffer;
        u32 wav_bytesize;
        if(!SDL_LoadWAV(path.data, &wav_spec, &wav_buffer, &wav_bytesize)){
            LOG_ERROR("SDL_LoadWav() FAILED - %s", SDL_GetError());
        }

        LOG_INFO("wav specification");
        LOG_INFO("- frequency : %i", wav_spec.freq);
        LOG_INFO("- format: %s", (wav_spec.format == AUDIO_S16) ? "audio_s16" : "unknown");
        LOG_INFO("- channels: %i", wav_spec.format);

        // NOTE(hugo): convert to the device specification when needed
        SDL_AudioCVT converter;
        s32 converter_return_code = SDL_BuildAudioCVT(&converter,
                wav_spec.format, wav_spec.channels, wav_spec.freq,
                device_spec.format, device_spec.channels, device_spec.freq);

        Buffer_Data buffer;

        // NOTE(hugo): wav_spec is usable by the device_spec
        if(converter_return_code == 0){
            buffer.data = (u8*)malloc(wav_bytesize);
            buffer.nsamples = wav_bytesize / sizeof(s16);

            memcpy(buffer.data, wav_buffer, wav_bytesize);
            SDL_FreeWAV(wav_buffer);

        // NOTE(hugo): conversion is required
        }else if(converter_return_code == 1){
            converter.len = wav_bytesize;
            converter.buf = (u8*)malloc(converter.len * converter.len_mult);

            memcpy(converter.buf, wav_buffer, wav_bytesize);
            SDL_FreeWAV(wav_buffer);

            if(SDL_ConvertAudio(&converter)){
                LOG_ERROR("SDL_ConvertAudio() FAILED - %s", SDL_GetError());
            }

            buffer.data = converter.buf;
            buffer.nsamples = converter.len_cvt / sizeof(s16);

        // NOTE(hugo): converter setup failed
        }else if(converter_return_code < 0){
            LOG_ERROR("SDL_BuildAudioCVT() FAILED - %s", SDL_GetError());
        }

        return {buffer_pool.insert(buffer)};
    }
    void unload_buffer(Audio_Buffer_ID buffer){
        Buffer_Data& buffer_entry = buffer_pool[buffer];
        ::free(buffer_entry.data);
        buffer_pool.remove(buffer);
    }

    Audio_Playing_ID play_buffer(Audio_Buffer_ID buffer){
        Play_Data play_data;
        play_data.buffer = buffer;
        play_data.cursor = 0u;
        play_data.generation = manager_generation;

        u32 index = play_pool.insert(play_data);
        u32 generation = manager_generation;

        ++manager_generation;

        return {index, generation};
    }

    // ---- data

    struct Audio_State{
        u8* buffer = nullptr;
        u32 nsamples = 0u;

        // NOTE(hugo): offset in samples
        // read_cursor:         writen by the audio thread, read    by the generator thread
        // generator_cursor:    read   by the audio thread, written by the generator thread
        volatile u32 reader_cursor = 0u;
        volatile u32 generator_cursor = 0u;
    } callback_data;

    SDL_AudioDeviceID device;
    SDL_AudioSpec device_spec;

    struct Buffer_Data{
        u8* data = nullptr;
        u32 nsamples = 0u;
    };
    dpool<Buffer_Data> buffer_pool;

    struct Play_Data{
        Audio_Buffer_ID buffer;
        u32 cursor = 0u;
        u64 generation = 0u;
    };
    dpool<Play_Data> play_pool;

    u64 manager_generation = 0u;
};

// NOTE(hugo): assumes that the generator will write with an offset of at least one frame in front of the audio thread
static void audio_callback(void* user_ptr, u8* out_stream, s32 out_stream_size){
    Audio_Manager::Audio_State* callback_data = (Audio_Manager::Audio_State*)user_ptr;
    size_t sample_bytesize = sizeof(s16);

    u32 reader_cursor = callback_data->reader_cursor;
    u32 generator_cursor = atomic_get(&callback_data->generator_cursor);
    u32 out_samples = (u32)out_stream_size / sample_bytesize;

    // NOTE(hugo): handling the case where out_stream_size is not a multiple of sample_bytesize
    u32 out_samples_bytesize = out_samples * (u32)sample_bytesize;
    for(u32 izero_byte = (u32)out_stream_size - out_samples * sample_bytesize; izero_byte != (u32)out_stream_size; ++izero_byte){
        out_stream[izero_byte] = 0;
    }

    // NOTE(hugo): single memcpy
    if(generator_cursor > reader_cursor){
        u32 samples_to_copy = min((generator_cursor - reader_cursor), out_samples);
        u32 samples_to_zero = out_samples - samples_to_copy;
        memcpy(out_stream, callback_data->buffer + reader_cursor * sample_bytesize, samples_to_copy * sample_bytesize);
        memset(out_stream + samples_to_copy * sample_bytesize, 0, samples_to_zero * sample_bytesize);

        atomic_set<u32>(&callback_data->reader_cursor, reader_cursor + samples_to_copy);

    // NOTE(hugo): two memcpy and wrap around
    }else if(generator_cursor < reader_cursor){
        u32 samples_to_copy_end = min(callback_data->nsamples - reader_cursor, out_samples);
        memcpy(out_stream, callback_data->buffer + reader_cursor * sample_bytesize, samples_to_copy_end * sample_bytesize);

        u32 samples_copied = samples_to_copy_end;
        if(samples_copied < out_samples){
            u32 samples_to_copy_start = min(generator_cursor, out_samples - samples_copied);
            memcpy(out_stream + samples_copied * sample_bytesize, callback_data->buffer, samples_to_copy_start * sample_bytesize);
            samples_copied += samples_to_copy_start;
            atomic_set<u32>(&callback_data->reader_cursor, samples_to_copy_start);
        }else{
            atomic_set<u32>(&callback_data->reader_cursor, reader_cursor + samples_to_copy_end);
        }

        u32 samples_to_zero = out_samples - samples_copied;
        memset(out_stream + samples_copied * sample_bytesize, 0, samples_to_zero * sample_bytesize);

    // NOTE(hugo): nothing provided - zero memory
    }else{
        LOG_TRACE("!WARNING! nothing provided - audio dropout !WARNING!");
        memset(out_stream, 0, out_samples * sample_bytesize);
    }
}

// ---- hardware / software detection

void audio_detect_devices();
void audio_detect_device_status(SDL_AudioDeviceID device);
void audio_detect_drivers();
void audio_detect_current_driver();
