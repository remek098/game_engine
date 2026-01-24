#include "vulkan_swapchain.h"

#include "core/logger.h"
#include "core/kmemory.h"
#include "vulkan_device.h"
#include "vulkan_image.h"

void create(vulkan_context* context, u32 width, u32 height, vulkan_swapchain* swapchain);
void destroy(vulkan_context* context, vulkan_swapchain* swapchain);

void vulkan_swapchain_create(vulkan_context* context, u32 width, u32 height, vulkan_swapchain* out_swapchain) {
    create(context, width, height, out_swapchain); // create a new one
}

void vulkan_swapchain_recreate(vulkan_context* context, u32 width, u32 height, vulkan_swapchain* swapchain) {
    destroy(context, swapchain);
    create(context, width, height, swapchain);
}

void vulkan_swapchain_destroy(vulkan_context* context, vulkan_swapchain* swapchain) {
    destroy(context, swapchain);
}


b8 vulkan_swapchain_acquire_next_image_index(
    vulkan_context* context, 
    vulkan_swapchain* swapchain,
    u64 timeout_ns, 
    VkSemaphore image_available_semaphore, 
    VkFence fence, 
    u32* out_image_index
) {
    VkResult result = vkAcquireNextImageKHR(
        context->device.logical_device, swapchain->handle, timeout_ns, image_available_semaphore, fence, out_image_index);

    if(result == VK_ERROR_OUT_OF_DATE_KHR) {
        // trigger swapchain recreation and boot out of loop
        vulkan_swapchain_recreate(context, context->framebuffer_width, context->framebuffer_height, swapchain);
        return false;
    }
    else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        KFATAL("-- Failed to acquire swapchain image!");
        return false;
    }

    return true;
}

void vulkan_swapchain_present(
    vulkan_context* context,
    vulkan_swapchain* swapchain,
    VkQueue graphics_queue,
    VkQueue present_queue,
    VkSemaphore render_complete_semaphore,
    u32 present_image_index
) {
    // return the image to swapchain for presentation
    VkPresentInfoKHR present_info = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &render_complete_semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swapchain->handle;
    // whatever index is retrieved by vulkan_swapchain_acquire_next_image_index() should be passed there
    present_info.pImageIndices = &present_image_index; 
    present_info.pResults = 0;
    
    VkResult result = vkQueuePresentKHR(present_queue, &present_info);
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // it means swapchain is out of date
        vulkan_swapchain_recreate(context, context->framebuffer_width, context->framebuffer_height, swapchain);
    }
    else if(result != VK_SUCCESS) {
        KFATAL("-- Failed to present swapchain image.");
    }

    context->current_frame = (context->current_frame + 1) % swapchain->max_frames_in_flight;
}

void create(vulkan_context* context, u32 width, u32 height, vulkan_swapchain* swapchain) {
    VkExtent2D swapchain_extent = {width, height};
    swapchain->max_frames_in_flight = 2; // triple buffering -> 2 frames rendered as we draw 3rd frame

    // choose a swap surface format
    b8 found = false;
    for(u32 i = 0; i < context->device.swapchain_support.format_count; ++i) {
        VkSurfaceFormatKHR format = context->device.swapchain_support.formats[i]; // retrieved during device creation
        if(format.format == VK_FORMAT_B8G8R8A8_UNORM && 
                    format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            swapchain->image_format = format;
            found = true;
            break;
        }
    }

    if(!found) {
        // if we didn't detect our prefered format, stick to the any available
        swapchain->image_format = context->device.swapchain_support.formats[0]; 
    }

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR; // this has to exist regarding vulkan spec
    for(u32 i = 0; i < context->device.swapchain_support.present_mode_count; ++i) {
        VkPresentModeKHR mode = context->device.swapchain_support.present_modes[i];
        // by default uses triple buffering => drawing 2 frames and choosing  the newest one -> gives the lowest latency
        // ofc there's immidiate mode but that disables VSYNC
        if(mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            present_mode = mode;
            break;
        }
    }

    // requery swapchain support
    //NOTE: this allocates memory to context->device.swapchain_support. Once unused pls release that memory (formats and present_modes)
    vulkan_device_query_swapchain_support(context->device.physical_device, context->surface, &context->device.swapchain_support);

    // swapchain extent
    if(context->device.swapchain_support.capabilities.currentExtent.width != UINT32_MAX) {
        // if we don't have some BS value we setup our VkExtent2D to the maximum value capable
        swapchain_extent = context->device.swapchain_support.capabilities.currentExtent;
    }

    // clamp to the value allowed by GPU
    VkExtent2D min = context->device.swapchain_support.capabilities.minImageExtent;
    VkExtent2D max = context->device.swapchain_support.capabilities.maxImageExtent;;
    swapchain_extent.width = KCLAMP(swapchain_extent.width, min.width, max.width);
    swapchain_extent.height = KCLAMP(swapchain_extent.height, min.height, max.height);

    // get supported image count. Usually can be 2. 
    // If for some reason our image_count is > maxImageCount just use that
    u32 image_count = context->device.swapchain_support.capabilities.minImageCount + 1;
    if(context->device.swapchain_support.capabilities.maxImageCount > 0 && 
                image_count > context->device.swapchain_support.capabilities.maxImageCount) {
        image_count = context->device.swapchain_support.capabilities.maxImageCount;
    }

    // swapchain create info
    VkSwapchainCreateInfoKHR sc_create_info = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    sc_create_info.surface = context->surface;
    sc_create_info.minImageCount = image_count;
    sc_create_info.imageFormat = swapchain->image_format.format;
    sc_create_info.imageColorSpace = swapchain->image_format.colorSpace;
    sc_create_info.imageExtent = swapchain_extent;
    sc_create_info.imageArrayLayers = 1;
    sc_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // setup queue family indices.
    if(context->device.graphics_queue_index != context->device.present_queue_index) {
        u32 queueFamilyIndices[] = {
            (u32)context->device.graphics_queue_index,
            (u32)context->device.present_queue_index
        };
        sc_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        sc_create_info.queueFamilyIndexCount = 2;
        sc_create_info.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        sc_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        sc_create_info.queueFamilyIndexCount = 0;
        sc_create_info.pQueueFamilyIndices = 0;
    }

    // TODO: Temporary currentTransform -> will swap it out later once we need
    sc_create_info.preTransform = context->device.swapchain_support.capabilities.currentTransform;
    sc_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sc_create_info.presentMode = present_mode;
    sc_create_info.clipped = VK_TRUE;
    sc_create_info.oldSwapchain = 0;

    // NOTE: Swapchain on creation also creates it's images. We need to get them from swapchain
    VK_CHECK(vkCreateSwapchainKHR(context->device.logical_device, &sc_create_info, context->allocator, &swapchain->handle));

    // start with zero frame index
    context->current_frame = 0;
    // images
    swapchain->image_count = 0; // query image_count
    VK_CHECK(vkGetSwapchainImagesKHR(context->device.logical_device, swapchain->handle, &swapchain->image_count, 0));
    if(!swapchain->images) {
        swapchain->images = (VkImage*)kmemory::allocate(sizeof(VkImage) * swapchain->image_count, MEMORY_TAG_RENDERER);
    }
    if(!swapchain->image_views) {
        swapchain->image_views = (VkImageView*)kmemory::allocate(sizeof(VkImageView) * swapchain->image_count, MEMORY_TAG_RENDERER);
    }
    // get images from swapchain
    VK_CHECK(vkGetSwapchainImagesKHR(context->device.logical_device, swapchain->handle, &swapchain->image_count, swapchain->images));

    // create image views for each image
    for(u32 i = 0; i < swapchain->image_count; ++i) {
        VkImageViewCreateInfo view_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        view_info.image = swapchain->images[i];
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = swapchain->image_format.format;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;
        VK_CHECK(vkCreateImageView(context->device.logical_device, &view_info, context->allocator, &swapchain->image_views[i]));
    }

    // get depth format
    if(!vulkan_device_detect_depth_format(&context->device)) {
        context->device.depth_format = VK_FORMAT_UNDEFINED;
        KFATAL("-- Failed to find a supported depth format!");
    }

    // need to create image for depth buffer and it's view
    vulkan_image_create(
        context,
        VK_IMAGE_TYPE_2D,
        swapchain_extent.width,
        swapchain_extent.height,
        context->device.depth_format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        true,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        &swapchain->depth_attachment
    );

    KINFO("-- Swapchain created successfully.");
}

void destroy(vulkan_context* context, vulkan_swapchain* swapchain) {
    vkDeviceWaitIdle(context->device.logical_device);

    // NOTE: allocations so far: 
    /**
     * 1)  (formats and present_modes) in swapchain -> actually this is freed by vulkan_device_destroy(); since this memory is owned by vulkan_device
     *                      in our context
     * 2) swapchain->images; swapchain->views
     * mental NOTE: Remember to use destroy function (or vulkan_swapchain_destroy in other files)
     */

    vulkan_image_destroy(context, &swapchain->depth_attachment);

    // destroy only views since images are owned by swapchain
    for(u32 i = 0; i < swapchain->image_count; ++i) {
        vkDestroyImageView(context->device.logical_device, swapchain->image_views[i], context->allocator);
    }

    vkDestroySwapchainKHR(context->device.logical_device, swapchain->handle, context->allocator);

    vkDeviceWaitIdle(context->device.logical_device);
    kmemory::free(swapchain->images, sizeof(VkImage) * swapchain->image_count, MEMORY_TAG_RENDERER);
    kmemory::free(swapchain->image_views, sizeof(VkImageView) * swapchain->image_count, MEMORY_TAG_RENDERER);
    swapchain->images = nullptr;
    swapchain->image_views = nullptr;
}