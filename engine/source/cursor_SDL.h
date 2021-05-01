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
    void create();
    void destroy();
    void set_state(const Cursor_State state);

    // ---- data

    SDL_Cursor* cursor[5u];
};

#endif
