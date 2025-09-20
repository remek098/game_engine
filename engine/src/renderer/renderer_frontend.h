#pragma once

#include "renderer_types.inl"
#include "renderer/renderer_backend.h"

struct static_mesh_data;
struct platform_state;

class renderer {
public:
    static b8 initialize(const char* application_name, platform_state* plat_state);
    static void shutdown();

    static void on_resized(u16 width, u16 height);
    
    static b8 draw_frame(render_packet* packet);

    static b8 begin_frame(f32 delta_time);
    static b8 end_frame(f32 delta_time);

public:
    static renderer_backend_struct* backend;
};