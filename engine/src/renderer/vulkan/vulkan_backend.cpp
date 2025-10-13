#include "vulkan_backend.h"

#include "core/logger.h"
#include "core/kstring.h"

// static vulkan_context, already zero'ed out by default
vulkan_context vulkan_renderer_backend::context;

b8 vulkan_renderer_backend::initialize(renderer_backend_struct* backend, const char* application_name, platform_state* plat_state) {
    // TODO: Custom allocator
    context.allocator = 0;

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

    

    KINFO("Vulkan renderer initialized successfully.");
    return true;
}

void vulkan_renderer_backend::shutdown(renderer_backend_struct* backend) {

}

void vulkan_renderer_backend::on_resized(renderer_backend_struct* backend, u16 width, u16 height) {

}
    
b8 vulkan_renderer_backend::begin_frame(renderer_backend_struct* backend, f32 delta_time) {
    return true;
}

b8 vulkan_renderer_backend::end_frame(renderer_backend_struct* backend, f32 delta_time) {
    return true;
}