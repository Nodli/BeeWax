#ifndef H_TEXTURE_ANIMATION
#define H_TEXTURE_ANIMATION

// ---- texture animation asset

struct Texture_Animation_Asset{
    struct Frame{
        Texture_View view;
        
    };
    array<Texture_Animation_Frame> frames;
    u32 frame_duration;
};

void free_texture_animation_asset(Texture_Animation_Asset* asset);

// ---- texture animation player

#if 0
struct Texture_Animation_Reference : Component_Reference{};

struct Texture_Animation_Player{
    void terminate();

    Texture_Animation_Reference start_playing(const Texture_Animation_Asset* asset);
    Texture_View get_frame(const Texture_Animation_Reference& reference);
    void stop_playing(const Texture_Animation_Reference& reference);
    bool is_playing(const Texture_Animation_Reference& reference);

    void next_frame();

    // ---- data

    struct Play_Info{
        Texture_Animation_Asset* asset = nullptr;
        u32 frame_counter = 0u;
        u32 frame_index = 0u;
    };
    Component_Storage<Play_Info> animating_queue;
};
#endif

#endif
