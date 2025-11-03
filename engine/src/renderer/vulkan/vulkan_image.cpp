#include "vulkan_image.h"

#include "vulkan_device.h"
#include "core/logger.h"
#include "core/kmemory.h"

void vulkan_image_create(
    vulkan_context* context,
    VkImageType image_type,
    u32 width,
    u32 height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage_flags,
    VkMemoryPropertyFlags memory_flags,
    b32 create_view,
    VkImageAspectFlags view_aspect_flags,
    vulkan_image* out_image
) {
    // copy params
    out_image->width = width;
    out_image->height = height;

    // creation info.
    VkImageCreateInfo image_create_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = width;
    image_create_info.extent.height = height;
    image_create_info.extent.depth = 1; // TODO: Support configurable depth
    image_create_info.mipLevels = 4; // TODO: Support mip mapping
    image_create_info.arrayLayers = 1; // TODO: Support number of layers in the image.
    image_create_info.format = format;
    image_create_info.tiling = tiling;      // VK_IMAGE_TILING_OPTIMAL is the best.
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = usage_flags;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;                  // TODO: Configurable sample count.
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;          // TODO: Configurable sharing mode.
    VK_CHECK(vkCreateImage(context->device.logical_device, &image_create_info, context->allocator, &out_image->handle));

    // query memory requirements for that image.
    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(context->device.logical_device, out_image->handle, &memory_requirements);

    i32 memory_type_index_for_image = context->find_memory_index(memory_requirements.memoryTypeBits, memory_flags);
    if(memory_type_index_for_image == -1) {
        KERROR("--Required memory type not found. Image not valid.");
    }

    // allocate memory
    VkMemoryAllocateInfo memory_for_image_AI = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    memory_for_image_AI.allocationSize = memory_requirements.size;
    memory_for_image_AI.memoryTypeIndex = memory_type_index_for_image;
    VK_CHECK(vkAllocateMemory(context->device.logical_device, &memory_for_image_AI, context->allocator, &out_image->memory_handle));

    // bind the memory
    VK_CHECK(vkBindImageMemory(context->device.logical_device, out_image->handle, 
            out_image->memory_handle, 0));          // TODO: configurable memory offset.

    if(create_view) {
        out_image->view = 0;
        vulkan_image_view_create(context, format, out_image, view_aspect_flags);
    }
}

void vulkan_image_view_create(vulkan_context* context, VkFormat format, 
            vulkan_image* image, VkImageAspectFlags aspect_flags) {
    VkImageViewCreateInfo view_CI = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_CI.image = image->handle;
    view_CI.viewType = VK_IMAGE_VIEW_TYPE_2D; // TODO: Make configurable
    view_CI.format = format;
    view_CI.subresourceRange.aspectMask = aspect_flags;

    // TODO: make it configurable
    view_CI.subresourceRange.baseMipLevel = 0;
    view_CI.subresourceRange.levelCount = 1;
    view_CI.subresourceRange.baseArrayLayer = 0;
    view_CI.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(context->device.logical_device, &view_CI, context->allocator, &image->view));
}

void vulkan_image_destroy(vulkan_context* context, vulkan_image* image) {
    if(image->view) {
        vkDestroyImageView(context->device.logical_device, image->view, context->allocator);
        image->view = nullptr;
    }
    if(image->memory_handle) {
        vkFreeMemory(context->device.logical_device, image->memory_handle, context->allocator);
        image->memory_handle = nullptr;
    }
    if(image->handle) {
        vkDestroyImage(context->device.logical_device, image->handle, context->allocator);
        image->handle = nullptr;
    }
}