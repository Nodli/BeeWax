void free_texture_animation_asset(Texture_Animation_Asset* asset){
    asset->frames.free();
}

void Texture_Animation_Player::terminate(){
    animating_queue.free();
}

Texture_Animation_Reference Texture_Animation_Player::start_playing(const Texture_Animation_Asset* asset){
    Component_Reference ref;
    Play_Info* info = animating_queue.create(ref);
    info->asset = asset;
    info->cursor = 0u;

    return {ref};
}

Texture_View Texture_Animation_Player::get_frame(const Texture_Animation_Reference& reference){
    Play_Info* play_info = animating_queue.search(reference);
    assert(play_info);

    return {};
}

void Texture_Animation_Player::stop_playing(const Texture_Animation_Reference& reference){
    animating_queue.remove(reference);
}

bool Texture_Animation_Player::is_playing(const Texture_Animation_Reference& reference){
    return animating_queue.is_valid(reference);
}

void Texture_Animation_Player::next_frame(){
    for(auto& anim : animating_queue.storage){
        Play_Info& info = anim.data;
        ++info.frame_counter;
        if(info.frame_counter == info.asset->frame_duration){
            info.frame_counter = 0u;
            ++info.frame_index;
            if(info.frame_index == info.asset->frames.size){
                info.frame_index = 0u;
            }
        }
    }
}
