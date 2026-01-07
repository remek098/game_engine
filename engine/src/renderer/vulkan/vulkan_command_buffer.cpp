#include "vulkan_command_buffer.h"
#include "core/kmemory.h"

void vulkan_command_buffer_allocate(
    vulkan_context* context,
    VkCommandPool cmd_pool,
    b8 is_primary,
    vulkan_command_buffer* out_command_buffer
) {
    kmemory::zero(out_command_buffer, sizeof(*out_command_buffer)); // sizeof(vulkan_command_buffer)
    VkCommandBufferAllocateInfo cmdBuff_AI = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    cmdBuff_AI.commandPool = cmd_pool;
    cmdBuff_AI.level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    cmdBuff_AI.commandBufferCount = 1;
    cmdBuff_AI.pNext = 0;

    out_command_buffer->cmd_buff_state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
    VK_CHECK(vkAllocateCommandBuffers(context->device.logical_device, &cmdBuff_AI, &out_command_buffer->handle));
    out_command_buffer->cmd_buff_state = COMMAND_BUFFER_STATE_READY;
}

void vulkan_command_buffer_free(
    vulkan_context* context,
    VkCommandPool cmd_pool,
    vulkan_command_buffer* command_buffer
) {
    if(command_buffer->handle == VK_NULL_HANDLE)
        vkFreeCommandBuffers(context->device.logical_device, cmd_pool, 1, &command_buffer->handle);
    
    command_buffer->handle = 0;
    command_buffer->cmd_buff_state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
}

void vulkan_command_buffer_begin(
    vulkan_command_buffer* command_buffer,
    b8 is_single_use,
    b8 is_renderpass_continue,
    b8 is_simultanous_use
) {
    VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin_info.flags = 0;
    if(is_single_use)
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if(is_renderpass_continue)
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    if(is_simultanous_use)
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    VK_CHECK(vkBeginCommandBuffer(command_buffer->handle, &begin_info));
    command_buffer->cmd_buff_state = COMMAND_BUFFER_STATE_RECORDING;
}

void vulkan_command_buffer_end(vulkan_command_buffer* command_buffer) {
    VK_CHECK(vkEndCommandBuffer(command_buffer->handle));
    command_buffer->cmd_buff_state = COMMAND_BUFFER_STATE_ENDED;
}

void vulkan_command_buffer_update_submitted(vulkan_command_buffer* command_buffer) {
    command_buffer->cmd_buff_state = COMMAND_BUFFER_STATE_SUBMITTED;
}

void vulkan_command_buffer_reset(vulkan_command_buffer* command_buffer) {
    command_buffer->cmd_buff_state = COMMAND_BUFFER_STATE_READY;
}

void vulkan_command_buffer_allocate_and_begin_single_use(
    vulkan_context* context,
    VkCommandPool cmd_pool,
    vulkan_command_buffer* out_command_buffer
) {
    // NOTE: making an assumption that single_use command buffer will always be primiary, etc.
    vulkan_command_buffer_allocate(context, cmd_pool, true, out_command_buffer);
    vulkan_command_buffer_begin(out_command_buffer, true, false, false);
}

void vulkan_command_buffer_end_single_use(
    vulkan_context* context,
    VkCommandPool cmd_pool,
    vulkan_command_buffer* command_buffer,
    VkQueue queue
) {
    vulkan_command_buffer_end(command_buffer);

    // submit the queue
    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer->handle;
    VK_CHECK(vkQueueSubmit(queue, 1, &submit_info, 0));

    // wait for it to finish since we don't use fence.
    VK_CHECK(vkQueueWaitIdle(queue));
    
    vulkan_command_buffer_free(context, cmd_pool, command_buffer);
}