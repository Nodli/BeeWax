struct Editor;

// NOTE(hugo): not at DLL boundary ie virtual methods
struct Module{
    template<typename T>
    static Module* default_creator(){ return (Module*)bw_new(T); }

    virtual ~Module(){};
    virtual void update(Editor*){};
    virtual void render(Editor*){};
};

struct Editor{
    Editor();
    ~Editor();

    enum Editor_Action{
        OPEN_UI = 0u,
        CAMERA_TRIGGER,
        CAMERA_CURSOR,
        CAMERA_ZOOM,
        NUMBER_OF_EDITOR_ACTIONS,
    };

    struct Module_Declaration{
        const char* name;
        Module* (*create_module)();
    };

    void update();
    void render();

    // ----

    Camera_2D scene_camera;

    struct{
        vec2 previous_cursor_screen;
        vec2 previous_cursor_canvas;
        vec2 cursor_screen;
        vec2 cursor_canvas;
    } editor_state;

    array<Module_Declaration> module_registry;

    struct{
        bool active;
        s32 imodule;
    } ui_state;

    Module* module;
};

#define EDITOR_REGISTER_MODULE(Editor_ptr, Module)                  \
do{                                                                 \
    Editor::Module_Declaration declaration;                         \
    declaration.name = STRINGIFY(Module);                           \
    declaration.create_module = &Module::default_creator<Module>;   \
    (*Editor_ptr).module_registry.push(declaration);                \
}while(false)
