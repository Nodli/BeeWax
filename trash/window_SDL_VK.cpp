void Window_SDL_VK::initialize(const Window_Settings_SDL_VK& required_settings){
    // ---- SDL Window Settings

    settings = required_settings;

    // ---- SDL Window Creation

    LOG_INFO("-- SDL Window creationg");

    SDL_DisplayMode display_mode = {};
    SDL_CHECK(SDL_GetDesktopDisplayMode(0, &display_mode) == 0);

    width = settings.width ? min(settings.width, display_mode.w) : display_mode.w;
    height = settings.height ? min(settings.height, display_mode.h) : display_mode.h;
    const char* window_name = settings.name ? settings.name : "window_SDL_VK";

    window = SDL_CreateWindow(window_name,
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            width, height,
            SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);
    SDL_CHECK(window);

    // ---- SDL Window Settings

    if(settings.mode == Window_Settings_SDL_VK::mode_windowed){
        SDL_SetWindowResizable(window, SDL_FALSE);
    }else if(settings.mode == Window_Settings_SDL_VK::mode_borderless){
        SDL_SetWindowResizable(window, SDL_FALSE);
        SDL_SetWindowBordered(window, SDL_FALSE);
    }else if(settings.mode == Window_Settings_SDL_VK::mode_fullscreen){
        SDL_SetWindowFullscreen(window, SDL_TRUE);
    }

    SDL_RaiseWindow(window);
}

void Window_SDL_VK::terminate(){
    SDL_DestroyWindow(window);
}

void Window_SDL_VK::swap_buffers(){
}

float Window_SDL_VK::aspect_ratio(){
    return (float)width / (float)height;
}

vec2 Window_SDL_VK::pixel_to_screen_coordinates(ivec2 pixel){
    return {
        ((float)pixel.x / (float)width) * 2.f - 1.f,
        ((float)pixel.y / (float)height) * 2.f - 1.f
    };
}

ivec2 Window_SDL_VK::screen_to_pixel_coordinates(vec2 screen){
    return {
        (int)((screen.x + 1.f) * 0.5f * (float)width),
        (int)((screen.y + 1.f) * 0.5f * (float)height)
    };
}

void push_instance_extensions(SDL_Window* window, array<const char*>& instance_extensions){
    u32 nSDL_extensions;
    SDL_CHECK(SDL_Vulkan_GetInstanceExtensions(window, &nSDL_extensions, nullptr));

    u32 offset = instance_extensions.size;
    instance_extensions.set_size(instance_extensions.size + nSDL_extensions);
    SDL_CHECK(SDL_Vulkan_GetInstanceExtensions(window, &nSDL_extensions, instance_extensions.storage.data + offset));
}

void Renderer_VK::initialize(){
    // ---- Vulkan Settings

    LOG_INFO("-- Vulkan initialization");

    u32 required_instance_version = VK_API_VERSION_1_0;

    array<const char*> instance_extensions;
    push_instance_extensions(window->window, instance_extensions);

    array<const char*> device_extensions;
    device_extensions.push(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    LOG_INFO("required instance extensions:");
    for(auto extension : instance_extensions){
        LOG_INFO("  %s", extension);
    }
    LOG_INFO("required device extensions:");
    for(auto extension : device_extensions){
        LOG_INFO("  %s", extension);
    }

#if defined(DEBUG_BUILD)
    const char* instance_layers[] = {
        "VK_LAYER_KHRONOS_validation"
    };
    instance_extensions.push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    LOG_INFO("required layers");
    for(u32 ilayer = 0u; ilayer != carray_size(instance_layers); ++ilayer){
        LOG_INFO("  %s", instance_layers[ilayer]);
    }
#endif

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = window->settings.name;
    app_info.applicationVersion = 0u;
    app_info.pEngineName = "";
    app_info.engineVersion = 0u;
    app_info.apiVersion = required_instance_version;

    // ---- Vulkan Setup

    // NOTE(hugo): instance
    VK::require_instance_version(required_instance_version);
    LOG_INFO("required version: %u %u", VK_VERSION_MAJOR(required_instance_version), VK_VERSION_MINOR(required_instance_version));
    VK::require_instance_extensions(instance_extensions);

#if defined(DEBUG_BUILD)
    VK::require_instance_layers(instance_layers, carray_size(instance_layers));
#endif

    VkInstanceCreateInfo create_info_instance = {};
    create_info_instance.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info_instance.pApplicationInfo = &app_info;
    create_info_instance.enabledExtensionCount = instance_extensions.size;
    create_info_instance.ppEnabledExtensionNames = instance_extensions.storage.data;

#if defined(DEBUG_BUILD)
    create_info_instance.enabledLayerCount = carray_size(instance_layers);
    create_info_instance.ppEnabledLayerNames = instance_layers;

    VkDebugUtilsMessengerCreateInfoEXT create_info_instance_creation_debug_messenger = VK::default_debug_messenger_create_info();
    create_info_instance.pNext = &create_info_instance_creation_debug_messenger;
#endif

    VK_CHECK(vkCreateInstance(&create_info_instance, nullptr, &instance));

#if defined(DEBUG_BUILD)
    VK::INITIALIZE_vkCreateDebugUtilsMessengerEXT(instance);
    VK::INITIALIZE_vkDestroyDebugUtilsMessengerEXT(instance);
    VK::INITIALIZE_vkSetDebugUtilsObjectNameEXT(instance);
    VK::register_debug_callback(instance, &debug_messenger);
#endif

    // NOTE(hugo): surface
    SDL_CHECK(SDL_Vulkan_CreateSurface(window->window, instance, &swapchain.surface));

    // NOTE(hugo): physical device and queue families
    u32 queue_family_indices[2];
    u32& graphics_family_index = queue_family_indices[0];
    u32& present_family_index = queue_family_indices[1];
    VK::select_physical_device(instance, device_extensions, swapchain.surface,
            physical_device,
            physical_device_properties,
            physical_device_memory_properties,
            physical_device_features,
            graphics_family_index, present_family_index);

    // NOTE(hugo): device and queues
    float queue_priority = 1.f;
    VkDeviceQueueCreateInfo create_info_queues[2] = {};
    for(u32 idevice = 0u; idevice != carray_size(create_info_queues); ++idevice){
        create_info_queues[idevice].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        create_info_queues[idevice].queueCount = 1;
        create_info_queues[idevice].pQueuePriorities = &queue_priority;
    }
    u32 nqueues = 0u;
    create_info_queues[nqueues++].queueFamilyIndex = graphics_family_index;
    if(present_family_index != graphics_family_index) create_info_queues[nqueues++].queueFamilyIndex = present_family_index;

    VkPhysicalDeviceFeatures required_features = {};

    VkDeviceCreateInfo create_info_device = {};
    create_info_device.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info_device.queueCreateInfoCount = nqueues;
    create_info_device.pQueueCreateInfos = create_info_queues;
    create_info_device.enabledExtensionCount = device_extensions.size;
    create_info_device.ppEnabledExtensionNames = device_extensions.storage.data;
    create_info_device.pEnabledFeatures = &required_features;

    VK_CHECK(vkCreateDevice(physical_device, &create_info_device, nullptr, &device));

#if defined(DEBUG_BUILD)
    VkDebugUtilsObjectNameInfoEXT device_name_info = {};
    device_name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    device_name_info.objectType = VK_OBJECT_TYPE_DEVICE;
    device_name_info.objectHandle = (u64)device;
    device_name_info.pObjectName = "DEVICE";
    VK_CHECK(vkSetDebugUtilsObjectNameEXT(device, &device_name_info));
#endif

    vkGetDeviceQueue(device, graphics_family_index, 0u, &graphics_queue);
    vkGetDeviceQueue(device, present_family_index, 0u, &present_queue);

    // TODO(hugo): allocator

    // NOTE(hugo): swapchain (format, present mode and extents)
    swapchain_image_index = 0u;

    array<VkSurfaceFormatKHR> surface_formats = VK::detect_physical_device_surface_formats(physical_device, swapchain.surface);
    LOG_INFO("detected surface formats:");
    for(auto& format : surface_formats){
        LOG_INFO("  %s", VK::format_as_string(format.format));
    }
    swapchain.format = VK::select_physical_device_swapchain_format(surface_formats);
    LOG_INFO("selected surface format: %s", VK::format_as_string(swapchain.format.format));

    array<VkPresentModeKHR> surface_present_modes = VK::detect_physical_device_surface_present_modes(physical_device, surface);
    LOG_INFO("detected surface present modes:");
    for(auto& mode : surface_present_modes){
        LOG_INFO("  %s", VK::present_mode_as_string(mode));
    }
    swapchain.present_mode = VK::select_physical_device_present_mode(surface_present_modes);
    LOG_INFO("selected surface present mode: %s", VK::present_mode_as_string(swapchain.present_mode));

    VkSurfaceCapabilitiesKHR surface_capabilities;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities));

    swapchain.extent = VK::determine_swapchain_extents(surface_capabilities, (u32)window->width, (u32)window->height);
    LOG_INFO("swapchain extents: %u %u", swapchain.extent.width, swapchain.extent.height);

    u32 swapchain_image_count = surface_capabilities.minImageCount + 1u;
    if(surface_capabilities.minImageCount == 0u && surface_capabilities.maxImageCount < swapchain_image_count){
        swapchain_image_count = surface_capabilities.minImageCount;
    }

    VkCompositeAlphaFlagBitsKHR swapchain_composite_mode = VK::determine_composite_mode(surface_capabilities);

    VkSwapchainCreateInfoKHR create_info_swapchain = {};
    create_info_swapchain.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info_swapchain.surface = surface;
    create_info_swapchain.minImageCount = swapchain_image_count;
    create_info_swapchain.imageFormat = swapchain_format.format;
    create_info_swapchain.imageColorSpace = swapchain_format.colorSpace;
    create_info_swapchain.imageExtent = swapchain_extent;
    create_info_swapchain.imageArrayLayers = 1u;
    create_info_swapchain.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info_swapchain.preTransform = surface_capabilities.currentTransform;
    create_info_swapchain.compositeAlpha = swapchain_composite_mode;
    create_info_swapchain.presentMode = swapchain_present_mode;
    create_info_swapchain.clipped = swapchain_present_mode;
    create_info_swapchain.oldSwapchain = VK_NULL_HANDLE;

    if(graphics_family_index == present_family_index){
        create_info_swapchain.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info_swapchain.queueFamilyIndexCount = 0u;
        create_info_swapchain.pQueueFamilyIndices = nullptr;

    }else{
        create_info_swapchain.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info_swapchain.queueFamilyIndexCount = 2u;
        create_info_swapchain.pQueueFamilyIndices = queue_family_indices;

    }

    VK_CHECK(vkCreateSwapchainKHR(device, &create_info_swapchain, nullptr, &swapchain));

    u32 swapchain_size;
    vkGetSwapchainImagesKHR(device, swapchain, &swapchain_size, nullptr);
    swapchain_images.set_size(swapchain_size);
    vkGetSwapchainImagesKHR(device, swapchain, &swapchain_size, swapchain_images.storage.data);

    swapchain_image_views.set_size(swapchain_size);
    for(u32 iimage = 0u; iimage != swapchain_size; ++iimage){
        VkImageViewCreateInfo create_info_view = {};
        create_info_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info_view.image = swapchain_images[iimage];
        create_info_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info_view.format = swapchain.format.format;
        create_info_view.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info_view.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info_view.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info_view.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info_view.subresourceRange.baseMipLevel = 0;
        create_info_view.subresourceRange.levelCount = 1;
        create_info_view.subresourceRange.baseArrayLayer = 0;
        create_info_view.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(device, &create_info_view, nullptr, &swapchain_image_views[iimage]));
    }

    // NOTE(hugo): swapchain synchronization

    VkSemaphoreCreateInfo create_info_processing_semaphore = {};
    create_info_processing_semaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo create_info_processing_fence = {};
    create_info_processing_fence.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    create_info_processing_fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(u32 iframe = 0u; iframe != VULKAN_MAX_FRAMES_PROCESSING; ++iframe){
        VK_CHECK(vkCreateSemaphore(device, &create_info_processing_semaphore, nullptr, &processing_sync[iframe].image_acquire));
        VK_CHECK(vkCreateSemaphore(device, &create_info_processing_semaphore, nullptr, &processing_sync[iframe].image_release));
        VK_CHECK(vkCreateFence(device, &create_info_processing_fence, nullptr, &processing_sync[iframe].processing));
    }

    // NOTE(hugo): default framebuffer render pass

    VkAttachmentDescription color_attachment_description = {};
    color_attachment_description.format = renderer.swapchain_image_format;
    color_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_reference = {};
    color_attachment_reference.attachment = 0u;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_description = {};
    subpass_description.flags = NULL;
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.inputAttachmentCount = 0u;
    subpass_description.pInputAttachments = nullptr;
    subpass_description.colorAttachmentCount = 1u;
    subpass_description.pColorAttachments = &color_attachment_reference;
    subpass_description.pResolveAttachments = nullptr;
    subpass_description.pDepthStencilAttachment = nullptr;
    subpass_description.preserveAttachmentCount = 0u;
    subpass_description.pPreserveAttachments = nullptr;

    VkSubpassDependency subpass_dependency = {};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0u;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask = NULL;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo create_info_render_pass = {};
    create_info_render_pass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info_render_pass.flags = NULL;
    create_info_render_pass.attachmentCount = 1u;
    create_info_render_pass.pAttachments = &color_attachment_description;
    create_info_render_pass.subpassCount = 1u;
    create_info_render_pass.pSubpasses = &subpass_description;
    create_info_render_pass.dependencyCount = 1u;
    create_info_render_pass.pDependencies = &subpass_dependency;

    Render_Pass_VK output_render_pass;
    VK_CHECK(vkCreateRenderPass(renderer.device, &create_info_render_pass, nullptr, &output_render_pass.render_pass));

    return output_render_pass;


    // NOTE(hugo): default framebuffers

    swapchain_image_sync.set_size(swapchain_images.size);
    for(auto& sync : swapchain_image_sync){
        sync = VK_NULL_HANDLE;
    }

    // NOTE(hugo): command pool

    VkCommandPoolCreateInfo create_info_command_pool = {};
    create_info_command_pool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info_command_pool.queueFamilyIndex = graphics_family_index;
    create_info_command_pool.flags = 0;

    VK_CHECK(vkCreateCommandPool(device, &create_info_command_pool, nullptr, &command_pool));

    // ----

    surface_present_modes.free();
    surface_formats.free();
    device_extensions.free();
    instance_extensions.free();
}

void Renderer_VK::terminate(){
    vkDestroyCommandPool(device, command_pool, nullptr);
    swapchain_image_sync.free();
    for(auto& view : swapchain_image_views){
        vkDestroyImageView(device, view, nullptr);
    }
    swapchain_image_views.free();
    swapchain_images.free();
    for(u32 iframe = 0u; iframe != VULKAN_MAX_FRAMES_PROCESSING; ++iframe){
        vkDestroySemaphore(device, processing_sync[iframe].image_acquire, nullptr);
        vkDestroySemaphore(device, processing_sync[iframe].image_release, nullptr);
        vkDestroyFence(device, processing_sync[iframe].processing, nullptr);
    }
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
    vkDestroyInstance(instance, nullptr);
}

void Renderer_VK::begin_frame(){
    Frame_Sync& sync = swapchain_sync[frame_index];
    vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, sync.image_acquire, VK_NULL_HANDLE, &frame_index);
}

void Renderer_VK::end_frame(){
    frame_index = (frame_index + 1u) % VULKAN_MAX_FRAMES_PROCESSING;
}

Buffer_VK Renderer_VK::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_type){
    Buffer_VK output_buffer;

    VkBufferCreateInfo create_info_buffer = {};
    create_info_buffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info_buffer.size = size;
    create_info_buffer.usage = usage;
    create_info_buffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(device, &create_info_buffer, nullptr, &output_buffer.buffer));

    VkMemoryRequirements buffer_memory_req;
    vkGetBufferMemoryRequirements(device, buffer.buffer, &buffer_memory_req);

    u32 mem_type_index = VK::select_memory_type_index(physical_device_memory_properties, buffer_memory_req.memoryTypeBits, memory_type);
    ENGINE_CHECK(mem_type_index != UINT32_MAX, "**VULKAN: NO COMPATIBLE MEMORY TYPE FOUND**");

    VkMemoryAllocateInfo mem_alloc_info = {};
    mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc_info.allocationSize = buffer_memory_req.size;
    mem_alloc_info.memoryTypeIndex = mem_type_index;

    VK_CHECK(vkAllocateMemory(device, &mem_alloc_info, nullptr, &output_buffer.memory));

    vkBindBufferMemory(device, output_buffer.buffer, output_buffer.memory, 0u);

    vkMapMemory(device, output_buffer.memory, 0u, size, NULL, &output_buffer.data);

    output_buffer.size = (size_t)size;

    return output_buffer;
}

void Renderer_VK::transfer_buffer(const Buffer_VK& buffer){
    assert(data != nullptr);

    buffer.data = nullptr;
    vkUnmapMemory(device, buffer.memory);
}

// NOTE(hugo): not required to unmap the buffer before freeing its memory
// REF(hugo):
// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkFreeMemory.html
void Renderer_VK::destroy_buffer(const Buffer_VK& buffer){
    vkDestroyBuffer(device, buffer.buffer, nullptr);
    vkFreeMemory(device, buffer.memory, nullptr);
}

Render_Pass_VK create_render_pass(){
}

void Renderer_VK::destroy_render_pass(const Render_Pass_VK& render_pass){
    vkDestroyRenderPass(renderer.device, render_pass, nullptr);
}

Pipeline_VK Renderer_VK::create_graphics_pipeline(const Graphics_Pipeline_Settings& settings){
    Pipeline_VK output_pipeline;

    // NOTE(hugo): modules

    VkShaderModule vertex_module = VK::create_shader_module(device, settings.vertex_spirv, settings.vertex_size);
    VkShaderModule fragment_module = VK::create_shader_module(device, settings.fragment_spirv, settings.fragment_size);

    // NOTE(hugo): stages

    VkPipelineShaderStageCreateInfo create_info_shader_stages[2] = {};
    VkPipelineShaderStageCreateInfo& create_info_vertex_stage = create_info_shader_stages[0];
    VkPipelineShaderStageCreateInfo& create_info_fragment_stage = create_info_shader_stages[1];

    create_info_vertex_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    create_info_vertex_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    create_info_vertex_stage.module = vertex_module;
    create_info_vertex_stage.pName = "main";

    create_info_fragment_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    create_info_fragment_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    create_info_fragment_stage.module = fragment_module;
    create_info_fragment_stage.pName = "main";

    // NOTE(hugo): bindings & attributes description

    VkVertexInputBindingDescription vertex_xyzrgba_binding_description = {};
    vertex_xyzrgba_binding_description.binding = 0u;
    vertex_xyzrgba_binding_description.stride = sizeof(vertex_xyzrgba);
    vertex_xyzrgba_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vertex_xyzrgba_attribute_description;
    vertex_xyzrgba_attribute_description.binding = 0u;
    vertex_xyzrgba_attribute_description.location = 0u;
    vertex_xyzrgba_attribute_description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    vertex_xyzrgba_attribute_description.offset = 0u;

    VkPipelineVertexInputStateCreateInfo create_info_vertex_input_state = {};
    create_info_vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    create_info_vertex_input_state.vertexBindingDescriptionCount = 1u;
    create_info_vertex_input_state.pVertexBindingDescriptions = &vertex_xyzrgba_binding_description;
    create_info_vertex_input_state.vertexAttributeDescriptionCount = 1u;
    create_info_vertex_input_state.pVertexAttributeDescriptions = &vertex_xyzrgba_attribute_description;

    // NOTE(hugo): primitive

    VkPipelineInputAssemblyStateCreateInfo create_info_input_assembly_state = {};
    create_info_input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    create_info_input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    create_info_input_assembly_state.primitiveRestartEnable = VK_FALSE;

    // NOTE(hugo): viewport and scissor - should probably be dynamic

    VkViewport viewport = {};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = (float)swapchain_extent.width;
    viewport.height = (float)swapchain_extent.height;
    viewport.minDepth = 0.f;
    viewport.minDepth = 1.f;

    VkRect2D scissor = {};
    scissor.offset.x = 0.f;
    scissor.offset.y = 0.f;
    scissor.extent = swapchain_extent;

    VkPipelineViewportStateCreateInfo create_info_viewport_state = {};
    create_info_viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    create_info_viewport_state.viewportCount = 1;
    create_info_viewport_state.pViewports = &viewport;
    create_info_viewport_state.scissorCount = 1;
    create_info_viewport_state.pScissors = &scissor;

    // NOTE(hugo): rasterization

    VkPipelineRasterizationStateCreateInfo create_info_rasterizer = {};
    create_info_rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    create_info_rasterizer.depthClampEnable = VK_FALSE;
    create_info_rasterizer.rasterizerDiscardEnable = VK_FALSE;
    create_info_rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    create_info_rasterizer.lineWidth = 1.f;
    create_info_rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    create_info_rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    create_info_rasterizer.depthBiasEnable = VK_FALSE;
    create_info_rasterizer.depthBiasConstantFactor = 0.f;
    create_info_rasterizer.depthBiasClamp = 0.f;
    create_info_rasterizer.depthBiasSlopeFactor= 0.f;

    // NOTE(hugo): multisampling

    VkPipelineMultisampleStateCreateInfo create_info_multisampling = {};
    create_info_multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    create_info_multisampling.sampleShadingEnable = VK_FALSE;
    create_info_multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    create_info_multisampling.minSampleShading = 1.f;
    create_info_multisampling.pSampleMask = nullptr;
    create_info_multisampling.alphaToCoverageEnable = VK_FALSE;
    create_info_multisampling.alphaToOneEnable = VK_FALSE;

    // NOTE(hugo): blending

    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo create_info_color_blend = {};
    create_info_color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    create_info_color_blend.logicOpEnable = VK_FALSE;
    create_info_color_blend.logicOp = VK_LOGIC_OP_COPY;
    create_info_color_blend.attachmentCount = 1u;
    create_info_color_blend.pAttachments = &color_blend_attachment;
    create_info_color_blend.blendConstants[0] = 0.f;
    create_info_color_blend.blendConstants[1] = 0.f;
    create_info_color_blend.blendConstants[2] = 0.f;
    create_info_color_blend.blendConstants[3] = 0.f;

    // NOTE(hugo): pipeline layout

    VkPipelineLayoutCreateInfo create_info_pipeline_layout = {};
    create_info_pipeline_layout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info_pipeline_layout.setLayoutCount = 0u;
    create_info_pipeline_layout.pSetLayouts = nullptr;
    create_info_pipeline_layout.pushConstantRangeCount = 0u;
    create_info_pipeline_layout.pPushConstantRanges = 0u;

    VK_CHECK(vkCreatePipelineLayout(device, &create_info_pipeline_layout, nullptr, &output_pipeline.layout));

    // NOTE(hugo): dynamic state

#if 0
    VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT
    };
    VkPipelineDynamicStateCreateInfo create_info_dynamic_state = {};
    create_info_dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    create_info_dynamic_state.dynamicStateCount = carray_size(dynamic_states);
    create_info_dynamic_state.pDynamicStates = dynamic_states;
#endif

    // NOTE(hugo): pipeline

    VkGraphicsPipelineCreateInfo create_info_graphics_pipeline = {};
    create_info_graphics_pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    create_info_graphics_pipeline.stageCount = 2u;
    create_info_graphics_pipeline.pStages = create_info_shader_stages;
    create_info_graphics_pipeline.pVertexInputState = &create_info_vertex_input_state;
    create_info_graphics_pipeline.pInputAssemblyState = &create_info_input_assembly_state;
    create_info_graphics_pipeline.pViewportState = &create_info_viewport_state;
    create_info_graphics_pipeline.pRasterizationState = &create_info_rasterizer;
    create_info_graphics_pipeline.pMultisampleState = &create_info_multisampling;
    create_info_graphics_pipeline.pDepthStencilState = nullptr;
    create_info_graphics_pipeline.pColorBlendState = &create_info_color_blend;
    create_info_graphics_pipeline.pDynamicState = nullptr;
    create_info_graphics_pipeline.layout = pipeline_layout;
    create_info_graphics_pipeline.renderPass = render_pass;
    create_info_graphics_pipeline.subpass = 0u;
    create_info_graphics_pipeline.basePipelineHandle = VK_NULL_HANDLE;
    create_info_graphics_pipeline.basePipelineIndex = -1;

    VkPipeline graphics_pipeline;
    VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &create_info_graphics_pipeline, nullptr, &output_pipeline.pipeline));
}

void Renderer_VK::destroy_pipeline(const Pipeline_VK& pipeline){
    vkDestroyPipeline(device, pipeline.pipeline, nullptr);
    vkDestroyPipelineLayout(device, pipeline.layout, nullptr);
}
