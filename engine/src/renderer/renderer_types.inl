#pragma once
#include "defines.h"

typedef enum renderer_backend_type {
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_DIRECTX
} renderer_backend_type;

// I could just create interface which would make more sense in terms of using C++ but whatever I like it that way
typedef struct renderer_backend_struct {
    struct platform_state* plat_state;
    u64 frame_number;

    b8 (*initialize)(struct renderer_backend_struct* backend, const char* application_name, struct platform_state* palt_state);
    void (*shutdown)(struct renderer_backend_struct* backend);

    void (*resized)(struct renderer_backend_struct* backend, u16 width, u16 height);


    b8 (*begin_frame)(struct renderer_backend_struct* backend, f32 delta_time);
    b8 (*end_frame)(struct renderer_backend_struct* backend, f32 delta_time);

} renderer_backend_struct;

// class KAPI Irenderer_backend {
// public:
//     virtual ~Irenderer_backend();
    
//     virtual b8 initialize(const char* application_name, struct platform_state* plat_state) = 0;
//     virtual void shutdown() = 0;

//     virtual void resized(u16 width, u16 height) = 0;
    
//     virtual b8 draw_frame(render_packet* packet); 
// public:
//     struct platform_state* plat_state;
// };

typedef struct render_packet {
    f32 delta_time;
} render_packet;