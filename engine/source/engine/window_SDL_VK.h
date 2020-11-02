#ifndef H_WINDOW_SDL_VK
#define H_WINDOW_SDL_VK

// REF:
// https://github.com/naivisoftware/vulkansdldemo/blob/master/src/main.cpp
// https://vulkan-tutorial.com/en/Drawing_a_triangle/Setup/Validation_layers

struct Window_SDL_VK{
    void initialize(const Window_Settings& settings);
    void terminate();
    void swap_buffers();
    float aspect_ratio();

    // ---- data

    s32 width;
    s32 height;
    SDL_Window* handle;

    VkInstance instance;
};

#endif
