#ifndef H_DEVICE_STATE
#define H_DEVICE_STATE

// ----- MANUAL -----

// - insert (variable_name, SDL_Keycode) in the FOR_EACH_KEYBOARD_BUTTON macro

// - use Keyboard_State.reset() when starting a frame
// - use Keyboard_State.register_event() to process the frame events
// - use Keyboard_State.name to access the Device_Button
// state_generation serves as a way to know the order in which buttons were pressed

// ------------------

#define FOR_EACH_KEYBOARD_BUTTON(FUNCTION)  \
FUNCTION(function_F1, SDLK_F1)              \
FUNCTION(arrow_left, SDLK_LEFT)             \
FUNCTION(arrow_right, SDLK_RIGHT)           \
FUNCTION(arrow_up, SDLK_UP)                 \
FUNCTION(arrow_down, SDLK_DOWN)             \
FUNCTION(enter, SDLK_RETURN)                \
FUNCTION(space, SDLK_SPACE)                 \
FUNCTION(escape, SDLK_ESCAPE)               \

// ---- codegen

struct Keyboard_State{
#define ADD_TO_ENUM_KEYBOARD_BUTTON(name, keycode) CONCATENATE(BUTTON_, name),
    enum Keyboard_Buttons{
        FOR_EACH_KEYBOARD_BUTTON(ADD_TO_ENUM_KEYBOARD_BUTTON)
        NUMBER_OF_KEYBOARD_BUTTONS
    };
#undef ADD_TO_ENUM_KEYBOARD_BUTTON

    void reset(){
        for(u32 ibutton = 0; ibutton != carray_size(storage); ++ibutton){
            storage[ibutton].npressed = 0u;
            storage[ibutton].nreleased = 0u;
        }
    }

    void register_event(SDL_Event& event){
        if((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
        && event.key.repeat == 0u){

            auto register_key = [&](Device_Button& button){
                assert(event.key.state == SDL_PRESSED || event.key.state == SDL_RELEASED);
                if(event.key.state == SDL_PRESSED){
                    ++button.npressed;
                    button.state = Device_Button::STATE_DOWN;
                    button.state_generation = state_generation++;
                }
                if(event.key.state == SDL_RELEASED){
                    ++button.nreleased;
                    button.state = Device_Button::STATE_UP;
                    button.state_generation = state_generation++;
                }
            };


            switch(event.key.keysym.sym){

#define DECLARE_KEYBOARD_REGISTRATION(name, keycode)        \
                case keycode:                               \
                    register_key(name);                     \
                    break;

                FOR_EACH_KEYBOARD_BUTTON(DECLARE_KEYBOARD_REGISTRATION)
#undef DECLARE_KEYBOARD_REGISTRATION

                default:
                    //LOG_TRACE("Keyboard event with an undetected keycode: %d", event.key.keysym.sym);
                    break;
            }
        }
    }

    // ---- data

    u64 state_generation = 0u;
    union{
        Device_Button storage[NUMBER_OF_KEYBOARD_BUTTONS] = {};
        struct{
#define DECLARE_KEYBOARD_BUTTON_VARIABLE(name, keycode) Device_Button name;
            FOR_EACH_KEYBOARD_BUTTON(DECLARE_KEYBOARD_BUTTON_VARIABLE)
#undef DECLARE_KEYBOARD_BUTTON_VARIABLE
        };
    };
};

#endif
