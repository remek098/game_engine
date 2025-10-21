#pragma once
#include "defines.h"



#include <string>
#include <cstdint>

// forward declerations for structs and classes
struct vulkan_context;

typedef struct platform_state {
    void* internal_state;
} platform_state;

class KAPI platform {
public:
    // Constructor/Destructor
    platform();
    ~platform();

    b8 startup(const std::string& applicationName, int32_t x, int32_t y, int32_t width, int32_t height);
    void shutdown();

    // Instance-based functionality
    b8 pump_messages();

    // --- Static utility functions ---
    // Memory -> irrelevant once we get our Memory subsystem running
    static void* allocate(u64 size, b8 aligned);
    static void  free(void* block, b8 aligned);
    static void* zero_memory(void* block, u64 size);
    static void* copy_memory(void* dest, const void* source, u64 size);
    static void* set_memory(void* dest, i32  value, u64 size);

    // Console
    static void console_write(const char* message, u8 color);
    static void console_writeError(const char* message, u8 color);

    // Timing
    static f64  get_absolute_time();
    static void sleep(f64 ms);


    // vulkan related stuff
    static b8 create_vulkan_surface(struct platform_state* plat_state, struct vulkan_context* context);
    static void destroy_vulkan_surface(struct vulkan_context* context);
    
public:
    // could use std::unique_ptr to avoid some extra care
    platform_state* plat_state;
    
private:
    const std::string m_class_name = "game_engine";
};