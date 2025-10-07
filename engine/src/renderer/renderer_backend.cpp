#include "renderer_backend.h"
#include "renderer/vulkan/vulkan_backend.h"

b8 renderer_backend::create(renderer_backend_type type,  platform_state* plat_state, renderer_backend_struct* out_renderer_backend) {
    out_renderer_backend->plat_state = plat_state;

    if(type == RENDERER_BACKEND_TYPE_VULKAN) {
        // setup function pointers
        out_renderer_backend->initialize = vulkan_renderer_backend::initialize;
        out_renderer_backend->shutdown = vulkan_renderer_backend::shutdown;
        out_renderer_backend->begin_frame = vulkan_renderer_backend::begin_frame;
        out_renderer_backend->end_frame = vulkan_renderer_backend::end_frame;
        out_renderer_backend->resized = vulkan_renderer_backend::on_resized;

        return true;
    }

    return false;
}

void renderer_backend::destroy(renderer_backend_struct* renderer_backend) {
    renderer_backend->initialize = 0;
    renderer_backend->shutdown = 0;
    renderer_backend->begin_frame = 0;
    renderer_backend->end_frame = 0;
    renderer_backend->resized = 0;
    
    // renderer_backend->plat_state = nullptr;
}

