void Action_Manager::create(){
    memset(&action_mapping[0u], 0x00, sizeof(action_mapping));
    memset(&actions[0u], 0x00, sizeof(actions));

    for(u32 ikeyb = KEYBOARD_BASE_EVENT_TYPE; ikeyb != KEYBOARD_LAST_EVENT_TYPE; ++ikeyb){
        actions[ikeyb].type = TYPE_BUTTON;
    }

    actions[MOUSE_BUTTON_LEFT].type         = TYPE_BUTTON;
    actions[MOUSE_BUTTON_MIDDLE].type       = TYPE_BUTTON;
    actions[MOUSE_BUTTON_RIGHT].type        = TYPE_BUTTON;
    actions[MOUSE_BUTTON_EXTRA_1].type      = TYPE_BUTTON;
    actions[MOUSE_BUTTON_EXTRA_2].type      = TYPE_BUTTON;
    actions[MOUSE_WHEEL_VERTICAL].type      = TYPE_AXIS;
    actions[MOUSE_WHEEL_HORIZONTAL].type    = TYPE_AXIS;
    actions[MOUSE_POSITION].type            = TYPE_CURSOR;
}

void Action_Manager::destroy(){
}

void Action_Manager::register_action(const u32 action_index, const Event_Source source){
    action_mapping[action_index] = source;
}

void Action_Manager::remove_action(const u32 action_index){
    action_mapping[action_index] = Event_Source::UNKNOWN_EVENT_SOURCE;
}

Action_Data Action_Manager::get_action(const u32 action_index){
    u32 index = action_mapping[action_index];
    assert(index);
    return actions[index];
}

void Action_Manager::new_frame(){
    for(u32 iaction = 0u; iaction != sizeof(actions) / sizeof(actions[0u]); ++iaction){
        Action_Data& action = actions[iaction];
        switch(action.type){
            case TYPE_BUTTON:
                action.button.nrepeat = action.button.active && !action.button.nstop;
                action.button.nstart = 0u;
                action.button.nstop = 0u;
                break;
            case TYPE_CURSOR:
                action.cursor.amplitude_x = 0;
                action.cursor.amplitude_y = 0;
                break;
            case TYPE_AXIS:
                action.axis.value = 0.f;
                break;

            default:
                assert(false);
        }
    }
}

bool Action_Manager::register_event(const SDL_Event& event){
    auto register_type_button = [&](Action_Data& data){
        if(event.key.state == SDL_PRESSED){
            ++data.button.nstart;
            data.button.active = 1u;

        }else if(event.key.state == SDL_RELEASED){
            ++data.button.nstop;
            data.button.active = 0u;

        }
    };

    auto register_type_cursor = [&](Action_Data& data){
        data.cursor.position_x = event.motion.x;
        data.cursor.position_y = event.motion.y;
        data.cursor.amplitude_x += (u32)abs(event.motion.x);
        data.cursor.amplitude_y += (u32)abs(event.motion.y);
    };

    auto register_type_axis = [&](Action_Data& data, float value){
        data.axis.value = value;
    };

    // NOTE(hugo): keyboard
    if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP){
        switch(event.key.keysym.sym){
            case SDLK_LEFT:     register_type_button(actions[KEYBOARD_ARROW_LEFT]);     return true;
            case SDLK_RIGHT:    register_type_button(actions[KEYBOARD_ARROW_RIGHT]);    return true;
            case SDLK_UP:       register_type_button(actions[KEYBOARD_ARROW_UP]);       return true;
            case SDLK_DOWN:     register_type_button(actions[KEYBOARD_ARROW_DOWN]);     return true;
            case SDLK_RETURN:   register_type_button(actions[KEYBOARD_ARROW_RETURN]);   return true;
            case SDLK_SPACE:    register_type_button(actions[KEYBOARD_ARROW_SPACE]);    return true;
        };

    // NOTE(hugo): mouse
    }else if(event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP){
        switch(event.button.button){
            case SDL_BUTTON_LEFT:       register_type_cursor(actions[MOUSE_BUTTON_LEFT]);       return true;
            case SDL_BUTTON_MIDDLE:     register_type_cursor(actions[MOUSE_BUTTON_MIDDLE]);     return true;
            case SDL_BUTTON_RIGHT:      register_type_cursor(actions[MOUSE_BUTTON_RIGHT]);      return true;
            case SDL_BUTTON_X1:         register_type_cursor(actions[MOUSE_BUTTON_EXTRA_1]);    return true;
            case SDL_BUTTON_X2:         register_type_cursor(actions[MOUSE_BUTTON_EXTRA_2]);    return true;
        }

    }else if(event.type == SDL_MOUSEWHEEL){
        s32 axis_sign = (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) ? -1 : 1;
        register_type_axis(actions[MOUSE_WHEEL_HORIZONTAL], (float)(event.wheel.x * axis_sign));
        register_type_axis(actions[MOUSE_WHEEL_VERTICAL], (float)(event.wheel.y * axis_sign));
        return true;

    }else if(event.type == SDL_MOUSEMOTION){
        register_type_cursor(actions[MOUSE_POSITION]);
        return true;
    }

    return false;
}
