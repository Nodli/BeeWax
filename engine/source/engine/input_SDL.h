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
    bool is_down(){
        return state == STATE_DOWN;
    }
    bool is_up(){
        return state == STATE_DOWN;
    }


    Button_State state;
    u64 state_generation;
    u32 npressed;
    u32 nreleased;
};

#endif
