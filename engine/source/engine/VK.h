#ifndef H_VK
#define H_VK

namespace VK{
    // NOTE(hugo): https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkResult.html
    const char* cstring_vk_result(VkResult result){
        switch(result){
            case VK_SUCCESS:
                return "VK_SUCCESS";
            case VK_NOT_READY:
                return "VK_NOT_READY";
            case VK_ERROR_EXTENSION_NOT_PRESENT:
                return "VK_ERROR_EXTENSION_NOT_PRESENT";
            default:
                return "UNKNOWN VkResult";
        }
    }

    static VKAPIATTR VkBool32 VKAPI_CALL debug_message_callback(
            VkDebugUtilsMessageSeverityFlagBitsEXT severity,
            VkDebugUtilsMessageTypeFlagsEXT type,
            const VkDebugUtilsMessengerCallbackDataEXT* message_data,
            void* user_param){
        LOG_TRACE("VK DEBUG MESSAGE: %s", message_data->pMessage);
        return VK_FALSE;
    }
};

#endif
