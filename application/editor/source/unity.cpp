using namespace bw;

static Engine* g_engine_ptr;

void set_engine_ptr(Engine* ptr){g_engine_ptr = ptr;};
Engine& get_engine(){return *g_engine_ptr;};

// ----

#include "imdrawer.h"
#include "imdrawer.cpp"

#include "editor.h"
#include "editor.cpp"

// ----

bool ImGuiInputText(const char* label, array<char>* buffer, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
bool ImGuiInputTextMultiline(const char* label, array<char>* buffer, const ImVec2& size, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
bool ImGuiInputTextWithHint(const char* label, const char* hint, array<char>* buffer, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
void ImGuiTextCentered(const char* str);

struct ImGuiInputTextCallbackUserData_array{
    array<char>* ptr;
    ImGuiInputTextCallback chain_callback;
    void* chain_callback_user_data;
};

int ImGuiInputTextCallback_array(ImGuiInputTextCallbackData* data){
    ImGuiInputTextCallbackUserData_array* user_data = (ImGuiInputTextCallbackUserData_array*)data->UserData;

    // NOTE(hugo): resize buffer
    if(data->EventFlag == ImGuiInputTextFlags_CallbackResize){
        array<char>* array = user_data->ptr;
        assert(data->Buf == array->data);

        array->resize(data->BufTextLen);
        data->Buf = array->data;

    // NOTE(hugo): forward callback
    }else if(user_data->chain_callback){
        data->UserData = user_data->chain_callback_user_data;
        return user_data->chain_callback(data);

    }

    return 0;
}

bool ImGuiInputText(const char* label, array<char>* buffer, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data){
    assert((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    ImGuiInputTextCallbackUserData_array cb_user_data;
    cb_user_data.ptr = buffer;
    cb_user_data.chain_callback = callback;
    cb_user_data.chain_callback_user_data = user_data;
    return ImGui::InputText(label, buffer->data, buffer->capacity, flags, &ImGuiInputTextCallback_array, &cb_user_data);
}

bool ImGuiInputTextMultiline(const char* label, array<char>* buffer, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data){
    assert((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    ImGuiInputTextCallbackUserData_array cb_user_data;
    cb_user_data.ptr = buffer;
    cb_user_data.chain_callback = callback;
    cb_user_data.chain_callback_user_data = user_data;
    return ImGui::InputTextMultiline(label, buffer->data, buffer->capacity, size, flags, &ImGuiInputTextCallback_array, &cb_user_data);
}

bool ImGuiInputTextWithHint(const char* label, const char* hint, array<char>* buffer, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data){
    assert((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    ImGuiInputTextCallbackUserData_array cb_user_data;
    cb_user_data.ptr = buffer;
    cb_user_data.chain_callback = callback;
    cb_user_data.chain_callback_user_data = user_data;
    return ImGui::InputTextWithHint(label, hint, buffer->data, buffer->capacity, flags, &ImGuiInputTextCallback_array, &cb_user_data);
}

void ImGuiTextCentered(const char* str){
    float wwidth = ImGui::GetWindowSize().x;
    float twidth = ImGui::CalcTextSize(str).x;

    ImGui::SetCursorPosX((wwidth - twidth) * 0.5f);
    ImGui::Text(str);
}

// ----

struct Random_Packing_v1 : Module{

    static constexpr vec4 background_default_color = {0.5f, 0.5f, 0.5f, 1.f};
    static constexpr vec4 palette_default_color = {0.f, 0.f, 0.f, 1.f};

    static constexpr float canvas_size = 1000.f;
    static constexpr float canvas_hsize = canvas_size * 0.5f;

    static constexpr u32 canvas_resolution = 4096u;
    static constexpr u32 canvas_sampling = 16u;

    static constexpr u32 max_ncircles = 1024u;
    static constexpr float max_max_radius = canvas_size * 0.5f;
    static constexpr float min_min_radius = canvas_size * 0.0001f;

    static constexpr const char default_savename[] = "output.png";

    struct Circle{
        vec2 position;
        float radius;
        u32 ipalette_color;
        bool can_have_liner;

        bool operator>(const Circle& c) const {return radius > c.radius;};
        bool operator<(const Circle& c) const {return radius < c.radius;};
    };

    // ----

    Random_Packing_v1(){
        ncircles = 256u;
        max_radius = 10.f;
        min_radius = 1.f;
        random_rejection = 512u;

        savename.create();
        savename.resize(sizeof(default_savename));
        memcpy(savename.data, default_savename, sizeof(default_savename));

        background_color = background_default_color;
        palette.create();
        palette.push(palette_default_color);
        ui_state.focus_ptr = nullptr;
        ui_state.focus_palette_index = -1;
        ui_state.error_string = nullptr;
        drawer.create();
        texture = Render_Layer_Invalid_Texture;
    }
    ~Random_Packing_v1(){
        savename.destroy();
        palette.destroy();
        drawer.destroy();
        if(texture != Render_Layer_Invalid_Texture) get_engine().render_layer.free_texture(texture);
    }

    void generate(){
        ui_state.error_string = nullptr;

        if(palette.size == 0u){
            ui_state.error_string = "Empty Palette";
            return;
        }

        if(texture != Render_Layer_Invalid_Texture) get_engine().render_layer.free_texture(texture);

        // NOTE(hugo): generate packing
        array<Circle> packing;
        packing.create();
        packing.reserve(ncircles);

        float dradius = max_radius - min_radius;

        u32 nrejection = 0u;

        while(packing.size != ncircles){
            vec2 position = {random_float() * canvas_size - canvas_hsize, random_float() * canvas_size - canvas_hsize};
            float radius = min_radius + random_float() * dradius;
            u32 icolor = random_u32_range_uniform(palette.size);

            s32 inside_index = -1;

            for(u32 ipacking = 0u; ipacking != packing.size; ++ipacking){
                Circle& pcircle = packing[ipacking];
                float pdist = length(pcircle.position - position);

                // NOTE(hugo): inside packing circle
                if(pdist < pcircle.radius){
                    if(inside_index == -1 || pcircle.radius < packing[inside_index].radius)
                        inside_index = ipacking;

                    continue;
                }

                // NOTE(hugo): distance to packing circle
                radius = min(radius, pdist - pcircle.radius);
            }

            if(inside_index != -1){
                Circle& parent = packing[inside_index];

                u32 insider_type;
                if(parent.can_have_liner){
                    insider_type = random_u32_range_uniform(2u);
                }else{
                    insider_type = 1u;
                }

                // NOTE(hugo): liner
                if(insider_type == 0u){
                    position = parent.position;
                    radius = parent.radius * 0.9f;
                    if(icolor == parent.ipalette_color){
                        u32 ioffset = random_u32_range_uniform(palette.size - 1u);
                        icolor = (icolor + ioffset) % palette.size;
                    }

                // NOTE(hugo): inclusion
                }else if(insider_type == 1u){
                    float dist_parent = length(parent.position - position);
                    assert(parent.radius >= dist_parent);
                    radius = min(radius, parent.radius - dist_parent);
                }
            }

            if(!(abs(radius) < min_radius)){
                if(inside_index != -1) packing[inside_index].can_have_liner = false;
                packing.push({position, radius, icolor, true});
                nrejection = 0u;
            }else{
                ++nrejection;
                if(nrejection == (u32)random_rejection) break;
            }
        }

        qsort<Circle, &comparison_decreasing_order>(packing.data, packing.size);

        // NOTE(hugo): borrow render target
        Render_Target target = get_engine().render_layer.get_render_target_multisample(canvas_resolution, canvas_resolution, canvas_sampling);
        get_engine().render_layer.use_render_target(target);
        get_engine().render_layer.clear_render_target(background_color);

        // NOTE(hugo): setup camera
        float dpix;
        {
            Camera_2D scene_camera;
            scene_camera.center = {0.f, 0.f};
            scene_camera.height = canvas_size;
            scene_camera.aspect_ratio = 1.f;

            mat4 camera_matrix = mat3D_from_mat2D(scene_camera.projection_matrix() * scene_camera.view_matrix());

            uniform_camera u_camera;
            u_camera.matrix = to_std140(camera_matrix);

            get_engine().render_layer.update_uniform(camera, (void*)&u_camera);

            dpix = scene_camera.height / canvas_resolution;
        }
        dpix /= canvas_sampling;

        // NOTE(hugo): draw
        ImDrawer gendrawer;
        gendrawer.create();
        {
            gendrawer.new_frame();

            for(auto& circle : packing){
                vec4 color = palette[circle.ipalette_color];
                gendrawer.command_disc(circle.position, circle.radius, 0.5f, rgba32(color.r, color.g, color.b, color.a), dpix);
            }

            gendrawer.draw();
        }
        gendrawer.destroy();
        packing.destroy();

        // NOTE(hugo): copy to texture and destroy target
        texture = get_engine().render_layer.copy_render_target_to_texture(target);
        get_engine().render_layer.generate_texture_mipmap(texture);
        get_engine().render_layer.free_render_target(target);
    }

    void save(){
        if(texture != Render_Layer_Invalid_Texture){
            void* data = GL::retrieve_texture(texture.texture, texture.width, texture.height);
            int stride = texture.width * 4u * sizeof(char);
            stbi_write_png(savename.data, texture.width, texture.height, 4u, data, stride);
        }
    }

    void update(Editor*){
        if(ImGui::Begin("##Module")){
            ImVec2 generate_button_size = {ImGui::GetWindowContentRegionWidth(), 0.f};

            if(ui_state.error_string){
                ImGui::TextColored({0.9f, 0.4f, 0.4f, 1.f}, ui_state.error_string);
                ImGui::Separator();
            }

            if(ImGui::Button("Generate", generate_button_size)) generate();

            ImGui::PushItemWidth(- generate_button_size.x * 0.5f);
            ImGuiInputText("##SaveName", &savename);
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if(ImGui::Button("Save", {ImGui::GetContentRegionAvail().x, 0.f})) save();

            ImGui::Separator();

            ImGuiTextCentered("Parameters");

            if(ImGui::Button("Randomize##ncircles")) ncircles = random_u32_range_uniform(1024u);
            ImGui::SameLine();
            ImGui::DragInt("ncircles", &ncircles, 1.f, 0.f, INT32_MAX);

            if(ImGui::Button("Randomize##max_radius")) max_radius = random_float() * 1000.f;
            ImGui::SameLine();
            ImGui::DragFloat("max_radius", &max_radius, 1.f, 0.1f, FLT_MAX);
            max_radius = max(0.1f, max_radius);

            if(ImGui::Button("Randomize##min_radius")) min_radius = random_float() * max_radius;
            ImGui::SameLine();
            ImGui::DragFloat("min_radius", &min_radius, 1.f, 0.1f, max_radius);
            min_radius = min_max(min_radius, 0.1f, max_radius);

            ImGui::DragInt("random_rejection", &random_rejection, 1.f);

            ImGui::Separator();

            ImGuiTextCentered("Background");

            ImVec4 im_background_color(background_color.x, background_color.y, background_color.z, background_color.w);
            if(ImGui::ColorButton("##background_color_edit", im_background_color)){
                ui_state.focus_ptr = &background_color;
                ui_state.focus_palette_index = -1;
            }
            background_color = {im_background_color.x, im_background_color.y, im_background_color.z, im_background_color.w};

            ImGui::Separator();

            ImGuiTextCentered("Palette");

            if(ImGui::Button("New")){
                ui_state.focus_ptr = nullptr;
                ui_state.focus_palette_index = palette.size;
                palette.push(palette_default_color);
            }
            ImGui::SameLine();
            if(ImGui::Button("Delete") && ui_state.focus_palette_index != -1){
                palette.remove(ui_state.focus_palette_index);
                ui_state.focus_palette_index = -1;
            }

            for(u32 ipal = 0u; ipal != palette.size; ++ipal){
                ImGui::PushID(ipal);

                vec4& palette_color = palette[ipal];
                ImVec4 im_palette_color(palette_color.x, palette_color.y, palette_color.z, palette_color.w);
                if(ImGui::ColorButton("##palette_color_edit", im_palette_color)){
                    ui_state.focus_palette_index = ipal;
                    ui_state.focus_ptr = nullptr;
                }
                palette_color = {im_palette_color.x, im_palette_color.y, im_palette_color.z, im_palette_color.w};

                ImGui::PopID();

                if(ipal != palette.size - 1u){
                    float next_button_pos = ImGui::GetItemRectMax().x + ImGui::GetStyle().ItemSpacing.x;
                    if(next_button_pos < ImGui::GetWindowContentRegionWidth()) ImGui::SameLine();
                }
            }

            float* color_picker_data = nullptr;
            if(ui_state.focus_ptr)                      color_picker_data = (*ui_state.focus_ptr).data;
            else if(ui_state.focus_palette_index != -1) color_picker_data = palette[ui_state.focus_palette_index].data;

            if(color_picker_data){
                ImGui::Separator();
                ImGui::ColorPicker4("##palette_color_picker", color_picker_data);
            }
        }
        ImGui::End();
    }
    void render(Editor*){
        if(texture != Render_Layer_Invalid_Texture){
            drawer.new_frame();

            float image_aspect_ratio = (float)texture.width / (float)texture.height;
            float image_height = 20.f;
            drawer.command_image(texture, {0.f, 0.f}, {image_aspect_ratio * image_height, image_height}, 0.5f);

            drawer.draw();
        }
    }

    // ----

    s32 ncircles;
    float max_radius;
    float min_radius;
    s32 random_rejection;

    array<char> savename;

    vec4 background_color;
    array<vec4> palette;

    struct{
        vec4* focus_ptr;
        s32 focus_palette_index;

        const char* error_string;
    } ui_state;

    ImDrawer drawer;
    Texture texture;
};

struct Random_Packing_v2 : Module{

    struct Circle{
        vec2 position;
        float radius;
        u32 palette_index;
    };

    // ----

    Random_Packing_v2(){
        drawer.create();
        texture = Render_Layer_Invalid_Texture;
    }
    ~Random_Packing_v2(){
        drawer.destroy();
        if(texture != Render_Layer_Invalid_Texture) get_engine().render_layer.free_texture(texture);
    }

    void generate(){
        if(texture != Render_Layer_Invalid_Texture) get_engine().render_layer.free_texture(texture);

        // --

        u32 ncircles = 1000u;
        u32 ncircles_reduction = 75u;
        u32 max_rejections = 1000u;

        float max_radius_packing = 125.f;
        float min_radius_packing = 5.f;
        float min_radius_packing_randomizer = 15.f;

        float min_radius_subdivision = 0.1f;

        float max_ratio_double = 0.2f;
        float max_spacing_double = 0.2f;

        float min_ratio_liner = 0.05f;
        float max_ratio_liner = 0.1f;

        u32 occurence_liner = 200u;
        u32 occurence_double = 900u;
        u32 occurence_stop = 5u;

        u32 canvas_resolution_horizontal = 1920u;
        u32 canvas_resolution_vertical = 1080u;
        u32 canvas_sampling = 16u;

        float canvas_aspect_ratio = (float)canvas_resolution_horizontal / (float)canvas_resolution_vertical;
        float canvas_height = 1000.f;
        float canvas_width = canvas_height * canvas_aspect_ratio;

        vec4 background_color = {0.5f, 0.5f, 0.5f, 1.f};

        vec4 palette[] = {
            {1.f, 1.f, 1.f, 1.f},
            {0.f, 0.f, 0.f, 1.f}
        };
        u32 palette_size = carray_size(palette);

        u32 occurence_same_color = 1u;
        u32 occurence_different_color = 100u;

        // --

        float radius_multiplier = expf(logf(min_radius_packing / max_radius_packing) / ncircles_reduction);

        assert(palette_size);
        assert(max_ratio_double > 0.f && max_ratio_double < 0.5f);

        u32 occurence_total = occurence_liner + occurence_double + occurence_stop;
        assert(occurence_total > 0u);

        float threshold_liner = (float)occurence_liner / (float)occurence_total;
        float threshold_double = (float)(occurence_liner + occurence_double) / (float)occurence_total;

        float threshold_different_color = (float)occurence_same_color / (float)(occurence_same_color + occurence_different_color);

        auto random_palette_index = [&](u32 parent_index){
            u32 index;
            if(random_float() > threshold_different_color){
                index = random_u32_range_uniform(palette_size - 1u);
                index += (index >= parent_index);

            }else{
                index = parent_index;

            }
            return index;
        };

        // --

        array<Circle> packing;
        packing.create();

        u32 ntry = 0u;

        // NOTE(hugo): circles with diminishing size
        {
            float radius = max_radius_packing;
            while(packing.size != ncircles_reduction && ntry < max_rejections){
                ++ntry;

                float range_width = canvas_width + radius * 0.5f;
                float range_height = canvas_height + radius * 0.5f;
                vec2 position = {
                    (random_float() - 0.5f) * range_width,
                    (random_float() - 0.5f) * range_height
                };

                bool no_overlap = true;

                for(u32 icircle = 0u; icircle != packing.size; ++icircle){
                    float radius_sum = radius + packing[icircle].radius;

                    if(sqlength(position - packing[icircle].position) < radius_sum * radius_sum){
                        no_overlap = false;
                        break;
                    }
                }

                if(no_overlap){
                    Circle new_circle;
                    new_circle.position = position;
                    new_circle.radius = radius;
                    new_circle.palette_index = random_u32_range_uniform(palette_size);

                    packing.push(new_circle);

                    ntry = 0u;
                    radius = max(radius * radius_multiplier, min_radius_packing);
                }
            }
        }

        // NOTE(hugo): circles at min size
        {
            while(packing.size != ncircles && ntry < max_rejections){
                ++ntry;

                float radius = min_radius_packing + random_float() * min_radius_packing_randomizer;
                float range_width = canvas_width + radius * 0.5f;
                float range_height = canvas_height + radius * 0.5f;

                vec2 position = {
                    (random_float() - 0.5f) * range_width,
                    (random_float() - 0.5f) * range_height
                };

                bool no_overlap = true;

                for(u32 icircle = 0u; icircle != packing.size; ++icircle){
                    float radius_sum = radius + packing[icircle].radius;

                    if(sqlength(position - packing[icircle].position) < radius_sum * radius_sum){
                        no_overlap = false;
                        break;
                    }
                }

                if(no_overlap){
                    Circle new_circle;
                    new_circle.position = position;
                    new_circle.radius = radius;
                    new_circle.palette_index = random_u32_range_uniform(palette_size);


                    packing.push(new_circle);

                    ntry = 0u;
                }
            }
        }

        // NOTE(hugo): packing subdivision
        {
            u32 packing_iterator = 0u;
            while(packing_iterator != packing.size){
                Circle parent = packing[packing_iterator];

                if(parent.radius > min_radius_subdivision){
                    float subdivision_roll = random_float();

                    u32 subdivision_type = 0u;
                    if(subdivision_roll > threshold_liner) ++subdivision_type;
                    if(subdivision_roll > threshold_double) ++subdivision_type;

                    switch(subdivision_type){
                        case 0u:
                        {
                            float liner_ratio = (1.f - min_ratio_liner) - random_float() * (max_ratio_liner - min_ratio_liner);

                            u32 palette_index = random_palette_index(parent.palette_index);

                            packing.push({parent.position, parent.radius * liner_ratio, palette_index});
                            break;
                        }
                        case 1u:
                        {
                            float orientation = random_float() * PI;
                            vec2 radius_dir = {bw::cos(orientation), bw::sin(orientation)};

                            float spacing = random_float() * max_spacing_double;
                            float occupied = 1.f - spacing;

                            float spacing_distance = parent.radius * spacing * 0.25f;
                            float occupied_distance = parent.radius * occupied;

                            float ratio = max_ratio_double + random_float() * (1.f - 2.f * max_ratio_double);
                            float radius_cA = occupied_distance * ratio;
                            float radius_cB = occupied_distance - radius_cA;

                            vec2 axis_origin = parent.position - radius_dir * parent.radius;
                            vec2 position_cA = axis_origin + radius_dir * (radius_cA + spacing_distance);
                            vec2 position_cB = position_cA + radius_dir * (radius_cA + radius_cB + 3.f * spacing_distance);

                            u32 palette_index_cA = random_palette_index(parent.palette_index);
                            u32 palette_index_cB = random_palette_index(parent.palette_index);

                            packing.push({position_cA, radius_cA, palette_index_cA});
                            packing.push({position_cB, radius_cB, palette_index_cB});
                            break;
                        }
                        case 2u:
                            break;
                    }
                }

                ++packing_iterator;
            }
        }

        // --

        Render_Target render_target = get_engine().render_layer.get_render_target_multisample(canvas_width, canvas_height, canvas_sampling);
        get_engine().render_layer.use_render_target(render_target);
        get_engine().render_layer.clear_render_target(background_color);

        // NOTE(hugo): setup camera
        float dpix;
        {
            Camera_2D scene_camera;
            scene_camera.center = {0.f, 0.f};
            scene_camera.height = canvas_height;
            scene_camera.aspect_ratio = canvas_aspect_ratio;

            mat4 camera_matrix = mat3D_from_mat2D(scene_camera.projection_matrix() * scene_camera.view_matrix());

            uniform_camera u_camera;
            u_camera.matrix = to_std140(camera_matrix);

            get_engine().render_layer.update_uniform(camera, (void*)&u_camera);

            dpix = scene_camera.height / max(canvas_width, canvas_height);
        }
        dpix /= canvas_sampling;

        // NOTE(hugo): draw
        ImDrawer gendrawer;
        gendrawer.create();
        {
            gendrawer.new_frame();

            for(auto& circle : packing){
                u32 color = rgba32(palette[circle.palette_index]);
                gendrawer.command_disc(circle.position, circle.radius, 0.5f, color, dpix);
            }

            gendrawer.draw();
        }
        gendrawer.destroy();

        packing.destroy();

        // NOTE(hugo): copy to texture and destroy target
        texture = get_engine().render_layer.copy_render_target_to_texture(render_target);
        get_engine().render_layer.generate_texture_mipmap(texture);
        get_engine().render_layer.free_render_target(render_target);
    }

    void update(Editor*){
        if(ImGui::Begin("##Module")){
            ImVec2 generate_button_size = {ImGui::GetWindowContentRegionWidth(), 0.f};
            if(ImGui::Button("Generate", generate_button_size)) generate();
        }
        ImGui::End();
    }

    void render(Editor*){
        if(texture != Render_Layer_Invalid_Texture){
            drawer.new_frame();

            float image_aspect_ratio = (float)texture.width / (float)texture.height;
            float image_height = 20.f;
            drawer.command_image(texture, {0.f, 0.f}, {image_aspect_ratio * image_height, image_height}, 0.5f);

            drawer.draw();
        }
    }

    // ----

    ImDrawer drawer;
    Texture texture;
};

// ----

int main(int argc, char* argv[]){
    Engine_Config configuration;
    configuration.window_name = "Engine";
    configuration.window_width = 1000u;
    configuration.window_height = 1000u;
    configuration.render_target_samples = 1u;
    configuration.asset_catalog_path = "./data/asset_catalog.json";

    Engine engine;
    set_engine_ptr(&engine);

    printf("-- starting\n");

    engine.create(configuration);
    Scene_Description& desc = engine.scene_manager.push_scene<Editor>("Editor");

    Editor* editor = (Editor*)desc.data;
    EDITOR_REGISTER_MODULE(editor, Random_Packing_v1);
    EDITOR_REGISTER_MODULE(editor, Random_Packing_v2);

    printf("-- mainloop\n");

    engine.run();

    printf("-- engine destroy \n");

    engine.destroy();

    printf("-- finished\n");

    DEV_Memtracker_Leakcheck();

    return 0;
}
