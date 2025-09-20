#pragma once
#include "renderer_types.inl"

struct platform_state;

class renderer_backend {
public:

    static b8 create(renderer_backend_type type, platform_state* plat_state, 
        renderer_backend_struct* out_renderer_backend);
    
    static void destroy(renderer_backend_struct* renderer_backend);
};