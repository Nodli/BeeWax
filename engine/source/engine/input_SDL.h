#ifndef H_INPUT
#define H_INPUT

struct Device_Button{
    enum Button_State {
        STATE_UP = SDL_RELEASED,
        STATE_DOWN = SDL_PRESSED,
    };

    Button_State previous_state(){
        // NOTE(hugo): the button state is the opposite to the current after each (pressed, released) pair
        if(npressed == nreleased){
            return state;
        }
        return (state == STATE_UP) ? STATE_DOWN : STATE_UP;
    }

    Button_State state = STATE_UP;
    u64 state_generation = 0u;
    u32 npressed = 0u;
    u32 nreleased = 0u;
};

#endif
