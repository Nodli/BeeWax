#ifndef H_AUDIO
#define H_AUDIO

#if 0
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

enum Audio_Channel_State : u32 {
    AUDIO_REFERENCE_INVALID = 0u,
    AUDIO_PLAYING,
};

struct Audio_Channel{
    u32 virtual_index;
    u32 generation;
};
constexpr Audio_Channel Audio_Channel_Invalid = {0u, 0u};

struct Audio_Player_SDL{
    void create();
    void destroy();

    Audio_Channel start(const Audio_Asset* asset, bool loop = false);
    void stop(const Audio_Channel& channel);

    void pause();
    void resume();

    // ---- data

    struct Audio_Info{
        enum State : u32 {
            FREE = 0u,
            STOP,
            PLAY,
            LOOP,
        };

        const Audio_Asset* asset;
        State state;
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

struct Audio_Player_SDL;

// ---- audio asset

struct Audio_Asset{
    s16* data = nullptr;
    u32 nsamples = 0u;
};

void make_audio_asset_from_wav_file(Audio_Asset* asset, const File_Path& path, const Audio_Player_SDL* player);
void free_audio_asset(Audio_Asset* asset);

// ---- audio player

constexpr u32 audio_device_buffer_in_samples = 256u;
constexpr u32 audio_info_per_bucket = 4u;

struct Audio_Info{
    enum Audio_State : u32{
        FREE,
        STOP,
        PLAY_UNIQUE,
        PLAY_LOOP,
    };

    Audio_State state = FREE;
    const Audio_Asset* asset = nullptr;
    u32 cursor = 0u;
    u32 generation = 0u;
};

struct Audio_Reference{
    Audio_Info* info = nullptr;
    u32 generation = 0u;
};
constexpr Audio_Reference unknown_audio_reference = {nullptr, 0u};

struct Audio_Player_SDL{
    void create();
    void destroy();

    Audio_Reference start(const Audio_Asset* asset, bool loop = false);
    void stop(const Audio_Reference& reference);
    bool is_valid(const Audio_Reference& reference);

    void pause();
    void resume();

    // ---- data

    SDL_AudioDeviceID device;
    SDL_AudioSpec device_spec;

    struct Audio_Info_Bucket{
        Audio_Info data[audio_info_per_bucket] = {};
        Audio_Info_Bucket* next = nullptr;
    };
    struct Audio_State{
        float* mix_buffer = nullptr;
        Audio_Info_Bucket* audio_mem = nullptr;
    } mixer_state;
};

// ---- hardware / software detection

void audio_detect_devices();
void audio_detect_device_status(SDL_AudioDeviceID device);
void audio_detect_drivers();
void audio_detect_current_driver();

#endif
