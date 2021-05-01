#ifndef H_WINDOW_SDL_VK
#define H_WINDOW_SDL_VK

struct Window_Settings_SDL_VK{
    enum : s32 {
        mode_windowed = 0,
        mode_borderless = 1,
        mode_fullscreen = 2,

        synchronize_none = 0,
        synchronize_vertical = 1,
        synchronize_adaptive = -1,

        buffering_single = 0,
        buffering_double = 1
    };

    s32 width = 0;
    s32 height = 0;

    const char* name = nullptr;
    s32 mode = mode_windowed;
    s32 synchronization = synchronize_adaptive;
    s32 buffering = buffering_double;
};

void push_instance_extensions();

struct Window_SDL_VK{
    void initialize(const Window_Settings_SDL_VK& required_settings);
    void terminate();

    void swap_buffers();
    float aspect_ratio();

    // NOTE(hugo):
    // pixel coordinates : origin at the bottom left
    // screen coordinates : OpenGL convention
    vec2 pixel_to_screen_coordinates(ivec2 pixel);
    ivec2 screen_to_pixel_coordinates(vec2 screen);

    // ---- data

    Window_Settings_SDL_VK settings;

    s32 width = 0u;
    s32 height = 0u;
    SDL_Window* window = nullptr;

};

// REF:
// https://vulkan-tutorial.com/Introduction
// https://github.com/kennyalive/Quake-III-Arena-Kenny-Edition/blob/master/src/engine/renderer/vk.cpp
// https://github.com/kondrak/vkQuake2/blob/master/ref_vk/qvk.h

struct Buffer_VK{
    VkBuffer buffer;
    VkDeviceMemory memory;
    void* data;
    size_t size;
};

struct Image_VK{
    VkImage image;
    VkImageView image_view;
    VkDeviceMemory memory;
};

struct Swapchain_VK{
    VkSurfaceKHR surface;
    VkSurfaceFormatKHR format;
    VkPresentModeKHR present_mode;
    VkExtent2D extent;

    VkSwapchainKHR swapchain;

    u32 image_index;
    array<VkImage> image;
    array<VkImageView> image_view;
    array<VkFramebuffer> framebuffer;
};

constexpr u32 VULKAN_MAX_FRAMES_PROCESSING = 2u;

struct Command_Buffer_VK{
    VkSemaphore image_acquire;
    VkSemaphore image_release;
    VkFence processing;
};

struct Graphics_Pipeline_Settings{
    const char* vertex_spirv;
    size_t vertex_size;
    const char* fragment_spirv;
    size_t fragment_size;

    // NOTE(hugo): binding & attribute description
    // NOTE(hugo): primitive topology
    // NOTE(hugo): rasterizer
    // NOTE(hugo): multisampling
    // NOTE(hugo): blending
    // NOTE(hugo): layout
};

struct Pipeline_VK{
    VkPipelineLayout layout;
    VkPipeline pipeline;
};

struct Context_VK{
    void initialize();
    void terminate();

    void begin_frame();
    void end_frame();

    Buffer_VK create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_flags);
    void transfer_buffer(const Buffer_VK& buffer);
    void destroy_buffer(const Buffer_VK& buffer);

    Image_VK create_image();
    void destroy_image(const Image_VK& image);

    Pipeline_VK create_graphics_pipeline(const Graphics_Pipeline_Settings& settings);
    void destroy_pipeline(const Pipeline_VK& pipeline);

    // ---- data

    Window_SDL_VK* window;

    VkInstance instance;
#if defined DEBUG_BUILD
    VkDebugUtilsMessengerEXT debug_messenger;
#endif

    VkPhysicalDevice physical_device;
    VkPhysicalDeviceProperties physical_device_properties;
    VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
    VkPhysicalDeviceFeatures physical_device_features;

    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;

    u32 swapchain_image_index;
    Swapchain_VK swapchain;

    VkCommandPool command_pool;
};

#endif
