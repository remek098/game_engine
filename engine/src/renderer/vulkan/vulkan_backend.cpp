#include "vulkan_backend.h"

#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_renderpass.h"
#include "vulkan_command_buffer.h"
#include "vulkan_framebuffer.h"
#include "vulkan_fence.h"
#include "vulkan_utils.h"

#include "core/logger.h"
#include "core/kstring.h"
#include "containers/darray.h"
#include "platform/platform.h"

#include "core/application.h"

// static vulkan_context, already zero'ed out by default
vulkan_context vulkan_renderer_backend::context;


u32 vulkan_renderer_backend::cached_framebuffer_width = 0;
u32 vulkan_renderer_backend::cached_framebuffer_height = 0;

// any forward declerations
VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_types,
        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
        void* user_data);


b8 vulkan_renderer_backend::initialize(renderer_backend_struct* backend, const char* application_name, platform_state* plat_state) {
    // function pointers
    context.find_memory_index = &vulkan_renderer_backend::find_memory_index;

    // TODO: Custom allocator
    context.allocator = 0;
    
    application::app_get_framebuffer_size(&cached_framebuffer_width, &cached_framebuffer_height);
    context.framebuffer_width = (cached_framebuffer_width != 0) ? cached_framebuffer_width : 800;
    context.framebuffer_height = (cached_framebuffer_height != 0) ? cached_framebuffer_height : 600;
    cached_framebuffer_width = 0;
    cached_framebuffer_height = 0;

    // setup vulkan instance
    VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.apiVersion = VK_API_VERSION_1_4;
    app_info.pApplicationName = application_name;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "K Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    // VkInstanceCreateInfo create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    // create_info.pApplicationInfo = &app_info;
    // create_info.enabledExtensionCount = 0;
    // create_info.ppEnabledExtensionNames = 0;
    // create_info.enabledLayerCount = 0;
    // create_info.ppEnabledLayerNames = 0;
    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    VkResult result = vkCreateInstance(&create_info, context.allocator, &context.instance);
    if(result != VK_SUCCESS) {
        KERROR("vkCreateInstance failed with result: %u", result );
        return false;
    }

    // default construct darray object, create it with default capacity
    darray<const char*> required_extension_names_darray;
    // required_extension_names_darray.create();
    required_extension_names_darray.push(VK_KHR_SURFACE_EXTENSION_NAME);
    required_extension_names_darray.push("VK_KHR_win32_surface");
    
// do the debug stuff if needed
#if defined(_DEBUG)
    required_extension_names_darray.push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    const char** required_extensions = required_extension_names_darray.get_array();
    
    u32 length = required_extension_names_darray.get_length();
    KDEBUG("Length of required extensions darray: %u", length);
    KDEBUG("Required extensions: ");
    for(u32 i = 0; i < length; ++i) {
        KDEBUG("Extension[%u]: %s", i, required_extensions[i]);
    }
#endif
    create_info.enabledExtensionCount = required_extension_names_darray.get_length();
    create_info.ppEnabledExtensionNames = required_extension_names_darray.get_array();

    // validation layers -> process the same with darray like required extensions if we need;
    // leaving required_validation_layers as 0 (nullptr) for now because darray by default would allocate a pointer to the one char* underneath
    const char** required_validation_layer_names = 0;
    u32 required_validation_layers_count = 0;

#if defined(_DEBUG)
    KINFO("Validation layers enabled. Enumerating...");

    // the list of validation layers required
    darray<const char*> required_validation_layer_names_darray;
    required_validation_layer_names_darray.push("VK_LAYER_KHRONOS_validation");
    required_validation_layers_count = required_validation_layer_names_darray.get_length();
    // remember that this has to be assigned only after we finish adding layer names to our darray
    required_validation_layer_names = required_validation_layer_names_darray.get_array();  
    // KINFO("%s", required_validation_layers[0]);
    // KINFO("%u", required_validation_layers_count);

    // obtain a list of available valiation layers
    // u32 available_layers_count = 0;
    // VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layers_count, 0));
    // darray<VkLayerProperties> available_layers(available_layers_count);
    // VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layers_count, available_layers.get_array()));

    u32 available_layers_count = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layers_count, nullptr));

    if (available_layers_count == 0) {
        return false;
    }

    darray<VkLayerProperties> available_layers(available_layers_count);

    u32 query_count = available_layers_count;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&query_count, available_layers.get_array()));
    available_layers.field_set(DARRAY_LENGTH, query_count);

    // verify all required layers are available
    for(u32 i = 0; i < required_validation_layers_count; ++i) {
        KINFO("Searching for layer: %s...", required_validation_layer_names[i]);
        b8 found = false;
        for(u32 j = 0; j < available_layers_count; ++j) {
            if(kstring::are_equal(required_validation_layer_names[i], available_layers.get_array()[j].layerName)) {
                found = true;
                KINFO("Found.");
                break;
            }
        }
        if(!found) {
            KFATAL("Required validation layer is missing: %s", required_validation_layer_names[i]);
            return false;
        }
        
    }
    KINFO("All required validation layers are present.");
#endif

    create_info.enabledLayerCount = required_validation_layers_count;
    create_info.ppEnabledLayerNames = required_validation_layer_names;

    VK_CHECK(vkCreateInstance(&create_info, context.allocator, &context.instance));
    KINFO("-----------------Vulkan instance created.");

// Debugger setup
#if defined(_DEBUG)
    KDEBUG("----- Creating Vulkan debugger ....");
    u32 log_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT    | 
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;  // | 
                                                                        // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT     |
                                                                        // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debug_create_info.messageSeverity = log_severity;
    debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     | 
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | 
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debug_create_info.pfnUserCallback = vk_debug_callback;

    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, 
        "vkCreateDebugUtilsMessengerEXT");
    
    KASSERT_MSG(func, "---- Failed to create debug messenger!");
    VK_CHECK(func(context.instance, &debug_create_info, context.allocator, &context.debug_messenger));
    KDEBUG("---- Vulkan debugger created.");
#endif

    // surface
    KDEBUG("Creating Vulkan Surface ...");
    if(!platform::create_vulkan_surface(plat_state, &context)) {
        KERROR("Failed to create platform surface!");
        return false;
    }
    KDEBUG("------- Vulkan surface created.");

    // device creation
    if(!vulkan_device_create(&context)) {
        KERROR("------------ Failed to create device!");
        return false;
    }

    // create swapchain
    vulkan_swapchain_create(&context, context.framebuffer_width, context.framebuffer_height, &context.swapchain);

    vulkan_renderpass_create(
        &context, 
        &context.main_renderpass,
        0, 0, context.framebuffer_width, context.framebuffer_height,
        0.0f, 0.0f, 0.2f, 1.0f,
        1.0f,
        0
    );

    context.swapchain.framebuffers = 
        reinterpret_cast<vulkan_framebuffer*>(kmemory::allocate(sizeof(vulkan_framebuffer) * context.swapchain.image_count, MEMORY_TAG_RENDERER));
    for(u32 i = 0; i < context.swapchain.image_count; ++i) {
        kmemory::zero(&context.swapchain.framebuffers[i], sizeof(vulkan_framebuffer));
    }
    regenerate_framebuffers(backend, &context.swapchain, &context.main_renderpass);

    create_command_buffers(backend);

    // create sync objects
    context.image_available_semaphores = reinterpret_cast<VkSemaphore*>(darray_reserve(VkSemaphore, context.swapchain.max_frames_in_flight));
    // context.queue_completed_semaphores = reinterpret_cast<VkSemaphore*>(darray_reserve(VkSemaphore, context.swapchain.max_frames_in_flight));
    // NOTE: changing to see if it hels
    context.queue_completed_semaphores = reinterpret_cast<VkSemaphore*>(darray_reserve(VkSemaphore, context.swapchain.image_count));
    context.in_flight_fences = reinterpret_cast<vulkan_fence*>(darray_reserve(vulkan_fence, context.swapchain.max_frames_in_flight));
    

    for(u8 i = 0; i < context.swapchain.max_frames_in_flight; ++i) {
        VkSemaphoreCreateInfo semaphore_ci = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        VK_CHECK(vkCreateSemaphore(context.device.logical_device, &semaphore_ci, context.allocator, &context.image_available_semaphores[i]));
        // VK_CHECK(vkCreateSemaphore(context.device.logical_device, &semaphore_ci, context.allocator, &context.queue_completed_semaphores[i]));

        vulkan_fence_create(&context, true, &context.in_flight_fences[i]);
    }

    // in flight fences should not yet exist at this point, so clear the list. These are stored in pointers 
    // because initial state should be 0, and will be 0 when not in use. Actual fences are not owned by this list
    // NOTE: maybe we want to assign sizes of vulkan_fence*, but idk for now. (just keep that in mind, might be error prone)
    KWARN("Checkout next size for context.images_in_flight !!! \n context.images_in_fligth = reinterpret_cast<vulkan_fence**>(darray_reserve(vulkan_fence, context.swapchain.image_count));");
    context.images_in_fligth = reinterpret_cast<vulkan_fence**>(darray_reserve(vulkan_fence, context.swapchain.image_count));
    for(u32 i = 0; i < context.swapchain.image_count; ++i) {
        context.images_in_fligth[i] = 0;

        VkSemaphoreCreateInfo semaphore_ci = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        VK_CHECK(vkCreateSemaphore(context.device.logical_device, &semaphore_ci, context.allocator, &context.queue_completed_semaphores[i]));
    }

    KINFO("Vulkan renderer initialized successfully.");
    return true;
}

void vulkan_renderer_backend::shutdown(renderer_backend_struct* backend) {
    // // free_command_buffers();
    // vulkan_renderpass_destroy(&context, &context.main_renderpass);
    // vulkan_swapchain_destroy(&context, &context.swapchain);
    // vulkan_device_destroy(&context);
    // platform::destroy_vulkan_surface(&context);
    // KDEBUG("---- Destroying Vulkan debugger...");
    // if(context.debug_messenger) {
    //     PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, 
    //         "vkDestroyDebugUtilsMessengerEXT");
    //     func(context.instance, context.debug_messenger, context.allocator);
    // }

    // KDEBUG("---- Destroying Vulkan Instance");
    // vkDestroyInstance(context.instance, context.allocator);

    if (context.device.logical_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(context.device.logical_device);
    }

    // destroy sync objects
    for(u8 i = 0; i < context.swapchain.max_frames_in_flight; ++i) {
        if(context.image_available_semaphores[i]) {
            vkDestroySemaphore(context.device.logical_device, context.image_available_semaphores[i], context.allocator);
            context.image_available_semaphores[i] = 0;
        }

        // if(context.queue_completed_semaphores[i]) {
        //     vkDestroySemaphore(context.device.logical_device, context.queue_completed_semaphores[i], context.allocator);
        //     context.queue_completed_semaphores[i] = 0;
        // }

        vulkan_fence_destroy(&context, &context.in_flight_fences[i]);
    }

    for(u32 i = 0; i < context.swapchain.image_count; ++i) {
        if(context.queue_completed_semaphores[i]) {
            vkDestroySemaphore(context.device.logical_device, context.queue_completed_semaphores[i], context.allocator);
            context.queue_completed_semaphores[i] = 0;
        }
    }


    darray_destroy(context.image_available_semaphores);
    // kmemory::free(context.image_available_semaphores, sizeof(VkSemaphore) * context.swapchain.max_frames_in_flight, MEMORY_TAG_DARRAY);
    context.image_available_semaphores = 0;

    darray_destroy(context.queue_completed_semaphores);
    // kmemory::free(context.queue_completed_semaphores, sizeof(VkSemaphore) * context.swapchain.max_frames_in_flight, MEMORY_TAG_DARRAY);
    context.queue_completed_semaphores = 0;

    darray_destroy(context.in_flight_fences);
    context.in_flight_fences = 0;

    darray_destroy(context.images_in_fligth);
    // kmemory::free(context.queue_completed_semaphores, sizeof(vulkan_fence) * context.swapchain.max_frames_in_flight, MEMORY_TAG_DARRAY);
    context.images_in_fligth = 0;


    for(u32 i = 0; i < context.swapchain.image_count; ++i) {
        vulkan_framebuffer_destroy(&context, &context.swapchain.framebuffers[i]);
        // KDEBUG("-- did destroy attachment %i", i);
    }
    kmemory::free(context.swapchain.framebuffers, sizeof(vulkan_framebuffer) * context.swapchain.image_count, MEMORY_TAG_RENDERER);
    context.swapchain.framebuffers = 0;

    vulkan_renderpass_destroy(&context, &context.main_renderpass);
    vulkan_swapchain_destroy(&context, &context.swapchain);

    if(context.graphics_command_buffers) {
        darray_destroy(context.graphics_command_buffers);
        context.graphics_command_buffers = 0;
    }

    // 🔴 3. Destroy command pool (DO NOT free command buffers manually)
    if (context.device.graphics_command_pool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(
            context.device.logical_device,
            context.device.graphics_command_pool,
            context.allocator
        );
        context.device.graphics_command_pool = VK_NULL_HANDLE;
    }
    vulkan_device_destroy(&context);
    platform::destroy_vulkan_surface(&context);
    KDEBUG("---- Destroying Vulkan debugger...");
    if (context.debug_messenger) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
        func(context.instance, context.debug_messenger, context.allocator);
    }

    vkDestroyInstance(context.instance, context.allocator);
}

void vulkan_renderer_backend::on_resized(renderer_backend_struct* backend, u16 width, u16 height) {
    // make sure to indicate that framebuffer size has changed
    cached_framebuffer_width = width;
    cached_framebuffer_height = height;
    context.framebuffer_size_generation++;

    KINFO("Vulkan renderer backend -> resized: w/h/gen: %i/%i/%llu", width, height, context.framebuffer_size_generation);
}
    
b8 vulkan_renderer_backend::begin_frame(renderer_backend_struct* backend, f32 delta_time) {
    vulkan_device* device = &context.device;

    // check if we're recreating swapchain. If yes, boot out.
    if(context.recreating_swapchain) {
        VkResult result = vkDeviceWaitIdle(device->logical_device);
        if(!vulkan_result_is_success(result)) {
            KERROR("vulkan_renderer_backend::begin_frame vkDeviceWaitIdle (1) failed: '%s'", vulkan_result_string(result, true));
            return false;
        }
        KINFO("Recreating swapchain, boot out.");
        return false;
    }

    // check if framebuffer has been resized. If yes, we got to recreate swapchain
    if(context.framebuffer_size_generation != context.framebuffer_size_last_generation) {
        VkResult result = vkDeviceWaitIdle(device->logical_device);
        if(!vulkan_result_is_success(result)) {
            KERROR("vulkan_renderer_backend::begin_frame vkDeviceWaitIdle (2) failed: '%s'", vulkan_result_string(result, true));
            return false;
        }

        // if swapchain recreation failed (because e.g. the window was minimized) -> boot out before unsetting the flag.
        if(!recreate_swapchain(backend)) {
            return false;
        }

        KINFO("Resized, booting.");
        return false; // we cannot begin drawing on this frame. We got to do this on next frame.
    }

    // wait for the execution of current frame to complete.
    if(!vulkan_fence_wait(&context, &context.in_flight_fences[context.current_frame], UINT64_MAX)) {
        KWARN("In-flight fence wait failure. (Begin frame)");
        return false;
    }

    // acquire next image from the swap chain. Pass along  the semaphore that should be signaled when this completes.
    // the same semaphore will later be waited on by queue submission to ensure this image is available.
    if(!vulkan_swapchain_acquire_next_image_index(
        &context,
        &context.swapchain,
        UINT64_MAX,
        context.image_available_semaphores[context.current_frame],
        0,
        &context.image_index
    )) {
        return false;
    }

    // begin recording commands
    vulkan_command_buffer* command_buffer = &context.graphics_command_buffers[context.image_index];
    vulkan_command_buffer_reset(command_buffer);
    vulkan_command_buffer_begin(command_buffer, false, false, false);

    // rendering from bottom up like OpenGL does
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (f32)context.framebuffer_height;
    viewport.width = (f32)context.framebuffer_width;
    viewport.height = -(f32)context.framebuffer_height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = context.framebuffer_width;
    scissor.extent.height = context.framebuffer_height;

    vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer->handle, 0, 1, &scissor);

    context.main_renderpass.w = context.framebuffer_width;
    context.main_renderpass.h = context.framebuffer_height;

    vulkan_renderpass_begin(command_buffer, &context.main_renderpass, context.swapchain.framebuffers[context.image_index].handle);
    

    return true;
}

b8 vulkan_renderer_backend::end_frame(renderer_backend_struct* backend, f32 delta_time) {
    vulkan_command_buffer* command_buffer = &context.graphics_command_buffers[context.image_index];

    vulkan_renderpass_end(command_buffer, &context.main_renderpass);
    vulkan_command_buffer_end(command_buffer);

    // make sure the previous frame is not using this image (i.e. it's fence is being waited on)
    if(context.images_in_fligth[context.image_index] != VK_NULL_HANDLE) {
        vulkan_fence_wait(&context, context.images_in_fligth[context.image_index], UINT64_MAX);
    }

    // mark the image fence as in-use by this frame
    context.images_in_fligth[context.image_index] = &context.in_flight_fences[context.current_frame];

    // reset the fence for use on the next frame
    vulkan_fence_reset(&context, &context.in_flight_fences[context.current_frame]);

    // submit queue and wait for operation to complete
    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer->handle;

    // the semaphore(s) to be signaled when the queue is complete.
    submit_info.signalSemaphoreCount = 1;
    // submit_info.pSignalSemaphores = &context.queue_completed_semaphores[context.current_frame];
    submit_info.pSignalSemaphores = &context.queue_completed_semaphores[context.image_index];

    // wait semaphore ensures that the operation cannot begin until the image is available.
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &context.image_available_semaphores[context.current_frame];

    // each semaphore wait on the corresponding pipeline stage to complete. 1:1 ratio.
    // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent colour attachment writes from executing until 
    // the semaphore signals (i.e. one frame is presented at the time)
    VkPipelineStageFlags flags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.pWaitDstStageMask = flags;

    VkResult result = vkQueueSubmit(
        context.device.graphics_queue,
        1,
        &submit_info,
        context.in_flight_fences[context.current_frame].handle
    );

    if(result != VK_SUCCESS) {
        KERROR("vkQueueSubmit failed with result '%s'", vulkan_result_string(result, true));
        return false;
    }

    vulkan_command_buffer_update_submitted(command_buffer);
    // end of queue submission

    // give the image back to swapchain
    vulkan_swapchain_present(
        &context, &context.swapchain, 
        context.device.graphics_queue,
        context.device.present_queue,
        context.queue_completed_semaphores[context.image_index],
        context.image_index
    );
    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_types,
        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
        void* user_data) {
    
    switch (message_severity) {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            KERROR(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            KWARN(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            KINFO(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            KTRACE(callback_data->pMessage);
            break;
    }
    return VK_FALSE;
}

i32 vulkan_renderer_backend::find_memory_index(u32 type_filter, u32 property_flags) {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(context.device.physical_device, &memory_properties);

    for(u32 i = 0; i < memory_properties.memoryTypeCount; ++i) {
        // check each memory type to see if its bit is set to 1.
        if(type_filter & (1 << i) && 
                (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags) {
            return i;
        }
    }

    KWARN("-- Unable to find suitable memory type!");
    return -1;
    
}

void vulkan_renderer_backend::create_command_buffers(renderer_backend_struct* backend) {
    if(!context.graphics_command_buffers) {
        // context.graphics_command_buffers = darray<vulkan_command_buffer>(context.swapchain.image_count).get_array();
        context.graphics_command_buffers = reinterpret_cast<vulkan_command_buffer*>(
            darray_reserve(vulkan_command_buffer, context.swapchain.image_count));
        
        for(u32 i = 0; i < context.swapchain.image_count; ++i) {
            kmemory::zero(&context.graphics_command_buffers[i], sizeof(vulkan_command_buffer));
        }
    }

    for(u32 i = 0; i < context.swapchain.image_count; ++i) {
        // if(context.graphics_command_buffers[i].handle) {
        //     vulkan_command_buffer_free(&context, context.device.graphics_command_pool, &context.graphics_command_buffers[i]);
        // }
        kmemory::zero(&context.graphics_command_buffers[i], sizeof(vulkan_command_buffer));
        vulkan_command_buffer_allocate(&context, context.device.graphics_command_pool, true, &context.graphics_command_buffers[i]);
    }

    KDEBUG("--Vulkan command buffers created.");
}

void vulkan_renderer_backend::regenerate_framebuffers(renderer_backend_struct* backend, vulkan_swapchain* swapchain, vulkan_renderpass* renderpass) {
    for(u32 i = 0; i < swapchain->image_count; ++i) {
        // TODO: make this dynamic based on the currently configured attachments
        u32 attachment_count = 2;
        VkImageView attachments[] = {
            swapchain->image_views[i],
            swapchain->depth_attachment.view
        };

        vulkan_framebuffer_create(&context, renderpass, context.framebuffer_width, context.framebuffer_height, 
            attachment_count, attachments, &context.swapchain.framebuffers[i]);
    }
}

b8 vulkan_renderer_backend::recreate_swapchain(renderer_backend_struct* backend) {
    if(context.recreating_swapchain) {
        KDEBUG("vulkan_renderer_backend::recreate_swapchain called when already recreating. booting out.");
        return false;
    }

    // detect if window is too small to be drawn to
    if(context.framebuffer_width == 0 || context.framebuffer_height == 0) {
        KDEBUG("vulkan_renderer_backend::recreate_swapchain called when window size is < 1 in a dimension. Booting out.");
        return false;
    }


    context.recreating_swapchain = true;
    vkDeviceWaitIdle(context.device.logical_device);

    // clear these out just in case.
    for(u32 i = 0; i < context.swapchain.image_count; ++i) {
        context.images_in_fligth[i] = 0;
    }

    // requery support
    vulkan_device_query_swapchain_support(context.device.physical_device, context.surface, &context.device.swapchain_support);
    vulkan_device_detect_depth_format(&context.device);
    
    vulkan_swapchain_recreate(&context, cached_framebuffer_width, cached_framebuffer_height, &context.swapchain);

    // sync the framebuffer size with the cached size
    context.framebuffer_width = cached_framebuffer_width;
    context.framebuffer_height = cached_framebuffer_height;
    context.main_renderpass.w = cached_framebuffer_width;
    context.main_renderpass.h = cached_framebuffer_height;
    cached_framebuffer_width = 0;
    cached_framebuffer_height = 0;

    // update framebuffer size generation
    context.framebuffer_size_last_generation = context.framebuffer_size_generation;

    // cleanup swapchain
    for(u32 i = 0; i < context.swapchain.image_count; ++i) {
        vulkan_command_buffer_free(&context, context.device.graphics_command_pool, &context.graphics_command_buffers[i]);
    }

    // destroy framebuffers since they're no longer valid
    for(u32 i = 0; i < context.swapchain.image_count; ++i) {
        vulkan_framebuffer_destroy(&context, &context.swapchain.framebuffers[i]);
    }

    context.main_renderpass.x = 0;
    context.main_renderpass.y = 0;
    context.main_renderpass.w = context.framebuffer_width;
    context.main_renderpass.h = context.framebuffer_height;

    regenerate_framebuffers(backend, &context.swapchain, &context.main_renderpass);

    create_command_buffers(backend);

    context.recreating_swapchain = false;

    return true;
}

// void vulkan_renderer_backend::free_command_buffers() {
//     if(context.graphics_command_buffers) {
//         for(u32 i = 0; i < context.swapchain.image_count; ++i) {
//             vulkan_command_buffer_free(&context, context.device.graphics_command_pool, &context.graphics_command_buffers[i]);
//         }

//     }
// }