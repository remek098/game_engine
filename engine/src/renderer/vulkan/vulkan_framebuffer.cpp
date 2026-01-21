#include "vulkan_framebuffer.h"
#include "core/kmemory.h"

void vulkan_framebuffer_create(
    vulkan_context* context,
    vulkan_renderpass* renderpass,
    u32 width, u32 height,
    u32 attachment_count,
    VkImageView* attachments,
    vulkan_framebuffer* out_framebuffer
) {
    // take a copy of attachments, renderpass and attachment_count
    out_framebuffer->attachments = reinterpret_cast<VkImageView*>(
        kmemory::allocate(sizeof(VkImageView) * attachment_count, MEMORY_TAG_RENDERER));

    for(u32 i = 0; i < attachment_count; ++i) {
        out_framebuffer->attachments[i] = attachments[i];
    }
    out_framebuffer->attachment_count = attachment_count;
    out_framebuffer->renderpass = renderpass;

    VkFramebufferCreateInfo framebuffer_ci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    framebuffer_ci.renderPass = renderpass->handle;
    framebuffer_ci.attachmentCount = attachment_count;
    framebuffer_ci.pAttachments = out_framebuffer->attachments;
    framebuffer_ci.width = width;
    framebuffer_ci.height = height;
    framebuffer_ci.layers = 1;
    VK_CHECK(vkCreateFramebuffer(context->device.logical_device, 
        &framebuffer_ci, context->allocator, &out_framebuffer->handle)
    );
    
}

void vulkan_framebuffer_destroy(vulkan_context* context, vulkan_framebuffer* framebuffer) {
    vkDestroyFramebuffer(context->device.logical_device, framebuffer->handle, context->allocator);

    if(framebuffer->attachments) {
        kmemory::free(framebuffer->attachments, sizeof(VkImageView) * framebuffer->attachment_count, MEMORY_TAG_RENDERER);
        framebuffer->attachments = nullptr;
    }
    framebuffer->handle = 0;
    framebuffer->attachment_count = 0;
    framebuffer->renderpass = 0;
}