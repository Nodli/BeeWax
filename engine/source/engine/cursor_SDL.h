#ifndef H_CURSOR
#define H_CURSOR

enum Cursor_State{
    CURSOR_ARROW,
    CURSOR_WAIT,
    CURSOR_CROSSHAIR,
    CURSOR_COMPASS,
    CURSOR_HAND,
    NUMBER_OF_CURSOR_NAMES,
    CURSOR_NONE = NUMBER_OF_CURSOR_NAMES,
};

struct Cursor_SDL{
    void initialize(){
        cursor[0u] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
        cursor[1u] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
        cursor[2u] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
        cursor[3u] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
        cursor[4u] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
        set_state(CURSOR_ARROW);
    }
    void terminate(){
        SDL_SetCursor(SDL_GetDefaultCursor());
        for(u32 icursor = 0u; icursor != carray_size(cursor); ++icursor){
            SDL_FreeCursor(cursor[icursor]);
        }
    }
    void set_state(Cursor_State state){
        SDL_SetCursor(cursor[state]);
        SDL_ShowCursor(SDL_ENABLE);
    }

    // ---- data

    SDL_Cursor* cursor[5u];
};

#endif
