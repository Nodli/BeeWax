
// ---- VK_CHECK

#if defined(RENDERER_VULKAN)
    #define VK_CHECK(EXPRESSION)                                                                        \
	do                                                                                                  \
	{                                                                                                   \
		VkResult result = (EXPRESSION);                                                                 \
		if (result != VK_SUCCESS){                                                                      \
			CRASH("**VULKAN ERROR**: %s\nEXPRESSION: %s", VK::result_as_string(result), #EXPRESSION);   \
		}                                                                                               \
	} while(false)
#endif



#ifndef H_VK
#define H_VK

// REF(hugo):
// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkResult.html
// https://www.lunarg.com/wp-content/uploads/2018/05/Vulkan-Debug-Utils_05_18_v1.pdf

static PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
static PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = nullptr;
static PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;

struct Render_Pass_Settings_VK{
};

namespace VK{
    // ---- INITIALIZATION

    void INITIALIZE_vkCreateDebugUtilsMessengerEXT(VkInstance instance);
    void INITIALIZE_vkDestroyDebugUtilsMessengerEXT(VkInstance instance);
    void INITIALIZE_vkSetDebugUtilsObjectNameEXT(VkInstance instance);

    // ----

    void require_instance_version(u32 instance_version);

    s32 search_missing_extension(const array<const char*>& required, const array<VkExtensionProperties>& detected);
    void require_extensions(const array<const char*>& required, const array<VkExtensionProperties>& detected);

    array<VkExtensionProperties> detect_instance_extensions();
    void require_instance_extensions(const array<const char*>& required);

    array<VkLayerProperties> detect_instance_layers();
    void require_instance_layers(const char** required, u32 nrequired);

    array<VkPhysicalDevice> detect_physical_devices(const VkInstance instance);
    void detect_device_extensions(VkPhysicalDevice device, array<VkExtensionProperties>& extensions);
    void detect_device_queue_families(VkPhysicalDevice device, array<VkQueueFamilyProperties>& queue_families);
    void select_physical_device(VkInstance instance, array<const char*>& required_extensions, VkSurfaceKHR surface);

    array<VkSurfaceFormatKHR> detect_physical_device_surface_formats(VkPhysicalDevice device, VkSurfaceKHR surface);
    VkSurfaceFormatKHR select_physical_device_swapchain_format(const array<VkSurfaceFormatKHR>& formats);

    array<VkPresentModeKHR> detect_physical_device_surface_present_modes(VkPhysicalDevice device, VkSurfaceKHR surface);
    VkPresentModeKHR select_physical_device_present_mode(const array<VkPresentModeKHR>& present_modes);

    VkExtent2D determine_swapchain_extents(const VkSurfaceCapabilitiesKHR& capabilities, u32 window_width, u32 window_height);
    VkCompositeAlphaFlagBitsKHR determine_composite_mode(const VkSurfaceCapabilitiesKHR& capabilities);

    VkShaderModule create_shader_module(VkDevice device, const char* shader_code, size_t bytesize);

    u32 select_memory_type_index(const VkPhysicalDeviceMemoryProperties& physical_device_mem_properties, u32 mem_type_bits, VkMemoryPropertyFlags memory_flags);

    // ---- DEBUG

    const char* result_as_string(VkResult result);
    const char* debug_severity_as_string(VkDebugUtilsMessageSeverityFlagBitsEXT severity);
    const char* debug_type_as_string(VkDebugUtilsMessageTypeFlagsEXT type);
    const char* object_type_as_string(VkObjectType type);
    const char* format_as_string(VkSurfaceFormatKHR format);
    const char* present_mode_as_string(VkPresentModeKHR present_mode);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* message_data, void* user_data);
    VkDebugUtilsMessengerCreateInfoEXT default_debug_messenger_create_info();
    void register_debug_callback(VkInstance instance, VkDebugUtilsMessengerEXT* debug_messenger);
}
#endif
