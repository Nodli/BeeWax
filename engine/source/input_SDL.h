#ifndef H_INPUT
#define H_INPUT

enum Action_Type{
    TYPE_BUTTON,
    TYPE_CURSOR,
    TYPE_AXIS,
};

struct Action_Data{
    Action_Type type;
    union{
        struct {
            u32 active;
            u32 nstart;
            u32 nstop;
            u32 nrepeat;
        } button;
        struct {
            int position_x;
            int position_y;
            u32 amplitude_x;
            u32 amplitude_y;
        } cursor;
        struct {
            float value;
        } axis;
    };
};

enum Event_Source{
    UNKNOWN_EVENT_SOURCE = 0u,

    KEYBOARD_BASE_EVENT_TYPE,
    KEYBOARD_ARROW_LEFT = KEYBOARD_BASE_EVENT_TYPE,
    KEYBOARD_ARROW_RIGHT,
    KEYBOARD_ARROW_UP,
    KEYBOARD_ARROW_DOWN,
    KEYBOARD_ARROW_RETURN,
    KEYBOARD_ARROW_SPACE,
    KEYBOARD_LAST_EVENT_TYPE,

    MOUSE_BASE_EVENT_TYPE = KEYBOARD_LAST_EVENT_TYPE,
    MOUSE_BUTTON_LEFT = MOUSE_BASE_EVENT_TYPE,
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_EXTRA_1,
    MOUSE_BUTTON_EXTRA_2,
    MOUSE_WHEEL_VERTICAL,
    MOUSE_WHEEL_HORIZONTAL,
    MOUSE_MOTION,
    MOUSE_NUMBER_OF_EVENT_TYPES,

    NUMBER_OF_EVENT_TYPES = MOUSE_NUMBER_OF_EVENT_TYPES,
};

namespace BEEWAX_INTERNAL{
    constexpr u32 input_max_action_index = 256u;
};

struct Action_Manager{
    void create();
    void destroy();

    void register_action(const u32 action_index, const Event_Source source);
    void remove_action(const u32 action_index);

    Action_Data get_action(const u32 action_index);

    void new_frame();
    bool register_event(const SDL_Event& event);

    // ---- data

    u32 action_mapping[BEEWAX_INTERNAL::input_max_action_index];
    Action_Data actions[NUMBER_OF_EVENT_TYPES];
};

#endif
