void Cursor_SDL::create(){
    cursor[0u] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    cursor[1u] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
    cursor[2u] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
    cursor[3u] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
    cursor[4u] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    set_state(CURSOR_ARROW);
}

void Cursor_SDL::destroy(){
    SDL_SetCursor(SDL_GetDefaultCursor());
    for(u32 icursor = 0u; icursor != carray_size(cursor); ++icursor){
        SDL_FreeCursor(cursor[icursor]);
    }
}

void Cursor_SDL::set_state(const Cursor_State state){
    SDL_SetCursor(cursor[state]);
    SDL_ShowCursor(SDL_ENABLE);
}
