#include "vulkan_backend.h"

#include "core/logger.h"
#include "core/kstring.h"
#include "containers/darray.h"

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