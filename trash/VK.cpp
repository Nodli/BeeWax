namespace VK{
    // ---- INITIALIZATION

    void INITIALIZE_vkCreateDebugUtilsMessengerEXT(VkInstance instance){
        vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        ENGINE_CHECK(vkCreateDebugUtilsMessengerEXT, "INITIALIZE_vkCreateDebugUtilsMessengerEXT");
    }

    void INITIALIZE_vkDestroyDebugUtilsMessengerEXT(VkInstance instance){
        vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        ENGINE_CHECK(vkDestroyDebugUtilsMessengerEXT, "INITIALIZE_vkDestroyDebugUtilsMessengerEXT");
    }


    void INITIALIZE_vkSetDebugUtilsObjectNameEXT(VkInstance instance){
        vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
        ENGINE_CHECK(vkSetDebugUtilsObjectNameEXT, "INITIALIZE_vkSetdebugUtilsObjectNameEXT");
    }

    // ----

    void require_instance_version(u32 instance_version){
        u32 detected_version;
        VK_CHECK(vkEnumerateInstanceVersion(&detected_version));

        ENGINE_CHECK((VK_VERSION_MAJOR(detected_version) > VK_VERSION_MAJOR(instance_version))
                || ((VK_VERSION_MAJOR(detected_version) == VK_VERSION_MAJOR(instance_version)) && (VK_VERSION_MINOR(detected_version) >= VK_VERSION_MINOR(instance_version))),
                "**VULKAN: OUTDATED VERSION** %u %u", VK_VERSION_MAJOR(detected_version), VK_VERSION_MINOR(detected_version));
    }

    s32 search_missing_extension(const array<const char*>& required, const array<VkExtensionProperties>& detected){
        for(u32 ireq = 0u; ireq != required.size; ++ireq){
            bool available = false;
            for(u32 iav = 0u; iav != detected.size; ++iav){
                if(strcmp(required[ireq], detected[iav].extensionName) == 0){
                    available = true;
                }
            }

            if(!available){
                return (s32)ireq;
            }
        }
        return -1;
    }

    void require_extensions(const array<const char*>& required, const array<VkExtensionProperties>& detected){
        s32 return_code = search_missing_extension(required, detected);
        ENGINE_CHECK(return_code == -1, "**VULKAN: MISSING EXTENSION**: %s", required[return_code]);
    }

    array<VkExtensionProperties> detect_instance_extensions(){
        u32 ndetected;
        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &ndetected, nullptr));

        array<VkExtensionProperties> detected;
        detected.set_size(ndetected);
        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &ndetected, detected.storage.data));

        return detected;
    }

    void require_instance_extensions(const array<const char*>& required){
        array<VkExtensionProperties> detected = detect_instance_extensions();
        require_extensions(required, detected);
        detected.free();
    }

    array<VkLayerProperties> detect_instance_layers(){
        u32 ndetected;
        VK_CHECK(vkEnumerateInstanceLayerProperties(&ndetected, nullptr));

        array<VkLayerProperties> detected;
        detected.set_size(ndetected);
        VK_CHECK(vkEnumerateInstanceLayerProperties(&ndetected, detected.storage.data));

        return detected;
    }

    void require_instance_layers(const char** required, u32 nrequired){
        array<VkLayerProperties> detected = detect_instance_layers();

        for(u32 ireq = 0u; ireq != nrequired; ++ireq){
            bool available = false;
            for(u32 iav = 0u; iav != detected.size; ++iav){
                if(strcmp(required[ireq], detected[iav].layerName) == 0){
                    available = true;
                }
            }

            ENGINE_CHECK(available, "**VULKAN: INSTANCE LAYER MISSING**: %s", required[ireq]);
        }

        detected.free();
    }

    array<VkPhysicalDevice> detect_physical_devices(VkInstance instance){
        u32 ndevice;
        VK_CHECK(vkEnumeratePhysicalDevices(instance, &ndevice, nullptr));

        array<VkPhysicalDevice> phys_devices;
        phys_devices.set_size(ndevice);
        VK_CHECK(vkEnumeratePhysicalDevices(instance, &ndevice, phys_devices.storage.data));

        return phys_devices;
    }

    void detect_device_extensions(VkPhysicalDevice device, array<VkExtensionProperties>& extensions){
        u32 ndetected;
        VK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &ndetected, nullptr));

        extensions.set_size(ndetected);
        VK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &ndetected, extensions.storage.data));
    }

    void detect_device_queue_families(VkPhysicalDevice device, array<VkQueueFamilyProperties>& queue_families){
        u32 ndetected;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &ndetected, nullptr);

        queue_families.set_size(ndetected);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &ndetected, queue_families.storage.data);
    }

    void select_physical_device(VkInstance instance, const array<const char*>& required_extensions, VkSurfaceKHR surface,
            VkPhysicalDevice& physical_device,
            VkPhysicalDeviceProperties& physical_device_properties,
            VkPhysicalDeviceMemoryProperties& physical_device_memory_properties,
            VkPhysicalDeviceFeatures& physical_device_features,
            u32& graphics_family_index, u32& present_family_index){

        array<VkPhysicalDevice> physical_devices = detect_physical_devices(instance);
        ENGINE_CHECK(physical_devices.size != 0u, "**VULKAN: NO CAPABLE DEVICE FOUND**");

        constexpr u8 GRAPHICS_QUEUE_FAMILY_BIT_INDEX = 1u;
        constexpr u8 PRESENT_QUEUE_FAMILY_BIT_INDEX = 2u;

        physical_device = VK_NULL_HANDLE;

        VkPhysicalDeviceProperties temp_properties;
        VkPhysicalDeviceMemoryProperties temp_memory_properties;
        VkPhysicalDeviceFeatures temp_features;
        array<VkExtensionProperties> extensions;
        array<VkQueueFamilyProperties> queue_families;

        u32 temp_graphics_family_index;
        u32 temp_present_family_index;

        for(auto& device : physical_devices){
            vkGetPhysicalDeviceProperties(device, &temp_properties);
            vkGetPhysicalDeviceMemoryProperties(device, &temp_memory_properties);
            vkGetPhysicalDeviceFeatures(device, &temp_features);

            detect_device_extensions(device, extensions);
            if(search_missing_extension(required_extensions, extensions) != -1) continue;

            detect_device_queue_families(device, queue_families);
            if(queue_families.size == 0u) continue;

            temp_graphics_family_index = UINT32_MAX;
            temp_present_family_index = UINT32_MAX;

            for(u32 ifam = 0u; ifam != queue_families.size; ++ifam){
                VkBool32 present_support;
                VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, ifam, surface, &present_support));
                VkBool32 graphics_support = queue_families[ifam].queueFlags & VK_QUEUE_GRAPHICS_BIT;

                if(graphics_support && (temp_graphics_family_index == UINT32_MAX || temp_graphics_family_index != temp_present_family_index)){
                    temp_graphics_family_index = ifam;
                }
                if(present_support && (temp_present_family_index == UINT32_MAX || temp_graphics_family_index != temp_present_family_index)){
                    temp_present_family_index = ifam;
                }
            }

            // NOTE(hugo): accept the device if it has the required queue families and favor discrete GPUs
            if(temp_graphics_family_index != UINT32_MAX && temp_present_family_index != UINT32_MAX
                    && (physical_device == VK_NULL_HANDLE ||
                        (physical_device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
                         && temp_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU))){

                physical_device = device;
                physical_device_properties = temp_properties;
                physical_device_memory_properties = temp_memory_properties;
                physical_device_features = temp_features;
                graphics_family_index = temp_graphics_family_index;
                present_family_index = temp_present_family_index;
            }
        }

        ENGINE_CHECK(physical_device != VK_NULL_HANDLE, "**VULKAN: NONE OF THE CAPABLE DEVICES FOUND WAS SUITABLE**");

        extensions.free();
        queue_families.free();
    }

    // NOTE(hugo): at least one format and one present mode (VK_PRESENT_MODE_FIFO_KHR is required by the spec.)
    // are guaranteed because vkGetPhysicalDeviceSurfaceSupportKHR was checked when selecting the physical device

    // REF(hugo):
    // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkGetPhysicalDeviceSurfaceFormatsKHR.html
    // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPresentModeKHR.html

    array<VkSurfaceFormatKHR> detect_physical_device_surface_formats(VkPhysicalDevice device, VkSurfaceKHR surface){
        u32 nformats;
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &nformats, nullptr));

        array<VkSurfaceFormatKHR> formats;
        formats.set_size(nformats);
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &nformats, formats.storage.data));

        return formats;
    }

    VkSurfaceFormatKHR select_physical_device_swapchain_format(const array<VkSurfaceFormatKHR>& formats){
        for(auto& surface_format : formats){
            if(surface_format.format == VK_FORMAT_B8G8R8A8_SRGB && surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return surface_format;
        }

        assert(formats.size > 0u);
        return formats[0u];
    }

    array<VkPresentModeKHR> detect_physical_device_surface_present_modes(VkPhysicalDevice device, VkSurfaceKHR surface){
        u32 nmodes;
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &nmodes, nullptr));

        array<VkPresentModeKHR> present_modes;
        present_modes.set_size(nmodes);
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &nmodes, present_modes.storage.data));

        return present_modes;
    }

    VkPresentModeKHR select_physical_device_present_mode(const array<VkPresentModeKHR>& present_modes){
        for(auto& mode : present_modes){
            if(mode == VK_PRESENT_MODE_MAILBOX_KHR) return mode;
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D determine_swapchain_extents(const VkSurfaceCapabilitiesKHR& capabilities, u32 window_width, u32 window_height){
        if(capabilities.currentExtent.width != 0xFFFFFFFF || capabilities.currentExtent.height != 0xFFFFFFFF) return capabilities.currentExtent;

        VkExtent2D extent;
        extent.width = min_max(window_width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = min_max(window_height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return extent;
    }

    VkCompositeAlphaFlagBitsKHR determine_composite_mode(const VkSurfaceCapabilitiesKHR& capabilities){
        ENGINE_CHECK(capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, "**VULKAN: UNSUPPORTED COMPOSITE MODE**");
        return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }

    VkShaderModule create_shader_module(VkDevice device, const char* shader_code, size_t bytesize){
        VkShaderModuleCreateInfo create_info_shader_module = {};
        create_info_shader_module.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info_shader_module.codeSize = bytesize;
        create_info_shader_module.pCode = (const u32*)shader_code;

        VkShaderModule shader_module;
        VK_CHECK(vkCreateShaderModule(device, &create_info_shader_module, nullptr, &shader_module));

        return shader_module;
    }

    u32 select_memory_type_index(const VkPhysicalDeviceMemoryProperties& physical_device_mem_properties, u32 mem_type_bits, VkMemoryPropertyFlags memory_flags){
        for(u32 itype = 0u; itype != physical_device_mem_properties.memoryTypeCount; ++itype){
            if((mem_type_bits & (1u << itype)) && (physical_device_mem_properties.memoryTypes[itype].propertyFlags & memory_flags) == memory_flags){
                return (s32)itype;
            }
        }

        return UINT32_MAX;
    }

    // ----- DEBUG

    const char* result_as_string(VkResult result){

#define VULKAN_RESULT_AS_STRING_CASE(RESULT)    \
        case VK_## RESULT:                      \
            return #RESULT;

        switch(result){
            VULKAN_RESULT_AS_STRING_CASE(SUCCESS)
            VULKAN_RESULT_AS_STRING_CASE(NOT_READY)
            VULKAN_RESULT_AS_STRING_CASE(TIMEOUT)
            VULKAN_RESULT_AS_STRING_CASE(EVENT_SET)
            VULKAN_RESULT_AS_STRING_CASE(EVENT_RESET)
            VULKAN_RESULT_AS_STRING_CASE(INCOMPLETE)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_OUT_OF_HOST_MEMORY)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_OUT_OF_DEVICE_MEMORY)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_INITIALIZATION_FAILED)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_DEVICE_LOST)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_MEMORY_MAP_FAILED)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_LAYER_NOT_PRESENT)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_EXTENSION_NOT_PRESENT)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_FEATURE_NOT_PRESENT)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_INCOMPATIBLE_DRIVER)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_TOO_MANY_OBJECTS)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_FORMAT_NOT_SUPPORTED)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_FRAGMENTED_POOL)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_UNKNOWN)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_OUT_OF_POOL_MEMORY)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_INVALID_EXTERNAL_HANDLE)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_FRAGMENTATION)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_SURFACE_LOST_KHR)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_NATIVE_WINDOW_IN_USE_KHR)
            VULKAN_RESULT_AS_STRING_CASE(SUBOPTIMAL_KHR)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_OUT_OF_DATE_KHR)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_INCOMPATIBLE_DISPLAY_KHR)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_VALIDATION_FAILED_EXT)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_INVALID_SHADER_NV)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_NOT_PERMITTED_EXT)
            VULKAN_RESULT_AS_STRING_CASE(ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
            VULKAN_RESULT_AS_STRING_CASE(THREAD_IDLE_KHR)
            VULKAN_RESULT_AS_STRING_CASE(THREAD_DONE_KHR)
            VULKAN_RESULT_AS_STRING_CASE(OPERATION_DEFERRED_KHR)
            VULKAN_RESULT_AS_STRING_CASE(OPERATION_NOT_DEFERRED_KHR)
            VULKAN_RESULT_AS_STRING_CASE(PIPELINE_COMPILE_REQUIRED_EXT)
            //VULKAN_RESULT_AS_STRING_CASE(ERROR_OUT_OF_POOL_MEMORY_KHR) = VK_ERROR_OUT_OF_POOL_MEMORY
            //VULKAN_RESULT_AS_STRING_CASE(ERROR_INVALID_EXTERNAL_HANDLE_KHR) = VK_ERROR_INVALID_EXTERNAL_HANDLE
            //VULKAN_RESULT_AS_STRING_CASE(ERROR_FRAGMENTATION_EXT) = VK_ERROR_FRAGMENTATION
            //VULKAN_RESULT_AS_STRING_CASE(ERROR_INVALID_DEVICE_ADDRESS_EXT) = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS
            //VULKAN_RESULT_AS_STRING_CASE(ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR) = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS
            //VULKAN_RESULT_AS_STRING_CASE(ERROR_PIPELINE_COMPILE_REQUIRED_EXT) = VK_PIPELINE_COMPILE_REQUIRED_EXT

            default:
                assert(false);
                return "UNKNOWN RESULT";
        }

#undef VULKAN_RESULT_AS_STRING_CASE
    }

    const char* debug_severity_as_string(VkDebugUtilsMessageSeverityFlagBitsEXT severity){
        switch(severity){
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                return "VERBOSE DIAGNOSTIC";

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                return "INFORMATION";

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                return "** WARNING **";

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                return "**** ERROR ****";

            default:
                assert(false);
                return "UNKNOWN SEVERITY";
        }
    }

    const char* debug_type_as_string(VkDebugUtilsMessageTypeFlagsEXT type){
        switch(type){
            case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
                return "GENERAL";

            case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
                return "VALIDATION";

            case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
                return "PERFORMANCE";

            default:
                assert(false);
                return "UNKNOWN TYPE";
        }
    }

    const char* object_type_as_string(VkObjectType type){

#define VULKAN_OBJECT_TYPE_AS_STRING_CASE(TYPE)     \
        case VK_OBJECT_TYPE_ ## TYPE :              \
            return #TYPE;

        switch(type){
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(UNKNOWN)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(INSTANCE)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(PHYSICAL_DEVICE)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(DEVICE)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(QUEUE)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(SEMAPHORE)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(COMMAND_BUFFER)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(FENCE)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(DEVICE_MEMORY)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(BUFFER)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(IMAGE)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(EVENT)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(QUERY_POOL)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(BUFFER_VIEW)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(IMAGE_VIEW)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(SHADER_MODULE)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(PIPELINE_CACHE)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(PIPELINE_LAYOUT)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(RENDER_PASS)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(PIPELINE)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(DESCRIPTOR_SET_LAYOUT)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(SAMPLER)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(DESCRIPTOR_POOL)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(DESCRIPTOR_SET)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(FRAMEBUFFER)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(COMMAND_POOL)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(SAMPLER_YCBCR_CONVERSION)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(DESCRIPTOR_UPDATE_TEMPLATE)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(SURFACE_KHR)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(DISPLAY_KHR)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(DISPLAY_MODE_KHR)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(DEBUG_REPORT_CALLBACK_EXT)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(DEBUG_UTILS_MESSENGER_EXT)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(ACCELERATION_STRUCTURE_KHR)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(VALIDATION_CACHE_EXT)
            //VULKAN_OBJECT_TYPE_AS_STRING_CASE(ACCELERATION_STRUCTURE_NV) = ACCELERATION_STRUCTURE_KHR
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(PERFORMANCE_CONFIGURATION_INTEL)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(DEFERRED_OPERATION_KHR)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(INDIRECT_COMMANDS_LAYOUT_NV)
            VULKAN_OBJECT_TYPE_AS_STRING_CASE(PRIVATE_DATA_SLOT_EXT)
            //VULKAN_OBJECT_TYPE_AS_STRING_CASE(DESCRIPTOR_UPDATE_TEMPLATE_KHR) = DESCRIPTOR_UPDATE_TEMPLATE
            //VULKAN_OBJECT_TYPE_AS_STRING_CASE(SAMPLER_YCBCR_CONVERSION_KHR) = VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION

            default:
                assert(false);
                return "UNKNOWN TYPE";
        }

#undef VULKAN_OBJECT_TYPE_AS_STRING_CASE
    }

    const char* format_as_string(VkFormat format){

#define VULKAN_FORMAT_AS_STRING_CASE(FORMAT)    \
    case VK_FORMAT_ ## FORMAT:                  \
        return #FORMAT;

    switch(format){
            VULKAN_FORMAT_AS_STRING_CASE(UNDEFINED)
            VULKAN_FORMAT_AS_STRING_CASE(R4G4_UNORM_PACK8)
            VULKAN_FORMAT_AS_STRING_CASE(R4G4B4A4_UNORM_PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(B4G4R4A4_UNORM_PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(R5G6B5_UNORM_PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(B5G6R5_UNORM_PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(R5G5B5A1_UNORM_PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(B5G5R5A1_UNORM_PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(A1R5G5B5_UNORM_PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(R8_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(R8_SNORM)
            VULKAN_FORMAT_AS_STRING_CASE(R8_USCALED)
            VULKAN_FORMAT_AS_STRING_CASE(R8_SSCALED)
            VULKAN_FORMAT_AS_STRING_CASE(R8_UINT)
            VULKAN_FORMAT_AS_STRING_CASE(R8_SINT)
            VULKAN_FORMAT_AS_STRING_CASE(R8_SRGB)
            VULKAN_FORMAT_AS_STRING_CASE(R8G8_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(R8G8_SNORM)
            VULKAN_FORMAT_AS_STRING_CASE(R8G8_USCALED)
            VULKAN_FORMAT_AS_STRING_CASE(R8G8_SSCALED)
            VULKAN_FORMAT_AS_STRING_CASE(R8G8_UINT)
            VULKAN_FORMAT_AS_STRING_CASE(R8G8_SINT)
            VULKAN_FORMAT_AS_STRING_CASE(R8G8_SRGB)
            VULKAN_FORMAT_AS_STRING_CASE(R8G8B8_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(R8G8B8_SNORM)
            VULKAN_FORMAT_AS_STRING_CASE(R8G8B8_USCALED)
            VULKAN_FORMAT_AS_STRING_CASE(R8G8B8_SSCALED)
            VULKAN_FORMAT_AS_STRING_CASE(R8G8B8_UINT)
            VULKAN_FORMAT_AS_STRING_CASE(R8G8B8_SINT)
            VULKAN_FORMAT_AS_STRING_CASE(R8G8B8_SRGB)
            VULKAN_FORMAT_AS_STRING_CASE(B8G8R8_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(B8G8R8_SNORM)
            VULKAN_FORMAT_AS_STRING_CASE(B8G8R8_USCALED)
            VULKAN_FORMAT_AS_STRING_CASE(B8G8R8_SSCALED)
            VULKAN_FORMAT_AS_STRING_CASE(B8G8R8_UINT)
            VULKAN_FORMAT_AS_STRING_CASE(B8G8R8_SINT)
            VULKAN_FORMAT_AS_STRING_CASE(B8G8R8_SRGB)
            VULKAN_FORMAT_AS_STRING_CASE(R8G8B8A8_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(R8G8B8A8_SNORM)
            VULKAN_FORMAT_AS_STRING_CASE(R8G8B8A8_USCALED)
            VULKAN_FORMAT_AS_STRING_CASE(R8G8B8A8_SSCALED)
            VULKAN_FORMAT_AS_STRING_CASE(R8G8B8A8_UINT)
            VULKAN_FORMAT_AS_STRING_CASE(R8G8B8A8_SINT)
            VULKAN_FORMAT_AS_STRING_CASE(R8G8B8A8_SRGB)
            VULKAN_FORMAT_AS_STRING_CASE(B8G8R8A8_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(B8G8R8A8_SNORM)
            VULKAN_FORMAT_AS_STRING_CASE(B8G8R8A8_USCALED)
            VULKAN_FORMAT_AS_STRING_CASE(B8G8R8A8_SSCALED)
            VULKAN_FORMAT_AS_STRING_CASE(B8G8R8A8_UINT)
            VULKAN_FORMAT_AS_STRING_CASE(B8G8R8A8_SINT)
            VULKAN_FORMAT_AS_STRING_CASE(B8G8R8A8_SRGB)
            VULKAN_FORMAT_AS_STRING_CASE(A8B8G8R8_UNORM_PACK32)
            VULKAN_FORMAT_AS_STRING_CASE(A8B8G8R8_SNORM_PACK32)
            VULKAN_FORMAT_AS_STRING_CASE(A8B8G8R8_USCALED_PACK32)
            VULKAN_FORMAT_AS_STRING_CASE(A8B8G8R8_SSCALED_PACK32)
            VULKAN_FORMAT_AS_STRING_CASE(A8B8G8R8_UINT_PACK32)
            VULKAN_FORMAT_AS_STRING_CASE(A8B8G8R8_SINT_PACK32)
            VULKAN_FORMAT_AS_STRING_CASE(A8B8G8R8_SRGB_PACK32)
            VULKAN_FORMAT_AS_STRING_CASE(A2R10G10B10_UNORM_PACK32)
            VULKAN_FORMAT_AS_STRING_CASE(A2R10G10B10_SNORM_PACK32)
            VULKAN_FORMAT_AS_STRING_CASE(A2R10G10B10_USCALED_PACK32)
            VULKAN_FORMAT_AS_STRING_CASE(A2R10G10B10_SSCALED_PACK32)
            VULKAN_FORMAT_AS_STRING_CASE(A2R10G10B10_UINT_PACK32)
            VULKAN_FORMAT_AS_STRING_CASE(A2R10G10B10_SINT_PACK32)
            VULKAN_FORMAT_AS_STRING_CASE(A2B10G10R10_UNORM_PACK32)
            VULKAN_FORMAT_AS_STRING_CASE(A2B10G10R10_SNORM_PACK32)
            VULKAN_FORMAT_AS_STRING_CASE(A2B10G10R10_USCALED_PACK32)
            VULKAN_FORMAT_AS_STRING_CASE(A2B10G10R10_SSCALED_PACK32)
            VULKAN_FORMAT_AS_STRING_CASE(A2B10G10R10_UINT_PACK32)
            VULKAN_FORMAT_AS_STRING_CASE(A2B10G10R10_SINT_PACK32)
            VULKAN_FORMAT_AS_STRING_CASE(R16_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(R16_SNORM)
            VULKAN_FORMAT_AS_STRING_CASE(R16_USCALED)
            VULKAN_FORMAT_AS_STRING_CASE(R16_SSCALED)
            VULKAN_FORMAT_AS_STRING_CASE(R16_UINT)
            VULKAN_FORMAT_AS_STRING_CASE(R16_SINT)
            VULKAN_FORMAT_AS_STRING_CASE(R16_SFLOAT)
            VULKAN_FORMAT_AS_STRING_CASE(R16G16_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(R16G16_SNORM)
            VULKAN_FORMAT_AS_STRING_CASE(R16G16_USCALED)
            VULKAN_FORMAT_AS_STRING_CASE(R16G16_SSCALED)
            VULKAN_FORMAT_AS_STRING_CASE(R16G16_UINT)
            VULKAN_FORMAT_AS_STRING_CASE(R16G16_SINT)
            VULKAN_FORMAT_AS_STRING_CASE(R16G16_SFLOAT)
            VULKAN_FORMAT_AS_STRING_CASE(R16G16B16_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(R16G16B16_SNORM)
            VULKAN_FORMAT_AS_STRING_CASE(R16G16B16_USCALED)
            VULKAN_FORMAT_AS_STRING_CASE(R16G16B16_SSCALED)
            VULKAN_FORMAT_AS_STRING_CASE(R16G16B16_UINT)
            VULKAN_FORMAT_AS_STRING_CASE(R16G16B16_SINT)
            VULKAN_FORMAT_AS_STRING_CASE(R16G16B16_SFLOAT)
            VULKAN_FORMAT_AS_STRING_CASE(R16G16B16A16_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(R16G16B16A16_SNORM)
            VULKAN_FORMAT_AS_STRING_CASE(R16G16B16A16_USCALED)
            VULKAN_FORMAT_AS_STRING_CASE(R16G16B16A16_SSCALED)
            VULKAN_FORMAT_AS_STRING_CASE(R16G16B16A16_UINT)
            VULKAN_FORMAT_AS_STRING_CASE(R16G16B16A16_SINT)
            VULKAN_FORMAT_AS_STRING_CASE(R16G16B16A16_SFLOAT)
            VULKAN_FORMAT_AS_STRING_CASE(R32_UINT)
            VULKAN_FORMAT_AS_STRING_CASE(R32_SINT)
            VULKAN_FORMAT_AS_STRING_CASE(R32_SFLOAT)
            VULKAN_FORMAT_AS_STRING_CASE(R32G32_UINT)
            VULKAN_FORMAT_AS_STRING_CASE(R32G32_SINT)
            VULKAN_FORMAT_AS_STRING_CASE(R32G32_SFLOAT)
            VULKAN_FORMAT_AS_STRING_CASE(R32G32B32_UINT)
            VULKAN_FORMAT_AS_STRING_CASE(R32G32B32_SINT)
            VULKAN_FORMAT_AS_STRING_CASE(R32G32B32_SFLOAT)
            VULKAN_FORMAT_AS_STRING_CASE(R32G32B32A32_UINT)
            VULKAN_FORMAT_AS_STRING_CASE(R32G32B32A32_SINT)
            VULKAN_FORMAT_AS_STRING_CASE(R32G32B32A32_SFLOAT)
            VULKAN_FORMAT_AS_STRING_CASE(R64_UINT)
            VULKAN_FORMAT_AS_STRING_CASE(R64_SINT)
            VULKAN_FORMAT_AS_STRING_CASE(R64_SFLOAT)
            VULKAN_FORMAT_AS_STRING_CASE(R64G64_UINT)
            VULKAN_FORMAT_AS_STRING_CASE(R64G64_SINT)
            VULKAN_FORMAT_AS_STRING_CASE(R64G64_SFLOAT)
            VULKAN_FORMAT_AS_STRING_CASE(R64G64B64_UINT)
            VULKAN_FORMAT_AS_STRING_CASE(R64G64B64_SINT)
            VULKAN_FORMAT_AS_STRING_CASE(R64G64B64_SFLOAT)
            VULKAN_FORMAT_AS_STRING_CASE(R64G64B64A64_UINT)
            VULKAN_FORMAT_AS_STRING_CASE(R64G64B64A64_SINT)
            VULKAN_FORMAT_AS_STRING_CASE(R64G64B64A64_SFLOAT)
            VULKAN_FORMAT_AS_STRING_CASE(B10G11R11_UFLOAT_PACK32)
            VULKAN_FORMAT_AS_STRING_CASE(E5B9G9R9_UFLOAT_PACK32)
            VULKAN_FORMAT_AS_STRING_CASE(D16_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(X8_D24_UNORM_PACK32)
            VULKAN_FORMAT_AS_STRING_CASE(D32_SFLOAT)
            VULKAN_FORMAT_AS_STRING_CASE(S8_UINT)
            VULKAN_FORMAT_AS_STRING_CASE(D16_UNORM_S8_UINT)
            VULKAN_FORMAT_AS_STRING_CASE(D24_UNORM_S8_UINT)
            VULKAN_FORMAT_AS_STRING_CASE(D32_SFLOAT_S8_UINT)
            VULKAN_FORMAT_AS_STRING_CASE(BC1_RGB_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(BC1_RGB_SRGB_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(BC1_RGBA_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(BC1_RGBA_SRGB_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(BC2_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(BC2_SRGB_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(BC3_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(BC3_SRGB_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(BC4_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(BC4_SNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(BC5_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(BC5_SNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(BC6H_UFLOAT_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(BC6H_SFLOAT_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(BC7_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(BC7_SRGB_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ETC2_R8G8B8_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ETC2_R8G8B8_SRGB_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ETC2_R8G8B8A1_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ETC2_R8G8B8A1_SRGB_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ETC2_R8G8B8A8_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ETC2_R8G8B8A8_SRGB_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(EAC_R11_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(EAC_R11_SNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(EAC_R11G11_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(EAC_R11G11_SNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_4x4_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_4x4_SRGB_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_5x4_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_5x4_SRGB_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_5x5_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_5x5_SRGB_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_6x5_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_6x5_SRGB_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_6x6_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_6x6_SRGB_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_8x5_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_8x5_SRGB_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_8x6_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_8x6_SRGB_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_8x8_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_8x8_SRGB_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_10x5_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_10x5_SRGB_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_10x6_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_10x6_SRGB_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_10x8_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_10x8_SRGB_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_10x10_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_10x10_SRGB_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_12x10_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_12x10_SRGB_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_12x12_UNORM_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_12x12_SRGB_BLOCK)
            VULKAN_FORMAT_AS_STRING_CASE(G8B8G8R8_422_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(B8G8R8G8_422_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(G8_B8_R8_3PLANE_420_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(G8_B8R8_2PLANE_420_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(G8_B8_R8_3PLANE_422_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(G8_B8R8_2PLANE_422_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(G8_B8_R8_3PLANE_444_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(R10X6_UNORM_PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(R10X6G10X6_UNORM_2PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(R10X6G10X6B10X6A10X6_UNORM_4PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(G10X6B10X6G10X6R10X6_422_UNORM_4PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(B10X6G10X6R10X6G10X6_422_UNORM_4PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(R12X4_UNORM_PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(R12X4G12X4_UNORM_2PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(R12X4G12X4B12X4A12X4_UNORM_4PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(G12X4B12X4G12X4R12X4_422_UNORM_4PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(B12X4G12X4R12X4G12X4_422_UNORM_4PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16)
            VULKAN_FORMAT_AS_STRING_CASE(G16B16G16R16_422_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(B16G16R16G16_422_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(G16_B16_R16_3PLANE_420_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(G16_B16R16_2PLANE_420_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(G16_B16_R16_3PLANE_422_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(G16_B16R16_2PLANE_422_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(G16_B16_R16_3PLANE_444_UNORM)
            VULKAN_FORMAT_AS_STRING_CASE(PVRTC1_2BPP_UNORM_BLOCK_IMG)
            VULKAN_FORMAT_AS_STRING_CASE(PVRTC1_4BPP_UNORM_BLOCK_IMG)
            VULKAN_FORMAT_AS_STRING_CASE(PVRTC2_2BPP_UNORM_BLOCK_IMG)
            VULKAN_FORMAT_AS_STRING_CASE(PVRTC2_4BPP_UNORM_BLOCK_IMG)
            VULKAN_FORMAT_AS_STRING_CASE(PVRTC1_2BPP_SRGB_BLOCK_IMG)
            VULKAN_FORMAT_AS_STRING_CASE(PVRTC1_4BPP_SRGB_BLOCK_IMG)
            VULKAN_FORMAT_AS_STRING_CASE(PVRTC2_2BPP_SRGB_BLOCK_IMG)
            VULKAN_FORMAT_AS_STRING_CASE(PVRTC2_4BPP_SRGB_BLOCK_IMG)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_4x4_SFLOAT_BLOCK_EXT)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_5x4_SFLOAT_BLOCK_EXT)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_5x5_SFLOAT_BLOCK_EXT)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_6x5_SFLOAT_BLOCK_EXT)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_6x6_SFLOAT_BLOCK_EXT)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_8x5_SFLOAT_BLOCK_EXT)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_8x6_SFLOAT_BLOCK_EXT)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_8x8_SFLOAT_BLOCK_EXT)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_10x5_SFLOAT_BLOCK_EXT)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_10x6_SFLOAT_BLOCK_EXT)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_10x8_SFLOAT_BLOCK_EXT)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_10x10_SFLOAT_BLOCK_EXT)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_12x10_SFLOAT_BLOCK_EXT)
            VULKAN_FORMAT_AS_STRING_CASE(ASTC_12x12_SFLOAT_BLOCK_EXT)
            //VULKAN_FORMAT_AS_STRING_CASE(A4R4G4B4_UNORM_PACK16_EXT)
            //VULKAN_FORMAT_AS_STRING_CASE(A4B4G4R4_UNORM_PACK16_EXT)
            // VULKAN_FORMAT_AS_STRING_CASE(G8B8G8R8_422_UNORM_KHR) = G8B8G8R8_422_UNORM
            // VULKAN_FORMAT_AS_STRING_CASE(B8G8R8G8_422_UNORM_KHR) = B8G8R8G8_422_UNORM
            // VULKAN_FORMAT_AS_STRING_CASE(G8_B8_R8_3PLANE_420_UNORM_KHR) = G8_B8_R8_3PLANE_420_UNORM
            // VULKAN_FORMAT_AS_STRING_CASE(G8_B8R8_2PLANE_420_UNORM_KHR) = G8_B8R8_2PLANE_420_UNORM
            // VULKAN_FORMAT_AS_STRING_CASE(G8_B8_R8_3PLANE_422_UNORM_KHR) = G8_B8_R8_3PLANE_422_UNORM
            // VULKAN_FORMAT_AS_STRING_CASE(G8_B8R8_2PLANE_422_UNORM_KHR) = G8_B8R8_2PLANE_422_UNORM
            // VULKAN_FORMAT_AS_STRING_CASE(G8_B8_R8_3PLANE_444_UNORM_KHR) = G8_B8_R8_3PLANE_444_UNORM
            // VULKAN_FORMAT_AS_STRING_CASE(R10X6_UNORM_PACK16_KHR) = R10X6_UNORM_PACK16
            // VULKAN_FORMAT_AS_STRING_CASE(R10X6G10X6_UNORM_2PACK16_KHR) = R10X6G10X6_UNORM_2PACK16
            // VULKAN_FORMAT_AS_STRING_CASE(R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR) = R10X6G10X6B10X6A10X6_UNORM_4PACK16
            // VULKAN_FORMAT_AS_STRING_CASE(G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR) = G10X6B10X6G10X6R10X6_422_UNORM_4PACK16
            // VULKAN_FORMAT_AS_STRING_CASE(B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR) = B10X6G10X6R10X6G10X6_422_UNORM_4PACK16
            // VULKAN_FORMAT_AS_STRING_CASE(G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR) = G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16
            // VULKAN_FORMAT_AS_STRING_CASE(G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR) = G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16
            // VULKAN_FORMAT_AS_STRING_CASE(G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR) = G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16
            // VULKAN_FORMAT_AS_STRING_CASE(G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR) = G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16
            // VULKAN_FORMAT_AS_STRING_CASE(G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR) = G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16
            // VULKAN_FORMAT_AS_STRING_CASE(R12X4_UNORM_PACK16_KHR) = R12X4_UNORM_PACK16
            // VULKAN_FORMAT_AS_STRING_CASE(R12X4G12X4_UNORM_2PACK16_KHR) = R12X4G12X4_UNORM_2PACK16
            // VULKAN_FORMAT_AS_STRING_CASE(R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR) = R12X4G12X4B12X4A12X4_UNORM_4PACK16
            // VULKAN_FORMAT_AS_STRING_CASE(G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR) = G12X4B12X4G12X4R12X4_422_UNORM_4PACK16
            // VULKAN_FORMAT_AS_STRING_CASE(B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR) = B12X4G12X4R12X4G12X4_422_UNORM_4PACK16
            // VULKAN_FORMAT_AS_STRING_CASE(G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR) = G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16
            // VULKAN_FORMAT_AS_STRING_CASE(G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR) = G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16
            // VULKAN_FORMAT_AS_STRING_CASE(G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR) = G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16
            // VULKAN_FORMAT_AS_STRING_CASE(G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR) = G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16
            // VULKAN_FORMAT_AS_STRING_CASE(G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR) = G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16
            // VULKAN_FORMAT_AS_STRING_CASE(G16B16G16R16_422_UNORM_KHR) = G16B16G16R16_422_UNORM
            // VULKAN_FORMAT_AS_STRING_CASE(B16G16R16G16_422_UNORM_KHR) = B16G16R16G16_422_UNORM
            // VULKAN_FORMAT_AS_STRING_CASE(G16_B16_R16_3PLANE_420_UNORM_KHR) = G16_B16_R16_3PLANE_420_UNORM
            // VULKAN_FORMAT_AS_STRING_CASE(G16_B16R16_2PLANE_420_UNORM_KHR) = G16_B16R16_2PLANE_420_UNORM
            // VULKAN_FORMAT_AS_STRING_CASE(G16_B16_R16_3PLANE_422_UNORM_KHR) = G16_B16_R16_3PLANE_422_UNORM
            // VULKAN_FORMAT_AS_STRING_CASE(G16_B16R16_2PLANE_422_UNORM_KHR) = G16_B16R16_2PLANE_422_UNORM
            // VULKAN_FORMAT_AS_STRING_CASE(G16_B16_R16_3PLANE_444_UNORM_KHR) = G16_B16_R16_3PLANE_444_UNORM

            default:
                assert(false);
                return "UNKNOWN FORMAT";
        }

#undef VULKAN_FORMAT_AS_STRING_CASE
    }

    const char* present_mode_as_string(VkPresentModeKHR present_mode){
        switch(present_mode){
            case VK_PRESENT_MODE_IMMEDIATE_KHR:
                return "VK_PRESENT_MODE_IMMEDIATE_KHR";
            case VK_PRESENT_MODE_MAILBOX_KHR:
                return "VK_PRESENT_MODE_MAILBOX_KHR";
            case VK_PRESENT_MODE_FIFO_KHR:
                return "VK_PRESENT_MODE_FIFO_KHR";
            case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
                return "VK_PRESENT_MODE_FIFO_RELAXED_KHR";
            case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
                return "VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR";
            case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
                return "VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR";

            default:
                assert(false);
                return "UNKNOWN PRESENT_MODE";
        }
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
            VkDebugUtilsMessageSeverityFlagBitsEXT severity,
            VkDebugUtilsMessageTypeFlagsEXT type,
            const VkDebugUtilsMessengerCallbackDataEXT* message_data,
            void* user_data){

        LOG_RAW("---- Vulkan Debug Message ----");
        LOG_RAW("SEVERITY: %s", debug_severity_as_string(severity));
        LOG_RAW("TYPE:     %s", debug_type_as_string(type));
        LOG_RAW("MESSAGE:  %s", message_data->pMessage);

        if(message_data->queueLabelCount){
            LOG_RAW("QUEUE LABELS:");
            for(u32 ilabel = 0u; ilabel != message_data->queueLabelCount; ++ilabel){
                LOG_RAW("%s", message_data->pQueueLabels[ilabel]);
            }
        }

        if(message_data->cmdBufLabelCount){
            LOG_RAW("COMMAND BUFFER LABELS:");
            for(u32 ilabel = 0u; ilabel != message_data->cmdBufLabelCount; ++ilabel){
                LOG_RAW("%s", message_data->pCmdBufLabels[ilabel]);
            }
        }

        if(message_data->objectCount){
            LOG_RAW("OBJECTS:");
            for(u32 iobject = 0u; iobject != message_data->objectCount; ++iobject){
                LOG_RAW(" TYPE:   %s", object_type_as_string(message_data->pObjects[iobject].objectType));
                LOG_RAW(" HANDLE: %u", message_data->pObjects[iobject].objectHandle);
                LOG_RAW(" NAME:   %s", message_data->pObjects[iobject].pObjectName);
            }
        }

        return VK_FALSE;
    }

    VkDebugUtilsMessengerCreateInfoEXT default_debug_messenger_create_info(){
        VkDebugUtilsMessengerCreateInfoEXT output = {};

        output.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

        output.messageSeverity = NULL
//            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
//            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        output.messageType = NULL
            | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        output.pfnUserCallback = VK::debug_callback;
        output.pUserData = nullptr;

        return output;
    }

    void register_debug_callback(VkInstance instance, VkDebugUtilsMessengerEXT* debug_messenger){
        VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info = default_debug_messenger_create_info();
        VK_CHECK(vkCreateDebugUtilsMessengerEXT(instance, &debug_messenger_create_info, nullptr, debug_messenger));
    }
};
