void Texture_Animation_Player::terminate(){
    to_play.free();
}

Texture_Animation_Playing_ID Texture_Animation_Player::start_playing(Texture_Animation_Asset* asset){
    Play_Data play_data;
    play_data.asset = asset;
    play_data.counter = 0u;
    play_data.frame_index = 0u;

    u32 index = to_play.insert(play_data);

    return index;
}
void Texture_Animation_Player::stop_playing(Texture_Animation_Playing_ID play){
    to_play.remove(play);
}

Texture_Animation_Frame* Texture_Animation_Player::get_frame(Texture_Animation_Playing_ID play){
    Play_Data& play_data = to_play[play];
    return &play_data.asset->frames[play_data.frame_index];
}

void Texture_Animation_Player::next_frame(){
    u32 index, counter;
    for(index = to_play.get_first(), counter = 0u;
            index < to_play.capacity;
            index = to_play.get_next(index), ++counter){

        Play_Data& play = to_play[index];
        ++play.counter;
        if(play.counter == play.asset->frame_duration){
            play.counter = 0u;
            ++play.frame_index;
            if(play.frame_index == play.asset->frames.size){
                play.frame_index = 0u;
            }
        }
    }
}

