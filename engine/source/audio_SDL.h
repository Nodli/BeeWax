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
    constexpr u32 synth_info_capacity = 32u;
};

struct Audio_Channel{
    u32 virtual_index;
    u32 generation;
};
constexpr Audio_Channel Audio_Invalid_Channel = {0u, 0u};

enum struct Audio_Channel_State : u32{
    FREE = 0u,
    STOP,
    PLAY,
    LOOP
};

struct Synth_Channel{
    u32 virtual_index;
};
constexpr Synth_Channel Audio_Invalid_Synth = {0u};

typedef void (*Synth_Process)(void* data, float* buffer, s32 buffer_size);

enum struct Synth_Channel_State : u32{
    FREE = 0u,
    DEACTIVATED,
    ACTIVE
};

struct Audio_Player_SDL{
    void create();
    void destroy();

    Audio_Channel start(const Audio_Asset* asset, Audio_Channel_State state);
    void stop(const Audio_Channel& channel);

    Synth_Channel start(void* data, Synth_Process process);
    void stop(const Synth_Channel& channel);

    void pause();
    void resume();

    // NOTE(hugo): ensures that the callback has seen or will see mixer state changes
    // * before destroying stopped synth
    // * before destroying stopeed audio asset
    void ensure_modification_visibility();

    // ---- data

    struct Audio_Channel_Data{
        const Audio_Asset* asset;
        Audio_Channel_State state;
        u32 cursor;
        u32 generation;
    };

    struct Synth_Channel_Data{
        void* data;
        Synth_Process process;
        Synth_Channel_State state;
    };

    struct Audio_Mixer{
        float buffer[BEEWAX_INTERNAL::audio_device_buffer_in_samples];
        Audio_Channel_Data audio_info[BEEWAX_INTERNAL::audio_info_capacity];
        Synth_Channel_Data synth_info[BEEWAX_INTERNAL::synth_info_capacity];
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
