#include "vulkan_device.h"

#include "core/logger.h"
#include "core/kstring.h"
#include "core/kmemory.h"
#include "containers/darray.h"

typedef struct vulkan_physical_device_requirements {
    b8 graphics;
    b8 present;
    b8 compute;
    b8 transfer;
    // darray
    darray<const char*> device_extension_names_darray;
    b8 sampler_anisotropy;
    b8 discrete_gpu;
} vulkan_physical_device_requirements;

typedef struct vulkan_physical_device_queue_family_info {
    u32 graphics_family_index;
    u32 present_family_index;
    u32 compute_family_index;
    u32 transfer_family_index;
} vulkan_physical_device_queue_family_info;


b8 select_physical_device(vulkan_context* context);
b8 physical_device_meets_requirements(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const vulkan_physical_device_requirements* requirements,
    vulkan_physical_device_queue_family_info* out_queue_family_info,
    vulkan_swapchain_support_info* out_swapchain_support_info
);

b8 vulkan_device_create(vulkan_context* vk_context) {
    if(!select_physical_device(vk_context)) {
        return false;
    }
    return true;
}


void vulkan_device_destroy(vulkan_context* vk_context) {
    KDEBUG("-----------------------------------Shutting down device ...");

    // potentially to free:
    // swapchain_support.formats and swapchain_support.present_modes
    if(vk_context->device.swapchain_support.present_modes) {
        kmemory::free(vk_context->device.swapchain_support.present_modes, 
            sizeof(VkPresentModeKHR) * vk_context->device.swapchain_support.present_mode_count, MEMORY_TAG_RENDERER);
    }

    if(vk_context->device.swapchain_support.formats) {
        kmemory::free(vk_context->device.swapchain_support.formats, 
            sizeof(VkSurfaceFormatKHR) * vk_context->device.swapchain_support.format_count, MEMORY_TAG_RENDERER);
    }
}

b8 select_physical_device(vulkan_context* context) {
    u32 physical_device_count = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physical_device_count, 0));
    if(physical_device_count == 0) {
        KFATAL("No devices which support Vulkan were found in your system.");
        return false;
    }
    
    darray<VkPhysicalDevice> physical_devices(physical_device_count);
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physical_device_count, physical_devices.get_array()));
    physical_devices.field_set(DARRAY_LENGTH, physical_device_count);
    for(u32 i = 0; i < physical_device_count; ++i) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physical_devices[i], &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(physical_devices[i], &features);

        VkPhysicalDeviceMemoryProperties memory_properties;
        vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &memory_properties);

        // our requirements we have
        // TODO: these requirements should be probably be driven by engine configuration
        vulkan_physical_device_requirements requirements = {};
        requirements.graphics = true;
        requirements.present = true;
        requirements.transfer = true;
        // NOTE: Enable this if compute will be required
        //requirements.compute = true;
        requirements.sampler_anisotropy = true;
        requirements.discrete_gpu = true;
        // darray<const char*> required_extension_names_darray; // default initialized to capacity of 1, size (length) of 0
        // required_extension_names_darray.push(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        requirements.device_extension_names_darray = darray<const char*>();
        requirements.device_extension_names_darray.push(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        vulkan_physical_device_queue_family_info queue_info = {};
        b8 result = physical_device_meets_requirements(
            physical_devices[i],
            context->surface,
            &properties,
            &features,
            &requirements,
            &queue_info,
            &context->device.swapchain_support
        );

        if(result) {
            KINFO("--Selected device: '%s'", properties.deviceName);
            // GPU type, etc.
            switch (properties.deviceType) {
                default:
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                    KINFO("----GPU type is Unknown.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    KINFO("----GPU type is Intergrated.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    KINFO("----GPU type is Discrete.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    KINFO("----GPU type is Virtual.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    KINFO("----GPU type is CPU.");
                    break;
            }
            KINFO("GPU Driver version: %d.%d.%d", 
                VK_VERSION_MAJOR(properties.driverVersion),
                VK_VERSION_MINOR(properties.driverVersion),
                VK_VERSION_PATCH(properties.driverVersion)
            );
    
            KINFO("Vulkan API version: %d.%d.%d", 
                VK_VERSION_MAJOR(properties.apiVersion),
                VK_VERSION_MINOR(properties.apiVersion),
                VK_VERSION_PATCH(properties.apiVersion)
            );
    
            // memory information
            for(u32 j = 0; j < memory_properties.memoryHeapCount; ++j) {
                f32 memory_size_gib = (((f32)memory_properties.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);
                if(memory_properties.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                    KINFO("--Local GPU memory: %.2f GiB", memory_size_gib);
                }
                else {
                    KINFO("--Shared system memory: %.2f GiB", memory_size_gib);
                }
            }

            
            context->device.physical_device = physical_devices[i];
            context->device.graphics_queue_index = queue_info.graphics_family_index;
            context->device.present_queue_index = queue_info.present_family_index;
            context->device.transfer_queue_index = queue_info.transfer_family_index;
            // NOTE: set compute index here if needed
            
            // keep a copy of properties, features and memory info for later use.
            context->device.properties = properties;
            context->device.features = features;
            context->device.memory = memory_properties;
            break;
            
        }
    }

    // ensure a device was selected
    if(!context->device.physical_device) {
        KERROR("--No physical devices were found which meet the requirements.");
        return false;
    }

    KINFO("-- Physical device selected.");
    return true;
}

b8 physical_device_meets_requirements(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const vulkan_physical_device_requirements* requirements,
    vulkan_physical_device_queue_family_info* out_queue_family_info,
    vulkan_swapchain_support_info* out_swapchain_support_info
) {
    // if smth is -1 it means it's not supported
    out_queue_family_info->graphics_family_index = -1; // wraps around u32 and sets that number to some insane value LULE
    out_queue_family_info->present_family_index = -1;
    out_queue_family_info->compute_family_index = -1;
    out_queue_family_info->transfer_family_index = -1;

    // Discrete GPU?
    if(requirements->discrete_gpu) {
        if(properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            KINFO("-- Choosing device ---- Device is not a discrete GPU, and one is required. Skipping.");
            return false;
        }
    }

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, 0);
    darray<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.get_array());
    queue_families.field_set(DARRAY_LENGTH, queue_family_count);
    // Look at each queue and see what queues it supports.
    KINFO("Graphics | Present | Compute | Transfer | Name");
    u32 min_transfer_score = 255;
    for(u32 i = 0; i < queue_family_count; ++i) {
        u8 current_transfer_score = 0;

        // Graphics queue?
        if(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            out_queue_family_info->graphics_family_index = i;
            ++current_transfer_score;
        }

        //Compute queue?
        if(queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            out_queue_family_info->compute_family_index = i;
            ++current_transfer_score;
        }

        //Transfer queue?
        if(queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            // take the index if it is the current lowest. This increases the likelihood that it's a dedicated transfer queue.
            if(current_transfer_score <= min_transfer_score) {
                out_queue_family_info->transfer_family_index = i;
                ++current_transfer_score;
            }
        }

        //Present queue?
        VkBool32 supports_present = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supports_present));
        if(supports_present) {
            out_queue_family_info->present_family_index = i;
        }
    }

    // print out some info about the device
    KINFO("         %d |         %d |         %d |         %d | %s ",
        out_queue_family_info->graphics_family_index    != -1,
        out_queue_family_info->present_family_index     != -1,
        out_queue_family_info->compute_family_index     != -1,
        out_queue_family_info->transfer_family_index    != -1,
        properties->deviceName
    );

    // does that actually meet requirements we wish -> ugly mf
    // if smth IS NOT a requirement OR smth (IS a requirement AND we have an index for it) 
    if( (!requirements->graphics || (requirements->graphics && out_queue_family_info->graphics_family_index != -1))  &&
        (!requirements->present  || (requirements->present && out_queue_family_info->present_family_index != -1))   &&
        (!requirements->compute  || (requirements->compute && out_queue_family_info->compute_family_index != -1))   &&
        (!requirements->transfer || (requirements->transfer && out_queue_family_info->transfer_family_index != -1))
    ) {
        KINFO("--Device meets queue requirements");
        KTRACE("---- Graphics Family Index: %i", out_queue_family_info->graphics_family_index);
        KTRACE("---- Present Family Index: %i", out_queue_family_info->present_family_index);
        KTRACE("---- Transfer Family Index: %i", out_queue_family_info->transfer_family_index);
        KTRACE("---- Compute Family Index: %i", out_queue_family_info->compute_family_index);
        
        // Query swapchain support
        vulkan_device_query_swapchain_support(device, surface, out_swapchain_support_info);
        if(out_swapchain_support_info->format_count < 1 || out_swapchain_support_info->present_mode_count < 1) {
            if(out_swapchain_support_info->formats) {
                kmemory::free(out_swapchain_support_info->formats, 
                    sizeof(VkSurfaceFormatKHR) * out_swapchain_support_info->format_count, MEMORY_TAG_RENDERER);
            }
            if(out_swapchain_support_info->present_modes) {
                kmemory::free(out_swapchain_support_info->present_modes, 
                    sizeof(VkPresentModeKHR) * out_swapchain_support_info->present_mode_count, MEMORY_TAG_RENDERER);
            }
            KINFO("-- Required swapchain support not present, skipping device");
            return false;
        }

        // Device extensions
        if(requirements->device_extension_names_darray.get_array()) {
            u32 available_extensions_count = 0;
            VkExtensionProperties* available_extensions = 0;
            VK_CHECK(vkEnumerateDeviceExtensionProperties(
                device,
                0,
                &available_extensions_count,
                0
            ));

            if(available_extensions_count != 0) {
                available_extensions = static_cast<VkExtensionProperties*>(kmemory::allocate(
                    sizeof(VkExtensionProperties) * available_extensions_count, MEMORY_TAG_RENDERER
                ));
                VK_CHECK(vkEnumerateDeviceExtensionProperties(
                    device,
                    0,
                    &available_extensions_count,
                    available_extensions
                ));

                u32 required_extension_count = requirements->device_extension_names_darray.get_length();
                for(u32 i = 0; i < required_extension_count; ++i) {
                    b8 found = false;
                    for(u32 j = 0; j < available_extensions_count; ++j) {
                        if(kstring::are_equal(requirements->device_extension_names_darray.get_array()[i], 
                                              available_extensions[j].extensionName)) {
                            found = true;
                            break;
                        }
                    }

                    if(!found) {
                        KINFO("--Required extension not found: '%s', skipping device.", 
                            requirements->device_extension_names_darray[i]);
                        kmemory::free(available_extensions, sizeof(VkExtensionProperties) * available_extensions_count,  MEMORY_TAG_RENDERER);
                        return false;
                    }
                }
            }
            // if we found required extensions we are good to go and we free up that memory
            kmemory::free(available_extensions, sizeof(VkExtensionProperties) * available_extensions_count,  MEMORY_TAG_RENDERER);
        }


        // Sampler anisotropy
        if(requirements->sampler_anisotropy && !features->samplerAnisotropy) {
            KINFO("-- Device does not support samplerAnisotropy, skipping.");
            return false;
        }

        // device meets all requirements.
        return true;
    }

    // if we somehow failed all checks we cannot use this device
    return false;
}

void vulkan_device_query_swapchain_support(VkPhysicalDevice physical_device,
    VkSurfaceKHR surface,
    vulkan_swapchain_support_info* out_support_info
) {
    // Surface capabilities
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &out_support_info->capabilities));
    
    // Surface formats
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface,
        &out_support_info->format_count,
        0)
    );
    
    if(out_support_info->format_count != 0) {
        if(!out_support_info->formats) {
            // literally could use darray for these allocations management but we want to track it for renderer
            out_support_info->formats = static_cast<VkSurfaceFormatKHR*>(
                kmemory::allocate(sizeof(VkSurfaceFormatKHR) * out_support_info->format_count, MEMORY_TAG_RENDERER)
            );
        }
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface,
            &out_support_info->format_count,
            out_support_info->formats));
    }

    // Present modes
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
        &out_support_info->present_mode_count, 0));
    
    if(out_support_info->present_mode_count != 0) {
        if(!out_support_info->present_modes) {
            out_support_info->present_modes = static_cast<VkPresentModeKHR*>(
                kmemory::allocate(sizeof(VkPresentModeKHR) * out_support_info->present_mode_count, MEMORY_TAG_RENDERER)
            );
        }
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
            &out_support_info->present_mode_count, out_support_info->present_modes));
    }
    
}