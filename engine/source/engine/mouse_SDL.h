#ifndef H_MOUSE_STATE
#define H_MOUSE_STATE

struct Mouse_State{
    void initialize(){
        for(u32 ibutton = 0u; ibutton != carray_size(button.storage); ++ibutton){
            button.storage[ibutton].state = Device_Button::STATE_UP;
            button.storage[ibutton].state_generation = 0u;
            button.storage[ibutton].npressed = 0u;
            button.storage[ibutton].nreleased = 0u;
        }
    }

    void next_frame(){
        for(u32 ibutton = 0u; ibutton != carray_size(button.storage); ++ibutton){
            button.storage[ibutton].npressed = 0u;
            button.storage[ibutton].nreleased = 0u;
        }

        wheel.xticks = 0u;
        wheel.yticks = 0u;

        motion.previous_position = motion.position;
        motion.amplitude = {0u, 0u};
    }

    void register_event(SDL_Event& event){
        if(event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP){
            auto register_button = [&](Device_Button& button){
                assert(event.button.state == SDL_PRESSED || event.button.state == SDL_RELEASED);
                if(event.button.state == SDL_PRESSED){
                    ++button.npressed;
                    button.state = Device_Button::STATE_DOWN;
                    button.state_generation = state_generation++;
                }
                if(event.button.state == SDL_RELEASED){
                    ++button.nreleased;
                    button.state = Device_Button::STATE_UP;
                    button.state_generation = state_generation++;
                }
            };

            switch(event.button.button){
                case SDL_BUTTON_LEFT:
                    register_button(button.left);
                    break;

                case SDL_BUTTON_MIDDLE:
                    register_button(button.middle);
                    break;

                case SDL_BUTTON_RIGHT:
                    register_button(button.right);
                    break;

                case SDL_BUTTON_X1:
                    register_button(button.x1);
                    break;

                case SDL_BUTTON_X2:
                    register_button(button.x2);
                    break;
            }

        }else if(event.type == SDL_MOUSEWHEEL){
            s32 direction = (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) ? -1 : 1;
            wheel.xticks += event.wheel.x * direction;
            wheel.yticks += event.wheel.y * direction;

        }else if(event.type == SDL_MOUSEMOTION){
            motion.position = {event.motion.x, event.motion.y};
            motion.amplitude.x += (u32)abs(event.motion.xrel);
            motion.amplitude.y += (u32)abs(event.motion.yrel);
        }
    }

    // ---- data

    u64 state_generation = 0u;
    union{
        Device_Button storage[5] = {};
        struct{
            Device_Button left;
            Device_Button middle;
            Device_Button right;
            Device_Button x1;
            Device_Button x2;
        };
    } button;
    struct{
        s32 xticks = 0u;
        s32 yticks = 0u;
    } wheel;
    struct{
        ivec2 position = {};
        ivec2 previous_position = {};
        uivec2 amplitude = {};
    } motion;
};

#endif
