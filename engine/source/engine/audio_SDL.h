#ifndef H_AUDIO
#define H_AUDIO

struct Audio_Player;

// ---- audio asset

struct Audio_Asset{
    s16* data = nullptr;
    u32 nsamples = 0u;
};

Audio_Asset audio_asset_from_wav_file(const File_Path& path, Audio_Player* player);
void free_audio_asset(Audio_Asset& asset);

// ---- audio player

// NOTE(hugo): 735 samples per frame per channel at 60 fps with a frequency of 44100Hz
constexpr u32 audio_device_buffer_in_samples = 512u;
constexpr u32 audio_manager_buffer_in_samples = 4096u;
// NOTE(hugo): generator should be at least one frame ahead of reader at all times
constexpr u32 audio_manager_generator_offset_in_samples = audio_manager_buffer_in_samples - 1u;

struct Audio_Playing_ID{
    u32 index;
    u64 generation;
};

struct Audio_Player{
    void setup();
    void terminate();

    Audio_Playing_ID start_playing(Audio_Asset* asset);
    void stop_playing(Audio_Playing_ID play);

    void mix_next_frame();

    void pause_audio();
    void resume_audio();

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

    struct Play_Data{
        Audio_Asset* asset;
        u32 cursor = 0u;
        u64 generation = 0u;
    };
    u64 manager_generation = 0u;
    diterpool<Play_Data> to_play;
};

// ---- hardware / software detection

void audio_detect_devices();
void audio_detect_device_status(SDL_AudioDeviceID device);
void audio_detect_drivers();
void audio_detect_current_driver();

#endif
