#ifndef H_TEXTURE_ANIMATION
#define H_TEXTURE_ANIMATION

struct Texture_Animation_Frame{
    Texture_ID texture;
    vec2 uvmin;
    vec2 uvmax;
};

struct Texture_Animation_Asset{
    darray<Texture_Animation_Frame> frames;
    u32 frame_duration;
};

void free_texture_animation_asset(Texture_Animation_Asset& asset){
    asset.frames.free();
}

typedef u32 Texture_Animation_Playing_ID;

constexpr Texture_Animation_Playing_ID unknown_texture_animation_playing = UINT_MAX;

struct Texture_Animation_Player{
    void terminate();

    Texture_Animation_Playing_ID start_playing(Texture_Animation_Asset* asset);
    void stop_playing(Texture_Animation_Playing_ID play);
    Texture_Animation_Frame* get_frame(Texture_Animation_Playing_ID play);

    void next_frame();

    // ---- data

    struct Play_Data{
        Texture_Animation_Asset* asset;
        u32 counter;
        u32 frame_index;
    };
    u64 manager_generation;
    diterpool<Play_Data> to_play;
};


#endif
