#include "renderer_frontend.h"
#include "renderer_backend.h"

#include "core/logger.h"
#include "core/kmemory.h"

renderer_backend_struct* renderer::backend = nullptr;

b8 renderer::initialize(const char* application_name, platform_state* plat_state) {
    backend = reinterpret_cast<renderer_backend_struct*>(kmemory::allocate(sizeof(renderer_backend_struct), MEMORY_TAG_RENDERER));
    
    // TODO: In future maybe make it configurable if e.g. I would want to implement DirectX 12 renderer
    renderer_backend::create(RENDERER_BACKEND_TYPE_VULKAN, plat_state, backend);
    backend->frame_number = 0;

    if(!backend->initialize(backend, application_name, plat_state)) {
        KFATAL("Renderer backend failed to initialize. Shutting down.");
        return false;
    }


    return true;
}

void renderer::shutdown() {
    backend->shutdown(backend);
    kmemory::free(backend, sizeof(renderer_backend_struct), MEMORY_TAG_RENDERER);
}

b8 renderer::begin_frame(f32 delta_time) {
    return backend->begin_frame(backend, delta_time);
}

b8 renderer::end_frame(f32 delta_time) {
    b8 result = backend->end_frame(backend, delta_time);
    backend->frame_number++;
    return result;
}

void renderer::on_resized(u16 width, u16 height) {
    if(backend) {
        backend->resized(backend, width, height);
    }
    else {
        KWARN("renderer backend does not exist to accept resize: %i %i", width, height);
    }
}

b8 renderer::draw_frame(render_packet* packet) {
    // if  this returns true, mid-frame operations may continue
    if(begin_frame(packet->delta_time)) {

        // end the frame. If this fails, it is likely unrecoverable
        b8 result = end_frame(packet->delta_time);
        if(!result) {
            KERROR("renderer::end_frame failed. Application shutting down.");
            return false;
        }

    }

    return true;
}