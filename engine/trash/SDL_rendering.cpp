SDL_Texture* TextureFromBitmap(const char* path, SDL_Renderer* renderer){
    SDL_Texture* texture;
    SDL_Surface* bitmap = SDL_LoadBMP(path);

    if(!bitmap){
        SDL_Error();
        return nullptr;
    }

    texture = SDL_CreateTextureFromSurface(renderer, bitmap);
    SDL_FreeSurface(bitmap);

    if(!texture){
        SDL_Error();
        return nullptr;
    }

    return texture;
}

void TextureBlit(SDL_Texture* texture, SDL_Renderer* renderer, int x, int y){
    SDL_Rect blit_area = {x, y};
    SDL_QueryTexture(texture, NULL, NULL, &blit_area.w, &blit_area.h);
    SDL_RenderCopy(renderer, texture, NULL, &blit_area);
}
void TextureBlitCentered(SDL_Texture* texture, SDL_Renderer* renderer, int x, int y){
    SDL_Rect blit_area;
    SDL_QueryTexture(texture, NULL, NULL, &blit_area.w, &blit_area.h);
    blit_area.x = x - (blit_area.w / 2);
    blit_area.y = y - (blit_area.h / 2);
    SDL_RenderCopy(renderer, texture, NULL, &blit_area);
}

struct Animation{
    int current_keyframe;
    int keyframes;
    int keyframes_size[2];
    SDL_Texture* sprite_sheet;
    int* keyframes_origins;
};

// sprite_sheet: row-major sprite sheet
// wframe: width of a frame in the sprite_sheet
// hframe: height of a frame in the sprite_sheet
// wsheet: number of frames in a line of the sprite_sheet
// start_frame: index of the frame starting the animation in the sprite sheet
// nframes: number of animation frames following the start_frame in the sprite sheet
Animation* AnimationFromSpriteSheet(
        SDL_Texture* sprite_sheet,
        int wframe, int hframe,
        int wsheet,
        int start_frame,
        int nframes){

    char* memory = new char[sizeof(Animation) + 2 * sizeof(int) * nframes];

    // NOTE(hugo): Animation is POD so a placement new is not required
    Animation* animation = (Animation*)memory;
    animation->current_keyframe = 0;
    animation->keyframes = nframes;
    animation->keyframes_size[0] = wframe;
    animation->keyframes_size[1] = hframe;
    animation->sprite_sheet = sprite_sheet;
    animation->keyframes_origins = (int*)(memory + sizeof(Animation));
    for(int iframe = start_frame; iframe != start_frame + nframes; ++iframe){

        // NOTE(hugo): Determining the position of the frame in the sprite sheet
        int ysheet = iframe / wsheet;
        int xsheet = iframe - ysheet * wsheet;

        animation->keyframes_origins[2 * iframe] = xsheet * wframe;
        animation->keyframes_origins[2 * iframe + 1] = ysheet * hframe;
    }

    return animation;
}

SDL_Rect KeyFrameMask(Animation* animation){
    // NOTE(hugo): Replace this with an assert or keep the flexibility ?
    assert(animation->current_keyframe < animation->keyframes);
    //keyframe_index = animation->current_keyframe % animation->keyframes;

    SDL_Rect mask = {
        animation->keyframes_origins[2 * animation->current_keyframe],
        animation->keyframes_origins[2 * animation->current_keyframe + 1],
        animation->keyframes_size[0],
        animation->keyframes_size[1]};
    return mask;
}

void AnimationBlit(Animation* animation, SDL_Renderer* renderer, int x, int y){
    SDL_Rect keyframe_mask = KeyFrameMask(animation);
    SDL_Rect blit_area = {x, y, keyframe_mask.w, keyframe_mask.h};
    SDL_RenderCopy(renderer, animation->sprite_sheet, &keyframe_mask, &blit_area);
}
void AnimationBlitCentered(Animation* animation, SDL_Renderer* renderer, int x, int y){
    SDL_Rect keyframe_mask = KeyFrameMask(animation);
    SDL_Rect blit_area = {x - (keyframe_mask.w / 2), y - (keyframe_mask.h / 2), keyframe_mask.w, keyframe_mask.h};
    SDL_RenderCopy(renderer, animation->sprite_sheet, &keyframe_mask, &blit_area);
}

// return: random number in [min, max]
int randint(int min, int max){
    return min + (rand() % (max - min + 1));
}

// return: random float in [min, max] with a resolution of (1 / RAND_MAX) * (max - min)
float randfloat(float min, float max){
    return min + (float)(rand() / RAND_MAX) * (max - min);
}

inline void Destroy(SDL_Renderer* renderer){
    SDL_DestroyRenderer(renderer);
}
inline void Destroy(SDL_Texture* texture){
    SDL_DestroyTexture(texture);
}

inline void Destroy(Animation* animation){
    delete[] animation;
}
