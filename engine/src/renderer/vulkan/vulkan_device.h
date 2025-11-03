#pragma once
#include "vulkan_types.inl"

// could create vulkan_device class (or with similar name) but there already is the vulkan_device typedef struct
// in vulkan_types.inl

b8 vulkan_device_create(vulkan_context* vk_context);
void vulkan_device_destroy(vulkan_context* vk_context);

void vulkan_device_query_swapchain_support(VkPhysicalDevice physical_device,
    VkSurfaceKHR surface,
    vulkan_swapchain_support_info* out_support_info
);

b8 vulkan_device_detect_depth_format(vulkan_device* device);