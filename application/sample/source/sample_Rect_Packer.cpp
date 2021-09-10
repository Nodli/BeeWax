#include "imdrawer.h"
#include "imdrawer.cpp"

#include "rect_packer.h"
#include "rect_packer.cpp"

constexpr u32 initial_extents = 128u;
constexpr u32 benchmark_extents = 2048u;

struct Rect_Packer_Scene{
    Rect_Packer_Scene(){
        drawer.create();

        toggle = false;
        checkbox_active = false;

        packer.create();
        packer.set_packing_area(initial_extents, initial_extents);

        stbrp_memory = bw_malloc(sizeof(stbrp_node) * initial_extents);
        stbrp_init_target(&rp, initial_extents, initial_extents, (stbrp_node*)stbrp_memory, initial_extents);
    }
    ~Rect_Packer_Scene(){
        drawer.destroy();
        packer.destroy();
        bw_free(stbrp_memory);
    }

    void update(){
        if(ImGui::Begin("")){
            if(ImGui::Button("Extend")){
                packer.set_packing_area(packer.area_width * 2u, packer.area_height * 2u);
            }

            auto pack_random_rectangle = [&](){
                u32 width = 10u + random_u32_range_uniform(15u);
                u32 height = 10u + random_u32_range_uniform(15u);

                uivec2 origin = packer.insert_rect(width, height);
                if(origin.x != UINT32_MAX) rect_Rect_Packer.push_back({origin, origin + uivec2({width, height})});

                stbrp_rect rect;
                rect.w = (s32)width;
                rect.h = (s32)height;
                stbrp_pack_rects(&rp, &rect, 1u);
                if(rect.was_packed) rect_stbrp.push_back({uivec2({(u32)rect.x, (u32)rect.y}), uivec2({(u32)rect.x + (u32)rect.w, (u32)rect.y + (u32)rect.h})});
            };

            if(ImGui::Button("Pack 1")){
                pack_random_rectangle();
            }
            if(ImGui::Checkbox("Pack Multiple", &checkbox_active) || checkbox_active){
                pack_random_rectangle();
            }

            if(ImGui::Button(toggle ? "Display Rect_Packer" : "Display stbrp")){
                toggle = !toggle;
            }

            if(ImGui::Button("Run Benchmark")){
                LOG_TRACE("-- Generating random rectangles");
                u32* rand = (u32*)bw_malloc(benchmark_extents * benchmark_extents * 2u * sizeof(u32));
                for(u32 ir = 0u; ir != benchmark_extents * benchmark_extents * 2u; ++ir){
                    rand[ir] = 1u + random_u32_range_uniform(10u);
                }

                LOG_TRACE("-- Benchmarking stbrp");
                u32 score_stbrp;
                {
                    stbrp_context cont;
                    void* ptr = bw_malloc(sizeof(stbrp_node) * benchmark_extents);
                    stbrp_init_target(&cont, benchmark_extents, benchmark_extents, (stbrp_node*)ptr, benchmark_extents);

                    u32 irect = 0u;
                    stbrp_rect out;
                    out.was_packed = true;

                    u32 timestamp = timer_ticks();

                    while(irect != (benchmark_extents * benchmark_extents) && out.was_packed){
                        out.w = rand[irect * 2u];
                        out.h = rand[irect * 2u + 1u];
                        stbrp_pack_rects(&cont, &out, 1u);
                        ++irect;
                    }

                    score_stbrp = timer_ticks() - timestamp;
                    LOG_TRACE("SCORE stbrp (ticks): %u", score_stbrp);
                    LOG_TRACE("Number of packed rectangles: %u", irect);

                    bw_free(ptr);
                }

                LOG_TRACE("-- Benchmarking Rect_Packer");
                u32 score_Rect_Packer;
                {
                    Rect_Packer p;
                    p.create();
                    p.set_packing_area(benchmark_extents, benchmark_extents);

                    u32 irect = 0u;
                    uivec2 out;

                    u32 timestamp = timer_ticks();

                    while(irect != (benchmark_extents * benchmark_extents) && out.x != UINT32_MAX){
                        out = p.insert_rect(rand[irect * 2u], rand[irect * 2u + 1u]);
                        ++irect;
                    }

                    score_Rect_Packer = timer_ticks() - timestamp;
                    LOG_TRACE("SCORE Rect_Packer (ticks): %u", score_Rect_Packer);
                    LOG_TRACE("Number of packed rectangles: %u", irect);

                    p.destroy();
                }

                LOG_TRACE("Performance ratio (Rect_Packer / stbrp): %f", (float)score_Rect_Packer / (float)score_stbrp);

                bw_free(rand);
            }
        }
        ImGui::End();
    }
    void render(){
        Camera_2D sc;
        sc.center = {packer.area_width * 0.5f, packer.area_height * 0.5f};
        sc.height = packer.area_height;
        sc.aspect_ratio = 1.f;

        mat4 camera_matrix = mat3D_from_mat2D(sc.projection_matrix() * sc.view_matrix());

        uniform_camera u_camera;
        u_camera.matrix = to_std140(camera_matrix);

        get_engine().render_layer.update_uniform(camera, (void*)&u_camera);

        vec4 color = {360.f, 1.f, 1.f, 1.f};

        drawer.new_frame();
        if(!toggle){
            for(auto& rect : rect_Rect_Packer){
                u32 rect_color = rgba32(hsva_to_rgba(color));

                constexpr float capsule_width = 0.25f;
                vec2 A = {(float)rect.min.x + capsule_width, (float)rect.min.y + capsule_width};
                vec2 B = {(float)rect.max.x - capsule_width, (float)rect.min.y + capsule_width};
                vec2 C = {(float)rect.max.x - capsule_width, (float)rect.max.y - capsule_width};
                vec2 D = {(float)rect.min.x + capsule_width, (float)rect.max.y - capsule_width};
                drawer.command_capsule(A, B, capsule_width, 0.5f, rgba32({1.f, 0.f, 0.f, 1.f}), 0.1f);
                drawer.command_capsule(B, C, capsule_width, 0.5f, rgba32({1.f, 0.f, 0.f, 1.f}), 0.1f);
                drawer.command_capsule(C, D, capsule_width, 0.5f, rgba32({1.f, 0.f, 0.f, 1.f}), 0.1f);
                drawer.command_capsule(D, A, capsule_width, 0.5f, rgba32({1.f, 0.f, 0.f, 1.f}), 0.1f);

                color = fibonacci_hue(color);
            }
        }else{
            for(auto& rect : rect_stbrp){
                u32 rect_color = rgba32(hsva_to_rgba(color));

                constexpr float capsule_width = 0.25f;
                vec2 A = {(float)rect.min.x + capsule_width, (float)rect.min.y + capsule_width};
                vec2 B = {(float)rect.max.x - capsule_width, (float)rect.min.y + capsule_width};
                vec2 C = {(float)rect.max.x - capsule_width, (float)rect.max.y - capsule_width};
                vec2 D = {(float)rect.min.x + capsule_width, (float)rect.max.y - capsule_width};
                drawer.command_capsule(A, B, capsule_width, 0.5f, rgba32({1.f, 0.f, 1.f, 1.f}), 0.1f);
                drawer.command_capsule(B, C, capsule_width, 0.5f, rgba32({1.f, 0.f, 1.f, 1.f}), 0.1f);
                drawer.command_capsule(C, D, capsule_width, 0.5f, rgba32({1.f, 0.f, 1.f, 1.f}), 0.1f);
                drawer.command_capsule(D, A, capsule_width, 0.5f, rgba32({1.f, 0.f, 1.f, 1.f}), 0.1f);

                color = fibonacci_hue(color);
            }
        }
        drawer.draw();
    }

    // ----

    bool checkbox_active;
    bool toggle;

    struct Rect{
        uivec2 min;
        uivec2 max;
    };
    ImDrawer drawer;

    Rect_Packer packer;
    std::vector<Rect> rect_Rect_Packer;

    void* stbrp_memory;
    stbrp_context rp;
    std::vector<Rect> rect_stbrp;
};


