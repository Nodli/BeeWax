void Window_SDL_VK::initialize(const Window_Settings& settings){
    SDL_DisplayMode display_mode;

    if(SDL_GetDesktopDisplayMode(0, &display_mode)){
        LOG_ERROR("Failed SDL_GetDesktopDisplayMode() %s\n", SDL_GetError());
    }

    assert(!(settings.width > display_mode.w));
    assert(!(settings.height > display_mode.h));

    if(settings.width){
        width = settings.width;
    }else{
        width = display_mode.w;
    }

    if(settings.height){
        height = settings.height;
    }else{
        height = display_mode.h;
    }

    const char* window_name = "window_SDL_VK";
    if(settings.name){
        window_name = settings.name;
    }

    handle = SDL_CreateWindow(window_name,
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            width, height,
            SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);
    if(!handle){
        LOG_ERROR("Failed SDL_CreateWindow() %s\n", SDL_GetError());
        return;
    }

    // ---- Vulkan

    // NOTE(hugo): application info
    VkApplicationInfo vk_application_info = {};
    vk_application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vk_application_info.pApplicationName = window_name;
    vk_application_info.applicationVersion = 0u;
    vk_application_info.pEngineName = "BeeWax";
    vk_application_info.engineVersion = 0u;
    vk_application_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);

    static const char* expected_layers[] = {
        "VK_LAYER_KHRONOS_validation"
    };

    static const char* expected_extensions[] = {
        "VK_EXT_DEBUG_REPORT_EXTENSION_NAME"
    };

    // NOTE(hugo): layers
    darray<const char*> layers;
    {
        u32 navailable_layers;
        darray<VkLayerProperties> available_layers;

        VkResult vk_result = vkEnumerateInstanceLayerProperties(&navailable_layers, nullptr);
        if(vk_result != VK_SUCCESS){
            LOG_ERROR("Failed vkEnumerateInstanceLayersProperties() %d %s\n", vk_result, VK::cstring_vk_result(vk_result));
        }

        available_layers.set_min_capacity(navailable_layers);
        vk_result = vkEnumerateInstanceLayerProperties(&navailable_layers, available_layers.data);
        if(vk_result != VK_SUCCESS){
            LOG_ERROR("Failed vkEnumerateInstanceLayersProperties() %d %s\n", vk_result, VK::cstring_vk_result(vk_result));
        }

        for(u32 iexpected = 0u; iexpected != carray_size(expected_layers); ++iexpected){
            for(u32 iavailable = 0u; iavailable != available_layers.size; ++iavailable){
                if(strcmp(expected_layers[iexpected], available_layers[iavailable].layerName) == 0u){
                    layers.push(expected_layers[iexpected]);
                    break;
                }
            }
        }

        available_layers.free();
    }

    // NOTE(hugo): required extensions
    u32 nextensions;
    darray<const char*> extensions;
    {
        if(!SDL_Vulkan_GetInstanceExtensions(handle, &nextensions, nullptr)){
            LOG_ERROR("Failed SDK_Vulkan_GetInstanceExtensions() to get the number of extensions %s\n", SDL_GetError());
        }

        extensions.set_min_capacity(nextensions);
        extensions.size = nextensions;
        if(!SDL_Vulkan_GetInstanceExtensions(handle, &nextensions, extensions.data)){
            LOG_ERROR("Failed SDK_Vulkan_GetInstanceExtensions() to get the extensions %s\n", SDL_GetError());
        }

    }

    extensions.set_min_capacity(nextensions + 1u);
    extensions.push(nullptr);
    extensions[nextensions] = "VK_EXT_DEBUG_REPORT_EXTENSION_NAME";

    // NOTE(hugo): additional extensions
    {
        u32 navailable_extensions;
        darray<VkExtensionProperties> available_extensions;

        VkResult vk_result = vkEnumerateInstanceExtensionProperties(nullptr, &navailable_extensions, nullptr);
        if(vk_result != VK_SUCCESS){
            LOG_ERROR("Failed vkEnumerateInstanceExtensionProperties() %d %s\n", vk_result, VK::cstring_vk_result(vk_result));
        }

        available_extensions.set_min_capacity(navailable_extensions);
        vk_result = vkEnumerateInstanceExtensionProperties(nullptr, &navailable_extensions, available_extensions.data);
        if(vk_result != VK_SUCCESS){
            LOG_ERROR("Failed vkEnumerateInstanceExtensionProperties() %d %s\n", vk_result, VK::cstring_vk_result(vk_result));
        }

        for(u32 iexpected = 0u; iexpected != carray_size(expected_extensions); ++iexpected){
            for(u32 iavailable = 0u; iavailable != available_extensions.size; ++iavailable){
                if(strcmp(expected_extensions[iexpected], available_extensions[iavailable].extensionName) == 0u){
                    extensions.push(expected_extensions[iexpected]);
                    break;
                }
            }
        }

        available_extensions.free();
    }

    // NOTE(hugo): create the vulkan instance
    VkInstanceCreateInfo vk_create_info = {};
    vk_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vk_create_info.pApplicationInfo = &vk_application_info;
    vk_create_info.enabledLayerCount = layers.size;
    vk_create_info.ppEnabledLayerNames = layers.data;
    vk_create_info.enabledExtensionCount = nextensions;
    vk_create_info.ppEnabledExtensionNames = extensions.data;

    VkResult vk_result_instance = vkCreateInstance(&vk_create_info, nullptr, &instance);
    if(vk_result_instance != VK_SUCCESS){
        LOG_ERROR("Failed to vkCreateInstance() %d %s\n", vk_result_instance, VK::cstring_vk_result(vk_result_instance));
    }

    layers.free();
    extensions.free();

    // NOTE(hugo): setup debug message callback
    VkDebugUtilsMessengerCreateInfoEXT vk_debug_callback_create_info = {};
    vk_debug_callback_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_TUILS_MESSENGER_CREATE_INFO_EXT;
    vk_debug_callback_create_info.messageSeverity =
        VK_DEBUG_TUILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    vk_debug_callback_create_info.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    vk_debug_callback_create_info.pfnUserCallback = VK::debug_message_callback;
    vk_debug_callback_create_info.pUserData = nullptr;

    VkResult vk_result_debug_callback = CreateDebugUtilsMessengerEXT(
            instance,
            &vk_debug_callback_create_info,
            nullptr,


    VkDebugReportCallbackEXT callback;

    // ----

    // NOTE(hugo): ---- settings that do not require to recreate a window
    // TODO(hugo): properly handle the window resize / settings change

    Window_Settings::Mode screen_mode_to_use = settings.mode;

    if(screen_mode_to_use == Window_Settings::WINDOWED){
        SDL_SetWindowResizable(handle, SDL_TRUE);
    }else if(screen_mode_to_use == Window_Settings::BORDERLESS){
        SDL_SetWindowResizable(handle, SDL_FALSE);
        SDL_SetWindowBordered(handle, SDL_FALSE);
    }else if(screen_mode_to_use == Window_Settings::FULLSCREEN){
        SDL_SetWindowFullscreen(handle, SDL_TRUE);
    }

    SDL_RaiseWindow(handle);
}

void Window_SDL_VK::terminate(){
    if(instance){
        vkDestroyInstance(instance, nullptr);
    }
    if(handle){
        SDL_DestroyWindow(handle);
    }
}

void Window_SDL_VK::swap_buffers(){
}

float Window_SDL_VK::aspect_ratio(){
    return (float)width / (float)height;
}
