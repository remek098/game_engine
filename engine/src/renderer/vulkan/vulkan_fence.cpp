#include "vulkan_fence.h"
#include "core/logger.h"

void vulkan_fence_create(
    vulkan_context* context,
    b8 create_signaled,
    vulkan_fence* out_fence
) {
    // make sure to signal fence if required
    out_fence->is_signaled = create_signaled;
    VkFenceCreateInfo fence_ci = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if(out_fence->is_signaled) {
        fence_ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

    VK_CHECK(vkCreateFence(
        context->device.logical_device,
        &fence_ci,
        context->allocator,
        &out_fence->handle
    ));
}

void vulkan_fence_destroy(vulkan_context* contect, vulkan_fence* fence) {
    if(fence->handle) {
        vkDestroyFence(contect->device.logical_device, fence->handle, contect->allocator);
        fence->handle = 0;
    }
    fence->is_signaled = false;
}


b8 vulkan_fence_wait(vulkan_context* context, vulkan_fence* fence, u64 timeout_ns) {
    if(!fence->is_signaled) {
        VkResult result = vkWaitForFences(
            context->device.logical_device,
            1,
            &fence->handle,
            true,
            timeout_ns
        );


        switch (result) {
            case VK_SUCCESS: {
                fence->is_signaled = true;
                return true;
            }
            case VK_TIMEOUT: {
                KWARN("vulkan_fence_wait - Timed out");
                break;
            }
            case VK_ERROR_DEVICE_LOST: {
                KWARN("vulkan_fence_wait - VK_ERROR_DEVICE_LOST");
                break;
            }
            case VK_ERROR_OUT_OF_HOST_MEMORY: {
                KWARN("vulkan_fence_wait - VK_ERROR_OUT_OF_HOST_MEMORY");
                break;
            }
            case VK_ERROR_OUT_OF_DEVICE_MEMORY: {
                KWARN("vulkan_fence_wait - VK_ERROR_OUT_OF_DEVICE_MEMORY");
                break;
            }
            default: {
                KERROR("vulkan_fence_wait - An unknown error has occured.");
                break;
            }
        }
    }
    else {
        // if already signaled, do not wait.
        return true;
    }

    return false;
}

void vulkan_fence_reset(vulkan_context* context, vulkan_fence* fence) {
    if(fence->is_signaled) {
        VK_CHECK(vkResetFences(context->device.logical_device, 1, &fence->handle));
        fence->is_signaled = false;
    }
}