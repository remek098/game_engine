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

    KINFO("-- Creating logical device.");
    // NOTE: Do not create additional queues for shared indices.
    b8 present_shares_graphics_queue = vk_context->device.graphics_queue_index == vk_context->device.present_queue_index;
    b8 transfer_shares_graphics_queue = vk_context->device.graphics_queue_index == vk_context->device.transfer_queue_index;
    u32 index_count = 1;

    if(!present_shares_graphics_queue)
        index_count++;
    if(!transfer_shares_graphics_queue)
        index_count++;

    u32 indices[index_count];
    u8 index = 0;
    indices[index++] = vk_context->device.graphics_queue_index;
    if(!present_shares_graphics_queue) {
        indices[index++] = vk_context->device.present_queue_index;
    }
    if(!transfer_shares_graphics_queue) {
        indices[index++] = vk_context->device.transfer_queue_index;
    }

    // fill it out for each index
    VkDeviceQueueCreateInfo queue_create_infos[index_count];
    for(u32 i = 0; i < index_count; ++i) {
        queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[i].queueFamilyIndex = indices[i];
        queue_create_infos[i].queueCount = 1;

        //TODO: Can be enabled. Doesn't have to for now.
        if(indices[i] == vk_context->device.graphics_queue_index) {
            queue_create_infos[i].queueCount = 2;
        }
        queue_create_infos[i].flags = 0;
        queue_create_infos[i].pNext = 0;
        f32 queue_priority = 1.0f;
        queue_create_infos[i].pQueuePriorities = &queue_priority;
    }

    // request device features.
    // TODO: Should be config driven
    VkPhysicalDeviceFeatures device_features = {};
    device_features.samplerAnisotropy = VK_TRUE; // request anisotrophy

    VkDeviceCreateInfo device_create_info = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    device_create_info.queueCreateInfoCount = index_count;
    device_create_info.pQueueCreateInfos = queue_create_infos;
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = 1;
    const char* extension_names = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    device_create_info.ppEnabledExtensionNames = &extension_names;

    // deprecated and ignored, pass nothing -> being explicit for no particular reason
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = 0;

    // create logical device
    VK_CHECK(vkCreateDevice(vk_context->device.physical_device, 
        &device_create_info, vk_context->allocator, &vk_context->device.logical_device));

    KINFO("-- Logical device created.");
    
    // hard coding 0 because it's the first queue we're requesting from that family. If another we would need to increment that.
    vkGetDeviceQueue(vk_context->device.logical_device, vk_context->device.graphics_queue_index, 0, &vk_context->device.graphics_queue);
    vkGetDeviceQueue(vk_context->device.logical_device, vk_context->device.present_queue_index, 0, &vk_context->device.present_queue);
    vkGetDeviceQueue(vk_context->device.logical_device, vk_context->device.transfer_queue_index, 0, &vk_context->device.transfer_queue);
    KINFO("-- Queues obtained.");

    // Create command pool for graphics queue
    VkCommandPoolCreateInfo pool_create_info = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pool_create_info.queueFamilyIndex = vk_context->device.graphics_queue_index;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK(vkCreateCommandPool(vk_context->device.logical_device, &pool_create_info, 
        vk_context->allocator, &vk_context->device.graphics_command_pool));

    KINFO("--Graphics command pool has been created.");
    return true;
}


void vulkan_device_destroy(vulkan_context* vk_context) {
    // unset queues
    vk_context->device.graphics_queue = 0;
    vk_context->device.present_queue = 0;
    vk_context->device.transfer_queue = 0;

    KINFO("--Destroying command pools");
    vkDestroyCommandPool(vk_context->device.logical_device, vk_context->device.graphics_command_pool,
        vk_context->allocator);


    KINFO("----------------------------------Destroying logical device..");
    if(vk_context->device.logical_device) {
        vkDestroyDevice(vk_context->device.logical_device, vk_context->allocator);
        vk_context->device.logical_device = 0;
    }

    KINFO("-----------------------------------Releasing physical device resources ...");
    vk_context->device.physical_device = 0;

    // potentially to free:
    // swapchain_support.formats and swapchain_support.present_modes
    if(vk_context->device.swapchain_support.present_modes) {
        kmemory::free(vk_context->device.swapchain_support.present_modes, 
            sizeof(VkPresentModeKHR) * vk_context->device.swapchain_support.present_mode_count, MEMORY_TAG_RENDERER);
        vk_context->device.swapchain_support.present_modes = 0;
        vk_context->device.swapchain_support.present_mode_count = 0;
    }

    if(vk_context->device.swapchain_support.formats) {
        kmemory::free(vk_context->device.swapchain_support.formats, 
            sizeof(VkSurfaceFormatKHR) * vk_context->device.swapchain_support.format_count, MEMORY_TAG_RENDERER);
        vk_context->device.swapchain_support.formats = 0;
        vk_context->device.swapchain_support.format_count = 0;
    }

    kmemory::zero(&vk_context->device.swapchain_support.capabilities, sizeof(VkSurfaceCapabilitiesKHR));
    vk_context->device.graphics_queue_index = -1;
    vk_context->device.present_queue_index = -1;
    vk_context->device.transfer_queue_index = -1;
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
                available_extensions = reinterpret_cast<VkExtensionProperties*>(kmemory::allocate(
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

b8 vulkan_device_detect_depth_format(vulkan_device* device) {
    const u64 candidate_count = 3;
    VkFormat candidates[3] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for(u64 i = 0; i < candidate_count; ++i) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(device->physical_device, candidates[i], &properties);
        if((properties.linearTilingFeatures & flags) == flags) {
            device->depth_format = candidates[i];
            return true;
        }
        else if((properties.optimalTilingFeatures & flags) == flags) {
            device->depth_format = candidates[i];
            return true;
        }
    }
    
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
            out_support_info->formats = reinterpret_cast<VkSurfaceFormatKHR*>(
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
            out_support_info->present_modes = reinterpret_cast<VkPresentModeKHR*>(
                kmemory::allocate(sizeof(VkPresentModeKHR) * out_support_info->present_mode_count, MEMORY_TAG_RENDERER)
            );
        }
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
            &out_support_info->present_mode_count, out_support_info->present_modes));
    }
    
}