#include "../engine/easy_setup.h"
#include "../engine/easy_setup.cpp"

struct Integration_Scene{
    void on_push(){
        camera = {{0.f, 0.f}, 2.f, g_engine.window.aspect_ratio()};

        buffer = g_engine.renderer.get_transient_buffer(sizeof(vertex_xyzrgba) * 3u);
        g_engine.renderer.format(buffer, xyzrgba);

        indexed_buffer = g_engine.renderer.get_transient_buffer_indexed(sizeof(vertex_xyzrgba) * 4u, sizeof(u16) * 6u);
        g_engine.renderer.format(indexed_buffer, xyzrgba);

        song = g_engine.audio_catalog.search("song");

        vg_renderer.renderer = &g_engine.renderer;
    }
    void update(){
        if(g_engine.keyboard.arrow_left.is_down()) rect_w += 0.0002f;
        if(g_engine.keyboard.arrow_right.is_down()) rect_w -= 0.0002f;

        if(g_engine.keyboard.arrow_up.is_down()) rect_h += 0.0001f;
        if(g_engine.keyboard.arrow_down.is_down()) rect_h -= 0.0001f;

        if(g_engine.keyboard.arrow_left.is_down()) segment_w += 0.0001f;
        if(g_engine.keyboard.arrow_right.is_down()) segment_w -= 0.0001f;

        if(g_engine.keyboard.space.npressed){
            if(!g_engine.audio.is_valid(song_ref)) song_ref = g_engine.audio.start(song);
            else g_engine.audio.stop(song_ref);
        }

        if(g_engine.keyboard.backquote.npressed){
            u32 current_frame = g_engine.frame_timing.frame_count;
            if(current_frame - show_ImGui_frame > input_delay){
                ImGui::SetNextWindowPos({0.f, 0.f});
                ImGui::SetNextWindowSize({(float)g_engine.window.width * 0.25f, (float)g_engine.window.height});

                show_ImGui = !show_ImGui;
                show_ImGui_frame = current_frame;
            }
        }

        if(show_ImGui){
            if(ImGui::Begin("ImGui easy_setup")){

                ImGui::Text("Text: Default Color");

                ImGui::PushStyleColor(ImGuiCol_Text, {1.f, 0.f, 0.f, 1.f});
                ImGui::Text("Text: Red");
                ImGui::PopStyleColor();

                ImGui::Separator();

                ImGui::PushStyleColor(ImGuiCol_Button, {0.f, 0.f, 0.f, 0.f});
                if(ImGui::Button("Button")) state_button = !state_button;
                if(state_button) ImGui::Text("Text: Button");
                ImGui::PopStyleColor();

                ImGui::Separator();

                if(ImGui::ArrowButton("Arrow None", ImGuiDir_None)) state_arrow_button = !state_arrow_button;
                if(ImGui::ArrowButton("Arrow Right", ImGuiDir_Right)) state_arrow_button = !state_arrow_button;
                if(ImGui::ArrowButton("Arrow Left", ImGuiDir_Left)) state_arrow_button = !state_arrow_button;
                if(ImGui::ArrowButton("Arrow Up", ImGuiDir_Up)) state_arrow_button = !state_arrow_button;
                if(ImGui::ArrowButton("Arrow Down", ImGuiDir_Down)) state_arrow_button = !state_arrow_button;
                if(state_arrow_button) ImGui::Text("Text: Arrow Button");

                ImGui::Separator();

                if(ImGui::SmallButton("Small Button")) state_small_button = !state_small_button;
                if(state_small_button) ImGui::Text("Text:: Small Button");

                ImGui::Separator();

                if(ImGui::RadioButton("Radio Button 1", state_radio_button == 0u)){
                    state_radio_button = 0u;
                }
                if(ImGui::RadioButton("Radio Button 2", state_radio_button == 1u)){
                    state_radio_button = 1u;
                }

                switch(state_radio_button){
                    case 0u:
                        ImGui::Text("Text: Radio Button 1");
                        break;
                    case 1u:
                        ImGui::Text("Text: Radio Button 2");
                        break;
                    default:
                        ImGui::Text("Text: No Radio Button");
                        break;
                }

                ImGui::Separator();

                ImGui::Checkbox("Checkbox A", &state_checkbox);
                if(state_checkbox) ImGui::Text("Text: Checkbox");

                ImGui::Separator();

                if(ImGui::Button("Progress Button")) ++state_progress;
                ImGui::ProgressBar((float)state_progress / 100.f);

                ImGui::Separator();

                ImGui::Bullet();
                ImGui::Text("Text: Bullet 1");

                ImGui::Bullet();
                ImGui::Text("Text: Bullet 2");

                ImGui::Text("Text: No Bullet");

                ImGui::Separator();

                const char* combo_name[] = {
                    "Text: Combo 0",
                    "Text: Combo 1",
                    "Text: Combo 2",
                    "Text: Combo 3",
                    "Text: Combo 4",
                };
                if(ImGui::BeginCombo("Text: Combo", combo_name[state_combo])){
                    if(ImGui::Button("Text: Combo 1")) state_combo = 1u;
                    if(ImGui::Button("Text: Combo 2")) state_combo = 2u;
                    if(ImGui::Button("Text: Combo 3")) state_combo = 3u;
                    if(ImGui::Button("Text: Combo 4")) state_combo = 4u;

                    ImGui::EndCombo();
                }

                ImGui::Separator();

                ImGui::DragFloat("DragFloat A", &state_dragfloat);
                ImGui::DragFloat2("DragFloat B", state_dragfloat2);

                ImGui::Separator();

                ImGui::SliderFloat("SliderFloat", &state_sliderfloat, 0.f, 1.f);
                ImGui::VSliderFloat("VSliderFloat", {100.f, 100.f}, &state_sliderfloat, 0.f, 2.f);

                ImGui::Separator();

                ImGui::InputText("Label: InputTest", state_inputtext, 128u);

                ImGui::Separator();

                ImGui::ColorEdit4("Label: ColorEdit4", state_coloredit4);
                ImGui::ColorPicker4("Label: ColorPicker4", state_colorpicker4);
                ImGui::ColorButton("Label: ColorButton", {0.f, 0.f, 0.f, 0.f});
                ImGui::ColorButton("Label: ColorButton", {1.f, 0.f, 0.f, 0.f});
                ImGui::ColorButton("Label: ColorButton", {0.f, 1.f, 0.f, 0.f});
                ImGui::ColorButton("Label: ColorButton", {0.f, 0.f, 1.f, 0.f});
            }
            ImGui::End();
        }

    }
    void render(){
        vg_renderer.next_frame();

        {
            uniform_camera_2D camera_matrix = {to_std140(camera.camera_matrix())};
            g_engine.renderer.update_uniform(camera_2D, (void*)&camera_matrix);
        }

        {
            g_engine.renderer.checkout(buffer);
            vertex_xyzrgba* vert = (vertex_xyzrgba*)buffer.ptr;
            vert[0u] = {{-0.5f, -0.5f, 0.1f}, {1.f, 1.f, 0.f, 1.f}};
            vert[1u] = {{ 0.5f, -0.5f, 0.1f}, {1.f, 0.f, 1.f, 1.f}};
            vert[2u] = {{ 0.0f, +0.5f, 0.1f}, {0.f, 1.f, 1.f, 1.f}};
            g_engine.renderer.commit(buffer);
        }

        {
            g_engine.renderer.checkout(indexed_buffer);
            vertex_xyzrgba* vert = (vertex_xyzrgba*)indexed_buffer.vptr;
            vert[0u] = {{-1.f, -0.5f, 0.1f}, {1.f, 1.f, 0.f, 1.f}};
            vert[1u] = {{-0.5f, -0.5f, 0.1f}, {1.f, 0.f, 1.f, 1.f}};
            vert[2u] = {{-0.5f, +0.5f, 0.1f}, {0.f, 1.f, 1.f, 1.f}};
            vert[3u] = {{-1.f, +0.5f, 0.1f}, {1.f, 1.f, 1.f, 1.f}};
            u16* idx = (u16*)indexed_buffer.iptr;
            idx[0u] = 0u;
            idx[1u] = 1u;
            idx[2u] = 3u;
            idx[3u] = 3u;
            idx[4u] = 1u;
            idx[5u] = 2u;
            g_engine.renderer.commit(indexed_buffer);
        }

        {
            float dpix = (float)camera.height / (float)g_engine.render_target.height;
            vg_renderer.rect({0.75f - rect_w * 0.5f, 0.75f - rect_h * 0.5f}, {0.75f + rect_w * 0.5f, 0.75f + rect_h * 0.5f}, 0.001f, {1.f, 0.f, 0.f, 1.f}, dpix, false);
            vg_renderer.disc({0.25f, 0.25f}, 0.25f, 0.0001f, {1.f, 1.f, 0.f, 1.f}, dpix, false);
            vg_renderer.segment({0.90f, 0.1f}, {1.f, 0.8f}, segment_w, 0.01f, {1.f, 0.f, 1.f, 1.f}, dpix, false);
        }

        g_engine.renderer.use_shader(polygon_2D);
        g_engine.renderer.draw(buffer, PRIMITIVE_TRIANGLES, 0u, 3u);
        g_engine.renderer.draw(indexed_buffer, PRIMITIVE_TRIANGLES, TYPE_USHORT, 6u, 0u);
        vg_renderer.draw();
    }
    void on_remove(){
        g_engine.renderer.free_transient_buffer(buffer);
        vg_renderer.terminate();
    }

    // ---- data

    Camera_2D camera;
    Transient_Buffer buffer = {};
    Transient_Buffer_Indexed indexed_buffer = {};

    Audio_Asset* song = nullptr;
    Audio_Reference song_ref = unknown_audio_reference;

    float rect_w = 0.21f;
    float rect_h = 0.1f;
    float segment_w = 0.21f;
    Vector_Graphics_Renderer vg_renderer;

    // -- GUI

    u32 input_delay = 10u;

    u32 show_ImGui_frame = 0u;
    bool show_ImGui = false;

    bool state_button = false;
    bool state_arrow_button = false;
    bool state_small_button = false;
    s32 state_radio_button = -1u;
    bool state_checkbox = false;
    u32 state_progress = 0u;
    u32 state_combo = 0u;
    float state_dragfloat = 0.f;
    float state_dragfloat2[2] = {0.f, 0.f};
    float state_sliderfloat = 0.f;
    char state_inputtext[128] = "";
    float state_coloredit4[4u] = {0.f, 0.f, 0.f, 0.f};
    float state_colorpicker4[4u] = {0.f, 0.f, 0.f, 0.f};
};

void easy_config(){
    g_config::window_name = "integration_easy_setup";
    g_config::window_width = 1280;
    g_config::window_height = 720;
    g_config::render_width = 256;
    g_config::render_height = 144;
    g_config::asset_catalog_path = "./data/asset_catalog.json";
}
void* easy_setup(){
    g_engine.scene.push_scene<Integration_Scene>("Integration_Scene");
    return nullptr;
}
void easy_terminate(void* user_data){}

#if 0
struct Rope_Reference : Component_Reference {};
constexpr Rope_Reference unknown_rope_reference = {unknown_component_reference};

struct Rope_Header{
    u32 nnodes;
};
struct Rope_Node{
    vec2 position;
    vec2 previous_position;
    vec2 force;
    float mass;
};

static Rope_Header* malloc_rope(u32 nnodes){
    size_t header_bytes = sizeof(Rope_Header) + align_offset_next(sizeof(Rope_Header), alignof(Rope_Node));
    size_t bytesize = header_bytes + nnodes * sizeof(Rope_Node);
    return (Rope_Header*)malloc(bytesize);
}
static Rope_Node* node_from_header(Rope_Header* ptr){
    return (Rope_Node*)((u8*)ptr + align_offset_next(sizeof(Rope_Header), alignof(Rope_Node)));
}

struct Physics{

    Rope_Reference create_rope(u32 nnodes){
        Rope_Header* header = malloc_rope(nnodes, header, rope);
        *header.nnodes = nnodes;
        for(u32 inode = 0u; inode != 10u;


    }
    void remove_rope(const Rope_Reference& ref);

    void update(float dt){
        for(u32 inode = 0u; inode != rope.size; ++inode){
            Rope_Node& node = nodes[inode];
            vec2 position_copy = position;

            vec2 acceleration = node.mass * node.force;

            node.position = 2.f * node.position - node.previous_position + acceleration * dt * dt;
            node.previous_position = position_copy;
        }
    }

    // ---- data

    array<Rope_Node> node;
};
#endif
