#pragma once

#include "renderer/renderer_backend.h"

#include "vulkan_types.inl"

class vulkan_renderer_backend {
public:
    static b8 initialize(renderer_backend_struct* backend, const char* application_name, platform_state* plat_state);
    static void shutdown(renderer_backend_struct* backend);

    static void on_resized(renderer_backend_struct* backend, u16 width, u16 height);
    
    static b8 begin_frame(renderer_backend_struct* backend, f32 delta_time);
    static b8 end_frame(renderer_backend_struct* backend, f32 delta_time);

public:
    static vulkan_context context;
};