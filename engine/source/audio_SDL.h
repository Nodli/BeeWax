#ifndef H_AUDIO
#define H_AUDIO

struct Audio_Player_SDL;

// ---- audio asset

struct Audio_Asset{
    s16* data;
    u32 nsamples;
};

void make_audio_asset_from_wav_file(Audio_Asset* asset, const File_Path& path, const Audio_Player_SDL* player);
void free_audio_asset(Audio_Asset* asset);

// ---- audio player

namespace BEEWAX_INTERNAL{
    constexpr u32 audio_device_buffer_in_samples = 256u;
    constexpr u32 audio_info_capacity = 32u;
};

struct Audio_Channel{
    u32 virtual_index;
    u32 generation;
};
constexpr Audio_Channel Audio_Invalid_Channel = {0u, 0u};

enum Audio_Channel_State : u32{
    FREE = 0u,
    STOP,
    PLAY,
    LOOP
};

struct Audio_Player_SDL{
    void create();
    void destroy();

    Audio_Channel start(const Audio_Asset* asset, Audio_Channel_State state);
    void stop(const Audio_Channel& channel);

    void pause();
    void resume();

    // ---- data

    struct Audio_Info{
        const Audio_Asset* asset;
        Audio_Channel_State state;
        u32 cursor;
        u32 generation;
    };

    struct Audio_Mixer{
        float buffer[BEEWAX_INTERNAL::audio_device_buffer_in_samples];
        Audio_Info audio_info[BEEWAX_INTERNAL::audio_info_capacity];
    };

    SDL_AudioDeviceID device;
    SDL_AudioSpec device_spec;

    Audio_Mixer mixer;
};

// ---- hardware / software detection

void audio_detect_devices();
void audio_detect_device_status(SDL_AudioDeviceID device);
void audio_detect_drivers();
void audio_detect_current_driver();

#endif
